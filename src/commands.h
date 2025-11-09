#ifndef STICKFIGUREANIMATOR_COMMANDS_H
#define STICKFIGUREANIMATOR_COMMANDS_H

#include "pivot.h"
#include <cutils/ring.h>

bool CommandPushCreateFigure(Stickfigure_array_t* figures, const Stickfigure * figure, unsigned zindex);
bool CommandPushAddEdge(const Stickfigure_array_t* figures, Stickfigure* figure, unsigned pivot, PivotEdgeData data, StickfigureEdge** created);
bool CommandPushChangeEdgeData(const Stickfigure_array_t* figures, Stickfigure * figure, PivotEdgeIndex edge, PivotEdgeData old, PivotEdgeData new);
bool CommandPushMoveEdge(const Stickfigure_array_t* figures, Stickfigure * figure, StickfigureEdge* edge, PolarCoords old);
bool CommandPushSetPosition(const Stickfigure_array_t* figures, Stickfigure* figure, Vector2 previous, Vector2 next);

constexpr size_t COMMAND_MAX_COUNT = 200;

void CommandUndo(Stickfigure_array_t* figures);
void CommandRedo(Stickfigure_array_t* figures);

#endif // STICKFIGUREANIMATOR_COMMANDS_H
