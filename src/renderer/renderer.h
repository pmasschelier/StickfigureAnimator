#ifndef STICKFIGURE_RENDERER_H
#define STICKFIGURE_RENDERER_H

#include "raylib.h"

typedef struct {
    Shader stickfigureShader;
} RendererState;

typedef enum {
    RENDERER_NOERROR,
    RENDERER_FAILED_TO_LOAD_SHADER,
} RendererError;

RendererError renderer_init(RendererState* state);
void renderer_render(RendererState* state);
void renderer_deinit(RendererState* state);

#endif // !STICKFIGURE_RENDERER_H
