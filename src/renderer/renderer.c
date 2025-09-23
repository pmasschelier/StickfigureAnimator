#include "renderer.h"
#include "src/interface.h"
#include "src/pivot.h"
#include "src/raylib.h"

RendererError renderer_init(RendererState *state) {
    state->stickfigureShader = LoadShader(nullptr, RESOURCE_PATH "resources/shaders/stick.frag");
    if (state->stickfigureShader.id == 0) {
        return RENDERER_FAILED_TO_LOAD_SHADER;
    }

    return RENDERER_NOERROR;
}
void renderer_render(RendererState *state, Stickfigure* stickfigure, Rectangle viewport) {
    if(stickfigure == nullptr)
        return;

    unsigned pos1 = GetShaderLocation(state->stickfigureShader, "start");
    SetShaderValue(state->stickfigureShader, pos1, &stickfigure->sticks.data[0].pivot, SHADER_UNIFORM_VEC2);

    unsigned pos2 = GetShaderLocation(state->stickfigureShader, "end");
    SetShaderValue(state->stickfigureShader, pos2, &stickfigure->sticks.data[0].handle, SHADER_UNIFORM_VEC2);

    float thickness = 10.f;
    unsigned t = GetShaderLocation(state->stickfigureShader, "thickness");
    SetShaderValue(state->stickfigureShader, t, &thickness, SHADER_UNIFORM_FLOAT);

    Vector4 color = {1.0, 0.0, 0.0, 1.0};
    unsigned c = GetShaderLocation(state->stickfigureShader, "color");
    SetShaderValue(state->stickfigureShader, c, &color, SHADER_UNIFORM_VEC4);

    BeginShaderMode(state->stickfigureShader);
    DrawRectangleRec(viewport, RED);
    EndShaderMode();
}

void renderer_deinit(RendererState *state);
