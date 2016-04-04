#include <check.h>
#include "../src/pb_graph.h"

/**
 * This file contains unit tests for all vertex-related functions in the graph API.
 */

START_TEST(create_no_data)
{
    pb_vertex* vert = pb_vertex_create(NULL);
    ck_assert_msg(vert->data == NULL, "Vertex data wasn't NULL.");
    ck_assert_msg(vert->adjacent != NULL), "Vertex adjacency was NULL.");
    ck_assert_msg(vert->adj_size == 0, "Vertex adjancency list size wasn't 0.");
    ck_assert_msg(vert->capacity == 2, "Vertex capacity wasn't 2.");
    
    free(vert->adjacent);
    free(vert);
}

END_TEST

START_TEST(adj_list_equality)
{
    size_t neighbours[2] = {0, 1};

    pb_vertex vert = {0};
    vert.adjacent = malloc(sizeof(size_t) * 2);
    vert.adj_size = 2;

    for(i = 0; i < 2; i++) {
        pb_vertex_add_edge(&vert, neighbours[i]);
    }

    for(i = 0; i < 2; i++) {
        ck_assert_msg(vert->adjacent[i], neighbours[i]) == 0, "Adjacency list isn't equal."); /* Test actual neighbour content */
    }
}
END_TEST

START_TEST(adj_list_expansion)
{
    pb_vertex neighbour;
    pb_vertex vert = {0};
    int i;

    /* Manually initialise vertex */
    vert.adjacent = malloc(sizeof(pb_vertex*) * 2);
    vert.adj_size = 2;

    /* Doesn't really matter what we add; we're just testing the size */
    for(i = 0; i < 5; ++i) {
        pb_vertex_add_edge(&vert, &neighbour);
    }

    ck_assert_msg(vert.adj_size == 5, "Incorrect adjacency size.");
    ck_assert_msg(vert.capacity == 8, "Incorrect adjacency capacity.");
    
    free(vert.adjacent);
}
END_TEST

START_TEST(adj_list_impossible_size)
{
    pb_vertex vert = {0};
    
    /* Manually initialise vertex */
    vert.adjacent = malloc(sizeof(pb_vertex*));
    vert.adj_size = 0x7FFFFFFFFFFFFFF;
    
    ck_assert_msg(pb_vertex_add_edge(&vert, NULL) == -1, "");

    ck_assert_msg(vert->adjacent == NULL, "Adjacency list is invalid pointer.");
    ck_assert_msg(vert->adj_size == 0, "Adjacency list has incorrect size.");
}
END_TEST