#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pb/util/pb_hash_utils.h>

#include <pb/util/MurmurHash3.h>

static uint32_t murmur_seed = 0xDEADBEEF;

uint32_t pb_murmurhash3(void const* key, size_t len) {
    uint32_t hash;
    MurmurHash3_x86_32(key, len, murmur_seed, (void*)&hash);
    return hash;
}

uint32_t pb_str_hash(void const* key) {
    size_t len = strlen((char*)key);
    return pb_murmurhash3(key, len);
}

int pb_str_eq(void const* str1, void const* str2) {
    return strcmp((char*)str1, (char*)str2) == 0;
}

uint32_t pb_pointer_hash(void const* key) {
    return (uint32_t)key;
}

int pb_pointer_eq(void const* p1, void const* p2) {
    return p1 == p2;
}