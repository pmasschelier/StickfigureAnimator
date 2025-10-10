#include "clay/clay.h"
#include "raylib.h"
#include "utils.h"
#include <stdint.h>

void RenderCanvas(Clay_ElementId id, OnHoverFn onHoverFunction, void* hoverFnData, Texture2D* texture) {
    Clay_BoundingBox bb = Clay_GetElementData(id).boundingBox;
    CLAY( {
        .id = id,
        .layout = {
            .sizing = { CLAY_SIZING_GROW(400), CLAY_SIZING_GROW(400) },
        },
    }) {
        CLAY({
            .layout = {
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            },
            .backgroundColor = { 0xFF, 0xFF, 0xFF, 0xFF },
            .image = {
                .imageData = texture,
                .sourceDimensions = { bb.width, bb.height }
            },
        }) {
            Clay_OnHover(onHoverFunction, (intptr_t)hoverFnData);
        }
    }
}
