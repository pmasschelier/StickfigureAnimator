#ifndef PIVOT_H
#define PIVOT_H

#include "raylib.h"
#include "src/array.h"
#include <stddef.h>

typedef enum {
    STICKFIGURE_STICK,
    STICKFIGURE_RING,
} StickfigurePartType;

typedef struct {
    double angle;
    double length;
    Color color;
} PivotEdgeData;

typedef struct {
    unsigned from;
    unsigned to;
    StickfigurePartType type;
    double rootAngle;
    double thickness;
    PivotEdgeData data;
} StickfigureEdge;

typedef struct {
    Vector2 pos;
} StickfigureJoint;

DEFINE_ARRAY_TYPE(StickfigureEdge)
DEFINE_ARRAY_TYPE(StickfigureJoint)

constexpr size_t STICKFIGURE_NAME_LENGTH = 64;

typedef struct {
    char name[STICKFIGURE_NAME_LENGTH];
    Vector2 position;
    StickfigureJoint_array_t joints;
    StickfigureEdge_array_t edges;
} Stickfigure;

DEFINE_ARRAY_TYPE(Stickfigure)

typedef struct {
    unsigned figure;
    unsigned joint;
} PivotJointIndex;

typedef struct {
    unsigned figure;
    unsigned edge;
} PivotEdgeIndex;

DEFINE_ARRAY_TYPE(PivotEdgeIndex)

Stickfigure* PivotCreateStickfigure(Stickfigure_array_t* array, const char* name, StickfigurePartType type, Vector2 pivot, PivotEdgeData data);
StickfigureEdge* PivotAddStick(Stickfigure* s, StickfigurePartType type, unsigned int pivot, PivotEdgeData data);
void PivotFreeAll(Stickfigure_array_t* array);
double PivotAngleFrom(Stickfigure* s, unsigned int joint, Vector2 point);
double PivotDistanceFrom(Stickfigure* s, unsigned int joint, Vector2 point);
bool PivotPointCollisionEdge(Stickfigure_array_t stickfigures, Vector2 point, PivotEdgeIndex* edge);
bool PivotPointCollisionJoint(Stickfigure_array_t stickfigures, Vector2 point, PivotJointIndex* joint);
void PivotMoveEdge(Stickfigure* s, unsigned int edge, double angle, double length);
void PivotRemoveEdge(Stickfigure* s, unsigned int edge);
void PivotAppendEdgesInsideRect(Stickfigure_array_t stickfigures, Rectangle rect, PivotEdgeIndex_array_t* edges);
StickfigureEdge* PivotFindRootEdge(Stickfigure* s, unsigned int joint);

#endif // !PIVOT_H
