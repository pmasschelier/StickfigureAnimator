#ifndef STICKFIGUREANIMATOR_PIVOT_IMPL_H
#define STICKFIGUREANIMATOR_PIVOT_IMPL_H

#include "pivot.h"

struct StickfigureEdge {
  unsigned from;
  unsigned to;
  PivotEdgeData data;
};

struct StickfigureJoint{
  Vector2 pos;
  double rootAngle;
};

DEFINE_IMPLEMENTATION_ARRAY_TYPE(StickfigureEdge)
DEFINE_IMPLEMENTATION_ARRAY_TYPE(StickfigureJoint)

struct Stickfigure {
  char name[STICKFIGURE_NAME_LENGTH];
  Vector2 position;
  StickfigureJoint_array_t joints;
  StickfigureEdge_array_t edges;
};

DEFINE_IMPLEMENTATION_ARRAY_TYPE(Stickfigure)

Stickfigure* PivotInsertFigure(Stickfigure_array_t* array, const Stickfigure* figure, unsigned zindex);

void PivotCloneStickfigure(const Stickfigure* figure, Stickfigure* clone);

StickfigureEdge* PivotAddStick(Stickfigure* s, unsigned pivot, PivotEdgeData data);

bool PivotRemoveLeafEdge(Stickfigure* s, PivotEdgeIndex edge);

#endif // STICKFIGUREANIMATOR_PIVOT_IMPL_H
