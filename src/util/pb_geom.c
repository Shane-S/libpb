#include <pb/util/pb_geom.h>
#include <stdlib.h>

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_to_pb_shape(pb_rect* rect, pb_shape* out) {
    pb_point* points = malloc(sizeof(pb_point) * 4);
    if (!points) {
        return 0;
    }
   
    /* Top left point */
    points[0].x = rect->bottom_left.x;
    points[0].y = rect->bottom_left.y + rect->h;

    /* Bottom left point*/
    points[1] = rect->bottom_left;

    /* Bottom right point */
    points[2].x = rect->bottom_left.x + rect->w;
    points[2].y = rect->bottom_left.y;

    /* Top right point */
    points[3].x = rect->bottom_left.x + rect->w;
    points[3].y = rect->bottom_left.y + rect->h;

    out->num_points = 4;
    out->points = points;
    out->connected = 1;

    return 1;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_shape_to_pb_rect(pb_shape* shape, pb_rect* out) {
    if (shape->num_points != 4) {
        return 0;
    }

    out->bottom_left = shape->points[1];
    out->h = shape->points[1].y - out->bottom_left.y;
    out->w = shape->points[2].x - out->bottom_left.x;

    return 1;
}