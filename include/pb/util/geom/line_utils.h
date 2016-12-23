#pragma once
#include <pb/util/geom/types.h>
#include <math.h>

/**
 * Gets the t values for x and y for the given along the given line.
 *
 * @param line  The line along which to find the t value.
 * @param point The point for which to find the t value.
 * @param tx    The t value for x. If line->end.x == line->start.x, this will be Inf.
 * @param ty    The y value for y. If line->end.y == line->stary.y, this will be Inf.
 *
 * @return The t values for x and y along this line.
 */
pb_point2D pb_line2D_get_t(pb_line2D const* line, pb_point2D const* point);

/**
 * Gets the (normalised) normal vector for the given line.
 *
 * @param line The line for which to get the normal vector. Precondition: line->start != line->end.
 * @return The normalised normal vector for vector line->start to line->end.
 */
pb_point2D pb_line2D_get_normal(pb_line2D const* line);