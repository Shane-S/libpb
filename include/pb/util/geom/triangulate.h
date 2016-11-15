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
