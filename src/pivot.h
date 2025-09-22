#ifndef PIVOT_H
#define PIVOT_H

#include "raylib.h"

typedef enum {
    STICKFIGURE_RECT,
    STICKFIGURE_CIRCLE,
} StickfigurePartType;

typedef struct {
    unsigned sticks[2];
    unsigned handles[2];
} StickfigureJoint;

typedef struct {
    StickfigurePartType type;
    Vector2 pivot;
    Vector2 handle;
    unsigned int handle_count;
    Vector2 *handles;
} StickfigurePart;

typedef struct {
    unsigned stick_count;
    unsigned joint_count;
    StickfigurePart *sticks;
    StickfigureJoint *joints;
} Stickfigure;

void InitStickfigureRenderer();
void DeinitStickfigureRenderer();
void RenderStickfigurePart(const StickfigurePart* part);

#endif // !PIVOT_H
