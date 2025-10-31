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

#endif // STICKFIGUREANIMATOR_PIVOT_IMPL_H
