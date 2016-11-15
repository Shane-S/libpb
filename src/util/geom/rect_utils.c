#include <pb/util/geom/types.h>
#include <pb/util/geom/rect_utils.h>
#include <stdlib.h>

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_to_pb_shape2D(pb_rect* rect, pb_shape2D* out) {
    pb_point2D* points;
    char* connected;
    if (pb_shape2D_init(out, 4) == -1) {
        return 0;
    }
   
    points = (pb_point2D*)out->points.items;
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
    
    /* All walls are connected */
    connected = (char*)out->connected.items;
    connected[0] = 1;
    connected[1] = 1;
    connected[2] = 1;
    connected[3] = 1;

    out->points.size = 4;
    out->connected.size = 4;

    return 1;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_shape2D_to_pb_rect(pb_shape2D* shape, pb_rect* out) {
    pb_point2D* points;
    if (shape->points.size != 4) {
        return 0;
    }

    points = (pb_point2D*)shape->points.items;

    out->bottom_left = points[1];
    out->h = points[0].y - out->bottom_left.y;
    out->w = points[2].x - out->bottom_left.x;

    return 1;
}

PB_UTIL_DECLSPEC int pb_shape2D_init(pb_shape2D* shape, unsigned int num_points) {
    
    /* Initialise the vectors */
    if (pb_vector_init(&shape->points, sizeof(pb_point2D), num_points) == -1) {
        return -1;
    } else if (pb_vector_init(&shape->connected, sizeof(char), num_points) == -1) {
        pb_vector_free(&shape->points);
        return -1;
    }
    
    return 0;
}

PB_UTIL_DECLSPEC pb_shape2D* pb_shape2D_create(unsigned int num_points) {
    pb_shape2D* result = NULL;
    pb_shape2D* points = NULL;

    result = malloc(sizeof(pb_shape2D));
    if (!result) return NULL;

    if (pb_shape2D_init(result, num_points) == -1) {
        free(result);
        return NULL;
    }

    return result;
}

PB_UTIL_DECLSPEC void pb_shape2D_free(pb_shape2D* shape) {
    pb_vector_free(&shape->points);
    pb_vector_free(&shape->connected);
}