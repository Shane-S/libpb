#include "../test_util.h"
#include <check.h>
#include <pb/util/pb_graph.h>

#ifdef _WIN32
#include <malloc.h>
#endif

/**
 * This file contains unit tests for all vertex-related functions in the graph API.
 */
START_TEST(create_no_data)
{
    pb_vertex* vert = pb_vertex_create(NULL);
    ck_assert_msg(vert->data == NULL, "Vertex data wasn't NULL.");
    ck_assert_msg(vert->edges_size == 0, "Vertex adjancency list size wasn't 0.");
    ck_assert_msg(vert->edges_capacity == 2, "Vertex capacity wasn't 2.");

    pb_vertex_free(vert);
}
END_TEST

/**
 * This file contains unit tests for all vertex-related functions in the graph API.
 */
START_TEST(create_with_data)
{
	int *data = malloc(sizeof(int));
	int vert_data;
    pb_vertex* vert = pb_vertex_create(data);
    
    *data = 3;
    vert_data = *((int*)vert->data);
    ck_assert_msg(vert->data == data, "Vertex data wasn't the same as assigned pointer.");
    ck_assert_msg(vert_data == *data, "Vertex data didn't match actual data.");

    pb_vertex_free(vert);
    free(data);
}
END_TEST

START_TEST(add_edge)
{
    pb_vertex *vert = pb_vertex_create(NULL);
    pb_edge edges[] = {
        { vert, (void*)1, 1.f, NULL },
    };

    pb_vertex_add_edge(vert, &edges[0]);
    
    ck_assert_msg(vert->edges_size == 1, "Incorrect adjacency size.");
    ck_assert_msg(vert->edges_capacity == 2, "Incorrect adjacency capacity.");
    ck_assert_msg(vert->edges[0]->from == edges[0].from);
    ck_assert_msg(vert->edges[0]->to == edges[0].to);
    ck_assert_msg(vert->edges[0]->weight == edges[0].weight);
    ck_assert_msg(vert->edges[0]->data == edges[0].data);
    
    pb_vertex_free(vert);
}
END_TEST

START_TEST(edge_list_expansion)
{
    pb_vertex *vert = pb_vertex_create(NULL);
    pb_edge edges[] = {
        { vert, (void*)1, 0.f, NULL },
        { vert, (void*)2, 0.f, NULL },
        { vert, (void*)3, 0.f, NULL },
        { vert, (void*)4, 0.f, NULL },
        { vert, (void*)5, 0.f, NULL }
    };
    int i;

    /* Doesn't really matter what we add; we're just testing the size */
    for(i = 0; i < 5; ++i) {
        pb_vertex_add_edge(vert, &edges[i]);
    }

    ck_assert_msg(vert->edges_size == 5, "Incorrect adjacency size.");
    ck_assert_msg(vert->edges_capacity == 8, "Incorrect adjacency capacity.");
    
	pb_vertex_free(vert);
}
END_TEST

START_TEST(remove_edge)
{
    pb_vertex *vert = pb_vertex_create(NULL);
    pb_edge edges[] = {
        { vert, (void*)1, 1.f, NULL },
    };

    pb_vertex_add_edge(vert, &edges[0]);
    pb_vertex_remove_edge(vert, &edges[0]);

    ck_assert_msg(vert->edges_size == 0, "Incorrect edge list size.");

    pb_vertex_free(vert);
}
END_TEST

START_TEST(remove_nonexistent_edge)
{
	pb_vertex *vert = pb_vertex_create(NULL);

	/* Test removing the fake edge. */
	ck_assert_msg(pb_vertex_remove_edge(vert, NULL) == -1, "Remove of non-existent edge didn't return -1.");
	pb_vertex_free(vert);
}
END_TEST

Suite *make_pb_vertex_suite(void)
{

    /* Life test case tests lifetime events (create and destroy);
     * Adjacency test case tests all functions related to the adjacency list
     */
    Suite *s;
    TCase *tc_life, *tc_adj_list;

    s = suite_create("Vertex");
    
    tc_life = tcase_create("Vertex lifetime functions");
    suite_add_tcase(s, tc_life);
    tcase_add_test(tc_life, create_no_data);
    tcase_add_test(tc_life, create_with_data);

    tc_adj_list = tcase_create("Vertex adjacency list tests");
    suite_add_tcase(s, tc_adj_list);
    tcase_add_test(tc_adj_list, add_edge);
	tcase_add_test(tc_adj_list, edge_list_expansion);
	tcase_add_test(tc_adj_list, remove_edge);
	tcase_add_test(tc_adj_list, remove_nonexistent_edge);
    return s;
}