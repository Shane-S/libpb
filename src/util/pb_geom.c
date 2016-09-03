#include <pb/util/pb_geom.h>
#include <stdlib.h>

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_to_pb_shape(pb_rect* rect, pb_shape* out) {
    pb_point* points = malloc(sizeof(pb_point) * 4);
    if (!points) {
        return 0;
    }

    points[0] = rect->top_left;
    
    points[1].x = rect->top_left.x;
    points[1].y = rect->top_left.y - rect->h;

    points[2].x = rect->top_left.x + rect->w;
    points[2].y = points[1].y;

    points[3].x = points[2].x;
    points[3].y = points[0].y;

    out->num_points = 4;
    out->points = points;
    out->connected = 1;

    return 1;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_shape_to_pb_rect(pb_shape* shape, pb_rect* out) {
    if (shape->num_points != 4) {
        return 0;
    }

    out->top_left = shape->points[0];
    out->h = out->top_left.y - shape->points[1].y;
    out->w = shape->points[2].x - out->top_left.x;

    return 1;
}