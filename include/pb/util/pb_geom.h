#pragma once
#include <stddef.h>
#include <pb/util/pb_util_exports.h>
#include <pb/util/pb_vector.h>

typedef struct {
    float x;
    float y;
} pb_point2D;

typedef struct {
    pb_vector points;
    pb_vector connected;
} pb_shape2D;

typedef struct {
    pb_point2D bottom_left;
    float w;
    float h;
} pb_rect;

typedef struct {
    float x;
    float y;
    float z;
} pb_point3D;

/**
 * A vertex in 3D geometry.
 * On most compilers, this should have no padding, so the buffer in pb_shape3D
 * will be tightly packed.
 */
typedef struct {
    /* The vertex's position in model space */
    float x;
    float y;
    float z;

    /* The normal vector */
    float nx;
    float ny;
    float nz;

    /* The u and v coordinates */
    float u;
    float v;
} pb_vert3D;

typedef struct {
    /* A buffer of triangles. Winding order is CCW. */
    pb_vert3D* tris;

    /* The number of triangles in this shape. As you'd expect, multiply by 3 to get the number of vertices. */
    size_t num_tris;
} pb_shape3D;

/**
 * Converts a pb_rect to a pb_shape2D.
 * Note that this function will assign a new array to shape->points, so either free any existing
 * points array or save it to avoid memory leaks.
 * 
 * @param rect  The rectangle to convert.
 * @param shape The rectangle represented as a pb_shape2D.
 * @return 0 on failure (out of memory), 1 on success.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_to_pb_shape2D(pb_rect* rect, pb_shape2D* out);

/**
 * Converts a pb_shape2D to a pb_rect.
 *
 * @param shape The shape to convert. The coordinates are expected to be in the form
 *              top-left, bottom-left, bottom-right, top-right.
 * @param out   The rectangle represented by the shape (if any).
 *
 * @return If the shape doesn't have exactly 4 points, returns 0. Otherwise, returns 1.
 *         Note that the function only checks that the shape has 4 points, NOT that it
 *         is actually a rectangle.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_shape2D_to_pb_rect(pb_shape2D* shape, pb_rect* out);

/**
 * Allocates a new shape with the given number of points.
 *
 * @param The number of points in the new shape.
 */
PB_UTIL_DECLSPEC pb_shape2D* pb_shape2D_create(unsigned int num_points);

/**
 * Initialises the given shape to have num_points.
 *
 * @param shape      The shape to initialise.
 * @param num_points The number of points that the shape should have.
 * @return 0 on success, -1 on out of memory.
 */
PB_UTIL_DECLSPEC int pb_shape2D_init(pb_shape2D* shape, unsigned int num_points);

/**
 * Frees the shape's points and connected arrays.
 * Don't call this if your arrays are allocated on the stack (or with new).
 *
 * @param shape The shape to free. Note that the shape itself won't be freed, just its internal arrays.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_shape2D_free(pb_shape2D* shape);