#include <math.h>
#include <limits.h>
#include <string.h>
#include <pb/sq_house.h>
#include <pb/extrusion.h>
#include <pb/internal/astar.h>
#include <pb/internal/sq_house_graph.h>
#include <pb/util/vector/vector.h>
#include <pb/util/pair/pair.h>
#include <pb/util/hashmap/hash_utils.h>
#include <pb/util/float_utils.h>
#include <pb/util/geom/rect_utils.h>
#include <pb/floor_plan.h>
#include <stdio.h>

int pb_sq_house_get_shared_wall(pb_rect* room1_rect, pb_rect* room2_rect) {
    int shares_top = 0;
    int shares_bottom = 0;
    int shares_left = 0;
    int shares_right = 0;

    pb_point2D points1[4];
    points1[0] = room1_rect->bottom_left;
    points1[1] = room1_rect->bottom_left;
    points1[2] = room1_rect->bottom_left;
    points1[3] = room1_rect->bottom_left;

    points1[0].y += room1_rect->h;
    points1[2].x += room1_rect->w;
    points1[3].x += room1_rect->w;
    points1[3].y += room1_rect->h;

    pb_point2D points2[4];
    points2[0] = room2_rect->bottom_left;
    points2[1] = room2_rect->bottom_left;
    points2[2] = room2_rect->bottom_left;
    points2[3] = room2_rect->bottom_left;

    points2[0].y += room2_rect->h;
    points2[2].x += room2_rect->w;
    points2[3].x += room2_rect->w;
    points2[3].y += room2_rect->h;

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
    } else if (shares_right) {
        return SQ_HOUSE_RIGHT;
    } else if (shares_left) {
        return SQ_HOUSE_LEFT;
    } else if (shares_bottom) {
        return SQ_HOUSE_BOTTOM;
    } else {
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
        pb_rect roomi_rect;
        pb_shape2D_to_pb_rect(&floor->rooms[i].shape, &roomi_rect);

        /* Stairs and hallways have no associated room spec; they can connect to any adjacent rooms */
        int roomi_has_spec = pb_hashmap_get(room_specs, floor->rooms[i].name, (void**)&spec) != -1;


        for (j = 0; j < floor->num_rooms; ++j) {
            if (i == j) continue;

            pb_point2D start;
            pb_point2D end;

            pb_rect roomj_rect;
            pb_shape2D_to_pb_rect(&floor->rooms[j].shape, &roomj_rect);
            
            int shared_wall = pb_sq_house_get_shared_wall(&roomi_rect, &roomj_rect);

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
                conn->has_door = 0;
                conn->wall = shared_wall;
                conn->can_connect = 1;

                float delta;
                if (pb_float_approx_eq(conn->overlap_start.x, conn->overlap_end.x, 5)) {
                    delta = conn->overlap_end.y - conn->overlap_start.y;
                } else {
                    delta = conn->overlap_end.x - conn->overlap_start.x;
                }
                conn->has_door = delta > house_spec->door_size;

//                if (!roomi_has_spec) {
//                    conn->can_connect = 1;
//
//                    /* Check whether there's enough wall surface area to actually fit a door here */
//                    float delta;
//                    if (pb_float_approx_eq(conn->overlap_start.x, conn->overlap_end.x, 5)) {
//                        delta = conn->overlap_end.y - conn->overlap_start.y;
//                    } else {
//                        delta = conn->overlap_end.x - conn->overlap_start.x;
//                    }
//                    conn->has_door = delta >= house_spec->door_size;
//                } else {
//                    /* Check whether the room spec for room i allows a connection to room j */
//                    for (adj = 0; adj < spec->num_adjacent; ++adj) {
//                        if (strcmp(floor->rooms[j].name, spec->adjacent[adj]) == 0) {
//                            conn->can_connect = 1;
//
//                            /* Check whether there's enough wall surface area to actually fit a door here */
//                            float delta;
//                            if (pb_float_approx_eq(conn->overlap_start.x, conn->overlap_end.x, 5)) {
//                                delta = conn->overlap_end.y - conn->overlap_start.y;
//                            } else {
//                                delta = conn->overlap_end.x - conn->overlap_start.x;
//                            }
//                            conn->has_door = delta >= house_spec->door_size;
//                            break;
//                        }
//                    }
//                }

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
 * Checks if the room stored in a given vertex doesn't have any valid connections to neighbouring
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

    // Disable disconnected rooms for now - hallway placement is pretty broken
    return disconnected;

//
//
//    pb_graph_for_each_vertex(floor_graph, process_disconnected_room, &outer_params);
//
//    if (error_occurred) {
//        pb_hashmap_free(disconnected);
//        return NULL;
//    } else {
//        /* Make sure that the first room isn't in the disconnected list if there's at least one other
//         * disconnected room. If there isn't, then we want to connect it, but if there is, then it
//         * will automatically be connected to a hallway since the other room will connect to it. */
//        pb_room* first;
//        if (pb_hashmap_get(disconnected, &floor->rooms[0], (void**)&first) == 0 && disconnected->size > 1) {
//            pb_hashmap_remove(disconnected, &floor->rooms[0]);
//        }
//        return disconnected;
//    }
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
    pb_point2D const* start;
    pb_point2D const* goal;
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

        /* Determine which walls are internal by checking whether their centre points lie along the exterior */
        pb_point2D wall0_centre = {points[0].x, points[0].y + (points[1].y - points[0].y) / 2.f};
        pb_point2D wall1_centre = {points[1].x + (points[2].x - points[1].x) / 2.f, points[1].y};;
        pb_point2D wall2_centre = {points[2].x, points[2].y + (points[3].y - points[2].y) / 2.f};
        pb_point2D wall3_centre = {points[3].x + (points[0].x - points[3].x) / 2.f, points[3].y};

        int internal[4] = {wall0_centre.x != 0.f,
                           wall1_centre.y != 0.f,
                           !pb_float_approx_eq(wall2_centre.x, fpoints[2].x, 5),
                           !pb_float_approx_eq(wall3_centre.y, fpoints[0].y, 5)};

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
            pb_point2D const* start_points = (pb_point2D*)params.start_room->shape.points.items;

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
                params.start = start_points + (i == 0 || i == 1 ? 1 - i : (4 % i) + 2); /* Maps 0<->1, 2<->3 */
            }
        }

        /* Remove disconnected room from the list */
        pb_hashmap_remove(disconnected, params.start_room);

        start = pb_graph_get_vertex(internal_graph, params.start);
        goal = pb_graph_get_vertex(internal_graph, params.goal);
        if (pb_astar(start, goal, euclid_squared, &astar_points) == -1) {
            /* Couldn't find a path - set min and max so that the next one has a chance at beating it */
            pb_vector_free(&hallway);
            params.dist = params.closest ? INFINITY : -INFINITY;
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
                pb_vector_free(astar_points);
                free(astar_points);
                pb_vector_free(&hallway);
                goto err_return;
            }

            pb_vector_free(astar_points);
            free(astar_points);
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
        return NULL;
    }
}

/**
 * Determines whether the given room and hallway intersect, and if so, adjusts the room's shape and stores all overlapping
 * pairs between the resulting shape and the hallway in the overlap_pairs vector.
 *
 * @param room_rect     The room's bounding rectangle.
 * @param hallway_rect  The hallway room's bounding rectangle.
 * @param room_shape    The room's actual 2D shape.
 * @param overlap_pairs The pairs of points where the two shapes overlap. Should be allocated to hold 6 points, which is the max
 *                      number of overlap points possible.
 *
 * @return Whether the shapes overlapped.
 *
 * TODO: Move duplicate detection outside this function. Pretty sure it can be done more efficiently that way.
 */
static int intrude_hallway(pb_rect const* room_rect, pb_rect const* hallway_rect, pb_shape2D* room_shape,
                           pb_point2D* ovlap0, pb_point2D* ovlap1) {
    
    pb_point2D* room_points = (pb_point2D*)room_shape->points.items;

    pb_point2D rrect[4];
    pb_point2D hrect[4];

    /* I could use pb_rect_to_pb_shape2D, but I want to do this on the stack */
    rrect[1] = room_rect->bottom_left;
    rrect[0] = rrect[1];
    rrect[0].y += room_rect->h;
    rrect[2] = rrect[1];
    rrect[2].x += room_rect->w;
    rrect[3] = rrect[0];
    rrect[3].x += room_rect->w;

    hrect[1] = hallway_rect->bottom_left;
    hrect[0] = hrect[1];
    hrect[0].y += hallway_rect->h;
    hrect[2] = hrect[1];
    hrect[2].x += hallway_rect->w;
    hrect[3] = hrect[0];
    hrect[3].x += hallway_rect->w;

    /* Figure out which hallway points the room's bounding rectangle contains */
    pb_point2D* hc0 = NULL;
    pb_point2D* hc1 = NULL;
    int hc0_idx;
    int hc1_idx;
    int i;
    for (i = 0; i < 4; ++i) {
        if (pb_rect_contains_point(room_rect, &hrect[i])) {
            if (hc0 == NULL) {
                hc0 = &hrect[i];
                hc0_idx = i;
            } else {
                hc1 = &hrect[i];
                hc1_idx = i;
                break;
            }
        }
    }

    pb_point2D* rc0 = NULL;
    pb_point2D* rc1 = NULL;
    int rc0_idx;
    int rc1_idx;
    for (i = 0; i < 4; ++i) {
        if (pb_rect_contains_point(hallway_rect, &rrect[i])) {
            if (rc0 == NULL) {
                rc0 = &rrect[i];
                rc0_idx = i;
            }
            else {
                rc1 = &rrect[i];
                rc1_idx = i;
                break;
            }
        }
    }

    if (hc0) {
        if (hc1) {
            /* Room rect contains two of the hallway rect's points
             * Possible cases:
             *  _____________   _______________   _____________
             * |             | |               | |             |
             * |             | |               | |             |
             * |    ______   | |______         | |_____________|
             * |___|______|__| |______|________| |_____________|
             *     |______|    |______|          |_____________|
             *
             */
            if (rc0 && rc1) {
                /* Last case - hallway does not abut any other hallways, happens to match the wall exactly 
                 * Because hallways will never overlap, we know that both of these points still exist in the actual 
                 * room shape, so find them and push them to the correct spots */
                pb_point2D* real_rc0 = NULL;
                pb_point2D* real_rc1 = NULL;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(rc0, room_points + i)) {
                        real_rc0 = room_points + i;
                        real_rc1 = room_points + i + 1;
                        break;
                    }
                }

                /* Determine where to put the points */
                float xdiff = rc1->x - rc0->x;
                float ydiff = rc1->y - rc0->y;
                int is_x = fabsf(xdiff) > fabsf(ydiff);
                
                pb_point2D first;
                pb_point2D second;

                /* There will be no duplicates in this case - if there were vertical hallways, they'd
                 * cause the horizontal hallway's start and/or end to be offset from the edge. */
                if (is_x) {
                    real_rc0->y = hc0->y;
                    real_rc1->y = hc0->y;

                    *ovlap0 = real_rc0->x < real_rc1->x ? *real_rc0 : *real_rc1;
                    *ovlap1 = real_rc0->x < real_rc1->x ? *real_rc1 : *real_rc0;
                }
                else {
                    real_rc0->x = hc0->x;
                    real_rc1->x = hc1->x;
                    *ovlap0 = real_rc0->y < real_rc1->y ? *real_rc0 : *real_rc1;
                    *ovlap1 = real_rc0->y < real_rc1->y ? *real_rc1 : *real_rc0;
                }
                return 1;
            } else if (rc0) {
                /* Second case - hallway possibly abutting another hallway along the other axis on one side */
                /* Move the room point to the hallway edge point and add points to the room's shape, removing any
                 * duplicates found */
                float hcxdiff = hc0->x - hc1->x;
                float hcydiff = hc0->y - hc1->y;
                int is_x = fabsf(hcxdiff) > fabsf(hcydiff);

                int delta_mult = is_x ? (hc0_idx == 0 ? -1 : 1) : (hcydiff < 0 ? -1 : 1);
                pb_point2D* edge_point = is_x ? (pb_float_approx_eq(hc0->x, rc0->x, 5) ? hc0 : hc1)
                                              : (pb_float_approx_eq(hc0->y, rc0->y, 5) ? hc0 : hc1);
                pb_point2D* non_edge_point = edge_point == hc0 ? hc1 : hc0;
                pb_point2D intersect;

                if (is_x) {
                    intersect.x = non_edge_point->x; 
                    intersect.y = non_edge_point->y + (hallway_rect->h / 2 * delta_mult);
                } else {
                    intersect.x = non_edge_point->x + (hallway_rect->w / 2 * delta_mult);
                    intersect.y = non_edge_point->y;
                }
                
                size_t real_rc0_idx;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(rc0, room_points + i)) {
                        real_rc0_idx = (size_t)i;
                        break;
                    }
                }

                /* For both axes, if the previous point's x or y coordinate (when is_x and !is_x, respectively) 
                 * isn't equal to ours, it means we're at the end of one of the rectangle's edges when following
                 * the points CCW; in that case, we need to insert the points before us. In the other case, we need to
                 * insert them after us. E.g. if we're at point 2 and are going along the x axis, then we're at the end
                 * of wall 1 and need to insert the non-edge hallway rectangle point, then the intersect point, both at point
                 * 2 such that the order is intersect->non-edge->point 2. */
                size_t insert_idx_add = (is_x * pb_float_approx_eq(rc0->x, rrect[rc0_idx == 0 ? 3 : rc0_idx - 1].x, 5)) +
                                        (!is_x * pb_float_approx_eq(rc0->y, rrect[rc0_idx == 0 ? 3 : rc0_idx - 1].y, 5));
                size_t insert_idx = (real_rc0_idx + insert_idx_add) % room_shape->points.size;
                
                int removed_intersect = 0;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(room_points + i, &intersect)) {
                        pb_vector_remove_at(&room_shape->points, i);
                        removed_intersect = 1;
                        real_rc0_idx = i < real_rc0_idx ? real_rc0_idx - 1 : real_rc0_idx;
                        insert_idx = i < insert_idx ? insert_idx - 1 : insert_idx;
                        break;
                    }
                }
                
                int removed_non_edge = 0;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(room_points + i, non_edge_point)) {
                        pb_vector_remove_at(&room_shape->points, i);
                        removed_non_edge = 1;
                        real_rc0_idx = i < real_rc0_idx ? real_rc0_idx - 1 : real_rc0_idx;
                        insert_idx = i < insert_idx ? insert_idx - 1 : insert_idx;
                    }
                }

                int result = 0;
                if (!removed_non_edge) {
                    if (removed_intersect) {
                        /* The non-edge point is still guaranteed to to be in the shape, so insert just that */
                        if (insert_idx == 0) {
                            result = pb_vector_push_back(&room_shape->points, non_edge_point);
                        }
                        else {
                            result = pb_vector_insert_at(&room_shape->points, non_edge_point, insert_idx);
                        }
                    }
                    else {
                        pb_point2D* insert_first = insert_idx_add ? &intersect : non_edge_point;
                        pb_point2D* insert_second = insert_idx_add ? non_edge_point : &intersect;

                        if (insert_idx == 0) {
                            result = pb_vector_push_back(&room_shape->points, insert_second);
                            result = result == -1 ? -1 : pb_vector_push_back(&room_shape->points, insert_first);
                        } else {
                            result = pb_vector_insert_at(&room_shape->points, insert_first, insert_idx);
                            result = result == -1 ? -1 : pb_vector_insert_at(&room_shape->points, insert_second, insert_idx);
                        }
                    }
                }
                
                if (result == -1) {
                    return -1;
                }
                room_points = (pb_point2D*)room_shape->points.items; /* Might have changed while inserting */
                real_rc0_idx = insert_idx == 0 || insert_idx_add ? real_rc0_idx : real_rc0_idx + 2 - removed_intersect - removed_non_edge;
                room_points[real_rc0_idx] = *edge_point;

                pb_point2D const* start = is_x ? (edge_point->x < non_edge_point->x ? edge_point : non_edge_point)
                                               : (edge_point->y < non_edge_point->y ? edge_point : non_edge_point);
                pb_point2D const* end = start == non_edge_point ? edge_point : non_edge_point;
                *ovlap0 = *start;
                *ovlap1 = *end;

                return 1;
            } else {
                /* First case - hallway abutting a hallway along the other axis, possibly on both sides */
                /* Find intersection points - if the room already contains any of those points, remove them 
                 * Add remaining points and mark them as overlaps */
                float xdiff = hc0->x - hc1->x;
                float ydiff = hc0->y - hc1->y;
                int is_x = fabsf(xdiff) > fabsf(ydiff);

                float cmpdiff = is_x ? xdiff : ydiff;
                pb_point2D* point0 = cmpdiff < 0 ? hc0 : hc1;
                pb_point2D* point1 = cmpdiff < 0 ? hc1 : hc0;
                pb_point2D intersect0 = cmpdiff > 0 ? hrect[1 + is_x] : hrect[0 + is_x];
                pb_point2D intersect1 = cmpdiff > 0 ? hrect[0 + is_x] : hrect[1 + is_x];

                pb_point2D* rect_point;
                size_t rect_point_idx = (size_t)-1;

                if (is_x) {
                    float intersect_y = hallway_rect->bottom_left.y < room_rect->bottom_left.y
                                        ? room_rect->bottom_left.y : room_rect->bottom_left.y + room_rect->h;
                    intersect0.y = intersect_y;
                    intersect1.y = intersect_y;
                } else {
                    float intersect_x = hallway_rect->bottom_left.x < room_rect->bottom_left.x
                                        ? room_rect->bottom_left.x : room_rect->bottom_left.x + room_rect->w;
                    intersect0.x = intersect_x;
                    intersect1.x = intersect_x;
                }

                size_t intersect0_idx = (size_t)-1;
                size_t intersect1_idx = (size_t)-1;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (is_x ? pb_float_approx_eq(intersect0.y, room_points[i].y, 5)
                             : pb_float_approx_eq(intersect0.x, room_points[i].x, 5)) {
                        rect_point = room_points + i;
                        rect_point_idx = (size_t)i;
                        break;
                    }

                    if (pb_point_eq(room_points + i, &intersect0)) {
                        intersect0_idx = (size_t)i;
                    } else if (pb_point_eq(room_points + i, &intersect1)) {
                        intersect1_idx = (size_t)i;
                    }
                }

                if (intersect0_idx != -1) {
                    pb_vector_remove_at(&room_shape->points, intersect0_idx);
                    rect_point_idx = rect_point_idx < intersect0_idx ? rect_point_idx - 1 : rect_point_idx;
                }

                if (intersect1_idx != -1) {
                    intersect1_idx = intersect0_idx < intersect1_idx ? intersect1_idx - 1 : intersect1_idx;
                    rect_point_idx = rect_point_idx < intersect1_idx ? rect_point_idx - 1 : rect_point_idx;
                    pb_vector_remove_at(&room_shape->points, intersect1_idx);
                }

                if (rect_point_idx == (size_t)-1) {
                    printf("Insert idx wasn't initialised\n");
                    printf("intersect 0: (%f, %f)\n", intersect0.x, intersect0.y);
                    printf("intersect 1: (%f, %f)\n", intersect1.x, intersect1.y);
                    printf("room rect: (%f, %f), w: %f, h: %f\n", room_rect->bottom_left.x, room_rect->bottom_left.y,
                           room_rect->w, room_rect->h);
                    printf("hallway rect: (%f, %f), w: %f, h: %f\n", hallway_rect->bottom_left.x, hallway_rect->bottom_left.y,
                           hallway_rect->w, hallway_rect->h);
                    for (i = 0; i < room_shape->points.size; ++i) {
                        printf("room point %d: (%f, %f)\n", i, room_points[i].x, room_points[i].y);
                    }
                }

                size_t insert_idx = (rect_point_idx + 1) % room_shape->points.size;
                int result = 0;
                int removed_intersect0 = intersect0_idx != -1;
                int removed_intersect1 = intersect1_idx != -1;
                if (insert_idx == 0) {
                    if (!removed_intersect0) {
                        result = pb_vector_push_back(&room_shape->points, &intersect0);
                    }
                    result = result == -1 ? -1 : pb_vector_push_back(&room_shape->points, point0);
                    result = result == -1 ? -1 : pb_vector_push_back(&room_shape->points, point1);
                    if (!removed_intersect1) {
                        result = result == -1 ? -1 : pb_vector_push_back(&room_shape->points, &intersect1);
                    }
                    
                } else {
                    if (!removed_intersect1) {
                        result = pb_vector_insert_at(&room_shape->points, &intersect1, insert_idx);
                    }
                    result = result == -1 ? -1 : pb_vector_insert_at(&room_shape->points, point1, insert_idx);
                    result = result == -1 ? -1 : pb_vector_insert_at(&room_shape->points, point0, insert_idx);
                    if (!removed_intersect0) {
                        result = result == -1 ? -1 : pb_vector_insert_at(&room_shape->points, &intersect0, insert_idx);
                    }
                }
                if (result == -1) {
                    return -1;
                }

                *ovlap0 = *point0;
                *ovlap1 = *point1;

                return 1;
            }
        } else {
            /* Room rect contains one of the hallway rect's points; possible cases: 
             *  _____________       ____________
             * |             |     |            |
             * |             |     |            |
             * |    _________|__   |____________|___________
             * |___|_________|  |  |____________|
             *     |____________|  |________________________
             */
            int is_edge = 0;
            for (i = 0; i < 3; ++i) {
                if (pb_float_approx_eq(rrect[i].x, hc0->x, 5) || pb_float_approx_eq(rrect[i].y, hc0->y, 5)) {
                    is_edge = 1;
                    break;
                }
            }

            if (is_edge) {
                if (!rc1) {
                    /* OK, so there was a third case...
                     * Hallway overlaps an edge, but does NOT actually intrude the shape.
                     * Fortunately, we can just return here since it doesn't matter.
                     *
                     * Talk about edge cases though, god damn...
                     *  ____________
                     * |            |
                     * |            |
                     * |            |
                     * |            |
                     * |____________|
                     * |   |
                     * |   |
                     * */
                    return 0;
                }
                /* Second case */
                int is_x = fabsf(rc0->x - rc1->x) > fabsf(rc0->y - rc1->y);
                pb_point2D* real_rc0 = NULL;
                pb_point2D* real_rc1 = NULL;
                int real_rc0_idx;
                int real_rc1_idx;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(rc0, room_points + i)) {
                        size_t prev_idx = i == 0 ? room_shape->points.size - 1 : i - 1;
                        size_t next_idx = (i + 1) % room_shape->points.size;

                        pb_point2D* prev = room_points + prev_idx;
                        
                        int prev_is_x = fabsf(rc0->x - prev->x) > fabsf(rc0->y - prev->y);

                        int is_prev = (is_x && prev_is_x) || (!is_x && !prev_is_x);

                        real_rc0_idx = i;
                        real_rc0 = room_points + i;
                        real_rc1_idx = is_prev ? prev_idx : next_idx;
                        real_rc1 = room_points + real_rc1_idx;
                        break;
                    }
                }

                pb_point2D real_rc0_adjusted = *real_rc0;
                pb_point2D real_rc1_adjusted = *real_rc1;

                pb_point2D* first;
                pb_point2D* second;
                if (is_x) {
                    real_rc0_adjusted.y = hc0->y;
                    real_rc1_adjusted.y = hc0->y;

                    first = real_rc0->x < real_rc1->x ? &real_rc0_adjusted : &real_rc1_adjusted;
                    second = real_rc0->x < real_rc1->x ? &real_rc1_adjusted : &real_rc0_adjusted;
                } else {
                    real_rc0_adjusted.x = hc0->x;
                    real_rc1_adjusted.x = hc0->x;

                    first = real_rc0->y < real_rc1->y ? &real_rc0_adjusted : &real_rc1_adjusted;
                    second = real_rc0->y < real_rc1->y ? &real_rc1_adjusted : &real_rc0_adjusted;
                }

                int removed_rc0 = 0;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(room_points + i, &real_rc0_adjusted)) {
                        size_t delta_rc1_idx = (i < real_rc1_idx) + (real_rc0_idx < real_rc1_idx);
                        real_rc1_idx -= delta_rc1_idx;

                        real_rc0_idx -= i < real_rc0_idx;
                        pb_vector_remove_at(&room_shape->points, i);                       
                        pb_vector_remove_at(&room_shape->points, real_rc0_idx);

                        removed_rc0 = 1;
                        break;
                    }
                }
                if (!removed_rc0) {
                    room_points[real_rc0_idx] = real_rc0_adjusted;
                }

                int removed_rc1 = 0;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(room_points + i, &real_rc1_adjusted)) {
                        
                        removed_rc1 = 1;
                        real_rc1_idx -= i < real_rc1_idx;
                        
                        pb_vector_remove_at(&room_shape->points, i);
                        pb_vector_remove_at(&room_shape->points, real_rc1_idx);
                        
                        break;
                    }
                }
                if (!removed_rc1) {
                    room_points[real_rc1_idx] = real_rc1_adjusted;
                }

                *ovlap0 = *first;
                *ovlap1 = *second;
            } else {
                /* First case */

                /* There's a currently unfixed edge case here: 
                 *  ______|__   |
                 * |      |  |  |
                 * |      |  |  |
                 * |      |__|__|___
                 * |_______|_|
                 *         | 
                 *         |_________
                 *
                 * The bottom horizontal rectangle is a hallway that just barely overlaps the top-left rectangle (a room).
                 * The other rectangle is a vertical hallway that abuts the horizontal one. If the overlap between the horizontal
                 * hallway and the room is < hallway_size / 2, then the abutting vertical hallway will push the overlap point
                 * (the intersect between the horizontal hallway's top wall and the room's right wall) behind the horizontal
                 * hallway's top-left corner, which means that that's no longer a valid overlap. Looking down the vertical hallway
                 * towards the room would then have part of the wall not present, and the inside of the room would be visible.
                 */

                pb_point2D delta = { rc0->x - hc0->x, rc0->y - hc0->y };
                pb_point2D intersect0 = { hc0->x + delta.x, hc0->y };
                pb_point2D intersect1 = { hc0->x, hc0->y + delta.y };

                pb_point2D* larger_ovlap_point = fabsf(delta.x) > fabsf(delta.y) ? &intersect0 : &intersect1;
                pb_point2D* ovlap0_point = larger_ovlap_point == &intersect0 ? (hc0->x < intersect0.x ? hc0 : &intersect0)
                                                                             : (hc0->y < intersect1.y ? hc0 : &intersect1);
                *ovlap0 = *ovlap0_point;
                *ovlap1 = ovlap0_point == hc0 ? *larger_ovlap_point : *hc0;

                int removed_intersect0 = 0;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(room_points + i, &intersect0)) {
                        pb_vector_remove_at(&room_shape->points, i);
                        removed_intersect0 = 1;
                        break;
                    }
                }

                int removed_intersect1 = 0;
                if (!removed_intersect0) {
                    for (i = 0; i < room_shape->points.size; ++i) {
                        if (pb_point_eq(room_points + i, &intersect1)) {
                            pb_vector_remove_at(&room_shape->points, i);
                            removed_intersect1 = 1;
                            break;
                        }
                    }
                }
                
                int would_remove_corner = 0;
                size_t corner_idx;
                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(room_points + i, hc0)) {
                        would_remove_corner = 1;
                        corner_idx = i;
                        break;
                    }
                }

                /* Find the corner point in the rectangle */
                pb_point2D* real_rc0;
                size_t real_rc0_idx;

                for (i = 0; i < room_shape->points.size; ++i) {
                    if (pb_point_eq(rc0, room_points + i)) {
                        real_rc0 = room_points + i;
                        real_rc0_idx = i;
                        break;
                    }
                }

                if (would_remove_corner) {
                    /* If we would have removed the corner, then there will only be one intersection point left, so
                     * move the contained corner point to there and then remove it */
                    room_points[real_rc0_idx] = removed_intersect0 ? intersect1 : intersect0;
                    pb_vector_remove_at(&room_shape->points, corner_idx);
                } else if (removed_intersect0 || removed_intersect1) {
                    /* The corner and 1 of the intersects are left; move the contained point to the corner and add
                     * the remaining intersect in the correct spot */
                    pb_point2D* first = !(hc0_idx % 2) ? &intersect0 : &intersect1;
                    pb_point2D* second = !(hc0_idx % 2) ? &intersect1 : &intersect0;
                    size_t insert_idx = first == &intersect0 ? (removed_intersect0 ? real_rc0_idx : real_rc0_idx + 1)
                                                             : (removed_intersect1 ? real_rc0_idx : real_rc0_idx + 1);
                    pb_point2D* to_insert = removed_intersect0 ? &intersect1 : &intersect0;
                    insert_idx = insert_idx % room_shape->points.size;

                    room_points[real_rc0_idx] = *hc0;
                    
                    int result = 0;
                    if (insert_idx == 0) {
                        result = pb_vector_push_back(&room_shape->points, to_insert);
                    } else {
                        result = pb_vector_insert_at(&room_shape->points, to_insert, insert_idx);
                    }

                    if (result == -1) {
                        return -1;
                    }
                } else {
                    /* Otherwise, all 3 points are left (any other hallway that would share an intersection
                     * point with this one would also share the corner). */
                    pb_point2D* first = !(hc0_idx % 2) ? &intersect1 : &intersect0;
                    pb_point2D* second = !(hc0_idx % 2) ? &intersect0 : &intersect1;

                    int result = 0;

                    *real_rc0 = *hc0;
                    if (real_rc0_idx == room_shape->points.size - 1) {
                        result = pb_vector_push_back(&room_shape->points, second);
                        result = result == -1 ? -1 : pb_vector_insert_at(&room_shape->points, first, real_rc0_idx);
                    } else if (real_rc0_idx == 0) {
                        result = pb_vector_insert_at(&room_shape->points, second, real_rc0_idx + 1);
                        result = result == -1 ? -1 : pb_vector_push_back(&room_shape->points, first);
                    } else {
                        result = pb_vector_insert_at(&room_shape->points, second, real_rc0_idx + 1);
                        result = result == -1 ? -1 : pb_vector_insert_at(&room_shape->points, first, real_rc0_idx);
                    }

                    if (result == -1) {
                        return -1;
                    }
                }
            }
            return 1;
        }
    } else {
        /* Neither room contains any of the other room's points - there's no overlap */
        if (!rc0) {
            return 0;
        } else {
            pb_point2D* real_rc0 = NULL;
            pb_point2D* real_rc1 = NULL;
            pb_point2D real_rc0_adjusted;
            pb_point2D real_rc1_adjusted;
            size_t real_rc0_idx;
            size_t real_rc1_idx;

            float xdiff = rc0->x - rc1->x;
            float ydiff = rc0->y - rc1->y;
            int is_x = fabsf(xdiff) > fabsf(ydiff);

            for (i = 0; i < room_shape->points.size; ++i) {
                if (pb_point_eq(rc0, room_points + i)) {
                    size_t prev_idx = i == 0 ? room_shape->points.size - 1 : i - 1;
                    size_t next_idx = (i + 1) % room_shape->points.size;

                    pb_point2D* prev = room_points + prev_idx;

                    int prev_is_x = fabsf(rc0->x - prev->x) > fabsf(rc0->y - prev->y);

                    int is_prev = (is_x && prev_is_x) || (!is_x && !prev_is_x);

                    real_rc0_idx = i;
                    real_rc0 = room_points + i;
                    real_rc1_idx = is_prev ? prev_idx : next_idx;
                    real_rc1 = room_points + real_rc1_idx;
                    break;
                }
            }

            real_rc0_adjusted = *real_rc0;
            real_rc1_adjusted = *real_rc1;

            /* If the room didn't contain any of the hallway rect's points, but the hallway contained one point,
             * then it definitely contains two points and overlaps a whole wall:
             *           _____________
             *          |             |
             *          |             |
             *     _____|_____________|__
             *          |_____________|  |
             *     ______________________|
             */
            if (is_x) {
                if (xdiff > 0) {
                    /* Room's top wall - use hallway bottom wall y-coord */
                    real_rc0_adjusted.y = hrect[2].y;
                    real_rc1_adjusted.y = hrect[2].y;
                } else {
                    /* Room's bottom wall - use hallway top wall y-coord */
                    real_rc0_adjusted.y = hrect[0].y;
                    real_rc1_adjusted.y = hrect[0].y;
                }
            } else {
                if (ydiff > 0) {
                    /* Room's left wall - use hallway right wall x-coord */
                    real_rc0_adjusted.x = hrect[2].x;
                    real_rc1_adjusted.x = hrect[2].x;
                } else {
                    /* Room's right wall - user hallway left wall x-coord */
                    real_rc0_adjusted.x = hrect[0].x;
                    real_rc1_adjusted.x = hrect[0].x;
                }
            }

            int removed_realrc0 = 0;
            for (i = 0; i < room_shape->points.size; ++i) {
                if (pb_point_eq(room_points + i, &real_rc0_adjusted)) {
                    
                    real_rc1_idx -= (i < real_rc1_idx) + (real_rc0_idx < real_rc1_idx);

                    removed_realrc0 = 1;
                    real_rc0_idx -= i < real_rc0_idx;
                    
                    pb_vector_remove_at(&room_shape->points, i);
                    pb_vector_remove_at(&room_shape->points, real_rc0_idx);
                    break;
                }
            }
            if (i == room_shape->points.size) {
                room_points[real_rc0_idx] = real_rc0_adjusted;
            }

            for (i = 0; i < room_shape->points.size; ++i) {
                if (pb_point_eq(room_points + i, &real_rc1_adjusted)) {
                    real_rc1_idx -= i < real_rc1_idx;
                    pb_vector_remove_at(&room_shape->points, i);
                    pb_vector_remove_at(&room_shape->points, real_rc1_idx);
                    break;
                }
            }
            if (i == room_shape->points.size) {
                room_points[real_rc1_idx] = real_rc1_adjusted;
            }

            pb_point2D const* first = is_x ? (xdiff < 0 ? &real_rc0_adjusted : &real_rc1_adjusted)
                                           : (ydiff < 0 ? &real_rc0_adjusted : &real_rc1_adjusted);
            pb_point2D const* second = first == &real_rc0_adjusted ? &real_rc1_adjusted : &real_rc0_adjusted;

            *ovlap0 = *first;
            *ovlap1 = *second;
            return 1;
        }
    }
}

static void vert_remove_edges(void const* key, pb_vertex* vert, void* param) {
    pb_graph* g = (pb_graph*)param;
    size_t i;
    while(vert->edges_size) {
        void* other_key = vert->edges[0]->to->data;
        pb_graph_remove_edge(g, key, other_key);
    }
}

/**
 * Reconstructs the floor graph after adding hallways.
 *
 * @param floor_graph The floor graph to reconstruct.
 * @param floor       The floor for which to construct a new graph.
 *
 * @return 0 on success, -1 on failure.
 */
static int reconstruct_floor_graph(pb_graph* floor_graph, pb_floor const* f, size_t num_hallways,
                                   pb_sq_house_house_spec const* h, pb_hashmap* room_specs) {
    size_t i, j;

    /* Won't be needing this anymore */
    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_for_each_vertex(floor_graph, vert_remove_edges, floor_graph);
    
    /* Remove and re-add all vertices since the floor's rooms array may have been assigned a new pointer */
    for (i = 0; i < floor_graph->vertices->cap; ++i) {
        if (floor_graph->vertices->states[i] == FULL) {
            pb_vertex_free(floor_graph->vertices->entries[i].val);
        }

        floor_graph->vertices->states[i] = EMPTY;
    }
    floor_graph->vertices->size = 0;
    for (i = 0; i < f->num_rooms; ++i) {
        if (pb_graph_add_vertex(floor_graph, f->rooms + i, f->rooms + i) == -1) {
            return -1;
        }
    }
    
    /* First pass: expand hallways, reshape rooms */
    for (i = 0; i < f->num_rooms - num_hallways; ++i) {

        pb_rect room_rect;
        pb_shape2D_to_pb_rect(&f->rooms[i].shape, &room_rect);

        for (j = f->num_rooms - num_hallways; j < f->num_rooms; ++j) {

            pb_rect hallway_rect;
            pb_shape2D_get_bounding_rect(&f->rooms[j].shape, &hallway_rect);

            pb_point2D start, end;
            int result = intrude_hallway(&room_rect, &hallway_rect, &f->rooms[i], &start, &end);

            if (result == -1) {
                /* The caller's cleanup will get rid of any new edges */
                return -1;
            } else if (result == 1) {
                pb_sq_house_room_conn* conn = malloc(sizeof(pb_sq_house_room_conn));

                if (!conn) {
                    return -1;
                }

                conn->overlap_start = start;
                conn->overlap_end = end;
                conn->room = f->rooms + i;
                conn->neighbour = f->rooms + j;
                conn->can_connect = 1;

                if (pb_graph_add_edge(floor_graph, f->rooms + i, f->rooms + j, 0, conn) == -1) {
                    free(conn);
                    return -1;
                }
            }
        }

        /* All hallway edges have been added - now find the final points (points may have been adjusted/removed) */
        pb_vertex* vert = pb_graph_get_vertex(floor_graph, f->rooms + i);
        size_t cur_edge;
        for (cur_edge = 0; cur_edge < vert->edges_size; ++cur_edge) {
            pb_edge* edge = vert->edges[cur_edge];
            pb_sq_house_room_conn* conn = edge->data;
            
            pb_room* room = f->rooms + i;
            pb_room* hallway = conn->neighbour;
            pb_point2D const* points = (pb_point2D*)room->shape.points.items;
            pb_point2D const* hallway_points = (pb_point2D*)hallway->shape.points.items;

            pb_sq_house_room_conn* hallway_conn = malloc(sizeof(pb_sq_house_room_conn));
            if (!hallway_conn) {
                return -1;
            }
            if (pb_graph_add_edge(floor_graph, hallway, room, 0.f, hallway_conn) == -1) {
                free(hallway_conn);
                return -1;
            }

            hallway_conn->can_connect = 1;
            hallway_conn->neighbour = f->rooms + i;
            hallway_conn->room = hallway;

            size_t start_point_idx = -1;
            size_t end_point_idx = -1;
            float xdiff = conn->overlap_end.x - conn->overlap_start.x;
            float ydiff = conn->overlap_end.y - conn->overlap_start.y;
            int is_x = xdiff > ydiff;
            size_t num_found = 0;

            size_t cur_point;
            for (cur_point = 0; cur_point < room->shape.points.size && (start_point_idx == -1 || end_point_idx == -1); ++cur_point) {
                if (pb_point_eq(points + cur_point, &conn->overlap_start)) {
                    start_point_idx = cur_point;
                    num_found++;
                } else if (pb_point_eq(points + cur_point, &conn->overlap_end)) {
                    end_point_idx = cur_point;
                    num_found++;
                }
            }

            if (num_found == 2) {
                /* Both points are still in the shape, so no adjustment needed */
                /* Dirty short-circuited or */
                conn->wall = start_point_idx > end_point_idx || end_point_idx - start_point_idx > 1 ? end_point_idx : start_point_idx;
                conn->has_door = is_x ? xdiff > h->door_size : ydiff > h->door_size;

                hallway_conn->overlap_start = conn->overlap_start;
                hallway_conn->overlap_end = conn->overlap_end;
                hallway_conn->has_door = conn->has_door;
                
                pb_point2D const* start = points + start_point_idx;
                size_t hallway_wall;
                size_t cur_hallway_point;
                for (cur_hallway_point = 0; cur_hallway_point < hallway->shape.points.size; ++cur_hallway_point) {
                    pb_point2D const* wall_start = hallway_points + cur_hallway_point;
                    pb_point2D const* wall_end = hallway_points + ((cur_hallway_point + 1) % hallway->shape.points.size);

                    int wall_is_x = fabsf(wall_start->x - wall_end->x) > fabsf(wall_start->y - wall_end->y);

                    if (is_x) {
                        if (!wall_is_x) continue;

                        float xmin = fminf(wall_start->x, wall_end->x);
                        float xmax = fmaxf(wall_start->x, wall_end->x);
                        if (pb_float_approx_eq(start->y, wall_start->y, 5) && start->x >= xmin && start->x <= xmax) {
                            hallway_wall = cur_hallway_point;
                            break;
                        }
                    } else {
                        if (wall_is_x) continue;

                        float ymin = fminf(wall_start->y, wall_end->y);
                        float ymax = fmaxf(wall_start->y, wall_end->y);
                        if (pb_float_approx_eq(start->x, wall_start->x, 5) && start->y >= ymin && start->y <= ymax) {
                            hallway_wall = cur_hallway_point;
                            break;
                        }
                    }
                }
                hallway_conn->wall = (int)hallway_wall;
            } else if (num_found == 1) {
                /* Only one of the points was still present in the shape, which happens in various situations.
                 * Determine whether the hallway wall specified in the original overlap still overlaps at all with
                 * the room. If so, adjust the overlap to the portion that still overlaps; otherwise, check the other
                 * axis. */

                pb_point2D new_overlap_start;
                pb_point2D new_overlap_end;
                float delta;

                size_t idx = end_point_idx == -1 ? start_point_idx : end_point_idx;
                size_t room_prev_idx = idx == 0 ? room->shape.points.size - 1 : idx - 1;
                size_t room_next_idx = (idx + 1) % room->shape.points.size;
                pb_point2D const* room_prev = points + room_prev_idx;
                pb_point2D const* room_next = points + room_next_idx;

                int prev_is_x = fabsf(points[idx].x - room_prev->x) > fabsf(points[idx].y - room_prev->y);
                int next_is_x = fabsf(points[idx].x - room_next->x) > fabsf(points[idx].y - room_next->y);

                pb_point2D const* other = prev_is_x == is_x ? room_prev : room_next;
                size_t other_idx = other == room_prev ? room_prev_idx : room_next_idx;

                pb_point2D const* first = is_x ? (other->x < points[idx].x ? other : &points[idx])
                                               : (other->y < points[idx].y ? other : &points[idx]);
                pb_point2D const* second = first == &points[idx] ? other : &points[idx];

                size_t cur_hallway_point;
                for (cur_hallway_point = 0; cur_hallway_point < hallway->shape.points.size; ++cur_hallway_point) {
                    pb_point2D const* wall_first = hallway_points + cur_hallway_point;
                    pb_point2D const* wall_second = hallway_points + ((cur_hallway_point + 1) % hallway->shape.points.size);
                    float wall_xdiff = wall_first->x - wall_second->x;
                    float wall_ydiff = wall_first->y - wall_second->y;
                    int wall_is_x = fabsf(wall_xdiff) > fabsf(wall_ydiff);

                    if (wall_is_x == is_x && pb_float_approx_eq(is_x ? wall_first->y : wall_first->x,
                                     is_x ? points[idx].y : points[idx].x, 5)) {
                        /* Rearrange the points so that they're in the correct order */
                        pb_point2D const* tmp = wall_first;
                        wall_first = is_x ? (wall_xdiff > 0 ? wall_second : wall_first)
                                          : (wall_ydiff > 0 ? wall_second : wall_first);
                        wall_second = wall_first == wall_second ? tmp : wall_second;;

                        pb_point2D const* start = is_x ? (first->x > wall_first->x ? first : wall_first)
                                                       : (first->y > wall_first->y ? first : wall_first);
                        pb_point2D const* end = is_x ? (second->x < wall_second->x ? second : wall_second)
                                                     : (second->y < wall_second->y ? second : wall_second);

                        delta = is_x ? end->x - start->x : end->y - start->y;
                        if (delta > 0) {
                            new_overlap_start = *start;
                            new_overlap_end = *end;
                            break;
                        }
                    }
                }

                if (cur_hallway_point != hallway->shape.points.size) {
                    /* Found the new overlap */
                    conn->overlap_start = new_overlap_start;
                    conn->overlap_end = new_overlap_end;
                    conn->wall = other_idx == room_next_idx ? (int)idx : (int)other_idx;
                    conn->has_door = delta > h->door_size;

                    hallway_conn->overlap_start = new_overlap_start;
                    hallway_conn->overlap_end = new_overlap_end;
                    hallway_conn->wall = (int)cur_hallway_point;
                    hallway_conn->has_door = conn->has_door;
                } else {
                    /* We didn't find the new overlap, meaning that the original hallway wall no longer 
                     * overlaps the room at all. The other overlap *must* be on the other axis (or the rooms
                     * no longer ovelap, which should be impossible). */
                    other = other == room_prev ? room_next : room_prev;
                    other_idx = other == room_prev ? room_prev_idx : room_next_idx;
                    is_x = !is_x;

                    first = is_x ? (other->x < points[idx].x ? other : &points[idx])
                                    : (other->y < points[idx].y ? other : &points[idx]);
                    second = first == &points[idx] ? other : &points[idx];

                    for (cur_hallway_point = 0; cur_hallway_point < hallway->shape.points.size; ++cur_hallway_point) {
                        pb_point2D const* wall_first = hallway_points + cur_hallway_point;
                        pb_point2D const* wall_second = hallway_points + ((cur_hallway_point + 1) % hallway->shape.points.size);
                        float wall_xdiff = wall_first->x - wall_second->x;
                        float wall_ydiff = wall_first->y - wall_second->y;
                        int wall_is_x = fabsf(wall_xdiff) > fabsf(wall_ydiff);

                        if (wall_is_x == is_x && pb_float_approx_eq(is_x ? wall_first->y : wall_first->x,
                                                                    is_x ? points[idx].y : points[idx].x, 5)) {
                            /* Rearrange the points so that they're in the correct order */
                            pb_point2D const* tmp = wall_first;
                            wall_first = xdiff > 0 ? wall_second : wall_first;
                            wall_second = xdiff > 0 ? tmp : wall_second;

                            pb_point2D const* start = is_x ? (first->x > wall_first->x ? first : wall_first)
                                : (first->y > wall_first->y ? first : wall_first);
                            pb_point2D const* end = is_x ? (second->x < wall_second->x ? second : wall_second)
                                : (second->y < wall_second->y ? second : wall_second);

                            delta = is_x ? end->x - start->x : end->y - start->y;
                            if (delta > 0) {
                                new_overlap_start = *start;
                                new_overlap_end = *end;
                                break;
                            }
                        }
                    }
                    conn->overlap_start = new_overlap_start;
                    conn->overlap_end = new_overlap_end;
                    conn->wall = other_idx == room_next_idx ? (int)idx : (int)other_idx;
                    conn->has_door = delta > h->door_size;

                    hallway_conn->overlap_start = new_overlap_start;
                    hallway_conn->overlap_end = new_overlap_end;
                    hallway_conn->wall = (int)cur_hallway_point;
                    hallway_conn->has_door = conn->has_door;
                }
            } else {
                /* Neither point was found - either the hallway wall completely overlapped a wall that has since
                 * been adjusted on both sides, or the original overlap was from a corner with both points removed.
                 * Find the new point locations */

                /* Try to find new overlap points in the room. If we don't find any, we'll have to check the other
                 * axis. */
                size_t cur_room_point;
                for (cur_room_point = 0; cur_room_point < room->shape.points.size; ++cur_room_point) {
                    pb_point2D const* cur = points + cur_room_point;
                    pb_point2D const* next = points + ((cur_room_point + 1) % room->shape.points.size);

                    int wall_is_x = fabsf(cur->x - next->x) > fabsf(cur->y - next->y);
                    if (is_x) {
                        if (!wall_is_x) continue;
                        if (pb_float_approx_eq(cur->y, conn->overlap_start.y, 5)) {
                            pb_point2D const* cur_start = cur->x > next->x ? next : cur;
                            pb_point2D const* cur_end = cur_start == next ? cur : next;

                            /* Check if the walls actually overlap */
                            if (cur_start->x >= conn->overlap_start.x && cur_end->x <= conn->overlap_end.x) {
                                conn->overlap_start = cur->x > next->x ? *next : *cur;
                                conn->overlap_end = cur->x > next->x ? *cur : *next;
                                break;
                            }
                        }
                    } else {
                        if (wall_is_x) continue;
                        if (pb_float_approx_eq(cur->x, conn->overlap_start.x, 5)) {
                            pb_point2D const* cur_start = cur->y > next->y ? next : cur;
                            pb_point2D const* cur_end = cur_start == next ? cur : next;
                            
                            if (cur_start->y >= conn->overlap_start.y && cur_end->y <= conn->overlap_end.y) {
                                conn->overlap_start = cur->y > next->y ? *next : *cur;
                                conn->overlap_end = cur->y > next->y ? *cur : *next;
                                break;
                            }
                        }
                    }
                }
                if (cur_room_point < room->shape.points.size) {
                    /* We found the updated overlap points; find the overlap wall in the hallway */
                    conn->wall = cur_room_point;
                    hallway_conn->overlap_start = conn->overlap_start;
                    hallway_conn->overlap_end = conn->overlap_end;

                    float delta = is_x ? conn->overlap_end.x - conn->overlap_start.x
                                       : conn->overlap_end.y - conn->overlap_start.y;
                    conn->has_door = delta > h->door_size;
                    hallway_conn->has_door = conn->has_door;

                    size_t cur_hallway_point;
                    for (cur_hallway_point = 0; cur_hallway_point < hallway->shape.points.size; ++cur_hallway_point) {
                        pb_point2D const* wall_start = hallway_points + cur_hallway_point;
                        pb_point2D const* wall_end = hallway_points + ((cur_hallway_point + 1) % hallway->shape.points.size);

                        int wall_is_x = fabsf(wall_start->x - wall_end->x) > fabsf(wall_start->y - wall_end->y);

                        if (is_x) {
                            if (!wall_is_x) continue;

                            float xmin = fminf(wall_start->x, wall_end->x);
                            float xmax = fmaxf(wall_start->x, wall_end->x);
                            if (pb_float_approx_eq(conn->overlap_start.y, wall_start->y, 5)) {
                                float xstart = fmaxf(wall_start->x, conn->overlap_start.x);
                                float xend = fminf(wall_end->x, conn->overlap_end.x);
                                if (xend - xstart > 0) {
                                    break;
                                }
                            }
                        }
                        else {
                            if (wall_is_x) continue;

                            float ymin = fminf(wall_start->y, wall_end->y);
                            float ymax = fmaxf(wall_start->y, wall_end->y);
                            if (pb_float_approx_eq(conn->overlap_start.x, wall_start->x, 5)) {
                                float ystart = fmaxf(wall_start->y, conn->overlap_start.y);
                                float yend = fminf(wall_end->y, conn->overlap_end.y);
                                if (yend - ystart > 0) {
                                    break;
                                }
                            }
                        }
                    }
                    hallway_conn->wall = cur_hallway_point;
                } else {
                    /* None of the walls overlapped, which means we chose poorly at a corner. Check the wall on the
                     * other axis in the hallway. */
                    size_t cur_hallway_point;
                    for (cur_hallway_point = 0; cur_hallway_point < hallway->shape.points.size; ++cur_hallway_point) {
                        /* One of the overlap points was in the hallway, so find it */
                        if (pb_point_eq(hallway_points + cur_hallway_point, &conn->overlap_start) ||
                            pb_point_eq(hallway_points + cur_hallway_point, &conn->overlap_end)) {
                            break;
                        }
                    }
                    pb_point2D const* hpoint = hallway_points + cur_hallway_point;
                    size_t hpoint_next_idx = (cur_hallway_point + 1) % hallway->shape.points.size;
                    size_t hpoint_prev_idx = (cur_hallway_point == 0 ? hallway->shape.points.size - 1 : cur_hallway_point - 1);
                    pb_point2D const* hpoint_next = hallway_points + hpoint_next_idx;
                    pb_point2D const* hpoint_prev = hallway_points + hpoint_prev_idx;
                    int prev_is_x = fabsf(hpoint->x - hpoint_prev->x) > fabsf(hpoint->y - hpoint_prev->y);
                    is_x = !is_x;

                    pb_point2D const* other = is_x ? (prev_is_x ? hpoint_prev : hpoint_next)
                                                   : (prev_is_x ? hpoint_next : hpoint_prev);

                    float to_match = is_x ? hpoint->y : hpoint->x;
                    float delta = is_x ? hpoint->x - other->x : hpoint->y - other->y;

                    pb_point2D const* first = delta < 0 ? hpoint : other;
                    pb_point2D const* second = delta < 0 ? other : hpoint;

                    /* Search the room for the wall overlapping this wall */
                    for (cur_room_point = 0; cur_room_point < room->shape.points.size; ++cur_room_point) {
                        pb_point2D const* cur = points + cur_room_point;
                        pb_point2D const* next = points + ((cur_room_point + 1) % room->shape.points.size);
                        int wall_is_x = fabsf(cur->x - next->x) > fabsf(cur->y - next->y);

                        if (wall_is_x == is_x) {
                            if (pb_float_approx_eq(is_x ? cur->y : cur->x, to_match, 5)) {
                                pb_point2D const* room_first = is_x ? (cur->x < next->y ? cur : next)
                                                                    : (cur->y < next->y ? cur : next);
                                pb_point2D const* room_second = room_first == cur ? next : cur;

                                pb_point2D const* possible_overlap_start = is_x ? (room_first->x > first->x ? room_first : first)
                                                                                : (room_first->y > first->y ? room_first : first);
                                pb_point2D const* possible_overlap_end = is_x ? (room_second->x < second->x ? room_second : second)
                                                                              : (room_second->y < second->y ? room_second : second);

                                float walls_delta = is_x ? possible_overlap_end->x - possible_overlap_start->x
                                                         : possible_overlap_end->y - possible_overlap_start->y;
                                if (walls_delta > 0) {
                                    conn->overlap_start = *possible_overlap_start;
                                    conn->overlap_end = *possible_overlap_end;
                                    conn->wall = cur_room_point;
                                    conn->has_door = walls_delta > h->door_size;

                                    hallway_conn->overlap_start = *possible_overlap_start;
                                    hallway_conn->overlap_end = *possible_overlap_end;
                                    hallway_conn->wall = other == hpoint_next ? cur_hallway_point : hpoint_prev_idx;
                                    hallway_conn->has_door = conn->has_door;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* Finally, realloc the walls array to the new size and set all walls to 1 */
        if (f->rooms[i].walls.cap < f->rooms[i].shape.points.size) {
            if (pb_vector_resize(&f->rooms[i].walls, f->rooms[i].shape.points.size) == -1) {
                return -1;
            }

            size_t wall;
            int* walls = (int*)f->rooms[i].walls.items;
            for (wall = 0; wall < f->rooms[i].shape.points.size; ++wall) {
                walls[wall] = 1;
            }
        }
    }

    /* Second pass: connect all adjacent non-hallway rooms */
    /* TODO: There's probably lots of stuff to optimise in here. */
    for (i = 0; i < f->num_rooms - num_hallways; ++i) {
        pb_rect room_rect;
        pb_shape2D_get_bounding_rect(&f->rooms[i].shape, &room_rect);

        for (j = 0; j < f->num_rooms - num_hallways; ++j) {
            if (i == j) {
                continue;
            }

            pb_rect other_rect;
            pb_point2D start, end;

            pb_shape2D_get_bounding_rect(&f->rooms[j].shape, &other_rect);

            int shared_wall = pb_sq_house_get_shared_wall(&room_rect, &other_rect);
            if (shared_wall != -1) {
                int is_x = shared_wall == 1 || shared_wall == 3;
                float to_match;
                switch (shared_wall) {
                case 0:
                    to_match = room_rect.bottom_left.x;
                    break;
                case 1:
                    to_match = room_rect.bottom_left.y;
                    break;
                case 2:
                    to_match = other_rect.bottom_left.x;
                    break;
                case 3:
                    to_match = other_rect.bottom_left.y;
                    break;
                }

                size_t cur_point = 0;
                int found_overlap = 0;
                /* Continue checking walls until we either find the overlap or run out of walls */
                while (cur_point < f->rooms[i].shape.points.size && !found_overlap) {
                    /* Find the actual points in both rooms that correspond to the given wall */
                    pb_point2D const* room_point0;
                    pb_point2D const* room_point1;
                    size_t room_wall;

                    pb_point2D const* other_point0;
                    pb_point2D const* other_point1;


                    for (; cur_point < f->rooms[i].shape.points.size; ++cur_point) {
                        pb_point2D const* room_points = (pb_point2D*)f->rooms[i].shape.points.items;
                        pb_point2D const* cur = room_points + cur_point;
                        pb_point2D const* next = room_points + ((cur_point + 1) % f->rooms[i].shape.points.size);

                        int wall_is_x = fabsf(cur->x - next->x) > fabsf(cur->y - next->y);
                        if (wall_is_x == is_x && pb_float_approx_eq(is_x ? cur->y : cur->x, to_match, 5)) {
                            room_point0 = cur;
                            room_point1 = next;
                            room_wall = cur_point;
                            break;
                        }
                    }

                    size_t other_point = 0;
                    while (other_point < f->rooms[j].shape.points.size && !found_overlap) {
                        for (; other_point < f->rooms[j].shape.points.size; ++other_point) {
                            pb_point2D const* other_points = (pb_point2D*)f->rooms[j].shape.points.items;
                            pb_point2D const* cur = other_points + other_point;
                            pb_point2D const* next = other_points + ((other_point + 1) % f->rooms[j].shape.points.size);

                            int wall_is_x = fabsf(cur->x - next->x) > fabsf(cur->y - next->y);
                            if (wall_is_x == is_x && pb_float_approx_eq(is_x ? cur->y : cur->x, to_match, 5)) {
                                other_point0 = cur;
                                other_point1 = next;
                                break;
                            }
                        }

                        /* Swap the points so that they're in the correct order */
                        pb_point2D const* temp = room_point0;
                        room_point0 = is_x ? (room_point0->x < room_point1->x ? room_point0 : room_point1)
                                           : (room_point0->y < room_point1->y ? room_point0 : room_point1);
                        room_point1 = room_point0 == room_point1 ? temp : room_point1;

                        temp = other_point0;
                        other_point0 = is_x ? (other_point0->x < other_point1->x ? other_point0 : other_point1)
                                            : (other_point0->y < other_point1->y ? other_point0 : other_point1);
                        other_point1 = other_point0 == other_point1 ? temp : other_point1;

                        /* Check if they actually do overlap */
                        start = is_x ? (room_point0->x > other_point0->x ? *room_point0 : *other_point0)
                                     : (room_point0->y > other_point0->y ? *room_point0 : *other_point0);
                        end = is_x ? (room_point1->x < other_point1->x ? *room_point1 : *other_point1)
                                   : (room_point1->y < other_point1->y ? *room_point1 : *other_point1);

                        float delta = is_x ? end.x - start.x : end.y - start.y;

                        if (delta > 0) {
                            pb_sq_house_room_conn* conn = malloc(sizeof(pb_sq_house_room_conn));
                            conn->room = f->rooms + i;
                            conn->neighbour = f->rooms + j;
                            if (!conn) {
                                /* So close... */
                                return -1;
                            }
                            if (pb_graph_add_edge(floor_graph, f->rooms + i, f->rooms + j, 0.f, conn) == -1) {
                                free(conn);
                                return -1;
                            }

                            pb_sq_house_room_spec* spec;
                            int has_spec = pb_hashmap_get(room_specs, f->rooms[i].name, &spec) != -1;

                            size_t cur_name;
                            if (has_spec) {
                                for (cur_name = 0; cur_name < spec->num_adjacent; ++cur_name) {
                                    if (strcmp(f->rooms[j].name, spec->adjacent[cur_name]) == 0) {
                                        break;
                                    }
                                }
                            }

                            conn->can_connect = !has_spec || cur_name != spec->num_adjacent;

                            conn->overlap_start = start;
                            conn->overlap_end = end;
                            conn->wall = (int)room_wall;
                            conn->has_door = (is_x ? end.x - start.x > h->door_size : end.y - start.y > h->door_size) &&
                                            conn->can_connect;

                            /* If the other room already has an edge to this room, check whether can_connect matches
                             * and set both to 1 (adjusting has_door for the other room as well) if not */
                            pb_edge const* other_to_room = pb_graph_get_edge(floor_graph, f->rooms + j, f->rooms + i);
                            if (other_to_room) {
                                pb_sq_house_room_conn* other_room_conn = (pb_sq_house_room_conn*)other_to_room->data;
                                if (other_room_conn->can_connect && !conn->can_connect) {
                                    conn->can_connect = 1;
                                    conn->has_door = other_room_conn->has_door;
                                }
                                else if (!other_room_conn->can_connect && conn->can_connect) {
                                    other_room_conn->can_connect = 1;
                                    other_room_conn->has_door = conn->has_door;
                                }
                            }
                            found_overlap = 1;
                        } else {
                            other_point++;
                        }
                    }

                    if (!found_overlap) {
                        cur_point++; /* we broke out of the loop, so this didn't get incremented */
                    }
                }
            }
        }
    }
    return 0;
}

static void vert_copy(void const* key, pb_vertex* vert, void* param) {
    pb_pair* pair = (pb_pair*)param;
    pb_graph* pruned = (pb_graph*)pair->first;
    int* err = (int*)pair->second;

    if (*err == 0 && pb_graph_add_vertex(pruned, key, key) == -1) {
        *err = 1;
    }
}

/**
 * Determines the axes along which this point is part of a line segment.
 *
 * @param vert  The vertex to check.
 * @param has_x Whether this point is part of a line parallel to the x axis.
 * @param has_y Whether this point is part of a line parallel to the y axis.
 */
static void get_point_axes(pb_vertex const* vert, size_t* has_x, size_t* has_y) {
    pb_point2D* point = (pb_point2D*)vert->data;
    switch (vert->edges_size) {
    case 1:
    {
        pb_point2D* next = (pb_point2D*)vert->edges[0]->to->data;
        float xdiff = point->x - next->x;
        float ydiff = point->y - next->y;
        *has_x = fabsf(xdiff) > fabsf(ydiff);
        *has_y = !*has_x;
        break;
    }
    case 2:
    {
        pb_point2D* n0 = (pb_point2D*)vert->edges[0]->to->data;
        pb_point2D* n1 = (pb_point2D*)vert->edges[1]->to->data;

        float xdiff0 = point->x - n0->x;
        float ydiff0 = point->y - n0->y;

        *has_x = fabsf(xdiff0) > fabsf(ydiff0);
        *has_y = !*has_x;

        float xdiff1 = point->x - n1->x;
        float ydiff1 = point->y - n1->y;

        *has_x = *has_x || fabsf(xdiff1) > fabsf(ydiff1);
        *has_y = *has_y || fabsf(ydiff1) > fabsf(xdiff1);
        break;
    }
    case 3:
    case 4:
        *has_x = 1;
        *has_y = 1;
        break;
    }
}

static uint32_t line_segment_hash(void const* line_key) {
    pb_pair const* pair = (pb_pair*)line_key;
    return (uint32_t)((size_t)pair->first + (size_t)pair->second);
}

static int line_segment_eq(void const* lhs, void const* rhs) {
    pb_pair const* lpair = (pb_pair*)lhs;
    pb_pair const* rpair = (pb_pair*)rhs;

    return lpair->first == rpair->first && lpair->second == rpair->second;
}

typedef struct {
    pb_point2D start;
    pb_point2D end;
} pb_wall_pair;

/* TODO: This function needs major refactoring.
 * []   It's way too long
 * []   "Cleverness" in parts basically just made it an unreadable mess
 * []   There are magic numbers everywhere */
int pb_sq_house_place_hallways(pb_floor* f, pb_sq_house_house_spec* hspec, pb_hashmap* room_specs,
                               pb_graph* floor_graph, pb_graph* internal_graph, pb_vector* hallways) {
    pb_vector* hallway_list = (pb_vector*)hallways->items;
    
    /* Find smallest dimension so that we can set the hallway dimensions */
    size_t i;
    float hallway_size = INFINITY;
    for (i = 0; i < hallways->size; ++i) {
        size_t j;
        for (j = 0; j < hallway_list[i].size; ++j) {
            pb_edge const* edge = ((pb_edge**)hallway_list[i].items)[j];
            pb_sq_house_room_conn const* conn = (pb_sq_house_room_conn*)edge->data;
            pb_point2D const* room_points = (pb_point2D*)conn->room->shape.points.items;
            pb_point2D const* neighbour_points = (pb_point2D*)conn->neighbour->shape.points.items;

            float local_min;
            int is_x = conn->overlap_start.y == conn->overlap_end.y;
            if (is_x) {
                float room_height = room_points[0].y - room_points[1].y;
                float neighbour_height = neighbour_points[0].y - neighbour_points[1].y;
                local_min = fminf(room_height, neighbour_height);
            } else {
                float room_width = room_points[2].x - room_points[1].x;
                float neighbour_width = neighbour_points[2].x - neighbour_points[1].x;
                local_min = fminf(room_width, neighbour_width);
            }

            hallway_size = fminf(hallway_size, local_min);
        }
    }
    hallway_size = fminf(hallway_size * 0.25f, hspec->hallway_width);

    pb_graph* pruned = pb_graph_create(internal_graph->vertices->hash, internal_graph->vertices->key_eq);
    if (pruned == NULL) {
        return -1;
    }

    /* Copy vertices to the pruned graph */
    int err = 0;
    pb_pair params = { pruned, &err };
    pb_graph_for_each_vertex(internal_graph, vert_copy, &params);
    if (err) {
        pb_graph_free(pruned);
        return -1;
    }

    /* Only copy hallway edges and the edges in the other direction to the new graph.
     * Vertices that aren't in hallways will still be in the graph but will be inaccessible. */
    for (i = 0; i < hallways->size; ++i) {
        size_t j;
        pb_edge** edges = (pb_edge**)hallway_list[i].items;

        for (j = 0; j < hallway_list[i].size; ++j) {
            pb_point2D* from_point = (pb_point2D*)edges[j]->from->data;
            pb_point2D* to_point = (pb_point2D*)edges[j]->to->data;

            if (pb_graph_add_edge(pruned, from_point, to_point, edges[j]->weight, edges[j]->data) == -1 ||
                pb_graph_add_edge(pruned, to_point, from_point, edges[j]->weight, edges[j]->data) == -1) {

                pb_graph_free(pruned);
                return -1;
            }
        }
    }

    pb_vector hallway_segments;
    pb_vector point_queue;
    point_queue.items = NULL;
    hallway_segments.items = NULL;
    if (pb_vector_init(&hallway_segments, sizeof(pb_vector), 0) == -1 ||
        pb_vector_init(&point_queue, sizeof(pb_pair), pruned->vertices->size) == -1) {
        goto err_return;
    }

    /* Get the start vertex from the pruned graph, since otherwise it will still have connections to vertices in the full graph */
    pb_vertex* start_vert = ((pb_edge**)hallway_list[0].items)[0]->from;
    pb_point2D* start_key = (pb_point2D*)start_vert->data;
    start_vert = pb_graph_get_vertex(pruned, start_key);

    size_t has_x;
    size_t has_y;
    pb_pair start;
    get_point_axes(start_vert, &has_x, &has_y);
    start.first = start_vert;
    start.second = (void*)has_x;

    if (pb_vector_push_back(&point_queue, &start) == -1) {
        goto err_return;
    }

    pb_hashmap* segments_disjoint_set = pb_hashmap_create(line_segment_hash, line_segment_eq);
    if (segments_disjoint_set == NULL) {
        goto err_return;
    }

    size_t num_4way = 0;

    /* Get a list of straight hallway segments to be expanded. Should use a queue for better performance. */
    while (point_queue.size) {
        pb_pair* pairs = (pb_pair*)point_queue.items;
        pb_vertex* cur = (pb_vertex*)pairs[0].first;
        size_t is_x = (size_t)pairs[0].second;

        pb_vector_remove_at(&point_queue, 0);

        if (cur->edges_size == 4 && is_x) {
            num_4way++;
        }

        /* Go as far as we can in the given direction (left if is_x, down otherwise) */
        pb_vertex* line_start = NULL;
        while (!line_start) {
            size_t j;
            for (j = 0; j < cur->edges_size; ++j) {
                pb_vertex* neighbour = cur->edges[j]->to;
                pb_point2D* npoint = (pb_point2D*)neighbour->data;
                if (is_x) {
                    float xdiff = npoint->x - ((pb_point2D*)cur->data)->x;
                    if (xdiff < 0) {
                        cur = neighbour;
                        break;
                    }
                } else {
                    float ydiff = npoint->y - ((pb_point2D*)cur->data)->y;
                    if (ydiff < 0) {
                        cur = neighbour;
                        break;
                    }
                }
            }
            /* Couldn't go any further in given direction - found the start point */
            if (j == cur->edges_size) {
                line_start = cur;
            }
        }

        pb_pair line_rep = { line_start, is_x }; /* "Representative" for this line */
        void* dummy;
        if (pb_hashmap_get(segments_disjoint_set, &line_rep, &dummy) == -1) {
            /* We haven't explored this line yet - add it to the set and check it out */
            pb_pair* line_key = malloc(sizeof(pb_pair));
            if (!line_key) {
                goto err_return;
            }

            *line_key = line_rep;
            if (pb_hashmap_put(segments_disjoint_set, line_key, line_key) == -1) {
                free(line_key);
                goto err_return;
            }

            pb_vector segment;
            if (pb_vector_init(&segment, sizeof(pb_vertex*), 0) == -1) {
                goto err_return;
            }

            while (1) {
                size_t cur_edges_size = cur->edges_size;

                /* Add each point to this segment */
                if (pb_vector_push_back(&segment, &cur) == -1) {
                    goto err_return;
                }

                /* Add "points of interest" to the list of points to check */
                size_t has_x, has_y;
                get_point_axes(cur, &has_x, &has_y);
                if (is_x && has_y) {
                    pb_pair poi = { cur, 0 };
                    if (pb_vector_push_back(&point_queue, &poi) == -1) {
                        goto err_return;
                    }
                } else if (!is_x && has_x) {
                    pb_pair poi = { cur, 1 };
                    if (pb_vector_push_back(&point_queue, &poi) == -1) {
                        goto err_return;
                    }
                }

                /* Find the next point on the line, if any */
                size_t j;
                for (j = 0; j < cur->edges_size; ++j) {
                    pb_vertex* neighbour = cur->edges[j]->to;
                    pb_point2D* npoint = (pb_point2D*)neighbour->data;
                    if (is_x) {
                        float xdiff = npoint->x - ((pb_point2D*)cur->data)->x;
                        if (xdiff > 0) {
                            cur = neighbour;
                            break;
                        }
                    }
                    else {
                        float ydiff = npoint->y - ((pb_point2D*)cur->data)->y;
                        if (ydiff > 0) {
                            cur = neighbour;
                            break;
                        }
                    }
                }

                /* Continue until we find a point where not a single edge goes in the correct direction */
                if (j == cur_edges_size) {
                    break;
                }
            }

            if (pb_vector_push_back(&hallway_segments, &segment) == -1) {
                pb_vector_free(&segment);
                goto err_return;
            }
        } else {
            if (cur->edges_size == 4 && is_x) {
                num_4way--;
            }
        }
    }

    /* Create new rooms using the line segments and add them to the floor */

    size_t old_num_rooms = f->num_rooms;
    size_t new_num_rooms = f->num_rooms + num_4way + hallway_segments.size;

    pb_room* new_rooms_list = realloc(f->rooms, sizeof(pb_room) * new_num_rooms);
    if (!new_rooms_list) {
        goto err_return;
    }

    f->rooms = new_rooms_list;

    err = 0;
    for (i = 0; i < hallway_segments.size; ++i) {
        pb_vector* segment = ((pb_vector*)hallway_segments.items) + i;
        pb_vertex const** points = (pb_vertex const**)segment->items;

        pb_point2D const* first = (pb_point2D*)points[0]->data;
        pb_point2D const* last = (pb_point2D*)points[segment->size - 1]->data;
        int is_x = last->x - first->x > last->y - first->y;

        pb_point2D room_start = *first;
        pb_point2D room_end;

        /* Whether to knock out the start and/or end wall for the next room
        * (wall[0]/wall[2] for vertical segments, wall[1]/wall[3] for horizontal) */
        int ko_start_wall = 0;
        int ko_end_wall = 0;

        /* For vertical walls, denotes which side a horizontal corner is on (if any)
        * None = -1, Left = 0, Right = 1 */
        int top_corner_side = -1;
        int bottom_corner_side = -1;

        float half_size = hallway_size / 2;

        /* Which gaps should be processed in reverse */
        int reverse = is_x; /* The list 1 goes in reverse when moving vertically, and list 0 when horiztonal */

        pb_vector gaps[2] = { 0 };
        if (pb_vector_init(&gaps[0], sizeof(pb_wall_pair), 0) == -1 ||
            pb_vector_init(&gaps[1], sizeof(pb_wall_pair), 0) == -1) {

            err = 1;
            break;
        }

        int add_room = 0;
        size_t j;
        for (j = 0; j < segment->size && !err; ++j) {
            pb_point2D* cur_point = (pb_point2D*)points[j]->data;

            add_room = j == segment->size - 1;

            /* Check for corner point or T-intersection */
            switch (points[j]->edges_size) {
                /* Set end point if we're at a dead end */
            case 1:
                if (j == segment->size - 1) {
                    room_end = *((pb_point2D*)points[j]->data);
                }
                break;
            case 2:
                /* Check for corner point
                 * If we find one, vertical segments will be expanded to fill space and have an extra gap added,
                 * horizontal segments will be shrunk. */
                if (j == 0 || j == segment->size - 1) {
                    pb_point2D* n0 = (pb_point2D*)points[j]->edges[0]->to->data;
                    pb_point2D* n1 = (pb_point2D*)points[j]->edges[1]->to->data;

                    int n0_is_x = fabsf(cur_point->x - n0->x) > fabsf(cur_point->y - n0->y);
                    int n1_is_x = fabsf(cur_point->x - n1->x) > fabsf(cur_point->y - n1->y);
                    int is_corner = (n0_is_x != is_x) || (n1_is_x != is_x);
                    if (is_corner) {
                        if (j == 0) {
                            if (is_x) {
                                room_start.x += half_size;
                                ko_start_wall = 1;
                            } else {
                                room_start.y -= half_size;
                                bottom_corner_side = n0_is_x != is_x ? (n0->x - cur_point->x) > 0
                                    : (n1->x - cur_point->x) > 0;
                            }
                        } else {
                            room_end = *cur_point;
                            if (is_x) {
                                room_end.x -= half_size;
                                ko_end_wall = 1;
                            }
                            else {
                                room_end.y += half_size;
                                top_corner_side = n0_is_x != is_x ? (n0->x - cur_point->x) > 0
                                    : (n1->x - cur_point->x) > 0;
                            }
                        }
                    }
                }
                break;
            case 3:
                /* We're at a T-intersection
                 * If we're at one of the line segment's ends, we're the descending part of the T;
                 * If we're somewhere in the middle of the segment, we're the top of the T */
                if (j == 0) {
                    ko_start_wall = 1;
                    if (is_x) {
                        room_start.x += half_size;
                    } else {
                        room_start.y += half_size;
                    }
                } else if (j == segment->size - 1) {
                    ko_end_wall = 1;
                    if (is_x) {
                        room_end.x -= half_size;
                    } else {
                        room_end.y -= half_size;
                    }
                } else {
                    /* Figure out which side the intersection is on */
                    pb_point2D* n0 = (pb_point2D*)points[j]->edges[0]->to->data;
                    pb_point2D* n1 = (pb_point2D*)points[j]->edges[1]->to->data;
                    pb_point2D* n2 = (pb_point2D*)points[j]->edges[2]->to->data;
                    pb_point2D* t_shaft;
                    int n0_is_x = fabsf(cur_point->x - n0->x) > fabsf(cur_point->y - n0->y);
                    int n1_is_x = fabsf(cur_point->x - n1->x) > fabsf(cur_point->y - n1->y);
                    int n2_is_x = fabsf(cur_point->x - n2->x) > fabsf(cur_point->y - n2->y);

                    t_shaft = n0_is_x == is_x ? (n1_is_x == is_x ? n2 : n1) : n0;
                    if (is_x) {
                        int is_above = t_shaft->y - cur_point->y > 0;
                        int which_list = is_above;
                        float delta = half_size * (1 + ((!is_above) * -2)); /* Who knows if this is actually more efficient... */
                        float y = cur_point->y + delta;
                        pb_wall_pair gap;
                        gap.start.x = cur_point->x - half_size;
                        gap.start.y = y;
                        gap.end.x = cur_point->x + half_size;
                        gap.end.y = y;

                        if (pb_vector_push_back(&gaps[which_list], &gap) == -1) {
                            err = 1;
                            break;
                        }
                    } else {
                        int is_right = t_shaft->x - cur_point->x > 0;
                        int which_list = is_right;
                        float delta = half_size * (1 + ((!is_right) * -2));
                        float x = cur_point->x + delta;

                        pb_wall_pair gap;
                        gap.start.x = x;
                        gap.start.y = cur_point->y - half_size;
                        gap.end.x = x;
                        gap.end.y = cur_point->y + half_size;

                        if (pb_vector_push_back(&gaps[which_list], &gap) == -1) {
                            err = 1;
                            break;
                        }
                    }
                }
                break;
            case 4:
                /* Pls no */
                if (is_x) {
                    room_end = *cur_point;
                    room_end.x -= half_size;
                    add_room = 1;
                } else {
                    pb_wall_pair gap_left;
                    pb_wall_pair gap_right;

                    gap_left.start.y = cur_point->y - half_size;
                    gap_left.start.x = cur_point->x - half_size;
                    gap_left.end.y = cur_point->y + half_size;
                    gap_left.end.x = cur_point->x - half_size;

                    gap_right.start.x = cur_point->x + half_size;
                    gap_right.start.y = gap_left.start.y;
                    gap_right.end.x = cur_point->x + half_size;
                    gap_right.end.y = gap_left.end.y;

                    if (pb_vector_push_back(&gaps[0], &gap_left) == -1 ||
                        pb_vector_push_back(&gaps[1], &gap_right) == -1) {

                        err = 1;
                        break;
                    }
                }
                break;
            }

            if (add_room) {
                pb_room* next = f->rooms + f->num_rooms++;
                pb_rect room_rect;

                next->name = PB_SQ_HOUSE_HALLWAY;
                
                if (is_x) {
                    room_rect.bottom_left.x = room_start.x;
                    room_rect.bottom_left.y = room_start.y - half_size;
                    room_rect.w = room_end.x - room_start.x;
                    room_rect.h = hallway_size;

                    ko_start_wall = ko_start_wall || (room_start.x != ((pb_point2D*)(points[0]->data))->x);

                    /* We're adding an intermediate room - move the start point to the next spot and kill necessary walls */
                    if (j != segment->size - 1) {
                        ko_end_wall = 1;
                        room_start.x = room_end.x + hallway_size;
                        room_start.y = cur_point->y;
                    }
                } else {
                    room_rect.bottom_left.x = room_start.x - half_size;
                    room_rect.bottom_left.y = room_start.y;
                    room_rect.w = hallway_size;
                    room_rect.h = room_end.y - room_start.y;
                }

                size_t num_walls = 4 + (gaps[0].size + gaps[1].size) * 2;
                num_walls += top_corner_side != -1 ? 1 : 0;
                num_walls += bottom_corner_side != -1 ? 1 : 0;

                if (pb_rect_to_pb_shape2D(&room_rect, &next->shape) == -1) {
                    err = 1;
                    break;
                } else if (pb_vector_init(&next->walls, sizeof(int), num_walls) == -1) {
                    pb_shape2D_free(&next->shape);
                    err = 1;
                    break;
                }

                next->walls.size = num_walls;

                int* walls = next->walls.items;
                size_t start_wall_idx = is_x ? 0
                                             : 1 + (gaps[0].size * 2)
                                               + (top_corner_side == 0 ? 1 : 0)
                                               + (bottom_corner_side == 0 ? 1 : 0);
                size_t end_wall_idx = is_x ? 2 + (gaps[0].size * 2) : num_walls - 1;
                
                walls[start_wall_idx] = !ko_start_wall;
                walls[end_wall_idx] = !ko_end_wall;

                /* UUUUGHHH */
                size_t wall_idx_offsets[2] = { !is_x && top_corner_side == 0, !is_x && bottom_corner_side == 1 };

                /* Special cased stuff for vertical walls since they might have corners... 
                 * Man do I ever hate this function 
                 * The reason that I have to insert at shape.points.size - 1 and 1 is because the vectors haven't expanded yet;
                 * even though I know the final indices, I can't use them because they're likely outside the vector right now */
                if (!is_x) {
                    if (top_corner_side == 1) {
                        walls[end_wall_idx - 1] = 0;
                        pb_point2D to_add = { room_rect.bottom_left.x + hallway_size, room_end.y - hallway_size };
                        if (pb_vector_insert_at(&next->shape.points, &to_add, next->shape.points.size - 1) == -1) {
                            err = 1;
                            break;
                        }
                    } 
                    
                    if (bottom_corner_side == 0) {
                        walls[start_wall_idx - 1] = 0;
                        pb_point2D to_add = { room_rect.bottom_left.x, room_start.y + hallway_size };
                        if (pb_vector_insert_at(&next->shape.points, &to_add, 1) == -1) {
                            err = 1;
                            break;
                        }
                    }
                }

                size_t gap_list_idx, gap_idx;
                for (gap_list_idx = 0; gap_list_idx < 2 && !err; ++gap_list_idx) {
                    pb_wall_pair* pairs = (pb_wall_pair*)gaps[gap_list_idx].items;
                    int reversed = gap_list_idx == reverse;

                    /* Where to insert the points in the room shape's point list 
                     * TODO: Come up with a less ridiculous way to determine this */
                    size_t point_insert_idx = reversed ? (end_wall_idx + 2) % num_walls : (start_wall_idx + 2) % num_walls;
                    int wall_idx = point_insert_idx + wall_idx_offsets[gap_list_idx];

                    for (gap_idx = 0; gap_idx < gaps[gap_list_idx].size; ++gap_idx) {
                        size_t point_idx = reversed ? gaps[gap_list_idx].size - 1 - gap_idx : gap_idx;
                        pb_point2D start_point = reversed ? pairs[point_idx].end : pairs[point_idx].start;
                        pb_point2D end_point = reversed ? pairs[point_idx].start : pairs[point_idx].end;

                        if (point_insert_idx == 0) {
                            if (pb_vector_push_back(&next->shape.points, &end_point) == -1 ||
                                pb_vector_push_back(&next->shape.points, &start_point) == -1) {

                                err = 1;
                                break;
                            }
                        } else {
                            if (pb_vector_insert_at(&next->shape.points, &end_point, point_insert_idx) == -1 ||
                                pb_vector_insert_at(&next->shape.points, &start_point, point_insert_idx) == -1) {

                                err = 1;
                                break;
                            }
                        }

                        walls[wall_idx] = 0;
                        walls[(wall_idx - 1) % num_walls] = 1;
                        wall_idx += 2;
                    }

                    walls[(wall_idx - 1) % num_walls] = 1;
                }

                if (!is_x) {
                    if (top_corner_side == 0) {
                        walls[0] = 0;
                        pb_point2D to_add = { room_rect.bottom_left.x, room_end.y - hallway_size };
                        if (pb_vector_push_back(&next->shape.points, &to_add) == -1) {
                            err = 1;
                            break;
                        }
                    }

                    if (bottom_corner_side == 1) {
                        walls[start_wall_idx + 1] = 0;
                        pb_point2D to_add = { room_rect.bottom_left.x + hallway_size, room_start.y + hallway_size };
                        if (pb_vector_insert_at(&next->shape.points, &to_add, start_wall_idx + 2) == -1) {
                            err = 1;
                            break;
                        }
                    }
                }
                ko_start_wall = 0;
                ko_end_wall = 0;
                add_room = 0;
            }
        }
        pb_vector_free(&gaps[0]);
        pb_vector_free(&gaps[1]);
        if (err) {
            goto err_return;
        }
    }

    if (reconstruct_floor_graph(floor_graph, f, new_num_rooms - old_num_rooms, hspec, room_specs) == -1) {
        /* :( */
        goto err_return;
    }

    /* Clean up */
    pb_graph_free(pruned);
    pb_vector_free(&point_queue);

    for (i = 0; i < hallway_segments.size; ++i) {
        pb_vector* vec = ((pb_vector*)hallway_segments.items) + i;
        pb_vector_free(vec);
    }
    pb_vector_free(&hallway_segments);
    pb_hashmap_for_each(segments_disjoint_set, pb_hashmap_free_entry_data, NULL);
    pb_hashmap_free(segments_disjoint_set);
    return 0;

err_return:
    /* Clean up everything that exists */
    pb_graph_free(pruned);
    pb_vector_free(&point_queue);

    if (hallway_segments.items) {
        for (i = 0; i < hallway_segments.size; ++i) {
            pb_vector* vec = ((pb_vector*)hallway_segments.items) + i;
            pb_vector_free(vec);
        }
        pb_vector_free(&hallway_segments);
    }

    if (segments_disjoint_set) {
        pb_hashmap_for_each(segments_disjoint_set, pb_hashmap_free_entry_data, NULL);
        pb_hashmap_free(segments_disjoint_set);
    }
    return -1;
}

static void add_vertex_doors(void const* vert_id, pb_vertex* vert, void* params) {
    pb_room* room = (pb_room*)vert->data;
    size_t i;
    size_t num_doors = 0;
    pb_pair* params_pair = (pb_pair*)params;
    int* err = (int*)params_pair->first;
    pb_sq_house_house_spec* hspec = (pb_sq_house_house_spec*)params_pair->second;


    for (i = 0; i < vert->edges_size; ++i) {
        pb_sq_house_room_conn* conn = (pb_sq_house_room_conn*)vert->edges[i]->data;
        num_doors += conn->has_door;
    }

    /* Technically this should always be true, but it sometimes won't be in the current implementation.
     * If it's not, just mark the room as having no doors and don't bother setting its pointer. */
    if (num_doors) {
        pb_wall_structure* doors = malloc(sizeof(pb_wall_structure) * num_doors);
        if (!doors) {
            *err = 1;
            return;
        }

        size_t cur_door = 0;
        for (i = 0; i < vert->edges_size; ++i) {
            pb_sq_house_room_conn* conn = (pb_sq_house_room_conn*) vert->edges[i]->data;

            if (conn->has_door) {
                float xdiff = conn->overlap_end.x - conn->overlap_start.x;
                float ydiff = conn->overlap_end.y - conn->overlap_start.y;
                int is_x = xdiff > ydiff;

                pb_point2D centre = {conn->overlap_start.x + xdiff / 2, conn->overlap_start.y + ydiff / 2};

                doors[cur_door].start.x = centre.x;
                doors[cur_door].start.y = centre.y;
                doors[cur_door].end.x = centre.x;
                doors[cur_door].end.y = centre.y;

                if (is_x) {
                    doors[cur_door].start.x -= hspec->door_size / 2;
                    doors[cur_door].end.x += hspec->door_size / 2;
                } else {
                    doors[cur_door].start.y -= hspec->door_size / 2;
                    doors[cur_door].end.y += hspec->door_size / 2;
                }

                doors[cur_door].wall = (size_t)conn->wall;
                ++cur_door;
            }
        }

        room->doors = doors;
        room->num_doors = num_doors;
    } else {
        room->num_doors = 0;
    }
}

int pb_sq_house_place_doors(pb_floor* f, pb_sq_house_house_spec* hspec, pb_graph* floor_graph, int is_first_floor) {
    int err = 0;
    pb_pair params = {&err, hspec};

    /* Just in case we have to clean them up later */
    /* TODO: Initialise everything in one place so that cleanup becomes easier */
    size_t i;
    for (i = 0; i < f->num_rooms; ++i) {
        f->rooms[i].doors = NULL;
        f->rooms[i].num_doors = 0;
    }

    /* If there's no floor graph, then there's a single room on a single floor */
    if (floor_graph) {
        pb_graph_for_each_vertex(floor_graph, add_vertex_doors, &params);
    }

    /* Add a door to the bottom wall in the first room on the first floor so that we can get outside */
    if (is_first_floor) {
        pb_wall_structure* floor_doors = malloc(sizeof(pb_wall_structure));
        if (!floor_doors) {
            f->doors = NULL;
            f->num_doors = 0;
            return -1;
        }

        pb_wall_structure* room0_doors = realloc(f->rooms[0].doors, sizeof(pb_wall_structure) * (f->rooms[0].num_doors + 1));
        if (!room0_doors) {
            return -1;
        }
        f->rooms[0].doors = room0_doors;

        f->doors = floor_doors;
        ++f->rooms[0].num_doors;

        pb_point2D const* room0_points = (pb_point2D*)f->rooms[0].shape.points.items;
        pb_point2D origin = {0.f, 0.f};
        pb_point2D next;
        for (i = 0; i < f->rooms[0].shape.points.size; ++i) {
            if (pb_point_eq(room0_points + i, &origin)) {
                next = room0_points[(i + 1) % f->rooms[0].shape.points.size];
                break;
            }
        }

        pb_point2D centre = {0.f + next.x / 2, 0.f};
        size_t idx = f->rooms[0].num_doors - 1;
        f->rooms[0].doors[idx].start.x = centre.x - hspec->door_size / 2;
        f->rooms[0].doors[idx].start.y = 0.f;
        f->rooms[0].doors[idx].end.x = centre.x + hspec->door_size / 2;
        f->rooms[0].doors[idx].end.y = 0.f;
        f->rooms[0].doors[idx].wall = i;

        f->doors[0].start = f->rooms[0].doors[idx].start;
        f->doors[0].end = f->rooms[0].doors[idx].end;
        f->doors[0].wall = 1;
        f->num_doors = 1;

    } else {
        f->num_doors = 0;
        f->doors = NULL;
    }

    return err ? -1 : 0;
}

int pb_sq_house_place_windows(pb_floor* f, pb_sq_house_house_spec* hspec, int is_first_floor) {
    pb_rect floor_rect;
    pb_point2D top_right;
    pb_point2D bottom_left;
    pb_shape2D_to_pb_rect(&f->shape, &floor_rect);
    top_right.x = floor_rect.bottom_left.x + floor_rect.w;
    top_right.y = floor_rect.bottom_left.y + floor_rect.h;
    bottom_left = floor_rect.bottom_left;

    size_t i;
    for (i = 0; i < f->num_rooms; ++i) {
        f->rooms[i].windows = NULL;
        f->rooms[i].num_windows = 0;
    }

    f->windows = NULL;
    f->num_windows = 0;

    for (i = 0; i < f->num_rooms; ++i) {
        /* Place all windows possible in this room */
        pb_room *room = f->rooms + i;
        pb_point2D const *points = (pb_point2D *) room->shape.points.items;

        size_t num_windows = 0;
        size_t cur_point;
        for (cur_point = 0; cur_point < room->shape.points.size; ++cur_point) {
            pb_point2D const *p = points + cur_point;
            pb_point2D const *next = points + ((cur_point + 1) % room->shape.points.size);
            float xdiff = next->x - p->x;
            float ydiff = next->y - p->y;

            pb_point2D centre = {p->x + xdiff / 2, p->y + ydiff / 2};

            int is_on_edge = pb_float_approx_eq(centre.x, bottom_left.x, 5) ||
                             pb_float_approx_eq(centre.x, top_right.x, 5) ||
                             pb_float_approx_eq(centre.y, bottom_left.y, 5) ||
                             pb_float_approx_eq(centre.y, top_right.y, 5);

            if (is_on_edge) {
                float abs_xdiff = fabsf(xdiff);
                float abs_ydiff = fabsf(ydiff);

                int is_x = abs_xdiff > abs_ydiff;

                /* Only add the window if it fits */
                if ((is_x ? abs_xdiff : abs_ydiff) > hspec->window_size) {
                    num_windows += !(is_first_floor && i == 0 && pb_point_eq(p, &bottom_left));
                }
            }
        }

        if (num_windows) {
            pb_wall_structure *windows = malloc(sizeof(pb_wall_structure) * num_windows);
            if (!windows) {
                return -1;
            }

            pb_wall_structure *floor_windows = realloc(f->windows,
                                                       sizeof(pb_wall_structure) * (f->num_windows + num_windows));
            if (!floor_windows) {
                free(windows);
                return -1;
            }
            f->windows = floor_windows;

            room->num_windows = num_windows;
            room->windows = windows;

            /* Go over the points again, and this time, actually add the windows */
            size_t cur_window = 0;
            for (cur_point = 0; cur_point < room->shape.points.size; ++cur_point) {
                pb_point2D const *p = points + cur_point;
                pb_point2D const *next = points + ((cur_point + 1) % room->shape.points.size);
                float xdiff = next->x - p->x;
                float ydiff = next->y - p->y;
                pb_point2D centre = {p->x + xdiff / 2, p->y + ydiff / 2};

                int is_top = pb_float_approx_eq(centre.y, top_right.y, 5);
                int is_bottom = pb_float_approx_eq(centre.y, bottom_left.y, 5);
                int is_left = pb_float_approx_eq(centre.x, bottom_left.x, 5);
                int is_right = pb_float_approx_eq(centre.x, top_right.x, 5);

                int is_on_edge = is_top || is_bottom || is_left || is_right;
                if (is_on_edge && !(is_first_floor && i == 0 && pb_point_eq(p, &bottom_left))) {
                    float abs_xdiff = fabsf(xdiff);
                    float abs_ydiff = fabsf(ydiff);
                    int is_x = abs_xdiff > abs_ydiff;

                    /* Window fits */
                    if ((is_x ? abs_xdiff : abs_ydiff) > hspec->window_size) {
                        pb_point2D window_start_delta = {is_x ? hspec->window_size / -2 : 0.f,
                                                         is_x ? 0.f : hspec->window_size / -2};
                        pb_point2D window_end_delta = {is_x ? hspec->window_size / 2 : 0.f,
                                                       is_x ? 0.f : hspec->window_size / 2};

                        room->windows[cur_window].start.x = centre.x + window_start_delta.x;
                        room->windows[cur_window].start.y = centre.y + window_start_delta.y;
                        room->windows[cur_window].end.x = centre.x + window_end_delta.x;
                        room->windows[cur_window].end.y = centre.y + window_end_delta.y;
                        room->windows[cur_window].wall = cur_point;

                        size_t floor_wall;
                        if (is_top) {
                            floor_wall = is_left ? 0 : 3;
                        } else if (is_bottom) {
                            floor_wall = is_right ? 2 : 1;
                        } else if (is_left) {
                            /* We already know it's not top or bottom */
                            floor_wall = 0;
                        } else {
                            floor_wall = 2;
                        }

                        f->windows[f->num_windows + cur_window].start = room->windows[cur_window].start;
                        f->windows[f->num_windows + cur_window].end = room->windows[cur_window].end;
                        f->windows[f->num_windows + cur_window].wall = floor_wall;

                        ++cur_window;
                    }
                }
            }
            f->num_windows += num_windows;
        }
    }
    return 0;
}