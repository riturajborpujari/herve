#ifndef __D_ARRAY__H__
#define __D_ARRAY__H__

#include <stdlib.h>

struct d_array {
    char** items;
    ssize_t size;
    ssize_t capacity;
};

int d_array_init(int size, struct d_array *arr) {
    arr->items    = malloc(size * sizeof(char*));
    arr->size     = 0;
    arr->capacity = size;

    return 0;
}

int d_array_append(struct d_array *arr, char *item) {
    if (arr->size == arr->capacity) {
        arr->items = realloc(arr->items, (arr->capacity + 16) * sizeof(char*));
        arr->capacity += 16;
    }

    arr->items[arr->size] = item;
    arr->size++;

    return 0;
}

int d_array_free(struct d_array *arr) {
    free(arr->items);
    return 0;
}

int d_array_clear(struct d_array *arr) {
    arr->size = 0;
}


#endif // __D_ARRAY__H__
