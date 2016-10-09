#include <pb/util/pb_geom.h>
#include <stdlib.h>

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_to_pb_shape(pb_rect* rect, pb_shape* out) {
    pb_point* points;
    char* connected;
    if (pb_shape_init(out, 4) == -1) {
        return 0;
    }
   
    points = (pb_point*)out->points.items;
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

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_shape_to_pb_rect(pb_shape* shape, pb_rect* out) {
    pb_point* points;
    if (shape->points.size != 4) {
        return 0;
    }

    points = (pb_point*)shape->points.items;

    out->bottom_left = points[1];
    out->h = points[0].y - out->bottom_left.y;
    out->w = points[2].x - out->bottom_left.x;

    return 1;
}

PB_UTIL_DECLSPEC int pb_shape_init(pb_shape* shape, unsigned int num_points) {
    
    /* Initialise the vectors */
    if (pb_vector_init(&shape->points, sizeof(pb_point), num_points) == -1) {
        return -1;
    } else if (pb_vector_init(&shape->connected, sizeof(char), num_points) == -1) {
        pb_vector_free(&shape->points);
        return -1;
    }
    
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
    pb_vector_free(&shape->points);
    pb_vector_free(&shape->connected);
}