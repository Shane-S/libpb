#pragma once
#include <pb/util/geom/types.h>

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * Allocates a new shape with the given number of points.
 *
 * @param num_tris The number of triangles in the new shape.
 */
PB_UTIL_DECLSPEC pb_shape3D* pb_shape3D_create(unsigned int num_tris);

/**
 * Initialises the given shape to have num_tris.
 *
 * @param shape    The shape to initialise.
 * @param num_tris The number of triangles that the shape should have.
 * @return 0 on success, -1 on out of memory.
 */
PB_UTIL_DECLSPEC int pb_shape3D_init(pb_shape3D* shape, unsigned int num_points);

/**
 * Frees the shape's triangles/
 * Don't call this if your arrays are allocated on the stack (or with new).
 *
 * @param shape The shape to free. Note that the shape itself won't be freed, just its internal arrays.
 */
PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_shape3D_free(pb_shape3D* shape);

#ifdef __cplusplus
}
#endif