#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uintptr_t nextAllocation;
  size_t capacity;
  char *memory;
} Arena;

static inline Arena arena_init(size_t capacity, void *memory) {
  return (Arena){0, capacity, (char *)memory};
}

static inline void *arena_allocate(Arena *arena, unsigned count, size_t size) {
  const size_t totalSizeBytes = count * size;
  uintptr_t nextAllocOffset =
      arena->nextAllocation + (64 - (arena->nextAllocation % 64));
  void *ret = nullptr;
  if (nextAllocOffset + totalSizeBytes <= arena->capacity) {
    ret = arena->memory + nextAllocOffset;
    arena->nextAllocation = nextAllocOffset + totalSizeBytes;
  }
  return ret;
}

static inline void arena_reset(Arena *arena) { arena->nextAllocation = 0; }

#endif // !ARENA_H
