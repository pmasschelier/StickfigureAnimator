#ifndef PIVOT_H
#define PIVOT_H

#include "raylib.h"
#include "src/array.h"

typedef enum {
    STICKFIGURE_RECT,
    STICKFIGURE_CIRCLE,
} StickfigurePartType;

typedef struct {
    unsigned sticks[2];
    unsigned handles[2];
} StickfigureJoint;

typedef struct {
    StickfigurePartType type;
    Vector2 pivot;
    Vector2 handle;
    unsigned int handle_count;
    Vector2 *handles;
} StickfigurePart;

DEFINE_ARRAY_TYPE(StickfigurePart)
DEFINE_ARRAY_TYPE(StickfigureJoint)

typedef struct {
    StickfigurePart_array_t sticks;
    StickfigureJoint_array_t joints;
} Stickfigure;

DEFINE_ARRAY_TYPE(Stickfigure)

Stickfigure* CreateStickfigureFromPart(Stickfigure_array_t* array, StickfigurePartType type, Vector2 pivot);

#endif // !PIVOT_H
