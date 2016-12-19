#ifndef PB_FLOOR_PLAN_H
#define PB_FLOOR_PLAN_H

#include <pb/util/geom/types.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Find a better name for this... */
typedef struct {
    pb_point2D start;
    pb_point2D end;
    size_t wall;
} pb_wall_structure;

/**
 * Represents a room in a building.
 */
typedef struct {
    /* The polygon occupied by the room. */
    pb_shape2D shape;

    /* A vector of ints that specifies which sides of the shape have walls
     * (1 for a given index means there is a wall, 0 means there isn't). */
    pb_vector walls;

    /* The doors in this room. */
    pb_wall_structure* doors;
    size_t num_doors;

    /* The windows in this room. */
    pb_wall_structure* windows;
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

    pb_wall_structure* doors;
    size_t num_doors;

    pb_wall_structure* windows;
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
