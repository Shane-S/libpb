#ifndef PB_INTERNAL_TEST_H
#define PB_INTERNAL_TEST_H

#include "../test_util.h"
#include <check.h>

Suite* make_pb_squarify_suite(void);
Suite* make_pb_sq_house_layout_suite(void);
Suite* make_pb_sq_house_graph_suite(void);
Suite* make_pb_astar_suite(void);

#endif /* PB_INTERNAL_TEST_H */