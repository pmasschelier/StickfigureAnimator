#ifndef PIVOT_H
#define PIVOT_H

#include "raylib.h"
#include "src/array.h"

typedef enum {
    STICKFIGURE_STICK,
    STICKFIGURE_RING,
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

typedef struct {
    unsigned figure;
    unsigned part;
    unsigned handle;
} PivotIndex;

DEFINE_ARRAY_TYPE(Stickfigure)

StickfigurePart* AddStickfigurePart(Stickfigure* stickfigure, StickfigurePartType type, unsigned part, unsigned handle);
Stickfigure* CreateStickfigureFromPart(Stickfigure_array_t* array, StickfigurePartType type, Vector2 pivot);
float GetNearestJoint(Stickfigure_array_t stickfigures, Vector2 position, PivotIndex* joint);
Vector2* GetHandlePosition(Stickfigure_array_t stickfigures, PivotIndex index);

#endif // !PIVOT_H
