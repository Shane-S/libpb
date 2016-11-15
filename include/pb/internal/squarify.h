#ifndef PB_SQUARIFY_H
#define PB_SQUARIFY_H

#include <pb/util/geom/types.h>

/**
 * @brief Determines the worst aspect ratio for an outer rectangle if attempting to fit the inner rectangles inside it.
 *
 * @param sum       The sum of the rectangles' areas.
 * @param min_dim   The minimum dimension (w or h).
 * @param rects     The list of rectangles for which to find the worst arrangement.
 * @psram num_rects The number of rectangles in the list.
 */
float worst(float sum, float min_dim, float* areas, size_t num_rects);

/**
 * Given a containing rectangle and a list of rectangles to be laid out inside it, attempts
 * to lay out the rectangles with their aspect ratios close to 1.
 *
 * @param rect               The outer rectangle into which children will be laid out.
 * @param areas              A list of areas to be laid out as rectangles. There must be at least 2 areas.
 * @param num_areas          The number of areas to be laid out.
 * @param children           A list of rectangles to store the resulting layout.
 * @param last_row_start     Holds the pointer to the first rectangle in the last row to be laid out (i.e., that contains rectangles).
 * @param last_row_size      Holds the number of rectangles in the last row to be laid out.
 * @param rect_hash_children Boolean indicating whether the current parent rectangle contained children upon return. If so, it contains
 *                           the rectangles from last_row_start to last_row_start + last_row_size.
 */
void pb_squarify(pb_rect* rect, float* areas, size_t num_areas, pb_rect* children, pb_rect** last_row_start, size_t* last_row_size, int* rect_has_children);


#endif /* PB_SQUARIFY_H */
