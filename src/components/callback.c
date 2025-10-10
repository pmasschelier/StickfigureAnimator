#include "callback.h"

typedef struct {
    CallbackFn fn;
} CallbackSimple_t;

typedef struct  {
    CallbackIndexFn fn;
    unsigned index;
} CallbackIndexed_t;

struct Callback {
    enum {
        CALLBACK_SINGLE,
        CALLBACK_INDEXED,
    } type;
    union {
        CallbackSimple_t simple;
        CallbackIndexed_t indexed;
    };
    void* params;
    Callback_t* then;
};

const size_t SizeofCallback = sizeof(Callback_t);

void CallbackRunAll(Callback_t* cb) {
    if(!cb)
        return;
    switch (cb->type) {
        case CALLBACK_SINGLE:
            if(cb->simple.fn)
                cb->simple.fn(cb->params);
            break;
        case CALLBACK_INDEXED:
            cb->indexed.fn(cb->indexed.index, cb->params);
            break;
    }
    CallbackRunAll(cb->then);
}

Callback_t* CallbackChainGroup(Arena* arena, Callback_t* cb, CallbackIndexFn fn, unsigned count, void* params) {
    Callback_t* ret = arena_allocate(arena, count, sizeof(Callback_t));
    for (unsigned i = 0; i < count; i++) {
        ret[i].type = CALLBACK_INDEXED;
        ret[i].indexed.fn = fn;
        ret[i].indexed.index = i;
        ret[i].params = params;
        ret[i].then = cb;
    }
    return ret;
}

Callback_t* CallbackChainGroupCopyParams(Arena* arena, Callback_t* cb, CallbackIndexFn fn, unsigned count, void* params, size_t paramsSize) {
    void* paramsCopy = arena_allocate(arena, 1, paramsSize);
    memcpy(paramsCopy, params, paramsSize);
    return CallbackChainGroup(arena, cb, fn, count, paramsCopy);
}


Callback_t* CallbackChain(Arena* arena, Callback_t* cb, CallbackFn fn, void* params) {
    Callback_t* ret = arena_allocate(arena, 1, sizeof(Callback_t));
    ret->type = CALLBACK_SINGLE;
    ret->simple.fn = fn;
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
        CallbackRunAll(data->onMouseDown);
        break;
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        CallbackRunAll(data->onMouseUp);
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
