#ifndef PB_FLOOR_PLAN_H
#define PB_FLOOR_PLAN_H

#include <pb/util/geom/types.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents a room in a building.
 */
typedef struct {
    /* The polygon occupied by the room. */
    pb_shape2D shape;

    /* Specifies which sides of the shape have walls (non-zero for a given index means there is a wall) */
    pb_vector walls;

    pb_point2D* doors;
    size_t num_doors;

    pb_point2D* windows;
    size_t num_windows;

    int has_floor;
    int has_ceiling;

    /* Data supplied by the algorithm that created the building plan. */
    void *data;
    char const* name;
} pb_room;

/**
 * Represents a floor, which contains a number of rooms.
 */
typedef struct {
    pb_room *rooms;
    size_t num_rooms;

    /* The shape occupied by the floor. Contains all rooms in the
     * given floor. */
    pb_shape2D shape;

    pb_point2D* doors;
    size_t num_doors;

    pb_point2D* windows;
    size_t num_windows;

    /* Data supplied by the algorithm that created the building plan. */
    void *data;
} pb_floor;

typedef struct {
    pb_floor *floors;
    size_t num_floors;

    /* Data supplied by the algorithm that created the building plan. */
    void *data;

    /* If non-zero, then the name pointer in every pb_room points to a
     * null-terminated string representing the room's name. The pointers' contents
     * are undefined otherwise.
     * 
     * The floor plan generation algorithm must set this variable. */
    int has_names;
} pb_building;

#ifdef __cplusplus
}
#endif

#endif /* PB_FLOOR_PLAN_H */
