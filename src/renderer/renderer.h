#ifndef STICKFIGURE_RENDERER_H
#define STICKFIGURE_RENDERER_H

#include "raylib.h"
#include "src/pivot.h"
#include "rlgl.h"

typedef struct RendererContext RendererContext;
extern const size_t SizeofRendererContext;

typedef enum {
  RENDERER_NOERROR,
  RENDERER_FAILED_TO_LOAD_SHADER,
} RendererError;

RendererContext* renderer_init( Rectangle worldViewport);
Texture2D* renderer_get_frame(RendererContext * state);
void renderer_render(RendererContext *state, Stickfigure_array_t stickfigures, PivotEdgeIndex_array_t selected, Vector2 res, float pivotRadius);
void renderer_deinit(RendererContext *state);
Vector2 renderer_get_screen_position(RendererContext* context, Vector2 worldPosition, Rectangle screenViewport);
Vector2 renderer_get_world_position(RendererContext* context, Vector2 canvasPosition, Vector2 res);
void renderer_set_selection(RendererContext* context, Rectangle selection);
void renderer_reset_selection(RendererContext* context);

#endif // !STICKFIGURE_RENDERER_H
