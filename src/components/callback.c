#include "callback.h"

typedef struct {
    CallbackFn fn;
} CallbackSingle_t;

typedef struct  {
    CallbackGroupFn fn;
    unsigned count;
} CallbackGroup_t;

struct Callback {
    enum {
        CALLBACK_SINGLE,
        CALLBACK_GROUP,
    } type;
    union {
        CallbackSingle_t single;
        CallbackGroup_t group;
    };
    void* params;
    Callback_t* then;
} ;

void RunCallbackAll(Callback_t* cb) {
    if(!cb)
        return;
    switch (cb->type) {
        case CALLBACK_SINGLE:
            if(cb->single.fn)
                cb->single.fn(cb->params);
            break;
        case CALLBACK_GROUP:
            for (unsigned i = 0; i < cb->group.count; i++)
                cb->group.fn(i, cb->params);
            break;
    }
    RunCallbackAll(cb->then);
}

Callback_t* CallbackChainGroup(Arena* arena, Callback_t* cb, CallbackGroupFn fn, unsigned count, void* params) {
    Callback_t* ret = arena_allocate(arena, 1, sizeof(Callback_t));
    ret->type = CALLBACK_GROUP;
    ret->group.fn = fn;
    ret->group.count = count;
    ret->params = params;
    ret->then = cb;
    return ret;
}

Callback_t* CallbackChainGroupCopyParams(Arena* arena, Callback_t* cb, CallbackGroupFn fn, unsigned count, void* params, size_t paramsSize) {
    void* paramsCopy = arena_allocate(arena, 1, paramsSize);
    memcpy(paramsCopy, params, paramsSize);
    return CallbackChainGroup(arena, cb, fn, count, paramsCopy);
}


Callback_t* CallbackChain(Arena* arena, Callback_t* cb, CallbackFn fn, void* params) {
    Callback_t* ret = arena_allocate(arena, 1, sizeof(Callback_t));
    ret->type = CALLBACK_SINGLE;
    ret->single.fn = fn;
    ret->params = params;
    ret->then = cb;
    return ret;
}

Callback_t* CallbackChainCopyParams(Arena* arena, Callback_t* cb, CallbackFn fn, void* params, size_t paramsSize) {
    void* paramsCopy = arena_allocate(arena, 1, paramsSize);
    memcpy(paramsCopy, params, paramsSize);
    return CallbackChain(arena, cb, fn, paramsCopy);
}

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

void SetButtonCallbacks(Arena* arena, ButtonData data) {
    ButtonData* button_data = arena_allocate(arena, 1, sizeof(ButtonData));
    if(!button_data)
        exit(-3);
    memcpy(button_data, &data, sizeof(ButtonData));
    Clay_OnHover(HandleHoverFunction, (intptr_t)button_data);
}
