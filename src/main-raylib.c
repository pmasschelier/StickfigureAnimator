#include "interface.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "src/components/utils.h"
#include "src/pivot.h"
#include "src/renderer/renderer.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/renderers/raylib/clay_renderer_raylib.c"

InterfaceData* data;
RendererContext *renderer_context;
GLFWwindow *window;

bool debugEnabled = false;

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector)                                 \
  (Clay_Vector2) { .x = vector.x, .y = vector.y }

#define CLAY_BOUNDINGBOX_TO_RAYLIB_RECTANGLE(boundingbox)                      \
  (Rectangle) {                                                                \
    .x = boundingbox.x, .y = boundingbox.y, .width = boundingbox.width,        \
    .height = boundingbox.height                                               \
  }

Font fonts[2];

void HandTakeStick(RendererData* data, PivotEdgeIndex index, Vector2 pointer) {
    Stickfigure* s = &data->stickfigure.data[index.figure];
    StickfigureEdge* edge = &s->edges.data[index.edge];
    data->hand.status = HAND_HOLDING_STICK;
    data->hand.edge.edge = index;
    data->hand.edge.initial = (PolarCoords) {
        .angle = edge->angle,
        .length = edge->length
    };
    data->hand.edge.pointerOffset = (PolarCoords) {
        .angle = edge->angle - PivotAngleFrom(s, edge->from, pointer),
        .length = edge->length - PivotDistanceFrom(s, edge->from, pointer)
    };
}

void HandTakeStickfigure(RendererData* data, unsigned stickfigure, Vector2 pointer) {
    data->hand.status = HAND_HOLDING_STICKFIGURE;
    data->hand.figure.initialPointer = pointer;
    data->hand.figure.initialStickfigure = data->stickfigure.data[stickfigure].position;
    data->hand.figure.figure = stickfigure;
}

void HandStartSelection(RendererData* data, Vector2 pointer) {
    data->hand.status = HAND_SELECT_EDGES;
    data->hand.selection.start = pointer;
}

void CanvasEventHandler(Clay_ElementId elementId, Clay_PointerData pointerInfo,
                        intptr_t userData) {
    RendererData *data = (void *)userData;
    Clay_ElementData elementData = Clay_GetElementData(elementId);
    /* Rectangle defaultViewport = {} */
    Vector2 canvasPosition = {
        .x = pointerInfo.position.x - elementData.boundingBox.x,
        .y = elementData.boundingBox.height -
            (pointerInfo.position.y - elementData.boundingBox.y),
    };
    Vector2 resolution = {
        elementData.boundingBox.width,
        elementData.boundingBox.height,
    };
    Vector2 worldPos =
        renderer_get_world_position(renderer_context, canvasPosition, resolution);
    bool isShiftPressed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    bool isControlPressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    PivotEdgeIndex edge;
    bool hoverEdge = PivotPointCollisionEdge(data->stickfigure, worldPos, &edge);
    PivotJointIndex joint;
    bool hoverJoint = PivotPointCollisionJoint(data->stickfigure, worldPos, &joint);

    switch (pointerInfo.state) {
    case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
        switch (data->mode) {
        case NORMAL:
            if(!isShiftPressed) {
                if(hoverEdge && isControlPressed) {
                    HandTakeStickfigure(data, edge.figure, worldPos);
                    data->mode = MOVE_STICKFIGURE;
                } else if (hoverJoint && (joint.joint == 0)) {
                    HandTakeStickfigure(data, joint.figure, worldPos);
                    data->mode = MOVE_STICKFIGURE;
                } else if(hoverEdge) {
                    HandTakeStick(data, edge, worldPos);
                    data->mode = MOVE_STICK;
                } else {
                    HandStartSelection(data, worldPos);
                }
            }
            break;
        case CREATE_STICK:
            data->hand.status = HAND_EMPTY;
            data->mode = NORMAL;
            break;
        default:
            break;
        }
        printf("Mode: %d\n", data->mode);
        printf("Mode: %d, hand = %d\n", data->mode, data->hand.status);
        break;
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        switch (data->mode) {
        case NORMAL:
            if(hoverJoint && isShiftPressed) {
                Stickfigure* s = &data->stickfigure.data[joint.figure];
                double angle = PivotAngleFrom(s, joint.joint, worldPos);
                double length = PivotDistanceFrom(s, joint.joint, worldPos);
                StickfigureEdge *part = PivotAddStick( s, data->stickType, joint.joint, angle, length);
                HandTakeStick(data, (PivotEdgeIndex) { joint.figure, array_indexof(s->edges, part)}, worldPos);
                data->mode = CREATE_STICK;
            } else {
                renderer_reset_selection(renderer_context);
                data->hand.status = HAND_EMPTY;
            }
            break;
        case MOVE_STICK:
        case MOVE_STICKFIGURE:
            data->hand.status = HAND_EMPTY;
            data->mode = NORMAL;
        default:
            break;
        }
        printf("Mode: %d, hand = %d\n", data->mode, data->hand.status);
        break;
    default:
        break;
    }
    switch (data->hand.status) {
        Stickfigure* s;
        case HAND_HOLDING_STICK:
            s = &data->stickfigure.data[data->hand.edge.edge.figure];
            StickfigureEdge* e = &s->edges.data[data->hand.edge.edge.edge];
            float angle = PivotAngleFrom(s, e->from, worldPos) + data->hand.edge.pointerOffset.angle;
            float length = PivotDistanceFrom(s, e->from, worldPos) + data->hand.edge.pointerOffset.length;
            PivotMoveEdge(s, data->hand.edge.edge.edge, angle, length);
            break;
        case HAND_HOLDING_STICKFIGURE:
            s = &data->stickfigure.data[data->hand.figure.figure];
            s->position = Vector2Add(data->hand.figure.initialStickfigure, Vector2Subtract(worldPos, data->hand.figure.initialPointer));
            break;
        case HAND_SELECT_EDGES:
            const Vector2 pos = Vector2Min(data->hand.selection.start, worldPos);
            const Vector2 extent = { fabsf(data->hand.selection.start.x - worldPos.x), fabsf(data->hand.selection.start.y - worldPos.y)};
            const Rectangle selection = { pos.x, pos.y, extent.x, extent.y};
            renderer_set_selection(renderer_context, selection);
            PivotEdgesInsideRect(data->stickfigure, selection, &data->selectedEdges);
            foreach(data->selectedEdges, e, PivotEdgeIndex) {
                printf("Selection nº%d: figure = %d, edge = %d\n", index, e->figure, e->edge);
            }
            break;
        default:
            break;
    }
}

void UpdateDrawFrame() {
    Vector2 mouseWheelDelta = GetMouseWheelMoveV();
    float mouseWheelX = mouseWheelDelta.x;
    float mouseWheelY = mouseWheelDelta.y;

    if (IsKeyPressed(KEY_D)) {
        debugEnabled = !debugEnabled;
        Clay_SetDebugModeEnabled(debugEnabled);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        auto hand = &data->rendererData.hand;
        Stickfigure* s = &data->rendererData.stickfigure.data[hand->edge.edge.figure];
        switch (data->rendererData.mode) {
            case CREATE_STICK:
                data->rendererData.hand.status = HAND_EMPTY;
                PivotRemoveEdge(s, hand->edge.edge.edge);
                data->rendererData.mode = NORMAL;
                break;
            case MOVE_STICK:
                data->rendererData.hand.status = HAND_EMPTY;
                PivotMoveEdge(s, hand->edge.edge.edge, hand->edge.initial.angle, hand->edge.initial.length);
                data->rendererData.mode = NORMAL;
                break;
            case MOVE_STICKFIGURE:
                data->rendererData.hand.status = HAND_EMPTY;
                s->position = data->rendererData.hand.figure.initialStickfigure;
                data->rendererData.mode = NORMAL;
                break;
            default:
                break;
        }
    }
    //----------------------------------------------------------------------------------
    // Handle scroll containers
    Clay_Vector2 mousePosition =
        RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
    Clay_SetPointerState(mousePosition, IsMouseButtonDown(0));
    Clay_SetLayoutDimensions(
        (Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()});

    Clay_UpdateScrollContainers(true, (Clay_Vector2){mouseWheelX, mouseWheelY},
                                GetFrameTime());
    // Generate the auto layout for rendering
    data->context.pointer = POINTER_DEFAULT;
    Clay_RenderCommandArray renderCommands = InterfaceLayout(data);
    switch (data->context.pointer) {
        case POINTER_CLICKABLE:
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            break;
        default:
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            break;
    }

    Clay_ElementData canvasData =
        Clay_GetElementData(Clay_GetElementId(CLAY_STRING("canvas")));
    renderer_render(
        renderer_context, data->rendererData.stickfigure,
        data->rendererData.selectedEdges,
        (Vector2){canvasData.boundingBox.width, canvasData.boundingBox.height},
        data->rendererData.pivotRadius);
    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    EndDrawing();
}

bool reinitializeClay = false;

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
    switch (errorData.errorType) {
    case CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED:
        reinitializeClay = true;
        Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
        break;
    case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED:
        reinitializeClay = true;
        Clay_SetMaxMeasureTextCacheWordCount( Clay_GetMaxMeasureTextCacheWordCount() * 2);
        break;
    default:
        break;
    }
}

void InitClay() {
    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
        totalMemorySize, malloc(totalMemorySize));
    Clay_Initialize(
        clayMemory,
        (Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()},
        (Clay_ErrorHandler){HandleClayErrors, 0});
    reinitializeClay = false;
}

int main(void) {
    /* SetTraceLogLevel(LOG_WARNING); */
    InitClay();
    Clay_Raylib_Initialize(1024, 768, "Clay - Raylib Renderer Example",
                            FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE |
                                FLAG_MSAA_4X_HINT);
    glfwMaximizeWindow(GetWindowHandle());
    renderer_context = renderer_init((Rectangle){0, 0, 100, 100});
    if (renderer_context == nullptr)
        return EXIT_FAILURE;

    data = InterfaceInit();

    fonts[FONT_ID_BODY_24] =
        LoadFontEx(RESOURCE_PATH "resources/Roboto-Regular.ttf", 24, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY_24].texture, TEXTURE_FILTER_BILINEAR);
    fonts[FONT_ID_BODY_32] =
        LoadFontEx(RESOURCE_PATH "resources/Roboto-Regular.ttf", 32, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY_32].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);
    SetExitKey(-1); // Deactivate ESCAPE_KEY exit

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        if (reinitializeClay)
            InitClay();
        UpdateDrawFrame();
    }
    InterfaceDeinit(data);
    renderer_deinit(renderer_context);
    Clay_Raylib_Close();
    return 0;
}
