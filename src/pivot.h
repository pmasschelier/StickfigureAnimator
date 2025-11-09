#ifndef PIVOT_H
#define PIVOT_H

#include "raylib.h"
#include <cutils/array.h>

#include <cutils/arena.h>
#include <stddef.h>

typedef enum : unsigned char {
    STICKFIGURE_STICK,
    STICKFIGURE_RING,
} StickfigurePartType;

typedef struct {
    double angle;
    double length;
} PolarCoords;

typedef struct {
    StickfigurePartType type;
    bool selected;
    double angle;
    double length;
    double thickness;
    Color color;
} PivotEdgeData;

typedef struct StickfigureJoint StickfigureJoint;
DEFINE_INTERFACE_ARRAY_TYPE(StickfigureJoint)
typedef struct StickfigureEdge StickfigureEdge;
DEFINE_INTERFACE_ARRAY_TYPE(StickfigureEdge)

constexpr size_t STICKFIGURE_NAME_LENGTH = 64;
typedef struct Stickfigure Stickfigure;
DEFINE_INTERFACE_ARRAY_TYPE(Stickfigure)

extern const size_t SizeofStickfigure;
#define get_figure(array, figure) (Stickfigure*)((char*)(array).data + (figure) * SizeofStickfigure)
#define PivotTopZIndex(array) (array).length

typedef struct {
    unsigned from, to;
} PivotEdgeIndex;

/**
 * @brief Initialize an allocated stickfigure
 *
 * Since Stickfigure is an opaque type the pointer should be allocated with
 * malloc or alloca.
 * Each Stickfigure initialized with this function should be deinitialized with
 * PivotClearFigure().
 *
 * @param figure Pointer to an allocated Stickfigure
 * @param name Name of the stickfigure (should be no longer than
 * STICKFIGURE_NAME_LENGTH
 * @param pivot Position of the root of the figure
 * @param data Properties of the initial edge
 * @return true iif figure was initialized properly
 *
 * @see STICKFIGURE_NAME_LENGTH
 * @see PivotFreeFigure
 */
bool PivotCreateFigure(Stickfigure* figure, const char* name, Vector2 pivot, PivotEdgeData data);

/**
 * @brief Clear a stickfigure
 * @warning This function does not free the memory pointed by figure
 * @param figure Stickfigure to clear
 */
void PivotClearFigure(Stickfigure* figure);

/**
 * @brief Compute the size needed to serialize a stickfigure
 * @param figure The figure you want to serialize
 * @return Size needed to serialize the figure
 */
size_t PivotGetSerializedSize(const Stickfigure* figure);

/**
 * @brief Serialize a figure to a buffer
 * The buffer should be allocated with at least the size returned
 * by PivotGetSerializedSize()
 * @param figure The figure you want to serialize
 * @param buffer A pointer to the buffer you want to serialize to
 */
void PivotSerialize(const Stickfigure* figure, char* buffer);

/**
 * @brief Read a serialized stickfigure from a buffer
 * @param buffer The buffer you want to read from
 * @param figure A pointer to an empty Stickfigure struct
 * @return true iif the deserialization could be done properly
 */
bool PivotDeserialize(const char* buffer, Stickfigure* figure);

void PivotRemoveStick(Stickfigure* s, StickfigureEdge* edge);

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
void PivotAppendEdgesInsideRect(Stickfigure_array_t stickfigures, Rectangle rect, bool unselectOthers);
StickfigureEdge* PivotFindRootEdge(Stickfigure* stickfigure, unsigned joint);
void PivotClearSelection(Stickfigure_array_t stickfigures);
void PivotSetColorSelection(Stickfigure_array_t* stickfigures, Color color);

#endif // !PIVOT_H
