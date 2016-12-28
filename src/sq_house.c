#include <pb/sq_house.h>
#include <pb/internal/sq_house_layout.h>
#include <pb/internal/sq_house_graph.h>
#include <pb/floor_plan.h>
#include <stdio.h>

PB_DECLSPEC pb_building* PB_CALL pb_sq_house(pb_sq_house_house_spec* house_spec, pb_hashmap* room_specs) {
    pb_building* b = malloc(sizeof(pb_building));
    if (!b) {
        return NULL;
    }
    b->has_names = 1;

    char const** room_list = (char const**)pb_sq_house_choose_rooms(room_specs, house_spec);
    if (!room_list) {
        return NULL;
    }

    pb_rect* floor_rects = pb_sq_house_layout_stairs(room_list, room_specs, house_spec, b);
    if (!floor_rects) {
        free(room_list);
        return NULL;
    }

    /* A single room in the house - just place doors + windows and exit */
    if (b->num_floors == 1 && b->floors[0].num_rooms == 1) {
        pb_room* first_room = b->floors[0].rooms;

        first_room->shape.points.items = NULL;
        first_room->walls.items = NULL;

        if (pb_rect_to_pb_shape2D(floor_rects, &b->floors[0].rooms[0].shape) == -1 ||
                pb_vector_init(&b->floors[0].rooms[0].walls, sizeof(int), 4) == -1) {
            
            pb_shape2D_free(&b->floors[0].rooms[0].shape);
            pb_vector_free(&b->floors[0].rooms[0].walls);
            free(b->floors[0].rooms[0].doors);

            pb_shape2D_free(&b->floors[0].shape);
            
            free(b);
            free(floor_rects);
            return NULL;
        }
        int* r0_walls = (int*)first_room->walls.items;
        r0_walls[0] = 1;
        r0_walls[1] = 1;
        r0_walls[2] = 1;
        r0_walls[3] = 1;
        first_room->walls.size = 4;

        first_room->has_ceiling = 1;
        first_room->has_floor = 1;
        first_room->name = room_list[0];
        first_room->data = NULL;

        if (pb_sq_house_place_doors(b->floors, house_spec, NULL, 1) == -1) {
            pb_shape2D_free(&b->floors[0].rooms[0].shape);
            pb_vector_free(&b->floors[0].rooms[0].walls);
            free(b->floors[0].rooms[0].doors);

            pb_shape2D_free(&b->floors[0].shape);
            free(b->floors[0].doors);

            free(b);
            free(floor_rects);
            return NULL;
        } else if (pb_sq_house_place_windows(b->floors, house_spec, 1) == -1) {
            pb_shape2D_free(&b->floors[0].rooms[0].shape);
            pb_vector_free(&b->floors[0].rooms[0].walls);
            free(b->floors[0].rooms[0].doors);
            free(b->floors[0].rooms[0].windows);

            pb_shape2D_free(&b->floors[0].shape);
            free(b->floors[0].doors);
            free(b->floors[0].windows);

            free(b);
            free(floor_rects);
            return NULL;
        }

        free(floor_rects);
        return b;
    }

    /* TODO: Create a function to initialise rooms/floor, because the variables tracking whether they've been added
     * are pretty hacky. */
    size_t cur_floor;
    size_t room_sum = 0;
    int added_doors;/* Whether we've added doors to the current floor */
    int added_windows; /* Whether we've added windows to the current floor. */
    for (cur_floor = 0; cur_floor < b->num_floors; ++cur_floor) {
        added_doors = 0;
        added_windows = 0;

        size_t num_stairs = b->num_floors > 1 ? (cur_floor > 0 && cur_floor < b->num_floors - 1 ? 2 : 1): 0;
        size_t actual_num_rooms = b->floors[cur_floor].num_rooms - num_stairs;

        const char** start_room = room_list + room_sum;
        room_sum += actual_num_rooms;
        if (pb_sq_house_layout_floor(start_room, room_specs, b->floors + cur_floor,
                                     actual_num_rooms, floor_rects + cur_floor, b->num_floors > 1 && cur_floor == 0) == -1) {
            cur_floor--;
            goto err_return;
        }

        pb_graph* floor_graph = pb_sq_house_generate_floor_graph(house_spec, room_specs, b->floors + cur_floor);
        if (!floor_graph) {
            goto err_return;
        }

        pb_hashmap* disconnected = pb_sq_house_find_disconnected_rooms(floor_graph, b->floors + cur_floor);
        if (!disconnected) {
            pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
            pb_graph_free(floor_graph);
            goto err_return;
        }

        if (disconnected->size > 0) {
            pb_graph* internal_graph = pb_sq_house_generate_internal_graph(floor_graph);
            if (!internal_graph) {
                pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
                pb_graph_free(floor_graph);
                pb_hashmap_free(disconnected);
                goto err_return;
            }

            pb_vector* hallways = pb_sq_house_get_hallways(b->floors + cur_floor, floor_graph, internal_graph, disconnected);

            if (!hallways) {
                pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
                pb_graph_free(floor_graph);
                pb_hashmap_free(disconnected);
                pb_graph_free(internal_graph);
                goto err_return;
            }

            /* TODO: Re-write hallway algorithm so that hallways are always found in this case */
            if (hallways->size) {
                if (pb_sq_house_place_hallways(b->floors + cur_floor, house_spec, room_specs, floor_graph,
                                               internal_graph, hallways) == -1) {
                    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
                    pb_graph_free(floor_graph);
                    pb_hashmap_free(disconnected);
                    pb_graph_free(internal_graph);
                    pb_vector_free(hallways);
                    free(hallways);
                    goto err_return;
                }
            }

            pb_graph_free(internal_graph);
            pb_vector_free(hallways);
            free(hallways);
        }
        pb_hashmap_free(disconnected);

        int door_place_result = pb_sq_house_place_doors(b->floors + cur_floor, house_spec, floor_graph, cur_floor == 0);
        added_doors = 1;

        pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
        pb_graph_free(floor_graph);

        if (door_place_result == -1) {
            goto err_return;
        }

        int window_place_result = pb_sq_house_place_windows(b->floors + cur_floor, house_spec, cur_floor == 0);
        added_windows = 1;
        if (window_place_result == -1) {
            goto err_return;
        }
    }

    free(floor_rects);
    free(room_list);

    return b;

err_return:
    if (cur_floor != -1) {
        do {
            pb_floor* f = b->floors + cur_floor;
            size_t cur_room;
            for (cur_room = 0; cur_room < f->num_rooms; ++cur_room) {
                pb_shape2D_free(&f->rooms[cur_room].shape);
                pb_vector_free(&f->rooms[cur_room].walls);
                if (added_doors) {
                    free(f->rooms[cur_room].doors);
                }
                if (added_windows) {
                    free(f->rooms[cur_room].windows);
                }
            }

            if (added_doors) {
                free(f->doors);
            }
            if (added_windows) {
                free(f->windows);
            }

            /* All floors before this one finished successfully */
            added_windows = 1;
            added_doors = 1;
        } while(cur_floor--);
    }
    free(b->floors);
    free(b);
    free(floor_rects);
    free(room_list);
    return NULL;
}


PB_DECLSPEC void PB_CALL pb_sq_house_free_room(pb_room const* room) {
    return;
}

PB_DECLSPEC void PB_CALL pb_sq_house_free_floor(pb_floor const* f) {
    return;
}

PB_DECLSPEC void PB_CALL pb_sq_house_free_building(pb_building const* building) {
    return;
}