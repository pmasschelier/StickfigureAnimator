#include "pivot.h"

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

void InitStickfigureRenderer() {

}

void DeinitStickfigureRenderer() {

}

void RenderStickfigurePart(const StickfigurePart* part) {}

