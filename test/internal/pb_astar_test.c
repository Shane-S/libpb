#include "../test_util.h"
#include <check.h>
#include <pb/util/pb_graph.h>
#include <pb/internal/pb_astar.h>
#include <pb/util/pb_hash_utils.h>
#include <stdint.h>

uint32_t int_hash(void* num) {
    int num_i = *((int*)num);
    return num_i;
}

int int_eq(void const* num1, void const* num2) {
    int num1_i = *((int*)num1);
    int num2_i = *((int*)num2);

    return num1_i == num2_i;
}

/* We're really running Dijkstra's now, but I'm testing for correctness, not performance */
float bad_heuristic(pb_vertex const* v1, pb_vertex const* v2) {
    return 0.f;
}

START_TEST(astar_simple)
{
    int vert_ids[] = { 0, 1, 2, 3, 4, 5 };
    int* first_id = &vert_ids[0];
    pb_graph* graph = pb_graph_create(int_hash, int_eq);

    pb_vector* path;
    int* expected[] = { first_id, first_id + 1, first_id + 4, first_id + 5 };
    pb_vertex** path_verts;

    unsigned i;
    for (i = 0; i < 6; ++i) {
        pb_graph_add_vertex(graph, first_id + i, first_id + i);
    }

    /* Graph looks like this (the edge weights don't correspond to the lengths at all, but you get the point)
     *          (4)
     *        2 / \ 1
     *         /   \
     *       (1)---(5) 
     *       /   4   \
     *      /         \
     *   3 /           \ 1
     *    /             \
     *   /   2        5  \
     * (0)-------(2)-----(3)
     */

    /* 0 <-> 2 */
    pb_graph_add_edge(graph, first_id, first_id + 2, 2.f, NULL);
    pb_graph_add_edge(graph, first_id + 2, first_id, 2.f, NULL);
    
    /* 0 <-> 1 */
    pb_graph_add_edge(graph, first_id, first_id + 1, 3.f, NULL);
    pb_graph_add_edge(graph, first_id + 1, first_id, 3.f, NULL);

    /* 2 <-> 3 */
    pb_graph_add_edge(graph, first_id + 2, first_id + 3, 5.f, NULL);
    pb_graph_add_edge(graph, first_id + 3, first_id + 2, 5.f, NULL);

    /* 3 <-> 5 */
    pb_graph_add_edge(graph, first_id + 3, first_id + 5, 1.f, NULL);
    pb_graph_add_edge(graph, first_id + 5, first_id + 3, 1.f, NULL);

    /* 1 <-> 5 */
    pb_graph_add_edge(graph, first_id + 1, first_id + 5, 4.f, NULL);
    pb_graph_add_edge(graph, first_id + 5, first_id + 1, 4.f, NULL);

    /* 1 <-> 4 */
    pb_graph_add_edge(graph, first_id + 1, first_id + 4, 2.f, NULL);
    pb_graph_add_edge(graph, first_id + 4, first_id + 1, 2.f, NULL);

    /* 4 <-> 5 */
    pb_graph_add_edge(graph, first_id + 4, first_id + 5, 1.f, NULL);
    pb_graph_add_edge(graph, first_id + 5, first_id + 4, 1.f, NULL);

    ck_assert_msg(pb_astar(pb_graph_get_vertex(graph, first_id), pb_graph_get_vertex(graph, first_id + 5), bad_heuristic, &path) == 0, "No path found.");
    ck_assert_msg(path->size == 4, "Path should have contained 4 vertices, had %lu", path->size);

    path_verts = (pb_vertex**)path->items;
    for (i = 0; i < 4; ++i) {
        ck_assert_msg(path_verts[i]->data == expected[i], "Vertex %u in the path should have had data %p, but had %p", i, expected[i], path_verts[i]->data);
    }

    pb_graph_free(graph);
    pb_vector_free(path);
}
END_TEST

START_TEST(astar_no_path)
{
    pb_graph* graph = pb_graph_create(int_hash, int_eq);
    pb_vector* path;

    int ids[] = { 0, 1, 2 };
    int* first_id = &ids[0];

    unsigned i;
    for (i = 0; i < 3; ++i) {
        pb_graph_add_vertex(graph, first_id + i, first_id + i);
    }

    pb_graph_add_edge(graph, first_id, first_id + 1, 1.f, NULL);
    pb_graph_add_edge(graph, first_id + 1, first_id, 1.f, NULL);

    ck_assert_msg(pb_astar(pb_graph_get_vertex(graph, first_id), pb_graph_get_vertex(graph, first_id + 2), bad_heuristic, &path) == -1, "Shouldn't have found a path but did anyway.");

    pb_graph_free(graph);
}
END_TEST

Suite *make_pb_astar_suite(void)
{
    Suite *s;
    TCase *tc_astar;

    s = suite_create("A* Path-Finding");

    tc_astar = tcase_create("A* correct output");
    suite_add_tcase(s, tc_astar);
    tcase_add_test(tc_astar, astar_simple);
    tcase_add_test(tc_astar, astar_no_path);

    return s;
}