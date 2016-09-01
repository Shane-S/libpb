#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pb/util/pb_hash_utils.h>

#include <pb/util/MurmurHash3.h>

static uint32_t murmur_seed = 0;

uint32_t pb_murmurhash3(void* key, int len) {
	if (!murmur_seed) {
		murmur_seed = (uint32_t)time(NULL);
	}

    uint32_t hash;
    MurmurHash3_x86_32(key, len, murmur_seed, (void*)&hash);
    return hash;
}

uint32_t pb_str_hash(void* key) {
    int len = strlen((char*)key);
    return pb_murmurhash3(key, len);
}

int pb_str_eq(void* str1, void* str2) {
    return strcmp((char*)str1, (char*)str2) == 0;
}