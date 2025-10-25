#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifndef ARRAY_MIN_CAPACITY
#define ARRAY_MIN_CAPACITY 64
#endif

#ifndef foreach
#define foreach(array, var, type)                                           \
    for(unsigned index = 0; index < (array).length; index = (array).length) \
    for(type* var = &(array).data[index];                                   \
        index < (array).length && (var = &(array).data[index]); index++)
#endif

#ifndef array_indexof
#define array_indexof(array, x) (x - array.data)
#endif

#ifndef EMPTY_ARRAY
#define EMPTY_ARRAY { .length = 0, .capacity = 0, .data = nullptr }
#endif

#define DEFINE_ARRAY_TYPE(type)                                                     \
    typedef struct {                                                                \
        unsigned length;                                                            \
        unsigned capacity;                                                          \
        type* data;                                                                 \
    } type ## _array_t;                                                             \
                                                                                    \
    static inline type* array_append_ ## type(type ## _array_t* array) {            \
        if(array->capacity == 0) {                                                  \
            array->data = malloc(ARRAY_MIN_CAPACITY * sizeof(type));                \
            array->capacity = ARRAY_MIN_CAPACITY;                                   \
        }                                                                           \
        if(array->capacity < array->length + 1) {                                   \
            void* memory = realloc(array->data, array->capacity * 2);               \
            if(memory == nullptr)                                                   \
                return nullptr;                                                     \
            array->data = memory;                                                   \
            array->capacity *= 2;                                                   \
        }                                                                           \
        array->length += 1;                                                         \
        return &array->data[array->length - 1];                                     \
    }                                                                               \
                                                                                    \
    static inline void array_swap_and_pop_back_ ## type(                            \
        type ## _array_t* array, unsigned index                                     \
    ) {                                                                             \
        assert(index < array->length);                                              \
        if(index == array->length - 1) {                                            \
            array->length--;                                                        \
            return;                                                                 \
        }                                                                           \
        array->length--;                                                            \
        memcpy(array->data + index, array->data + array->length, sizeof(type));     \
    }                                                                               \
                                                                                    \
    static inline void array_free_ ## type(type ## _array_t* array) {               \
        free(array->data);                                                          \
        array->length = 0;                                                          \
        array->capacity = 0;                                                        \
        array->data = nullptr;                                                      \
    }


#endif // !ARRAY_H
