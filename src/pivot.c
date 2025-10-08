#include "pivot.h"
#include "array.h"
#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <math.h>

Stickfigure* PivotCreateStickfigure(Stickfigure_array_t* array, StickfigurePartType type, Vector2 pivot, Vector2 handle) {
    Stickfigure* sf = array_append_Stickfigure(array);
    sf->joints = (StickfigureJoint_array_t){};
    sf->edges = (StickfigureEdge_array_t){};
    StickfigureEdge* edge = array_append_StickfigureEdge(&sf->edges);
    *edge = (StickfigureEdge) {
        .from = 0,
        .to = 1,
        .type = type
    };
    StickfigureJoint* ppivot = array_append_StickfigureJoint(&sf->joints);
    ppivot->pos = pivot;
    StickfigureJoint* phandle = array_append_StickfigureJoint(&sf->joints);
    phandle->pos = handle;
    return sf;
}

StickfigureEdge* PivotAddStick(Stickfigure* s, StickfigurePartType type, unsigned int pivot) {
    assert(s && pivot < s->joints.length);
    int ret = s->joints.length;
    StickfigureJoint* handle = array_append_StickfigureJoint(&s->joints);
    handle->pos = s->joints.data[pivot].pos;
    StickfigureEdge* edge = array_append_StickfigureEdge(&s->edges);
    *edge = (StickfigureEdge) {
        .from = pivot,
        .to = ret,
        .type = type
    };
    return edge;
}

void PivotFreeAll(Stickfigure_array_t* array) {
    for(unsigned i = 0; i < array->length; i++) {
        array_free_StickfigureEdge(&array->data[i].edges);
        array_free_StickfigureJoint(&array->data[i].joints);
    }
    array_free_Stickfigure(array);
}

static inline float distance2(Vector2 a, Vector2 b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

float PivotGetNearestJoint(Stickfigure_array_t stickfigures, Vector2 position, PivotIndex* joint)
{
    if(stickfigures.length == 0 || stickfigures.data[0].joints.length == 0)
        return 0.f;
    PivotIndex i = {}, min = {};
    float min_distance2 = distance2(stickfigures.data[0].joints.data[0].pos, position);
    for(i.figure = 0, i.joint = 1; i.figure < stickfigures.length; i.figure++, i.joint = 0) {
        for(; i.joint < stickfigures.data[i.figure].joints.length; i.joint++) {
            float dist  = distance2(stickfigures.data[i.figure].joints.data[i.joint].pos, position);
            if(dist < min_distance2) {
                min_distance2 = dist;
                min = i;
            }
        }
    }
    *joint = min;
    return sqrtf(min_distance2);
}

unsigned int PivotFindPredecessor(Stickfigure* s, unsigned int joint) {
    foreach(s->edges, e, StickfigureEdge) {
        if(e->to == joint)
            return e->from;
    }
    return -1;
}

void PivotMoveJoint(Stickfigure_array_t stickfigures, PivotIndex joint, Vector2 start, float angle) {
    unsigned int pred = PivotFindPredecessor(&stickfigures.data[joint.figure], joint.joint);
    Vector2 pivot = stickfigures.data[joint.figure].joints.data[joint.joint].pos;
    Vector2 handle = stickfigures.data[joint.figure].joints.data[pred].pos;
    Vector2 edge = Vector2Subtract(handle, pivot);
    Vector2 edge2 = Vector2Rotate(edge, angle);
    stickfigures.data[joint.figure].joints.data[joint.joint].pos = Vector2Add(pivot, edge2);
}
