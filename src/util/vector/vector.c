#include <pb/util/vector/vector.h>
#include <stdlib.h>
#include <string.h>

static const float PB_VECTOR_GROWTH_RATE = 1.5;

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_init(pb_vector* vec, size_t item_size, size_t cap) {
    void* items;
    cap = cap == 0 ? PB_VECTOR_DEFAULT_CAPACITY : cap;
    
    items = malloc(item_size * cap);
    if (!items) return -1;

    vec->items = items;
    vec->cap = cap;
    vec->size = 0;
    vec->item_size = item_size;

    return 0;
}

PB_UTIL_DECLSPEC pb_vector* PB_UTIL_CALL pb_vector_create(size_t item_size, size_t cap) {
    pb_vector* vec = malloc(sizeof(pb_vector));
    if (!vec) return NULL;

    if (pb_vector_init(vec, item_size, cap) == -1) return NULL;

    return vec;
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vector_remove_at(pb_vector* vec, unsigned i) {
    unsigned char* items = (unsigned char*)vec->items;
    unsigned char* prev = items + (vec->item_size * i);
    unsigned char* start = prev + vec->item_size;
    unsigned char* end = items + (vec->item_size * vec->size);
    unsigned char* cur;

    /* Shift all elements past i down by one */
    for (cur = start; cur != end; cur += vec->item_size) {
        memcpy(prev, cur, vec->item_size);
        prev = cur;
    }

    vec->size--;
}

static int pb_vector_expand(pb_vector* vec, size_t new_cap) {
    void* new_items = realloc(vec->items, new_cap * vec->item_size);
    if (!new_items) return -1;
    vec->items = new_items;
    return 0;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_insert_at(pb_vector* vec, void* item, unsigned i) {
    unsigned char* items;
    unsigned char* prev;
    unsigned char* start;
    unsigned char* end;
    unsigned char* cur;

    if (vec->size == vec->cap) {
        size_t new_cap = (size_t)(vec->cap * PB_VECTOR_GROWTH_RATE);
        if (pb_vector_expand(vec, new_cap) == -1) {
            return -1;
        }
        vec->cap = new_cap;
    }

    items = (unsigned char*)vec->items;
    end = items + (vec->item_size * vec->size);
    start = items + (vec->item_size * i);

    /* Shift items i through the end of the vector up by one */
    for (cur = end; cur > start; cur -= vec->item_size) {
        prev = cur - vec->item_size;
        memcpy(cur, prev, vec->item_size);
    }
    
    /* Copy the item into its position */
    memcpy(start, item, vec->item_size);
    vec->size++;

    return 0;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_push_back(pb_vector* vec, void* item) {   
    return pb_vector_insert_at(vec, item, vec->size);
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vector_reverse_no_alloc(pb_vector* vec, void* tmp) {
    unsigned char* front;
    unsigned char* back;
    unsigned char* items = vec->items;

    for (front = items, back = items + (vec->item_size * (vec->size - 1)); front < back; front += vec->item_size, back -= vec->item_size) {
        memcpy(tmp, front, vec->item_size);
        memcpy(front, back, vec->item_size);
        memcpy(back, tmp, vec->item_size);
    }
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_reverse(pb_vector* vec) {
    unsigned char* tmp;
    unsigned char* items = (unsigned char*)vec->items;
    
    /* No space; use a temporary buffer */
    if (vec->size == vec->cap) {
        tmp = malloc(vec->item_size);
        if (!tmp) {
            return -1;
        } else {
            pb_vector_reverse_no_alloc(vec, tmp);
            free(tmp);
            return 0;
        }
    }

    /* There's some space at the end of the vector; use that as scratch space */
    tmp = items + (vec->item_size * vec->size);
    pb_vector_reverse_no_alloc(vec, tmp);
    return 0;
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vector_free(pb_vector const* vec) {
    free(vec->items);
}