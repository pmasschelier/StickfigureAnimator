#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "raylib.h"
#include "utils.h"
#include "callback.h"
#include "arena.h"


void RenderCanvas(Clay_ElementId id, OnHoverFn onHoverFunction, void* hoverFnData, Texture2D* texture);
void RenderDropdownMenuItem(
    Clay_String text,
    ItemData data,
    Callback_t* onClick,
    ComponentContext* context
);
void RenderMenuBarButton(
    Clay_String title,
    Clay_ElementId buttonId,
    Clay_ElementId menuId,
    bool *menuVisible,
    void RenderMenu(void *, Callback_t* onMouseReleased),
    void *priv,
    Arena* arena
);

void RenderIconButton(Clay_ElementId id, Texture2D* icon, Callback_t* onMouseUp, ComponentContext* context);
void RenderIconButtonGroup(Clay_ElementId id, Texture2D* icons, Callback_t* onSelected, unsigned count, int selected, ComponentContext* context);

#endif // !COMPONENTS_H
