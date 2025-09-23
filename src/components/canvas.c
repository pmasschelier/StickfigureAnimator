#include "clay/clay.h"
#include "src/components/utils.h"
#include <stdint.h>

void RenderCanvas(Clay_ElementId id, OnHoverFn onHoverFunction, void* hoverFnData) {
    CLAY( {
        .id = id,
        .layout = {
            .sizing = { CLAY_SIZING_GROW(400), CLAY_SIZING_GROW(400) },
        },
        .backgroundColor = { 0xFF, 0xFF, 0xFF, 0xFF },
    }) {
         Clay_OnHover(onHoverFunction, (intptr_t)hoverFnData);
    }
}
