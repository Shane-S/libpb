#include <libcompat.h>
#include <check.h>
#include <pb/internal/pb_heap.h>
#include <pb/pb_types.h>
#include "test_util.h"

/* Heap to use for the tests. */
pb_heap* heap;

void pb_heap_test_teardown() {
    pb_heap_free(heap);
}

int int_cmp(void* int1, void* int2) {
    return (int)int1 - (int)int2;
}

START_TEST(create_default_test)
{
    pb_heap* default_heap = pb_heap_create(int_cmp, 0);
    ck_assert_msg(default_heap->cap == PB_HEAP_DEFAULT_CAP, "Default heap's capacity was %d, should be %d.", default_heap->cap, PB_HEAP_DEFAULT_CAP);
    ck_assert_msg(default_heap->size == 0, "Default heap's size was %u, should be 0.", default_heap->size);
    
    pb_heap_free(default_heap);
}
END_TEST

/**
 * Creates an empty heap and ensures that everything is correctly initialised.
 */
START_TEST(create_with_size_test)
{
    /* Global variable used as fixture */
    heap = pb_heap_create(int_cmp, 10);
    ck_assert_msg(heap->cap == 10, "Heap's capacity was %d, should be %d.", heap->cap, 10);
    ck_assert_msg(heap->size == 0, "Heap's size was %u, should be 0.", heap->size);
}
END_TEST

START_TEST(empty_get_test)
{
    void* test1 = pb_heap_peek_min(heap);
    void* test2 = pb_heap_get_min(heap);
    ck_assert_msg(test1 == NULL, "pb_heap_peek_min didn't return NULL for empty heap.");
    ck_assert_msg(test2 == NULL, "pb_heap_get_min didn't return NULL for empty heap.");
}
END_TEST

/**
 * Inserts some integers into the heap and checks that the minimum is correct.
 */
START_TEST(insert_test)
{
    int item;
    pb_heap_insert(heap, (void*)6);
    pb_heap_insert(heap, (void*)5);
    pb_heap_insert(heap, (void*)4);
    pb_heap_insert(heap, (void*)9);
    item = (int)pb_heap_peek_min(heap);
    
    ck_assert_msg(heap->size == 4, "Heap's size should be 1 after insertion, was %u.", heap->size);
    ck_assert_msg(item == 4, "Min item should have been 4, was %d", item);
}
END_TEST

/**
 * Retrieves and removes the minimum element and checks that the heap's size and new minimum element
 * are correct.
 */
START_TEST(get_test)
{
    int old_min = (int)pb_heap_get_min(heap);
    int new_min = (int)pb_heap_peek_min(heap);
    ck_assert_msg(old_min == 4, "Min should have been 4, was %d", old_min);
    ck_assert_msg(new_min == 5, "New min should have been 5, was %d", new_min);
    ck_assert_msg(heap->size == 3, "Heap's size should have been 3, was %u", heap->size);
}
END_TEST

/**
 * Inserts and new minimum element and checks that it's returned by pb_heap_peek_min.
 */
START_TEST(insert_smaller_test)
{
    int new_min;
    pb_heap_insert(heap, (void*)2);
    new_min = (int)pb_heap_peek_min(heap);
    ck_assert_msg(new_min == 2, "New minimum value in heap should be 2, was %d", new_min);
}
END_TEST

/**
 * Inserts enough items to exceed the heap's capacity and cause it to expand.
 */
START_TEST(expand_test)
{
    pb_heap_insert(heap, (void*)1);
    pb_heap_insert(heap, (void*)2);
    pb_heap_insert(heap, (void*)3);
    pb_heap_insert(heap, (void*)4);
    pb_heap_insert(heap, (void*)5);
    pb_heap_insert(heap, (void*)6);
    pb_heap_insert(heap, (void*)113);
    ck_assert_msg(heap->cap == 20, "Heap's capacity was %u, should have been 20.", heap->cap);
}
END_TEST

Suite *make_pb_heap_suite(void)
{
	Suite *s;
	TCase *tc_heap;

	s = suite_create("Heap");

	tc_heap = tcase_create("Heap basic operations");
	suite_add_tcase(s, tc_heap);
    tcase_add_test(tc_heap, create_default_test);
	tcase_add_test(tc_heap, create_with_size_test);
	tcase_add_test(tc_heap, empty_get_test);
    tcase_add_test(tc_heap, insert_test);
    tcase_add_test(tc_heap, get_test);
    tcase_add_test(tc_heap, insert_smaller_test);
    tcase_add_test(tc_heap, expand_test);

    tcase_add_unchecked_fixture(tc_heap, NULL, pb_heap_test_teardown);
    
	return s;
}
