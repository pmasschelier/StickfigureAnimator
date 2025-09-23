#include "interface.h"
#include "raylib.h"
#include "src/components/utils.h"
#include "src/pivot.h"
#include "src/renderer/renderer.h"
#include <stdlib.h>
#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/renderers/raylib/clay_renderer_raylib.c"

MainLayoutData data;
RendererState state;

bool debugEnabled = false;
bool clickable_hovered = false;

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector)                                 \
  (Clay_Vector2) { .x = vector.x, .y = vector.y }

uint16_t selected_font = 0;
Font fonts[2];

void CanvasEventHandler([[maybe_unused]] Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    RendererData* data = (void*)userData;
    Vector2 mousePos = { pointerInfo.position.x, pointerInfo.position.y };
    switch (pointerInfo.state) {
        case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
            printf("MODE: %d\n", data->mode);
            switch(data->mode) {
                case BEGIN_CREATE_STICK:
                    Stickfigure* sf = CreateStickfigureFromPart(&data->stickfigure, STICKFIGURE_RECT, mousePos);
                    data->currentHandle = &sf->sticks.data[0].handle;
                    data->mode = END_CREATE_STICK;
                    break;
                case END_CREATE_STICK:
                    data->mode = BEGIN_CREATE_STICK;
                    break;
                default:
                    break;
            }
            break;
        default:
            switch (data->mode) {
                case END_CREATE_STICK:
                    if(data->currentHandle)
                        *data->currentHandle = mousePos;
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
    Clay_SetLayoutDimensions((Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()});

    Clay_UpdateScrollContainers(true, (Clay_Vector2){mouseWheelX, mouseWheelY}, GetFrameTime());
    // Generate the auto layout for rendering
    clickable_hovered = false;
    Clay_RenderCommandArray renderCommands = MainLayout_CreateLayout(&data);
    if (clickable_hovered)
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    else
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    Clay_ElementData canvasData =
        Clay_GetElementData(Clay_GetElementId(CLAY_STRING("canvas")));
    renderer_render(
    &state,
    &data.rendererData.stickfigure.data[0],
    (Rectangle){canvasData.boundingBox.x, canvasData.boundingBox.y, canvasData.boundingBox.width, canvasData.boundingBox.height}
    );
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
  if (RENDERER_NOERROR != renderer_init(&state))
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
  Clay_Raylib_Close();
  return 0;
}
