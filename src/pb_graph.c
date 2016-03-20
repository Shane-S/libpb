#include <stdlib.h>
#include "pb_graph.h"

pb_vertex* pb_vertex_create(void *data) {
    pb_vertex *vert = NULL;
    pb_vertex **adjacent = NULL;

    vert = malloc(sizeof(pb_vertex));
    if(!vert) {
        goto err_return;
    }

    adjacent = malloc(sizeof(pb_vertex*) * 2);
    if(!adjacent) {
        goto err_return;
    }

    vert->data = data;
    vert->adjacent = malloc(sizeof(pb_vertex*) * 2);
    vert->adj_size = 0;
    vert->adj_capacity = 2;

    return vert;
    
err_return:
    free(vert);
    free(adjacent);
    return NULL;
}

void pb_vertex_free(pb_vertex* vert, int free_data) {
    free(vert->adjacent);
    if(free_data) {
        free(vert->data);
    }
    free(vert);
}

int pb_vertex_add_edge(pb_vertex *start, size_t *dest) {
    if(start->adj_size == start->adj_capacity) {
        size_t *new_adjacent = realloc(start->adjacent, sizeof(size_t) * start->adj_capacity * 2);
        if(!new_adjacent) return -1;
        start->adjacent = new_adjacent;
        start->adj_capacity *= 2;
    }

    start->adjacent[start->adj_size++] = dest;
    return 0;
}

int pb_vertex_remove_edge(pb_vertex *start, size_t dest) {
    int i;
    int dest_idx = -1;

    /* I'm not sure where this leaves us for removal from a graph. O(|V||E|)? Not ideal, but probably won't be
       an issue with our graph sizes.*/
    for(i = 0; i < start->adj_size; i++) {
        if(start->adjacent[i] == dest) {
            dest_idx = i;
            break;
        }
    }

    if(dest_idx == -1) {
        return -1;
    }

    /* Shift all elements up by one */
    for(i = dest_idx + 1; i < start->adj_size; ++i) {
        start->adjacent[i - 1] = start->adjacent[i];
    }

    start->adj_size--;
    /* TODO: Consider trimming the array size if it becomes too wasteful */
    
    return 0;
}

pb_graph* pb_graph_create() {
    pb_graph *graph = NULL;
    pb_vertex **vertices = NULL;

    graph = malloc(sizeof(pb_graph));
    if(!graph) {
        goto err_return;
    }

    vertices = malloc(sizeof(pb_vertex*) * 2);
    if(!vertices) {
        goto err_return;
    }

    graph->vertices = vertices;
    graph->size = 0;
    graph->capacity = 2;

    return graph;

err_return:
    free(graph);
    free(vertices);
    return NULL;
}

int pb_graph_add_vertex(pb_graph* graph, pb_vertex* vert) {
    if(graph->size == graph->capacity) {
        size_t **new_vertices = realloc(graph->vertices, graph->capacity * 2 * sizeof(pb_vertex*));
        if(!new_vertices) return -1;
        graph->vertices = new_vertices;
        graph->capacity *= 2;
    }
    
    graph->vertices[graph->size++] = vert;
    return graph->size - 1;
}

pb_vertex* pb_graph_remove_vertex(pb_graph* graph, size_t vert) {
    pb_vertex *to_remove = graph->vertices[vert];
    size_t i;
    /* Remove any edges containing the given vertex */
    for(i = 0; i < graph->size; ++i) {
        if(i == vert) continue;
        pb_vertex_remove_edge(graph->vertices[i], vert);
    }

    /* Shift elements up */
    for(i = vert + 1; i < graph->size; ++i) {
        graph->vertices[i - 1] = graph->vertices[i];
    }

    graph->size--;

    return to_remove;
}

int pb_graph_add_edge(pb_graph *graph, size_t from, size_t to) {
    return pb_vertex_add_edge(graph->vertices[from], to);
}

int pb_graph_remove_edge(pb_graph *graph, size_t from, size_t to) {
    return pb_vertex_remove_edge(graph->vertices[from], to);
}

void pb_graph_free(pb_graph *graph, int free_data) {
    size_t i;
    for(i = 0; i < graph->size; ++i) {
        pb_vertex_free(graph->vertices[i], free_data);
    }
    free(graph->vertices);
}
