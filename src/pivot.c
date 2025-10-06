#include "pivot.h"
#include "raylib.h"
#include <math.h>

Stickfigure* CreateStickfigureFromPart(Stickfigure_array_t* array, StickfigurePartType type, Vector2 pivot) {
    Stickfigure* sf = array_append_Stickfigure(array);
    sf->joints = (StickfigureJoint_array_t){};
    sf->sticks = (StickfigurePart_array_t){};
    StickfigurePart* stick = array_append_StickfigurePart(&sf->sticks);
    stick->pivot = pivot;
    stick->handle = pivot;
    stick->handles = nullptr;
    stick->handle_count = 0;
    stick->type = type;
    return sf;
}

StickfigurePart* AddStickfigurePart(Stickfigure* stickfigure, unsigned part, unsigned handle) {
    Vector2 position;
    if(handle == 0)
        position = stickfigure->sticks.data[part].pivot;
    else if(handle == 1)
        position = stickfigure->sticks.data[part].handle;
    else
        position = stickfigure->sticks.data[part].handles[handle - 2];
    StickfigureJoint* joint = array_append_StickfigureJoint(&stickfigure->joints);
    joint->sticks[0] = part;
    joint->handles[0] = handle;
    joint->sticks[1] = stickfigure->sticks.length;
    joint->handles[1] = 0;
    StickfigurePart* stick = array_append_StickfigurePart(&stickfigure->sticks);
    stick->type = STICKFIGURE_RECT;
    stick->pivot = position;
    stick->handle = position;
    stick->handle_count = 0;
    return stick;
}

static inline float distance2(Vector2 a, Vector2 b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

static inline float min(float a, float b) {
    return a < b ? a : b;
}

float GetNearestJointPart(StickfigurePart* part, Vector2 position, unsigned* handle) {
    float dist_pivot = distance2(position, part->pivot);
    float dist_handle = distance2(position, part->handle);
    float d;
    unsigned d_index;
    if(dist_pivot < dist_handle) {
        d = dist_pivot;
        d_index = 0;
    } else {
        d = dist_handle;
        d_index = 1;
    }
    for (unsigned i = 0; i < part->handle_count; i++) {
        dist_handle = distance2(position, part->handles[i]);
        if(dist_handle < d) {
            d = dist_handle;
            d_index = 2 + i;
        }
    }
    *handle = d_index;
    return d;
}

float GetNearestJoint(Stickfigure_array_t stickfigures, Vector2 position, PivotIndex* joint)
{
    if(stickfigures.length == 0 || stickfigures.data[0].sticks.length == 0)
        return 0.f;
    PivotIndex i = {}, min = {};
    float min_distance2 = GetNearestJointPart(&stickfigures.data[0].sticks.data[0], position, &min.handle);
    for(i.figure = 0, i.part = 1; i.figure < stickfigures.length; i.figure++, i.part = 0) {
        for(; i.part < stickfigures.data[i.figure].sticks.length; i.part++) {
            float dist = GetNearestJointPart(&stickfigures.data[i.figure].sticks.data[i.part], position, &i.handle);
            if(dist < min_distance2) {
                min_distance2 = dist;
                min = i;
            }
        }
    }
    *joint = min;
    return sqrtf(min_distance2);
}

Vector2 GetHandlePosition(Stickfigure_array_t stickfigures, PivotIndex index) {
    Vector2 joint = {};
    if(index.figure >= stickfigures.length)
        return joint;
    Stickfigure* s = &stickfigures.data[index.figure];
    if(index.part >= s->sticks.length)
        return joint;
    StickfigurePart* p = &s->sticks.data[index.part];
    if(index.handle == 0)
        return p->pivot;
    if(index.handle == 1)
        return p->handle;
    if(index.handle >= 2 + p->handle_count)
        return joint;
    return p->handles[index.handle - 2];
}
