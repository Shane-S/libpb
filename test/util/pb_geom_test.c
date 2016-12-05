#include "../test_util.h"
#include <pb/util/geom/types.h>
#include <pb/util/geom/rect_utils.h>

START_TEST(rect_to_shape)
{
    pb_shape2D shape;
    pb_rect rect;
    pb_point2D expected[] = {
        {0.f, 10.f},
        {0.f, 0.f},
        {20.f, 0.f},
        {20.f, 10.f}
    };
    pb_point2D* points;
    char* connected;
    int i;

    rect.bottom_left.x = 0;
    rect.bottom_left.y = 0;
    rect.h = 10;
    rect.w = 20;

    pb_rect_to_pb_shape2D(&rect, &shape);
    ck_assert_msg(shape.points.size == 4, "Shape should have had 4 points, instead had %u points", shape.points.size);
    ck_assert_msg(shape.connected.size == 4, "Shape should have had 4 connected entries, instead had %u", shape.connected.size);

    points = (pb_point2D*)shape.points.items;
    connected = (char*)shape.connected.items;
    for (i = 0; i < 4; ++i) {
        ck_assert_msg(points[i].x == expected[i].x && points[i].y == expected[i].y,
            "Point %d should have been {%f, %f}, was {%f, %f}",
            i, expected[i].x, expected[i].y, points[i].x, points[i].y);

        ck_assert_msg(connected[i] == 1, "Connected[%d] should have been 1, was %c", i, connected[i]);
    }

    pb_shape2D_free(&shape);
}
END_TEST

START_TEST(shape_to_rect_basic) {
    pb_shape2D big_old_rectangle;
    pb_rect out;
    pb_point2D* points;

    pb_shape2D_init(&big_old_rectangle, 4);
    big_old_rectangle.points.size = 4;
    points = (pb_point2D*)big_old_rectangle.points.items;

    points[0].x = 10;
    points[0].y = 100;

    points[1].x = 10;
    points[1].y = 0;

    points[2].x = 120;
    points[2].y = 0;

    points[3].x = 120;
    points[3].y = 100;

    pb_shape2D_to_pb_rect(&big_old_rectangle, &out);
    ck_assert_msg(out.bottom_left.x == 10 && out.bottom_left.y == 0, "Rectangle's bottom-left should have been {10.0, 100.0}, was {%f, %f}", out.bottom_left.x, out.bottom_left.y);
    ck_assert_msg(out.w == 110, "Width should have been 110, was %u", out.w);
    ck_assert_msg(out.h == 100, "Height should have been 100, was %u", out.h);

    pb_shape2D_free(&big_old_rectangle);
}
END_TEST

START_TEST(shape_to_rect_bad_shape) {
    pb_shape2D fake;
    pb_rect out_fake;

    fake.points.size = 1;
    ck_assert_msg(pb_shape2D_to_pb_rect(&fake, &out_fake) == 0, "pb_shape2D_to_rect should have failed with 3-point shape but succeeded.");
}
END_TEST

Suite *make_pb_geom_suite(void) {
    Suite *s;
    TCase *tc_pb_rect_conversion;

    s = suite_create("libpb Geometry");

    tc_pb_rect_conversion = tcase_create("Conversion between pb_shape2D and pb_rect");
    suite_add_tcase(s, tc_pb_rect_conversion);
    tcase_add_test(tc_pb_rect_conversion, rect_to_shape);
    tcase_add_test(tc_pb_rect_conversion, shape_to_rect_basic);
    tcase_add_test(tc_pb_rect_conversion, shape_to_rect_bad_shape);

    return s;
}