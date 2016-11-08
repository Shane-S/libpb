#include "../test_util.h"
#include <check.h>
#include <pb/util/pb_vector.h>

typedef struct _test_struct {
    int x;
    int y;
} test_struct;

START_TEST(create_sizes)
{
    pb_vector vec;

    pb_vector_init(&vec, sizeof(int), 0);
    
    ck_assert_msg(vec.cap == PB_VECTOR_DEFAULT_CAPACITY, "vec.cap should have been PB_VECTOR_DEFAULT_CAPCITY (%lu), was %lu", PB_VECTOR_DEFAULT_CAPACITY, vec.cap);
    ck_assert_msg(vec.size == 0, "vec.size should have been 0, was %lu", vec.size);
    ck_assert_msg(vec.item_size == sizeof(int), "vec.item_size should have been %lu, was %lu", sizeof(int), vec.item_size);

    pb_vector_free(&vec);

    pb_vector_init(&vec, sizeof(int), 10);
    ck_assert_msg(vec.cap == 10, "vec.cap should have been 10, was %lu", vec.cap);
    pb_vector_free(&vec);
}
END_TEST

START_TEST(push_back_ints)
{
    int items[] = { 1, 2, 3 };
    pb_vector vec;
    int* vec_ints;
    int i;

    pb_vector_init(&vec, sizeof(int), 0);

    for (i = 0; i < 3; ++i) {
        pb_vector_push_back(&vec, &items[i]);
    }
    ck_assert_msg(vec.size == 3, "vec.size should have been 3, was %lu", vec.size);
    
    vec_ints = (int*)vec.items;
    for (i = 0; i < 3; ++i) {
        int val = vec_ints[i];
        ck_assert_msg(val == items[i], "vec[%d] should have been %d, was %d", i, items[i], val);
    }

    pb_vector_free(&vec);
}
END_TEST

START_TEST(push_back_structs)
{
    test_struct items[] = {
        {0, 1},
        {2, 3},
        {4, 5}
    };
    pb_vector vec;
    test_struct* vec_structs;
    int i;

    pb_vector_init(&vec, sizeof(test_struct), 0);

    for (i = 0; i < 3; ++i) {
        pb_vector_push_back(&vec, &items[i]);
    }

    vec_structs = (test_struct*)vec.items;
    for (i = 0; i < 3; ++i) {
        test_struct* ts = vec_structs + i;
        ck_assert_msg(ts->x == items[i].x && ts->y == items[i].y, "vec[%d] should have been {%d, %d}, was {%d, %d}", i, items[i].x, items[i].y, ts->x, ts->y);
    }

    pb_vector_free(&vec);
}
END_TEST

START_TEST(expand)
{
    int items[] = { 1, 2, 3, 4, 5, 6 };
    pb_vector vec;
    int i;

    pb_vector_init(&vec, sizeof(int), 4);

    for (i = 0; i < 4; ++i) {
        pb_vector_push_back(&vec, &items[i]);
    }
    ck_assert_msg(vec.cap == 4, "vec.cap should have been 4, was %lu", vec.cap);

    for (i = 4; i < 6; ++i) {
        pb_vector_push_back(&vec, &items[i]);
    }
    ck_assert_msg(vec.cap > 4, "vec.cap should have been > 4, was %lu", vec.cap);
    pb_vector_free(&vec);
}
END_TEST

START_TEST(remove_at)
{
    int items[] = { 1, 2, 3 };
    pb_vector vec;
    int* vec_ints;
    int i;

    pb_vector_init(&vec, sizeof(int), 3);

    for (i = 0; i < 3; ++i) {
        pb_vector_push_back(&vec, &items[i]);
    }

    pb_vector_remove_at(&vec, 0);
    ck_assert_msg(vec.size == 2, "vec.size should have been 2, was %lu", vec.size);

    vec_ints = (int*)vec.items;
    for (i = 0; i < vec.size; ++i) {
        int val = vec_ints[i];
        ck_assert_msg(val == items[i + 1], "vec[%d] should have been %d, was %d", i, items[i + 1], val);
    }

    pb_vector_free(&vec);
}
END_TEST

START_TEST(insert_at)
{
    int items[] = { 1, 2, 3 };
    int expected[] = { 0, 1, 2, 3 };
    pb_vector vec;
    int* vec_ints;
    int i;
    int to_insert = 0;

    pb_vector_init(&vec, sizeof(int), 3);

    for (i = 0; i < 3; ++i) {
        pb_vector_push_back(&vec, &items[i]);
    }

    pb_vector_insert_at(&vec, &to_insert, 0);

    ck_assert_msg(vec.size == 4, "vec.size should have been 4, was %lu", vec.size);
    vec_ints = (int*)vec.items;
    for (i = 0; i < vec.size; ++i) {
        int val = vec_ints[i];
        ck_assert_msg(val == expected[i], "vec[%d] should have been %d, was %d", i, expected[i], val);
    }

    pb_vector_free(&vec);
}
END_TEST

START_TEST(reverse)
{
    int items[] = { 1, 2, 3, 4 };
    int expected[] = { 4, 3, 2, 1 };
    int i;
    pb_vector vec;
    int* vec_ints;

    pb_vector_init(&vec, sizeof(int), 0);
    for (i = 0; i < 4; ++i) {
        pb_vector_push_back(&vec, &items[i]);
    }

    pb_vector_reverse_no_alloc(&vec, &i);
    vec_ints = (int*)vec.items;
    for (i = 0; i < 4; ++i) {
        int val = vec_ints[i];
        ck_assert_msg(val == expected[i], "vec[%d} should have been %d, was %d", i, expected[i], val);
    }

    pb_vector_free(&vec);
}
END_TEST
Suite* make_pb_vector_suite(void) {
    Suite* s = suite_create("pb_vector suite");
    TCase* tc_vector_tests;

    tc_vector_tests = tcase_create("pb_vector tests");
    suite_add_tcase(s, tc_vector_tests);
    tcase_add_test(tc_vector_tests, create_sizes);
    tcase_add_test(tc_vector_tests, push_back_ints);
    tcase_add_test(tc_vector_tests, push_back_structs);
    tcase_add_test(tc_vector_tests, expand);
    tcase_add_test(tc_vector_tests, remove_at);
    tcase_add_test(tc_vector_tests, insert_at);
    tcase_add_test(tc_vector_tests, reverse);

    return s;
}