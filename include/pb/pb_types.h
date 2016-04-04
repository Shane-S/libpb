#ifndef PB_BUILDING_H
#define PB_BUILDING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _pb_point {
    float x; 
    float y;
} pb_point;

typedef struct _pb_rect {
    pb_point top_left;
    float w;
    float h;
} pb_rect;

typedef struct _pb_shape {
    pb_point *points;
    size_t num_points;
} pb_shape;

/**
 * Represents a room in a building.
 */
typedef struct _pb_room {
    char *name;
    size_t name_len;
    
    /* Rooms with higher priority will get more floor space,
     * and rooms with the same level of priority will get about
     * the same amount of floor space. */
    size_t priority;
    
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
    pb_rect floor_rect;
} pb_floor;

typedef struct _pb_building {
    pb_floor *floors;
    size_t num_floors;
} pb_building;

#ifdef __cplusplus
}
#endif

#endif /* PB_BUILDING_H */