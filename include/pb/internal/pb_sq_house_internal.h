#ifndef PB_SQ_HOUSE_INTERNAL_H
#define PB_SQ_HOUSE_INTERNAL_H

#include <pb/pb_sq_house.h>
#include <pb/internal/pb_squarify.h>

typedef enum side {
    TOP = 0,
    LEFT = 1,
    BOTTOM = 2,
    RIGHT = 3,
} side;

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
 * @returns A list of rectangles indicating the free space on each corresponding floor.
 */
pb_rect* pb_sq_house_layout_stairs(char const** rooms, pb_hash* room_specs, pb_sq_house_house_spec* h_spec, pb_building* house);

/**
 * Lays out the specified number of rooms on the given floor using pb_squarify.
 *
 * @param rooms      The list of rooms to be laid out in the house. This pointer should start at the first room to be laid out on this floor.
 * @param room_specs The map of room names to room specifications.
 * @param floor      The floor on which the rooms will be placed.
 * @param floor_rect The rectangle of available space on the floor.
 *
 * @return 0 on success, -1 on failure (out of memory). Note that on returning -1, all room_shapes allocated on the this floor will have been freed;
 *         the caller must clean up all preceding floors.
 */
int pb_sq_house_layout_floor(char const** rooms, pb_hash* room_specs, pb_floor* floor, size_t num_rooms, pb_rect* floor_rect);

/**
 * Fills in any remaining space after pb_squarify has run.
 *
 * @param final_floor_rect  The final floor rectangle after pb_squarify.
 * @param rect_has_children Whether the final rectangle from pb_squarify had children.
 * @param last_row_start    Pointer the rectangle child_rects that starts the last row in pb_squarify.
 * @param last_row_size     The number of rectangles in the last row.
 */
void pb_sq_house_fill_remaining_floor(pb_rect* final_floor_rect, int rect_has_children, pb_rect* last_row_start, size_t last_row_size);

/**
 * Determines which wall (if any) is shared by room1 and room2.
 *
 * @param room1 The first room to check for a shared wall.
 * @param room2 The second room to check for a shared wall.
 *
 * @return The appropriate value from the side enum if there is a shared wall,
 *         -1 if there are no shared walls between the rooms.
 */
int pb_sq_house_get_shared_wall(pb_room* room1, pb_room* room2);

/**
 * Gets the overlap between the shared walls in room1 and room2.
 *
 * Note that end-start in the dimension of overlap will always be increasing.
 * E.g. if the top wall intersected, end.x - start.x will always be >= 0.
 *
 * @param room1 The first room (must be the same as room1 in pb_sq_house_shared_wall).
 * @param room2 The second room (must be the same as room2 in pb_sq_house_shared_wall).
 * @param wall  The wall shared between the rooms (as per pb_sq_house_shared_wall).
 *              Must be [1, 4]; if it's outside this range, the function returns without modifying
 *              its arguments.
 * @param start Holds the start point of overlap.
 * @param end   Holds the end point of overlap.
 */
void pb_sq_house_get_wall_overlap(pb_room const* room1, pb_room const* room2, int wall, pb_point* start, pb_point* end);
#endif /* PB_SQ_HOUSE_INTERNAL */