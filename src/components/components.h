#ifndef COMPONENTS_H
#define COMPONENTS_H
#include "utils.h"

void RenderCanvas(Clay_ElementId id);
void RenderDropdownMenuItem(
    Clay_String text,
    ItemData data,
    void (*onClick)(void *),
    void *clickData,
    Arena *arena
);
void RenderMenuBarButton(
    Clay_String title,
    Clay_ElementId buttonId,
    Clay_ElementId menuId,
    bool *menuVisible,
    void RenderMenu(void *),
    void *priv
);

#endif // !COMPONENTS_H
