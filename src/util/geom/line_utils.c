#include <pb/util/geom/line_utils.h>

pb_point2D pb_line2D_get_t(pb_line2D const* line, pb_point2D const* point) {
    pb_point2D t;
    float xdiff = line->end.x - line->start.x;
    float ydiff = line->end.y - line->start.y;

    /* Most implementations probably do this already, so we should consider just doing the division normally. */
    if (xdiff == 0.f) {
        t.x = INFINITY;
    } else {
        t.x = (point->x - line->start.x) / xdiff;
    }


    if (ydiff == 0.f) {
        t.y = INFINITY;
    } else {
        t.y = (point->y - line->start.y) / ydiff;
    }

    return t;
}

pb_point2D pb_line2D_get_normal(pb_line2D const* line) {
    pb_point2D normal = {line->end.x - line->start.x,
                         line->end.y - line->start.y};

    float x = -normal.y;
    float y = normal.x;
    normal.x = x;
    normal.y = y;

    float len = sqrtf((normal.x * normal.x) + (normal.y * normal.y));
    normal.x /= len;
    normal.y /= len;

    return normal;
}