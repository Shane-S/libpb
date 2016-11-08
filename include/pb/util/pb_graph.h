/*
  Lib Proc-Building
  Copyright (C) 2016 Shane Spoor <shane.d.spoor@gmail.com>
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef PB_GRAPH_H
#define PB_GRAPH_H

#include <stddef.h>
#include <pb/util/pb_util_exports.h>
#include <pb/util/pb_hashmap.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pb_vertex pb_vertex;

typedef struct {
    pb_vertex* from;
    pb_vertex* to;
    float weight;
    void* data;
} pb_edge;

/**
 * A vertex, which contains some data and a list of vertices to which it is connected.
 */
struct pb_vertex {
    pb_edge** edges;
    void *data;
    size_t edges_size;
    size_t edges_capacity;
};

/*
 * Function type for processing each vertex in a graph. Use with pb_graph_for_each_vertex.
 */
typedef void(*pb_graph_vertex_iterator_func)(void const* vert_id, pb_vertex* vert, void* param);

/*
 * Function type for processing each edge in a graph. Use with pb_graph_grah_for_each_edge.
 */
typedef void(*pb_graph_edge_iterator_func)(pb_edge const* edge, void* param);

/**
 * Creates a new vertex with the given data.
 *
 * @param data The data to store at this vertex.
 * @return A vertex initialised with the given data and an empty edge list or NULL on failure (out of memory).
 */
PB_UTIL_DECLSPEC pb_vertex* PB_UTIL_CALL pb_vertex_create(void* data);

/**
 * Frees the vertex and its internal structures.
 * The vertex will be unusable after this operation.
 *
 * @param vert The vertex to free.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vertex_free(pb_vertex* vert);

/**
 * Adds a vertex to this vertex's adjacency list. Note that this function doesn't check whether
 * an edge to dest already exists; use pb_vertex_get_edge to modify existing edges.
 *
 * @param start The start vertex.
 * @param dest  The destination vertex.
 * @param edge  The edge to add. This function creates a shallow copy of edge.
 *
 * @return 0 on success, -1 on out of memory.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vertex_add_edge(pb_vertex *vert, pb_edge* edge);

/**
 * Removes a vertex from this vertex's adjacency list. The neighour will not be freed.
 * @param start The start vertex.
 * @param dest  The destination vertex to which the edge will be removed.
 *
 * @return 0 on sucess, -1 when the vertex didn't have this neighbour.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vertex_remove_edge(pb_vertex *vert, pb_edge* edge);

/**
 * A graph, which is basically a collection of vertices and adjacency lists.
 */
typedef struct {
    pb_hashmap* vertices;
    pb_hashmap* edges;
} pb_graph;

/**
 * Allocates and initialies a new, empty graph.
 *
 * @param id_hash The hash function to use for vertex ID's.
 * @param id_eq   The equality function to use for vertex ID's.
 *
 * @return An empty graph or NULL if out of memory.
 */
PB_UTIL_DECLSPEC pb_graph* pb_graph_create(pb_hash_func id_hash, pb_hash_eq_func id_eq);

/**
 * Adds a vertex to the graph.
 * @param graph   The graph to which the vertex will be added.
 * @param vert_id A unique identifier for the vertex that will be hashed with the initially supplied
 *                id_hash function.
 * @param data    The data to store in the vertex.
 *
 * @return 0 on success, -1 when out of memory.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_graph_add_vertex(pb_graph* graph, void const* vert_id, void* data);

/**
 * Removes a vertex from the graph if the vertex was actually in the graph.
 *
 * @param graph   The graph from which the vertex will be removed.
 * @param vert_id The ID of the vertex to remove.
 * @return 0 on success, -1 if the vertex didn't exist.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_graph_remove_vertex(pb_graph* graph, void const* vert_id);

/**
 * Gets the vertex associated with vert_id, if there is one.
 *
 * Note that the returned vertex is read-only; use the pb_graph_add_vertex and pb_graph_remove_vertex
 * functions to add a new vertex to the graph.
 *
 * @param vert_id The unique identifier associated with the vertex.
 * @return The associated vertex or NULL if there is no vertex associated with that ID.
 */
PB_UTIL_DECLSPEC pb_vertex const* PB_UTIL_CALL pb_graph_get_vertex(pb_graph const* graph, void const* vert_id);

/**
 * Adds an edge from vertex "from" to vertex "to" in the given graph.
 *
 * @param graph   The graph on which to operate.
 * @param from_id The start vertex ID.
 * @param to+id   The destination vertex ID.
 * @param weight  The edge weight, if there is one.
 * @param data    The (optional) data to associate with this edge.
 *
 * @return 0 on success, -1 on failure (out of memory or at least one of the given vertices didn't exist).
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_graph_add_edge(pb_graph* graph, void const* from_id, void const* to_id, float weight, void* data);

/**
 * Removes an edge from vertex "from" to vertex "to" in the given graph.
 *
 * @param graph   The graph on which to operate.
 * @param from_id The start vertex ID.
 * @param to_id   The end vertex ID.
 *
 * @return 0 on success, -1 if the given edge didn't exist.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_graph_remove_edge(pb_graph* graph, void const* from_id, void const* to_id);

/**
 * Gets the edge from vertex from to vertex to. If there is no such edge, return NULL.
 *
 * Note that the edge can't be modified. Edges can only be changed via the pb_graph_add_edge
 * and pb_graph_remove_edge functions.
 *
 * @param graph   The graph in which to find the edge weight.
 * @param from_id The start vertex's ID.
 * @param to_id   The destination vertex's ID.
 *
 * @return The edge on success, NULL if the edge didn't exist.
 */
PB_UTIL_DECLSPEC pb_edge const* PB_UTIL_CALL pb_graph_get_edge(pb_graph const* graph, void const* from_id, void const* to_id);

/**
 * Runs func (with the supplied parameter) on every edge in the graph.
 * @param func  The function to run for every edge in the graph.
 * @param param The (optional) parameter which will be supplied to func on each iteration.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_for_each_edge(pb_graph* graph, pb_graph_edge_iterator_func func, void* param);

/**
 * Runs func (with the supplied parameter) on every vertex in the graph.
 * @param func  The function to run for every vertex in the graph.
 * @param param The (optional) parameter which will be supplied to func on each iteration.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_for_each_vertex(pb_graph* graph, pb_graph_vertex_iterator_func func, void* param);

/**
 * When passed to pb_graph_for_each_vertex, frees vert->data for every vertex in the graph.
 * @param vert_id The vertex id (unused).
 * @param vert    The vertex being processed.
 * @param unused  The optional parameter to pb_graph_graph_vertex_iterator_func, which is unused in this function.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_free_vertex_data(void const* vert_id, pb_vertex* vert, void* unused);

/**
 * When passed to pb_graph_for_each_edge, frees edge->data for every vertex in the graph.
 * @param edge   The edge being processed.
 * @param unused The optional parameter to pb_graph_edge_iterator_func, which is unused in this function.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_free_edge_data(pb_edge const* edge, void* unused);

/**
 * Frees the graph and its vertices.
 * The graph will be unusable after this operation.
 *
 * @param graph The graph to free.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_free(pb_graph* graph);

#ifdef __cplusplus
}
#endif
#endif /* PB_GRAPH_H */