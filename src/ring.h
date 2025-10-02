#ifndef RING_H

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
        const unsigned pop = ring->end;                                 \
        ring->end = next;                                               \
        return &ring->data[pop];                                        \
    }                                                                   \
                                                                        \
    static bool ring_pop_ ## type(type* popped) {                       \
        if(ring->begin == ring->next)                                   \
            return false;                                               \
        memcpy(popped, ring->data[ring->begin], sizeof(type));          \
        ring->begin = (ring->begin + 1) % ring->capacity;               \
        return true;                                                    \
    }

#endif // !RING_H
