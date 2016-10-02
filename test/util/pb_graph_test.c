#include <libcompat.h>
#include <check.h>
#include <pb/util/pb_graph.h>
#include <pb/util/pb_hash_utils.h>

/* Simple hash functions to use for vertices (data will just be an integer) */
static uint32_t test_hash(void const* key) {
    return (uint32_t)key;
}

static int test_eq(void const* val1, void const* val2) {
    return val1 == val2;
}

/* Hash map to use for the tests. */
pb_graph* graph;

/* Use this graph throughout the tests. */
void pb_graph_test_setup() {
    graph = pb_graph_create(test_hash, test_eq);
    if (!graph) {
        ck_abort_msg("Couldn't crreate graph; aborting graph test.");
    }
}

void pb_graph_test_teardown() {
    pb_graph_free(graph);
}

START_TEST(add_vertex)
{
    pb_graph_add_vertex(graph, (void*)1, (void*)1);
    ck_assert_msg(graph->vertices->size == 1, "Graph didn't contain any vertices after pb_graph_add_vertex");
}
END_TEST

START_TEST(get_vertex)
{
    pb_vertex const* vert = pb_graph_get_vertex(graph, (void*)1);
    ck_assert_msg(vert != NULL, "graph didn't contain vertex.");
    ck_assert_msg(vert->data == (void*)1, "vertex data should have been 1, was %p", vert->data);
}
END_TEST

START_TEST(get_nonexistent_vertex)
{
    pb_vertex const* vert = pb_graph_get_vertex(graph, (void*)3);
    ck_assert_msg(vert == NULL, "vert should have been NULL, was %p", vert);
}
END_TEST

START_TEST(remove_vertex)
{
    /* Remove the vertex we added in the previous test */
    /* Unit tests aren't supposed to depend on order, but this is more for convenience; I could create a new graph, add to it, and remove stuff just as well */
    pb_graph_remove_vertex(graph, (void*)1);

	ck_assert_msg(graph->vertices->size == 0, "Graph size was %lu, should have been 0.", graph->vertices->size);
}
END_TEST

START_TEST(add_edge)
{
    int result;
    pb_vertex const* v0;
    pb_vertex const* v1;

    result = pb_graph_add_vertex(graph, (void*)0, (void*)0);
    if (result == -1) {
        ck_abort_msg("Ran out of memory while adding vertex.");
    }

    result = pb_graph_add_vertex(graph, (void*)1, (void*)1);
    if (result == -1) {
        ck_assert_msg("Ran out of memory while adding vertex.");
    }

	result = pb_graph_add_edge(graph, (void*)0, (void*)1, 0.0f, (void*)5);
	if (result == -1) {
		/* Somehow we ran out of memory during the test. Oops */
		ck_abort_msg("Unexpectedly out of memory during test.");
	}

    v0 = pb_graph_get_vertex(graph, (void*)0);
    v1 = pb_graph_get_vertex(graph, (void*)1);

    ck_assert_msg(v0->edges_size == 1, "No edge was added to v0.");
}
END_TEST

START_TEST(get_edge)
{
    pb_vertex const* v0;
    pb_vertex const* v1;
    pb_edge const* edge;

    v0 = pb_graph_get_vertex(graph, (void*)0);
    v1 = pb_graph_get_vertex(graph, (void*)1);
    edge = pb_graph_get_edge(graph, (void*)0, (void*)1);

    ck_assert_msg(edge != NULL, "graph didn't contain edge from v0 to v1");
    ck_assert_msg(v0->edges[0] == edge, "v0 doesn't contain correct edge.");
    ck_assert_msg(edge->from == v0, "edge->from is incorrect; should have been %p, was %p", v0, edge->from);
    ck_assert_msg(edge->to == v1, "edge->to is incorrect; should have been %p, was %p", v1, edge->to);
}
END_TEST

START_TEST(get_nonexistent_edge)
{
    pb_edge const* edge = pb_graph_get_edge(graph, (void*)5, (void*)6);
    ck_assert_msg(edge == NULL, "Incorrect return value (should have been NULL, was %p).", edge);
}
END_TEST

START_TEST(remove_edge)
{
    int result = pb_graph_remove_edge(graph, (void*)0, (void*)1);
    pb_vertex const* v0 = pb_graph_get_vertex(graph, (void*)0);

    ck_assert_msg(result == 0, "Incorrect return value (should have been 0, was %d).", result);
    ck_assert_msg(graph->edges->size == 0, "graph->edges->size should have been 0, was %lu", graph->edges->size);
    ck_assert_msg(v0->edges_size == 0, "v0->edges_size should have been 0, was %lu", v0->edges_size);
}
END_TEST

START_TEST(remove_vertex_with_edge)
{
    pb_vertex const* v1 = pb_graph_get_vertex(graph, (void*)1);

    pb_graph_add_edge(graph, (void*)0, (void*)1, 0.f, (void*)10);
    pb_graph_add_edge(graph, (void*)1, (void*)0, 1.f, (void*)3);

    pb_graph_remove_vertex(graph, (void*)0);
    
    ck_assert_msg(graph->edges->size == 0, "graph->edges->size should have been 0, was %lu", graph->edges->size);
    ck_assert_msg(v1->edges_size == 0, "v1->edges_size should have been 0, was %lu", v1->edges_size);
}
END_TEST

static void add_one_to_vertex_data(void const* vert_id, pb_vertex* vert, void* param) {
    int* data = (int*)vert->data;
    *data += 1;
}

START_TEST(graph_for_each_vertex)
{
    pb_graph* g = pb_graph_create(pb_pointer_hash, pb_pointer_eq);
    int items[] = { 1, 2, 3 };
    int expected[] = { 2, 3, 4 };
    int i;

    /* Add a pointer to each element in items as a vertex */
    for (i = 0; i < 3; ++i) {
        pb_graph_add_vertex(g, &items[i], &items[i]);
    }

    pb_graph_for_each_vertex(g, add_one_to_vertex_data, NULL);

    for (i = 0; i < 3; ++i) {
        ck_assert_msg(items[i] == expected[i], "items[%d] should have been %d, was %d", i, expected[i], items[i]);
    }

    pb_graph_free(g);
}
END_TEST

static void add_one_to_edge_data(pb_edge const* edge, void* param) {
    int* data = (int*)edge->data;
    *data += 1;
}

START_TEST(graph_for_each_edge)
{
    pb_graph* g = pb_graph_create(pb_pointer_hash, pb_pointer_eq);
    int items[] = { 1, 2, 3 };
    int expected[] = { 2, 3, 4 };
    int i;

    int vert_items[] = { 5, 6, 7, 8 };

    /* Add a pointer to each element in vert_items as a vertex */
    for (i = 0; i < 4; ++i) {
        pb_graph_add_vertex(g, &vert_items[i], NULL);
    }

    /* Add an edge from the first vertex to each other vertex, with their data as addresses to the items array */
    for (i = 0; i < 3; ++i) {
        pb_graph_add_edge(g, &vert_items[0], &vert_items[i + 1], 0.f, &items[i]);
    }

    pb_graph_for_each_edge(g, add_one_to_edge_data, NULL);

    for (i = 0; i < 3; ++i) {
        ck_assert_msg(items[i] == expected[i], "items[%d] should have been %d, was %d", i, expected[i], items[i]);
    }

    pb_graph_free(g);
}
END_TEST

Suite *make_pb_graph_suite(void) {
	/* Life test case tests lifetime events (create and destroy);
	* Adjacency test case tests all functions related to the adjacency list
	*/
	Suite *s;
	TCase *tc_vertices, *tc_edges, *tc_iterator;

	s = suite_create("Graph");

    /* Don't need remove_nonexistent[edge/vertex] because we already test that with the equivalent get_nonexistent[edge/vertex] */

	tc_vertices = tcase_create("Vertex addition and removal");
	suite_add_tcase(s, tc_vertices);
	tcase_add_test(tc_vertices, add_vertex);
    tcase_add_test(tc_vertices, get_vertex);
    tcase_add_test(tc_vertices, get_nonexistent_vertex);
	tcase_add_test(tc_vertices, remove_vertex);

	tc_edges = tcase_create("Addition and removal of edges");
	suite_add_tcase(s, tc_edges);
	tcase_add_test(tc_edges, add_edge);
    tcase_add_test(tc_edges, get_edge);
    tcase_add_test(tc_edges, get_nonexistent_edge);
	tcase_add_test(tc_edges, remove_edge);
    tcase_add_test(tc_edges, remove_vertex_with_edge);

    tc_iterator = tcase_create("for_each (iterator) functions");
    suite_add_tcase(s, tc_iterator);
    tcase_add_test(tc_iterator, graph_for_each_vertex);
    tcase_add_test(tc_iterator, graph_for_each_edge);

    /* This is pretty hacky now. Should probably think about these tests a bit more... */
    tcase_add_unchecked_fixture(tc_vertices, pb_graph_test_setup, NULL);
    tcase_add_unchecked_fixture(tc_edges, NULL, pb_graph_test_teardown);
	return s;
}