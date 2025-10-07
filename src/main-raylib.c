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

void CanvasEventHandler(Clay_ElementId elementId, Clay_PointerData pointerInfo,
                        intptr_t userData) {
  RendererData *data = (void *)userData;
  Clay_ElementData elementData = Clay_GetElementData(elementId);
  /* Rectangle defaultViewport = {} */
  Vector2 canvasPosition = {
        .x = pointerInfo.position.x - elementData.boundingBox.x,
        .y = elementData.boundingBox.height - (pointerInfo.position.y - elementData.boundingBox.y)
  };
  Vector2 resolution = { elementData.boundingBox.width, elementData.boundingBox.height };
  Vector2 worldPos = renderer_get_world_position(renderer_context, canvasPosition, resolution);
  PivotIndex nearestjoint;
  float dist = GetNearestJoint(data->stickfigure, worldPos, &nearestjoint);
  switch (pointerInfo.state) {
  case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
    printf("MODE: %d\n", data->mode);
    for (unsigned i = 0; i < data->stickfigure.length; i++) {
      printf("Stickfigure nº%d\n", i);
      for (unsigned j = 0; j < data->stickfigure.data[i].sticks.length; j++) {
        StickfigurePart *part = &data->stickfigure.data[i].sticks.data[j];
        printf("(%f, %f) - (%f, %f)\n", part->pivot.x, part->pivot.y,
               part->handle.x, part->handle.y);
      }
    }
    switch (data->mode) {
    case EDIT:
      if (data->stickfigure.length > 0) {
        printf("Clicked near (%d/%d, %d/%d, %d/%d): d = %f\n",
               nearestjoint.figure, data->stickfigure.length, nearestjoint.part,
               data->stickfigure.data[nearestjoint.figure].sticks.length,
               nearestjoint.handle,
               data->stickfigure.data[nearestjoint.figure]
                       .sticks.data[nearestjoint.part]
                       .handle_count +
                   2,
               dist);
      }
      if (data->stickfigure.length > 0 && dist < 3.f) {
        StickfigurePart *part = AddStickfigurePart(
    &data->stickfigure.data[nearestjoint.figure],
            data->stickType, nearestjoint.part, nearestjoint.handle);
        data->currentHandle = &part->handle;
      } else {
        Stickfigure *sf = CreateStickfigureFromPart(&data->stickfigure,
                                                    data->stickType, worldPos);
        printf("(%f, %f)\n", worldPos.x, worldPos.y);
        data->currentHandle = &sf->sticks.data[0].handle;
      }
      data->mode = CLOSE_EDIT;
      break;
    case CLOSE_EDIT:
      data->mode = EDIT;
      break;
    default:
      break;
    }
    break;
  default:
    switch (data->mode) {
    case CLOSE_EDIT:
      if (data->currentHandle)
        *data->currentHandle = worldPos;
      break;
    default:
      break;
    }
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
      (Vector2){canvasData.boundingBox.width, canvasData.boundingBox.height});
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
