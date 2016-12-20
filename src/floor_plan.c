#include <pb/floor_plan.h>
#include <stdlib.h>

PB_DECLSPEC void PB_CALL pb_room_free(pb_room* room, pb_room_free_func r_free) {
    r_free(room);
    pb_shape2D_free(&room->shape);
    pb_vector_free(&room->walls);
    free(room->doors);
    free(room->windows);
}

PB_DECLSPEC void PB_CALL pb_floor_free(pb_floor* f, pb_floor_free_func f_free, pb_room_free_func r_free) {
    size_t cur_room;
    for (cur_room = 0; cur_room < f->num_rooms; ++cur_room) {
        pb_room_free(f->rooms + cur_room, r_free);
    }

    free(f->rooms);
    pb_shape2D_free(&f->shape);
    free(f->doors);
    free(f->windows);
}

PB_DECLSPEC void PB_CALL pb_building_free(pb_building* building, pb_building_free_func b_free, pb_floor_free_func f_free,
                                          pb_room_free_func r_free) {
    size_t cur_floor;
    for(cur_floor = 0; cur_floor < building->num_floors; ++cur_floor) {
        pb_floor_free(building->floors + cur_floor, f_free, r_free);
    }

    b_free(building);
    free(building->floors);
}
