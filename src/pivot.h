#ifndef PIVOT_H
#define PIVOT_H

#include "raylib.h"
#include "src/array.h"
#include <stddef.h>

typedef enum : unsigned char {
    STICKFIGURE_STICK,
    STICKFIGURE_RING,
} StickfigurePartType;

typedef struct {
    double angle;
    double length;
    double thickness;
    Color color;
    bool selected;
} PivotEdgeData;

typedef struct StickfigureJoint StickfigureJoint;
DEFINE_INTERFACE_ARRAY_TYPE(StickfigureJoint)
typedef struct StickfigureEdge StickfigureEdge;
DEFINE_INTERFACE_ARRAY_TYPE(StickfigureEdge)

constexpr size_t STICKFIGURE_NAME_LENGTH = 64;
typedef struct Stickfigure Stickfigure;
extern const size_t SizeofStickfigure;
#define get_figure(array, figure) (Stickfigure*)((char*)(array).data + (figure) * SizeofStickfigure)
DEFINE_INTERFACE_ARRAY_TYPE(Stickfigure)

typedef struct {
    unsigned from, to;
} PivotEdgeIndex;

Stickfigure* PivotCreateStickfigure(Stickfigure_array_t* array, const char* name, StickfigurePartType type, Vector2 pivot, PivotEdgeData data);
StickfigureEdge* PivotAddStick(Stickfigure* s, StickfigurePartType type, unsigned pivot, PivotEdgeData data);
StickfigureEdge* PivotFindEdge(Stickfigure* array, PivotEdgeIndex edge);
const PivotEdgeData* PivotGetEdgeData(const StickfigureEdge* edge);
unsigned PivotGetPivotIndex(const StickfigureEdge* edge);
Vector2* PivotStickfigurePosition(Stickfigure* s);
char* PivotStickfigureName(Stickfigure* s);
void PivotFreeAll(Stickfigure_array_t* array);
double PivotAngleFrom(const Stickfigure* s, unsigned pivot, Vector2 point);
double PivotDistanceFrom(const Stickfigure* s, unsigned pivot, Vector2 point);
bool PivotPointCollisionEdge(Stickfigure_array_t stickfigures, Vector2 point, Stickfigure** figure, StickfigureEdge** edge);
bool PivotPointCollisionJoint(Stickfigure_array_t stickfigures, Vector2 point, Stickfigure** figure, unsigned* joint);
void PivotMoveEdge(Stickfigure* s, StickfigureEdge* edge, double angle, double length);
void PivotSelectEdge(StickfigureEdge* edge, bool toggle);
void PivotRemoveEdge(Stickfigure* s, unsigned int edge);
void PivotAppendEdgesInsideRect(Stickfigure_array_t stickfigures, Rectangle rect, bool unselectOthers);
StickfigureEdge* PivotFindRootEdge(Stickfigure* stickfigure, unsigned joint);
void PivotClearSelection(Stickfigure_array_t stickfigures);
void PivotSetColorSelection(Stickfigure_array_t* stickfigures, Color color);

#endif // !PIVOT_H
