#include "clay/clay.h"
#include "raylib.h"

#include <assert.h>
#include <stdio.h>

void RenderShaderImage(Clay_ElementId id, Clay_Sizing sizing, RenderTexture* texture, Shader shader) {
    assert(texture != NULL);
    const Clay_ElementData data = Clay_GetElementData(id);
    const Vector2 res = {data.boundingBox.width, data.boundingBox.height};

    const int resLoc = GetShaderLocation(shader, "res");
    SetShaderValue(shader, resLoc, &res, SHADER_UNIFORM_VEC2);

    bool create = false;
    if (texture->id == 0) {
        create = true;
    } else if (texture->texture.width != res.x || texture->texture.height != res.y) {
        UnloadRenderTexture(*texture);
        create = true;
    }
    if (res.x > 0 && res.y > 0) {
        if (create)
            *texture = LoadRenderTexture(res.x, res.y);
        BeginTextureMode(*texture);
        BeginShaderMode(shader);
        DrawRectangle(0, 0, res.x, res.y, BLACK);
        EndShaderMode();
        EndTextureMode();
    }

    CLAY({
        .id = id,
        .layout = {.sizing = sizing},
    }) {
        CLAY({
             .layout = { .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}},
             .image = { .imageData = &texture->texture, .sourceDimensions = {res.x, res.y}}
        }) {}
    }
}