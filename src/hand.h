#ifndef STICKFIGUREANIMATOR_HAND_H
#define STICKFIGUREANIMATOR_HAND_H

#include "pivot.h"

static inline void HandClearSelection(PivotEdgeIndex_array_t* edges) {
    edges->length = 0;
}

static inline void HandSelectEdge(PivotEdgeIndex_array_t* edges, PivotEdgeIndex index) {
    PivotEdgeIndex* edge = array_append_PivotEdgeIndex(edges);
    *edge = index;
}

#endif // STICKFIGUREANIMATOR_HAND_H
