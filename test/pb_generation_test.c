#include <libcompat.h>
#include <check.h>
#include <pb/pb_generation.h>

#ifdef _WIN32
#include <malloc.h>
#endif

/**
 * This file contains unit tests for all vertex-related functions in the graph API.
 */
START_TEST(generate_simple_single_floor)
{
    /* Do nothing for now */
}
END_TEST

Suite *make_pb_generation_suite(void)
{

    Suite *s;
    TCase *tc_generate_single;

    s = suite_create("Generation");
    
    tc_generate_single = tcase_create("Test cases for generating a single floor");
    suite_add_tcase(s, tc_generate_single);
    tcase_add_test(tc_generate_single, generate_simple_single_floor);
    return s;
}