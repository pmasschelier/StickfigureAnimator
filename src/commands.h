#ifndef STICKFIGUREANIMATOR_COMMANDS_H
#define STICKFIGUREANIMATOR_COMMANDS_H

#include "pivot.h"
#include <cutils/ring.h>

typedef enum {
    COMMAND_UNKNOWN,
    COMMAND_CREATE_FIGURE,
    COMMAND_DELETE_FIGURE,
    COMMAND_CREATE_STICK,
    COMMAND_REMOVE_STICK,
    COMMAND_CHANGE_STICK_DATA,
} CommandType;

typedef struct {
    char* figure;
    unsigned zindex;
} CommandCreateFigureData;

typedef struct {
    unsigned figure, from;
    PivotEdgeData data;
} CommandCreateStickData;

typedef struct {
    unsigned figure, from, to;
    PivotEdgeData data;
} CommandChangeStickData;

typedef struct {
    CommandType type;
    union {
        CommandCreateFigureData createFigure, deleteFigure;
        CommandCreateStickData createStick, removeStick;
        CommandChangeStickData changeStick;
    };
} Command;

Command CommandCreateFigure(const Stickfigure * figure, unsigned zindex);

constexpr size_t COMMAND_MAX_COUNT = 200;

void CommandPush(Stickfigure_array_t* figures, Command cmd);
void CommandUndo(Stickfigure_array_t* figures);
void CommandRedo(Stickfigure_array_t* figures);
Command CommandComputeInverse(const Command* cmd);
bool CommandRun(Stickfigure_array_t* figures, const Command* cmd);

#endif // STICKFIGUREANIMATOR_COMMANDS_H
