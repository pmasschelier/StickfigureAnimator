#include "components/utils.h"
#include "interface.h"
#include "raylib.h"
#include <stdio.h>

void RenderIconButton(Clay_ElementId id, Arena* arena, Texture2D* icon, Callback_t* onMouseUp) {
    const float size = 32.f;
    Clay_BorderElementConfig border = { .color = { 0.f, 0.f, 0.f, 255.f }};
    if(Clay_PointerOver(id)) {
        border.width = (Clay_BorderWidth) { 1.f, 1.f, 1.f, 1.f, 0.f };
    }
    CLAY({
        .id = id,
        .border = border,
        .layout = {
         .padding = CLAY_PADDING_ALL(4.f),
        },
         .cornerRadius = CLAY_CORNER_RADIUS(3.f)
    }) {
        if(onMouseUp)
            SetButtonCallbacks(arena, (ButtonData) { .onMouseUp = onMouseUp });
        CLAY({
            .layout = {
                .sizing = CLAY_SIZING_FIXED(size),
            },
            .image = { .imageData = icon, .sourceDimensions = { size, size }},
        }) {}
    }
}
