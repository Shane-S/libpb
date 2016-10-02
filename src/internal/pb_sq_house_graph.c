#include <math.h>
#include <pb/internal/pb_sq_house_graph.h>
#include <pb/util/pb_float_utils.h>
#include <pb/util/pb_hash_utils.h>
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
            int shared_wall = pb_sq_house_get_shared_wall(floor->rooms + i, floor->rooms + j, &start, &end);

            if (shared_wall != -1) {
                pb_sq_house_room_conn* conn = malloc(sizeof(pb_sq_house_room_conn));
                unsigned adj;
                if (!conn) goto err_return;
                
                conn->neighbour = floor->rooms + j;
                conn->overlap_start = start;
                conn->overlap_end = end;
                conn->door = 0;
                conn->wall = shared_wall;

                /* Check whether the room spec for room i allows a connection to room j */
                for (adj = 0; adj < spec->num_adjacent; ++adj) {
                    if (strcmp((char*)floor->rooms[j].data, spec->adjacent[adj]) == 0) {
                        conn->door = 1;
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
    {
        /* Free all the edge info we just created */
        size_t i;
        for (i = 0; i < g->edges->cap; ++i) {
            if (g->edges->states[i] == FULL) {
                free(((pb_edge*)g->edges->entries[i].val)->data);
            }
        }
    }

    pb_graph_free(g);
    return NULL;
}