#ifndef PB_EXTRUSION_H
#define PB_EXTRUSION_H

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

    /* TODO: Add doors and windows */
} pb_room;

/**
* Represents a floor, which contains a number of rooms
*/
typedef struct _pb_floor {
    pb_room *rooms;
    size_t num_rooms;

    /* The shape occupied by the floor. Contains all rooms in the
    * given floor. */
    pb_shape floor_shape;

    /* TODO: Add doors and windows */
} pb_floor;

typedef struct _pb_building {
    pb_floor *floors;
    size_t num_floors;
} pb_building;

#ifdef __cplusplus
}
#endif

#endif /* PB_EXTRUSION_H */