#ifndef CALLBACK_H
#define CALLBACK_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "arena.h"
#include "clay/clay.h"

typedef void (*CallbackFn)(void*);
typedef void (*CallbackIndexFn)(unsigned, void*);
typedef struct Callback Callback_t;

extern const size_t SizeofCallback;
#define CALLBACK_INDEX(cb, index) (Callback_t*)((char*)cb + i * SizeofCallback)

typedef struct {
    Callback_t* onMouseDown;
    Callback_t* onMouseUp;
} ButtonData;

#define CallbackCreate(arena, fn, params) CallbackChain(arena, nullptr, fn, params)
#define CallbackCreateCopyParams(arena, fn, params, paramsSize) CallbackChainCopyParams(arena, nullptr, fn, params, paramsSize)
#define CallbackCreateGroup(arena, fn, count, params) CallbackChainGroup(arena, nullptr, fn, count, params)
#define CallbackCreateGroupCopyParams(arena, fn, count, params, paramsSize) CallbackChainGroupCopyParams(arena, nullptr, fn, count, params, paramsSize)

void CallbackRunAll(Callback_t* cb);
Callback_t* CallbackChainGroup(Arena* arena, Callback_t* cb, CallbackIndexFn fn, unsigned count, void* params);
Callback_t* CallbackChainGroupCopyParams(Arena* arena, Callback_t* cb, CallbackIndexFn fn, unsigned count, void* params, size_t paramsSize);
Callback_t* CallbackChain(Arena* arena, Callback_t* cb, CallbackFn fn, void* params);
Callback_t* CallbackChainCopyParams(Arena* arena, Callback_t* cb, CallbackFn fn, void* params, size_t paramsSize);
void SetButtonCallbacks(Arena* arena, ButtonData data);


#endif // !CALLBACK_H
