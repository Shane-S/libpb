#include "../test_util.h"
#include <check.h>
#include <pb/util/pb_heap.h>

/* Heap to use for the tests. */
pb_heap* heap;

void pb_heap_test_setup(void) {
    heap = pb_heap_create(10);
}

void pb_heap_test_teardown(void) {
    pb_heap_free(heap);
}

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
    pb_heap_insert(heap, (void*)6, 6.f);
    pb_heap_insert(heap, (void*)5, 5.f);
    pb_heap_insert(heap, (void*)4, 4.f);
    pb_heap_insert(heap, (void*)9, 9.f);
    item = (int)pb_heap_peek_min(heap);
    
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
    ck_assert_msg(heap->items.size == 3, "Heap's size should have been 3, was %u", heap->items.size);
}
END_TEST

/**
 * Inserts and new minimum element and checks that it's returned by pb_heap_peek_min.
 */
START_TEST(insert_smaller_test)
{
    int new_min;
    pb_heap_insert(heap, (void*)2, 2.f);
    new_min = (int)pb_heap_peek_min(heap);
    ck_assert_msg(new_min == 2, "New minimum value in heap should be 2, was %d", new_min);
}
END_TEST

/**
 * Inserts enough items to exceed the heap's capacity and cause it to expand.
 */
START_TEST(expand_test)
{
    pb_heap_insert(heap, (void*)1, 1.f);
    pb_heap_insert(heap, (void*)3, 3.f);
    pb_heap_insert(heap, (void*)7, 7.f);
    pb_heap_insert(heap, (void*)8, 8.f);
    pb_heap_insert(heap, (void*)10, 10.f);
    pb_heap_insert(heap, (void*)113, 113.f);
    ck_assert_msg(heap->items.size == 10, "Heap's size was %u, should have been 10.", heap->items.size);
}
END_TEST

START_TEST(decrease_test)
{
    void* min;

    /* This really makes no sense now, but who cares */
    pb_heap_decrease_key(heap, (void*)113, 0.5f);
    min = pb_heap_peek_min(heap);

    ck_assert_msg(min == (void*)113, "Heap should have had new min of 0x71 (113) after decrease_key, had %p", min);
}
END_TEST

Suite *make_pb_heap_suite(void)
{
	Suite *s;
	TCase *tc_heap;

	s = suite_create("Heap");

	tc_heap = tcase_create("Heap basic operations");
	suite_add_tcase(s, tc_heap);
	tcase_add_test(tc_heap, empty_get_test);
    tcase_add_test(tc_heap, insert_test);
    tcase_add_test(tc_heap, get_test);
    tcase_add_test(tc_heap, insert_smaller_test);
    tcase_add_test(tc_heap, expand_test);
    tcase_add_test(tc_heap, decrease_test);

    tcase_add_unchecked_fixture(tc_heap, pb_heap_test_setup, pb_heap_test_teardown);
    
	return s;
}
