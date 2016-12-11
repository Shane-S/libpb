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
        pb_hashmap_get(room_specs, floor->rooms[i].name, (void**)&spec);

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
                    if (strcmp(floor->rooms[j].name, spec->adjacent[adj]) == 0) {
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
 * []   My "cleverness" in parts basically just made it an unreadable mess
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

                /* Keep track of how many 4-way intersections there are to realloc the floor's rooms array later */
                if (cur->edges_size == 4) {
                    num_4way++;
                }

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
        pb_vertex const** points = (pb_vertex**)segment->items;

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
                    }
                    else {
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
                }
                else {
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
                        if (pb_vector_insert_at(&next->shape.points, &to_add, num_walls - 1) == -1) {
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
    /* Reconstruct floor graph */

    /* Clean up */
    pb_graph_free(pruned);
    pb_vector_free(&point_queue);

    for (i = 0; i < hallway_segments.size; ++i) {
        pb_vector* vec = ((pb_vector*)hallway_segments.items) + i;
        pb_vector_free(vec);
    }
    pb_vector_free(&hallway_segments);
    pb_hashmap_for_each(segments_disjoint_set, pb_hashmap_free_entry_data, NULL);
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
    }
    return -1;
}