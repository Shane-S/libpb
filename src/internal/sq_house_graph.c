#include <math.h>
#include <pb/internal/sq_house_graph.h>
#include <pb/util/float_utils.h>
#include <pb/util/hashmap/hash_utils.h>
#include <pb/util/pair/pair.h>
#include <pb/util/float_utils.h>
#include <string.h>
#include <pb/sq_house.h>
#include <pb/extrusion.h>
#include <pb/util/vector/vector.h>

int pb_sq_house_get_shared_wall(pb_room* room1, pb_room* room2) {
    int shares_top = 0;
    int shares_bottom = 0;
    int shares_left = 0;
    int shares_right = 0;

    pb_point2D* points1 = (pb_point2D*)room1->shape.points.items;
    pb_point2D* points2 = (pb_point2D*)room2->shape.points.items;

    /* We need to use approximate float comparisons because the stairs may not have exactly
     * the same coordinates as the rooms on each floor. */
    shares_top = pb_float_approx_eq(points1[0].y, points2[1].y, 5) &&
        points1[0].x < points2[2].x &&
        points1[3].x > points2[1].x;

    shares_bottom = pb_float_approx_eq(points1[1].y, points2[0].y, 5) &&
        points1[1].x < points2[3].x &&
        points1[2].x > points2[0].x;

    shares_right = pb_float_approx_eq(points1[3].x, points2[0].x, 5) &&
        points1[2].y < points2[0].y &&
        points1[3].y > points2[1].y;

    shares_left = pb_float_approx_eq(points1[0].x, points2[3].x, 5) &&
        points1[1].y < points2[3].y &&
        points1[0].y > points2[2].y;

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

void pb_sq_house_get_wall_overlap(pb_room const* room1, pb_room const* room2, int wall, pb_point2D* start, pb_point2D* end) {
    pb_point2D* points1 = (pb_point2D*)room1->shape.points.items;
    pb_point2D* points2 = (pb_point2D*)room2->shape.points.items;


    switch (wall) {
    case SQ_HOUSE_TOP:
    case SQ_HOUSE_BOTTOM:
        start->x = fmaxf(points1[0].x, points2[0].x);
        end->x = fminf(points1[3].x, points2[3].x);

        /* Calculating overlap in the x axis is the same whether the bottom or the top
         * wall is shared, so use this hacky calculation to choose a point on the correct
         * wall from which to get the y coord. */
        start->y = points1[wall].y;
        end->y = start->y;
        return;

    case SQ_HOUSE_RIGHT:
    case SQ_HOUSE_LEFT:
        start->y = fmaxf(points1[2].y, points2[2].y);
        end->y = fminf(points1[3].y, points2[3].y);

        /* Same thing here, just picks right or left point instead of top/bottom */
        start->x = points1[wall].x;
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
 * @param house_spec The house specification, which lists how big doors should be.
 * @param room_specs The map containing room specifications for this house.
 * @param floor      The floor for which the connectivity graph will be generated.
 * @return A graph containing the rooms' connections or NULL on failure.
 */
pb_graph* pb_sq_house_generate_floor_graph(pb_sq_house_house_spec* house_spec, pb_hashmap* room_specs, pb_floor* floor) {
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
        pb_hashmap_get(room_specs, floor->rooms[i].data, (void**)&spec);

        for (j = 0; j < floor->num_rooms; ++j) {
            if (i == j) continue;

            pb_point2D start;
            pb_point2D end;
            int shared_wall = pb_sq_house_get_shared_wall(floor->rooms + i, floor->rooms + j);

            if (shared_wall != -1) {
                pb_sq_house_room_conn* conn = malloc(sizeof(pb_sq_house_room_conn));
                unsigned adj;
                if (!conn) goto err_return;

                pb_sq_house_get_wall_overlap(floor->rooms + i, floor->rooms + j, shared_wall, &start, &end);

                conn->room = floor->rooms + i;
                conn->neighbour = floor->rooms + j;
                conn->overlap_start = start;
                conn->overlap_end = end;
                conn->can_connect = 0;
                conn->wall = shared_wall;

                /* Check whether the room spec for room i allows a connection to room j */
                for (adj = 0; adj < spec->num_adjacent; ++adj) {
                    if (strcmp((char*)floor->rooms[j].data, spec->adjacent[adj]) == 0) {
                        conn->can_connect = 1;

                        /* Check whether there's enough wall surface area to actually fit a door here */
                        float delta;
                        if (pb_float_approx_eq(conn->overlap_start.x, conn->overlap_end.x, 5)) {
                            delta = conn->overlap_end.y - conn->overlap_start.y;
                        } else {
                            delta = conn->overlap_end.x - conn->overlap_start.x;
                        }
                        conn->has_door = delta >= house_spec->door_size;
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
 * rooms (i.e., if every edge has a can_connect value of 0); if so, adds the vert_id to the list
 * of disconnected rooms.
 *
 * If a given room can connect to a neighbour but the neighbour can't connect to that room or vice-versa,
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
    pb_hashmap* disconnected = (pb_hashmap*)inner_pair->first;
    int* err = (int*)inner_pair->second;
    
    size_t i;
    for (i = 0; i < vert->edges_size; ++i) {
        pb_edge* edge = vert->edges[i];
        pb_sq_house_room_conn* conn = (pb_sq_house_room_conn*)edge->data;

        /* Cheat really hard and use the knowledge that from_id and to_id are actually just room pointers
         * and that those room pointers are in fact stored as the data for the vertex to which they correspond */
        pb_edge const* neighbour_edge = pb_graph_get_edge(g, edge->to->data, vert_id);
        pb_sq_house_room_conn* conn2 = (pb_sq_house_room_conn*)neighbour_edge->data;

        /* has_door implies connectivity */
        if(conn->has_door || conn2->has_door) {
            conn->has_door = 1;
            conn2->has_door = 1;
            conn->can_connect = 1;
            conn2->can_connect = 1;

            break;
        } else if (conn->can_connect || conn2->can_connect) {
            /* If just one side said it could connect, then both can connect */
            conn->can_connect = 1;
            conn2->can_connect = 1;
        }
    }

    /* If there were no edges with doors, add this room to the list of disconnected rooms */
    if (i == vert->edges_size) {
        if (pb_hashmap_put(disconnected, (void*)vert_id, vert->data) == -1) {
            *err = 1;
        }
    }
}

pb_hashmap* pb_sq_house_find_disconnected_rooms(pb_graph* floor_graph, pb_floor* floor) {
    pb_hashmap* disconnected = pb_hashmap_create(pb_pointer_hash, pb_pointer_eq);
    int error_occurred = 0;
    pb_pair outer_params;
    pb_pair inner_params = { disconnected, &error_occurred };
    outer_params.first = floor_graph;
    outer_params.second = &inner_params;

    pb_graph_for_each_vertex(floor_graph, process_disconnected_room, &outer_params);

    if (error_occurred) {
        pb_hashmap_free(disconnected);
        return NULL;
    } else {
        /* Make sure that the first room isn't in the disconnected list if there's at least one other
         * disconnected room. If there isn't, then we want to connect it, but if there is, then it
         * will automatically be connected to a hallway since the other room will connect to it. */
        pb_room* first;
        if (pb_hashmap_get(disconnected, &floor->rooms[0], (void**)&first) == 0 && disconnected->size > 1) {
            pb_hashmap_remove(disconnected, &floor->rooms[0]);
        }
        return disconnected;
    }
}

uint32_t pb_point_hash(void const* point) {
    pb_point2D const* p = (pb_point2D*)point;
    uint32_t x_fuzzed = pb_fuzz_float(p->x, 5);
    uint32_t y_fuzzed = pb_fuzz_float(p->y, 5);

    /* This might not actually be half-bad, but we can revisit it if necessary */
    uint32_t hash = x_fuzzed;
    hash += hash * 37;
    hash += y_fuzzed;
    return hash;
}

int pb_point_eq(void const* point1, void const* point2) {
    pb_point2D const* p1 = (pb_point2D*)point1;
    pb_point2D const* p2 = (pb_point2D*)point2;

    uint32_t x1_fuzzed = pb_fuzz_float(p1->x, 5);
    uint32_t y1_fuzzed = pb_fuzz_float(p1->y, 5);

    uint32_t x2_fuzzed = pb_fuzz_float(p2->x, 5);
    uint32_t y2_fuzzed = pb_fuzz_float(p2->y, 5);

    return (x1_fuzzed == x2_fuzzed) && (y1_fuzzed == y2_fuzzed);
}

static void add_internal_points(void const* vert_id, pb_vertex* vert, void* params) {
    pb_pair* params_pair = (pb_pair*)params;
    pb_graph* internal_graph = (pb_graph*)params_pair->first;
    int* error = (int*)params_pair->second;

    /* Add each point from each room to the graph so that we can do hallway pathfinding on it */
    size_t i;
    for(i = 0; i < vert->edges_size; ++i) {
        pb_sq_house_room_conn* conn = (pb_sq_house_room_conn*)vert->edges[i]->data;

        /* Add points and edge between them to graph if they don't already exist */
        if (pb_graph_get_vertex(internal_graph, &conn->overlap_start) == NULL) {
            if (pb_graph_add_vertex(internal_graph, &conn->overlap_start, &conn->overlap_start) == -1) {
                *error = 1;
                return;
            }
        }

        if (pb_graph_get_vertex(internal_graph, &conn->overlap_end) == NULL) {
            if (pb_graph_add_vertex(internal_graph, &conn->overlap_end, &conn->overlap_end) == -1) {
                *error = 1;
                return;
            }
        }

        if (pb_graph_get_edge(internal_graph, &conn->overlap_start, &conn->overlap_end) == NULL) {
            float x_diff_squared = (conn->overlap_end.x - conn->overlap_start.x) *
                                   (conn->overlap_end.x - conn->overlap_start.x);
            float y_diff_squared = (conn->overlap_end.y - conn->overlap_start.y) *
                                   (conn->overlap_end.y - conn->overlap_start.y);

            /* Store the squared distance as the weight - we're only using it for pathfinding */
            float weight = x_diff_squared + y_diff_squared;

            /* Both edges have a reference to the conn, which stores both neighbours in the overlap */
            if (pb_graph_add_edge(internal_graph, &conn->overlap_start, &conn->overlap_end, weight, conn) == -1 ||
                pb_graph_add_edge(internal_graph, &conn->overlap_end, &conn->overlap_start, weight, conn) == -1) {
                *error = 1;
                return;
            }
        }
    }
}

pb_graph* pb_sq_house_generate_internal_graph(pb_graph* floor_graph) {
    pb_graph* internal = pb_graph_create(pb_point_hash, pb_point_eq);
    int error = 0;
    pb_pair params = {internal, &error};

    if (internal == NULL) {
        return NULL;
    }

    pb_graph_for_each_vertex(floor_graph, add_internal_points, &params);
    if (error) {
        pb_graph_free(internal);
        return NULL;
    }

    return internal;
}

pb_vector* pb_sq_house_get_hallways(pb_floor* f, pb_graph* floor_graph, pb_graph* internal_graph,
                                    pb_hashmap* disconnected) {

    pb_vector hallway_points;
    pb_vector* hallways;
    pb_point2D* fpoints = (pb_point2D*)f->shape.points.items;
    pb_room* room;

    hallways = pb_vector_create(sizeof(pb_vector), 0);
    if (!hallways) {
        return NULL;
    }

    /* If the only disconnected room on the floor is room[0] (the room connected to outside on the first floor, or
     * stairs on any other floor), then make a hallway along its largest internal wall and return */
    if (disconnected->size == 1 && pb_hashmap_get(disconnected, &f->rooms[0], (void**)&room) != -1) {
        pb_vector hallway;
        pb_point2D* points = (pb_point2D*)room->shape.points.items;
        pb_vertex const* cur;
        pb_vertex const* goal;

        int max_w = (points[2].x - points[1].x) > (points[0].y - points[1].y);

        /* Determine which walls are internal by checking whether one of their points lies along the house's exterior */
        int internal[4] = {points[0].x != 0.f,
                           points[1].y != 0.f,
                           !pb_float_approx_eq(points[2].x, fpoints[2].x, 5),
                           !pb_float_approx_eq(points[0].y, fpoints[0].y, 5)};

        int wall;
        int direction_is_x;
        /* Try to find an internal wall along the room's larger dimension */
        if (max_w) {
            wall = internal[1] ? 1 : internal[3] ? 3 : internal[0] ? 0 : 2;
        } else {
            wall = internal[0] ? 0 : internal[2] ? 2 : internal[1] ? 1 : 3;
        }

        direction_is_x = (side)wall == SQ_HOUSE_TOP || (side)wall == SQ_HOUSE_BOTTOM;

        if (pb_vector_init(&hallway, sizeof(pb_edge*), 0) == -1) {
            pb_vector_free(hallways);
            free(hallways);
            return NULL;
        }

        cur = pb_graph_get_vertex(internal_graph, points + wall);
        goal = pb_graph_get_vertex(internal_graph, points + ((wall + 1) % room->shape.points.size));

        while(cur != goal) {
            pb_point2D const* cur_point = (pb_point2D*)cur->data;
            size_t i;
            for(i = 0; i < cur->edges_size; ++i) {
                pb_vertex const* to = cur->edges[i]->to;
                pb_point2D const* to_point = (pb_point2D*)cur->edges[i]->to->data;
                if (direction_is_x && to_point->x > cur_point->x) {
                    pb_edge const* edge = pb_graph_get_edge(internal_graph, cur_point, to_point);
                    if (pb_vector_push_back(&hallway, &edge) == -1) {
                        goto room0_err_return;
                    }
                    cur = to;
                    break;
                } else if (to_point->y > cur_point->y) {
                    pb_edge const* edge = pb_graph_get_edge(internal_graph, cur_point, to_point);
                    if (pb_vector_push_back(&hallway, &edge) == -1) {
                        goto room0_err_return;
                    }
                    cur = to;
                    break;
                }
            }
        }

        if (pb_vector_push_back(hallways, &hallway) != -1) {
            return hallways;
        }
        room0_err_return:
            pb_vector_free(&hallway);
            pb_vector_free(hallways);
            free(hallways);
            return NULL;
    }
/*
// First, we find the furthest pair of points
List of valid goal points = list of goal room points that are contained in internal graph
For each disconnected room in disconnected room map
    For each point in disconnected room
        If internal graph contains point
            For each point in valid goal points
                If distance (room point, goal point) > max // Well distance squared since we only care that it's bigger
                    start point = room point
                    end point = goal point

Remove start room from disconnected list

A* points = A* (start point, end point)
First hallway's edges = empty list
For each point in A* points - 1
    Get edge from point to point + 1
    Remove edge.data.room and edge.data.neighbour from disconnected list
    Add edge to first hallway's edges

Append result of A* points to hallway points
Add first hallway's edges to list of lists
Free A* points

For each disconnected room
    Find closest point in hallway points
    Get suitable point on wall segment to get to closest hallway point
    A* points = Perform A* (suitable point on wall segment, closest hallway point)

    Remove start room from disconnected list

    D/C Room hallway edges = empty list
    // This is actually safe since entries are processed "in order"; we can't skip any
    For each point in A* points -1
        Get edge from point to point - 1
        Remove edge.data.room and edge.data.neighbour from disconnected list
        Add edge to D/C Room hallway edges

    Add D/C Room hallway edges to edges list of lists

Free hallway points
Return hallway edges list of lists
     */
}