#ifndef COMPONENTS_UTILS_H
#define COMPONENTS_UTILS_H

#include "clay/clay.h"

extern bool clickable_hovered;

typedef void (*OnHoverFn)(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
typedef void (*CallbackFn)(void*);

typedef struct {
    CallbackFn fn;
    void* params;
} Callback_t;

typedef struct {
    unsigned item_index;
    unsigned item_count;
} ItemData;

typedef struct {
    void (*onMouseDown)(void);
} ButtonData;

#endif // !COMPONENTS_UTILS_H
