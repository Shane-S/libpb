#include "../test_util.h"
#include <check.h>
#include <pb/floor_plan.h>
#include <pb/extrusion.h>
#include <pb/simple_extruder.h>
#include <pb/util/geom/types.h>
#include <pb/util/geom/shape_utils.h>
#include <pb/util/geom/line_utils.h>

#ifdef _WIN32
#include <malloc.h>
#endif

START_TEST(extrude_wall_exterior_simple_yaxis)
{
    /* Loop variables since we'll probably need them everywhere */
    size_t i, j;

    /* Inputs */
    pb_line2D wall = {{5.f, 3.f}, {5.f, 1.f}};
    pb_line2D wall_flipped = {{wall.end.x, wall.end.y}, {wall.start.x, wall.start.y}};

    float floor_height = 2.f;
    float start_height = 0.f;
    pb_point2D bottom_floor_centre = {8.f, 2.f};

    float window_height = 0.f;
    float door_height = 0.f;

    pb_point2D doors_in[] = {0};
    size_t num_doors_in = 0;

    pb_point2D windows_in[] = {0};
    size_t num_windows_in = 0;

    /* Expected outputs */
    pb_vert3D expected_wall0_points[] = {
            {0.f, -1.f, -1.f, -1.f, 0.f, 0.f, 0.f, 1.f},
            {0.f,  1.f,  1.f, -1.f, 0.f, 0.f, 1.f, 0.f},
            {0.f,  1.f, -1.f, -1.f, 0.f, 0.f, 0.f, 0.f},
            {0.f, -1.f, -1.f, -1.f, 0.f, 0.f, 0.f, 1.f},
            {0.f, -1.f,  1.f, -1.f, 0.f, 0.f, 1.f, 1.f},
            {0.f,  1.f,  1.f, -1.f, 0.f, 0.f, 1.f, 0.f},
    };
    pb_shape3D expected_wall0_shape = {&expected_wall0_points[0],
                                       sizeof(expected_wall0_points) / sizeof(pb_vert3D),
                                       {-3.f, 1.f, 0.f}};

    size_t expected_wall_point_counts[] = {
            sizeof(expected_wall0_points) / sizeof(pb_vert3D),
    };
    pb_shape3D* expected_walls[] = {&expected_wall0_shape};
    size_t expected_num_walls = sizeof(expected_walls) / sizeof(pb_shape3D*);


    size_t expected_door_point_counts[] = {
            0
    };
    pb_shape3D* expected_doors[] = {0};
    size_t expected_num_doors = 0;

    size_t expected_window_point_counts[] = {
            0
    };
    pb_shape3D* expected_windows[] = {0};
    size_t expected_num_windows = 0;

    /* Actual outputs */
    pb_shape3D* walls;
    size_t num_walls;

    pb_shape3D* doors;
    size_t num_doors;

    pb_shape3D* windows;
    size_t num_windows;

    pb_point2D normal = pb_line2D_get_normal(&wall_flipped);

    pb_extrude_wall(&wall,
                    &doors_in[0], num_doors_in,
                    &windows_in[0], num_windows_in,
                    &bottom_floor_centre,
                    &normal,
                    start_height, floor_height, door_height, window_height,
                    pb_simple_door_extruder, pb_simple_window_extruder, NULL, NULL,
                    &walls, &num_walls, &doors, &num_doors, &windows, &num_windows);

    ck_assert_msg(num_walls == expected_num_walls, "Expected num_walls == %lu, was %lu");

    for (i = 0; i < expected_num_walls; ++i) {
        ck_assert_msg(walls[i].num_tris == expected_wall_point_counts[i] / 3,
                      "wall %lu: expected num_tris == %lu, was %lu",
                      i, expected_wall_point_counts[i], walls[i].num_tris);

        pb_vert3D* verts = walls[i].tris;
        for (j = 0; j < walls[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_walls[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "wall %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "wall %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "wall %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    for (i = 0; i < expected_num_doors; ++i) {
        ck_assert_msg(doors[i].num_tris == expected_door_point_counts[i] / 3,
                      "door %lu: expected num_tris == %lu, was %lu",
                      i, expected_door_point_counts[i], walls[i].num_tris);

        pb_vert3D* verts = doors[i].tris;
        for (j = 0; j < doors[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_doors[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "door %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "door %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "door %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    for (i = 0; i < expected_num_windows; ++i) {
        ck_assert_msg(walls[i].num_tris == expected_window_point_counts[i] / 3,
                      "window %lu: expected num_tris == %lu, was %lu",
                      i, expected_window_point_counts[i], windows[i].num_tris);

        pb_vert3D* verts = windows[i].tris;
        for (j = 0; j < windows[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_windows[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "window %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "window %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "window %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    for (i = 0; i < num_walls; ++i) {
        pb_shape3D_free(walls + i);
    }

    for (i = 0; i < num_doors; ++i) {
        pb_shape3D_free(doors + i);
    }

    for (i = 0; i < num_windows; ++i) {
        pb_shape3D_free(windows + i);
    }
}
END_TEST

START_TEST(extrude_wall_exterior_simple_xaxis)
{
    /* Loop variables since we'll probably need them everywhere */
    size_t i, j;

    /* Inputs */
    pb_line2D wall = {{11.f, 3.f}, {5.f, 3.f}};
    pb_line2D wall_flipped = {{wall.end.x, wall.end.y}, {wall.start.x, wall.start.y}};

    float floor_height = 2.f;
    float start_height = 0.f;
    pb_point2D bottom_floor_centre = {8.f, 1.f};

    float window_height = 0.f;
    float door_height = 0.f;

    pb_point2D doors_in[] = {0};
    size_t num_doors_in = 0;

    pb_point2D windows_in[] = {0};
    size_t num_windows_in = 0;

    /* Expected outputs */
    pb_vert3D expected_wall0_points[] = {
            { 3.f, -1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f},
            {-3.f,  1.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f},
            { 3.f,  1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f},
            { 3.f, -1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f},
            {-3.f, -1.f, 0.f, 0.f, 0.f, -1.f, 1.f, 1.f},
            {-3.f,  1.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f},
    };
    pb_shape3D expected_wall0_shape = {&expected_wall0_points[0],
                                       sizeof(expected_wall0_points) / sizeof(pb_vert3D),
                                       {0.f, 1.f, -2.f}};

    size_t expected_wall_point_counts[] = {
            sizeof(expected_wall0_points) / sizeof(pb_vert3D),
    };
    pb_shape3D* expected_walls[] = {&expected_wall0_shape};
    size_t expected_num_walls = sizeof(expected_walls) / sizeof(pb_shape3D*);


    size_t expected_door_point_counts[] = {
            0
    };
    pb_shape3D* expected_doors[] = {0};
    size_t expected_num_doors = 0;

    size_t expected_window_point_counts[] = {
            0
    };
    pb_shape3D* expected_windows[] = {0};
    size_t expected_num_windows = 0;

    /* Actual outputs */
    pb_shape3D* walls;
    size_t num_walls;

    pb_shape3D* doors;
    size_t num_doors;

    pb_shape3D* windows;
    size_t num_windows;

    pb_point2D normal = pb_line2D_get_normal(&wall_flipped);

    pb_extrude_wall(&wall,
                    &doors_in[0], num_doors_in,
                    &windows_in[0], num_windows_in,
                    &bottom_floor_centre,
                    &normal,
                    start_height, floor_height, door_height, window_height,
                    pb_simple_door_extruder, pb_simple_window_extruder, NULL, NULL,
                    &walls, &num_walls, &doors, &num_doors, &windows, &num_windows);

    ck_assert_msg(num_walls == expected_num_walls, "Expected num_walls == %lu, was %lu");

    for (i = 0; i < expected_num_walls; ++i) {
        ck_assert_msg(walls[i].num_tris == expected_wall_point_counts[i] / 3,
                      "wall %lu: expected num_tris == %lu, was %lu",
                      i, expected_wall_point_counts[i], walls[i].num_tris);

        pb_vert3D* verts = walls[i].tris;
        for (j = 0; j < walls[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_walls[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "wall %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "wall %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "wall %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    for (i = 0; i < expected_num_doors; ++i) {
        ck_assert_msg(doors[i].num_tris == expected_door_point_counts[i] / 3,
                      "door %lu: expected num_tris == %lu, was %lu",
                      i, expected_door_point_counts[i], walls[i].num_tris);

        pb_vert3D* verts = doors[i].tris;
        for (j = 0; j < doors[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_doors[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "door %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "door %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "door %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    for (i = 0; i < expected_num_windows; ++i) {
        ck_assert_msg(walls[i].num_tris == expected_window_point_counts[i] / 3,
                      "window %lu: expected num_tris == %lu, was %lu",
                      i, expected_window_point_counts[i], windows[i].num_tris);

        pb_vert3D* verts = windows[i].tris;
        for (j = 0; j < windows[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_windows[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "window %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "window %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "window %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    for (i = 0; i < num_walls; ++i) {
        pb_shape3D_free(walls + i);
    }

    for (i = 0; i < num_doors; ++i) {
        pb_shape3D_free(doors + i);
    }

    for (i = 0; i < num_windows; ++i) {
        pb_shape3D_free(windows + i);
    }
}
END_TEST

START_TEST(extrude_wall_exterior_windows_yaxis)
{
    /* Loop variables since we'll probably need them everywhere */
    size_t i, j;

    /* Inputs */
    pb_line2D wall = {{5.f, 13.f}, {5.f, 1.f}};
    pb_line2D wall_flipped = {{wall.end.x, wall.end.y}, {wall.start.x, wall.start.y}};

    float floor_height = 2.f;
    float start_height = 4.f;
    pb_point2D bottom_floor_centre = {8.f, 7.f};

    float window_height = 0.5f;
    float door_height = 0.f;

    pb_wall_structure doors_in[] = {0};
    size_t num_doors_in = 0;

    pb_wall_structure windows_in[] = {
            {{5.f, 4.f}, {5.f, 6.f}, 0},
            {{5.f, 8.f}, {5.f, 11.f}, 0},
    };
    size_t num_windows_in = sizeof(windows_in) / sizeof(pb_wall_structure);

    /* Expected outputs */
    pb_vert3D expected_wall0_points[] = {
          /* x     y     z     nx   ny   nz   u          v  */
            {0.f, -1.f, -1.f, -1.f, 0.f, 0.f, 0.f,       1.f},
            {0.f,  1.f,  1.f, -1.f, 0.f, 0.f, 1.f / 6.f, 0.f},
            {0.f,  1.f, -1.f, -1.f, 0.f, 0.f, 0.f,       0.f},
            {0.f, -1.f, -1.f, -1.f, 0.f, 0.f, 0.f,       1.f},
            {0.f, -1.f,  1.f, -1.f, 0.f, 0.f, 1.f / 6.f, 1.f},
            {0.f,  1.f,  1.f, -1.f, 0.f, 0.f, 1.f / 6.f, 0.f},
    };
    pb_shape3D expected_wall0_shape = {&expected_wall0_points[0],
                                       sizeof(expected_wall0_points) / sizeof(pb_vert3D),
                                       {-3.f, 5.f, -5.f}};

    pb_vert3D expected_wall1_points[] = {
          /* x     y        z      nx   ny   nz   u           v  */
            {0.f, -0.375f, -1.5f, -1.f, 0.f, 0.f, 1.f / 6.f,  1.f      },
            {0.f,  0.375f,  1.5f, -1.f, 0.f, 0.f, 5.f / 12.f, 5.f / 8.f},
            {0.f,  0.375f, -1.5f, -1.f, 0.f, 0.f, 1.f / 6.f,  5.f / 8.f},
            {0.f, -0.375f, -1.5f, -1.f, 0.f, 0.f, 1.f / 6.f,  1.f      },
            {0.f, -0.375f,  1.5f, -1.f, 0.f, 0.f, 5.f / 12.f, 1.f      },
            {0.f,  0.375f,  1.5f, -1.f, 0.f, 0.f, 5.f / 12.f, 5.f / 8.f},
    };
    pb_shape3D expected_wall1_shape = {&expected_wall1_points[0],
                                       sizeof(expected_wall1_points) / sizeof(pb_vert3D),
                                       {-3.f, 4.375f, -2.5f}};

    pb_vert3D expected_wall2_points[] = {
          /* x     y        z      nx   ny   nz   u           v  */
            {0.f, -0.375f, -1.5f, -1.f, 0.f, 0.f, 1.f / 6.f,  3.f / 8.f},
            {0.f,  0.375f,  1.5f, -1.f, 0.f, 0.f, 5.f / 12.f, 0.f      },
            {0.f,  0.375f, -1.5f, -1.f, 0.f, 0.f, 1.f / 6.f,  0.f      },
            {0.f, -0.375f, -1.5f, -1.f, 0.f, 0.f, 1.f / 6.f,  3.f / 8.f},
            {0.f, -0.375f,  1.5f, -1.f, 0.f, 0.f, 5.f / 12.f, 3.f / 8.f},
            {0.f,  0.375f,  1.5f, -1.f, 0.f, 0.f, 5.f / 12.f, 0.f      },
    };
    pb_shape3D expected_wall2_shape = {&expected_wall2_points[0],
                                       sizeof(expected_wall2_points) / sizeof(pb_vert3D),
                                       {-3.f, 5.625f, -2.5f}};

    pb_vert3D expected_wall3_points[] = {
            /* x     y     z   nx   ny   nz   u          v  */
            {0.f, -1.f, -1.f, -1.f, 0.f, 0.f, 5.f / 12.f, 1.f},
            {0.f,  1.f,  1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 0.f},
            {0.f,  1.f, -1.f, -1.f, 0.f, 0.f, 5.f / 12.f, 0.f},
            {0.f, -1.f, -1.f, -1.f, 0.f, 0.f, 5.f / 12.f, 1.f},
            {0.f, -1.f,  1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 1.f},
            {0.f,  1.f,  1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 0.f},
    };
    pb_shape3D expected_wall3_shape = {&expected_wall3_points[0],
                                       sizeof(expected_wall3_points) / sizeof(pb_vert3D),
                                       {-3.f, 5.f, 0.f}};

    pb_vert3D expected_wall4_points[] = {
            /* x     y        z   nx   ny   nz   u           v  */
            {0.f, -0.375f, -1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 1.f      },
            {0.f,  0.375f,  1.f, -1.f, 0.f, 0.f, 9.f / 12.f, 5.f / 8.f},
            {0.f,  0.375f, -1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 5.f / 8.f},
            {0.f, -0.375f, -1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 1.f      },
            {0.f, -0.375f,  1.f, -1.f, 0.f, 0.f, 9.f / 12.f, 1.f      },
            {0.f,  0.375f,  1.f, -1.f, 0.f, 0.f, 9.f / 12.f, 5.f / 8.f},
    };
    pb_shape3D expected_wall4_shape = {&expected_wall4_points[0],
                                       sizeof(expected_wall4_points) / sizeof(pb_vert3D),
                                       {-3.f, 4.375f, 2.f}};

    pb_vert3D expected_wall5_points[] = {
            /* x     y        z   nx   ny   nz   u           v  */
            {0.f, -0.375f, -1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 3.f / 8.f},
            {0.f,  0.375f,  1.f, -1.f, 0.f, 0.f, 9.f / 12.f, 0.f      },
            {0.f,  0.375f, -1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 0.f      },
            {0.f, -0.375f, -1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 3.f / 8.f},
            {0.f, -0.375f,  1.f, -1.f, 0.f, 0.f, 9.f / 12.f, 3.f / 8.f},
            {0.f,  0.375f,  1.f, -1.f, 0.f, 0.f, 9.f / 12.f, 0.f      },
    };
    pb_shape3D expected_wall5_shape = {&expected_wall5_points[0],
                                       sizeof(expected_wall5_points) / sizeof(pb_vert3D),
                                       {-3.f, 5.625f, 2.f}};

    pb_vert3D expected_wall6_points[] = {
            /* x   y     z     nx    ny   nz   u           v  */
            {0.f, -1.f, -1.5f, -1.f, 0.f, 0.f, 9.f / 12.f, 1.f},
            {0.f,  1.f,  1.5f, -1.f, 0.f, 0.f, 1.f,        0.f},
            {0.f,  1.f, -1.5f, -1.f, 0.f, 0.f, 9.f / 12.f, 0.f},
            {0.f, -1.f, -1.5f, -1.f, 0.f, 0.f, 9.f / 12.f, 1.f},
            {0.f, -1.f,  1.5f, -1.f, 0.f, 0.f, 1.f,        1.f},
            {0.f,  1.f,  1.5f, -1.f, 0.f, 0.f, 1.f,        0.f},
    };
    pb_shape3D expected_wall6_shape = {&expected_wall6_points[0],
                                       sizeof(expected_wall6_points) / sizeof(pb_vert3D),
                                       {-3.f, 5.f, 4.5f}};

    size_t expected_wall_point_counts[] = {
            sizeof(expected_wall0_points) / sizeof(pb_vert3D),
            sizeof(expected_wall1_points) / sizeof(pb_vert3D),
            sizeof(expected_wall2_points) / sizeof(pb_vert3D),
            sizeof(expected_wall3_points) / sizeof(pb_vert3D),
            sizeof(expected_wall4_points) / sizeof(pb_vert3D),
            sizeof(expected_wall5_points) / sizeof(pb_vert3D),
            sizeof(expected_wall6_points) / sizeof(pb_vert3D),
    };
    pb_shape3D* expected_walls[] = {&expected_wall0_shape,
                                    &expected_wall1_shape,
                                    &expected_wall2_shape,
                                    &expected_wall3_shape,
                                    &expected_wall4_shape,
                                    &expected_wall5_shape,
                                    &expected_wall6_shape,};
    size_t expected_num_walls = sizeof(expected_walls) / sizeof(pb_shape3D*);

    size_t expected_door_point_counts[] = {
            0
    };
    pb_shape3D* expected_doors[] = {0};
    size_t expected_num_doors = 0;

    pb_vert3D expected_window0_points[] = {
            /* x   y       z      nx   ny   nz   u           v  */
            {0.f, -0.25f, -1.5f, -1.f, 0.f, 0.f, 1.f / 6.f,  5.f / 8.f},
            {0.f,  0.25f,  1.5f, -1.f, 0.f, 0.f, 5.f / 12.f, 3.f / 8.f},
            {0.f,  0.25f, -1.5f, -1.f, 0.f, 0.f, 1.f / 6.f,  3.f / 8.f},
            {0.f, -0.25f, -1.5f, -1.f, 0.f, 0.f, 1.f / 6.f,  5.f / 8.f},
            {0.f, -0.25f,  1.5f, -1.f, 0.f, 0.f, 5.f / 12.f, 5.f / 8.f},
            {0.f,  0.25f,  1.5f, -1.f, 0.f, 0.f, 5.f / 12.f, 3.f / 8.f},
    };
    pb_shape3D expected_window0_shape = {&expected_window0_points[0],
                                         sizeof(expected_window0_points) / sizeof(pb_vert3D),
                                         {-3.f, 5.25f, -2.5f}};

    pb_vert3D expected_window1_points[] = {
            /* x   y       z      nx   ny   nz   u           v  */
            {0.f, -0.25f, -1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 5.f / 8.f},
            {0.f,  0.25f,  1.f, -1.f, 0.f, 0.f, 9.f / 12.f, 3.f / 8.f},
            {0.f,  0.25f, -1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 3.f / 8.f},
            {0.f, -0.25f, -1.f, -1.f, 0.f, 0.f, 7.f / 12.f, 5.f / 8.f},
            {0.f, -0.25f,  1.f, -1.f, 0.f, 0.f, 9.f / 12.f, 5.f / 8.f},
            {0.f,  0.25f,  1.f, -1.f, 0.f, 0.f, 9.f / 12.f, 3.f / 8.f},
    };
    pb_shape3D expected_window1_shape = {&expected_window1_points[0],
                                         sizeof(expected_window1_points) / sizeof(pb_vert3D),
                                         {-3.f, 5.25f, -2.5f}};

    size_t expected_window_point_counts[] = {
            sizeof(expected_window0_points) / sizeof(pb_vert3D),
            sizeof(expected_window1_points) / sizeof(pb_vert3D),
    };
    pb_shape3D* expected_windows[] = {&expected_window0_shape,
                                      &expected_window1_shape};
    size_t expected_num_windows = sizeof(expected_windows) / sizeof(pb_shape3D*);

    /* Actual outputs */
    pb_shape3D* walls;
    size_t num_walls;

    pb_shape3D* doors;
    size_t num_doors;

    pb_shape3D* windows;
    size_t num_windows;

    pb_point2D normal = pb_line2D_get_normal(&wall_flipped);

    pb_extrude_wall(&wall,
                    &doors_in[0], num_doors_in,
                    &windows_in[0], num_windows_in,
                    &bottom_floor_centre,
                    &normal,
                    start_height, floor_height, door_height, window_height,
                    pb_simple_door_extruder, pb_simple_window_extruder, NULL, NULL,
                    &walls, &num_walls, &doors, &num_doors, &windows, &num_windows);

    ck_assert_msg(num_walls == expected_num_walls, "Expected num_walls == %lu, was %lu", expected_num_walls, num_walls);

    for (i = 0; i < expected_num_walls; ++i) {
        ck_assert_msg(walls[i].num_tris == expected_wall_point_counts[i] / 3,
                      "wall %lu: expected num_tris == %lu, was %lu",
                      i, expected_wall_point_counts[i], walls[i].num_tris);

        pb_vert3D* verts = walls[i].tris;
        for (j = 0; j < walls[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_walls[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "wall %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "wall %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "wall %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    ck_assert_msg(num_doors == expected_num_doors, "Expected num_doors == %lu, was %lu", expected_num_doors, num_doors);
    for (i = 0; i < expected_num_doors; ++i) {
        ck_assert_msg(doors[i].num_tris == expected_door_point_counts[i] / 3,
                      "door %lu: expected num_tris == %lu, was %lu",
                      i, expected_door_point_counts[i], walls[i].num_tris);

        pb_vert3D* verts = doors[i].tris;
        for (j = 0; j < doors[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_doors[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "door %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "door %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "door %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    ck_assert_msg(num_windows == expected_num_windows, "Expected num_windows == %lu, was %lu", expected_num_windows, num_windows);
    for (i = 0; i < expected_num_windows; ++i) {
        ck_assert_msg(walls[i].num_tris == expected_window_point_counts[i] / 3,
                      "window %lu: expected num_tris == %lu, was %lu",
                      i, expected_window_point_counts[i], windows[i].num_tris);

        pb_vert3D* verts = windows[i].tris;
        for (j = 0; j < windows[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_windows[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "window %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "window %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "window %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    for (i = 0; i < num_walls; ++i) {
        pb_shape3D_free(walls + i);
    }
    free(walls);

    for (i = 0; i < num_doors; ++i) {
        pb_shape3D_free(doors + i);
    }
    free(doors);

    for (i = 0; i < num_windows; ++i) {
        pb_shape3D_free(windows + i);
    }
    free(windows);
}
END_TEST

START_TEST(extrude_wall_exterior_windows_xaxis)
{
    /* Loop variables since we'll probably need them everywhere */
    size_t i, j;

    /* Inputs */
    pb_line2D wall = {{11.f, 9.f}, {1.f, 9.f}};
    pb_line2D wall_flipped = {{wall.end.x, wall.end.y}, {wall.start.x, wall.start.y}};

    float floor_height = 2.f;
    float start_height = 4.f;
    pb_point2D bottom_floor_centre = {6.f, 7.f};

    float window_height = 0.5f;
    float door_height = 0.f;

    pb_wall_structure doors_in[] = {0};
    size_t num_doors_in = 0;

    pb_wall_structure windows_in[] = {
            {{9.f, 9.f}, {7.f, 9.f}, 0},
            {{5.f, 9.f}, {2.f, 9.f}, 0},
    };
    size_t num_windows_in = sizeof(windows_in) / sizeof(pb_wall_structure);

    pb_vert3D expected_wall0_points[] = {
           /* x     y    z    nx   ny    nz   u           v  */
            { 1.f, -1.f, 0.f, 0.f, 0.f, -1.f, 0.f,        1.f},
            {-1.f,  1.f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 0.f},
            { 1.f,  1.f, 0.f, 0.f, 0.f, -1.f, 0.f,        0.f},
            { 1.f, -1.f, 0.f, 0.f, 0.f, -1.f, 0.f,        1.f},
            {-1.f, -1.f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 1.f},
            {-1.f,  1.f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 0.f},
    };
    pb_shape3D expected_wall0_shape = {&expected_wall0_points[0],
                                       sizeof(expected_wall0_points) / sizeof(pb_vert3D),
                                       {4.f, 5.f, -2.f}};

    pb_vert3D expected_wall1_points[] = {
           /* x     y       z    nx   ny    nz   u           v  */
            { 1.f, -0.375f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 1.f      },
            {-1.f,  0.375f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 5.f / 8.f},
            { 1.f,  0.375f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 5.f / 8.f},
            { 1.f, -0.375f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 1.f      },
            {-1.f, -0.375f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 1.f      },
            {-1.f,  0.375f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 5.f / 8.f},
    };
    pb_shape3D expected_wall1_shape = {&expected_wall1_points[0],
                                       sizeof(expected_wall1_points) / sizeof(pb_vert3D),
                                       {2.f, 5.625f, -2.f}};

    pb_vert3D expected_wall2_points[] = {
           /* x     y       z    nx   ny    nz   u           v  */
            { 1.f, -0.375f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 3.f / 8.f},
            {-1.f,  0.375f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 0.f      },
            { 1.f,  0.375f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 0.f      },
            { 1.f, -0.375f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 3.f / 8.f},
            {-1.f, -0.375f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 3.f / 8.f},
            {-1.f,  0.375f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 0.f      },
    };
    pb_shape3D expected_wall2_shape = {&expected_wall2_points[0],
                                       sizeof(expected_wall2_points) / sizeof(pb_vert3D),
                                       {2.f, 4.375f, -2.f}};

    pb_vert3D expected_wall3_points[] = {
           /* x     y    z    nx   ny    nz   u           v  */
            { 1.f, -1.f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 1.f},
            {-1.f,  1.f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 0.f},
            { 1.f,  1.f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 0.f},
            { 1.f, -1.f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 1.f},
            {-1.f, -1.f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 1.f},
            {-1.f,  1.f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 0.f},
    };
    pb_shape3D expected_wall3_shape = {&expected_wall3_points[0],
                                       sizeof(expected_wall3_points) / sizeof(pb_vert3D),
                                       {0.f, 5.f, -2.f}};

    pb_vert3D expected_wall4_points[] = {
           /* x      y       z    nx   ny    nz   u           v  */
            { 1.5f, -0.375f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 1.f      },
            {-1.5f,  0.375f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 5.f / 8.f},
            { 1.5f,  0.375f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 5.f / 8.f},
            { 1.5f, -0.375f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 1.f      },
            {-1.5f, -0.375f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 1.f      },
            {-1.5f,  0.375f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 5.f / 8.f},
    };
    pb_shape3D expected_wall4_shape = {&expected_wall4_points[0],
                                       sizeof(expected_wall4_points) / sizeof(pb_vert3D),
                                       {-2.5f, 5.625f, -2.f}};

    pb_vert3D expected_wall5_points[] = {
           /* x     y        z    nx   ny    nz   u           v  */
            { 1.5f, -0.375f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 3.f / 8.f},
            {-1.5f,  0.375f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 0.f      },
            { 1.5f,  0.375f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 0.f      },
            { 1.5f, -0.375f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 3.f / 8.f},
            {-1.5f, -0.375f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 3.f / 8.f},
            {-1.5f,  0.375f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 0.f      },
    };
    pb_shape3D expected_wall5_shape = {&expected_wall5_points[0],
                                       sizeof(expected_wall5_points) / sizeof(pb_vert3D),
                                       {-2.5f, 4.375f, -2.f}};

    /* Expected outputs */
    pb_vert3D expected_wall6_points[] = {
           /* x       y    z    nx   ny    nz   u          v  */
            { 0.5f, -1.f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 1.f},
            {-0.5f,  1.f, 0.f, 0.f, 0.f, -1.f, 1.f,        0.f},
            { 0.5f,  1.f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 0.f},
            { 0.5f, -1.f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 1.f},
            {-0.5f, -1.f, 0.f, 0.f, 0.f, -1.f, 1.f,        1.f},
            {-0.5f,  1.f, 0.f, 0.f, 0.f, -1.f, 1.f,        0.f},
    };
    pb_shape3D expected_wall6_shape = {&expected_wall6_points[0],
                                       sizeof(expected_wall6_points) / sizeof(pb_vert3D),
                                       {-5.5f, 5.f, -2.f}};

    size_t expected_wall_point_counts[] = {
            sizeof(expected_wall0_points) / sizeof(pb_vert3D),
            sizeof(expected_wall1_points) / sizeof(pb_vert3D),
            sizeof(expected_wall2_points) / sizeof(pb_vert3D),
            sizeof(expected_wall3_points) / sizeof(pb_vert3D),
            sizeof(expected_wall4_points) / sizeof(pb_vert3D),
            sizeof(expected_wall5_points) / sizeof(pb_vert3D),
            sizeof(expected_wall6_points) / sizeof(pb_vert3D),
    };
    pb_shape3D* expected_walls[] = {&expected_wall0_shape,
                                    &expected_wall1_shape,
                                    &expected_wall2_shape,
                                    &expected_wall3_shape,
                                    &expected_wall4_shape,
                                    &expected_wall5_shape,
                                    &expected_wall6_shape,};
    size_t expected_num_walls = sizeof(expected_walls) / sizeof(pb_shape3D*);

    size_t expected_door_point_counts[] = {
            0
    };
    pb_shape3D* expected_doors[] = {0};
    size_t expected_num_doors = 0;

    pb_vert3D expected_window0_points[] = {
            /* x   y       z    nx   ny    nz   u           v  */
            { 1.f, -0.25f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 5.f / 8.f},
            {-1.f,  0.25f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 3.f / 8.f},
            { 1.f,  0.25f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 3.f / 8.f},
            { 1.f, -0.25f, 0.f, 0.f, 0.f, -1.f, 2.f / 10.f, 5.f / 8.f},
            {-1.f, -0.25f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 5.f / 8.f},
            {-1.f,  0.25f, 0.f, 0.f, 0.f, -1.f, 4.f / 10.f, 3.f / 8.f},
    };
    pb_shape3D expected_window0_shape = {&expected_window0_points[0],
                                         sizeof(expected_window0_points) / sizeof(pb_vert3D),
                                         {1.f, 5.25f, -2.f}};

    pb_vert3D expected_window1_points[] = {
            /* x     y      z    nx   ny    nz   u           v  */
            { 1.5f, -0.25f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 5.f / 8.f},
            {-1.5f,  0.25f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 3.f / 8.f},
            { 1.5f,  0.25f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 3.f / 8.f},
            { 1.5f, -0.25f, 0.f, 0.f, 0.f, -1.f, 6.f / 10.f, 5.f / 8.f},
            {-1.5f, -0.25f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 5.f / 8.f},
            {-1.5f,  0.25f, 0.f, 0.f, 0.f, -1.f, 9.f / 10.f, 3.f / 8.f},
    };
    pb_shape3D expected_window1_shape = {&expected_window1_points[0],
                                         sizeof(expected_window1_points) / sizeof(pb_vert3D),
                                         {-2.5f, 5.25f, -2.f}};

    size_t expected_window_point_counts[] = {
            sizeof(expected_window0_points) / sizeof(pb_vert3D),
            sizeof(expected_window1_points) / sizeof(pb_vert3D),
    };
    pb_shape3D* expected_windows[] = {&expected_window0_shape,
                                      &expected_window1_shape};
    size_t expected_num_windows = sizeof(expected_windows) / sizeof(pb_shape3D*);

    /* Actual outputs */
    pb_shape3D* walls;
    size_t num_walls;

    pb_shape3D* doors;
    size_t num_doors;

    pb_shape3D* windows;
    size_t num_windows;

    pb_point2D normal = pb_line2D_get_normal(&wall_flipped);

    pb_extrude_wall(&wall,
                    &doors_in[0], num_doors_in,
                    &windows_in[0], num_windows_in,
                    &bottom_floor_centre,
                    &normal,
                    start_height, floor_height, door_height, window_height,
                    pb_simple_door_extruder, pb_simple_window_extruder, NULL, NULL,
                    &walls, &num_walls, &doors, &num_doors, &windows, &num_windows);

    ck_assert_msg(num_walls == expected_num_walls, "Expected num_walls == %lu, was %lu", expected_num_walls, num_walls);

    for (i = 0; i < expected_num_walls; ++i) {
        ck_assert_msg(walls[i].num_tris == expected_wall_point_counts[i] / 3,
                      "wall %lu: expected num_tris == %lu, was %lu",
                      i, expected_wall_point_counts[i], walls[i].num_tris);

        pb_vert3D* verts = walls[i].tris;
        for (j = 0; j < walls[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_walls[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "wall %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "wall %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "wall %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    ck_assert_msg(num_doors == expected_num_doors, "Expected num_doors == %lu, was %lu", expected_num_doors, num_doors);
    for (i = 0; i < expected_num_doors; ++i) {
        ck_assert_msg(doors[i].num_tris == expected_door_point_counts[i] / 3,
                      "door %lu: expected num_tris == %lu, was %lu",
                      i, expected_door_point_counts[i], walls[i].num_tris);

        pb_vert3D* verts = doors[i].tris;
        for (j = 0; j < doors[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_doors[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "door %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "door %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "door %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    ck_assert_msg(num_windows == expected_num_windows, "Expected num_windows == %lu, was %lu", expected_num_windows, num_windows);
    for (i = 0; i < expected_num_windows; ++i) {
        ck_assert_msg(walls[i].num_tris == expected_window_point_counts[i] / 3,
                      "window %lu: expected num_tris == %lu, was %lu",
                      i, expected_window_point_counts[i], windows[i].num_tris);

        pb_vert3D* verts = windows[i].tris;
        for (j = 0; j < windows[i].num_tris * 3; ++j) {
            pb_vert3D* vert = verts + j;
            pb_vert3D* expected_vert = expected_windows[i]->tris + j;

            /* Back to using assert_close_enough because my other float comparison function chokes in some situations */
            ck_assert_msg(assert_close_enough(vert->x, expected_vert->x, 5) &&
                          assert_close_enough(vert->y, expected_vert->y, 5) &&
                          assert_close_enough(vert->z, expected_vert->z, 5),
                          "window %lu vert %lu: expected position (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->x, expected_vert->y, expected_vert->z, vert->x, vert->y, vert->z);

            ck_assert_msg(assert_close_enough(vert->nx, expected_vert->nx, 5) &&
                          assert_close_enough(vert->ny, expected_vert->ny, 5) &&
                          assert_close_enough(vert->nz, expected_vert->nz, 5),
                          "window %lu vert %lu: expected normal (%.3f, %.3f, %.3f), was (%.3f, %.3f, %.3f)",
                          i, j, expected_vert->nx, expected_vert->ny, expected_vert->nz, vert->nx, vert->ny, vert->nz);

            ck_assert_msg(assert_close_enough(vert->u, expected_vert->u, 5) &&
                          assert_close_enough(vert->v, expected_vert->v, 5),
                          "window %lu vert %lu: expected UVs (%.3f, %.3f), was (%.3f, %.3f)",
                          i, j, expected_vert->u, expected_vert->v, vert->u, vert->v);
        }
    }

    for (i = 0; i < num_walls; ++i) {
        pb_shape3D_free(walls + i);
    }
    free(walls);

    for (i = 0; i < num_doors; ++i) {
        pb_shape3D_free(doors + i);
    }
    free(doors);

    for (i = 0; i < num_windows; ++i) {
        pb_shape3D_free(windows + i);
    }
    free(windows);
}
END_TEST

START_TEST(extrude_wall_exterior_windows_doors_yaxis)
{

}
END_TEST

Suite *make_pb_extrusion_suite(void)
{

    Suite *s;
    TCase *tc_extrude_exterior_wall;

    s = suite_create("Extrusion");

    tc_extrude_exterior_wall = tcase_create("Exterior wall extrusion tests");
    suite_add_tcase(s, tc_extrude_exterior_wall);
    tcase_add_test(tc_extrude_exterior_wall, extrude_wall_exterior_simple_yaxis);
    tcase_add_test(tc_extrude_exterior_wall, extrude_wall_exterior_simple_xaxis);
    tcase_add_test(tc_extrude_exterior_wall, extrude_wall_exterior_windows_yaxis);
    tcase_add_test(tc_extrude_exterior_wall, extrude_wall_exterior_windows_xaxis);
    return s;
}