#ifndef COMPONENTS_UTILS_H
#define COMPONENTS_UTILS_H

#include "arena.h"
#include "callback.h"
#include "clay/clay.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*OnHoverFn)(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
typedef struct {
    unsigned item_index;
    unsigned item_count;
} ItemData;

typedef struct {
    float hue, saturation, value;
} ColorHSV;

typedef enum {
    ROLE_PRIMARY,
    ROLE_PRIMARY_FOREGROUND,
    ROLE_SECONDARY,
    ROLE_SECONDARY_FOREGROUND,
    ROLE_COUNT
} ThemeRole;

typedef struct {
    Clay_Color color[ROLE_COUNT];
} ComponentTheme;

typedef enum {
    POINTER_DEFAULT,
    POINTER_CLICKABLE,
} PointerMode;

typedef struct {
    Arena* arena;
    ComponentTheme theme;
    PointerMode pointer;
    bool arenaOverflowed;
    uint16_t selected_font;
    Clay_ElementId active;
} ComponentContext;

#endif // !COMPONENTS_UTILS_H
