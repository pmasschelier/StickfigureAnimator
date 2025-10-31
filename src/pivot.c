#include "pivot.h"
#include "cutils/when_macros.h"
#include "pivot_impl.h"
#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <cutils/array.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <alloca.h>

const size_t SizeofStickfigure = sizeof(Stickfigure);

static Vector2 PivotComputePosition(Vector2 origin, double angle, double length) {
    return (Vector2) {
        origin.x + length * cos(angle),
        origin.y + length * sin(angle),
    };
}

static void PivotUpdateJointsRec(Stickfigure* s, StickfigureEdge* edge, double angle) {
    double rootAngle = angle + edge->data.angle;
    s->joints.data[edge->to].pos = PivotComputePosition(s->joints.data[edge->from].pos, rootAngle, edge->data.length);
    s->joints.data[edge->to].rootAngle = rootAngle;
    foreach(s->edges, next, StickfigureEdge) {
        if(next->from == edge->to) {
            PivotUpdateJointsRec(s, next, rootAngle);
        }
    }
}

bool PivotCreateFigure(Stickfigure* figure, const char* name, Vector2 pivot, PivotEdgeData data) {
    if(name)
        strncpy(figure->name, name, STICKFIGURE_NAME_LENGTH);
    figure->joints = (StickfigureJoint_array_t){};
    figure->edges = (StickfigureEdge_array_t){};
    figure->position = pivot;
    StickfigureEdge* edge = array_append_StickfigureEdge(&figure->edges, 1);
    *edge = (StickfigureEdge) {
        .from = 0,
        .to = 1,
        .data = data,
    };
    StickfigureJoint* joints = array_append_StickfigureJoint(&figure->joints, 2);
    joints[0] = (StickfigureJoint) {
        .pos = Vector2Zero(),
        .rootAngle = 0.f
    };
    joints[1] = (StickfigureJoint) {
        .pos = PivotComputePosition(Vector2Zero(), data.angle, data.length),
        .rootAngle = data.angle
    };
    return figure;
}

void PivotClearFigure(Stickfigure* figure) {
    array_free_StickfigureJoint(&figure->joints);
    array_free_StickfigureEdge(&figure->edges);
}

size_t PivotGetSerializedSize(const Stickfigure* figure) {
    size_t size = 0;
    size += sizeof(figure->name);
    size += sizeof(figure->position);
    size += sizeof(figure->joints.length);
    size += sizeof(StickfigureJoint) * figure->joints.length;
    size += sizeof(figure->edges.length);
    size += sizeof(StickfigureEdge) * figure->edges.length;
    return size;
}

void PivotSerialize(const Stickfigure* figure, char* buffer) {
    memcpy(buffer, &figure->name, sizeof(figure->name));
    buffer += sizeof(figure->name);
    *(Vector2*)buffer = figure->position;
    buffer += sizeof(figure->position);
    *(unsigned*)buffer = figure->joints.length;
    buffer += sizeof(figure->joints.length);
    const size_t jointBufferSize = figure->joints.length * sizeof(StickfigureJoint);
    memcpy(buffer, figure->joints.data, jointBufferSize);
    buffer += jointBufferSize;
    *(unsigned*)buffer = figure->edges.length;
    buffer += sizeof(figure->edges.length);
    const size_t edgeBufferSize = figure->edges.length * sizeof(StickfigureEdge);
    memcpy(buffer, figure->edges.data, edgeBufferSize);
}

bool PivotDeserialize(const char* buffer, Stickfigure* figure) {
    bool ret = true;
    printf("buffer: %p\n", buffer);
    // Read the figure name from buffer
    memcpy(&figure->name, buffer, sizeof(figure->name));
    buffer += sizeof(figure->name);
    printf("buffer: %p\n", buffer);
    // Read the figure position from buffer
    figure->position = *(Vector2*)buffer;
    buffer += sizeof(figure->position);
    printf("buffer: %p\n", buffer);
    // Read the joints from buffer
    const unsigned jointCount = *(unsigned*)buffer;
    const unsigned jointSize = jointCount * sizeof(StickfigureJoint);
    buffer += sizeof(figure->joints.length);
    printf("buffer: %p\n", buffer);
    figure->joints = (StickfigureJoint_array_t){};
    StickfigureJoint* joints = array_append_StickfigureJoint(&figure->joints, jointCount);
    when_null_jmp(joints, false, error1);
    memcpy(joints, buffer, jointSize);
    buffer += jointSize;
    printf("buffer: %p\n", buffer);
    // Read the edges from buffer
    const unsigned edgeCount = *(unsigned*)buffer;
    const unsigned edgeSize = jointCount * sizeof(StickfigureEdge);
    buffer += sizeof(figure->edges.length);
    printf("buffer: %p\n", buffer);
    figure->edges = (StickfigureEdge_array_t){};
    StickfigureEdge* edges = array_append_StickfigureEdge(&figure->edges, edgeCount);
    when_null_jmp(edges, false, error2);
    memcpy(edges, buffer, edgeSize);
    printf("buffer: %p\n", buffer);
    return ret;
    error2:
    array_free_StickfigureJoint(&figure->joints);
    error1:
    return ret;
}

void PivotCloneStickfigure(const Stickfigure* figure, Stickfigure* clone) {
    strncpy(clone->name, figure->name, STICKFIGURE_NAME_LENGTH);
    clone->position = figure->position;
    clone->edges = array_shallow_clone_StickfigureEdge(&figure->edges);
    clone->joints = array_shallow_clone_StickfigureJoint(&figure->joints);
}

Stickfigure* PivotInsertFigure(Stickfigure_array_t* array, const Stickfigure* figure, unsigned zindex) {
    Stickfigure* ret = array_insert_Stickfigure(array, nullptr, zindex);
    PivotCloneStickfigure(figure, ret);
    return ret;
}

StickfigureEdge* PivotFindRootEdge(Stickfigure* s, unsigned joint) {
    foreach(s->edges, e, StickfigureEdge) {
        if(e->to == joint)
            return e;
    }
    return nullptr;
}

StickfigureEdge* PivotAddStick(Stickfigure* s, unsigned pivot, PivotEdgeData data) {
    const unsigned handleId = s->joints.length;
    StickfigureJoint* joint = array_append_StickfigureJoint(&s->joints, 1);
    StickfigureEdge* edge = array_append_StickfigureEdge(&s->edges, 1);
    if (joint == nullptr || edge == nullptr)
        return nullptr;
    *edge = (StickfigureEdge) {
        .from = pivot,
        .to = handleId,
        .data = data,
    };
    double rootAngle = s->joints.data[pivot].rootAngle;
    PivotUpdateJointsRec(s, edge, rootAngle);
    return edge;
}

void PivotMarkRec(Stickfigure* s, unsigned joint, bool *markedJoints, bool* markedEdges) {
    foreach(s->edges, e, StickfigureEdge) {
        if(e->from == joint) {
            if (markedEdges)
                markedEdges[index] = true;
            if (markedJoints)
                markedJoints[e->to] = true;
            PivotMarkRec(s, e->to, markedJoints, markedEdges);
        }
    }
}

void PivotRemoveStick(Stickfigure* s, StickfigureEdge* edge) {
    // Marks the edges and joints from edge for removal
    bool* markedJoints = alloca(s->joints.length * sizeof(bool));
    memset(markedJoints, 0, s->joints.length * sizeof(bool));
    markedJoints[edge->to] = true;
    bool* markedEdges = alloca(s->edges.length * sizeof(bool));
    memset(markedEdges, 0, s->edges.length * sizeof(bool));
    markedEdges[array_indexof(s->edges, edge)] = true;
    PivotMarkRec(s, edge->to, markedJoints, markedEdges);
    // Remove marked joints and edges
    array_filter_StickfigureJoint(&s->joints, markedJoints);
    array_filter_StickfigureEdge(&s->edges, markedEdges);
    // Compute new joint ids after removal
    long* jointIds = alloca(s->joints.length * sizeof(long));
    unsigned id = 0;
    for (unsigned i = 0; i < s->joints.length; i++)
        jointIds[i] = markedJoints[i] ? -1 : id++;
    // Update joint ids of edges
    foreach(s->edges, e, StickfigureEdge) {
        assert(jointIds[e->from] >= 0);
        e->from = jointIds[e->from];
        assert(jointIds[e->to] >= 0);
        e->to = jointIds[e->to];
    }
}

StickfigureEdge* PivotFindEdge(Stickfigure* s, PivotEdgeIndex edge) {
    foreach(s->edges, e, StickfigureEdge) {
        if (e->from == edge.from && e->to == edge.to)
            return e;
    }
    return nullptr;
}

const PivotEdgeData* PivotGetEdgeData(const StickfigureEdge* edge) {
    return &edge->data;
}

unsigned PivotGetPivotIndex(const StickfigureEdge* edge) {
    return edge->from;
}

Vector2* PivotStickfigurePosition(Stickfigure* s) {
    return &s->position;
}

char* PivotStickfigureName(Stickfigure* s) {
    return s->name;
}

void PivotFreeAll(Stickfigure_array_t* array) {
    for(unsigned i = 0; i < array->length; i++) {
        array_free_StickfigureEdge(&array->data[i].edges);
        array_free_StickfigureJoint(&array->data[i].joints);
    }
    array_free_Stickfigure(array);
}

static float sdSegment(Vector2 p, Vector2 a, Vector2 b)
{
    const Vector2 pa = Vector2Subtract(p, a);
    const Vector2 ba = Vector2Subtract(b, a);
    const float h = Clamp(Vector2DotProduct(pa,ba) / Vector2DotProduct(ba,ba), 0.f, 1.f );
    return Vector2Length(Vector2Subtract(pa, Vector2Scale(ba, h)));
}

static float sdRing(Vector2 p, Vector2 a, Vector2 b) {
    const Vector2 center = Vector2Scale(Vector2Add(a, b), 0.5f);
    const float radius = 0.5f * Vector2Distance(a, b);
    return fabsf(Vector2Distance(p, center) - radius);
}

static bool PivotPointCollisionStickfigure(const Stickfigure* s, const Vector2 point, StickfigureEdge **edge) {
    foreach(s->edges, e, StickfigureEdge) {
        double d1;
        const Vector2 from = s->joints.data[e->from].pos;
        const Vector2 to = s->joints.data[e->to].pos;
        if(e->data.type == STICKFIGURE_STICK) {
            d1 = sdSegment(point, from, to);
        } else {
            d1 = sdRing(point, from, to);
        }
        d1 -= 0.5f * e->data.thickness;
        const double d2 = Vector2Distance(point, s->joints.data[e->from].pos) - 0.5f * e->data.thickness;
        if(fmax(-d2, d1) < 0.f) {
            *edge = e;
            return true;
        }
    }
    return false;
}

bool PivotPointCollisionEdge(Stickfigure_array_t stickfigures, Vector2 point, Stickfigure** figure, StickfigureEdge** edge) {
    foreach(stickfigures, s, Stickfigure) {
        const Vector2 relativePointer = Vector2Subtract(point, s->position);
        if(PivotPointCollisionStickfigure(s, relativePointer, edge)) {
            *figure = s;
            return true;
        }
    }
    return false;
}

bool PivotPointCollisionJoint(Stickfigure_array_t stickfigures, Vector2 point, Stickfigure** figure, unsigned* joint) {
    foreach(stickfigures, s, Stickfigure) {
        Vector2 relativePointer = Vector2Subtract(point, s->position);
        foreach(s->joints, j, StickfigureJoint) {
            if(Vector2Distance(j->pos, relativePointer) < 1.f) {
                *joint = index;
                *figure = s;
                return true;
            }
        }
    }
    return false;
}

void PivotAppendEdgesInsideRect(Stickfigure_array_t stickfigures, Rectangle rect, bool unselectOthers) {
    foreach(stickfigures, s, Stickfigure) {
        Rectangle rectRel = { rect.x - s->position.x, rect.y - s->position.y, rect.width, rect.height};
        foreach(s->edges, e, StickfigureEdge) {
            const Vector2 from = s->joints.data[e->from].pos;
            const Vector2 to = s->joints.data[e->to].pos;
            const bool isInside = CheckCollisionPointRec(from, rectRel) && CheckCollisionPointRec(to, rectRel);
            e->data.selected = isInside || (e->data.selected && !unselectOthers);
        }
    }
}

double PivotAngleFrom(
    const Stickfigure* s, const unsigned pivot, const Vector2 point
) {
    const double rootAngle = s->joints.data[pivot].rootAngle;
    const Vector2 relativePointer = Vector2Subtract(point, s->position);
    const Vector2 arrow = Vector2Subtract(relativePointer, s->joints.data[pivot].pos);
    const double angle = atan2f(arrow.y, arrow.x);
    return angle - rootAngle;
}

double PivotDistanceFrom(
    const Stickfigure* s, const unsigned pivot, const Vector2 point
    ) {
    const Vector2 relativePointer = Vector2Subtract(point, s->position);
    return Vector2Distance(s->joints.data[pivot].pos, relativePointer);
}

void PivotMoveEdge(Stickfigure* s, StickfigureEdge* edge, double angle, double length) {
    edge->data.angle = angle;
    edge->data.length = length;
    const double rootAngle = s->joints.data[edge->from].rootAngle;
    PivotUpdateJointsRec(s, edge, rootAngle);
}

void PivotSelectEdge(StickfigureEdge* edge, bool toggle) {
    edge->data.selected = !toggle || !edge->data.selected;
}

void PivotRemoveEdge(Stickfigure* s, unsigned int edge) {
    assert(edge < s->edges.length);
    StickfigureEdge *e = &s->edges.data[edge];
    foreach(s->edges, next, StickfigureEdge) {
        if(next->from == e->to)
            return;
    }
    array_swap_and_pop_back_StickfigureJoint(&s->joints, e->to);
    array_swap_and_pop_back_StickfigureEdge(&s->edges, edge);
    foreach(s->edges, edge, StickfigureEdge) {
        if(edge->from == s->joints.length)
            edge->from = e->to;
        if(edge->to == s->joints.length)
            edge->to = e->to;
    }
}

void PivotClearSelection(Stickfigure_array_t stickfigures) {
    foreach(stickfigures, s, Stickfigure) {
        foreach(s->edges, e, StickfigureEdge) {
            e->data.selected = false;
        }
    }
}

void PivotSetColorSelection(Stickfigure_array_t* stickfigures, Color color) {
    foreach(*stickfigures, s, Stickfigure) {
        foreach(s->edges, e, StickfigureEdge) {
            if (e->data.selected)
                e->data.color = color;
        }
    }
}
