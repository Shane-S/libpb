#include <math.h>
#include <pb/internal/sq_house_graph.h>
#include <pb/util/float_utils.h>
#include <pb/util/hashmap/hash_utils.h>
#include <pb/util/pair/pair.h>
#include <limits.h>
#include <pb/util/float_utils.h>
#include <string.h>
#include <pb/sq_house.h>
#include <pb/extrusion.h>
#include <pb/util/vector/vector.h>
#include <pb/internal/astar.h>

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

typedef struct {
    /* Input */
    pb_graph* internal_graph;
    pb_vector* hallway_points;
    pb_floor* floor;
    int closest;

    /* Output */
    pb_point2D* start;
    pb_point2D* goal;
    pb_room* start_room;
    float dist;
} pb_hallway_room_selection_params;

static void get_next_pair(pb_hashmap_entry* entry, void* param) {
    pb_hallway_room_selection_params* params = (pb_hallway_room_selection_params*) param;
    pb_room* room = (pb_room*)entry->val;

    pb_point2D* fpoints = (pb_point2D*)params->floor->shape.points.items;
    pb_point2D* room_points = (pb_point2D*)room->shape.points.items;
    pb_point2D** hpoints = (pb_point2D**)params->hallway_points->items;

    unsigned i;
    for(i = 0; i < room->shape.points.size; ++i) {
        unsigned j;
        /* Check that the point is an internal point */
        if (!pb_point_eq(&room_points[i], fpoints) &&
            !pb_point_eq(&room_points[i], fpoints + 1) &&
            !pb_point_eq(&room_points[i], fpoints + 2) &&
            !pb_point_eq(&room_points[i], fpoints + 3)) {

            /* Check if we beat the min or max and update if so */
            for(j = 0; j < params->hallway_points->size; ++j) {
                float x_diff = room_points[i].x - hpoints[j]->x;
                float y_diff = room_points[i].y - hpoints[j]->y;
                float dist = x_diff * x_diff + y_diff * y_diff;
                int better = params->closest ? dist < params->dist : dist > params->dist;
                if (better) {
                    params->start = &room_points[i];
                    params->start_room = room;
                    params->goal = hpoints[j];
                    params->dist = dist;
                }
            }
        }
    }
}

static float euclid_squared(pb_vertex const* vert, pb_vertex const* goal) {
    pb_point2D const* v_point = (pb_point2D*)vert->data;
    pb_point2D const* g_point = (pb_point2D*)goal->data;

    float x_diff = v_point->x - g_point->x;
    float y_diff = v_point->y - g_point->y;
    float dist = x_diff * x_diff + y_diff * y_diff;
    return dist;
}

pb_vector* pb_sq_house_get_hallways(pb_floor* f, pb_graph* floor_graph, pb_graph* internal_graph,
                                    pb_hashmap* disconnected) {
    pb_vector hallway_points;
    pb_vector* hallways;
    pb_point2D* fpoints = (pb_point2D*)f->shape.points.items;
    pb_room* room;
    pb_hallway_room_selection_params params;

    pb_vector* astar_points;
    pb_vertex const* start;
    pb_vertex const* goal;

    unsigned i;

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

    if (pb_vector_init(&hallway_points, sizeof(pb_point2D*), 0) == -1) {
        pb_vector_free(hallways);
        return NULL;
    }

    /* Get list of internal goal points, which in this case means points that aren't on a corner of the floor */
    for(i = 0; i < f->rooms->shape.points.size; ++i) {
        pb_point2D const* p = ((pb_point2D*)f->rooms->shape.points.items) + i;
        pb_point2D const* fp = (pb_point2D*)f->shape.points.items;
        if (!pb_point_eq(p, fp) && !pb_point_eq(p, fp + 1) && !pb_point_eq(p, fp + 2) && !pb_point_eq(p, fp + 3)) {
            if (pb_vector_push_back(&hallway_points, &p) == -1) {
                goto err_return;
            }
        }
    }

    params.internal_graph = internal_graph;
    params.hallway_points = &hallway_points;
    params.floor = f;
    params.closest = 0;
    params.dist = 0.f;

    while(disconnected->size) {
        pb_vector hallway;
        size_t i;
        if (pb_vector_init(&hallway, sizeof(pb_edge*), 0) == -1) {
            goto err_return;
        }

        pb_hashmap_for_each(disconnected, get_next_pair, &params);

        /* Using the closest point to the hallway won't include a full wall segment of the room,
         * so use the point "behind" the closest point as the start point */
        if (params.closest) {
            float x_diff = params.start->x - params.goal->x;
            float y_diff = params.start->y - params.goal->y;
            pb_point2D const* start_points = (pb_shape2D*)params.start_room->shape.points.items;

            /* Figure out which point is the start point */
            for (i = 0; i < params.start_room->shape.points.size; ++i) {
                if (pb_point_eq(params.start, start_points + i)) {
                    break;
                }
            }

            /* If we're moving more in the x direction, use point along the x axis and vice-versa */
            if (fabsf(x_diff) > fabsf(y_diff)) {
                params.start = start_points + (3 - i); /* Maps 1<->2, 0<->3*/
            } else {
                params.start = i == 0 || i == 1 ? 1 - i : (4 % i) + 2; /* Maps 0<->1, 2<->3 */
            }
        }

        /* Remove disconnected room from the list */
        pb_hashmap_remove(disconnected, params.start_room);

        start = pb_graph_get_vertex(internal_graph, params.start);
        goal = pb_graph_get_vertex(internal_graph, params.goal);
        if (pb_astar(start, goal, euclid_squared, &astar_points) == -1) {
            /* Couldn't find a path */
            pb_vector_free(&hallway);
            continue;
        }

        {
            /* TODO: Refactor this garbage to use mixed declarations and code */
            /* Remove any disconnected rooms that this hallway touches */
            pb_vertex** verts = (pb_vertex**)astar_points->items;
            for (i = 0; i < astar_points->size - 1; ++i) {
                pb_point2D* p0 = (pb_point2D*)verts[i]->data;
                pb_point2D* p1 = (pb_point2D*)verts[i + 1]->data;

                pb_edge const* edge = pb_graph_get_edge(internal_graph, p0, p1);
                pb_hashmap_remove(disconnected, ((pb_sq_house_room_conn*)edge->data)->room);
                pb_hashmap_remove(disconnected, ((pb_sq_house_room_conn*)edge->data)->neighbour);

                if (pb_vector_push_back(&hallway, &edge) == -1) {
                    pb_vector_free(&hallway);
                    goto err_return;
                }

                if (pb_vector_push_back(&hallway_points, p0) == -1) {
                    pb_vector_free(&hallway);
                    goto err_return;
                }
            }

            /* Add the last point to the list of points and add the constructed hallway to the hallway list */
            if (pb_vector_push_back(hallways, &hallway) == -1 ||
                pb_vector_push_back(&hallway_points, verts[astar_points->size - 1]->data) == -1) {
                pb_vector_free(&hallway);
                goto err_return;
            }
        }
        params.closest = 1;
        params.dist = INFINITY;
    }

    pb_vector_free(&hallway_points);
    return hallways;

    err_return:
    {
        pb_vector* hallway_items = (pb_vector*)hallways->items;
        for(i = 0; i < hallways->size; ++i) {
            pb_vector_free(hallway_items + i);
        }
        pb_vector_free(&hallway_points);
        pb_vector_free(hallways);
        free(hallways);
    }
}