#include <stdlib.h>
#include <string.h>
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
    free(sorted);
    
    /* Room specifications don't provide enough instances to meet the desired number */
    if (num_added != house_spec->num_rooms) {
        free(result);
        return NULL;
    } else {
        shuffle_arr(result, house_spec->num_rooms);
        return result;
    }
}