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
 * Determines the worst resulting aspect ratio of a set of rectangles laid out in a larger one.
 *
 * @param sum       The sum of all areas in the list.
 * @param min_dim   The value of the outer rectangle's minimum dimension.
 * @param rects     The rectangles containing the areas to be laid out. The area of a rectangle
 *                  must be set as its width.
 * @param num_rects The number of rectangles in the list.
 */
float worst(float sum, float min_dim, pb_rect *rects, size_t num_rects) {
    float sum_sq = sum * sum;
    float min_dim_sq = min_dim * min_dim;

    float min_area = rects[0].w;
    float max_area = rects[0].w;
    size_t i;
    
    /* Determine the rectangles with the largest and smallest areas */
    for(i = 1; i < num_rects; ++i) {
        if(rects[i].w < min_area) {
			min_area = rects[i].w;
        } else if(rects[i].w > max_area) {
			max_area = rects[i].w;
        }
    }

    return fmaxf(min_dim_sq * max_area / sum_sq, sum_sq / (min_dim_sq * min_area));
}

/* Lays out the rectangles in the current row/column */
void layout(pb_rect *rect,
            float prev_sum,
            float min_dim,
            int is_height,
            pb_rect *children,
            size_t num_to_layout) {

    float dim = prev_sum / min_dim; /* Length of the side that will be the same for all rectangles */
    float pos_x = rect->top_left.x;
    float pos_y = rect->top_left.y;
    size_t i;
	for (i = 0; i < num_to_layout; ++i) {
        /* Length of the other dimension to make make correct area */ 
        float other_dim = children[i].w / dim;
        if(is_height) {
            children[i].w = dim;
            children[i].h = other_dim;
            children[i].top_left.x = pos_x;
            children[i].top_left.y = pos_y;
            pos_y += other_dim;
        } else {
            children[i].w = other_dim;
            children[i].h = dim;
            children[i].top_left.x = pos_x;
            children[i].top_left.y = pos_y;
            pos_x += other_dim;
        }
    }
}

/**
 * Given a containing rectangle and a list of rectangles to be laid out inside it, attempts
 * to lay out the rectangles while minimising their aspect ratios.
 *
 * @param rect         The outer rectangle into which children will be laid out.
 * @param min_dim      The value of the smallest dimension of the current layout rectangle.
 * @param is_height    Non-zero if min_dim is the rectangle's height.
 * @param children     A list of rectangles to be laid out. The rectangles actually represent a list
 *                     of areas to be laid out; their width MUST be set to the desired area for the
 *                     algorithm to work.
 * @param num_children The number of children in the given list.
 * @param layout_size  The size of the list of rectangles in the current layout. This should always start at 0.
 * @param prev_sum     The sum of all the areas in the current layout. Should always start at 0.
 */
void pb_squarify(pb_rect *rect,
                 float min_dim,
                 int is_height,
                 pb_rect *children, size_t num_children,
                 size_t layout_size,
                 float prev_sum) {

	/* Current child to be laid out is the last one to have been added to the row */
	pb_rect *child;
	float child_area;

    /* Added all children without messing up aspect ratio */ 
	if (layout_size == num_children) {
		layout(rect, prev_sum, min_dim, is_height, children, layout_size);
		return;
	}

	child = children + layout_size;;
	child_area = child->w;

    /* Determine whether adding the child to the current row would worsen the row's aspect ratios */
	if (layout_size == 0 ||
		worst(prev_sum, min_dim, children, layout_size) >= worst(prev_sum + child_area, min_dim, children, layout_size + 1)) {
        
		layout_size++;
        prev_sum += child_area;     
		pb_squarify(rect, min_dim, is_height, children, num_children, layout_size, prev_sum);
    } else {
		layout(rect, prev_sum, min_dim, is_height, children, layout_size);
        
        /* Move the layout rectangle to the appropriate spot */
        if(is_height) {
            rect->w -= children[0].w;
            rect->top_left.x += children[0].w;
        } else {
            rect->h -= children[0].h;
            rect->top_left.y += children[0].h;
        }
        
        /* Only go until we have no more children to lay out */
		num_children -= layout_size;
        if(num_children == 0) return;

        /* Move to the next set of children */
		children += layout_size;
		layout_size = 0;
        prev_sum = 0;

        is_height = rect->h < rect->w;
		pb_squarify(rect, is_height ? rect->h : rect->w, is_height, children, num_children, layout_size, prev_sum);
    }
}
