#include "SDL3/SDL_gpu.h"
#include "clay/clay.h"
#include <assert.h>
#include <stdlib.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>
#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/renderers/SDL3/clay_renderer_SDL3.c"

#include <stdio.h>

#include "interface.h"
#include "renderer/renderer.h"

typedef struct {
    SDL_Window *window;
    Clay_SDL3RendererData rendererData;
    Clay_Context *context;
} WindowState;

typedef struct {
    bool pointerDown;
    Clay_Vector2 mouse;
} Input;

constexpr unsigned WINDOW_COUNT = 2;

typedef struct app_state {
    WindowState windows[WINDOW_COUNT];
    MainLayoutData demoData;
    Input input;
    SDL_GPUDevice *device;
    RendererState renderer;
} AppState;

constexpr unsigned FONT_COUNT = 1;
constexpr Uint32 FONT_ID = 0;
uint16_t selected_font = 0;
TTF_Font **fonts;

extern bool clickable_hovered;
struct {
    SDL_Cursor *cursor;
    SDL_Cursor *pointer;
} cursors;

// This function is new since the video was published
void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

static inline Clay_Dimensions SDL_MeasureText(
    Clay_StringSlice text,
    Clay_TextElementConfig *config,
    void *userData
) {
    TTF_Font **fonts = userData;
    TTF_Font *font = fonts[config->fontId];
    int width, height;

    TTF_SetFontSize(font, config->fontSize);
    if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s", SDL_GetError()
        );
    }

    return (Clay_Dimensions){ (float)width, (float)height };
}

bool ConfigureWindowState(
    WindowState *state,
    SDL_Window *window,
    SDL_Renderer *renderer,
    SDL_GPUDevice *device
) {
    state->window = window;
    state->rendererData.renderer = renderer;

    SDL_ClaimWindowForGPUDevice(device, window);

    TTF_TextEngine *textEngine = TTF_CreateRendererTextEngine(renderer);
    if (!textEngine) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to create text engine from renderer: %s",
            SDL_GetError()
        );
        return false;
    }

    state->rendererData.textEngine = textEngine;
    state->rendererData.fonts = fonts;

    /* Initialize Clay */
    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = (Clay_Arena){
        .memory = SDL_malloc(totalMemorySize),
        .capacity = totalMemorySize,
    };

    int width, height;
    SDL_GetWindowSize(state->window, &width, &height);
    state->context = Clay_Initialize(
        clayMemory,
        (Clay_Dimensions){ (float)width, (float)height },
        (Clay_ErrorHandler){ HandleClayErrors }
    );
    return true;
}

SDL_AppResult SDL_AppInit(
    void **appstate,
    [[maybe_unused]] int argc,
    [[maybe_unused]] char *argv[]
) {
    if (!TTF_Init()) {
        return SDL_APP_FAILURE;
    }

    AppState *state = SDL_calloc(1, sizeof(AppState));
    if (!state) {
        return SDL_APP_FAILURE;
    }
    *appstate = state;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!SDL_CreateWindowAndRenderer(
            "Clay Demo", 640, 480, 0, &window, &renderer
        )) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to create window and renderer: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowResizable(window, true);
    cursors.pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    cursors.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);

    state->device =
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);

    fonts = SDL_calloc(1, sizeof(TTF_Font *));
    if (!fonts) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR,
            "Failed to allocate memory for the font array: %s",
            SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }

    TTF_Font *font = TTF_OpenFont("resources/Roboto-Regular.ttf", 16);
    if (!font) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s", SDL_GetError()
        );
        return SDL_APP_FAILURE;
    }

    fonts[FONT_ID] = font;
    ConfigureWindowState(&state->windows[0], window, renderer, state->device);

    Clay_SetMeasureTextFunction(SDL_MeasureText, fonts);
    state->demoData = MainLayout_Initialize();

    RendererError renderer_err = renderer_init(window, state->device, &state->renderer);
    if(renderer_err != RENDERER_NOERROR)
        return SDL_APP_FAILURE;

    *appstate = state;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *state = appstate;
    SDL_AppResult ret_val = SDL_APP_CONTINUE;

    switch (event->type) {
    case SDL_EVENT_QUIT:
        ret_val = SDL_APP_SUCCESS;
        break;
    /* case SDL_EVENT_KEY_UP: */
    /*     if (event->key.scancode == SDL_SCANCODE_SPACE) { */
    /*         show_demo = !show_demo; */
    /*     } */
    /*     break; */
    case SDL_EVENT_WINDOW_RESIZED:
        Clay_SetLayoutDimensions((Clay_Dimensions){
            (float)event->window.data1, (float)event->window.data2 });
        break;
    case SDL_EVENT_MOUSE_MOTION:
        state->input.mouse.x = event->motion.x;
        state->input.mouse.y = event->motion.y;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event->button.button == SDL_BUTTON_LEFT)
            state->input.pointerDown = true;
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event->button.button == SDL_BUTTON_LEFT)
            state->input.pointerDown = false;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        Clay_UpdateScrollContainers(
            true, (Clay_Vector2){ event->wheel.x, event->wheel.y }, 0.01f
        );
        break;
    default:
        break;
    };

    return ret_val;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *state = appstate;

    Clay_SetPointerState(state->input.mouse, state->input.pointerDown);

    clickable_hovered = false;
    Clay_RenderCommandArray render_commands =
        MainLayout_CreateLayout(&state->demoData);

    if (clickable_hovered)
        SDL_SetCursor(cursors.pointer);
    else
        SDL_SetCursor(cursors.cursor);

    SDL_SetRenderDrawColor(
        state->windows[0].rendererData.renderer, 0, 0, 0, 255
    );
    SDL_RenderClear(state->windows[0].rendererData.renderer);

    SDL_Clay_RenderClayCommands(
        &state->windows[0].rendererData, &render_commands
    );

    Clay_ElementData canvas = Clay_GetElementData(Clay_GetElementId(CLAY_STRING("canvas")));
    /* SDL_GPUViewport viewport = { canvas.boundingBox.x, canvas.boundingBox.y, canvas.boundingBox.width, canvas.boundingBox.height, 0.f, 1.f}; */
    SDL_GPUViewport viewport = { 0.f, 0.f, 50.f, 50.f, 0.f, 1.f};

    SDL_RenderPresent(state->windows[0].rendererData.renderer);

    renderer_render(&state->renderer, &viewport);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    (void)result;

    if (result != SDL_APP_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Application failed to run");
    }

    AppState *state = appstate;

    renderer_deinit(&state->renderer);

    if (state) {
        if (fonts) {
            for (size_t i = 0; i < FONT_COUNT; i++) {
                TTF_CloseFont(fonts[i]);
            }

            SDL_free(fonts);
        }
        SDL_DestroyGPUDevice(state->device);
        for (int i = 0; i < WINDOW_COUNT; i++) {
            if (state->windows[i].rendererData.renderer)
                SDL_DestroyRenderer(state->windows[i].rendererData.renderer);

            if (state->windows[i].window)
                SDL_DestroyWindow(state->windows[i].window);

            if (state->windows[i].rendererData.textEngine)
                TTF_DestroyRendererTextEngine(
                    state->windows[i].rendererData.textEngine
                );
        }

        SDL_free(state);
    }
    TTF_Quit();
}
