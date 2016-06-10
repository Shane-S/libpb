#ifndef PB_HASH_UTILS
#define PB_HASH_UTILS

#include <pb/pb_exports.h>
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
PB_DECLSPEC uint32_t PB_CALL pb_murmurhash3(void* key, int len);

/**
    * Generates a hash value for a null-terminated string.
    * @param key The string to hash.
    * @return The hash value for the given string.
    */
PB_DECLSPEC uint32_t PB_CALL pb_str_hash(void* key);

/**
    * Wrapper around strcmp for use with pb_hash.
    *
    * @param str1 The first null-terminated string.
    * @param str2 The second null-terminated string.
    * @return Whether str1 and str2 are equal (non-zero return) or not equal (zero).
    */
PB_DECLSPEC int PB_CALL pb_str_eq(void* str1, void* str2);

#ifdef __cplusplus
}
#endif
#endif /* PB_HASH_UTILS */