#ifndef COMPONENTS_UTILS_H
#define COMPONENTS_UTILS_H

#include "arena.h"
#include "clay/clay.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern bool clickable_hovered;

typedef void (*OnHoverFn)(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
typedef void (*CallbackFn)(void*);

typedef struct Callback Callback_t;
struct Callback{
    CallbackFn fn;
    void* params;
    Callback_t* then;
};

static inline void RunCallbackAll(Callback_t* cb) {
    if(!cb)
        return;
    if(cb->fn)
        cb->fn(cb->params);
    RunCallbackAll(cb->then);
}

#define CallbackCreate(arena, fn, params) CallbackChain(arena, nullptr, fn, params)
#define CallbackCreateCopyParams(arena, fn, params, paramsSize) CallbackChainCopyParams(arena, nullptr, fn, params, paramsSize)

static inline Callback_t* CallbackChain(Arena* arena, Callback_t* cb, CallbackFn fn, void* params) {
    Callback_t* ret = arena_allocate(arena, 1, sizeof(Callback_t));
    ret->fn = fn;
    ret->params = params;
    ret->then = cb;
    return ret;
}

static inline Callback_t* CallbackChainCopyParams(Arena* arena, Callback_t* cb, CallbackFn fn, void* params, size_t paramsSize) {
    void* paramsCopy = arena_allocate(arena, 1, paramsSize);
    memcpy(paramsCopy, params, paramsSize);
    return CallbackChain(arena, cb, fn, paramsCopy);
}

typedef struct {
    unsigned item_index;
    unsigned item_count;
} ItemData;

typedef struct {
    Callback_t* onMouseDown;
    Callback_t* onMouseUp;
} ButtonData;

static void HandleHoverFunction(
    [[maybe_unused]] Clay_ElementId elementId,
    Clay_PointerData pointerInfo,
    intptr_t userData
) {
    ButtonData *data = (void *)userData;
    switch (pointerInfo.state) {
    case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
        RunCallbackAll(data->onMouseDown);
        break;
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        RunCallbackAll(data->onMouseUp);
        break;
    default:
        break;
    }
}

static inline Callback_t* CallbackDeepCopy(Arena* arena, Callback_t* other) {
    Callback_t* current = nullptr;
    Callback_t* previous = nullptr;
    while(other) {
        current = arena_allocate(arena, 1, sizeof(ButtonData));
        if(!current)
            exit(-3);
        current->fn = other->fn;
        current->params = other->params;
        if(previous)
            previous->then = current;
        previous = current;
        other = other->then;
    }
    return current;
}


static inline void SetButtonCallbacks(Arena* arena, ButtonData data) {
    ButtonData* button_data = arena_allocate(arena, 1, sizeof(ButtonData));
    if(!button_data)
        exit(-3);
    // button_data->onMouseDown = CallbackDeepCopy(arena, data.onMouseDown);
    // button_data->onMouseUp = CallbackDeepCopy(arena, data.onMouseUp);
    memcpy(button_data, &data, sizeof(ButtonData));
    Clay_OnHover(HandleHoverFunction, (intptr_t)button_data);
}


#endif // !COMPONENTS_UTILS_H
