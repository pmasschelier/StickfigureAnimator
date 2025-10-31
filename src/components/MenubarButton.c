#include "components/callback.h"
#include "components/utils.h"
#include <stdint.h>

static void CloseMenu(bool* open) {
    *open = false;
}

void RenderMenuBarButton(
    Clay_String title,
    Clay_ElementId buttonId,
    Clay_ElementId menuId,
    bool *menuVisible,
    void RenderMenu(void *, Callback_t* onMouseReleased),
    void *priv,
    ComponentContext* context
) {
    CLAY(
        { .id = buttonId,
          .layout = { .padding = { 16, 16, 8, 8 } },
          .backgroundColor = { 140, 140, 140, 255 },
          .cornerRadius = CLAY_CORNER_RADIUS(5) }
    ) {
        CLAY_TEXT(
            title,
            CLAY_TEXT_CONFIG({
                .fontId = context->selected_font,
                .fontSize = 16,
                .textColor = { 255, 255, 255, 255 } }
            )
        );
        if(Clay_Hovered()) {
            context->pointer = POINTER_CLICKABLE;
        }
        if (Clay_GetPointerState().state
            == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
            if (Clay_PointerOver(buttonId))
                *menuVisible = !(*menuVisible);
            else if(!Clay_PointerOver(menuId))
                *menuVisible = false;
        }

        if (*menuVisible) { // Below has been changed slightly to fix
                            // the small bug where the menu would
                            // dismiss when mousing over the top gap
            CLAY({
                .id = menuId,
                .floating = {
                    .attachTo = CLAY_ATTACH_TO_PARENT,
                    .attachPoints = {
                        .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM
                    },
                },
                .layout = {
                    .padding = {0, 0, 8, 8 }
                },
            }) {
                CLAY({
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = {
                                .width = CLAY_SIZING_FIXED(200)
                        },
                        .padding = CLAY_PADDING_ALL(4),
                    },
                    .backgroundColor = {40, 40, 40, 255 },
                    .cornerRadius = {0.f, 8.f, 8.f, 8.f},
                }) {
                    RenderMenu(priv, CallbackCreate(&context->allocator, (CallbackFn)CloseMenu, menuVisible));
                }
            }
        }
    }
}
