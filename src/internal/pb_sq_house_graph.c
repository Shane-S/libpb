#include <math.h>
#include <pb/internal/pb_sq_house_graph.h>
#include <pb/util/pb_float_utils.h>
#include <pb/util/pb_hash_utils.h>
#include <pb/util/pb_pair.h>
#include <string.h>

int pb_sq_house_get_shared_wall(pb_room* room1, pb_room* room2) {
    int shares_top = 0;
    int shares_bottom = 0;
    int shares_left = 0;
    int shares_right = 0;

    /* We need to use approximate float comparisons because the stairs may not have exactly
     * the same coordinates as the rooms on each floor. */
    shares_top = pb_float_approx_eq(room1->room_shape.points[0].y, room2->room_shape.points[1].y, 5) &&
        room1->room_shape.points[0].x < room2->room_shape.points[2].x &&
        room1->room_shape.points[3].x > room2->room_shape.points[1].x;

    shares_bottom = pb_float_approx_eq(room1->room_shape.points[1].y, room2->room_shape.points[0].y, 5) &&
        room1->room_shape.points[1].x < room2->room_shape.points[3].x &&
        room1->room_shape.points[2].x > room2->room_shape.points[0].x;

    shares_right = pb_float_approx_eq(room1->room_shape.points[3].x, room2->room_shape.points[0].x, 5) &&
        room1->room_shape.points[2].y < room2->room_shape.points[0].y &&
        room1->room_shape.points[3].y > room2->room_shape.points[1].y;

    shares_left = pb_float_approx_eq(room1->room_shape.points[0].x, room2->room_shape.points[3].x, 5) &&
        room1->room_shape.points[1].y < room2->room_shape.points[3].y &&
        room1->room_shape.points[0].y > room2->room_shape.points[2].y;

    if (shares_top) {
        return SQ_HOUSE_TOP;
    }
    else if (shares_right) {
        return SQ_HOUSE_RIGHT;
    }
    else if (shares_left) {
        return SQ_HOUSE_LEFT;
    }
    else if (shares_bottom) {
        return SQ_HOUSE_BOTTOM;
    }
    else {
        return -1;
    }
}

void pb_sq_house_get_wall_overlap(pb_room const* room1, pb_room const* room2, int wall, pb_point* start, pb_point* end) {

    switch (wall) {
    case SQ_HOUSE_TOP:
    case SQ_HOUSE_BOTTOM:
        start->x = fmaxf(room1->room_shape.points[0].x, room2->room_shape.points[0].x);
        end->x = fminf(room1->room_shape.points[3].x, room2->room_shape.points[3].x);

        /* Calculating overlap in the x axis is the same whether the bottom or the top
         * wall is shared, so use this hacky calculation to choose a point on the correct
         * wall from which to get the y coord. */
        start->y = room1->room_shape.points[wall].y;
        end->y = start->y;
        return;

    case SQ_HOUSE_RIGHT:
    case SQ_HOUSE_LEFT:
        start->y = fmaxf(room1->room_shape.points[2].y, room2->room_shape.points[2].y);
        end->y = fminf(room1->room_shape.points[3].y, room2->room_shape.points[3].y);

        /* Same thing here, just picks right or left point instead of top/bottom */
        start->x = room1->room_shape.points[wall].x;
        end->x = start->x;

        return;
    default:
        return;
    }
}

/**
 * Generates a connectivity graph between the rooms of a given floor.
 * The edges in the graph are of type pb_sq_house_room_edge and indicate whether these rooms
 * could be connected by a door (based on their room_specs).
 *
 * @param room_specs The map containing room specifications for this house.
 * @param floor      The floor for which the connectivity graph will be generated.
 * @return A graph containing the rooms' connections.
 */
pb_graph* pb_sq_house_generate_floor_graph(pb_hash* room_specs, pb_floor* floor) {
    pb_graph* g = pb_graph_create(pb_pointer_hash, pb_pointer_eq); /* Hash based on each room's pointer */

    if (!g) return NULL;

    /* Add all the rooms to the graph */
    unsigned i;
    for (i = 0; i < floor->num_rooms; ++i) {
        if (pb_graph_add_vertex(g, &floor->rooms[i], &floor->rooms[i]) == -1) {
            goto err_return;
        }
    }

    /* Do a brute force check of all rooms to determine their shared edges */
    /* Note that this doesn't account for connections to outside; that's done during window placement */
    for (i = 0; i < floor->num_rooms; ++i) {
        unsigned j;
        pb_sq_house_room_spec* spec;
        pb_hash_get(room_specs, (void*)floor->rooms[i].data, (void**)&spec);

        for (j = i + 1; j < floor->num_rooms; ++j) {
            pb_point start;
            pb_point end;
            int shared_wall = pb_sq_house_get_shared_wall(floor->rooms + i, floor->rooms + j);

            if (shared_wall != -1) {
                pb_sq_house_room_conn* conn = malloc(sizeof(pb_sq_house_room_conn));
                unsigned adj;
                if (!conn) goto err_return;

                pb_sq_house_get_wall_overlap(floor->rooms + i, floor->rooms + j, shared_wall, &start, &end);
                
                conn->neighbour = floor->rooms + j;
                conn->overlap_start = start;
                conn->overlap_end = end;
                conn->can_connect = 0;
                conn->wall = shared_wall;

                /* Check whether the room spec for room i allows a connection to room j */
                for (adj = 0; adj < spec->num_adjacent; ++adj) {
                    if (strcmp((char*)floor->rooms[j].data, spec->adjacent[adj]) == 0) {
                        conn->can_connect = 1;
                        break;
                    }
                }

                if (pb_graph_add_edge(g, floor->rooms + i, floor->rooms + j, 0.f, conn) == -1) {
                    free(conn);
                    goto err_return;
                }
            }
        }
    }

    return g;

err_return:
    /* Free the graph and all the edge info we just created */
    pb_graph_for_each_edge(g, pb_graph_free_edge_data, NULL);
    pb_graph_free(g);
    return NULL;
}

/**
 * Checks if the room stored in a given vertex doesn't have any valid connections to neigbouring
 * rooms (e.g., if every edge has a can_connect value of 0); if so, adds the vert_id to the list
 * of disconnected rooms.
 *
 * If a given room can connect to a neighbour but the neighbour can't connecto that room or vice-versa,
 * can_connect for both rooms will be set to 1.
 *
 * @param vert_id The vertex id (a pointer to a room, in this case).
 * @param vert    The vertex corresponding to the given room.
 * @param params  A pb_pair with first set to a pointer to the graph, and second set to a pointer to
 *                another pb_pair. second->first = the list of disconnected rooms, second->second = pointer
 *                to an int to indicate an error.
 */
static void process_disconnected_room(void const* vert_id, pb_vertex* vert, void* params) {
    /* Get the pointer to the graph */
    pb_pair* outer_pair = (pb_pair*)params;
    pb_graph* g = (pb_graph*)outer_pair->first;

    /* Get the pointer to the disconnected list and the error int */
    pb_pair* inner_pair = outer_pair->second;
    pb_vector* dl = (pb_vector*)inner_pair->first;
    int* err = (int*)inner_pair->second;
    
    size_t i;
    for (i = 0; i < vert->edges_size; ++i) {
        pb_edge* edge = vert->edges[i];
        pb_sq_house_room_conn* conn = (pb_sq_house_room_conn*)edge->data;

        /* Cheat really hard and use the knowledge that from_id and to_id are actually just room pointers
         * and that those room pointers are in fact stored as the data for the vertex to which they correspond */
        pb_edge* neighbour_edge = pb_graph_get_edge(g, edge->to->data, vert_id);
        pb_sq_house_room_conn* conn2 = (pb_sq_house_room_conn*)neighbour_edge->data;

        if (conn->can_connect == 1 || conn2->can_connect == 1) {
            /* If just one of the rooms said there was a valid connection, assume that there should be a connection
             * and set both to have one */
            conn->can_connect = 1;
            conn2->can_connect = 1;
            break;
        }
    }

    /* If there were no edges with doors, add this room to the list of disconnected rooms */
    if (i == vert->edges_size) {
        if (pb_vector_push_back(dl, (void*)&vert_id) == -1) {
            *err = 1;
        }
    }
}

pb_vector* pb_sq_house_find_disconnected_rooms(pb_graph* floor_graph, pb_floor* floor) {
    pb_vector* disconnected = pb_vector_create(sizeof(void*), 0);
    int error_occurred = 0;
    pb_pair outer_params;
    pb_pair inner_params = { disconnected, &error_occurred };
    outer_params.first = floor_graph;
    outer_params.second = &inner_params;

    pb_graph_for_each_vertex(floor_graph, process_disconnected_room, &outer_params);

    if (error_occurred) {
        pb_vector_free(disconnected);
        free(disconnected);
        return NULL;
    } else {
        /* Make sure that the first room isn't in the disconnected list */
        unsigned i;
        for (i = 0; i < disconnected->size; ++i) {
            pb_room* room = *((pb_room**)pb_vector_get_at(disconnected, i));
            if (room == &floor->rooms[0]) {
                pb_vector_remove_at(disconnected, i);
                break;
            }
        }
        return disconnected;
    }
}