#include <libcompat.h>
#include <check.h>
#include <pb/pb_sq_house.h>
#include <pb/internal/pb_sq_house_layout.h>

START_TEST(get_shared_wall_right)
{
    /* Given a room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}} and a second room with points {{10, 15}, {10, 0}, {25, 0}, {25, 15}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be RIGHT
     */
    pb_point points1[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };
    pb_point points2[] = { { 10.f, 15.f }, { 10.f, 0.f }, { 25.f, 0.f }, { 25.f, 15.f } };

    pb_room r1;
    pb_room r2;

    int result;
    
    r1.room_shape.points = &points1[0];
    r1.room_shape.num_points = 4;

    r2.room_shape.points = &points2[0];
    r2.room_shape.num_points = 4;

    result = pb_sq_house_get_shared_wall(&r1, &r2);
    ck_assert_msg(result == SQ_HOUSE_RIGHT, "result should have been 2 (right), was %d", result);
}
END_TEST

START_TEST(get_shared_wall_left)
{
    /* Given a room with points {{10, 15}, {10, 0}, {25, 0}, {25, 15}} and a second room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be LEFT */

    pb_point points1[] = { { 10.f, 15.f }, { 10.f, 0.f }, { 25.f, 0.f }, { 25.f, 15.f } };
    pb_point points2[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };

    pb_room r1;
    pb_room r2;

    int result;

    r1.room_shape.points = &points1[0];
    r1.room_shape.num_points = 4;

    r2.room_shape.points = &points2[0];
    r2.room_shape.num_points = 4;

    result = pb_sq_house_get_shared_wall(&r1, &r2);
    ck_assert_msg(result == SQ_HOUSE_LEFT, "result should have been 3 (left), was %d", result);
}
END_TEST

START_TEST(get_shared_wall_top)
{
    /* Given a room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}} and a second room with points {{0, 30}, {0, 10}, {18, 10}, {18, 30}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be TOP */

    pb_point points1[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };
    pb_point points2[] = { { 0.f, 30.f }, { 0.f, 10.f }, { 18.f, 10.f }, { 18.f, 30.f } };

    pb_room r1;
    pb_room r2;

    int result;

    r1.room_shape.points = &points1[0];
    r1.room_shape.num_points = 4;

    r2.room_shape.points = &points2[0];
    r2.room_shape.num_points = 4;

    result = pb_sq_house_get_shared_wall(&r1, &r2);
    ck_assert_msg(result == SQ_HOUSE_TOP, "result should have been 1 (top), was %d", result);
}
END_TEST

START_TEST(get_shared_wall_bottom)
{
    /* Given a room with points {{0, 30}, {0, 10}, {18, 10}, {18, 30}} and a second room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be BOTTOM */

    pb_point points1[] = { { 0.f, 30.f }, { 0.f, 10.f }, { 18.f, 10.f }, { 18.f, 30.f } };
    pb_point points2[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };

    pb_room r1;
    pb_room r2;

    int result;

    r1.room_shape.points = &points1[0];
    r1.room_shape.num_points = 4;

    r2.room_shape.points = &points2[0];
    r2.room_shape.num_points = 4;

    result = pb_sq_house_get_shared_wall(&r1, &r2);
    ck_assert_msg(result == SQ_HOUSE_BOTTOM, "result should have been 4 (bottom), was %d", result);
}
END_TEST

START_TEST(get_shared_wall_none)
{
    /* Given a room with points {{0, 30}, {0, 15}, {18, 15}, {18, 30}} and a second room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be 0 (none) */

    pb_point points1[] = { { 0.f, 30.f }, { 0.f, 15.f }, { 18.f, 15.f }, { 18.f, 30.f } };
    pb_point points2[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };

    pb_room r1;
    pb_room r2;

    int result;

    r1.room_shape.points = &points1[0];
    r1.room_shape.num_points = 4;

    r2.room_shape.points = &points2[0];
    r2.room_shape.num_points = 4;

    result = pb_sq_house_get_shared_wall(&r1, &r2);
    ck_assert_msg(result == -1, "result should have been -1 (none), was %d", result);
}
END_TEST

START_TEST(get_wall_overlap_top)
{
    /* Given a room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}}, a second room with points {{5, 30}, {5, 10}, {18, 10}, {18, 30}},
     * and TOP as the overlapping wall
     * When I invoke pb_sq_house_get_wall_overlap
     * start should contain {5, 10} and end should contain {10, 10} */

    pb_point points1[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };
    pb_point points2[] = { { 5.f, 30.f }, { 5.f, 10.f }, { 18.f, 10.f }, { 18.f, 30.f } };

    pb_point start;
    pb_point end;

    pb_point s_expected = points2[1];
    pb_point e_expected = points1[3];

    pb_room r1;
    pb_room r2;

    int result;

    r1.room_shape.points = &points1[0];
    r1.room_shape.num_points = 4;

    r2.room_shape.points = &points2[0];
    r2.room_shape.num_points = 4;

    pb_sq_house_get_wall_overlap(&r1, &r2, SQ_HOUSE_TOP, &start, &end);

    ck_assert_msg(start.x == s_expected.x && start.y == s_expected.y, "Start should have been {%.3f, %.3f}, was {%.3f, %.3f}", s_expected.x, s_expected.y, start.x, start.y);
    ck_assert_msg(end.x == e_expected.x && end.y == e_expected.y, "Start should have been {%.3f, %.3f}, was {%.3f, %.3f}", e_expected.x, e_expected.y, end.x, end.y);
}
END_TEST

START_TEST(get_wall_overlap_right)
{
    /* Given a room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}} a second room with points {{10, 15}, {10, 0}, {25, 0}, {25, 15}},
     * and RIGHT as the overlapping wall
     * When I invoke pb_sq_house_get_wall_overlap
     * start should contain {10, 0} and end should contain {10, 10} */

    pb_point points1[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };
    pb_point points2[] = { { 10.f, 15.f }, { 10.f, 0.f }, { 25.f, 0.f }, { 25.f, 15.f } };

    pb_point start;
    pb_point end;

    pb_point s_expected = points1[2];
    pb_point e_expected = points1[3];

    pb_room r1;
    pb_room r2;

    r1.room_shape.points = &points1[0];
    r1.room_shape.num_points = 4;

    r2.room_shape.points = &points2[0];
    r2.room_shape.num_points = 4;

    pb_sq_house_get_wall_overlap(&r1, &r2, SQ_HOUSE_RIGHT, &start, &end);

    ck_assert_msg(start.x == s_expected.x && start.y == s_expected.y, "Start should have been {%.3f, %.3f}, was {%.3f, %.3f}", s_expected.x, s_expected.y, start.x, start.y);
    ck_assert_msg(end.x == e_expected.x && end.y == e_expected.y, "Start should have been {%.3f, %.3f}, was {%.3f, %.3f}", e_expected.x, e_expected.y, end.x, end.y);
}
END_TEST

Suite *make_pb_sq_house_graph_suite(void)
{
    Suite* s;
    TCase* tc_sq_house_get_shared_wall;
    TCase* tc_sq_house_get_wall_overlap;

    s = suite_create("Squarified house generation");

    tc_sq_house_get_shared_wall = tcase_create("Get shared wall tests");
    suite_add_tcase(s, tc_sq_house_get_shared_wall);
    tcase_add_test(tc_sq_house_get_shared_wall, get_shared_wall_left);
    tcase_add_test(tc_sq_house_get_shared_wall, get_shared_wall_right);
    tcase_add_test(tc_sq_house_get_shared_wall, get_shared_wall_top);
    tcase_add_test(tc_sq_house_get_shared_wall, get_shared_wall_bottom);
    tcase_add_test(tc_sq_house_get_shared_wall, get_shared_wall_none);

    tc_sq_house_get_wall_overlap = tcase_create("Get shared wall overlap tests");
    suite_add_tcase(s, tc_sq_house_get_wall_overlap);
    tcase_add_test(tc_sq_house_get_wall_overlap, get_wall_overlap_top);
    tcase_add_test(tc_sq_house_get_wall_overlap, get_wall_overlap_right);

    return s;
}