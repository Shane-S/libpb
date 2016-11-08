#include "../test_util.h"
#include <check.h>
#include <pb/pb_floor_plan.h>

#ifdef _WIN32
#include <malloc.h>
#endif

/**
 * This file contains unit tests for all vertex-related functions in the graph API.
 */
START_TEST(extrude_simple_single_floor)
{
    /* Do nothing for now */
}
END_TEST

Suite *make_pb_extrusion_suite(void)
{

    Suite *s;
    TCase *tc_extrude_single;

    s = suite_create("Extrusion");
    
    tc_extrude_single = tcase_create("Single-floor extrusion tests");
    suite_add_tcase(s, tc_extrude_single);
    tcase_add_test(tc_extrude_single, extrude_simple_single_floor);
    return s;
}