#ifndef PB_HEAP_H
#define PB_HEAP_H

#include <stddef.h>
#include <pb/util/pb_util_exports.h>
#include <pb/util/pb_vector.h>
#include <pb/util/pb_hashmap.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PB_HEAP_DEFAULT_CAP 4

typedef struct {
    void* data;
    float priority;
} pb_heap_entry;

/**
 * A min-heap containing a list of partially sorted items.
 */
typedef struct {
    pb_vector   items;     /* The items contained in the heap */
    pb_hashmap* index_map; /* A map of items=>indices for quicker decrease-key operation */
} pb_heap;

/**
 * Creates and initialises a heap with an optional initial capacity.
 * @param init_cap    The initial capacity for this heap.
 *                    If 0, the heap will be initialised with PB_HEAP_DEFAULT_CAP.
 * @return An initialised heap on success, NULL on failure (out of memory).
 */
PB_UTIL_DECLSPEC pb_heap* PB_UTIL_CALL pb_heap_create(size_t init_cap);

/**
 * Deallocates the heap. Note that the elements in the heap won't be freed.
 * @param heap The heap to free.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_heap_free(pb_heap* heap);

/**
 * Adds the given item to the heap.
 * @param heap     The heap to which the item should be added.
 * @param item     The item to add to the heap.
 * @param priority The item's priority.
 * @return 0 on success, -1 on failure if out of memory.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_heap_insert(pb_heap* heap, void* item, float priority);

/**
 * Decreases the priority of the given item to new_priority.
 *
 * @param heap         The heap containing the item to decrease.
 * @param key          The item whose priority should be lowered.
 * @param new_priority The new priority to assign to item. Precondition: this is < the current priority.
 * @return 0 on success, -1 if the item wasn't found in the heap.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_heap_decrease_key(pb_heap* heap, void* item, float new_priority);

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