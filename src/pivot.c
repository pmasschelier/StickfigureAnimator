#include "pivot.h"
#include "array.h"
#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "pivot_impl.h"

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
    };
    StickfigureJoint* ppivot = array_append_StickfigureJoint(&sf->joints);
    *ppivot = (StickfigureJoint) {
        .pos = Vector2Zero(),
        .rootAngle = 0.f
    };
    StickfigureJoint* phandle = array_append_StickfigureJoint(&sf->joints);
    *phandle = (StickfigureJoint) {
        .pos = PivotComputePosition(Vector2Zero(), data.angle, data.length),
        .rootAngle = data.angle
    };
    return sf;
}

StickfigureEdge* PivotFindRootEdge(Stickfigure* s, unsigned joint) {
    foreach(s->edges, e, StickfigureEdge) {
        if(e->to == joint)
            return e;
    }
    return nullptr;
}

StickfigureEdge* PivotAddStick(Stickfigure* s, StickfigurePartType type, unsigned pivot, PivotEdgeData data) {
    const unsigned joint = s->joints.length;
    array_append_StickfigureJoint(&s->joints);
    StickfigureEdge* edge = array_append_StickfigureEdge(&s->edges);
    *edge = (StickfigureEdge) {
        .from = pivot,
        .to = joint,
        .type = type,
        .data = data,
    };
    double rootAngle = s->joints.data[pivot].rootAngle;
    PivotUpdateJointsRec(s, edge, rootAngle);
    return edge;
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
        if(e->type == STICKFIGURE_STICK) {
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
        unsigned s_index = index;
        foreach(s->edges, e, StickfigureEdge) {
            Vector2 from = s->joints.data[e->from].pos;
            Vector2 to = s->joints.data[e->to].pos;
            bool isInside = CheckCollisionPointRec(from, rectRel) && CheckCollisionPointRec(to, rectRel);
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