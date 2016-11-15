#ifndef PB_EXTRUSION_H
#define PB_EXTRUSION_H

#include <pb/util/geom/types.h>

/**
 * A room corresponding to a pb_room specified in a floor plan.
 *
 * walls:   The walls encapsulating the room. Each wall corresponds to a pair of points in floor.shape, e.g. wall[0]
 *          contains the shapes for the wall from floor.shape[0] to floor.shape[1], etc. Note that if the shape indicated
 *          that a given wall wasn't connected, then the corresponding wall be set to NULL.
 * windows: The set of windows in the room, corresponding to the windows in a pb_room.
 * doors:   The set of doors in the room, corresponding to the doors in a pb_room.
 * ground:  The ground. If the corresponding pb_room has has_ground set to 0, then this will be NULL.
 * ceiling: The room's ceiling. If the corresponding pb_room has has_ceiling set to 0, then this will be NULL.
 */
typedef struct {
    pb_shape3D** walls;
    size_t* wall_shape_counts;

    pb_shape3D* windows;
    size_t num_windows;

    pb_shape3D* doors;
    size_t num_doors;

    pb_shape3D* ground;
    pb_shape3D* ceiling;
} pb_extruded_room;

/**
 * A floor corresponding to a pb_floor specified in a floor plan.
 *
 * rooms:   The rooms extruded by the extrusion algorithm. The number of rooms will be specified in the corresponding
 *          floor in the floor plan.
 * walls:   The walls encapsulating the floor. Each wall corresponds to a pair of points in floor.shape, e.g. wall[0]
 *          contains the shapes for the wall from floor.shape[0] to floor.shape[1], etc.
 * windows: The set of windows on the floor, corresponding to the windows in a pb_floor.
 * doors:   The set of doors on the floor, corresponding to the doors in a pb_floor.
 */
typedef struct {
    pb_extruded_room* rooms;

    pb_shape3D** walls;
    size_t* wall_counts;

    pb_shape3D* windows;
    size_t num_windows;

    pb_shape3D* doors;
    size_t num_door;
} pb_extruded_floor;

#ifdef __cplusplus
}
#endif

#endif /* PB_EXTRUSION_H */
