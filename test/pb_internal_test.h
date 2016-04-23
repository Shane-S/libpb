#ifndef PB_INTERNAL_TEST_H
#define PB_INTERNAL_TEST_H

#include <libcompat.h>
#include <check.h>

Suite *make_pb_vertex_suite(void);
Suite *make_pb_graph_suite(void);
Suite *make_pb_squarify_suite(void);
#endif /* PB_INTERNAL_TEST_H */