#pragma once

#include <stddef.h>
#include <pb/util/util_exports.h>

#ifdef __cplusplus
extern "C" {
#endif

static const size_t PB_VECTOR_DEFAULT_CAPACITY = 8;

typedef struct {
    void* items;
    size_t item_size;
    size_t size;
    size_t cap;
} pb_vector;

/**
 * Initialises a pb_vector with the given element size and capacity.
 *
 * @param vec       The vector to initialise.
 * @param item_size The size (in chars, e.g. from sizeof(elem)) of each element in the vector.
 * @param cap       The vector's initial capacity. Pass 0 to use PB_VECTOR_DEFAULT_CAPACITY.
 * @return 0 on success, -1 on out of memory.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_init(pb_vector* vec, size_t item_size, size_t cap);

/**
 * Allocates and initialises a pb_vector.
 *
 * @param item_size The size (in chars, e.g. from sizeof(elem)) of each element in the vector.
 * @param cap       The vector's initial capacity. Pass 0 to use PB_VECTOR_DEFAULT_CAPACITY.
 * @return A pointer to pb_vector on success, NULL on failure (out of memory).
 */
PB_UTIL_DECLSPEC pb_vector* PB_UTIL_CALL pb_vector_create(size_t item_size, size_t cap);

/**
 * Removes the element at i. Note that no bounds-checking is performed.
 *
 * @param vec The vector from which the item will be removed.
 * @param i   The position from which the item will be removed.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vector_remove_at(pb_vector* vec, unsigned i);

/**
 * Inserts an item at the given index.
 * @param vec  The vector into which the item will be inserted.
 * @param item A pointer to an item of size vec->item_size that will be inserted into the vector.
 * @param i    The position at which to insert item. Note that no bounds-checking is performed; the
 *             position must be 0 <= i < vec->size.
 * @return     0 on success, -1 on if out of memory.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_insert_at(pb_vector* vec, void* item, unsigned i);

/**
 * Adds an element to the back of the vector. The element will be copied by value.
 *
 * @param vec  The vector to which the item will be added.
 * @param item The item to add to the vector.
 * @return 0 on success, -1 on out of memory.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_push_back(pb_vector* vec, void* item);

/**
 * Attempts to resize the vector to the new given size. Note that shrinking the vector
 * will make any elements beyond the new capacity inaccessible, so they should be freed
 * if necessary first.
 *
 * @param vec The vector to resize.
 * @param cap The new capacity, which will be multiplied by vec->item_size.
 *
 * @return 0 on success, -1 on failure. If the operation fails, the items pointer
 *         will still be in a valid state, so it can be cleaned up if necessary.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_resize(pb_vector* vec, size_t cap);

/**
 * Reverses the vector vec in-place.
 *
 * @param vec The vector to reverse.
 * @param tmp A pointer to a buffer of size vec->item_size to use as temporary storage in swaps.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vector_reverse_no_alloc(pb_vector* vec, void* tmp);

/**
 * Attempts to reverse the contents of the vector in-place.
 * Note: If the vector is full (vec->size == vec->cap), this will allocate a temporary buffer as
 * scratch space for swapping, so it might fail. Call pb_vector_reverse_no_alloc instead to ensure it
 * doesn't fail.
 *
 * @param vec The vector to reverse.
 *
 * @return 0 on success, -1 if out of memory.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_reverse(pb_vector* vec);

/**
 * Frees the vector's internal list. The pointer to vec itself won't be freed.
 * @param vec The vector for which to free the internal list.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vector_free(pb_vector const* vec);

#ifdef __cplusplus
}
#endif
