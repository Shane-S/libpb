#pragma once
#include <stddef.h>
#include <pb/util/util_exports.h>
#include <pb/util/vector/vector.h>
#include <pb/util/geom/types.h>

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

