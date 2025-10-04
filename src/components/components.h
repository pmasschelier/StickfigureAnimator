#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "raylib.h"
#include "utils.h"
#include "arena.h"


void RenderCanvas(Clay_ElementId id, OnHoverFn onHoverFunction, void* hoverFnData, Texture2D* texture);
void RenderDropdownMenuItem(
    Clay_String text,
    ItemData data,
    Callback_t onClick,
    Arena *arena
);
void RenderMenuBarButton(
    Clay_String title,
    Clay_ElementId buttonId,
    Clay_ElementId menuId,
    bool *menuVisible,
    void RenderMenu(void *, Callback_t onMouseReleased),
    void *priv
);

#endif // !COMPONENTS_H
