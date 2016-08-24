#ifndef PB_GENERATION_H
#define PB_GENERATION_H

#include <pb/pb_exports.h>
#include <pb/pb_types.h>
#include <pb/pb_geom.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* Represents a room in a building.
*/
typedef struct _pb_room {
    char *name;

    /* The polygon occupied by the room. */
    pb_shape room_shape;
} pb_room;

/**
* Represents a floor, which contains a number of rooms
*/
typedef struct _pb_floor {
    pb_room *rooms;
    size_t num_rooms;

    /* Pairs of points marking the start and end of line segments
    * representing windows. There are num_windows * 2 points in the
    * list. */
    pb_point *windows;
    size_t num_windows;

    /* Pairs of points marking the start and end of line segments
    * representing doors. There are num_doors * 2 points in the
    * list. */
    pb_point *doors;
    size_t num_doors;

    /* The rectangle occupied by the floor. Contains all rooms in the
    * given floor. */
    pb_shape floor_shape;
} pb_floor;

typedef struct _pb_building {
    pb_floor *floors;
    size_t num_floors;
} pb_building;


/**
 * Generates a single-floor building, using up to num_rooms rooms, with a floor of size w by h.
 *
 * @param room_types Descriptions of each room
 */
PB_DECLSPEC pb_building* PB_CALL pb_gen_single_floor(pb_room* room_types, size_t num_rooms, size_t w, size_t h);

/**
 *
 */
PB_DECLSPEC pb_building* PB_CALL pb_gen_multi_floor(pb_room* room_types, size_t num_rooms, size_t w, size_t h);


#ifdef __cplusplus
}
#endif
#endif /* PB_GENERATION_H */