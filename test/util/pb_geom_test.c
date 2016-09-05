#include <libcompat.h>
#include <check.h>
#include <pb/util/pb_geom.h>


/* We're going to assume that pb_graph_free and pb_graph_create work.
* If not, then we'll find out later while profiling memory usage :) */

/* TODO: One day, we should probably just use a fixture for the graph. Since I don't currently feel like doing that, I won't. */
START_TEST(rect_to_shape)
{
    pb_shape shape;
    pb_rect rect;
    pb_point expected[] = {
        {0.f, 10.f},
        {0.f, 0.f},
        {20.f, 0.f},
        {20.f, 10.f}
    };
    int i;

    rect.bottom_left.x = 0;
    rect.bottom_left.y = 0;
    rect.h = 10;
    rect.w = 20;

    pb_rect_to_pb_shape(&rect, &shape);
    ck_assert_msg(shape.num_points == 4, "Shape should have had 4 points, instead had %u points", shape.num_points);

    for (i = 0; i < 4; ++i) {
        ck_assert_msg(shape.points[i].x == expected[i].x && shape.points[i].y == expected[i].y,
                      "Point %d should have been {%f, %f}, was {%f, %f}", i, expected[i].x, expected[i].y, shape.points[i].x, shape.points[i].y);
    }
}
END_TEST

START_TEST(shape_to_rect_basic) {
    pb_shape big_old_rectangle;
    pb_rect out;

    big_old_rectangle.points = malloc(sizeof(pb_point) * 4);
    big_old_rectangle.num_points = 4;
    big_old_rectangle.connected = 1;

    big_old_rectangle.points[0].x = 10;
    big_old_rectangle.points[0].y = 100;

    big_old_rectangle.points[1].x = 10;
    big_old_rectangle.points[1].y = 0;

    big_old_rectangle.points[2].x = 120;
    big_old_rectangle.points[2].y = 0;

    big_old_rectangle.points[3].x = 120;
    big_old_rectangle.points[3].y = 100;

    pb_shape_to_pb_rect(&big_old_rectangle, &out);
    ck_assert_msg(out.bottom_left.x == 10 && out.bottom_left.y == 0, "Rectangle's top-left should have been {%10.0, %100.0}, was {%f, %f}", out.bottom_left.x, out.bottom_left.y);
    ck_assert_msg(out.w == 110, "Width should have been 110, was %u", out.w);
    ck_assert_msg(out.h == 100, "Height should have been 100, was %u", out.h);

    free(big_old_rectangle.points);
}
END_TEST

START_TEST(shape_to_rect_bad_shape) {
    pb_shape fake;
    pb_rect out_fake;

    fake.num_points = 3;
    ck_assert_msg(pb_shape_to_pb_rect(&fake, &out_fake) == 0, "pb_shape_to_rect should have failed with 3-point shape but succeeded.");
}
END_TEST

Suite *make_pb_geom_suite(void) {
    /* Life test case tests lifetime events (create and destroy);
    * Adjacency test case tests all functions related to the adjacency list
    */
    Suite *s;
    TCase *tc_pb_rect_conversion;

    s = suite_create("libpb Geometry");

    tc_pb_rect_conversion = tcase_create("Conversion between pb_shape and pb_rect");
    suite_add_tcase(s, tc_pb_rect_conversion);
    tcase_add_test(tc_pb_rect_conversion, rect_to_shape);
    tcase_add_test(tc_pb_rect_conversion, shape_to_rect_basic);
    tcase_add_test(tc_pb_rect_conversion, shape_to_rect_bad_shape);

    return s;
}