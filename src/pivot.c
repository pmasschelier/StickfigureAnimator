#include "pivot.h"
#include "array.h"
#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static Vector2 PivotComputePosition(Vector2 origin, double angle, double length) {
    return (Vector2) {
        origin.x + length * cos(angle),
        origin.y + length * sin(angle),
    };
}

static void PivotUpdateJointsRec(Stickfigure* s, StickfigureEdge* edge, double angle) {
    double rootAngle = angle + edge->data.angle;
    // fprintf(stderr, "joints.capacity = %d, joints.length = %d, edge = (%d, %d)\n", s->joints.capacity, s->joints.length, edge->from, edge->to);
    s->joints.data[edge->to].pos = PivotComputePosition(s->joints.data[edge->from].pos, rootAngle, edge->data.length);
    foreach(s->edges, next, StickfigureEdge) {
        if(next->from == edge->to) {
            next->rootAngle = rootAngle;
            PivotUpdateJointsRec(s, next, rootAngle);
        }
    }
}

Stickfigure* PivotCreateStickfigure(Stickfigure_array_t* array, const char* name, StickfigurePartType type, Vector2 pivot, PivotEdgeData data) {
    unsigned index = array->length;
    Stickfigure* sf = array_append_Stickfigure(array);
    if(!name)
        snprintf(sf->name, STICKFIGURE_NAME_LENGTH, "Stickfigure #%d", index);
    else
        strncpy(sf->name, name, STICKFIGURE_NAME_LENGTH);
    sf->joints = (StickfigureJoint_array_t){};
    sf->edges = (StickfigureEdge_array_t){};
    sf->position = pivot;
    StickfigureEdge* edge = array_append_StickfigureEdge(&sf->edges);
    *edge = (StickfigureEdge) {
        .from = 0,
        .to = 1,
        .type = type,
        .data = data,
        .thickness = 1.f,
    };
    StickfigureJoint* ppivot = array_append_StickfigureJoint(&sf->joints);
    ppivot->pos = Vector2Zero();
    StickfigureJoint* phandle = array_append_StickfigureJoint(&sf->joints);
    phandle->pos = PivotComputePosition(Vector2Zero(), data.angle, data.length);
    return sf;
}

StickfigureEdge* PivotFindRootEdge(Stickfigure* s, unsigned int joint) {
    foreach(s->edges, e, StickfigureEdge) {
        if(e->to == joint)
            return e;
    }
    return nullptr;
}

StickfigureEdge* PivotAddStick(Stickfigure* s, StickfigurePartType type, unsigned int pivot, PivotEdgeData data) {
    assert(s && pivot < s->joints.length);
    StickfigureEdge* root = PivotFindRootEdge(s, pivot);
    double rootAngle = root ? root->rootAngle + root->data.angle : 0.0;
    const unsigned joint = s->joints.length;
    array_append_StickfigureJoint(&s->joints);
    StickfigureEdge* edge = array_append_StickfigureEdge(&s->edges);
    *edge = (StickfigureEdge) {
        .from = pivot,
        .to = joint,
        .type = type,
        .rootAngle = rootAngle,
        .data = data,
        .thickness = 1.f,
    };
    PivotUpdateJointsRec(s, edge, rootAngle);
    return edge;
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

static bool PivotPointCollisionStickfigure(Stickfigure* s, Vector2 point, unsigned *edge) {
    foreach(s->edges, e, StickfigureEdge) {
        double d1;
        Vector2 from = s->joints.data[e->from].pos;
        Vector2 to = s->joints.data[e->to].pos;
        if(e->type == STICKFIGURE_STICK) {
            d1 = sdSegment(point, from, to);
        } else {
            d1 = sdRing(point, from, to);
        }
        d1 -= 0.5f * e->thickness;
        double d2 = Vector2Distance(point, s->joints.data[e->from].pos) - 0.5f * e->thickness;
        if(fmax(-d2, d1) < 0.f) {
            *edge = index;
            return true;
        }
    }
    return false;
}

bool PivotPointCollisionEdge(Stickfigure_array_t stickfigures, Vector2 point, PivotEdgeIndex* edge) {
    unsigned edgeId;
    foreach(stickfigures, s, Stickfigure) {
        Vector2 relativePointer = Vector2Subtract(point, s->position);
        if(PivotPointCollisionStickfigure(s, relativePointer, &edgeId)) {
            *edge = (PivotEdgeIndex){
                .figure = index,
                .edge = edgeId
            };
            return true;
        }
    }
    return false;
}

bool PivotPointCollisionJoint(Stickfigure_array_t stickfigures, Vector2 point, PivotJointIndex* joint) {
    foreach(stickfigures, s, Stickfigure) {
        Vector2 relativePointer = Vector2Subtract(point, s->position);
        int figure = index;
        foreach(s->joints, j, StickfigureJoint) {
            if(Vector2Distance(j->pos, relativePointer) < 1.f) {
                *joint = (PivotJointIndex) {
                    .figure = figure,
                    .joint = index
                };
                return true;
            }
        }
    }
    return false;
}

void PivotAppendEdgesInsideRect(Stickfigure_array_t stickfigures, Rectangle rect, PivotEdgeIndex_array_t* edges) {

    foreach(stickfigures, s, Stickfigure) {
        Rectangle rectRel = { rect.x - s->position.x, rect.y - s->position.y, rect.width, rect.height};
        unsigned s_index = index;
        foreach(s->edges, e, StickfigureEdge) {
            Vector2 from = s->joints.data[e->from].pos;
            Vector2 to = s->joints.data[e->to].pos;
            if (CheckCollisionPointRec(from, rectRel) && CheckCollisionPointRec(to, rectRel)) {
                PivotEdgeIndex* id = array_append_PivotEdgeIndex(edges);
                *id = (PivotEdgeIndex){ s_index, index };
                printf("(V) Edge (%d, %d) is selected\n", s_index, index);
            } else {
                printf("(X) Edge (%d, %d) is not selected\n", s_index, index);
            }
        }
    }
}

double PivotAngleFrom(Stickfigure* s, unsigned int joint, Vector2 point) {
    StickfigureEdge* e = PivotFindRootEdge(s, joint);
    Vector2 edge = { 1.f, 0.f };
    if(e != nullptr)
        edge = Vector2Subtract(s->joints.data[e->to].pos, s->joints.data[e->from].pos);
    Vector2 relativePointer = Vector2Subtract(point, s->position);
    Vector2 arrow = Vector2Subtract(relativePointer, s->joints.data[joint].pos);
    return Vector2Angle(edge, arrow);
}

double PivotDistanceFrom(Stickfigure* s, unsigned int joint, Vector2 point) {
    Vector2 relativePointer = Vector2Subtract(point, s->position);
    return Vector2Distance(s->joints.data[joint].pos, relativePointer);
}

void PivotMoveEdge(Stickfigure* s, unsigned int edge, double angle, double length) {
    assert(edge < s->edges.length);
    StickfigureEdge* e = &s->edges.data[edge];
    e->data.angle = angle;
    e->data.length = length;
    PivotUpdateJointsRec(s, e, e->rootAngle);
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
