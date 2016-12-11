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

/**
 * For a shape with 3 or more points, gets the bounding rectangle.
 *
 * @param shape The shape for which to retrieve the bounding rectangle.
 * @param out   The variable where the resulting rectangle will be stored, if there is one.
 *
 * @return 0 on success, -1 on failure (shape had < 3 points).
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_shape2D_get_bounding_rect(pb_shape2D const* shape, pb_rect* out);

/**
 * Determines whether the given rectangle contains the given point. Points on an edge are considered contained.
 * 
 * @param rect  The rectangle to use as a container.
 * @param point The point to check for containment
 *
 * @return Non-zero if the point is contained, 0 if not.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_contains_point(pb_rect const* rect, pb_point2D const point);

/**
 * Determines whether and where two rectangles overlap.
 *
 * @param rect  The first rectangle to check for overlap.
 * @param other The second rectangle to check for overlap.
 * @param start The start point of overlap, if any. If is_edge is 0, then this contains the first
 *              intersection point.
 * @param end   The end point of overlap, if any. If is_edge is 0, then this contains the second
 *              intersection point. This point may not be on the same side of rect as the start point.
 *
 * @return Non-zero if the rectangles overlap, 0 if not.
 */
PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_get_overlap(pb_rect const* rect, pb_rect const* other, pb_point2D* start, pb_point2D* end, int* is_edge);

