#include <stdlib.h>
#include <string.h>
#include <pb/util/graph/graph.h>
#include <pb/util/hashmap/hash_utils.h>
#include <pb/util/pair/pair.h>

static void pb_graph_remove_edge_internal(pb_graph* graph, pb_edge* edge);

PB_UTIL_DECLSPEC pb_vertex* PB_UTIL_CALL pb_vertex_create(void* data) {
    pb_vertex* vert = NULL;
    pb_edge** edges = NULL;

    vert = malloc(sizeof(pb_vertex));
    if (!vert) {
        return NULL;
    }

    edges = malloc(sizeof(pb_edge*) * 2);
    if (!edges) {
        free(vert);
        return NULL;
    }
    
    vert->edges = edges;
    vert->edges_capacity = 2;
    vert->edges_size = 0;
    vert->in_degree = 0;
    vert->data = data;
    return vert;
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_vertex_free(pb_vertex* vert) {
    free(vert->edges);
    free(vert);
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vertex_add_edge(pb_vertex *vert, pb_edge* edge) {
    if (vert->edges_size == vert->edges_capacity) {
        pb_edge** new_edges;

        new_edges = realloc(vert->edges, sizeof(pb_edge*) * vert->edges_capacity * 2);
        if (!new_edges) {
            return -1;
        }

        vert->edges = new_edges;
        vert->edges_capacity = vert->edges_capacity * 2;
    }
    
    vert->edges[vert->edges_size] = edge;
	vert->edges_size++;
	return 0;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_vertex_remove_edge(pb_vertex *start, pb_edge* edge) {
    size_t i;
    int edge_idx = -1;

    /* I'm not sure where this leaves us for removal from a graph. O(|V||E|)? Not ideal, but probably won't be
       an issue with our graph sizes.*/
    for(i = 0; i < start->edges_size; i++) {
        if(start->edges[i] == edge) {
            edge_idx = i;
            break;
        }
    }

    if (edge_idx == -1) {
        return -1;
    }

    /* Shift all elements and corresponding edge weights up by one */
    for (i = edge_idx + 1; i < start->edges_size; ++i) {
        start->edges[i - 1] = start->edges[i];
    }

    start->edges_size--;
    /* TODO: Consider trimming the array size if it becomes too wasteful */
    
    return 0;
}

static uint32_t edge_hash(void const* edge) {
    pb_edge* actual = (pb_edge*)edge;
    uint32_t hash = 23;

    /* Hash the addresses; every edge will have a different pair of vertices, so the combination of
     * addresses will identify the edge */
    hash += 31 * pb_murmurhash3((void*)&actual->from, (int)sizeof(pb_vertex*));
    hash += 31 * pb_murmurhash3((void*)&actual->to, (int)sizeof(pb_vertex*));

    return hash;
}

static int edge_eq(void const* edge1, void const* edge2) {
    pb_edge* actual1 = (pb_edge*)edge1;
    pb_edge* actual2 = (pb_edge*)edge2;

    return actual1->from == actual2->from && actual1->to == actual2->to;
}

PB_UTIL_DECLSPEC pb_graph* PB_UTIL_CALL pb_graph_create(pb_hash_func id_hash, pb_hash_eq_func id_eq) {
    pb_graph* graph = malloc(sizeof(pb_graph));
    pb_hashmap* vertices;
    pb_hashmap* edges;
    
    if (!graph) {
        return NULL;
    }

    vertices = pb_hashmap_create(id_hash, id_eq);
    if (vertices == NULL) {
        free(graph);
        return NULL;
    }

    edges = pb_hashmap_create(edge_hash, edge_eq);
    if (!edges) {
        pb_hashmap_free(vertices);
        free(graph);
        return NULL;
    }

    graph->vertices = vertices;
    graph->edges = edges;

    return graph;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_graph_add_vertex(pb_graph* graph, void const* vert_id, void* data) {
    pb_vertex* vert = pb_vertex_create(data);
    if (!vert) {
        return -1;
    }

    if (pb_hashmap_put(graph->vertices, vert_id, (void*)vert) == -1) {
        pb_vertex_free(vert);
        return -1;
    } else {
        return 0;
    }
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_graph_remove_vertex(pb_graph* graph, void const* vert_id) {
    pb_vertex* vert;
    size_t i;
    if (pb_hashmap_get(graph->vertices, vert_id, (void**)&vert) == -1) {
        return -1; /* There was no vertex with the given ID in this graph */
    }

    /* Decrease destination vertices' in-degree */
    for (i = 0; i < vert->edges_size; ++i) {
        vert->edges[i]->to->in_degree--;
        pb_hashmap_remove(graph->edges, vert->edges[i]);
    }

    /* Iterate over the edges list to remove all edges to the given vertex */
    for (i = 0; i < graph->edges->cap; ++i) {
        if (graph->edges->states[i] == FULL) {
            pb_edge* edge = (pb_edge*)graph->edges->entries[i].val;
            if (edge->to == vert) {
                pb_graph_remove_edge_internal(graph, edge);
            }
        }
    }

    pb_hashmap_remove(graph->vertices, vert_id);
    pb_vertex_free(vert);

    return 0;
}

PB_UTIL_DECLSPEC pb_vertex const* PB_UTIL_CALL pb_graph_get_vertex(pb_graph const* graph, void const* vert_id) {
    pb_vertex* out;
    if (pb_hashmap_get(graph->vertices, vert_id, (void**)&out) == -1) {
        return NULL;
    }

    return out;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_graph_add_edge(pb_graph *graph, void const* from_id, void const* to_id, float weight, void* data) {
    pb_edge* edge;
    pb_vertex* from = (pb_vertex*)pb_graph_get_vertex(graph, from_id); /* Breaking my own const-ness rules here. Oops */
    pb_vertex* to = (pb_vertex*)pb_graph_get_vertex(graph, to_id);

    if (!from || !to) {
        return -1;
    }

    edge = malloc(sizeof(pb_edge));
    if (!edge) {
        return -1;
    }

    edge->weight = weight;
    edge->from = from;
    edge->to = to;
    edge->data = data;

    if (pb_vertex_add_edge(from, edge) == -1) {
        free(edge);
        return -1;
    }

    if (pb_hashmap_put(graph->edges, edge, edge) == -1) {
        pb_vertex_remove_edge(from, edge);
        free(edge);
        return -1;
    }

    edge->to->in_degree++;
    return 0;
}

static void pb_graph_remove_edge_internal(pb_graph* graph, pb_edge* edge) {
    edge->to->in_degree--;
    pb_vertex_remove_edge(edge->from, edge);
    pb_hashmap_remove(graph->edges, (void*)edge);
    free(edge);
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_graph_remove_edge(pb_graph *graph, void const* from_id, void const* to_id) {
    pb_edge* edge = (pb_edge*)pb_graph_get_edge(graph, from_id, to_id); /* Breaking const-ness again */
    if (!edge) return -1;

    pb_graph_remove_edge_internal(graph, edge);
    return 0;
}

PB_UTIL_DECLSPEC pb_edge const* PB_UTIL_CALL pb_graph_get_edge(pb_graph const* graph, void const* from_id, void const* to_id) {
    pb_vertex* from;
    pb_vertex* to;
    pb_edge* out;
    pb_edge temp;

    if (pb_hashmap_get(graph->vertices, from_id, (void**)&from) == -1 ||
        pb_hashmap_get(graph->vertices, to_id, (void**)&to) == -1) {
        return NULL;
    }

    temp.from = from;
    temp.to = to;

    if (pb_hashmap_get(graph->edges, (void*)&temp, (void**)&out) == -1) {
        return NULL;
    }

    return out;
}

static void free_hashed_vertex(pb_hashmap_entry* entry, void* unused) {
    pb_vertex_free((pb_vertex*)entry->val);
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_free(pb_graph *graph) {
    pb_hashmap_for_each(graph->vertices, free_hashed_vertex, NULL);
    pb_hashmap_free(graph->vertices);

    pb_hashmap_for_each(graph->edges, pb_hashmap_free_entry_data, NULL);
    pb_hashmap_free(graph->edges);

    free(graph);
}

/**
 * Iterates over the edge hash map, calling a given pb_graph_edge_iterator_func on each edge.
 *
 * @param entry An entry from graph->edges.
 * @param param A pair containing the edge iterator function and the parameter supplied to it.
 */
static void edge_hash_iterator(pb_hashmap_entry* entry, void* param) {
    pb_pair* p = (pb_pair*)param;
    pb_graph_edge_iterator_func f = (pb_graph_edge_iterator_func)p->first;
    void* edge_param = p->second;
    pb_edge* edge = entry->val;
    
    f(edge, edge_param);
}

/**
 * Iterates over the vertex hash map, calling a given pb_graph_vertex_iterator_func on each entry.
 *
 * @param entry An entry from graph->vertices.
 * @param param A pair containing the vertex iterator function and the parameter supplied to it.
 */
static void vertex_hash_iterator(pb_hashmap_entry* entry, void* param) {
    pb_pair* p = (pb_pair*)param;
    pb_graph_vertex_iterator_func f = (pb_graph_vertex_iterator_func)p->first;
    void* vert_param = p->second;

    pb_vertex* vert = entry->val;
    void* vert_id = entry->key;

    f(vert_id, vert, vert_param);
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_for_each_edge(pb_graph* graph, pb_graph_edge_iterator_func func, void* param) {
    pb_pair params = { func, param };
    pb_hashmap_for_each(graph->edges, edge_hash_iterator, &params);
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_for_each_vertex(pb_graph* graph, pb_graph_vertex_iterator_func func, void* param) {
    pb_pair params = { func, param };
    pb_hashmap_for_each(graph->vertices, vertex_hash_iterator, &params);
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_free_vertex_data(void const* vert_id, pb_vertex* vert, void* unused) {
    free(vert->data);
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_graph_free_edge_data(pb_edge const* edge, void* unused) {
    free(edge->data);
}
