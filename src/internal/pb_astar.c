#include <pb/internal/pb_astar.h>
#include <pb/util/pb_heap.h>
#include <pb/util/pb_hashmap.h>
#include <pb/util/pb_hash_utils.h>
#include <stdlib.h>

typedef struct pb_astar_node pb_astar_node;

struct pb_astar_node {
    pb_vertex const* vert;
    pb_astar_node* parent; /* Previous node in least-cost path to this node */
    float g_cost; /* Actual cost to reach this vertex */
    float h_cost; /* Estimated cost from this vertex to the goal */
};

int pb_astar(pb_vertex const* start, pb_vertex const* goal, pb_astar_heuristic heuristic, pb_vector** path) {
    pb_vector* result;
    pb_heap* frontier;
    pb_hashmap* visited;

    pb_astar_node* start_node;
    int found_path = 0;

    result = pb_vector_create(sizeof(pb_vertex*), 0);
    if (!result) {
        return -1;
    }

    frontier = pb_heap_create(0);
    if (!frontier) {
        pb_vector_free(result);
        return -1;
    }

    visited = pb_hashmap_create(pb_pointer_hash, pb_pointer_eq);
    if (!visited) {
        pb_vector_free(result);
        pb_heap_free(frontier);
        return -1;
    }

    /* The easy case */
    if (start == goal)  {
        if (pb_vector_push_back(result, start) == -1) {
            goto err_return;
        } else {
            *path = result;
            return 0;
        }
    }

    /* Every other case lol */
    start_node = malloc(sizeof(pb_astar_node));
    if (!start_node) {
        goto err_return;
    }

    start_node->vert = start;
    start_node->g_cost = 0.f;
    start_node->h_cost = heuristic(start, goal);
    start_node->parent = NULL;
    if (pb_heap_insert(frontier, start_node, start_node->g_cost + start_node->h_cost) == -1 || pb_hashmap_put(visited, start, start_node) == -1) {
        goto err_return;
    }

    pb_astar_node* node = start_node;
    while (frontier->items.size) {
        unsigned i;
        
        node = (pb_astar_node*)pb_heap_get_min(frontier);

        if (node->vert == goal) {
            found_path = 1;
            break;
        }

        /* Check out all neighbouring nodes */
        for (i = 0; i < node->vert->edges_size; ++i) {
            pb_edge* edge = node->vert->edges[i];
            float g_cost_neighbour = node->g_cost + edge->weight;
            
            /* Add the node to the visited map if it hasn't been visited already; otherwise, update its cost if appropriate */
            pb_astar_node* neighbour_node;
            if (pb_hashmap_get(visited, edge->to, &neighbour_node) == -1) {
                neighbour_node = malloc(sizeof(pb_astar_node));
                if (!neighbour_node) {
                    goto err_return;
                }

                /* Add the neighbour to the visited list */
                if (pb_hashmap_put(visited, edge->to, neighbour_node) == -1) {
                    free(neighbour_node);
                    goto err_return;
                }

                neighbour_node->parent = node;
                neighbour_node->vert = edge->to;
                neighbour_node->g_cost = g_cost_neighbour;
                neighbour_node->h_cost = heuristic(edge->to, goal);

                /* Add the neighbour to the frontier and keep searching */
                if (pb_heap_insert(frontier, neighbour_node, neighbour_node->g_cost + neighbour_node->h_cost) == -1) {
                    goto err_return;
                }
            } else {
                /* Decrease the neighbour's cost if we found a better path */
                if (g_cost_neighbour < neighbour_node->g_cost) {
                    neighbour_node->g_cost = g_cost_neighbour;
                    neighbour_node->parent = node;
                    pb_heap_decrease_key(frontier, neighbour_node, neighbour_node->g_cost + neighbour_node->h_cost);
                }
            }
        }
    }

    if (!found_path) {
        return -1;
    }

    /* Push all the elements onto the list */
    while (node) {
        if (pb_vector_push_back(result, &node->vert) == -1) {
            goto err_return;
        }
        node = node->parent;
    }

    /* Reverse the list to give the correct path from start to goal */
    {
        pb_vertex* temp;
        pb_vector_reverse_no_alloc(result, &temp);
    }

    pb_hashmap_for_each(visited, pb_hashmap_free_entry_data, 0);
    pb_hashmap_free(visited);
    pb_heap_free(frontier);
    *path = result;
    return 0;

err_return:
    pb_hashmap_for_each(visited, pb_hashmap_free_entry_data, 0);
    pb_hashmap_free(visited);
    pb_vector_free(result);
    pb_heap_free(frontier);
    return -1;
}