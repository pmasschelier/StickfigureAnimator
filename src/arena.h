#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef uintptr_t ArenaOffset;

typedef struct {
  ArenaOffset nextAllocation;
  size_t capacity;
  char *memory;
  bool reallocOnOverflow;
} Arena;

static inline Arena arena_init(size_t capacity, void *memory) {
    return (Arena){0, capacity, (char *)memory, false};
}

static inline Arena arena_create(size_t capacity) {
    return (Arena){ 0, capacity, malloc(capacity), true};
}

static inline void arena_free(Arena* arena) {
    free(arena->memory);
    arena->nextAllocation = 0;
    arena->capacity = 0;
    arena->memory = nullptr;
}

static inline void arena_reset(Arena *arena) { arena->nextAllocation = 0; }

static inline ArenaOffset arena_push_frame(Arena* arena) {
    return arena->nextAllocation;
}
static inline void arena_pop_frame(Arena* arena, ArenaOffset frame) {
    arena->nextAllocation = frame;
}

#define ARENA_NEXT_ALLOC_ALIGNED(arena, align) ((arena)->nextAllocation + ((align) - ((arena)->nextAllocation % (align))))

static inline void *arena_allocate(Arena *arena, unsigned count, size_t size) {
    const size_t totalSizeBytes = count * size;
    ArenaOffset nextAllocOffset = ARENA_NEXT_ALLOC_ALIGNED(arena, 64);
    size_t minCapacity = nextAllocOffset + totalSizeBytes;
    if (arena->capacity < minCapacity) {
        if(!arena->reallocOnOverflow)
            return nullptr;
        arena->capacity = 2 * minCapacity;
        arena->memory = realloc(arena->memory, arena->capacity);
    }
    arena->nextAllocation = nextAllocOffset + totalSizeBytes;
    return arena->memory + nextAllocOffset;
}

#endif // !ARENA_H
