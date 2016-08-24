#include <libcompat.h>
#include <check.h>
#include <pb/internal/pb_graph.h>

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
START_TEST(create_with_data)
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

START_TEST(adj_list_equality)
{
	/*
	 *    void *data;
     *    pb_vertex **adjacent;
	 *    size_t *edge_weights;
	 *    size_t adj_size;
	 *    size_t adj_capacity;
	 */

    pb_vertex neighbours[2] = 
	{ {1, 3, 5, 0, 2},
	  {2, 4, 6, 0, 2} };
    size_t i;

    pb_vertex vert = {0};
    vert.adjacent = malloc(sizeof(pb_vertex*) * 2);
	vert.edge_weights = malloc(sizeof(size_t) * 2);
    vert.adj_capacity = 2;

    for(i = 0; i < 2; i++) {
        pb_vertex_add_edge(&vert, &neighbours[i], 1);
    }

	/* This is probably kind of a bogus test. Oh well */
    for(i = 0; i < 2; i++) {
        ck_assert_msg(vert.adjacent[i]->data == neighbours[i].data, "Neighbour data isn't equal.");
		ck_assert_msg(vert.adjacent[i]->adjacent == neighbours[i].adjacent, "Neighbour adjacency list pointer isn't equal.");
		ck_assert_msg(vert.adjacent[i]->edge_weights == neighbours[i].edge_weights, "Neighbour edge weights list isn't equal.");
		ck_assert_msg(vert.adjacent[i]->adj_size == neighbours[i].adj_size, "Neighbour adjacency list size isn't equal.");
		ck_assert_msg(vert.adjacent[i]->adj_capacity == neighbours[i].adj_capacity, "Neighbour adjacency list capacity isn't equal.");
    }

	free(vert.adjacent);
	free(vert.edge_weights);
}
END_TEST

START_TEST(adj_list_expansion)
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

START_TEST(remove_edge)
{
	pb_vertex *vert = pb_vertex_create(NULL);
	pb_vertex *fake = (pb_vertex*)1;

	/* Test removing the fake edge. */
	pb_vertex_add_edge(vert, fake, 1);
	pb_vertex_remove_edge(vert, fake);

	ck_assert_msg(vert->adj_capacity == 2, "Vertex adjacency list capacity should be 2, was %lu.", vert->adj_capacity);
	ck_assert_msg(vert->adj_size == 0, "Vertex adjacency list size should be 0, was %lu.", vert->adj_size);

	pb_vertex_free(vert, 0);
}
END_TEST

START_TEST(remove_non_existent_edge)
{
	pb_vertex *vert = pb_vertex_create(NULL);
	pb_vertex *fake = (pb_vertex*)1;

	/* Test removing the fake edge. */
	ck_assert_msg(pb_vertex_remove_edge(vert, fake) == -1, "Remove of non-existent edge didn't return -1.");
	ck_assert_msg(vert->adj_capacity == 2, "Vertex adjacency list capacity should be 2, was %lu.", vert->adj_capacity);
	ck_assert_msg(vert->adj_size == 0, "Vertex adjacency list size should be 0, was %lu.", vert->adj_size);

	pb_vertex_free(vert, 0);
}
END_TEST

START_TEST(get_edge_weight)
{
	pb_vertex *vert = pb_vertex_create(NULL);
	pb_vertex *fake = (pb_vertex*)1;
	int res;
	size_t weight;
	/* Test removing the fake edge. */
	pb_vertex_add_edge(vert, fake, 1);
	
	res = pb_vertex_get_weight(vert, fake, &weight);
	ck_assert_msg(res == 0, "Return value should have been 0, got %d.", res);
	ck_assert_msg(weight == 1, "Weight should have been 1, got %lu.", weight);

	pb_vertex_free(vert, 0);
}
END_TEST

START_TEST(get_nonexistent_edge_weight)
{
	pb_vertex *vert = pb_vertex_create(NULL);
	pb_vertex *fake = (pb_vertex*)1;
	int res;
	size_t weight;

	res = pb_vertex_get_weight(vert, fake, &weight);
	ck_assert_msg(res == -1, "Return value should have been -1, got %d.", res);
	pb_vertex_free(vert, 0);
}
END_TEST

Suite *make_pb_vertex_suite(void)
{

    /* Life test case tests lifetime events (create and destroy);
     * Adjacency test case tests all functions related to the adjacency list
     */
    Suite *s;
    TCase *tc_life, *tc_adj_list, *tc_edge_weights;

    s = suite_create("Vertex");
    
    tc_life = tcase_create("Vertex lifetime functions");
    suite_add_tcase(s, tc_life);
    tcase_add_test(tc_life, create_no_data);
    tcase_add_test(tc_life, create_with_data);

    tc_adj_list = tcase_create("Vertex adjacency list tests");
    suite_add_tcase(s, tc_adj_list);
    tcase_add_test(tc_adj_list, adj_list_equality);
	tcase_add_test(tc_adj_list, adj_list_expansion);
	tcase_add_test(tc_adj_list, remove_edge);
	tcase_add_test(tc_adj_list, remove_non_existent_edge);
    
	tc_edge_weights = tcase_create("Vertex edge weight functions");
	suite_add_tcase(s, tc_edge_weights);
	tcase_add_test(tc_edge_weights, get_edge_weight);
	tcase_add_test(tc_edge_weights, get_nonexistent_edge_weight);
    return s;
}