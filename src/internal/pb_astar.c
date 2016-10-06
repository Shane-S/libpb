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

int pb_astar_node_cmp(void* node1, void* node2) {
    pb_astar_node* n1 = (pb_astar_node*)node1;
    pb_astar_node* n2 = (pb_astar_node*)node2;

    float t_cost1 = n1->g_cost + n1->h_cost;
    float t_cost2 = n2->g_cost + n2->h_cost;
    float diff = t_cost1 - t_cost2;
    
    /* Theoretically this could cause problem if floats are infinity, NaN, etc., but I won't check that here */
    if (diff < 0) {
        return -1;
    } else if (diff > 0) {
        return 1;
    } else {
        return 0;
    }
}

pb_vector* pb_astar(pb_vertex const* start, pb_vertex const* goal, pb_astar_heuristic heuristic) {
    pb_vector* result;
    pb_heap* frontier;
    pb_hashmap* visited;

    pb_astar_node* start_node;

    result = pb_vector_create(sizeof(pb_vertex*), 0);
    if (!result) {
        return NULL;
    }

    frontier = pb_heap_create(pb_astar_node_cmp, 0);
    if (!frontier) {
        pb_vector_free(result);
        return NULL;
    }

    visited = pb_hashmap_create(pb_pointer_hash, pb_pointer_eq);
    if (!visited) {
        pb_vector_free(result);
        pb_heap_free(frontier);
        return NULL;
    }

    /* The easy case */
    if (start == goal)  {
        if (pb_vector_push_back(result, start) == -1) {
            goto err_return;
        } else {
            return result;
        }
    }

    start_node = malloc(sizeof(pb_astar_node));
    if (!start_node) {
        goto err_return;
    }

    /* Every other case lol */
    start_node->vert = start;
    start_node->g_cost = 0.f;
    start_node->h_cost = h(start, goal);
    start_node->parent = NULL;
    if (pb_heap_insert(frontier, start_node) == -1 || pb_hashmap_put(visited, start, start_node) == -1) {
        goto err_return;
    }

    while (frontier->size) {
        pb_astar_node* node = (pb_astar_node*)pb_heap_get_min(frontier);

        if (node->vert == goal) {
            break;
        }
    }

err_return:
    pb_hashmap_for_each(visited, pb_hashmap_free_entry_data, 0);
    pb_hashmap_free(visited);
    pb_vector_free(result);
    pb_heap_free(frontier);
    return NULL;
}