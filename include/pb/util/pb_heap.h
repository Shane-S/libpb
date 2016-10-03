#ifndef PB_HEAP_H
#define PB_HEAP_H

#include <stddef.h>
#include <pb/util/pb_util_exports.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PB_HEAP_DEFAULT_CAP 4

/**
 * Function comparing two items. Returns a positive value if param1 is larger,
 * negative if param1 is smaller, and 0 if the parameters are equal.
 */
typedef int (*pb_heap_cmp)(void*, void*);

/**
 * A min-heap containing a list of partially sorted items.
 */
typedef struct {
    size_t      size;  /* The number of items in the heap. */
    size_t      cap;   /* The heap's capacity. */
    pb_heap_cmp cmp;   /* The comparison function to sort items. */
    void**      items; /* The list of items contained in the heap. */
} pb_heap;

/**
 * Creates and initialises a heap with an optional initial capacity.
 * @param pb_heap_cmp The comparison function to use in this heap.
 * @param init_cap    The initial capacity for this heap.
 *                    If 0, the heap will be initialised with PB_HEAP_DEFAULT_CAP.
 * @return An initialised heap on success, NULL on failure (out of memory).
 */
PB_UTIL_DECLSPEC pb_heap* PB_UTIL_CALL pb_heap_create(pb_heap_cmp cmp, size_t init_cap);

/**
 * Deallocates the heap. Note that the elements in the heap won't be freed.
 * @param heap The heap to free.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_heap_free(pb_heap* heap);

/**
 * Adds the given item to the heap.
 * @param heap The heap to which the item should be added.
 * @param item The item to add to the heap.
 * @return Non-zero on success, 0 on failure (out of memory).
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_heap_insert(pb_heap* heap, void* item);

/**
 * Retrieves the item with the lowest priority in the heap but does not remove it.
 *
 * @param heap The heap from which to retrieve the item.
 * @return The item with the lowest priority or NULL if the heap is empty.
 */
PB_UTIL_DECLSPEC void* PB_UTIL_CALL pb_heap_peek_min(pb_heap* heap);

/**
 * Retrieves the item with the lowest priority from the heap and removes it,
 * causing the next-lowest-priority item to take its place.
 *
 * @param heap The heap from which to remove the item.
 * @return The item with the lowest priority or NULL if the heap is empty.
 */
PB_UTIL_DECLSPEC void* PB_UTIL_CALL pb_heap_get_min(pb_heap* heap);

#ifdef __cplusplus
}
#endif
#endif /* PB_HEAP_H */