#include "commands.h"

#include "clay/clay.h"

#include <cutils/allocator/arena.h>
#include <pivot_impl.h>

arena_allocator_t arena;

typedef enum {
    COMMAND_UNKNOWN,
    COMMAND_CREATE_FIGURE,
    COMMAND_DELETE_FIGURE,
    COMMAND_CREATE_EDGE,
    COMMAND_REMOVE_EDGE,
    COMMAND_MOVE_EDGE,
    COMMAND_CHANGE_EDGE_DATA,
    COMMAND_SET_POSITION,
} CommandType;

typedef struct {
    unsigned zindex;
    char *serialized;
} CommandCreateFigureData;

typedef struct {
    unsigned figure;
    PivotEdgeIndex edge;
    PivotEdgeData *serialized;
} CommandCreateStickData;

typedef struct {
    unsigned figure;
    PivotEdgeIndex edge;
    PolarCoords previous, next;
    bool inversed;
} CommandMoveEdgeData;

typedef struct {
    unsigned figure;
    PivotEdgeIndex edge;
    bool inversed;
    PivotEdgeData *serialized;
} CommandChangeStickData;

typedef struct {
    unsigned figure;
    Vector2 previous;
    Vector2 next;
    bool inversed;
} CommandSetPositionData;

typedef struct {
    CommandType type;
    bool tmp;
    union {
        CommandCreateFigureData createFigure, deleteFigure;
        CommandCreateStickData createStick, removeStick;
        CommandMoveEdgeData moveEdge;
        CommandChangeStickData changeStick;
        CommandSetPositionData setPosition;
    };
} Command;

void CommandFree(Command* cmd) {
    switch (cmd->type) {
    case COMMAND_CREATE_FIGURE:
        free(cmd->createFigure.serialized);
        cmd->createFigure.serialized = nullptr;
        break;
    case COMMAND_CREATE_EDGE:
        free(cmd->createStick.serialized);
        cmd->createStick.serialized = nullptr;
        break;
    default:
        break;
    }
}

DEFINE_RING_TYPE(Command)

Command cmd_buffer[COMMAND_MAX_COUNT];
auto cmd_list = ring_init(Command, cmd_buffer, COMMAND_MAX_COUNT, CommandFree);

unsigned cmd_index = 0;

static bool CommandRunAddEdge(Stickfigure* figure, CommandCreateStickData* cmdData, const PivotEdgeData* edgeData, StickfigureEdge** created) {
    StickfigureEdge *edge =
        PivotAddStick(figure, cmdData->edge.from, *edgeData);
    if (!edge) return false;
    cmdData->edge.to = edge->to;
    if (created) *created = edge;
    return true;
}
static bool CommandRunMoveEdge(Stickfigure* figure, CommandMoveEdgeData* cmdData) {
    StickfigureEdge* edge = PivotFindEdge(figure, cmdData->edge);
    if (!edge) return false;
    PolarCoords coord = cmdData->inversed ? cmdData->previous : cmdData->next;
    PivotMoveEdge(figure, edge, coord.angle, coord.length);
    return true;
}

static bool CommandRunChangeEdgeData(Stickfigure* figure, const CommandChangeStickData* cmdData) {
    // auto* e = PivotFindEdge(figure, cmdData->edge);
    // *PivotGetEdgeData(e) = cmdData->serialized[cmdData->inversed];
}

static bool CommandRunSetPosition(Stickfigure* figure, const CommandSetPositionData* cmdData) {
    const Vector2 position = cmdData->inversed ? cmdData->previous : cmdData->next;
    *PivotStickfigurePosition(figure) = position;
    return true;
}

bool CommandRun(Stickfigure_array_t* figures, Command* command)  {
    Stickfigure* figure;
    switch (command->type) {
    case COMMAND_CREATE_FIGURE:
        figure = array_insert_Stickfigure(figures, nullptr, command->createFigure.zindex);
        return PivotDeserialize(command->createFigure.serialized, figure);
    case COMMAND_DELETE_FIGURE:
        return array_remove_Stickfigure(figures, nullptr, command->deleteFigure.zindex);
    case COMMAND_CREATE_EDGE:
        return CommandRunAddEdge(&figures->data[command->createStick.figure], &command->createStick, command->createStick.serialized, nullptr);
    case COMMAND_REMOVE_EDGE:
        return PivotRemoveLeafEdge(&figures->data[command->removeStick.figure], command->removeStick.edge);
    case COMMAND_MOVE_EDGE:
        return CommandRunMoveEdge(&figures->data[command->setPosition.figure], &command->moveEdge);
    case COMMAND_SET_POSITION:
        return CommandRunSetPosition(&figures->data[command->setPosition.figure], &command->setPosition);
    case COMMAND_CHANGE_EDGE_DATA:
        break;
    case COMMAND_UNKNOWN:
        break;
    }
    return true;
}

static Command* CommandPush(const Command cmd) {
    cmd_index = ring_head_Command(&cmd_list, cmd_index - cmd_list.begin);
    if (!ring_full(cmd_list)) cmd_index++;
    Command* ptr = ring_push_back_Command(&cmd_list, true);
    memcpy(ptr, &cmd, sizeof(Command));
    return ptr;
}

bool CommandPushCreateFigure(Stickfigure_array_t* figures, const Stickfigure * figure, const unsigned zindex) {
    const size_t size = PivotGetSerializedSize(figure);
    char* buffer = malloc(size);
    PivotSerialize(figure, buffer);
    Command* cmd = CommandPush((Command) {
        .type = COMMAND_CREATE_FIGURE,
        .tmp = false,
        .createFigure = {
            .serialized = buffer,
            .zindex = zindex
        }
    });
    if (!cmd) return false;
    return CommandRun(figures, cmd);
}

bool CommandPushAddEdge(const Stickfigure_array_t * figures, Stickfigure* figure, const unsigned pivot, const PivotEdgeData data, StickfigureEdge** created) {
    char* buffer = malloc(sizeof(PivotEdgeData));
    memcpy(buffer, &data, sizeof(PivotEdgeData));
    Command* cmd = CommandPush((Command) {
        .type = COMMAND_CREATE_EDGE,
        .tmp = true,
        .createStick = {
            .figure = array_indexof(*figures, figure),
            .edge = {. from = pivot },
            .serialized = buffer,
        }
    });
    if (!cmd) return false;
    return CommandRunAddEdge(figure, &cmd->createStick, &data, created);
}

bool CommandPushChangeEdgeData(const Stickfigure_array_t* figures,
                               Stickfigure * figure, const PivotEdgeIndex edge,
                               const PivotEdgeData old,
                               const PivotEdgeData new) {
    PivotEdgeData* buffer = malloc(2 * sizeof(PivotEdgeData));
    memcpy(buffer,  &old, sizeof(PivotEdgeData));
    memcpy(buffer + 1,  &new, sizeof(PivotEdgeData));
    Command* cmd = CommandPush((Command) {
        .type = COMMAND_CHANGE_EDGE_DATA,
        .tmp = false,
        .changeStick = {
            .figure = array_indexof(*figures, figure),
            .inversed = false,
            .edge = edge,
            .serialized = buffer
        }
    });
    if (!cmd) return false;
    return CommandRunChangeEdgeData(figure, &cmd->changeStick);
}

bool CommandPushMoveEdge(const Stickfigure_array_t* figures, Stickfigure * figure, StickfigureEdge* edge, PolarCoords old) {
    const PivotEdgeData* data = PivotGetEdgeData(edge);
    Command* cmd = CommandPush((Command) {
        .type = COMMAND_MOVE_EDGE,
        .tmp = false,
        .moveEdge = {
            .figure = array_indexof(*figures, figure),
            .edge = { .from = edge->from, .to = edge->to },
            .inversed = false,
            .previous = old,
            .next = {.angle = data->angle, .length = data->length}
        }
    });
    if (!cmd) return false;
    return CommandRunMoveEdge(figure, &cmd->moveEdge);
}

bool CommandPushSetPosition(const Stickfigure_array_t* figures, Stickfigure* figure, Vector2 previous, Vector2 next) {
    const Command * cmd = CommandPush((Command) {
        .type = COMMAND_SET_POSITION,
        .tmp = false,
        .setPosition = {
            .figure = array_indexof(*figures, figure),
            .previous = previous,
            .next = next,
            .inversed = false
        }
    });
    return CommandRunSetPosition(figure, &cmd->setPosition);
}

Command CommandComputeInverse(const Command* command) {
    Command ret;
    switch (command->type) {
    case COMMAND_CREATE_FIGURE:
        ret = *command;
        ret.type = COMMAND_DELETE_FIGURE;
        break;
    case COMMAND_DELETE_FIGURE:
        ret = *command;
        ret.type = COMMAND_CREATE_FIGURE;
        break;
    case COMMAND_CREATE_EDGE:
        ret = *command;
        ret.type = COMMAND_REMOVE_EDGE;
        break;
    case COMMAND_REMOVE_EDGE:
        ret = *command;
        ret.type = COMMAND_CREATE_EDGE;
        break;
    case COMMAND_MOVE_EDGE:
        ret = *command;
        ret.moveEdge.inversed = !ret.moveEdge.inversed;
        break;
    case COMMAND_CHANGE_EDGE_DATA:
        ret = *command;
        ret.changeStick.inversed = !ret.changeStick.inversed;
        break;
    case COMMAND_SET_POSITION:
        ret = *command;
        ret.setPosition.inversed = !ret.setPosition.inversed;
        break;
    case COMMAND_UNKNOWN:
        return (Command) { .type = COMMAND_UNKNOWN };
    }
    return ret;
}

void CommandUndo(Stickfigure_array_t* figures) {
    if (ring_empty(cmd_list) || cmd_index == 0)
        return;
    Command reverse = CommandComputeInverse(&cmd_list.data[--cmd_index]);
    CommandRun(figures, &reverse);
    Command* cmd;
    while (cmd_index != 0 && (cmd = ring_get(cmd_list, cmd_index - 1)) != NULL && cmd->tmp) {
        reverse = CommandComputeInverse(&cmd_list.data[--cmd_index]);
        CommandRun(figures, &reverse);
    }
}

void CommandRedo(Stickfigure_array_t* figures) {
    Command* cmd;
    while (cmd_index != cmd_list.length && (cmd = ring_get(cmd_list, cmd_index)) != NULL && cmd->tmp) {
        CommandRun(figures, &cmd_list.data[cmd_index++]);
    }
    if (cmd_index == cmd_list.length)
        return;
    CommandRun(figures, &cmd_list.data[cmd_index++]);
}