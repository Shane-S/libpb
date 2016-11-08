#include "../test_util.h"
#include <check.h>
#include <pb/internal/pb_squarify.h>

START_TEST(simple_test)
{
	pb_rect container = { { 0.f, 0.f }, 1.f, 1.f };
    float areas[] = { 1.f };
    pb_rect children[1] = { 0.f };
    pb_rect* last_row_start;
    size_t last_row_size;
    int rect_has_children;

    pb_squarify(&container, areas, 1, children, &last_row_start, &last_row_size, &rect_has_children);
	ck_assert_msg(children[0].w == 1.f && children[0].h == 1.f, "First child's width and height should have been 1, were %f and %f", children[0].w, children[0].h);
}
END_TEST

START_TEST(paper_test) /* The example dimensions given in the paper */
{
	pb_rect container = { { 0.f, 0.f }, 6.f, 4.f };
    float areas[] = { 6.f, 6.f, 4.f, 3.f, 2.f, 2.f, 1.f };
	pb_rect children[7] = { 0.f };

	pb_rect expected[] = {
		{ { 0.f, 0.f }, 3.f, 2.f },
		{ { 0.f, 2.f }, 3.f, 2.f },
		{ { 3.f, 0.f }, 12.f / 7.f, 7.f / 3.f },
		{ { 33.f / 7.f, 0.f }, 9.f / 7.f, 7.f / 3.f },
		{ { 3.f, 7.f / 3.f }, 6.f / 5.f, 5.f / 3.f },
		{ { 21.f / 5.f, 7.f / 3.f }, 6.f / 5.f, 5.f / 3.f },
		{ { 27.f / 5.f, 7.f / 3.f }, 3.f / 5.f, 5.f / 3.f }
	};
	size_t num_children = 7;
    size_t i;
    pb_rect* last_row_start;
    size_t last_row_size;
    int rect_has_children;
	
    pb_squarify(&container, areas, 7, children, &last_row_start, &last_row_size, &rect_has_children);
	for (i = 0; i < num_children; i++) {
		int equal;

        equal = assert_close_enough(children[i].bottom_left.x, expected[i].bottom_left.x, 5);
        equal = equal && assert_close_enough(children[i].bottom_left.y, expected[i].bottom_left.y, 5);
		equal = equal && assert_close_enough(children[i].w, expected[i].w, 5);
		equal = equal && assert_close_enough(children[i].h, expected[i].h, 5);

		ck_assert_msg(equal, "Incorrect dimensions/position: expected (%f, %f), w %f, h %f; got (%f, %f), w %f, h %f",
            expected[i].bottom_left.x, expected[i].bottom_left.y, expected[i].w, expected[i].h,
            children[i].bottom_left.x, children[i].bottom_left.y, children[i].w, children[i].h);
	}
}
END_TEST

START_TEST(uniform_test)
{
	/* We're giving a bunch of rectangles with area 1, and the parent rectangle should fit them all
	 * perfectly as a uniform grid of 1x1 squares */
	pb_rect container = { { 0.f, 0.f }, 2.f, 3.f };
    float areas[6] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
	pb_rect children[6] = { 0.f };

	size_t num_children = 6;
	size_t i;
	float min_dim = 2.f;
    pb_rect* last_row_start;
    size_t last_row_size;
    int rect_has_children;

    pb_squarify(&container, areas, 6, children, &last_row_start, &last_row_size, &rect_has_children);
	for (i = 0; i < num_children; i++) {
		int equal = children[i].w == 1 && children[i].h == 1;

		ck_assert_msg(equal, "Expected width and height of 1 for all children, got w %f, h %f", children[i].w, children[i].h);
	}
}
END_TEST

Suite *make_pb_squarify_suite(void)
{

	Suite *s;
	TCase *tc_squarify;

	s = suite_create("Squarify");

	tc_squarify = tcase_create("Squarify correct output");
	suite_add_tcase(s, tc_squarify);
	tcase_add_test(tc_squarify, simple_test);
	tcase_add_test(tc_squarify, paper_test);
	tcase_add_test(tc_squarify, uniform_test);
	return s;
}