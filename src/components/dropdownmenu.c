#include "src/arena.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>

extern uint16_t selected_font;

static void HandleDropDownMenuItem(
    [[maybe_unused]] Clay_ElementId elementId,
    Clay_PointerData pointerInfo,
    intptr_t userData
) {
    Callback_t *data = (void *)userData;
    switch (pointerInfo.state) {
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        if (data->fn)
            data->fn(data->params);
        break;
    default:
        break;
    }
}

void RenderDropdownMenuItem(
    Clay_String text,
    ItemData data,
    Callback_t onClick,
    Arena *arena
) {
    Clay_CornerRadius cornerRadius = {};
    Clay_Color background = {};

    bool isFirst = data.item_index == 0;
    bool isLast = data.item_index == (data.item_count - 1);
    if (isFirst) {
        cornerRadius.topLeft = 4;
        cornerRadius.topRight = 4;
    }
    if (isLast) {
        cornerRadius.bottomLeft = 4;
        cornerRadius.bottomRight = 4;
    }

    Callback_t* onHoverData = arena_allocate(arena, 1, sizeof(Callback_t));
    *onHoverData = onClick;

    CLAY(
        { .layout = { .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) } } }
    ) {
        if (onClick.fn) {
            if (Clay_Hovered()) {
                background = (Clay_Color){ 0xFF, 0xFF, 0xFF, 0x33 };
                clickable_hovered = true;
            }
            Clay_OnHover(HandleDropDownMenuItem, (intptr_t)onHoverData);
        }
        CLAY( {
            .layout = {
                .padding = CLAY_PADDING_ALL(16),
                .sizing = {  CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0) },
             },
            .backgroundColor = background,
            .cornerRadius = cornerRadius,
        }) {
            CLAY_TEXT(
                text,
                CLAY_TEXT_CONFIG(
                    {
                        .fontId = selected_font,
                        .fontSize = 16,
                        .textColor = { 255, 255, 255, 255 },
                    },
                )
            );
        }
    }
}
