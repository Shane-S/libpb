#ifndef PB_SQUARIFY_H
#define PB_SQUARIFY_H

/**
 * @brief Determines the worst aspect ratio for an outer rectangle if attempting to fit the inner rectangles inside it.
 *
 * @param sum       The sum of the rectangles' areas.
 * @param min_dim   The minimum dimension (w or h).
 * @param rects     The list of rectangles for which to find the worst arrangement.
 * @psram num_rects The number of rectangles in the list.
 */
float worst(float sum, float min_dim, pb_rect *rects, size_t num_rects);

/**
 * Places the list of child rectangles into the specified outer rectangle, attempting
 * to minimise their aspect ratios.
 *
 * Preconditions:
 *    - Areas of all rectangles to be laid out must be about == to rect area (within floating point error ranges)
 *    - List of rectangles to lay out must contain >= 2 rectangles
 *
 * @param rect         The rect into which the children are to be placed.
 * @param min_dim      The size of the smallest dimension of the outer rectangle.
 * @param is_height    Non-zero if the smallest dimension is the height; 0 otherwise.
 * @param children     The list of children to be placed in the outer rectangle.
 * @param num_children The number of children in the list.
 * @param last_in_row  The index of the last rectangle in the current row (obtained by children + last_in_row).
 * @param prev_sum     The sum of all of the previous rectangles' areas.
 */
void pb_squarify(pb_rect *rect,
                 float min_dim,
                 int is_height,
                 pb_rect *children, size_t num_children,
                 size_t last_in_row,
                 float prev_sum);


#endif /* PB_SQUARIFY_H */