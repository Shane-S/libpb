#include <pb/extrusion.h>
#include <stdlib.h>
#include <math.h>
#include <pb/floor_plan.h>
#include <pb/util/geom/types.h>
#include <pb/util/geom/shape_utils.h>
#include <pb/util/geom/line_utils.h>
#include <string.h>

/* Assumes that the points are actually on the line, and thus that the t values
 * for x and y are interchangeable (unless one of them is INFINITY). */
static pb_line2D const* cmp_line = NULL;
int pb_line2D_sort(void const* struct1, void const* struct2) {
    pb_wall_structure* s1 = (pb_wall_structure*)struct1;
    pb_wall_structure* s2 = (pb_wall_structure*)struct2;

    pb_point2D p1_t = pb_line2D_get_t(cmp_line, &s1->start);
    pb_point2D p2_t = pb_line2D_get_t(cmp_line, &s2->start);

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
    pb_point2D start_t = pb_line2D_get_t(parent_wall, &s);
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
    pb_point2D end_t = pb_line2D_get_t(parent_wall, &e);
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
                                        pb_wall_structure const* doors, size_t num_doors,
                                        pb_wall_structure const* windows, size_t num_windows,
                                        pb_point2D const* bottom_floor_centre,
                                        pb_point2D const* normal,
                                        float start_height,
                                        float floor_height,
                                        float door_height,
                                        float window_height,
                                        pb_wall_structure_extruder const* door_extruder,
                                        pb_wall_structure_extruder const* window_extruder,
                                        void* door_extruder_param,
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
    }
    size_t window_list_size = 0;
    size_t door_list_size = 0;
    size_t wall_list_size = num_doors + num_windows + 1;

    size_t i;
    for (i = 0; i < num_doors; ++i) {
        pb_line2D door = {doors[i].start, doors[i].end};

        size_t cur_door_walls;
        size_t cur_door_shapes;

        door_extruder->count(wall, &door, normal, bottom_floor_centre,
                             floor_height, window_height, start_height,
                             door_extruder_param,
                             &cur_door_walls, &cur_door_shapes);

        door_list_size += cur_door_shapes;
        wall_list_size += cur_door_walls;
    }

    for (i = 0; i < num_windows; ++i)  {
        pb_line2D window = {windows[i].start, windows[i].end};
        size_t cur_window_walls;
        size_t cur_window_shapes;

        window_extruder->count(wall, &window, normal, bottom_floor_centre,
                               floor_height, window_height, start_height,
                               window_extruder_param,
                               &cur_window_walls, &cur_window_shapes);

        window_list_size += cur_window_shapes;
        wall_list_size += cur_window_walls;
    }

    pb_shape3D* wall_list = NULL;
    pb_shape3D* door_list = NULL;
    pb_shape3D* window_list = NULL;

    /* calloc so freeing later is easier if necessary */
    /* That probably doesn't justify the performance hit... but there's a lot of other stuff that's probably
     * way worse on the performance front anyway */
    wall_list = calloc(sizeof(pb_shape3D),  wall_list_size);
    door_list = door_list_size == 0 ? NULL : calloc(sizeof(pb_shape3D), door_list_size);
    window_list = window_list_size == 0 ? NULL : calloc(sizeof(pb_shape3D), window_list_size);

    if (!wall_list || (door_list_size != 0 && !door_list) || (window_list_size != 0 && !window_list)) {
        free(wall_list);
        free(door_list);
        free(window_list);
        return -1;
    }

    /* Sort the door and window lists according to how far they are along the wall.
     * sizeof(pb_point2D) * 2 because we want to skip the end points. */
    cmp_line = wall;
    if (num_doors) {
        qsort(doors, num_doors, sizeof(pb_wall_structure), pb_line2D_sort);
    }
    if (num_windows) {
        qsort(windows, num_windows, sizeof(pb_wall_structure), pb_line2D_sort);
    }

    int end_is_start = 0;
    if (num_doors) {
        pb_point2D door0_start_t = pb_line2D_get_t(wall, &doors[0].start);
        pb_point2D door0_end_t = pb_line2D_get_t(wall, &doors[0].end);

        end_is_start = door0_end_t.x != INFINITY ? door0_end_t.x < door0_start_t.x
                                                 : door0_end_t.y < door0_start_t.y;
    } else {
        pb_point2D window0_start_t = pb_line2D_get_t(wall, &windows[0].start);
        pb_point2D window0_end_t = pb_line2D_get_t(wall, &windows[0].end);

        end_is_start = window0_end_t.x != INFINITY ? window0_end_t.x < window0_start_t.x
                                                   : window0_end_t.y < window0_start_t.y;
    }

    pb_point2D start_to_end = {wall->end.x - wall->start.x, wall->end.y - wall->start.y};
    int x_is_cmp = start_to_end.x != 0.f;
    int cmp_is_less = x_is_cmp ? start_to_end.x > 0.f : start_to_end.y > 0.f; /* Are smaller x/y values closer to the start? */

    /* The current index into the input doors/windows lists */
    size_t cur_door = 0;
    size_t cur_window = 0;

    /* The current index into the doors/window shapes lists - doors/windows can produce multiple shapes, so these
     * may not be equal to cur_door */
    size_t cur_door_shape_count = 0;
    size_t cur_window_shape_count = 0;
    size_t cur_wall_count = 0;

    pb_point2D wall_start = wall->start;
    pb_point2D wall_end;

    while(cur_door < num_doors || cur_window < num_windows) {
        if (cur_door < num_doors && cur_window < num_windows) {
            int is_door;

            if (x_is_cmp) {
                if (end_is_start) {
                    is_door = cmp_is_less ? doors[cur_door].end.x < windows[cur_window].end.x
                                          : doors[cur_door].end.x > windows[cur_window].end.x;
                } else {
                    is_door = cmp_is_less ? doors[cur_door].start.x < windows[cur_window].start.x
                                          : doors[cur_door].start.x > windows[cur_window].start.x;
                }
            } else {
                if (end_is_start) {
                    is_door = cmp_is_less ? doors[cur_door].end.y < windows[cur_window].end.y
                                          : doors[cur_door].end.y > windows[cur_window].end.y;
                } else {
                    is_door = cmp_is_less ? doors[cur_door].start.y < windows[cur_window].start.y
                                          : doors[cur_door].start.y > windows[cur_window].start.y;
                }
            }

            pb_line2D structure;

            if (is_door) {
                structure.start = doors[cur_door].start;
                structure.end = doors[cur_door].end;
            } else {
                structure.start = windows[cur_window].start;
                structure.end = windows[cur_window].end;
            }
            /* Lots of copying by value here, but it's easier ¯\_(ツ)_/¯ */
            wall_end = end_is_start ? structure.end : structure.start;

            pb_shape3D* cur_wall = wall_list + cur_wall_count;
            if (pb_shape3D_init(cur_wall, 2) == -1) {
                goto err_return;
            }

            pb_line2D sub_wall = {wall_start, wall_end};
            extrude_wall_internal(wall, &sub_wall, bottom_floor_centre, start_height, floor_height, normal, cur_wall);
            cur_wall_count++;

            size_t structure_wall_count;
            size_t structure_shape_count;
            pb_shape3D* structure_walls;
            pb_shape3D* structure_shapes;

            /* I could actually make this even more general by selecting the extruder and shapes using conditional
             * operators, but I don't really feel like it right now */
            if (is_door) {
                door_extruder->count(wall, &structure, normal, bottom_floor_centre,
                                     floor_height, door_height, start_height,
                                     door_extruder_param,
                                     &structure_wall_count, &structure_shape_count);

                if (door_extruder->extrude(wall, &structure, normal, bottom_floor_centre,
                                           floor_height, door_height, start_height,
                                           door_extruder_param,
                                           &structure_walls, &structure_shapes) == -1) {
                    goto err_return;
                }

                memcpy(wall_list + cur_wall_count, structure_walls, sizeof(pb_shape3D) * structure_wall_count);
                memcpy(door_list + cur_door_shape_count, structure_shapes, sizeof(pb_shape3D) * structure_shape_count);

                cur_door_shape_count += structure_shape_count;
                cur_door++;
            } else {
                window_extruder->count(wall, &structure, normal, bottom_floor_centre,
                                       floor_height, window_height, start_height,
                                       window_extruder_param,
                                       &structure_wall_count, &structure_shape_count);

                if (window_extruder->extrude(wall, &structure, normal, bottom_floor_centre,
                                             floor_height, window_height, start_height,
                                             window_extruder_param,
                                             &structure_walls, &structure_shapes) == -1) {
                    goto err_return;
                }

                memcpy(wall_list + cur_wall_count, structure_walls, sizeof(pb_shape3D) * structure_wall_count);
                memcpy(window_list + cur_window_shape_count, structure_shapes, sizeof(pb_shape3D) * structure_shape_count);

                cur_window_shape_count += structure_shape_count;
                cur_window++;
            }

            free(structure_walls);
            free(structure_shapes);

            cur_wall_count += structure_wall_count;
            wall_start = end_is_start ? structure.start : structure.end;

        } else if (cur_door < num_doors) {
            wall_end = end_is_start ? doors[cur_door].end : doors[cur_door].start;

            pb_shape3D* cur_wall = wall_list + cur_wall_count;
            if (pb_shape3D_init(cur_wall, 2) == -1) {
                goto err_return;
            }

            pb_line2D sub_wall = {wall_start, wall_end};
            extrude_wall_internal(wall, &sub_wall, bottom_floor_centre, start_height, floor_height, normal, cur_wall);
            cur_wall_count++;

            size_t door_wall_count;
            size_t door_shape_count;
            pb_shape3D* door_walls;
            pb_shape3D* door_shapes;

            pb_line2D door_line = {doors[cur_door].start, doors[cur_door].end};

            door_extruder->count(wall, &door_line, normal, bottom_floor_centre,
                                 floor_height, window_height, start_height,
                                 door_extruder_param,
                                 &door_wall_count, &door_shape_count);

            if (door_extruder->extrude(wall, &door_line, normal, bottom_floor_centre,
                                       floor_height, door_height, start_height,
                                       door_extruder_param, &door_walls, &door_shapes) == -1) {
                goto err_return;
            }

            memcpy(wall_list + cur_wall_count, door_walls, sizeof(pb_shape3D) * door_wall_count);
            memcpy(door_list + cur_door_shape_count, door_shapes, sizeof(pb_shape3D) * door_shape_count);

            cur_door_shape_count += door_shape_count;
            cur_wall_count += door_wall_count;
            cur_door++;

            wall_start = end_is_start ? doors[cur_door - 1].start : doors[cur_door - 1].end;

        } else {
            wall_end = end_is_start ? windows[cur_window].end : windows[cur_window].start;

            pb_shape3D* cur_wall = wall_list + cur_wall_count;
            if (pb_shape3D_init(cur_wall, 2) == -1) {
                goto err_return;
            }

            pb_line2D sub_wall = {wall_start, wall_end};
            extrude_wall_internal(wall, &sub_wall, bottom_floor_centre, start_height, floor_height, normal, cur_wall);
            cur_wall_count++;

            size_t window_wall_count;
            size_t window_shape_count;
            pb_shape3D* window_walls;
            pb_shape3D* window_shapes;

            pb_line2D window_line = {windows[cur_window].start, windows[cur_window].end};

            window_extruder->count(wall, &window_line, normal, bottom_floor_centre,
                                   floor_height, window_height, start_height,
                                   window_extruder_param,
                                   &window_wall_count, &window_shape_count);

            if (window_extruder->extrude(wall, &window_line, normal, bottom_floor_centre,
                                         floor_height, window_height, start_height,
                                         window_extruder_param,
                                         &window_walls, &window_shapes) == -1) {
                goto err_return;
            }

            memcpy(wall_list + cur_wall_count, window_walls, sizeof(pb_shape3D) * window_wall_count);
            memcpy(window_list + cur_window_shape_count, window_shapes, sizeof(pb_shape3D) * window_shape_count);

            cur_window_shape_count += window_shape_count;
            cur_wall_count += window_wall_count;
            cur_window++;

            wall_start = end_is_start ? windows[cur_window - 1].start : windows[cur_window - 1].end;
        }
    }

    pb_line2D last_wall = {wall_start, wall->end};
    pb_shape3D* last_wall_shape = wall_list + cur_wall_count; /* cur_wall_count should at this point be walls_list_size - 1 */
    if (pb_shape3D_init(last_wall_shape, 2) == -1) {
        goto err_return;
    }
    extrude_wall_internal(wall, &last_wall, bottom_floor_centre, start_height, floor_height, normal, last_wall_shape);
    *walls_out = wall_list;
    *num_walls_out = wall_list_size;

    *doors_out = num_doors ? door_list : NULL;
    *num_doors_out = door_list_size;

    *windows_out = num_windows ? window_list : NULL;
    *num_windows_out = window_list_size;

    return 0;

err_return:
    for (i = 0; i < cur_wall_count; ++i) {
        pb_shape3D_free(wall_list + i);
    }
    free(wall_list);

    for (i = 0; i < cur_door_shape_count; ++i) {
        pb_shape3D_free(door_list + i);
    }
    free(door_list);

    for (i = 0; i < cur_window_shape_count; ++i) {
        pb_shape3D_free(window_list + i);
    }
    free(window_list);

    *walls_out = NULL;
    *num_walls_out = 0;

    *doors_out = NULL;
    *num_doors_out = 0;

    *windows_out = NULL;
    *num_windows_out = 0;

    return -1;
}

static int wall_structure_cmp(void const* s1, void const* s2) {
    pb_wall_structure* s1_struct = (pb_wall_structure*)s1;
    pb_wall_structure* s2_struct = (pb_wall_structure*)s2;

    return (int)s1_struct->wall - (int)s2_struct->wall;
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

    pb_extruded_room* out = NULL;
    pb_shape3D** walls_out = NULL;
    size_t* wall_counts = NULL;

    pb_vector doors_out;
    pb_vector windows_out;

    doors_out.items = NULL;
    doors_out.size = 0;

    windows_out.items = NULL;
    windows_out.size = 0;

    int doors_init_result = room->num_doors != 0 ? pb_vector_init(&doors_out, sizeof(pb_shape3D), room->num_doors) : 0;
    int windows_init_result = room->num_windows != 0 ? pb_vector_init(&windows_out, sizeof(pb_shape3D), room->num_windows) : 0;

    out = malloc(sizeof(pb_extruded_room));
    walls_out = calloc(sizeof(pb_shape3D*), room->walls.size);
    wall_counts = malloc(sizeof(size_t) * room->walls.size);

    if (!out || !walls_out || !wall_counts ||
            (room->num_doors != 0 && doors_init_result == -1) ||
            (room->num_windows != 0 && windows_init_result == -1)) {
        free(out);
        free(walls_out);
        free(wall_counts);
        pb_vector_free(&doors_out);
        pb_vector_free(&windows_out);
        return NULL;
    }

    if (room->num_doors != 0) {
        qsort(room->doors, room->num_doors, sizeof(pb_wall_structure), wall_structure_cmp);
    }

    if (room->num_windows != 0) {
        qsort(room->windows, room->num_windows, sizeof(pb_wall_structure), wall_structure_cmp);
    }

    int* walls = (int*)room->walls.items;
    pb_point2D* room_points = (pb_point2D*)room->shape.points.items;
    size_t cur_wall;
    size_t cur_door = 0;
    size_t cur_window = 0;

    for (cur_wall = 0; cur_wall < room->walls.size; ++cur_wall) {
        if (walls[cur_wall]) {
            /* Find the list of doors and windows for this wall, if any */
            while(cur_door < room->num_doors && room->doors[cur_door].wall < cur_wall) ++cur_door;
            while(cur_window < room->num_windows && room->windows[cur_window].wall < cur_wall) ++ cur_door;

            size_t door_list_end = cur_door;
            size_t window_list_end = cur_window;

            if (cur_door < room->num_doors && room->doors[cur_door].wall == cur_wall) {
                while(door_list_end < room->num_doors && room->doors[door_list_end].wall == cur_wall) ++door_list_end;
            }
            if (cur_window < room->num_windows && room->windows[cur_window].wall == cur_wall) {
                while(window_list_end < room->num_windows && room->windows[window_list_end].wall == cur_wall) ++window_list_end;
            }

            pb_shape3D* door_shapes;
            size_t num_door_shapes;
            pb_shape3D* window_shapes;
            size_t num_window_shapes;

            pb_point2D* point0 = room_points + cur_wall;
            pb_point2D* point1 = room_points + ((cur_wall + 1) % room->shape.points.size);

            /* The wall's start and end have to be flipped to correctly generate UVs */
            pb_line2D wall_line = {*point1, *point0};
            pb_line2D wall_normal_line = {*point0, *point1};

            pb_point2D normal = pb_line2D_get_normal(&wall_normal_line);

            size_t num_doors = cur_door == room->num_doors ? 0 : door_list_end - cur_door + 1;
            size_t num_windows = cur_window == room->num_windows ? 0 : window_list_end - cur_window + 1;

            int wall_result = pb_extrude_wall(&wall_line,
                                              num_doors ? room->doors + cur_door : NULL, num_doors,
                                              num_windows ? room->windows + cur_window : NULL, num_windows,
                                              bottom_floor_centre, &normal,
                                              start_height, floor_height, door_height, window_height,
                                              door_extruder, window_extruder, door_extruder_param, window_extruder_param,
                                              walls_out + cur_wall, wall_counts + cur_wall,
                                              &door_shapes, &num_door_shapes,
                                              &window_shapes, &num_window_shapes);

            if (wall_result == -1) {
                goto err_return;
            }

            /* Copy the doors and windows to our own arrays and free the other ones. Probably should just pass
             * an already allocated array to push on to like how walls are passed... */
            int push_back_err = 0;
            size_t j;
            for (j = 0; j < num_door_shapes; ++j) {
                if (!push_back_err && pb_vector_push_back(&doors_out, door_shapes + j) == -1) {
                    push_back_err = 1;
                }
                if (push_back_err) {
                    pb_shape3D_free(door_shapes + j);
                }
            }
            free(door_shapes);

            for (j = 0; j < num_window_shapes; ++j) {
                if (!push_back_err && pb_vector_push_back(&windows_out, window_shapes + j) == -1) {
                    push_back_err = 1;
                }
                if (push_back_err) {
                    pb_shape3D_free(window_shapes + j);
                }
            }
            free(window_shapes);

            if (push_back_err) {
                goto err_return;
            }

        } else {
            walls_out[cur_wall] = NULL;
            wall_counts[cur_wall] = 0;
        }
    }

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

    out->walls = walls_out;
    out->wall_counts = wall_counts;
    out->num_walls = room->shape.points.size;
    out->doors = (pb_shape3D*)doors_out.items;
    out->num_doors = doors_out.size;
    out->windows = (pb_shape3D*)windows_out.items;
    out->num_windows = windows_out.size;

    return out;

err_return:
{
    size_t i, j;
    for (i = 0; i < cur_wall; ++i) {
        for (j = 0; j < wall_counts[i]; ++j) {
            pb_shape3D_free(walls_out[i] + j);
        }
        free(walls_out[i]);
    }
    free(walls_out);

    pb_shape3D* door_shapes = (pb_shape3D*)doors_out.items;
    for (i = 0; i < doors_out.size; ++i) {
        pb_shape3D_free(door_shapes + i);
    }
    pb_vector_free(&doors_out);

    pb_shape3D* window_shapes = (pb_shape3D*)windows_out.items;
    for (i = 0; i < windows_out.size; ++i) {
        pb_shape3D_free(window_shapes + i);
    }
    pb_vector_free(&windows_out);

    /* Free the ceiling and ground */

    return NULL;
};
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


