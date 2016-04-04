#ifndef PB_SQUARIFY_H
#define PB_SQUARIFY_H

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