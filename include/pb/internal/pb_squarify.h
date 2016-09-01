#ifndef PB_SQUARIFY_H
#define PB_SQUARIFY_H

#include <pb/pb_geom.h>

/**
 * @brief Determines the worst aspect ratio for an outer rectangle if attempting to fit the inner rectangles inside it.
 *
 * @param sum       The sum of the rectangles' areas.
 * @param min_dim   The minimum dimension (w or h).
 * @param rects     The list of rectangles for which to find the worst arrangement.
 * @psram num_rects The number of rectangles in the list.
 */
float worst(float sum, float min_dim, pb_shape *rects, size_t num_rects);

/**
 * Given a containing rectangle and a list of rectangles to be laid out inside it, attempts
 * to lay out the rectangles with their aspect ratios close to 1.
 *
 * @param rect         The outer rectangle into which children will be laid out.
 * @param min_dim      The size of the smallest dimension of the current layout rectangle.
 * @param is_height    Non-zero if min_dim is the rectangle's height.
 * @param areas        A list of areas to be laid out as rectangles. There must be at least 2 areas.
 * @param num_areas    The number of areas to be laid out.
 * @param children     A list of rectangles to store the resulting layout.
 * @param layout_size  The size of the list of rectangles in the current layout. This should always start at 0.
 * @param prev_sum     The sum of all the areas in the current layout. Should always start at 0.
 */
void pb_squarify(pb_rect *rect,
                 float min_dim,
                 int is_height,
                 float* areas,
                 size_t num_areas,
                 pb_rect *children,
                 size_t layout_size,
                 float prev_sum);


#endif /* PB_SQUARIFY_H */