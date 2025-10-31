#include "commands.h"

#include "clay/clay.h"

#include <cutils/arena.h>
#include <pivot_impl.h>

arena_allocator_t arena;

DEFINE_RING_TYPE(Command)

Command cmd_buffer[COMMAND_MAX_COUNT];
Command_ring_t cmd_list = {
    .data = cmd_buffer,
    .capacity = COMMAND_MAX_COUNT
};
unsigned cmd_index = 0;

Command CommandCreateFigure(const Stickfigure * figure, const unsigned zindex) {
    const size_t size = PivotGetSerializedSize(figure);
    char* buffer = arena_allocate(&arena, size);
    PivotSerialize(figure, buffer);
    return (Command) {
        .type = COMMAND_CREATE_FIGURE,
        .createFigure = {
            .figure = buffer,
            .zindex = zindex
        }
    };
}

void CommandPush(Stickfigure_array_t* figures, const Command cmd) {
    Command* ptr = ring_push_back_Command(&cmd_list, true);
    memcpy(ptr, &cmd, sizeof(Command));
    CommandRun(figures, &cmd);
    cmd_index = ring_next(cmd_list, cmd_index);
}

void CommandUndo(Stickfigure_array_t* figures) {
    if (ring_empty(cmd_list))
        return;
    cmd_index = ring_prev(cmd_list, cmd_index);
    const Command reverse = CommandComputeInverse(&cmd_list.data[cmd_index]);
    CommandRun(figures, &reverse);
}

void CommandRedo(Stickfigure_array_t* figures) {
    if (cmd_index == cmd_list.next)
        return;
    CommandRun(figures, &cmd_list.data[cmd_index]);
    cmd_index = ring_next(cmd_list, cmd_index);
}

void CommandListInit() {
    cmd_list = (Command_ring_t) {
        .data = cmd_buffer,
        .capacity = COMMAND_MAX_COUNT
    };
}

Command CommandComputeInverse(const Command* command) {
    switch (command->type) {
    case COMMAND_CREATE_FIGURE:
        return (Command) {
            .type = COMMAND_DELETE_FIGURE,
            .deleteFigure = command->createFigure,
        };
    case COMMAND_DELETE_FIGURE:
        return (Command) {
            .type = COMMAND_CREATE_FIGURE,
            .createFigure = command->deleteFigure,
        };
    case COMMAND_CREATE_STICK:
        return (Command) {
            .type = COMMAND_REMOVE_STICK,
            .removeStick = command->createStick,
        };
    case COMMAND_REMOVE_STICK:
        return (Command) {
            .type = COMMAND_CREATE_STICK,
            .createStick = command->removeStick,
        };
    case COMMAND_CHANGE_STICK_DATA:
        return (Command) {
            .type = COMMAND_CHANGE_STICK_DATA,
            .changeStick = command->changeStick,
        };
    case COMMAND_UNKNOWN:
        return (Command) { .type = COMMAND_UNKNOWN };
    }
    return (Command) { .type = COMMAND_UNKNOWN };
}

bool CommandRun(Stickfigure_array_t* figures, const Command* command)  {
    Stickfigure* figure;
    switch (command->type) {
    case COMMAND_CREATE_FIGURE:
        figure = array_insert_Stickfigure(figures, nullptr, command->createFigure.zindex);
        PivotDeserialize(command->createFigure.figure, figure);
        break;
    case COMMAND_DELETE_FIGURE:
        array_remove_Stickfigure(figures, nullptr, command->deleteFigure.zindex);
        break;
    case COMMAND_CREATE_STICK:
        break;
    case COMMAND_REMOVE_STICK:
        break;
    case COMMAND_CHANGE_STICK_DATA:
        break;
    case COMMAND_UNKNOWN:
        break;
    }
    return true;
}