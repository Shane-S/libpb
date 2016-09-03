#ifndef PB_SQ_HOUSE_INTERNAL_H
#define PB_SQ_HOUSE_INTERNAL_H

#include <pb/pb_sq_house.h>
#include <pb/internal/pb_squarify.h>

/**
 * Determines which rooms will go be in the house.
 *
 * @param room_specs The specifications for each room type.
 * @param house_spec The specifications for the house.
 */
char** pb_sq_house_choose_rooms(pb_hash* room_specs, pb_sq_house_house_spec* house_spec);

/**
 * Determines the number of floors in the house, allocates an appropriately sized pb_room list for each, and inserts
 * stairs on each floor.
 *
 * @param rooms      The rooms chosen for the house by pb_sq_house_choose_rooms.
 * @param room_specs The hash map containing the room specification for each room.
 * @param h_spec     The house specification (containing the total number of rooms).
 * @param house      The floor plan for the building.
 *
 * @returns A list of rectangles indicating the free space on the corresponding floor.
 */
pb_rect* pb_sq_house_layout_stairs(char const** rooms, pb_hash* room_specs, pb_sq_house_house_spec* h_spec, pb_building* house);

#endif /* PB_SQ_HOUSE_INTERNAL */