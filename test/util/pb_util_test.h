#ifndef PB_UTIL_TEST_H
#define PB_UTIL_TEST_H

#include "../test_util.h"
#include <check.h>

Suite* make_pb_hash_suite(void);
Suite* make_pb_vertex_suite(void);
Suite* make_pb_graph_suite(void);
Suite* make_pb_heap_suite(void);
Suite* make_pb_geom_suite(void);
Suite* make_pb_vector_suite(void);
Suite* make_triangulate_suite(void);

#endif /* PB_UTIL_TEST_H */