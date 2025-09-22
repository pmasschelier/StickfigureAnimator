#ifndef INTERFACE_H
#define INTERFACE_H

#include "arena.h"
#include "clay/clay.h"
#include <stdint.h>

constexpr int FONT_ID_BODY_32 = 0;
constexpr int FONT_ID_BODY_24 = 1;

typedef enum { NORMAL, CREATE_STICK, CREATE_CIRCLE } EditMode;

constexpr unsigned MENUBAR_BUTTON_COUNT = 5;

typedef struct {
    int32_t selectedDocumentIndex;
    float yOffset;
    EditMode mode;
    Arena arena;
    bool isMenuBarButtonOpen[MENUBAR_BUTTON_COUNT];
} MainLayoutData;

MainLayoutData MainLayout_Initialize();
Clay_RenderCommandArray MainLayout_CreateLayout(MainLayoutData *data);

#endif // !INTERFACE_H
