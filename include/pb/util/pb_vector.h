#ifndef PB_VECTOR_H
#define PB_VECTOR_H
#include <stddef.h>
#include <pb/util/pb_util_exports.h>

static const size_t PB_VECTOR_DEFAULT_CAPACITY = 16;

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
 * @return A pointer to pb_vector on success, NULL on failure.
 */
PB_UTIL_DECLSPEC pb_vector* PB_UTIL_CALL pb_vector_create(size_t item_size, size_t cap);

/**
 * Retrieves the element at i. Note that no bounds-checking is performed.
 *
 * @param vec The vector from which to retrieve an item.
 * @param i   The item's position in vec.
 * @return    The item at vec[i].
 */
PB_UTIL_DECLSPEC void* PB_UTIL_CALL pb_vector_get_at(pb_vector* vec, unsigned i);

/**
 * Removes the element at i. Note that no bounds-checking is performed.
 *
 * @param vec The vector from which the item will be removed.
 * @param i   The position from which the item will be removed.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vector_remove_at(pb_vector* vec, unsigned i);

/**
 * Adds an element to the back of the vector. The element will be copied by value.
 *
 * @param vec  The vector to which the item will be added.
 * @param item The item to add to the vector.
 * @return 0 on success, -1 on out of memory.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vector_push_back(pb_vector* vec, void* item);

/**
 * Frees the vector's internal list. The pointer to vec itself won't be freed.
 * @param vec The vector for which to free the internal list.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vector_free(pb_vector const* vec);

#endif /* PB_VECTOR_H */