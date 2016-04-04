#include <math.h>
#include <pb/pb_types.h>
#include <pb/internal/pb_squarify.h>

/*
 *outer = [15, 20]

[3, 5]
[2, 10]
[4, 11]

[6, 2]


minimum dimension^2 * area / sum^2, sum^2 / (minimum dimension ^ 2 * area)
minimum dimension^2 * (width * height) /

sum0 = (3 * 5) + (2 * 10) + (4 * 11) = 79
min ^ 2 = 225


Edge cases (should ensure that they can't happen before passing to function)
*   Too many rectangles/outer rect isn't big enough to contain
*   Given list of rectangles don't fill up the space (this one will probably actually come up)
*   Single rectangle

Pre-conditions for the function (may deal with these later, but for now, that's just how the function has to be called)
*   Areas of all rectangles to be laid out must be about == to layout rect area (within floating point error ranges)
*   List of rectangles to lay out must contain >= 2 rectangles
 */

/**
 * Determines the worst combination of stuff.
 *
 */
float worst(float sum, float min_dim, pb_rect *rects, size_t num_rects) {
    float sum_sq = sum * sum;
    float min_dim_sq = min_dim * min_dim;

    float min_area = rects[0].w * rects[0].h;
    float max_area = rects[0].w * rects[0].h;

    size_t i;
    
    /* Determine the rectangles with the largest and smallest areas */
    for(i = 1; i < num_rects; ++i) {
        float area = rects[i].w * rects[i].h;
        if(area < min_area) {
            min_area = area;
        } else if(area > max_area) {
            max_area = area;
        }
    }

    return fmaxf(min_dim_sq * max_area / sum_sq, sum_sq / (min_dim_sq * min_area));
}

void pb_squarify(pb_rect *rect,
                 float min_dim,
                 int is_height,
                 pb_rect *children, size_t num_children,
                 size_t last_in_row,
                 float prev_sum) {

    pb_rect *child;
    float child_area;

    /* Laid out all children without having one that compromised (TODO: could this actually happen given our pre-conditions?)*/ 
    if(last_in_row + 1 == num_children) return;

    /* Current child to be laid out is the last one to have been added to the row */
    child = children + last_in_row;
    child_area = child->w * child->h;

    /* Determine whether adding the child to the current row would improve the row's aspect ratios */
    if(worst(prev_sum, min_dim, children, last_in_row + 1) <= worst(prev_sum + child_area, min_dim, children, last_in_row + 2)) {
        last_in_row++;
        prev_sum += child_area;     
        pb_squarify(rect, min_dim, is_height, children, num_children, last_in_row, prev_sum);
    } else {
        /* If not, finalise the current row, resize the rectangle into which children are laid out, and continue squarifying */
        if(is_height) {
            rect->w -= children[0].w;
            rect->top_left.x += children[0].w;
        } else {
            rect->h -= children[0].h;
            rect->top_left.y += children[0].h;
        }
        
        /* Only go until we have no more children to lay out */
        num_children -= last_in_row + 1;
        if(num_children == 0) return;

        /* Move to the next set of children */
        children += last_in_row + 1;
        last_in_row = 0;
        prev_sum = 0;

        is_height = rect->h < rect->w;
        pb_squarify(rect, is_height ? rect->h : rect->w, is_height, children, num_children, last_in_row, prev_sum);
    }
}
