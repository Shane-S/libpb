#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pb/util/pb_hash.h>
#include <pb/util/pb_hash_utils.h>
#include <pb/pb_sq_house.h>
#include <pb/internal/pb_sq_house_internal.h>

static void shuffle_arr(char const** arr, size_t size) {
    size_t i;
    for (i = size - 1; i > 0; --i) {
        int num = rand() % (i + 1);
        char* tmp = arr[i];
        arr[i] = arr[num];
        arr[num] = tmp;
    }
}

int pb_sq_house_room_spec_cmp(void const* room_spec1, void const* room_spec2) {
    pb_sq_house_room_spec const* spec1 = (pb_sq_house_room_spec*)room_spec1;
    pb_sq_house_room_spec const* spec2 = (pb_sq_house_room_spec*)room_spec2;

    return spec1->priority - spec2->priority;
}

char** pb_sq_house_choose_rooms(pb_hash* room_specs, pb_sq_house_house_spec* house_spec) {
    pb_sq_house_room_spec* sorted = malloc(room_specs->size * sizeof(pb_sq_house_room_spec));
    size_t i;
    size_t sorted_pos = 0;

    pb_hash* instances = pb_hash_create(pb_str_hash, pb_str_eq);
    size_t num_added = 0;
    int did_add = 1;
    int has_outside;

    char const** result = malloc(sizeof(char*) * house_spec->num_rooms);

    /* Create a sorted copy of the hash map to select by priority */
    for (i = 0; i < room_specs->cap; ++i) {
        if (room_specs->states[i] == FULL) {
            memcpy(sorted + sorted_pos, room_specs->entries[i].val, sizeof(pb_sq_house_room_spec));
            ++sorted_pos;
        }
    }
    qsort(sorted, room_specs->size, sizeof(pb_sq_house_room_spec), pb_sq_house_room_spec_cmp);

    /* Initialise the room names -> number of instances map */
    for (i = 0; i < room_specs->size; ++i) {
        pb_hash_put(instances, (void*)sorted[i].name, 0);
    }

    /* Starting with the highest priority rooms and working down the list, add a random number of each 
     * room type until the desired number of rooms have been added to the house (wrapping back the start
     * of the list when necessary) */
    while (num_added != house_spec->num_rooms && did_add) {
        did_add = 0;

        for (i = 0; i < room_specs->size && num_added != house_spec->num_rooms; ++i)  {
            void* num_placed;
            pb_sq_house_room_spec* spec;

            pb_hash_get(instances, (void*)sorted[i].name, &num_placed);
            pb_hash_get(room_specs, (void*)sorted[i].name, (void**)&spec);

            if ((size_t)num_placed < spec->max_instances) {
                size_t result_pos;

                /* Choose a number between 1 and (max instances - already placed) to add to the house */
                size_t remaining = spec->max_instances - (size_t)num_placed;
                size_t added = rand() % remaining + 1;
                if (added + num_added > house_spec->num_rooms) {
                    added = house_spec->num_rooms - num_added;
                }

                pb_hash_put(instances, (void*)sorted[i].name, (void*)((size_t)num_placed + added));

                /* Add num_added instances of the current room to the rooms array */
                for (result_pos = num_added; result_pos < num_added + added; ++result_pos) {
                    result[result_pos] = spec->name;
                }

                num_added += added;
                did_add = 1;
            }
        }
    }

    pb_hash_free(instances);

    /* Room specifications don't provide enough instances to meet the desired number */
    if (num_added != house_spec->num_rooms) {
        free(result);
        free(sorted);
        fprintf(stderr, "pb_sq_house: house specification's num_rooms exceeds sum of all room_spec max_instances");
        return NULL;
    } else {
        int outside_idx;
        shuffle_arr(result, house_spec->num_rooms);
        has_outside = 0;

        for (i = 0; i < house_spec->num_rooms && !has_outside; ++i) {
            int j;
            pb_sq_house_room_spec* spec;
            pb_hash_get(room_specs, (void*)result[i], (void**)&spec);
            for (j = 0; j < spec->num_adjacent; ++j) {
                if (strcmp(spec->adjacent[j], PB_SQ_HOUSE_OUTSIDE) == 0) {
                    /* Put the room that connects to outside at the start of the rooms list */
                    outside_idx = i;
                    has_outside = 1;
                    break;
                }
            }
        }

        /* No rooms connect to outside were selected; we need to randomly replace one with a room that CAN connect to outside */
        if (!has_outside) {
            int outside_room = -1;
            outside_idx = rand() % (house_spec->num_rooms + 1);

            for (i = 0; i < room_specs->size && outside_room == -1; ++i) {
                int j;
                for (j = 0; j < sorted[i].num_adjacent; ++j) {
                    if (strcmp(sorted[i].adjacent[j], PB_SQ_HOUSE_OUTSIDE) == 0) {
                        outside_room = i;
                        break;
                    }
                }
            }

            if (outside_room == -1) {
                fprintf(stderr, "pb_sq_house: no rooms can connect to outside; at least one room must have PB_SQ_HOUSE_OUTSIDE in its adjacency list");
                free(result);
                result = NULL;
            }
        }

        free(sorted);
        return result;
    }
}

pb_rect* pb_sq_house_layout_stairs(char const** rooms, pb_hash* room_specs, pb_sq_house_house_spec* h_spec, pb_building* house) {
    /* Stores sums of room areas added to the current floor */
    float* areas = NULL;
    
    /* Stores the available floor space after insertion of stairs on each floor */
    pb_rect* floor_rects = NULL;
    pb_rect current_floor_rect = { { 0.f, 0.f }, h_spec->width, h_spec->height };

    int num_rooms_added = 0;
    int current_floor = 0;
    int current_area_idx = 0;

    /* The width (or height depending on orientation of stairs) for any stair rooms. */
    float stair_width;
    float max_house_dim;
    stair_location last_stair_loc = (stair_location)(rand() % 4);

    areas = malloc(sizeof(float) * h_spec->num_rooms);
    if (!areas) {
        return NULL;
    }

    house->floors = malloc(sizeof(pb_floor));
    if (!house->floors) {
        free(areas);
        return NULL;
    }
    house->num_floors = 1;
    house->floors[0].num_rooms = 0;
    house->floors[0].rooms = NULL;

    floor_rects = malloc(sizeof(pb_rect));
    if (!floor_rects) {
        goto err_return;
    }

    /* If the stair room width occupies more than a quarter of the largest house dimension, resize it */
    max_house_dim = h_spec->width > h_spec->height ? h_spec->width : h_spec->height;
    stair_width = 0.25 * max_house_dim < h_spec->stair_room_width ? 0.25 * max_house_dim : h_spec->stair_room_width;

    /* Add stairs to each floor and add rectangle containing remaining space on each floor to list of floor rects */
    while (1) {
        pb_rect next_floor_rect = { { 0.f, 0.f }, h_spec->width, h_spec->height };
        
        float current_floor_area = current_floor_rect.w * current_floor_rect.h;
        
        int current_room = 1;
        pb_sq_house_room_spec* spec;

        pb_hash_get(room_specs, (void*)rooms[num_rooms_added], (void**)&spec);
        areas[0] = spec->area;

        /* Add rooms to this floor until we have either added all rooms in the house or exceeded this floor's area */
        for (current_room; current_room + num_rooms_added < h_spec->num_rooms; ++current_room) {
            pb_hash_get(room_specs, (void*)rooms[current_room + num_rooms_added], (void**)&spec);
            areas[current_room] = areas[current_room - 1] + spec->area;
            if (areas[current_room] > current_floor_area) {
                break;
            }
        }

        /* If we've added all remaining rooms in the house, update the current floor and break out of the loop; 
         * otherwise, add a set of stairs, create a new floor, and continue */
        if (current_room + num_rooms_added == h_spec->num_rooms) {
            size_t total_rooms_on_floor = house->floors[current_floor].num_rooms + current_room;
            pb_room* new_rooms = realloc(house->floors[current_floor].rooms, sizeof(pb_room) * total_rooms_on_floor);
            
            if (!new_rooms)
                goto err_return;

            house->floors[current_floor].rooms = new_rooms;
            house->floors[current_floor].num_rooms += current_room;

            floor_rects[current_floor] = current_floor_rect;
            break;
        } else {
            /* Temp storage for reallocated arrays in case of allocation failure */
            pb_floor* new_floors;
            pb_room* new_rooms;
            pb_rect* new_floor_rects;

            pb_rect stair_rect;
            pb_shape stair_shape;
            int stair_index; /* The index into the rooms array where the stairs will be added for the current room */

            pb_rect next_floor_rect = { { 0.f, 0.f }, h_spec->width, h_spec->height };
            size_t total_rooms_on_floor;

            new_floors = realloc(house->floors, sizeof(pb_floor) * (house->num_floors + 1));
            if (!new_floors) {
                goto err_return;
            }
            house->floors = new_floors;
            house->floors[current_floor].rooms = NULL;
            house->floors[current_floor + 1].rooms = NULL;

            /* Don't have two stairs right beside each other */
            stair_location new_stair_loc;
            do {
                new_stair_loc = (stair_location)(rand() % 4);
            } while (new_stair_loc == last_stair_loc);

            /* Resize/move the current floor's rectangle according to the stair placement and do the same to the next floor's */
            switch (new_stair_loc) {
            case LEFT:
                current_floor_rect.bottom_left.x += stair_width;
                current_floor_rect.w -= stair_width;
                next_floor_rect.bottom_left.x += stair_width;
                next_floor_rect.w -= stair_width;

                stair_rect.bottom_left.x = 0.f;
                stair_rect.bottom_left.y = 0.f;
                stair_rect.w = stair_width;
                stair_rect.h = current_floor_rect.h;
                break;
            case RIGHT:
                current_floor_rect.w -= stair_width;
                next_floor_rect.w -= stair_width;

                stair_rect.bottom_left.x = current_floor_rect.w;
                stair_rect.bottom_left.y = 0.f;
                stair_rect.w = stair_width;
                stair_rect.h = current_floor_rect.h;
                break;
            case TOP:
                current_floor_rect.h -= stair_width;
                next_floor_rect.h -= stair_width;

                stair_rect.bottom_left.x = 0;
                stair_rect.bottom_left.y = current_floor_rect.h;
                stair_rect.w = current_floor_rect.w;
                stair_rect.h = stair_width;
                break;
            case BOTTOM:
                current_floor_rect.bottom_left.y += stair_width;
                current_floor_rect.h -= stair_width;
                next_floor_rect.bottom_left.y += stair_width;
                next_floor_rect.h -= stair_width;

                stair_rect.bottom_left.x = 0.f;
                stair_rect.bottom_left.y = 0.f;
                stair_rect.w = current_floor_rect.w;
                stair_rect.h = stair_width;
                break;
            }

            /* Determine which rooms can fit on the floor given the newly placed stairs */
            current_floor_area = current_floor_rect.w * current_floor_rect.h;
            for (current_room; current_room > 0; --current_room) {
                if (areas[current_room - 1] <= current_floor_area)
                    break;
            }

            /* No rooms fit on this floor with the stairs; this will be dealt with before squarification later, so say we added a single room for now */
            if (current_room == 0) {
                current_room = 1;
            }

            num_rooms_added += current_room;
            stair_index = house->floors[current_floor].num_rooms;
            if (!pb_rect_to_pb_shape(&stair_rect, &stair_shape)) {
                goto err_return;
            }

            new_floor_rects = realloc(floor_rects, sizeof(pb_rect) * (house->num_floors + 1));
            if (!new_floor_rects) {
                goto err_return;
            }
            floor_rects = new_floor_rects;

            new_rooms = realloc(house->floors[current_floor].rooms, sizeof(pb_room) * (house->floors[current_floor].num_rooms + current_room + 1));
            if (!new_rooms) goto err_return;

            /* Add the stairs to the list of rooms the current and next floors' room lists */
            house->floors[current_floor].rooms = new_rooms;
            house->floors[current_floor].num_rooms += current_room + 1; /* + 1 since we're also adding stairs */
            house->floors[current_floor].rooms[stair_index].room_shape = stair_shape;
            house->floors[current_floor].rooms[stair_index].data = (void*)PB_SQ_HOUSE_STAIRS;
            house->floors[current_floor].rooms[stair_index + 1].room_shape.points = NULL; /* Stop here when freeing if we go to err_return */

            new_rooms = realloc(house->floors[current_floor + 1].rooms, sizeof(pb_room) * 2);
            if (!new_rooms) goto err_return;

            house->floors[current_floor + 1].num_rooms = 1;
            house->floors[current_floor + 1].rooms = new_rooms;
            house->floors[current_floor + 1].rooms[0].room_shape = stair_shape;
            house->floors[current_floor + 1].rooms[0].data = (void*)PB_SQ_HOUSE_STAIRS;
            house->floors[current_floor + 1].rooms[1].room_shape.points = NULL;

            floor_rects[current_floor] = current_floor_rect;
            current_floor_rect = next_floor_rect;
            house->num_floors++;
            current_floor++;
        }
    }

    free(areas);
    return floor_rects;

err_return:
    free(areas);
    free(floor_rects);
    /* ANSI C and its lack of mixed code/declarations makes this a bit awkward, but we'll just
     * continuously decrement house->num_floors instead of making a new scope */
    while (house->num_floors) {
        /* Only the shapes in rooms with non-null points arrays (i.e. the stairs) actually need to be freed */
        int i;
        for (i = 0; house->floors[house->num_floors - 1].rooms && house->floors[house->num_floors - 1].rooms[i].room_shape.points; ++i) {
            free(house->floors[house->num_floors - 1].rooms[i].room_shape.points);
        }
        free(house->floors[house->num_floors - 1].rooms);
        house->num_floors--;
    }
    free(house->floors);
    return NULL;
}