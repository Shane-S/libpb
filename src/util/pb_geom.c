#include <pb/util/pb_geom.h>
#include <stdlib.h>

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_to_pb_shape(pb_rect* rect, pb_shape* out) {
    if (pb_shape_init(out, 4) == -1) {
        return 0;
    }
   
    /* Top left point */
    out->points[0].x = rect->bottom_left.x;
    out->points[0].y = rect->bottom_left.y + rect->h;

    /* Bottom left point*/
    out->points[1] = rect->bottom_left;

    /* Bottom right point */
    out->points[2].x = rect->bottom_left.x + rect->w;
    out->points[2].y = rect->bottom_left.y;

    /* Top right point */
    out->points[3].x = rect->bottom_left.x + rect->w;
    out->points[3].y = rect->bottom_left.y + rect->h;

    out->connected = 1;

    return 1;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_shape_to_pb_rect(pb_shape* shape, pb_rect* out) {
    if (shape->num_points != 4) {
        return 0;
    }

    out->bottom_left = shape->points[1];
    out->h = shape->points[0].y - out->bottom_left.y;
    out->w = shape->points[2].x - out->bottom_left.x;

    return 1;
}

PB_UTIL_DECLSPEC int pb_shape_init(pb_shape* shape, unsigned int num_points) {
    
    pb_point* points = malloc(sizeof(pb_point) * num_points);
    if (!points) return -1;

    shape->points = points;
    shape->num_points = num_points;
    return 0;
}

PB_UTIL_DECLSPEC pb_shape* pb_shape_create(unsigned int num_points) {
    pb_shape* result = NULL;
    pb_point* points = NULL;

    result = malloc(sizeof(pb_shape));
    if (!result) return NULL;

    if (pb_shape_init(result, num_points) == -1) {
        free(result);
        return NULL;
    }

    return result;
}

PB_UTIL_DECLSPEC void pb_shape_free(pb_shape* shape) {
    free(shape->points);
}