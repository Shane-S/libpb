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
    char const *name;

    /* The rectangle occupied by the room. Currently there are no plans
    * to support shapes other than rectangles, but it might be wise to
    * change this to pb_shape in case I decide to add those later (or
    * risk breaking code that relies on this version). */
    pb_rect rect;
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
} pb_floor;

typedef struct _pb_building {
    pb_floor *floors;
    size_t num_floors;
} pb_building;

#ifdef __cplusplus
}
#endif

#endif /* PB_EXTRUSION_H */