#include <libcompat.h>
#include <check.h>
#include <pb/internal/pb_graph.h>

#ifdef _WIN32
#include <malloc.h>
#endif

/**
 * This file contains unit tests for all vertex-related functions in the graph API.
 */
START_TEST(vert_create_no_data)
{
    pb_vertex* vert = pb_vertex_create(NULL);
    ck_assert_msg(vert->data == NULL, "Vertex data wasn't NULL.");
    ck_assert_msg(vert->adjacent != NULL, "Vertex adjacency was NULL.");
    ck_assert_msg(vert->adj_size == 0, "Vertex adjancency list size wasn't 0.");
    ck_assert_msg(vert->adj_capacity == 2, "Vertex capacity wasn't 2.");
    
    free(vert->adjacent);
	free(vert->edge_weights);
    free(vert);
}
END_TEST

/**
 * This file contains unit tests for all vertex-related functions in the graph API.
 */
START_TEST(vert_create_with_data)
{
	int *data = malloc(sizeof(int));
	int vert_data;
    pb_vertex* vert = pb_vertex_create(data);
    
    *data = 3;
    vert_data = *((int*)vert->data);
    ck_assert_msg(vert->data == data, "Vertex data wasn't the same as assigned pointer.");
    ck_assert_msg(vert_data == *data, "Vertex data didn't match actual data.");
    
    free(vert->adjacent);
	free(vert->edge_weights);
    free(vert);
    free(data);
}
END_TEST

START_TEST(vert_adj_list_equality)
{
    pb_vertex neighbours[2] = {0, 1};
    size_t i;

    pb_vertex vert = {0};
    vert.adjacent = malloc(sizeof(pb_vertex*) * 2);
	vert.edge_weights = malloc(sizeof(size_t) * 2);
    vert.adj_capacity = 2;

    for(i = 0; i < 2; i++) {
        pb_vertex_add_edge(&vert, &neighbours[i], 1);
    }

    //for(i = 0; i < 2; i++) {
    //    ck_assert_msg(vert.adjacent[i], neighbours[i]) == 0, "Adjacency list isn't equal."); /* Test actual neighbour content */
    //}

	free(vert.adjacent);
	free(vert.edge_weights);
}
END_TEST

START_TEST(vert_adj_list_expansion)
{
    pb_vertex neighbour;
    pb_vertex *vert = pb_vertex_create(NULL);
    int i;

    /* Doesn't really matter what we add; we're just testing the size */
    for(i = 0; i < 5; ++i) {
        pb_vertex_add_edge(vert, &neighbour, 1);
    }

    ck_assert_msg(vert->adj_size == 5, "Incorrect adjacency size.");
    ck_assert_msg(vert->adj_capacity == 8, "Incorrect adjacency capacity.");
    
	pb_vertex_free(vert, 0);
}
END_TEST

START_TEST(vert_adj_list_impossible_size)
{
    pb_vertex vert = {0};
	size_t alloc_size;

#ifdef _WIN32
	alloc_size = 0xfffdefff / 2; /* This was the ACTUAL max size that Windows broke at, so we're just going to have to hard-code it. */
#else
	alloc_size = 0x7FFFFFFFFFFFFFF;
#endif

    /* Manually initialise vertex */
    vert.adjacent = malloc(sizeof(pb_vertex*));
	vert.edge_weights = (malloc(sizeof(size_t) * 2));
	vert.adj_capacity = alloc_size;
	vert.adj_size = alloc_size;
    
    ck_assert_msg(pb_vertex_add_edge(&vert, NULL, 1) == -1, "Invalid memory allocation succeeded.");

	free(vert.adjacent);
	free(vert.edge_weights);
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
    tcase_add_test(tc_life, vert_create_no_data);
    tcase_add_test(tc_life, vert_create_with_data);

    tc_adj_list = tcase_create("Vertex adjacency list");
    suite_add_tcase(s, tc_adj_list);
    tcase_add_test(tc_adj_list, vert_adj_list_equality);
    tcase_add_test(tc_adj_list, vert_adj_list_expansion);
    tcase_add_test(tc_adj_list, vert_adj_list_impossible_size);
    
    return s;
}