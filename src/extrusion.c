#include <pb/extrusion.h>
#include <stdlib.h>
#include <math.h>
#include <pb/floor_plan.h>
#include <pb/util/geom/types.h>
#include <pb/util/geom/shape_utils.h>
#include <pb/util/geom/line_utils.h>

/* Assumes that the points are actually on the line, and thus that the t values
 * for x and y are interchangeable (unless one of them is INFINITY). */
static pb_line2D const* cmp_line = NULL;
int pb_line2D_sort(void const* p1, void const* p2) {
    pb_point2D p1_t = pb_line2D_get_t(cmp_line, p1);
    pb_point2D p2_t = pb_line2D_get_t(cmp_line, p2);

    if (p1_t.x != INFINITY) {
        return p1_t.x < p2_t.x ? -1 : 1;
    } else {
        return p1_t.y < p2_t.y ? -1 : 1;
    }
}

/**
 * Extrudes a wall and stores it in the provided shape parameter (which must be
 * allocated to hold two triangles).
 *
 * @param parent_wall  The wall of which this wall is a subsection.
 * @param wall         The wall to extrude.
 * @param start_height The wall's starting height.
 * @param height       The wall's height.
 * @param normal       The normal vector for the 2D wall.
 * @param dest         A shape that will hold the extruded wall.
 */
static void extrude_wall_internal(pb_line2D const* parent_wall, pb_line2D const* wall,
                                  pb_point2D const* bottom_floor_centre,
                                  float start_height, float height,
                                  pb_point2D const* normal,
                                  pb_shape3D* dest) {

    pb_point2D parent_wall_vec = {parent_wall->end.x - parent_wall->start.x,
                                  parent_wall->end.y - parent_wall->start.y};

    pb_point2D wall_vec = {wall->end.x - wall->start.x,
                           wall->end.y - wall->start.y};
    pb_point2D wall_centre = {wall->start.x + wall_vec.x / 2.f,
                              wall->start.y + wall_vec.y / 2.f};

    pb_point2D wall_len = {fabsf(wall_vec.x), fabsf(wall_vec.y)};

    pb_point2D parent_wall_end_to_start = {parent_wall->start.x - parent_wall->end.x,
                                           parent_wall->start.y - parent_wall->end.y};
    pb_point2D parent_wall_start_to_end;

    /* 2D->3D: positive x is still positive x, but positive y is negative z (for now) */
    parent_wall_end_to_start.x = parent_wall_end_to_start.x < 0.f ? -1.f : (parent_wall_end_to_start.x > 0.f ? 1.f : 0.f);
    parent_wall_end_to_start.y = parent_wall_end_to_start.y < 0.f ? 1.f : (parent_wall_end_to_start.y > 0.f ? -1.f : 0.f);

    parent_wall_start_to_end.x = parent_wall_end_to_start.x * -1.f;
    parent_wall_start_to_end.y = parent_wall_end_to_start.y * -1.f;

    dest->pos.x = wall_centre.x - bottom_floor_centre->x;
    dest->pos.y = start_height + (height / 2.f);
    dest->pos.z = (wall_centre.y - bottom_floor_centre->y) * -1.f;

    pb_vert3D* tris = dest->tris;
    tris[0].x = wall_len.x / 2.f * parent_wall_end_to_start.x;
    tris[0].y = height / 2.f * -1.f;
    tris[0].z = wall_len.y / 2.f * parent_wall_end_to_start.y;
    tris[0].nx = normal->x;
    tris[0].ny = 0.f;
    tris[0].nz = -normal->y;
    tris[0].v = 1.f;

    pb_point2D s = {wall_centre.x + tris[0].x, wall_centre.y - tris[0].z};
    pb_point2D start_t = pb_line2D_get_t(wall, &s);
    float start_u = start_t.x == INFINITY ? start_t.y : start_t.x;
    tris[0].u = start_u;

    tris[1].x = wall_len.x / 2.f * parent_wall_start_to_end.x;
    tris[1].y = height / 2.f;
    tris[1].z = wall_len.y / 2.f * parent_wall_start_to_end.y;
    tris[1].nx = normal->x;
    tris[1].ny = 0.f;
    tris[1].nz = -normal->y;
    tris[1].v = 0.f;

    pb_point2D e = {wall_centre.x + tris[1].x, wall_centre.y - tris[1].z};
    pb_point2D end_t = pb_line2D_get_t(wall, &e);
    float end_u = end_t.x == INFINITY ? end_t.y : end_t.x;
    tris[1].u = end_u;

    tris[2].x = wall_len.x / 2.f * parent_wall_end_to_start.x;
    tris[2].y = height / 2.f;
    tris[2].z = wall_len.y / 2.f * parent_wall_end_to_start.y;
    tris[2].nx = normal->x;
    tris[2].ny = 0.f;
    tris[2].nz = normal->y * -1.f;
    tris[2].u = start_u;
    tris[2].v = 0.f;

    tris[3] = tris[0];

    tris[4].x = wall_len.x / 2.f * parent_wall_start_to_end.x;
    tris[4].y = height / 2.f * -1.f;
    tris[4].z = wall_len.y / 2.f * parent_wall_start_to_end.y;
    tris[4].nx = normal->x;
    tris[4].ny = 0.f;
    tris[4].nz = normal->y * -1.f;
    tris[4].u = end_u;
    tris[4].v = 1.f;

    tris[5] = tris[1];
}

PB_DECLSPEC int PB_CALL pb_extrude_wall(pb_line2D const* wall,
                                        pb_point2D const* doors, size_t num_doors,
                                        pb_point2D const* windows, size_t num_windows,
                                        pb_point2D const* bottom_floor_centre,
                                        pb_point2D const* normal,
                                        float start_height,
                                        float floor_height,
                                        float door_height,
                                        float window_height,
                                        pb_wall_structure_extruder const* door_extruder,
                                        pb_wall_structure_extruder const* window_extruder,
                                        void* extruder_param,
                                        void* window_extruder_param,
                                        pb_shape3D** walls_out, size_t* num_walls_out,
                                        pb_shape3D** doors_out, size_t* num_doors_out,
                                        pb_shape3D** windows_out, size_t* num_windows_out) {

    /* The easy case */
    if (num_doors == 0 && num_windows == 0) {
        pb_shape3D* wall_out = pb_shape3D_create(2);
        if (!wall) {
            return -1;
        }

        extrude_wall_internal(wall, wall, bottom_floor_centre, start_height, floor_height, normal, wall_out);
        *walls_out = wall_out;
        *num_walls_out = 1;

        *doors_out = NULL;
        *num_doors_out = 0;

        *windows_out = NULL;
        *num_windows_out = 0;
        return 0;
    } else if (num_doors != 0 && num_windows == 0) {

    } else if (num_doors == 0 && num_windows != 0) {

    } else {
        /* Sort the walls and doors based on how far they are along the parent wall */
        cmp_line = wall;
    }

    return -1;
}

PB_DECLSPEC pb_extruded_room* PB_CALL pb_extrude_room(pb_room const* room,
                                                       pb_point2D const* bottom_floor_centre,
                                                       float start_height,
                                                       float floor_height,
                                                       float door_height,
                                                       float window_height,
                                                       pb_wall_structure_extruder const* door_extruder,
                                                       pb_wall_structure_extruder const* window_extruder,
                                                       void* door_extruder_param,
                                                       void* window_extruder_param) {
    /* Allocate a pb_extruded_room* struct */
    /* If it fails, return NULL */

    /* If there are doors, sort them by wall number */
    /* If there are windows, sort them by wall number */
    /* For each wall in the room */
    /* If the walls array says there's actually a wall there */
    /* Determine the start and end door and window for that wall */
    /* Get the normal for that wall */
    /* Count the resulting number of walls, doors, and windows */
    /* Call pb_extrude_wall with the appropriate params */

    /* Once that's done, and assuming it didn't fail */
    /* If there's a floor, allocate a shape with num_tris = get_num_tris */
    /* Triangulate it using ear clipping */
    /* Copy the vertices into the correct positions */
    /* Get the shape's bounding rectangle */
    /* Loop over the shape's points and get the centre point */
    /* For each vertex in the new shape */
    /* Calculate its relation to the bottom-right corner of the rectangle and assign that as the UV */
    /* Assign (0, 1, 0) as the normal */
    /* Assign x = x - centre.x */
    /* Assign y = centre.y - y */

    /* If there's a ceiling */
    /* Allocate a shape with the same number of tris */
    /* Copy the whole set of triangles */
    /* For each triangle, switch the first and last vertices */
    /* TODO: Figure out if UVs also need to be adjusted when doing this */
    return NULL;
}

PB_DECLSPEC pb_extruded_floor* PB_CALL pb_extrude_floor(pb_floor const* f,
                                                         pb_point2D const* bottom_floor_centre,
                                                         float start_height,
                                                         float floor_height,
                                                         float door_height,
                                                         float window_height,
                                                         pb_wall_structure_extruder const* door_extruder,
                                                         pb_wall_structure_extruder const* window_extruder,
                                                         void* door_extruder_param,
                                                         void* window_extruder_param) {
    return NULL;
}

PB_DECLSPEC pb_extruded_floor** PB_CALL pb_extrude_building(pb_building* building,
                                                            float floor_height,
                                                            float door_height,
                                                            float window_height,
                                                            pb_wall_structure_extruder const* door_extruder,
                                                            pb_wall_structure_extruder const* window_extruder,
                                                            void* door_extruder_param,
                                                            void* window_extruder_param) {

    pb_extruded_floor** result = malloc(sizeof(pb_extruded_floor*) * building->num_floors);
    if (!result) {
        return NULL;
    }

    pb_point2D bottom_centre = {0.f, 0.f};
    pb_point2D const* bottom_floor_points = (pb_point2D const*)building->floors[0].shape.points.items;
    size_t i;
    for (i = 0; i < building->floors[0].shape.points.size; ++i) {
        bottom_centre.x += bottom_floor_points[i].x;
        bottom_centre.y += bottom_floor_points[i].y;
    }

    for (i = 0; i < building->num_floors; ++i) {
        float cur_height = i * floor_height; // Could actually allow people to specify this for things like basements
        result[i] = pb_extrude_floor(building->floors + i, &bottom_centre,
                                     cur_height, floor_height, door_height, window_height,
                                     door_extruder, window_extruder, door_extruder_param, window_extruder_param);
        if (result[i] == NULL) {
            break;
        }
    }

    if (i == building->num_floors) {
        return result;
    } else {
        size_t j;
        for (j = 0; j < i; ++j) {
            pb_extruded_floor_free(result[i]);
            free(result[i]);
        }
        free(result);
        return NULL;
    }
}

PB_DECLSPEC void pb_extruded_room_free(pb_extruded_room* r) {
    size_t i;
    for (i = 0; i < r->num_walls; ++i) {
        pb_shape3D_free(r->walls[i]);
    }
    free(r->walls);

    for (i = 0; i < r->num_doors; ++i) {
        pb_shape3D_free(r->doors + i);
    }
    free(r->doors);

    for (i = 0; i < r->num_windows; ++i) {
        pb_shape3D_free(r->windows + i);
    }
    free(r->windows);
}

PB_DECLSPEC void pb_extruded_floor_free(pb_extruded_floor* f) {
    size_t i;
    for (i = 0; i < f->num_rooms; ++i) {
        pb_extruded_room_free(f->rooms[i]);
        free(f->rooms[i]);
    }
    free(f->rooms);

    for (i = 0; i < f->num_doors; ++i) {
        pb_shape3D_free(f->doors + i);
    }
    free(f->doors);

    for (i = 0; i < f->num_windows; ++i) {
        pb_shape3D_free(f->windows + i);
    }
    free(f->windows);
}


