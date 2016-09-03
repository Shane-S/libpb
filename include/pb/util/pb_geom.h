#pragma once
#include <stddef.h>
#include <pb/util/pb_util_exports.h>

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

    // When num_points > 2, determines whether the last vertex connects to the first
    int connected;
} pb_shape;

/**
 * A vertex in 3D geometry.
 */
typedef struct _pb_geom_vertex {
    // The vertex's position in model space
    float x;
    float y;
    float z;

    // The normal vector
    float nx;
    float ny;
    float nz;

    // The u and v coordinates
    float u;
    float v;
} pb_geom_vertex;

/**
 * Converts a pb_rect to a pb_shape.
 * Note that this function will assign a new array to shape->points, so either free any existing
 * points array or save it to avoid memory leaks.
 * 
 * @param rect  The rectangle to convert.
 * @param shape The rectangle represented as a pb_shape.
 * @return 0 on failure (out of memory), 1 on success.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_to_pb_shape(pb_rect* rect, pb_shape* out);

/**
 * Converts a pb_shape to a pb_rect.
 *
 * @param shape The shape to convert. The coordinates are expected to be in the form
 *              top-left, bottom-left, bottom-right, top-right.
 * @param out   The rectangle represented by the shape (if any).
 *
 * @return If the shape doesn't have exactly 4 points, returns 0. Otherwise, returns 1.
 *         Note that the function only checks that the shape has 4 points, NOT that it
 *         is actually a rectangle.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_shape_to_pb_rect(pb_shape* shape, pb_rect* out);