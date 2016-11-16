#pragma once
#include <pb/util/util_exports.h>
#include <pb/util/geom/types.h>
#include <stddef.h>

/**
 * Returns the number of triangles that the triangulation of
 * shape will contain.
 *
 * @param shape Pointer to the shape for which retrieve the number of triangles.
 *              This must be a simple polygon without holes.
 *
 * @return The number of triangles in a triangulation of shape.
 */
PB_UTIL_DECLSPEC size_t PB_UTIL_CALL pb_shape2D_get_num_tris(pb_shape2D const* shape);

/**
 * Triangulates a simple polygon without holes. Note that the "connected" array in
 * shape is ignored.
 *
 * @param shape The shape to be triangulated.
 * @return A set of triangles (indices into shape.points.items, CCW) on success,
 *         NULL on failure (OOM). Use pb_shape2D_get_num_tris to determine how
 *         many triangles are in the returned array.
 */
PB_UTIL_DECLSPEC size_t* PB_UTIL_CALL pb_triangulate(pb_shape2D const* shape);

/**
 * Tests whether the point p is contained in the triangle defined by t0, t1 and t2.
 *
 * Code adapted from StackOverflow since I don't remember barycentric coordinates
 * and am in a rush
 * http://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle#answer-14382692
 *
 * @param t0 The first point of the triangle.
 * @param t1 The second point of the triangle.
 * @param t2 The third point of the triangle.
 * @param p  The point to check.
 *
 * @return non-zero if the point is inside the triangle (including on one of the edges),
 *         0 if the point lies completely outside the triangle.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_tri_contains(pb_point2D* const t0, pb_point2D* const t1, pb_point2D* const t2, pb_point2D* const p);
