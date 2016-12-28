#ifndef PB_SQ_HOUSE_H
#define PB_SQ_HOUSE_H

#include <pb/exports.h>
#include <pb/floor_plan.h>
#include <pb/util/hashmap/hashmap.h>
#include <pb/util/geom/types.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Names indicating a room for stairs and a hallway "room" in a house. These cannot be used as room names by client code. */
#define PB_SQ_HOUSE_STAIRS "Stairs"
#define PB_SQ_HOUSE_HALLWAY "Hallway"
#define PB_SQ_HOUSE_OUTSIDE "Outside"

/* Sides of a rectangle. Each value can also be used to refer to a side in a pb_shape2D converted from a
 * pb_rect; SQ_HOUSE_LEFT (0) is the side from point 0 to 1, SQ_HOUSE_BOTTOM is the side from point 1 to 2, etc. */
typedef enum side {
    SQ_HOUSE_NONE = -1,
    SQ_HOUSE_LEFT = 0,
    SQ_HOUSE_BOTTOM = 1,
    SQ_HOUSE_RIGHT = 2,
    SQ_HOUSE_TOP = 3,
} side;

typedef struct {
    char const* name;
    char const** adjacent;
    size_t num_adjacent;
    float area;
    unsigned int max_instances;
    unsigned priority;
} pb_sq_house_room_spec;

typedef struct {
    float height;
    float width;
    unsigned int num_rooms;

    /* The width (or height if the stairs run along the x-axis) of a room containing stairs.
     * This may be adjusted to fit stairs within the house properly. */
    float stair_room_width;

    /* The width (or height where hallway sections run along the x-axis) of hallways. This may be adjusted 
     * to fit the hallway on a given floor. */
    float hallway_width;

    /* The width or height of a door in the plan. Note that in the current implementation, if doors are too big, they
     * won't be placed in the plan at all, which could render some rooms accessible. */
    float door_size;

    /* The width or height of a window in the plan. */
    float window_size;
} pb_sq_house_house_spec;

PB_DECLSPEC pb_building* PB_CALL pb_sq_house(pb_sq_house_house_spec* house_spec, pb_hashmap* room_specs);

/* Hooks for freeing building data. Currently, these do nothing since the algorithm allocates no metadata. */
PB_DECLSPEC void PB_CALL pb_sq_house_free_room(pb_room const* room);
PB_DECLSPEC void PB_CALL pb_sq_house_free_floor(pb_floor const* f);
PB_DECLSPEC void PB_CALL pb_sq_house_free_building(pb_building const* building);

#ifdef __cplusplus
}
#endif
#endif /* PB_SQ_HOUSE_H */
