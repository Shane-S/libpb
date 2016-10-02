#ifndef PB_HASH_H
#define PB_HASH_H

#include <stdint.h>
#include <stddef.h>
#include <pb/util/pb_util_exports.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum pb_hash_entry_state {
    EMPTY,
    DELETED,
    FULL
} pb_hash_entry_state;

typedef uint32_t (*pb_hash_func)(void const*);

/* A function to compare keys for equality; non-zero return indicates equality. */
typedef int      (*pb_hash_eq_func)(void const*, void const*);

typedef struct pb_hash_entry {
    void* key;   /* The key associated with the given value. */
    void* val;   /* The value stored with this key. */
} pb_hash_entry;

typedef struct pb_hash {
    pb_hash_entry*       entries;    /* A list of entries in the map. */
    pb_hash_entry_state* states;     /* Stores the state (EMPTY, DELETED, FULL) of the corresponding entry in the entries array. */
    size_t               cap;        /* The entry list's capacity. */
	size_t               expand_num; /* The size after which the hash map will expand (determined by load factor). */
    size_t               size;       /* The number of items actually in the list. */
    pb_hash_func         hash;       /* The function for computing a key's hash value. */
    pb_hash_eq_func      key_eq;     /* The function to compare keys for equality. */
} pb_hash;

/* Used to iterate over the hash map's entries. */
typedef void(*pb_hash_iterator_func)(pb_hash_entry* entry, void* param);

/**
 * Iterator function to free data held by map entries. Pass this to pb_hash_for_each to free all data.
 *
 * @param entry    The entry being processed.
 * @param free_key If this is 0, frees entry->val only; if it's non-zero, frees both entry->key and entry->val.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_hash_free_entry_data(pb_hash_entry* entry, void* free_key);

/**
 * Creates a new, empty hash map with the given hash and equals functions.
 * @param hash   A hash function to determine where an item will go in the array.
 * @param key_eq A function that compares two keys for equality.
 * @return An empty hash map with the given hash and equals functions.
 */
PB_UTIL_DECLSPEC pb_hash* PB_UTIL_CALL pb_hash_create(pb_hash_func hash, pb_hash_eq_func key_eq);

/**
 * Frees the given hash map, but not its contents (the actual keys and values).
 *
 * @param map The map to be freed.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_hash_free(pb_hash* map);

/**
 * Places an item in the hash map. If there's already a value associated with the given
 * key, it will be over-written (not freed).
 *
 * @param map The map into which to insert the item.
 * @param key The key to associate with the item.
 * @param val The item to insert into the map.
 * @return Whether it was inserted? I actually don't remember why this returns anything. Maybe it returns -1 on OOM or something
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_hash_put(pb_hash* map, void const* key, void const* val);

/**
 * Gets the value associated with the given key, if any.
 *
 * @param map The map to search for the key.
 * @param key The key for which to search.
 * @param out A variable to hold the associated value on success.
 * @return 0 if the key is not found, 1 if it is.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_hash_get(pb_hash* map, void const* key, void** out);

/**
 * Removes the item associated with the given key from the map (if it's there).
 * 
 * @param map The map from which the item will be removed.
 * @param key The key associated with the item to remove.
 * @return Whether there was actually an item with that key in the map.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_hash_remove(pb_hash* map, void const* key);

/**
 * Calls the specified function on every entry in the hash map.
 *
 * NOTE: Modifying keys will render the map unusable (or at least, if you use the new keys,
 * will likely lead to much worse performance). Only modify keys if you don't intend to use
 * the map again.
 *
 * @param func  The function to call for every entry in the hash map.
 * @param param The (optional) parameter to supply to the given function.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_hash_for_each(pb_hash* map, pb_hash_iterator_func func, void* param);

#ifdef __cplusplus
}
#endif
#endif /* PB_HASH_H */