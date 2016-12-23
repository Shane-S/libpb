#include <pb/util/geom/types.h>
#include <pb/util/geom/rect_utils.h>
#include <stdlib.h>
#include <math.h>
#include <pb/util/geom/shape_utils.h>

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
    
    out->points.size = 4;
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

PB_UTIL_DECLSPEC int pb_shape2D_get_bounding_rect(pb_shape2D const* shape, pb_rect* out) {
    
    float max_x = -INFINITY, min_x = INFINITY, max_y = -INFINITY, min_y = INFINITY;
    pb_point2D const* points = (pb_point2D*)shape->points.items;
    size_t i;

    if (shape->points.size < 3) {
        return -1;
    }

    for (i = 0; i < shape->points.size; ++i) {
        if (points[i].x > max_x) {
            max_x = points[i].x;
        }
        
        if (points[i].x < min_x) {
            min_x = points[i].x;
        }

        if (points[i].y > max_y) {
            max_y = points[i].y;
        } 
        
        if (points[i].y < min_y) {
            min_y = points[i].y;
        }
    }

    out->bottom_left.x = min_x;
    out->bottom_left.y = min_y;
    out->w = max_x - min_x;
    out->h = max_y - min_y;
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_rect_contains_point(pb_rect const* rect, pb_point2D const* point) {
    return point->x >= rect->bottom_left.x && point->x <= (rect->bottom_left.x + rect->w) &&
           point->y >= rect->bottom_left.y && point->y <= (rect->bottom_left.y + rect->h);
}
