#ifndef PB_SQ_HOUSE_H
#define PB_SQ_HOUSE_H

#include <pb/exports.h>
#include <pb/floor_plan.h>
#include <pb/util/hashmap/hashmap.h>
#include <pb/util/geom/types.h>

#include <stddef.h>
#include <stdint.h>

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

#ifdef __cplusplus
extern "C" {
#endif

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

    /* The width or height of a door in the plan. Note that this may be adjusted to fit smaller walls (a custom door
     * extruder in the extrusion algorithm can decide whether the placed door is too small and include/not include it
     * accordingly). */
    float door_size;
} pb_sq_house_house_spec;

PB_DECLSPEC pb_building* PB_CALL pb_sq_house(pb_sq_house_house_spec* house_spec, pb_hashmap* room_specs);

#ifdef __cplusplus
}
#endif
#endif /* PB_SQ_HOUSE_H */
