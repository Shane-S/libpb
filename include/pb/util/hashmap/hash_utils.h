#ifndef PB_HASH_UTILS
#define PB_HASH_UTILS

#include <pb/util/util_exports.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A wrapper around the MurmurHash3 hash function to simplify use of the hash table.
 * The seed is generated at program start-up.
 *
 * @param key The key for which to generate a hash value.
 * @param len The length (in bytes) of the given key.
 * @return The hash value for the key.
 */
PB_UTIL_DECLSPEC uint32_t PB_UTIL_CALL pb_murmurhash3(void const* key, size_t len);

/**
 * Generates a hash value for a null-terminated string.
 * @param key The string to hash.
 * @return The hash value for the given string.
 */
PB_UTIL_DECLSPEC uint32_t PB_UTIL_CALL pb_str_hash(void const* key);

/**
 * Wrapper around strcmp for use with pb_hash.
 *
 * @param str1 The first null-terminated string.
 * @param str2 The second null-terminated string.
 * @return Whether str1 and str2 are equal (non-zero return) or not equal (zero).
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_str_eq(void const* str1, void const* str2);

/**
 * Generates a hash value for a pointer. This really just returns the pointer cast to uint32_t.
 *
 * @param key The pointer to hash.
 * @return key cast to uint32_t.
 */
PB_UTIL_DECLSPEC uint32_t PB_UTIL_CALL pb_pointer_hash(void const* key);

/**
 * Compares two pointers for equality.
 * @param p1 The first pointer.
 * @param p2 The second pointer.
 * @return Whether p1 == p2.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_pointer_eq(void const* p1, void const* p2);

#ifdef __cplusplus
}
#endif
#endif /* PB_HASH_UTILS */
