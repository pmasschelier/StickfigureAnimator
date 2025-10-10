#include "utils.h"
#include <stddef.h>
#include <stdint.h>

extern uint16_t selected_font;

void RenderDropdownMenuItem(
    Clay_String text,
    ItemData data,
    Callback_t* onClick,
    ComponentContext* context
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

    CLAY(
        { .layout = { .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) } } }
    ) {
        if (onClick) {
            if (Clay_Hovered()) {
                background = (Clay_Color){ 0xFF, 0xFF, 0xFF, 0x33 };
                context->clickablHovered = true;
            }
            SetButtonCallbacks(context->arena, (ButtonData){ .onMouseUp = onClick });
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
