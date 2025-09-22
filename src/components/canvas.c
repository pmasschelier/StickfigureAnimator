#include "clay/clay.h"

void RenderCanvas(Clay_ElementId id) {
    CLAY( {
        .id = id,
        .layout = {
            .sizing = { CLAY_SIZING_GROW(400), CLAY_SIZING_GROW(400) },
        },
        .backgroundColor = { 0xFF, 0xFF, 0xFF, 0xFF },
    }) {}
}
