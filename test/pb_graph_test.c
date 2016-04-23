#include <libcompat.h>
#include <check.h>
#include <pb/internal/pb_graph.h>

/* We're going to assume that pb_graph_free and pb_graph_create work.
 * If not, then we'll find out later while profiling memory usage :) */

/* TODO: One day, we should probably just use a fixture for the graph. Since I don't currently feel like doing that, I won't. */
START_TEST(add_single_vertex)
{
    pb_vertex* vert = pb_vertex_create(NULL);
    pb_graph* graph = pb_graph_create();
    
	int idx = pb_graph_add_vertex(graph, vert);
	ck_assert_msg(idx == 0, "Index was %d, should have been 0.", idx);
	ck_assert_msg(graph->capacity == 2, "Graph capacity was %lu, should have been 2.", graph->capacity);
	ck_assert_msg(graph->size = 1, "Graph size was %lu, should have been 1.", graph->size);
	ck_assert_msg(graph->vertices[0] == vert, "Vertex wasn't successfully added.");

	pb_graph_free(graph, 0);
}
END_TEST

/* TODO: One day, we should probably just use a fixture for the graph. Since I don't currently feel like doing that, I won't. */
START_TEST(remove_single_vertex)
{
	pb_vertex* vert = pb_vertex_create(NULL);
	pb_graph* graph = pb_graph_create();
	pb_vertex* removed;

	/* Already it seems like we might have some issues with this whole lack of hash map... */
	pb_graph_add_vertex(graph, vert);
	removed = pb_graph_remove_vertex(graph, 0);

	ck_assert_msg(removed == vert, "Wrong vertex removed or invalid pointer (vert was %p, removed was %p).", vert, removed);
	ck_assert_msg(graph->capacity == 2, "Graph capacity was %lu, should have been 2.", graph->capacity);
	ck_assert_msg(graph->size == 0, "Graph size was %lu, should have been 0.", graph->size);

	pb_vertex_free(removed, 0);
	pb_graph_free(graph, 0);
}
END_TEST

START_TEST(expand_capacity)
{
	pb_vertex* v0 = (pb_vertex*)NULL;
	pb_vertex* v1 = (pb_vertex*)NULL;
	pb_vertex* v2 = (pb_vertex*)NULL;
	int idx0, idx1, idx2;
	pb_graph* graph = pb_graph_create();
	idx0 = pb_graph_add_vertex(graph, v0);
	idx1 = pb_graph_add_vertex(graph, v1);
	idx2 = pb_graph_add_vertex(graph, v2);

	ck_assert_msg(idx0 == 0, "Incorrect index %d for vertex 0.", idx0);
	ck_assert_msg(idx1 == 1, "Incorrect index %d for vertex 1.", idx1);
	ck_assert_msg(idx2 == 2, "Incorrect index %d for vertex 2.", idx2);
	ck_assert_msg(graph->vertices[2] == NULL, "Vertex 2 was %p, should have been NULL.", graph->vertices[2]);
	ck_assert_msg(graph->capacity == 4, "Graph capacity was %lu, should have been 4.", graph->capacity);
	ck_assert_msg(graph->size = 3, "Graph size was %lu, should have been 3.", graph->size);

	/* Don't want to free my bogus pointers here. */
	free(graph->vertices);
	free(graph);
}
END_TEST

START_TEST(add_edge)
{
	pb_vertex* vert = pb_vertex_create(NULL);
	pb_vertex* fake = (pb_vertex*)1;
	pb_graph* graph = pb_graph_create();
	int idx1, idx2;
	int res;
	
	idx1 = pb_graph_add_vertex(graph, vert);
	idx2 = pb_graph_add_vertex(graph, fake);

	if (idx1 == -1 || idx2 == -1) {
		/* Somehow we ran out of memory during the test. Oops */
		ck_abort_msg("Unexpectedly out of memory during test.");
	}

	res = pb_graph_add_edge(graph, (size_t)idx1, (size_t)idx2, 5);
	if (res == -1) {
		/* Somehow we ran out of memory during the test. Oops */
		ck_abort_msg("Unexpectedly out of memory during test.");
	}

	ck_assert_msg(vert->adjacent[0] == fake, "Edge not correctly added: expected vert->adjacent 0 to be %p, was %p.", fake, vert->adjacent[0]);
	ck_assert_msg(vert->edge_weights[0] == 5, "incorrect edge weight for added vertex: expected 5, was %lu.", vert->edge_weights[0]);

	pb_vertex_free(vert, 0);
	free(graph->vertices);
	free(graph);
}
END_TEST

START_TEST(remove_edge)
{
	pb_vertex* vert = pb_vertex_create(NULL);
	pb_vertex* fake = (pb_vertex*)1;
	pb_graph* graph = pb_graph_create();
	int idx1, idx2;
	int res;

	idx1 = pb_graph_add_vertex(graph, vert);
	idx2 = pb_graph_add_vertex(graph, fake);

	if (idx1 == -1 || idx2 == -1) {
		/* Somehow we ran out of memory during the test. Oops */
		ck_abort_msg("Unexpectedly out of memory during test.");
	}

	res = pb_graph_add_edge(graph, (size_t)idx1, (size_t)idx2, 5);
	if (res == -1) {
		/* Somehow we ran out of memory during the test. Oops */
		ck_abort_msg("Unexpectedly out of memory during test.");
	}

	res = pb_graph_remove_edge(graph, (size_t)idx1, (size_t)idx2);
	ck_assert_msg(res == 0, "Incorrect return value (should have been 0, was %d).", res);
	ck_assert_msg(vert->adj_size == 0, "Edge was not removed.");

	pb_vertex_free(vert, 0);
	free(graph->vertices);
	free(graph);
}
END_TEST

START_TEST(remove_non_existent_edge)
{
	pb_vertex* vert = pb_vertex_create(NULL);
	pb_graph* graph = pb_graph_create();
	int idx1;
	int res;

	idx1 = pb_graph_add_vertex(graph, vert);

	if (idx1 == -1) {
		/* Somehow we ran out of memory during the test. Oops */
		ck_abort_msg("Unexpectedly out of memory during test.");
	}

	res = pb_graph_remove_edge(graph, (size_t)idx1, (size_t)1);
	ck_assert_msg(res == -1, "Incorrect return value (should have been -1, was %d).", res);

	pb_vertex_free(vert, 0);
	free(graph->vertices);
	free(graph);
}
END_TEST

START_TEST(get_edge_weight)
{
	pb_vertex* vert = pb_vertex_create(NULL);
	pb_vertex* fake = (pb_vertex*)1;
	pb_graph* graph = pb_graph_create();
	int idx1, idx2;
	int res;
	size_t weight;

	idx1 = pb_graph_add_vertex(graph, vert);
	idx2 = pb_graph_add_vertex(graph, fake);

	if (idx1 == -1 || idx2 == -1) {
		/* Somehow we ran out of memory during the test. Oops */
		ck_abort_msg("Unexpectedly out of memory during test.");
	}

	res = pb_graph_add_edge(graph, (size_t)idx1, (size_t)idx2, 5);
	if (res == -1) {
		/* Somehow we ran out of memory during the test. Oops */
		ck_abort_msg("Unexpectedly out of memory during test.");
	}

	res = pb_graph_get_weight(graph, (size_t)idx1, (size_t)idx2, &weight);
	ck_assert_msg(res == 0, "Incorrect return value (should have been 0, was %d).", res);
	ck_assert_msg(weight == 5, "Weight should have been 5, was %lu.", weight);

	pb_vertex_free(vert, 0);
	free(graph->vertices);
	free(graph);
}
END_TEST

START_TEST(get_nonexistent_edge_weight)
{
	pb_vertex* vert = pb_vertex_create(NULL);
	pb_vertex* fake = (pb_vertex*)1;
	pb_graph* graph = pb_graph_create();
	int idx1, idx2;
	int res;
	size_t weight;

	idx1 = pb_graph_add_vertex(graph, vert);
	idx2 = pb_graph_add_vertex(graph, fake);

	if (idx1 == -1 || idx2 == -1) {
		/* Somehow we ran out of memory during the test. Oops */
		ck_abort_msg("Unexpectedly out of memory during test.");
	}

	res = pb_graph_get_weight(graph, (size_t)idx1, (size_t)idx2, &weight);
	ck_assert_msg(res == -1, "Incorrect return value (should have been -1, was %d).", res);

	pb_vertex_free(vert, 0);
	free(graph->vertices);
	free(graph);
}

END_TEST

Suite *make_pb_graph_suite(void) {
	/* Life test case tests lifetime events (create and destroy);
	* Adjacency test case tests all functions related to the adjacency list
	*/
	Suite *s;
	TCase *tc_vertices, *tc_edges, *tc_edge_weights;

	s = suite_create("Graph");

	tc_vertices = tcase_create("Vertex addition and removal");
	suite_add_tcase(s, tc_vertices);
	tcase_add_test(tc_vertices, add_single_vertex);
	tcase_add_test(tc_vertices, remove_single_vertex);
	tcase_add_test(tc_vertices, expand_capacity);

	tc_edges = tcase_create("Addition and removal of edges");
	suite_add_tcase(s, tc_edges);
	tcase_add_test(tc_edges, add_edge);
	tcase_add_test(tc_edges, remove_edge);
	tcase_add_test(tc_edges, remove_non_existent_edge);

	tc_edge_weights = tcase_create("Vertex edge weights");
	suite_add_tcase(s, tc_edge_weights);
	tcase_add_test(tc_edge_weights, get_edge_weight);
	tcase_add_test(tc_edge_weights, get_nonexistent_edge_weight);
	return s;
}