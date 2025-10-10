#include "pivot.h"
#include "array.h"
#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <math.h>

static inline Vector2 PivotComputePosition(Vector2 origin, double angle, double length) {
    return (Vector2) {
        origin.x + length * cos(angle),
        origin.y + length * sin(angle),
    };
}

static void PivotUpdateJointsRec(Stickfigure* s, StickfigureEdge* edge, double angle) {
    double rootAngle = angle + edge->angle;
    s->joints.data[edge->to].pos = PivotComputePosition(s->joints.data[edge->from].pos, rootAngle, edge->length);
    foreach(s->edges, next, StickfigureEdge) {
        if(next->from == edge->to) {
            next->rootAngle = rootAngle;
            PivotUpdateJointsRec(s, next, rootAngle);
        }
    }
}

Stickfigure* PivotCreateStickfigure(Stickfigure_array_t* array, StickfigurePartType type, Vector2 pivot, double angle, double length) {
    Stickfigure* sf = array_append_Stickfigure(array);
    sf->joints = (StickfigureJoint_array_t){};
    sf->edges = (StickfigureEdge_array_t){};
    StickfigureEdge* edge = array_append_StickfigureEdge(&sf->edges);
    *edge = (StickfigureEdge) {
        .from = 0,
        .to = 1,
        .type = type,
        .angle = angle,
        .length = length,
    };
    StickfigureJoint* ppivot = array_append_StickfigureJoint(&sf->joints);
    ppivot->pos = pivot;
    StickfigureJoint* phandle = array_append_StickfigureJoint(&sf->joints);
    phandle->pos = PivotComputePosition(pivot, angle, length);
    return sf;
}

StickfigureEdge* PivotFindRootEdge(Stickfigure* s, unsigned int joint) {
    foreach(s->edges, e, StickfigureEdge) {
        if(e->to == joint)
            return e;
    }
    return nullptr;
}

StickfigureEdge* PivotAddStick(Stickfigure* s, StickfigurePartType type, unsigned int pivot, double angle, double length) {
    assert(s && pivot < s->joints.length);
    StickfigureEdge* root = PivotFindRootEdge(s, pivot);
    double rootAngle = root ? root->rootAngle + root->angle : 0.0;
    int joint = s->joints.length;
    array_append_StickfigureJoint(&s->joints);
    StickfigureEdge* edge = array_append_StickfigureEdge(&s->edges);
    *edge = (StickfigureEdge) {
        .from = pivot,
        .to = joint,
        .type = type,
        .rootAngle = rootAngle,
        .angle = angle,
        .length = length
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

float sdSegment(Vector2 p, Vector2 a, Vector2 b)
{
    Vector2 pa = Vector2Subtract(p, a);
    Vector2 ba = Vector2Subtract(b, a);
    float h = Clamp(Vector2DotProduct(pa,ba) / Vector2DotProduct(ba,ba), 0.0, 1.0 );
    return Vector2Length(Vector2Subtract(pa, Vector2Scale(ba, h)));
}

bool PivotPointCollisionRec(Stickfigure* s, Vector2 point, unsigned joint, unsigned *edge) {
    foreach(s->edges, e, StickfigureEdge) {
        if(e->from == joint) {
            float d1 = sdSegment(point, s->joints.data[e->from].pos, s->joints.data[e->to].pos) - 1.f;
            float d2 = Vector2Distance(point, s->joints.data[e->from].pos) - 1.f;
            if(fmax(-d2, d1) < 0.f) {
                *edge = index;
                return true;
            }
            if(PivotPointCollisionRec(s, point, e->to, edge))
                return true;
        }
    }
    return false;
}

bool PivotPointCollisionEdge(Stickfigure_array_t stickfigures, Vector2 point, PivotEdgeIndex* edge) {
    unsigned edgeId;
    foreach(stickfigures, s, Stickfigure) {
        if(PivotPointCollisionRec(s, point, 0, &edgeId)) {
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
        int figure = index;
        foreach(s->joints, j, StickfigureJoint) {
            if(Vector2Distance(j->pos, point) < 1.f) {
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

double PivotAngleFrom(Stickfigure* s, unsigned int joint, Vector2 point) {
    StickfigureEdge* e = PivotFindRootEdge(s, joint);
    Vector2 edge = { 1.f, 0.f };
    if(e != nullptr)
        edge = Vector2Subtract(s->joints.data[e->to].pos, s->joints.data[e->from].pos);
    Vector2 arrow = Vector2Subtract(point, s->joints.data[joint].pos);
    /* printf("EDGE (%f, %f) ARROW (%f, %f)\n", edge.x, edge.y, arrow.x, arrow.y); */
    return Vector2Angle(edge, arrow);
}

void PivotMoveEdge(Stickfigure* s, unsigned int edge, double angle, double length) {
    assert(edge < s->edges.length);
    StickfigureEdge* e = &s->edges.data[edge];
    e->angle = angle;
    e->length = length;
    PivotUpdateJointsRec(s, e, e->rootAngle);
}
