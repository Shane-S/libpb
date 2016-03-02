#include <stddef.h>

typedef struct _pb_vertex pb_vertex;

/**
 * A vertex, which contains some data and a list of vertices to which it is connected.
 */
struct _pb_vertex {
    void *data;
    size_t *adjacent; /* Stores indices of other vertices in the graph */
    size_t adj_size;
    size_t adj_capacity;
};

/**
 * Creates a new vertex with the given data and adjacency list.
 *
 * @param data The data to include in this vertex.
 *
 * @return A pointer to a new vertex object on success, or NULL on failure (out of memory).
 */
pb_vertex* pb_vertex_create(void* data);

/**
 * Frees a vertex and its adjacency list.
 *
 * @param vert      The vertex to free.
 * @param free_data Whether to free the data contained in the vertex.
 */
void pb_vertex_free(pb_vertex* vert, int free_data);

/**
 * Adds a vertex to this vertex's adjacency list.
 * @param start The start vertex.
 * @param dest  The destination vertex's index.
 *
 * @return 0 on success, -1 on failure (out of memory).
 */
int pb_vertex_add_edge(pb_vertex *start, size_t dest);

/**
 * Removes a vertex from this vertex's adjacency list. The neighour will not be freed.
 * @param start The start vertex.
 * @param dest  The destination vertex's index.
 *
 * @return 0 on sucess, -1 when the vertex didn't have this neighbour.
 */
int pb_vertex_remove_edge(pb_vertex *start, size_t dest);

/**
 * A graph, which is basically a collection of vertices and adjacency lists.
 */
typedef struct _pb_graph {
    pb_vertex **vertices;
    size_t size;
    size_t capacity;
} pb_graph;

/**
 * Creates an empty graph.
 *
 * @return An empty graph, or NULL on failure (out of memory).
 */
pb_graph* pb_graph_create();

/**
 * Adds a vertex to the graph.
 * @param graph The graph to which the vertex will be added.
 * @param vert  The vertex to be added to the graph. Cannot be NULL.
 *
 * @return The index of the newly added vertex on success, -1 when out of memory.
 */
int pb_graph_add_vertex(pb_graph* graph, pb_vertex* vert);

/**
 * Removes a vertex from the graph. The vertex isn't freed.
 * Pre-condition: The specified index is within the graph's size.
 *
 * @param graph The graph from which the vertex will be deleted.
 * @param vert  The index of the vertex to be deleted from the graph.
 *
 * @return The removed vertex.
 */
pb_vertex* pb_graph_remove_vertex(pb_graph* graph, size_t vert);

/**
 * Adds an edge from vertex "from" to vertex "to" in the given graph.
 *
 * @param graph The graph on which to operate.
 * @param from  The index of the start vertex.
 * @param to    The index of the end vertex.
 *
 * @return 0 on success, -1 on failure (out of memory).
 */
int pb_graph_add_edge(pb_graph* graph, size_t from, size_t to);

/**
 * Removes an edge from vertex "from" to vertex "to" in the given graph.
 *
 * @param graph The graph on which to operate.
 * @param from  The index of the start vertex.
 * @param to    The index of the end vertex.
 *
 * @return 0 on success, -1 if the given edge didn't exist.
 */
int pb_graph_remove_edge(pb_graph* graph, size_t from, size_t to);

/**
 * Frees the graph and all of its vertices.
 * @param graph     The graph to free.
 * @param free_data Whether to free the data associated with the vertices.
 */
void pb_graph_free(pb_graph* graph, int free_data);

