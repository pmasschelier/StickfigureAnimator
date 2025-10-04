#ifndef RING_H
#define RING_H

#include <stdlib.h>
#include <string.h>

#define DEFINE_RING_TYPE(type)                                          \
    typedef struct {                                                    \
        unsigned capacity;                                              \
        unsigned begin;                                                 \
        unsigned next;                                                  \
        type* data;                                                     \
    } type ## _ring_t;                                                  \
                                                                        \
    static type ## _ring_t ring_init_ ## type(unsigned capacity) {      \
        return (type ## _ring_t) {                                      \
            capacity, 0, 0,                                             \
            malloc(capacity * sizeof(type))                             \
        };                                                              \
    }                                                                   \
                                                                        \
    static type* ring_tail_ ## type(type ## _ring_t* ring) {            \
        if(ring->begin == ring->next)                                   \
            return nullptr;                                             \
        return &ring->data[ring->begin];                                \
    }                                                                   \
                                                                        \
    static void ring_deinit_ ## type(type ## _ring_t* ring) {           \
        free(ring->data);                                               \
        ring->capacity = 0;                                             \
        ring->begin = 0;                                                \
        ring->next = 0;                                                 \
    }                                                                   \
                                                                        \
    static type* ring_push_ ## type(type ## _ring_t* ring) {            \
        const unsigned next = (ring->next + 1) % ring->capacity;        \
        if(ring->begin == next)                                         \
            return NULL;                                                \
        const unsigned pop = ring->next;                                \
        ring->next = next;                                              \
        return &ring->data[pop];                                        \
    }                                                                   \
                                                                        \
    static bool ring_pop_ ## type(type ## _ring_t* ring, type* popped) {\
        if(ring->begin == ring->next)                                   \
            return false;                                               \
        if(popped != nullptr)                                           \
            memcpy(popped, &ring->data[ring->begin], sizeof(type));     \
        ring->begin = (ring->begin + 1) % ring->capacity;               \
        return true;                                                    \
    }

#endif // !RING_H
