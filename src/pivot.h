#ifndef PIVOT_H
#define PIVOT_H

#include "raylib.h"
#include "src/array.h"

typedef enum {
    STICKFIGURE_STICK,
    STICKFIGURE_RING,
} StickfigurePartType;

// typedef struct {
//     unsigned sticks[2];
//     unsigned handles[2];
// } StickfigureJoint;

// typedef struct {
//     StickfigurePartType type;
//     Vector2 pivot;
//     Vector2 handle;
//     unsigned int handle_count;
//     Vector2 *handles;
// } StickfigurePart;
//
// DEFINE_ARRAY_TYPE(StickfigurePart)
// DEFINE_ARRAY_TYPE(StickfigureJoint)

// typedef struct {
//     StickfigurePart_array_t sticks;
//     StickfigureJoint_array_t joints;
// } Stickfigure;
//

typedef struct {
    unsigned from;
    unsigned to;
    StickfigurePartType type;
    double angle;
} StickfigureEdge;

typedef struct {
    Vector2 pos;
} StickfigureJoint;

DEFINE_ARRAY_TYPE(StickfigureEdge)
DEFINE_ARRAY_TYPE(StickfigureJoint)

typedef struct {
    StickfigureJoint_array_t joints;
    StickfigureEdge_array_t edges;
} Stickfigure;

typedef struct {
    unsigned figure;
    unsigned joint;
} PivotIndex;

DEFINE_ARRAY_TYPE(Stickfigure)

Stickfigure* PivotCreateStickfigure(Stickfigure_array_t* array, StickfigurePartType type, Vector2 pivot, Vector2 handle);
StickfigureEdge* PivotAddStick(Stickfigure* s, StickfigurePartType type, unsigned int pivot);
void PivotFreeAll(Stickfigure_array_t* array);
float PivotGetNearestJoint(Stickfigure_array_t stickfigures, Vector2 position, PivotIndex* joint);

#endif // !PIVOT_H
