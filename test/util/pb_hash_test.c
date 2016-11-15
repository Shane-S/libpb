#include "../test_util.h"
#include <pb/util/hashmap/hashmap.h>
#include <pb/util/hashmap/hash_utils.h>

/* Hash map to use for the tests. */
static pb_hashmap* map;

void pb_hash_test_teardown() {
    pb_hashmap_free(map);
}

/**
 * Creates an empty hash map and ensures that everything is correctly initialised.
 */
START_TEST(create_test)
{
    unsigned int i;
    map = pb_hashmap_create(pb_str_hash, pb_str_eq);

    ck_assert_msg(map->cap == 7, "Map's capacity was %u, should be 7.", map->cap);
    ck_assert_msg(map->size == 0, "Map's size should be 0, was %u", map->size);
    
    for(i = 0; i < map->cap; ++i)
        ck_assert_msg(map->states[i] == EMPTY, "Map entry wasn't initially EMPTY.");
    
    ck_assert_msg(map->hash == pb_str_hash, "Map's hash function wasn't initialised correctly.");
    ck_assert_msg(map->key_eq == pb_str_eq, "Map's key equality function wasn't initialised correctly.");
    
    ck_assert_msg(map->expand_num == 5, "Map's expansion threshold was %u, should be 5", map->expand_num);
}
END_TEST

/**
 * Inserts a string-int pair into the hash map.
 */
START_TEST(put_test)
{
    pb_hashmap_put(map, (void*)"test", (void*)((size_t)4));
    
    /* TODO: Change states[0] to the actual hashed value. */
    ck_assert_msg(map->size == 1, "Map's size should be 1 after insertion, was %u.", map->size);
}
END_TEST

/**
 * Inserts a string-int pair into the hash map.
 */
START_TEST(get_test)
{
    size_t test_num;
    pb_hashmap_get(map, (void*)"test", (void**)&test_num);
    ck_assert_msg(test_num == 4, "Number corresponding to \"test\" should be 4, was %d", test_num);
}
END_TEST

/**
 * Overwrites the value associated with the previously inserted string-int pair.
 */
START_TEST(overwrite_test)
{
    size_t overwritten;
    pb_hashmap_put(map, (void*)"test", (void*)((size_t)7));
    
    pb_hashmap_get(map, (void*)"test", (void**)&overwritten);
    ck_assert_msg(overwritten == 7, "Value should be 7, was %d", overwritten);
}
END_TEST

/**
 * Removes the previously inserted string-int pair.
 */
START_TEST(remove_test)
{
    pb_hashmap_remove(map, (void*)"test");
    
    ck_assert_msg(map->size == 0, "Map's size should be 0, was %u", map->size);
}
END_TEST

/**
 * Inserts enough items to exceed the load factor and tests expansion.
 */
START_TEST(expand_test)
{
    int all_contained;
    int out;
    
    /* Load factor is hard-coded to 0.75, and since we're starting at 7, 6 will definitely exceed this */
    pb_hashmap_put(map, (void*)"test0", (void*)((size_t)0));
    pb_hashmap_put(map, (void*)"test1", (void*)((size_t)1));
    pb_hashmap_put(map, (void*)"test2", (void*)((size_t)2));
    pb_hashmap_put(map, (void*)"test3", (void*)((size_t)3));
    
    /* Check that the map hasn't yet expanded since we haven't reached the threshold */
    ck_assert_msg(map->cap == 7, "Capacity should have been 7, was %u", map->cap);

    /* Check that the map did expand now that we've exceeded the threshold */
    pb_hashmap_put(map, (void*)"test4", (void*)((size_t)4));
    pb_hashmap_put(map, (void*)"test5", (void*)((size_t)5));
    ck_assert_msg(map->cap == 11 /* Next prime >= 1.5 * cap */, "Capacity should have been 11, was %u", map->cap);
    ck_assert_msg(map->size == 6, "Map's size should have been 6, was %u", map->size);

    all_contained = pb_hashmap_get(map, (void*)"test0", (void**)&out) == 0 &&
		            pb_hashmap_get(map, (void*)"test1", (void**)&out) == 0 &&
                    pb_hashmap_get(map, (void*)"test2", (void**)&out) == 0 &&
                    pb_hashmap_get(map, (void*)"test3", (void**)&out) == 0 &&
                    pb_hashmap_get(map, (void*)"test4", (void**)&out) == 0 &&
                    pb_hashmap_get(map, (void*)"test5", (void**)&out) == 0;

    ck_assert_msg(all_contained, "Values were not properly transferred to new array.");
}
END_TEST

static void add_one_to_entries(pb_hashmap_entry* entry, void* param) {
    *(size_t*)(entry->val) += 1;
}

START_TEST(for_each_test)
{
    char const* keys[] = { "one", "two", "three" };
    size_t values[] = { 1, 2, 3 };
    size_t expected[] = { 2, 3, 4 };
    int i;
    pb_hashmap* map2 = pb_hashmap_create(pb_str_hash, pb_str_eq);
    
    for (i = 0; i < 3; ++i) {
        pb_hashmap_put(map2, (void*)keys[i], (void*)(&values[i]));
    }

    pb_hashmap_for_each(map2, add_one_to_entries, NULL);

    for (i = 0; i < 3; ++i) {
        ck_assert_msg(values[i] == expected[i], "values[%d} should have been %d, was %d", i, expected[i], values[i]);
    }

    pb_hashmap_free(map2);
}
END_TEST

Suite *make_pb_hash_suite(void)
{
	Suite *s;
	TCase *tc_hashmap;

	s = suite_create("Hash map");

    tc_hashmap = tcase_create("Hash map basic operations");
	suite_add_tcase(s, tc_hashmap);
	tcase_add_test(tc_hashmap, create_test);
	tcase_add_test(tc_hashmap, put_test);
	tcase_add_test(tc_hashmap, get_test);
    tcase_add_test(tc_hashmap, overwrite_test);
    tcase_add_test(tc_hashmap, remove_test);
    tcase_add_test(tc_hashmap, expand_test);
    tcase_add_test(tc_hashmap, for_each_test);

    tcase_add_unchecked_fixture(tc_hashmap, NULL, pb_hash_test_teardown);
    
	return s;
}
