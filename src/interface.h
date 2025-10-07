#ifndef INTERFACE_H
#define INTERFACE_H

#include "arena.h"
#include "clay/clay.h"
#include "raylib.h"
#include "src/pivot.h"
#include <stdint.h>

constexpr int FONT_ID_BODY_32 = 0;
constexpr int FONT_ID_BODY_24 = 1;

typedef enum {
    NORMAL,
    EDIT,
    CLOSE_EDIT
} EditMode;

constexpr unsigned MENUBAR_BUTTON_COUNT = 5;

typedef struct {
    Stickfigure_array_t stickfigure;
    Vector2* currentHandle;
    EditMode mode;
    StickfigurePartType stickType;
    Vector2 offset;
    float zoom;
} RendererData;

typedef struct {
    int32_t selectedDocumentIndex;
    float yOffset;
    Arena arena;
    bool isMenuBarButtonOpen[MENUBAR_BUTTON_COUNT];
    RendererData rendererData;
} MainLayoutData;

void CanvasEventHandler(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
MainLayoutData MainLayout_Initialize();
Clay_RenderCommandArray MainLayout_CreateLayout(MainLayoutData *data);

#endif // !INTERFACE_H
