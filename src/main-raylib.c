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

MainLayoutData data;
RendererContext *renderer_context;
GLFWwindow *window;

bool debugEnabled = false;
bool clickable_hovered = false;

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector)                                 \
  (Clay_Vector2) { .x = vector.x, .y = vector.y }

#define CLAY_BOUNDINGBOX_TO_RAYLIB_RECTANGLE(boundingbox)                      \
  (Rectangle) {                                                                \
    .x = boundingbox.x, .y = boundingbox.y, .width = boundingbox.width,        \
    .height = boundingbox.height                                               \
  }

uint16_t selected_font = 0;
Font fonts[2];

void HandTake(RendererData* data, PivotEdgeIndex index, Vector2 pointer) {
    Stickfigure* s = &data->stickfigure.data[index.figure];
    StickfigureEdge* edge = &s->edges.data[index.edge];
    data->hand.holding = true;
    data->hand.edge = index;
    data->hand.pointerOffset = Vector2Subtract(s->joints.data[edge->to].pos, pointer);
    data->hand.angle = edge->angle;
    data->hand.length = edge->length;
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
  /* PivotIndex nearestjoint; */
  /* float dist = PivotGetNearestJoint(data->stickfigure, worldPos, &nearestjoint); */
  /* bool isOnJoint = data->stickfigure.length > 0 && dist < data->pivotRadius; */
  bool isShiftPressed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  PivotEdgeIndex edge;
  bool hoverEdge = PivotPointCollisionEdge(data->stickfigure, worldPos, &edge);
  PivotJointIndex joint;
  bool hoverJoint = PivotPointCollisionJoint(data->stickfigure, worldPos, &joint);

  switch (pointerInfo.state) {
  case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
    switch (data->mode) {
    case NORMAL:
      if (!isShiftPressed && hoverEdge) {
        printf("Take: figure = %d edge = %d\n", edge.figure, edge.edge);
        HandTake(data, edge, worldPos);
      }
      break;
    default:
      break;
    }
    break;
  case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
    switch (data->mode) {
    case NORMAL:
      data->hand.holding = false;
        if (isShiftPressed && hoverJoint) {
          Stickfigure* s = &data->stickfigure.data[joint.figure];
          Vector2 from = data->stickfigure.data[joint.figure].joints.data[joint.joint].pos;
          double angle = PivotAngleFrom(s, joint.joint, worldPos);
          double length = Vector2Distance(from, worldPos);
          StickfigureEdge *part = PivotAddStick( s, data->stickType, joint.joint, angle, length);
          HandTake(data, (PivotEdgeIndex) { joint.figure, array_indexof(s->edges, part)}, worldPos);
        }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  if (data->hand.holding) {
    Stickfigure* s = &data->stickfigure.data[data->hand.edge.figure];
    StickfigureEdge* e = &s->edges.data[data->hand.edge.edge];
    Vector2 from = s->joints.data[e->from].pos;
    Vector2 virtual = Vector2Add(worldPos, data->hand.pointerOffset);
    float angle = PivotAngleFrom(s, e->from, virtual);
    float length = Vector2Distance(from, virtual);
    PivotMoveEdge(s, data->hand.edge.edge, angle, length);
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
  clickable_hovered = false;
  Clay_RenderCommandArray renderCommands = MainLayout_CreateLayout(&data);
  if (clickable_hovered)
    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
  else
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);

  Clay_ElementData canvasData =
      Clay_GetElementData(Clay_GetElementId(CLAY_STRING("canvas")));
  renderer_render(
      renderer_context, data.rendererData.stickfigure,
      (Vector2){canvasData.boundingBox.width, canvasData.boundingBox.height},
      data.rendererData.pivotRadius);
  BeginDrawing();
  ClearBackground(BLACK);
  /* rlViewport(0, 0, GetScreenWidth(), GetScreenHeight()); */
  Clay_Raylib_Render(renderCommands, fonts);
  EndDrawing();
}

bool reinitializeClay = false;

void HandleClayErrors(Clay_ErrorData errorData) {
  printf("%s", errorData.errorText.chars);
  if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
    reinitializeClay = true;
    Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
  } else if (errorData.errorType ==
             CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
    reinitializeClay = true;
    Clay_SetMaxMeasureTextCacheWordCount(
        Clay_GetMaxMeasureTextCacheWordCount() * 2);
  }
}

int main(void) {
  /* SetTraceLogLevel(LOG_WARNING); */
  uint64_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
      totalMemorySize, malloc(totalMemorySize));
  Clay_Initialize(
      clayMemory,
      (Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()},
      (Clay_ErrorHandler){HandleClayErrors, 0});
  Clay_Raylib_Initialize(1024, 768, "Clay - Raylib Renderer Example",
                         FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE |
                             FLAG_MSAA_4X_HINT);
  renderer_context = malloc(SizeofRendererContext);
  renderer_context = renderer_init((Rectangle){0, 0, 100, 100});
  if (renderer_context == nullptr)
    return EXIT_FAILURE;

  data = MainLayout_Initialize();

  fonts[FONT_ID_BODY_24] =
      LoadFontEx(RESOURCE_PATH "resources/Roboto-Regular.ttf", 24, 0, 400);
  SetTextureFilter(fonts[FONT_ID_BODY_24].texture, TEXTURE_FILTER_BILINEAR);
  fonts[FONT_ID_BODY_32] =
      LoadFontEx(RESOURCE_PATH "resources/Roboto-Regular.ttf", 32, 0, 400);
  SetTextureFilter(fonts[FONT_ID_BODY_32].texture, TEXTURE_FILTER_BILINEAR);
  Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    if (reinitializeClay) {
      Clay_SetMaxElementCount(8192);
      totalMemorySize = Clay_MinMemorySize();
      clayMemory = Clay_CreateArenaWithCapacityAndMemory(
          totalMemorySize, malloc(totalMemorySize));
      Clay_Initialize(
          clayMemory,
          (Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()},
          (Clay_ErrorHandler){HandleClayErrors, 0});
      reinitializeClay = false;
    }
    UpdateDrawFrame();
  }
  renderer_deinit(renderer_context);
  Clay_Raylib_Close();
  return 0;
}
