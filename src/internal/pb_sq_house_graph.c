#include <pb/internal/pb_sq_house_graph.h>
#include <pb/util/pb_float_utils.h>
#include <math.h>

int pb_sq_house_get_shared_wall(pb_room* room1, pb_room* room2) {
    int shares_top = 0;
    int shares_bottom = 0;
    int shares_left = 0;
    int shares_right = 0;

    /* We need to use approximate float comparisons because the stairs may not have exactly
     * the same coordinates as the rooms on each floor. */
    shares_top = pb_float_approx_eq(room1->room_shape.points[0].y, room2->room_shape.points[1].y, 5) &&
        room1->room_shape.points[0].x < room2->room_shape.points[2].x &&
        room1->room_shape.points[3].x > room2->room_shape.points[1].x;

    shares_bottom = pb_float_approx_eq(room1->room_shape.points[1].y, room2->room_shape.points[0].y, 5) &&
        room1->room_shape.points[1].x < room2->room_shape.points[3].x &&
        room1->room_shape.points[2].x > room2->room_shape.points[0].x;

    shares_right = pb_float_approx_eq(room1->room_shape.points[3].x, room2->room_shape.points[0].x, 5) &&
        room1->room_shape.points[2].y < room2->room_shape.points[0].y &&
        room1->room_shape.points[3].y > room2->room_shape.points[1].y;

    shares_left = pb_float_approx_eq(room1->room_shape.points[0].x, room2->room_shape.points[3].x, 5) &&
        room1->room_shape.points[1].y < room2->room_shape.points[3].y &&
        room1->room_shape.points[0].y > room2->room_shape.points[2].y;

    if (shares_top) {
        return SQ_HOUSE_TOP;
    }
    else if (shares_right) {
        return SQ_HOUSE_RIGHT;
    }
    else if (shares_left) {
        return SQ_HOUSE_LEFT;
    }
    else if (shares_bottom) {
        return SQ_HOUSE_BOTTOM;
    }
    else {
        return -1;
    }
}

void pb_sq_house_get_wall_overlap(pb_room const* room1, pb_room const* room2, int wall, pb_point* start, pb_point* end) {

    switch (wall) {
    case SQ_HOUSE_TOP:
    case SQ_HOUSE_BOTTOM:
        start->x = fmaxf(room1->room_shape.points[0].x, room2->room_shape.points[0].x);
        end->x = fminf(room1->room_shape.points[3].x, room2->room_shape.points[3].x);

        /* Calculating overlap in the x axis is the same whether the bottom or the top
         * wall is shared, so use this hacky calculation to choose a point on the correct
         * wall from which to get the y coord. */
        start->y = room1->room_shape.points[wall].y;
        end->y = start->y;
        return;

    case SQ_HOUSE_RIGHT:
    case SQ_HOUSE_LEFT:
        start->y = fmaxf(room1->room_shape.points[2].y, room2->room_shape.points[2].y);
        end->y = fminf(room1->room_shape.points[3].y, room2->room_shape.points[3].y);

        /* Same thing here, just picks right or left point instead of top/bottom */
        start->x = room1->room_shape.points[wall].x;
        end->x = start->x;

        return;
    default:
        return;
    }
}