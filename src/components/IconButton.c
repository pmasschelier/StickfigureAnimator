#include "components/callback.h"
#include "utils.h"
#include "raylib.h"

void RenderIconButtonGroup(Clay_ElementId id, Texture2D* icons, Callback_t* onSelected, unsigned count, int selected, ComponentContext* context) {
    const float size = 32.f;
    Clay_BorderElementConfig border = { .color = { 0.f, 0.f, 0.f, 255.f }};
    if(Clay_PointerOver(id)) {
        border.width = (Clay_BorderWidth) { 1.f, 1.f, 1.f, 1.f, 1.f };
        context->pointer = POINTER_CLICKABLE;
    }
    CLAY({
        .id = id,
        .border = border,
         .cornerRadius = CLAY_CORNER_RADIUS(3.f)
    }) {
        for (unsigned int i = 0; i < count; i++) {
            Clay_Color background = {};
            if(selected == (int)i) {
                background = context->theme.color[ROLE_PRIMARY];
            }
            CLAY({
                .layout = {
                    .padding = CLAY_PADDING_ALL(4.f),
                },
                .backgroundColor = background
             }) {
                Callback_t* cb = CALLBACK_INDEX(onSelected, i);
                if(onSelected)
                    SetButtonCallbacks(&context->allocator, (ButtonData) { .onMouseUp = cb });
                CLAY({
                    .layout = {
                        .sizing = { CLAY_SIZING_FIXED(size), CLAY_SIZING_FIXED(size) },
                    },
                    .image = { .imageData = &icons[i], .sourceDimensions = { size, size }},
                }) {}

            } 
        }
    }
}

void RenderIconButton(Clay_ElementId id, Texture2D* icon, Callback_t* onMouseUp, ComponentContext* context) {
    const float size = 32.f;
    Clay_BorderElementConfig border = { .color = { 0.f, 0.f, 0.f, 255.f }};
    if(Clay_PointerOver(id)) {
        border.width = (Clay_BorderWidth) { 1.f, 1.f, 1.f, 1.f, 0.f };
        context->pointer = POINTER_CLICKABLE;
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
            SetButtonCallbacks(&context->allocator, (ButtonData) { .onMouseUp = onMouseUp });
        CLAY({
            .layout = {
                .sizing = { CLAY_SIZING_FIXED(size), CLAY_SIZING_FIXED(size) },
            },
            .image = { .imageData = icon, .sourceDimensions = { size, size }},
        }) {}
    }
}
