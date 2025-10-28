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


#define DEFINE_ARRAY_TYPE(type)                                                \
    DEFINE_INTERFACE_ARRAY_TYPE(type)                                          \
    DEFINE_IMPLEMENTATION_ARRAY_TYPE(type)                                     \

#define DEFINE_INTERFACE_ARRAY_TYPE(type)                                      \
    typedef struct {                                                           \
        unsigned length;                                                       \
        unsigned capacity;                                                     \
        type *data;                                                            \
    } type ## _array_t;

#define DEFINE_IMPLEMENTATION_ARRAY_TYPE(type)                                 \
    static type *array_append_ ## type(type ## _array_t *array) {              \
        if (array->capacity == 0) {                                            \
            array->data = malloc(ARRAY_MIN_CAPACITY * sizeof(type));           \
            array->capacity = ARRAY_MIN_CAPACITY;                              \
        }                                                                      \
        if (array->capacity < array->length + 1) {                             \
            unsigned capacity = array->capacity * 2;                           \
            void *memory = realloc(array->data, capacity * sizeof(type));      \
            assert(memory != nullptr);                                         \
            array->data = memory;                                              \
            array->capacity *= 2;                                              \
        }                                                                      \
        array->length += 1;                                                    \
        return &array->data[array->length - 1];                                \
    }                                                                          \
                                                                               \
    static type *array_insert_ ## type(                                        \
        type##_array_t *array, type *item, unsigned index                      \
    ) {                                                                        \
        type* last = array_append_ ## type(array);                             \
        memmove(&array->data[index + 1], &array->data[index],                  \
                (array->length - 1 - index) * sizeof(type));                   \
        memcpy(&array->data[index], item, sizeof(type));                       \
        return &array->data[index];                                            \
    }                                                                          \
                                                                               \
    static void array_swap_and_pop_back_ ## type(                              \
        type ## _array_t *array, unsigned index                                \
    ) {                                                                        \
        assert(index < array->length);                                         \
        if (index == array->length - 1) {                                      \
            array->length--;                                                   \
            return;                                                            \
        }                                                                      \
        array->length--;                                                       \
        memcpy(array->data + index, array->data + array->length,               \
               sizeof(type));                                                  \
    }                                                                          \
                                                                               \
    static void array_free_##type(type##_array_t *array) {                     \
        free(array->data);                                                     \
        array->length = 0;                                                     \
        array->capacity = 0;                                                   \
        array->data = nullptr;                                                 \
    }                                                                          \
                                                                               \
    typedef int (*compare_ ## type ##_fn)(const type *a, const type *b);       \
                                                                               \
    static type* array_insert_sorted_ ## type(                                 \
        type ## _array_t* array, compare_ ## type ## _fn fn, type* x           \
    ) {                                                                        \
        if (array->length == 0 ||                                              \
            fn(x, &array->data[array->length - 1]) > 0) {                      \
            type *p = array_append_##type(array);                              \
            memcpy(p, x, sizeof(type));                                        \
            return p;                                                          \
        } else if (fn(x, &array->data[0]) < 0) {                               \
            return array_insert_##type(array, x, 0);                           \
        }                                                                      \
        unsigned a = 0, b = array->length, mid;                                \
        while (a <= b) {                                                       \
            mid = (a + b) / 2;                                                 \
            int cmp = fn(x, &array->data[mid]);                                \
            if (cmp < 0) {                                                     \
                b = mid - 1;                                                   \
            } else if (cmp > 0) {                                              \
                a = mid + 1;                                                   \
            } else {                                                           \
                return array_insert_##type(array, x, mid);                     \
            }                                                                  \
        }                                                                      \
        return array_insert_##type(array, x, mid);                             \
    }                                                                          \
                                                                               \
    static type* array_find_sorted_ ## type(                                   \
        const type ## _array_t* array, compare_ ## type ## _fn fn, type* x     \
    ) {                                                                        \
        if (array->length == 0                                                 \
            || fn(x, &array->data[0]) < 0 ||                                   \
            fn(x, &array->data[array->length - 1]) > 0) {                      \
            return nullptr;                                                    \
        }                                                                      \
        unsigned a = 0, b = array->length, mid;                                \
        while (a <= b) {                                                       \
            mid = (a + b) / 2;                                                 \
            int cmp = fn(x, &array->data[mid]);                                \
            if (cmp < 0) {                                                     \
                b = mid - 1;                                                   \
            } else if (cmp > 0) {                                              \
                a = mid + 1;                                                   \
            } else {                                                           \
                return &array->data[mid];                                      \
            }                                                                  \
        }                                                                      \
        return nullptr;                                                        \
    }

#endif // !ARRAY_H
