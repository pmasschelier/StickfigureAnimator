#include "renderer.h"
#include "src/raylib.h"

RendererError renderer_init(RendererState* state) {
    LoadShader(NULL, "shaders/stick.frag");
}
void renderer_render(RendererState* state);
void renderer_deinit(RendererState* state);
