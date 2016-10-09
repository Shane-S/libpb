#include <pb/util/pb_hashmap.h>
#include <stdlib.h>

#define LOAD_FACTOR 0.75f

/**
 * See the StackOverflow answer here: http://stackoverflow.com/questions/4475996/given-prime-number-n-compute-the-next-prime
 */
static const uint32_t small_primes[] =
{
    2,
    3,
    5,
    7,
    11,
    13,
    17,
    19,
    23,
    29
};

static const uint32_t indices[] =
{
    1,
    7,
    11,
    13,
    17,
    19,
    23,
    29
};

static int is_prime(uint32_t x)
{
    const uint32_t N = sizeof(small_primes) / sizeof(small_primes[0]);
    for (uint32_t i = 3; i < N; ++i)
    {
        const uint32_t p = small_primes[i];
        const uint32_t q = x / p;
        if (q < p)
            return 1;
        if (x == q * p)
            return 0;
    }
    for (uint32_t i = 31; 1;)
    {
        uint32_t q = x / i;
        if (q < i)
            return 1;
        if (x == q * i)
            return 0;
        i += 6;

        q = x / i;
        if (q < i)
            return 1;
        if (x == q * i)
            return 0;
        i += 4;

        q = x / i;
        if (q < i)
            return 1;
        if (x == q * i)
            return 0;
        i += 2;

        q = x / i;
        if (q < i)
            return 1;
        if (x == q * i)
            return 0;
        i += 4;

        q = x / i;
        if (q < i)
            return 1;
        if (x == q * i)
            return 0;
        i += 2;

        q = x / i;
        if (q < i)
            return 1;
        if (x == q * i)
            return 0;
        i += 4;

        q = x / i;
        if (q < i)
            return 1;
        if (x == q * i)
            return 0;
        i += 6;

        q = x / i;
        if (q < i)
            return 1;
        if (x == q * i)
            return 0;
        i += 2;
    }
    return 1;
}

static uint32_t* lower_bound(uint32_t const* start, uint32_t const* end, uint32_t num) {
	uint32_t const* pos;
	for (pos = start; pos != end; ++pos) {
		if (num <= *pos) return pos;
	}
	return end - 1;
}

static uint32_t next_prime(uint32_t n)
{
    const uint32_t L = 30;
    const uint32_t N = sizeof(small_primes) / sizeof(small_primes[0]);

    /* If n is small enough, search in small_primes */
	if (n <= small_primes[N - 1]) {
		return *lower_bound(small_primes, small_primes + N, n);
	} else {
		/* Else n > largest small_primes */
		/* Start searching list of potential primes: L * k0 + indices[in] */
		const uint32_t M = sizeof(indices) / sizeof(indices[0]);

		/* Select first potential prime >= n */
		/*   Known a-priori n >= L */
		uint32_t k0 = n / L;
		uint32_t in = lower_bound(indices, indices + M, n - k0 * L) - indices;
		n = L * k0 + indices[in];
		while (!is_prime(n))
		{
			if (++in == M)
			{
				++k0;
				in = 0;
			}
			n = L * k0 + indices[in];
		}
		return n;
	}
}

PB_UTIL_DECLSPEC pb_hashmap* PB_UTIL_CALL pb_hashmap_create(pb_hash_func hash, pb_hash_eq_func key_eq) {
    pb_hashmap* map = malloc(sizeof(pb_hashmap));
    int i;
    
    if(!map) {
        return NULL;
    }
    
    map->entries = NULL;
    map->states = NULL;
    
    map->entries = malloc(sizeof(pb_hashmap_entry) * 7);
    if(!map->entries) goto err_return;
    
    map->states = malloc(sizeof(pb_hashmap_entry_state) * 7);
    if(!map->states) goto err_return;
    
    for(i = 0; i < 7; ++i) {
        map->states[i] = EMPTY;
    }
    
    map->cap = 7;
    map->expand_num = (uint32_t)(map->cap * LOAD_FACTOR);
    map->size = 0;
    map->hash = hash;
    map->key_eq = key_eq;
    
    return map;

err_return:
    free(map->entries);
    free(map->states);
    free(map);
    return NULL;
}

void pb_hashmap_free(pb_hashmap* map) {
    free(map->entries);
    free(map->states);
    free(map);
}

/**
 * Resizes the hash map to the given size, copying the keys to the new map. 
 *
 * @param map      The map to resize.
 * @param new_size The new size for the map.
 * @return 0 if the map could not be successfully resized (due to failed memory allocation), 1 on success.
 */
static int resize_hash(pb_hashmap* map, size_t new_cap) {
    size_t               cur_cap = map->cap;
    size_t               cur_size = map->size;
    pb_hashmap_entry*       cur_entries = map->entries;
    pb_hashmap_entry_state* cur_states = map->states;
    size_t  i;
    
    /* Try to allocate a new array to contain the list of values */
    map->cap = new_cap;
    map->size = 0; /* We're calling pb_hashmap_put, so need to 0 size */
    map->entries = malloc(map->cap * sizeof(pb_hashmap_entry)); 
    map->states = calloc(map->cap, sizeof(pb_hashmap_entry_state));
    
    if(!map->entries || !map->states) {
        free(map->entries);
        free(map->states);
        map->entries = cur_entries;
        map->states = cur_states;
        map->cap = cur_cap;
        return -1;
    }
    
    /* Loop over the entries, moving all elements to the new list */
    for(i = 0; i < cur_cap; ++i) {
        if(cur_states[i] == FULL) {
            pb_hashmap_put(map, cur_entries[i].key, cur_entries[i].val);
        }
    }

    free(cur_entries);
    free(cur_states);
    return 0;
}

/**
 * Probes the hash map for the given key and returns its position if found.
 *
 * @param map The map to search.
 * @param key The key for which to search.
 * @param out A variable to hold the position (if any).
 * @return -1 if the key wasn't found or the position if it was .
 */
static size_t get_pos(pb_hashmap* map, void const* key) {
    size_t pos = map->hash(key);
    size_t probe_pos;
    size_t i;
        
    for(probe_pos = pos, i = 0; i < map->cap; ++probe_pos, ++i) {
        probe_pos %= map->cap;

        if(map->states[probe_pos] == EMPTY) {
            return -1;
        } else if(map->key_eq(map->entries[probe_pos].key, key)) {
            if (map->states[probe_pos] == FULL) {
                return probe_pos;
            } else {
                return -1;
            }
        }
    }
        
    return -1;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_hashmap_put(pb_hashmap* map, void const* key, void const* val) {
    size_t pos;
    size_t probe_pos;
    size_t i;
    
    /* If the map already contains the key, update its associated value */
    if((pos = get_pos(map, key)) != -1) {
        map->entries[pos].val = val;
        return 0;
    }
    
    /* Expand if necessary */
    if(map->size == map->expand_num) {
        size_t next_cap = next_prime((uint32_t)(map->cap / LOAD_FACTOR));
        map->size++;
        map->expand_num = (uint32_t)(next_cap * LOAD_FACTOR);
        
        if(resize_hash(map, next_cap) == -1) {
            map->size--;
            return -1;
        }
    }
        
    /* Try to insert it in the next available spot */
    pos = map->hash(key);    
    for(probe_pos = pos, i = 0; i < map->cap; ++probe_pos, ++i) {
        probe_pos %= map->cap;
        if(map->states[probe_pos] != FULL) {
            map->entries[probe_pos].key = key;
            map->entries[probe_pos].val = val;
            map->states[probe_pos] = FULL;
            map->size++;
            return 0;
        }
    }

    /* SHOULD NEVER GET HERE (I guess?)*/
    return -1;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_hashmap_get(pb_hashmap* map, void const* key, void** val) {
    size_t pos;
    if((pos = get_pos(map, key, &pos)) == -1) return -1;
    
    *val = map->entries[pos].val;
    return 0;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_hashmap_remove(pb_hashmap* map, void const* key) {
    size_t pos;
    if ((pos = get_pos(map, key, &pos)) == -1) return -1;
    
    map->states[pos] = DELETED;
    map->size--;
    
    /* TODO: Resize to smaller map to save memory where necessary */
    return 0;
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_hashmap_for_each(pb_hashmap* map, pb_hash_iterator_func func, void* param) {
    size_t i;
    for (i = 0; i < map->cap; ++i) {
        if (map->states[i] == FULL) {
            func(map->entries + i, param);
        }
    }
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_hashmap_free_entry_data(pb_hashmap_entry* entry, void* free_key) {
    free(entry->val);
    if (free_key) {
        free(entry->key);
    }
}