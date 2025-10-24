#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "raylib.h"
#include "utils.h"

void RenderShaderImage(Clay_ElementId id, Clay_Sizing sizing, RenderTexture* texture, Shader shader);
void RenderColorPicker(Clay_String id, ComponentContext* context);
void RenderCanvas(Clay_ElementId id, OnHoverFn onHoverFunction, void* hoverFnData, Texture2D* texture, ComponentContext* context);
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
    ComponentContext* context
);

void RenderIconButton(Clay_ElementId id, Texture2D* icon, Callback_t* onMouseUp, ComponentContext* context);
void RenderIconButtonGroup(Clay_ElementId id, Texture2D* icons, Callback_t* onSelected, unsigned count, int selected, ComponentContext* context);

#endif // !COMPONENTS_H
