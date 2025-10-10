#ifndef CALLBACK_H
#define CALLBACK_H

#include <string.h>
#include <stdlib.h>
#include "arena.h"
#include "clay/clay.h"

typedef void (*CallbackFn)(void*);
typedef void (*CallbackGroupFn)(unsigned, void*);
typedef struct Callback Callback_t;

typedef struct {
    Callback_t* onMouseDown;
    Callback_t* onMouseUp;
} ButtonData;

#define CallbackCreate(arena, fn, params) CallbackChain(arena, nullptr, fn, params)
#define CallbackCreateCopyParams(arena, fn, params, paramsSize) CallbackChainCopyParams(arena, nullptr, fn, params, paramsSize)

void CallbackRunAll(Callback_t* cb);
Callback_t* CallbackChainGroup(Arena* arena, Callback_t* cb, CallbackGroupFn fn, unsigned count, void* params);
Callback_t* CallbackChainGroupCopyParams(Arena* arena, Callback_t* cb, CallbackGroupFn fn, unsigned count, void* params, size_t paramsSize);
Callback_t* CallbackChain(Arena* arena, Callback_t* cb, CallbackFn fn, void* params);
Callback_t* CallbackChainCopyParams(Arena* arena, Callback_t* cb, CallbackFn fn, void* params, size_t paramsSize);
void SetButtonCallbacks(Arena* arena, ButtonData data);


#endif // !CALLBACK_H
