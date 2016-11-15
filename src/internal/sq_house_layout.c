#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pb/util/hashmap/hashmap.h>
#include <pb/util/hashmap/hash_utils.h>
#include <pb/sq_house.h>
#include <pb/internal/sq_house_layout.h>

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

char** pb_sq_house_choose_rooms(pb_hashmap* room_specs, pb_sq_house_house_spec* house_spec) {
    pb_sq_house_room_spec* sorted = malloc(room_specs->size * sizeof(pb_sq_house_room_spec));
    size_t i;
    size_t sorted_pos = 0;

    pb_hashmap* instances = pb_hashmap_create(pb_str_hash, pb_str_eq);
    size_t num_added = 0;
    int did_add = 1;
    int has_outside;

    char** result = malloc(sizeof(char*) * house_spec->num_rooms);

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
        pb_hashmap_put(instances, (void*)sorted[i].name, 0);
    }

    /* Starting with the highest priority rooms and working down the list, add a random number of each 
     * room type until the desired number of rooms have been added to the house (wrapping back the start
     * of the list when necessary) */
    while (num_added != house_spec->num_rooms && did_add) {
        did_add = 0;

        for (i = 0; i < room_specs->size && num_added != house_spec->num_rooms; ++i)  {
            void* num_placed;
            pb_sq_house_room_spec* spec;

            pb_hashmap_get(instances, (void*)sorted[i].name, &num_placed);
            pb_hashmap_get(room_specs, (void*)sorted[i].name, (void**)&spec);

            if ((size_t)num_placed < spec->max_instances) {
                size_t result_pos;

                /* Choose a number between 1 and (max instances - already placed) to add to the house */
                size_t remaining = spec->max_instances - (size_t)num_placed;
                size_t added = rand() % remaining + 1;
                if (added + num_added > house_spec->num_rooms) {
                    added = house_spec->num_rooms - num_added;
                }

                pb_hashmap_put(instances, (void*)sorted[i].name, (void*)((size_t)num_placed + added));

                /* Add num_added instances of the current room to the rooms array */
                for (result_pos = num_added; result_pos < num_added + added; ++result_pos) {
                    result[result_pos] = spec->name;
                }

                num_added += added;
                did_add = 1;
            }
        }
    }

    pb_hashmap_free(instances);

    /* Room specifications don't provide enough instances to meet the desired number */
    if (num_added != house_spec->num_rooms) {
        free(result);
        free(sorted);
        fprintf(stderr, "pb_sq_house: house specification's num_rooms exceeds sum of all room_spec max_instances\n");
        return NULL;
    } else {
        unsigned int outside_idx;
        shuffle_arr(result, house_spec->num_rooms);
        has_outside = 0;

        for (i = 0; i < house_spec->num_rooms && !has_outside; ++i) {
            unsigned int j;
            pb_sq_house_room_spec* spec;
            pb_hashmap_get(room_specs, (void*)result[i], (void**)&spec);
            for (j = 0; j < spec->num_adjacent; ++j) {
                if (strcmp(spec->adjacent[j], PB_SQ_HOUSE_OUTSIDE) == 0) {
                    /* Put the room that connects to outside at the start of the rooms list */
                    outside_idx = i;
                    has_outside = 1;
                    break;
                }
            }
        }

        /* No rooms that connect to outside were selected; we need to randomly replace one with a room that CAN connect to outside */
        if (!has_outside) {
            int outside_room = -1;
            outside_idx = rand() % (house_spec->num_rooms + 1);

            for (i = 0; i < room_specs->size && outside_room == -1; ++i) {
                unsigned int j;
                for (j = 0; j < sorted[i].num_adjacent; ++j) {
                    if (strcmp(sorted[i].adjacent[j], PB_SQ_HOUSE_OUTSIDE) == 0) {
                        outside_room = i;
                        break;
                    }
                }
            }

            if (outside_room == -1) {
                fprintf(stderr, "pb_sq_house: no rooms can connect to outside; at least one room must have PB_SQ_HOUSE_OUTSIDE in its adjacency list\n");
                free(result);
                result = NULL;
            } else {
                /* Replace room at the chosen index with a room that connects to outside */
                result[outside_idx] = sorted[outside_room].name;
            }
        } else {
            /* Move the outside-connecting room to the start */
            char* temp = result[outside_idx];
            result[outside_idx] = result[0];
            result[0] = temp;
        }

        free(sorted);
        return result;
    }
}

static void adjust_rect(pb_rect* rect, pb_point2D* bleft_adjust, float w_adjust, float h_adjust) {
    rect->bottom_left.x += bleft_adjust->x;
    rect->bottom_left.y += bleft_adjust->y;
    rect->w += w_adjust;
    rect->h += h_adjust;
}

static int add_stairs(pb_floor* f, unsigned int num_added, pb_shape2D* stair_shape, unsigned int stair_index) {
    pb_room* new_rooms = realloc(f->rooms, sizeof(pb_room) * (f->num_rooms + num_added));
    if (!new_rooms) return -1;

    /* Add the stairs to the list of rooms the current and next floors' room lists */
    f->rooms = new_rooms;
    f->num_rooms += num_added; /* + 1 since we're also adding stairs */
    f->rooms[stair_index].shape = *stair_shape;
    f->rooms[stair_index].data = (void*)PB_SQ_HOUSE_STAIRS;

    return 0;
}

pb_rect* pb_sq_house_layout_stairs(char const** rooms, pb_hashmap* room_specs, pb_sq_house_house_spec* h_spec, pb_building* house) {
    /* Stores sums of room areas added to the current floor */
    float* areas = NULL;
    
    /* Stores the available floor space after insertion of stairs on each floor */
    pb_rect* floor_rects = NULL;
    pb_rect current_floor_rect = { { 0.f, 0.f }, h_spec->width, h_spec->height };

    unsigned int num_rooms_added = 0;
    unsigned int current_floor = 0;
    unsigned int current_area_idx = 0;

    /* The width (or height depending on orientation of stairs) for any stair rooms. */
    float stair_width;
    float max_house_dim;
    side last_stair_loc = 0;

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
    max_house_dim = fmaxf(h_spec->width, h_spec->height);
    stair_width = fminf(0.25f * max_house_dim, h_spec->stair_room_width);

    /* Add stairs to each floor and add rectangle containing remaining space on each floor to list of floor rects */
    while (1) {    
        float current_floor_area = current_floor_rect.w * current_floor_rect.h;
        
        unsigned int current_room = 1;
        pb_sq_house_room_spec* spec;

        pb_hashmap_get(room_specs, (void*)rooms[num_rooms_added], (void**)&spec);
        areas[0] = spec->area;

        /* Add rooms to this floor until we have either added all rooms in the house or exceeded this floor's area */
        for (current_room; current_room + num_rooms_added < h_spec->num_rooms; ++current_room) {
            pb_hashmap_get(room_specs, (void*)rooms[current_room + num_rooms_added], (void**)&spec);
            areas[current_room] = areas[current_room - 1] + spec->area;
            if (areas[current_room] > current_floor_area) {
                break;
            }
        }

        /* If we've added all remaining rooms in the house, update the current floor and break out of the loop; 
         * otherwise, add a set of stairs, create a new floor, and continue */
        if (current_room + num_rooms_added == h_spec->num_rooms) {
            size_t total_rooms_on_floor = house->floors[current_floor].num_rooms + current_room; /* house->floors[current_floor].num_rooms is the number of stairs */
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
            pb_rect* new_floor_rects;

            pb_rect current_stair_rect = { 0 };
            pb_rect next_stair_rect = { 0 };
            pb_shape2D current_stair_shape = { 0 };
            pb_shape2D next_stair_shape = { 0 };
            unsigned int stair_index; /* The index into the rooms array where the stairs will be added for the current floor */

            /* The amount by which the bottom left corner of the current and next floor rectangles will need to be adjusted */
            pb_point2D bottom_left_adjustment = { 0.f, 0.f };
            float width_adjustment = 0.f;
            float height_adjustment = 0.f;

            pb_rect next_floor_rect = { { 0.f, 0.f }, h_spec->width, h_spec->height };

            /* Place the current and next stairs at the bottom left of their respective floors */
            current_stair_rect.bottom_left = current_floor_rect.bottom_left;
            next_stair_rect.bottom_left = next_floor_rect.bottom_left;

            /* Reallocate the floors array to hold another floor*/
            new_floors = realloc(house->floors, sizeof(pb_floor) * (house->num_floors + 1));
            if (!new_floors) {
                goto err_return;
            }
            house->floors = new_floors;
            house->floors[current_floor + 1].rooms = NULL;

            /* Don't have two stairs right beside each other */
            side new_stair_loc;
            do {
                new_stair_loc = (side)(rand() % 4);
            } while (new_stair_loc == last_stair_loc);


            /* Resize/move the current floor's rectangle according to the stair placement and do the same to the next floor's */
            switch (new_stair_loc) {
            case SQ_HOUSE_LEFT:
                bottom_left_adjustment.x = stair_width;
                width_adjustment = -stair_width;
                
                current_stair_rect.w = stair_width;
                current_stair_rect.h = current_floor_rect.h;

                next_stair_rect.w = stair_width;
                next_stair_rect.h = next_floor_rect.h;
                break;
            case SQ_HOUSE_RIGHT:
                width_adjustment = -stair_width;

                current_stair_rect.bottom_left.x = current_floor_rect.w - stair_width;
                current_stair_rect.w = stair_width;
                current_stair_rect.h = current_floor_rect.h;

                next_stair_rect.bottom_left.x = next_floor_rect.w - stair_width;
                next_stair_rect.w = stair_width;
                next_stair_rect.h = next_floor_rect.h;
                break;
            case SQ_HOUSE_TOP:
                height_adjustment = -stair_width;

                current_stair_rect.bottom_left.y = current_floor_rect.h - stair_width;
                current_stair_rect.w = current_floor_rect.w;
                current_stair_rect.h = stair_width;

                next_stair_rect.bottom_left.y = next_floor_rect.h - stair_width;
                next_stair_rect.w = next_floor_rect.w;
                next_stair_rect.h = stair_width;
                break;
            case SQ_HOUSE_BOTTOM:
                bottom_left_adjustment.y = stair_width;
                height_adjustment = -stair_width;

                current_stair_rect.w = current_floor_rect.w;
                current_stair_rect.h = stair_width;

                next_stair_rect.w = next_floor_rect.w;
                next_stair_rect.h = stair_width;
                break;
            }

            /* Resize the two rectangles */
            adjust_rect(&current_floor_rect, &bottom_left_adjustment, width_adjustment, height_adjustment);
            adjust_rect(&next_floor_rect, &bottom_left_adjustment, width_adjustment, height_adjustment);

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

            /* Convert the stair rectangles to pb_shape2D to add them to the floors */
            num_rooms_added += current_room;
            stair_index = house->floors[current_floor].num_rooms;
            
            if (pb_rect_to_pb_shape2D(&current_stair_rect, &current_stair_shape) == -1 ||
                pb_rect_to_pb_shape2D(&next_stair_rect, &next_stair_shape) == -1) {
                /* These were already 0-initialised earlier, so we can safely try to free both of them */
                pb_shape2D_free(&current_stair_shape);
                pb_shape2D_free(&next_stair_shape);
                goto err_return;
            }

            new_floor_rects = realloc(floor_rects, sizeof(pb_rect) * (house->num_floors + 1));
            if (!new_floor_rects) {
                goto err_return;
            }
            floor_rects = new_floor_rects;

            house->floors[current_floor + 1].num_rooms = 0;

            if (add_stairs(house->floors + current_floor, current_room + 1, &current_stair_shape, stair_index) == -1 ||
                add_stairs(house->floors + current_floor + 1, 1, &next_stair_shape, 0) == -1) {
                goto err_return;
            }

            house->floors[current_floor].rooms[stair_index + 1].shape.points.items = NULL; /* Stop here when freeing if we go to err_return */
            
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

    while (house->num_floors) {
        /* Only the shapes in rooms with non-null points arrays (i.e. the stairs) actually need to be freed */
        unsigned int i;
        for (i = 0; house->floors[house->num_floors - 1].rooms && house->floors[house->num_floors - 1].rooms[i].shape.points.items; ++i) {
            pb_shape2D_free(&house->floors[house->num_floors - 1].rooms[i].shape);
        }
        free(house->floors[house->num_floors - 1].rooms);
        house->num_floors--;
    }
    free(house->floors);
    return NULL;
}

int pb_sq_house_layout_floor(char const** rooms, pb_hashmap* room_specs, pb_floor* floor, size_t num_rooms, pb_rect* floor_rect) {
    float* areas = NULL;
    float total_area = 0.f;

    float floor_rect_area = floor_rect->w * floor_rect->h;
    pb_rect* rects = NULL;

    size_t num_stairs;
    size_t i;

    pb_rect* last_row_start;
    size_t last_row_size;
    int rect_has_children;

    /* If there's only one room on the floor besides the stairs, it will take up the entire rectangle regardless */
    if (num_rooms == 1) {
        if (pb_rect_to_pb_shape2D(floor_rect, &floor->rooms[floor->num_rooms - 1].shape) == -1) {
            return -1;
        }
        floor->rooms[floor->num_rooms - 1].data = (void*)rooms[0];
        return 0;
    }

    /* Otherwise, we have to squarify etc. */
    areas = malloc(sizeof(float) * num_rooms);
    if (!areas) {
        goto err_return;
    }

    for (i = 0; i < num_rooms; ++i) {
        pb_sq_house_room_spec* spec;
        pb_hashmap_get(room_specs, (void*)rooms[i], (void**)&spec);
        areas[i] = spec->area;
        total_area += areas[i];
    }

    pb_squarify(floor_rect, areas, num_rooms, rects, &last_row_start, &last_row_size, &rect_has_children);

    /* The total area of the rooms does't add up to the floor rectangle; expand the last rooms */
    if (total_area < floor_rect_area) {
        pb_sq_house_fill_remaining_floor(floor_rect, rect_has_children, last_row_start, last_row_size);
    }

    /* num_rooms is the number of rooms from the room specification list; floor->num_rooms is that number
     * PLUS the number of stairs already placed on the floor*/
    num_stairs = floor->num_rooms - num_rooms;
    /* Convert the rectangles from pb_squarify to pb_shape2Ds for each room */
    for (i = num_stairs; i < floor->num_rooms; ++i) {
        floor->rooms[i].data = (void*)rooms[i - num_stairs];
        floor->rooms[i].shape.points.items = NULL;

        if (pb_rect_to_pb_shape2D(&(rects[i - num_stairs]), &(floor->rooms[i].shape)) == -1) {
            goto err_return;
        }
    }

    free(areas);
    free(rects);
    return 0;

err_return:
    free(areas);
    free(rects);

    /* The caller will be responsible for cleaning all other floors up */
    for (i = 0; i < floor->num_rooms; ++i) {
        if (floor->rooms[i].shape.points.items == NULL) {
            break;
        } else {
            pb_shape2D_free(&floor->rooms[i].shape);
        }
    }

    return -1;
}

void pb_sq_house_fill_remaining_floor(pb_rect* final_floor_rect, int rect_has_children, pb_rect* last_row_start, size_t last_row_size) {
    size_t current_rect;

    /* The floor will have unfilled space in two cases */

    /* Case 1: at least two rectangles (the one-rectangle case is handled elsewhere) were laid out in final_floor_rectangle, but they don't 
     * completely fill it. */
    if (rect_has_children) {

        /* The rectangles are laid out along the smallest dimension of the parent rectangle, so increase their size in the 
         * other dimension to fill the space */
        int x_axis_min = last_row_start[0].bottom_left.y == last_row_start[1].bottom_left.y;
        float delta = x_axis_min ? final_floor_rect->h - last_row_start->h : final_floor_rect->w - last_row_start->w;

        for (current_rect = 0; current_rect < last_row_size; ++current_rect) {
            if (x_axis_min) {
                last_row_start[current_rect].h += delta;
            } else {
                last_row_start[current_rect].w += delta;
            }
        }

    } else {
        /* Case 2: final_floor_rect contains none of the rooms because we just finished laying out a row */
        /* pb_squarify always works towards the top-right corner, so the final rect will be either above or to the right */
        int is_right = last_row_start[0].bottom_left.x < final_floor_rect->bottom_left.x;

        /* Expand the rectangles in the last row to fill the remaining space */
        for (current_rect = 0; current_rect < last_row_size; ++current_rect) {
            if (is_right) {
                last_row_start[current_rect].w += final_floor_rect->w;
            } else {
                last_row_start[current_rect].h += final_floor_rect->h;
            }
        }
    }
}
