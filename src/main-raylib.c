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

void HandTakeStick(CanvasData* data, Stickfigure* figure, StickfigureEdge* edge,
                   const Vector2 pointer) {
    const PivotEdgeData * edgeData = PivotGetEdgeData(edge);
    data->hand.status = HAND_HOLDING_STICK;
    data->hand.edge.edge = edge;
    data->hand.edge.figure = figure;
    data->hand.edge.initial = (PolarCoords) {
        .angle = edgeData->angle,
        .length = edgeData->length,
    };
    const unsigned jointId = PivotGetPivotIndex(edge);
    data->hand.edge.pointerOffset = (PolarCoords) {
        .angle = edgeData->angle - PivotAngleFrom(figure, jointId, pointer),
        .length = edgeData->length - PivotDistanceFrom(figure, jointId, pointer)
    };
}

void HandTakeStickfigure(CanvasData* data, Stickfigure* figure,
                         const Vector2 pointer) {
    data->hand.status = HAND_HOLDING_STICKFIGURE;
    data->hand.figure.initialPointer = pointer;
    data->hand.figure.initialStickfigure = *PivotStickfigurePosition(figure);
    data->hand.figure.figure = figure;
}

void HandStartSelection(CanvasData* data, const Vector2 pointer) {
    data->hand.status = HAND_SELECT_EDGES;
    data->hand.selection.start = pointer;
}

void CanvasEventHandler(Clay_ElementId elementId, Clay_PointerData pointerInfo,
                        intptr_t userData) {
    CanvasData *canvas = (void*)userData;
    const Clay_ElementData elementData = Clay_GetElementData(elementId);
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

    const bool isShiftPressed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    const bool isControlPressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

    StickfigureEdge* edge;
    Stickfigure* edgeFigure;
    const bool hoverEdge = PivotPointCollisionEdge(canvas->stickfigure, worldPos, &edgeFigure, &edge);

    unsigned joint;
    Stickfigure* jointFigure;
    const bool hoverJoint = PivotPointCollisionJoint(canvas->stickfigure, worldPos, &jointFigure, &joint);

    switch (pointerInfo.state) {
    case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
        switch (canvas->mode) {
        case NORMAL:
            if(!isShiftPressed) {
                if (hoverJoint) {
                    if (joint == 0) {
                        HandTakeStickfigure(canvas, jointFigure, worldPos);
                    } else {
                        StickfigureEdge* rootEdge = PivotFindRootEdge(jointFigure, joint);
                        HandTakeStick(canvas, jointFigure, rootEdge, worldPos);
                        canvas->mode = MOVE_STICK;
                    }
                } else if(hoverEdge) {
                    if (isControlPressed) {
                        HandTakeStick(canvas, edgeFigure, edge, worldPos);
                        canvas->mode = MOVE_STICK;
                    } else {
                        const PivotEdgeData* edgeData = PivotGetEdgeData(edge);
                        // Set color picker's color to the one of the clicked edge
                        const Vector3 color = ColorToHSV(edgeData->color);
                        *canvas->color = (ColorHSV){color.x, color.y, color.z};
                        // Clear selection
                        PivotClearSelection(canvas->stickfigure);
                        // Move the stick figure
                        HandTakeStickfigure(canvas, edgeFigure, worldPos);
                        canvas->mode = MOVE_STICKFIGURE;
                    }
                    PivotSelectEdge(edge, isControlPressed);
                } else {
                    if (!isControlPressed)
                        PivotClearSelection(canvas->stickfigure);
                    HandStartSelection(canvas, worldPos);
                }
            }
            break;
        case CREATE_STICK:
            canvas->hand.status = HAND_EMPTY;
            canvas->mode = NORMAL;
            break;
        default:
            break;
        }
        printf("Mode: %d\n", canvas->mode);
        printf("Mode: %d, hand = %d\n", canvas->mode, canvas->hand.status);
        break;
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        switch (canvas->mode) {
        case NORMAL:
            if (!isShiftPressed) {
                renderer_reset_selection(renderer_context);
                canvas->hand.status = HAND_EMPTY;
            }
            else if(hoverJoint) {
                const PivotEdgeData edgeData = {
                    .angle = PivotAngleFrom(jointFigure, joint, worldPos),
                    .length = PivotDistanceFrom(jointFigure, joint, worldPos),
                    .color = ColorFromHSV(canvas->color->hue, canvas->color->saturation, canvas->color->value),
                    .thickness = canvas->thickness
                };
                StickfigureEdge *part = PivotAddStick(jointFigure, canvas->stickType, joint, edgeData);
                HandTakeStick(canvas, jointFigure, part, worldPos);
                canvas->mode = CREATE_STICK;
            }
            break;
        case MOVE_STICK:
        case MOVE_STICKFIGURE:
            canvas->hand.status = HAND_EMPTY;
            canvas->mode = NORMAL;
        default:
            break;
        }
        printf("Mode: %d, hand = %d\n", canvas->mode, canvas->hand.status);
        break;
    default:
        break;
    }
    switch (canvas->hand.status) {
        Stickfigure* s;
        case HAND_HOLDING_STICK:
            const unsigned pivot = PivotGetPivotIndex(canvas->hand.edge.edge);
            const double angle = PivotAngleFrom(canvas->hand.edge.figure, pivot, worldPos) + canvas->hand.edge.pointerOffset.angle;
            const double length = PivotDistanceFrom(canvas->hand.edge.figure, pivot, worldPos) + canvas->hand.edge.pointerOffset.length;
            PivotMoveEdge(canvas->hand.edge.figure, canvas->hand.edge.edge, angle, length);
            break;
        case HAND_HOLDING_STICKFIGURE:
            Vector2* position = PivotStickfigurePosition(canvas->hand.figure.figure);
            *position = Vector2Add(canvas->hand.figure.initialStickfigure, Vector2Subtract(worldPos, canvas->hand.figure.initialPointer));
            break;
        case HAND_SELECT_EDGES:
            const Vector2 pos = Vector2Min(canvas->hand.selection.start, worldPos);
            const Vector2 extent = { fabsf(canvas->hand.selection.start.x - worldPos.x), fabsf(canvas->hand.selection.start.y - worldPos.y)};
            const Rectangle selection = { pos.x, pos.y, extent.x, extent.y};
            renderer_set_selection(renderer_context, selection);
            PivotAppendEdgesInsideRect(canvas->stickfigure, selection, !isControlPressed);
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
        switch (data->rendererData.mode) {
            case CREATE_STICK:
                data->rendererData.hand.status = HAND_EMPTY;
                // TODO: Remove edge on escape
                // PivotRemoveEdge(hand->edge.figure, hand->edge.edge);
                data->rendererData.mode = NORMAL;
                break;
            case MOVE_STICK:
                data->rendererData.hand.status = HAND_EMPTY;
                PivotMoveEdge(hand->edge.figure, hand->edge.edge, hand->edge.initial.angle, hand->edge.initial.length);
                data->rendererData.mode = NORMAL;
                break;
            case MOVE_STICKFIGURE:
                data->rendererData.hand.status = HAND_EMPTY;
                Vector2* position = PivotStickfigurePosition(data->rendererData.hand.figure.figure);
                *position = hand->figure.initialStickfigure;
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
