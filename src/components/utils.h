#ifndef COMPONENTS_UTILS_H
#define COMPONENTS_UTILS_H

#include "clay/clay.h"
#include "../arena.h"

extern bool clickable_hovered;

typedef struct {
    unsigned item_index;
    unsigned item_count;
} ItemData;

typedef struct {
    void (*onMouseDown)(void);
} ButtonData;

#endif // !COMPONENTS_UTILS_H
