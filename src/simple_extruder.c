#include <pb/simple_extruder.h>
#include <stdlib.h>
#include <pb/util/geom/types.h>
#include <pb/util/geom/shape_utils.h>
#include <math.h>
#include <pb/util/geom/line_utils.h>

void pb_simple_door_extruder_count(pb_line2D const* wall, pb_line2D const* wall_structure, pb_point2D const* normal,
                                   pb_point2D const* bottom_floor_centre, float floor_height,
                                   float struct_height, float start_height,
                                   void* param, size_t* num_walls, size_t* num_structures) {
    *num_walls = 1;
    *num_structures = 1;
}

int pb_simple_door_extruder_func(pb_line2D const* wall, pb_line2D const* wall_structure, pb_point2D const* normal,
                                 pb_point2D const* bottom_floor_centre, float floor_height,
                                 float struct_height, float start_height,
                                 void* param, pb_shape3D** walls_out, pb_shape3D** structures_out) {
    pb_shape3D* door = NULL;
    pb_shape3D* door_wall = NULL;

    /* The door will take up the entire width, with some space at the top */
    door = malloc(sizeof(pb_shape3D));
    door_wall = malloc(sizeof(pb_shape3D));

    if (!door || !door_wall) {
        free(door);
        free(door_wall);
        return -1;
    }

    /* Make sure we can safely free if necessary */
    size_t i;
    door->tris = NULL;
    door_wall->tris = NULL;

    if (pb_shape3D_init(door, 2) == -1 ||
        pb_shape3D_init(door_wall, 2) == -1) {
        goto err_return;
    }

    pb_point2D wall_structure_vec  = {wall_structure->end.x - wall_structure->start.x,
                                      wall_structure->end.y - wall_structure->start.y};
    pb_point2D wall_structure_centre = {wall_structure->start.x + wall_structure_vec.x / 2.f,
                                        wall_structure->start.y + wall_structure_vec.y / 2.f};
    pb_point2D wall_structure_len = {fabsf(wall_structure_vec.x), fabsf(wall_structure_vec.y)};

    /* Vectors in either direction */
    pb_point2D wall_end_to_start = {wall->start.x - wall->end.x, wall->start.y - wall->end.y};
    pb_point2D wall_start_to_end;

    /* 2D->3D: positive x is still positive x, but positive y is negative z (for now) */
    wall_end_to_start.x = wall_end_to_start.x < 0.f ? -1.f : (wall_end_to_start.x > 0.f ? 1.f : 0.f);
    wall_end_to_start.y = wall_end_to_start.y < 0.f ? 1.f : (wall_end_to_start.y > 0.f ? -1.f : 0.f);

    wall_start_to_end.x = -wall_end_to_start.x;
    wall_start_to_end.y = -wall_end_to_start.y;

    float actual_door_height = fminf(floor_height * 0.95f, struct_height); /* Leave some space at the top */
    float door_top_v = 1.f - (actual_door_height / floor_height);
    door->pos.x = wall_structure_centre.x - bottom_floor_centre->x;
    door->pos.y = start_height + (actual_door_height / 2.f);
    door->pos.z = -(wall_structure_centre.y - bottom_floor_centre->y);

    /* Bottom left corner */
    door->tris[0].nx = normal->x;
    door->tris[0].ny = 0.f;
    door->tris[0].nz = -normal->y;
    door->tris[0].x = wall_structure_len.x / 2.f * wall_end_to_start.x;
    door->tris[0].y = -actual_door_height / 2.f;
    door->tris[0].z = wall_structure_len.y / 2.f * wall_end_to_start.y;
    door->tris[0].v = 1.f;

    /* Now that we've figured out where the *actual* start of the wall is, calculate its u coordinate */
    pb_point2D s = {wall_structure_centre.x + door->tris[0].x, wall_structure_centre.y - door->tris[0].z};
    pb_point2D start_t = pb_line2D_get_t(wall, &s);
    float start_u = start_t.x == INFINITY ? start_t.y : start_t.x;
    door->tris[0].u = start_u;

    /* Top right corner */
    door->tris[1].nx = normal->x;
    door->tris[1].ny = 0.f;
    door->tris[1].nz = -normal->y;
    door->tris[1].x = wall_structure_len.x / 2.f * wall_start_to_end.x;
    door->tris[1].y = actual_door_height / 2.f;
    door->tris[1].z = wall_structure_len.y / 2.f * wall_start_to_end.y;
    door->tris[1].v = door_top_v;

    /* Same for the end */
    pb_point2D e = {wall_structure_centre.x + door->tris[1].x, wall_structure_centre.y - door->tris[1].z};
    pb_point2D end_t = pb_line2D_get_t(wall, &e);
    float end_u = end_t.x == INFINITY ? end_t.y : end_t.x;
    door->tris[1].u = end_u;

    /* Top left corner */
    door->tris[2].nx = normal->x;
    door->tris[2].ny = 0.f;
    door->tris[2].nz = -normal->y;
    door->tris[2].x = wall_structure_len.x / 2.f * wall_end_to_start.x;
    door->tris[2].y = actual_door_height / 2.f;
    door->tris[2].z = wall_structure_len.y / 2.f * wall_end_to_start.y;
    door->tris[2].u = start_u;
    door->tris[2].v = door_top_v;

    /* Bottom left corner again */
    door->tris[3] = door->tris[0];

    /* Bottom right corner */
    door->tris[4].nx = normal->x;
    door->tris[4].ny = 0.f;
    door->tris[4].nz = -normal->y;
    door->tris[4].x = wall_structure_len.x / 2.f * wall_start_to_end.x;
    door->tris[4].y = -actual_door_height / 2.f;
    door->tris[4].z = wall_structure_len.y / 2.f * wall_start_to_end.y;
    door->tris[4].u = end_u;
    door->tris[4].v = 1.f;

    /* Top right corner again */
    door->tris[5] = door->tris[1];

    float door_wall_height = floor_height - actual_door_height;
    door_wall->pos = door->pos;
    door_wall->pos.y += actual_door_height;

    door_wall->tris[0] = door->tris[0];
    door_wall->tris[0].y = -door_wall_height / 2.f;
    door_wall->tris[0].v = door_top_v;

    door_wall->tris[1] = door->tris[1];
    door_wall->tris[1].y = door_wall_height / 2.f;
    door_wall->tris[1].v = 0.f;

    door_wall->tris[2] = door->tris[2];
    door_wall->tris[2].y = door_wall_height / 2.f;
    door_wall->tris[2].v = 0.f;

    door_wall->tris[3] = door_wall->tris[0];

    door_wall->tris[4] = door->tris[4];
    door_wall->tris[4].y = -door_wall_height / 2.f;
    door_wall->tris[4].v = door_top_v;

    door_wall->tris[5] = door_wall->tris[1];

    *walls_out = door_wall;
    *structures_out = door;

    return 0;

err_return:
    pb_shape3D_free(door);
    pb_shape3D_free(door_wall);

    free(door_wall);
    free(door);
    return -1;
}

void pb_simple_window_extruder_count(pb_line2D const* wall, pb_line2D const* wall_structure, pb_point2D const* normal,
                                     pb_point2D const* bottom_floor_centre, float floor_height,
                                     float struct_height, float start_height,
                                     void* param, size_t* num_walls, size_t* num_structures) {
    *num_walls = 2;
    *num_structures = 1;
}

int pb_simple_window_extruder_func(pb_line2D const* wall, pb_line2D const* wall_structure, pb_point2D const* normal,
                                   pb_point2D const* bottom_floor_centre, float floor_height,
                                   float struct_height, float start_height,
                                   void* param, pb_shape3D** walls_out, pb_shape3D** structures_out) {
    pb_shape3D* window = NULL;
    pb_shape3D* window_walls = NULL;

    /* The door will take up the entire width, with some space at the top */
    window = malloc(sizeof(pb_shape3D));
    window_walls = malloc(sizeof(pb_shape3D) * 2);

    if (!window || !window_walls) {
        free(window);
        free(window_walls);
        return -1;
    }

    /* Make sure we can safely free if necessary */
    size_t i;
    window->tris = NULL;
    window_walls[0].tris = NULL;
    window_walls[1].tris = NULL;

    if (pb_shape3D_init(window, 2) == -1 ||
        pb_shape3D_init(window_walls + 0, 2) == -1 ||
        pb_shape3D_init(window_walls + 1, 2) == -1) {
        goto err_return;
    }

    pb_point2D wall_structure_vec  = {wall_structure->end.x - wall_structure->start.x,
                                      wall_structure->end.y - wall_structure->start.y};
    pb_point2D wall_structure_centre = {wall_structure->start.x + wall_structure_vec.x / 2.f,
                                        wall_structure->start.y + wall_structure_vec.y / 2.f};
    pb_point2D wall_structure_len = {fabsf(wall_structure_vec.x), fabsf(wall_structure_vec.y)};

    /* Vectors in either direction */
    pb_point2D wall_end_to_start = {wall->start.x - wall->end.x, wall->start.y - wall->end.y};
    pb_point2D wall_start_to_end;

    /* 2D->3D: positive x is still positive x, but positive y is negative z (for now) */
    wall_end_to_start.x = wall_end_to_start.x < 0.f ? -1.f : (wall_end_to_start.x > 0.f ? 1.f : 0.f);
    wall_end_to_start.y = wall_end_to_start.y < 0.f ? 1.f : (wall_end_to_start.y > 0.f ? -1.f : 0.f);

    wall_start_to_end.x = -wall_end_to_start.x;
    wall_start_to_end.y = -wall_end_to_start.y;

    float actual_window_height = fminf(floor_height * 0.75f, struct_height); /* Leave some space at the top and bottom */
    float window_wall_height = (floor_height - actual_window_height) / 2.f;
    float window_top_v = window_wall_height / floor_height;
    float window_bottom_v = 1 - window_top_v;

    window->pos.x = wall_structure_centre.x - bottom_floor_centre->x;
    window->pos.y = start_height + (actual_window_height / 2.f);
    window->pos.z = wall_structure_centre.y - bottom_floor_centre->y;

    /* Bottom left corner */
    window->tris[0].nx = normal->x;
    window->tris[0].ny = 0.f;
    window->tris[0].nz = -normal->y;
    window->tris[0].x = wall_structure_len.x / 2.f * wall_end_to_start.x;
    window->tris[0].y = -actual_window_height / 2.f;
    window->tris[0].z = wall_structure_len.y / 2.f * wall_end_to_start.y;
    window->tris[0].v = window_bottom_v;

    /* Now that we've figured out where the *actual* start of the wall is, calculate its u coordinate */
    pb_point2D s = {wall_structure_centre.x + window->tris[0].x, wall_structure_centre.y - window->tris[0].z};
    pb_point2D start_t = pb_line2D_get_t(wall, &s);
    float start_u = start_t.x == INFINITY ? start_t.y : start_t.x;
    window->tris[0].u = start_u;

    /* Top right corner */
    window->tris[1].nx = normal->x;
    window->tris[1].ny = 0.f;
    window->tris[1].nz = -normal->y;
    window->tris[1].x = wall_structure_len.x / 2.f * wall_start_to_end.x;
    window->tris[1].y = actual_window_height / 2.f;
    window->tris[1].z = wall_structure_len.y / 2.f * wall_start_to_end.y;
    window->tris[1].v = window_top_v;

    /* Same for the end */
    pb_point2D e = {wall_structure_centre.x + window->tris[1].x, wall_structure_centre.y - window->tris[1].z};
    pb_point2D end_t = pb_line2D_get_t(wall, &e);
    float end_u = end_t.x == INFINITY ? end_t.y : end_t.x;
    window->tris[1].u = end_u;

    /* Top left corner */
    window->tris[2].nx = normal->x;
    window->tris[2].ny = 0.f;
    window->tris[2].nz = -normal->y;
    window->tris[2].x = wall_structure_len.x / 2.f * wall_end_to_start.x;
    window->tris[2].y = actual_window_height / 2.f;
    window->tris[2].z = wall_structure_len.y / 2.f * wall_end_to_start.y;
    window->tris[2].u = start_u;
    window->tris[2].v = window_top_v;

    /* Bottom left corner again */
    window->tris[3] = window->tris[0];

    /* Bottom right corner */
    window->tris[4].nx = normal->x;
    window->tris[4].ny = 0.f;
    window->tris[4].nz = -normal->y;
    window->tris[4].x = wall_structure_len.x / 2.f * wall_start_to_end.x;
    window->tris[4].y = -actual_window_height / 2.f;
    window->tris[4].z = wall_structure_len.y / 2.f * wall_start_to_end.y;
    window->tris[4].u = end_u;
    window->tris[4].v = window_bottom_v;

    /* Top right corner again */
    window->tris[5] = window->tris[1];

    window_walls[0].pos = window->pos;
    window_walls[0].pos.y -= window_wall_height;

    window_walls[0].tris[0] = window->tris[0];
    window_walls[0].tris[0].y = -window_wall_height / 2.f;
    window_walls[0].tris[0].v = 1.f;

    window_walls[0].tris[1] = window->tris[1];
    window_walls[0].tris[1].y = window_wall_height / 2.f;
    window_walls[0].tris[1].v = window_bottom_v;

    window_walls[0].tris[2] = window->tris[2];
    window_walls[0].tris[2].y = window_wall_height / 2.f;
    window_walls[0].tris[2].v = window_bottom_v;

    window_walls[0].tris[3] = window_walls[0].tris[0];

    window_walls[0].tris[4] = window->tris[4];
    window_walls[0].tris[4].y = -window_wall_height / 2.f;
    window_walls[0].tris[4].v = 1.f;

    window_walls[0].tris[5] = window_walls[0].tris[1];


    window_walls[1].pos = window->pos;
    window_walls[1].pos.y += window_wall_height;

    window_walls[1].tris[0] = window->tris[0];
    window_walls[1].tris[0].y = -window_wall_height / 2.f;
    window_walls[1].tris[0].v = window_top_v;

    window_walls[1].tris[1] = window->tris[1];
    window_walls[1].tris[1].y = window_wall_height / 2.f;
    window_walls[1].tris[1].v = 0.f;

    window_walls[1].tris[2] = window->tris[2];
    window_walls[1].tris[2].y = window_wall_height / 2.f;
    window_walls[1].tris[2].v = 0.f;

    window_walls[1].tris[3] = window_walls[1].tris[0];

    window_walls[1].tris[4] = window->tris[4];
    window_walls[1].tris[4].y = -window_wall_height / 2.f;
    window_walls[1].tris[4].v = window_top_v;

    window_walls[1].tris[5] = window_walls[1].tris[1];

    *walls_out = window_walls;
    *structures_out = window;

    return 0;

err_return:
    pb_shape3D_free(window);
    pb_shape3D_free(window_walls);
    pb_shape3D_free(window_walls + 1);

    free(window);
    free(window_walls);
    return -1;
}


static pb_wall_structure_extruder simple_door_extruder = {
    pb_simple_door_extruder_count,
    pb_simple_door_extruder_func,
};
pb_wall_structure_extruder const* pb_simple_door_extruder = &simple_door_extruder;

static pb_wall_structure_extruder simple_window_extruder = {
        pb_simple_window_extruder_count,
        pb_simple_window_extruder_func,
};

pb_wall_structure_extruder const* pb_simple_window_extruder = &simple_window_extruder;


