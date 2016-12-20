#ifndef PB_FLOOR_PLAN_H
#define PB_FLOOR_PLAN_H

#include <pb/exports.h>
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

/* Floor plan generators can clean up their data by defining cleanup functions for rooms, floors and the building.
 * Note that only the data (and names, for rooms) should be modified; all other struct members should be treated as
 * read-only.
 *
 * The order of function calls is:
 *
 * For each floor in the building
 *     For each room on the floor
 *         pb_room_free_func(room)
 *         ... free room data structures ...
 *
 *     pb_floor_free_func(floor)
 *     ... free floor data structures ...
 *
 * pb_building_free_func(building)
 * ... free building data structures ...
 * */
typedef void (*pb_room_free_func)(pb_room const* room);
typedef void (*pb_floor_free_func)(pb_floor const* f);
typedef void (*pb_building_free_func)(pb_building const* building);

/**
 * Frees a single room on a floor.
 * @param room   The room to free.
 * @param r_free A function that frees room metadata allocated by the floor plan generation algorithm.
 */
PB_DECLSPEC void pb_room_free(pb_room* room, pb_room_free_func r_free);

/**
 * Frees a single floor in a building. Note that the floor pointer itself will still be valid, but all of its members
 * will no longer be usable.
 *
 * @param f      The floor to free.
 * @param f_free A function that frees floor metadata allocated by the floor plan generation algorithm.
 * @param r_free A function that frees room metadata allocated by the floor plan generation algorithm.
 */
PB_DECLSPEC void pb_floor_free(pb_floor* f, pb_floor_free_func f_free, pb_room_free_func r_free);

/**
 * Frees each room and floor in the given building. Note: the building pointer itself will NOT have been freed after the
 * function has run, but all of its members will be invalid.
 *
 * @param building The building to free.
 * @param b_free   A function that frees building metadata allocated by the floor plan generation algorithm.
 * @param f_free   A function that frees floor metadata allocated by the floor plan generation algorithm.
 * @param r_free   A function that frees room metadata allocated by the floor plan generation algorithm.
 */
PB_DECLSPEC void PB_CALL pb_building_free(pb_building* building, pb_building_free_func b_free, pb_floor_free_func f_free,
                                          pb_room_free_func r_free);
#ifdef __cplusplus
}
#endif

#endif /* PB_FLOOR_PLAN_H */
