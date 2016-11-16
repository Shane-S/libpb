#include "../test_util.h"
#include <pb/util/geom/types.h>
#include <pb/util/geom/triangulate.h>
#include <pb/util/vector/vector.h>
#include <pb/extrusion.h>

START_TEST(triangulate_get_num_tris)
{
    /* Input: shape with 5 vertices
     * Expected output: 3 (triangulation always has shape.points.size - 2 triangles) */

    size_t result;
    pb_shape2D fake;

    pb_shape2D_init(&fake, 5);
    fake.points.size = 5;
    result = pb_shape2D_get_num_tris(&fake);

    ck_assert_msg(result == 3, "shape with 5 vertices should have had 3 triangles, had %lu", result);

    pb_shape2D_free(&fake);
}
END_TEST


START_TEST(triangle_contains_inside)
{
    /* Input: A triangle with points {0, 0}, {1, 0}, {0.5, 1}, and point {0.5, 0.5}
     * Expected output: non-zero (point is inside triangle) */
    pb_point2D t0 = {0.f, 0.f};
    pb_point2D t1 = {1.f, 0.f};
    pb_point2D t2 = {0.5f, 1.f};
    pb_point2D p = {0.5f, 0.5f};

    ck_assert_msg(pb_tri_contains_point(&t0, &t1, &t2, &p), "Triangle should have contained {0.5, 0.5}");
}
END_TEST

START_TEST(triangle_contains_edge)
{
    /* Input: A triangle with points {0, 0}, {1, 0}, {0.5, 1}, and point {0.25, 0.5}
     * Expected output: non-zero (point is inside triangle) */
    pb_point2D t0 = {0.f, 0.f};
    pb_point2D t1 = {1.f, 0.f};
    pb_point2D t2 = {0.5f, 1.f};
    pb_point2D p = {0.25f, 0.5f};

    ck_assert_msg(pb_tri_contains_point(&t0, &t1, &t2, &p), "Triangle should have contained {0.25, 0.5}");
}
END_TEST

START_TEST(triangle_contains_corner)
{
    /* Input: A triangle with points {0, 0}, {1, 0}, {0.5, 1}, and point {1, 0}
     * Expected output: non-zero (point is inside triangle) */
    pb_point2D t0 = {0.f, 0.f};
    pb_point2D t1 = {1.f, 0.f};
    pb_point2D t2 = {0.5f, 1.f};
    pb_point2D p = {1.f, 0.f};

    ck_assert_msg(pb_tri_contains_point(&t0, &t1, &t2, &p), "Triangle should have contained {1, 0}");
}
END_TEST

START_TEST(triangle_contains_outside)
{
    /* Input: A triangle with points {0, 0}, {1, 0}, {0.5, 1}, and point {1, 1}
     * Expected output: 0 (point is outside triangle) */
    pb_point2D t0 = {0.f, 0.f};
    pb_point2D t1 = {1.f, 0.f};
    pb_point2D t2 = {0.5f, 1.f};
    pb_point2D p = {1.f, 1.f};

    ck_assert_msg(!pb_tri_contains_point(&t0, &t1, &t2, &p), "Triangle shouldn't have contained {1, 1}");
}
END_TEST

/* I didn't want these to be accessible to other stuff for the time being, so we need to forward-declare it */
extern int pb_earclip_is_convex(pb_point2D const* point, pb_point2D const* prev, pb_point2D const* next);
START_TEST(is_convex_convex)
{
    /* Input: prev is {1, 0}, point is {0.5, 1}, next is {0, 0}
     * Expected output: non-zero (point is convex) */
    pb_point2D point = {0.5f, 1.f};
    pb_point2D prev = {1.f, 0.f};
    pb_point2D next = {0.f, 0.f};
    ck_assert_msg(pb_earclip_is_convex(&point, &prev, &next), "point should have been convex");
}
END_TEST

START_TEST(is_convex_180_degrees)
{
    /* Input: prev is {1, 0}, point is {0.5, 0}, next is {0, 0}
     * Expected output: 0 (point is not convex; must be strictly < 180 degree angle) */
    pb_point2D point = {0.5f, 0.f};
    pb_point2D prev = {1.f, 0.f};
    pb_point2D next = {0.f, 0.f};
    ck_assert_msg(!pb_earclip_is_convex(&point, &prev, &next), "point shouldn't have been convex");
}
END_TEST

START_TEST(is_convex_reflex)
{
    /* Input: prev is {1, 0}, point is {0.5, -1.f}, next is {0, 0}
     * Expected output: 0 (point is actually reflex) */
    pb_point2D point = {0.5f, -1.f};
    pb_point2D prev = {1.f, 0.f};
    pb_point2D next = {0.f, 0.f};
    ck_assert_msg(!pb_earclip_is_convex(&point, &prev, &next), "point shouldn't have been convex");
}
END_TEST

/* Forward declarations etc. to get symbols in triangulate.o that aren't in the header */
static int CONVEX = 0;
static int REFLEX = 1;
static int EAR = 2;

typedef struct {
    int pt;
    size_t point_idx;
} pb_earclip_point;

int pb_earclip_is_ear(pb_vector const* list, size_t list_idx, pb_shape2D const* shape);

START_TEST(is_ear_simple)
{
    /* Input: Rectangle with points (0, 0), (2, 0), (2, 1), (0, 1)
     *        Vector of earclip_point: {pt = CONVEX, point_idx = 0}, {pt = CONVEX, point_idx = 1},
     *                                 {pt = CONVEX, point_idx = 2}, {pt = CONVEX, point_idx = 3}
     *        Index: 0
     *
     * Expectd output: non-zero (point is an ear)
     */

    pb_shape2D rect;
    pb_point2D rect_points[] = {{0, 0}, {2, 0}, {2, 1}, {0, 1}};
    char unused = 1;

    pb_vector earclip_list;
    pb_earclip_point earclip_points[] = {{CONVEX, 0}, {CONVEX, 1},
                                         {CONVEX, 2}, {CONVEX, 3}};

    unsigned i;

    pb_shape2D_init(&rect, 4);
    for(i = 0; i < 4; ++i) {
        pb_vector_push_back(&rect.points, &rect_points[i]);
        pb_vector_push_back(&rect.connected, &unused);
    }

    pb_vector_init(&earclip_list, sizeof(pb_earclip_point), 4);
    for(i = 0; i < 4; ++i) {
        pb_vector_push_back(&earclip_list, &earclip_points[i]);
    }

    ck_assert_msg(pb_earclip_is_ear(&earclip_list, 0, &rect), "item 0 should have been an ear.");

    pb_vector_free(&earclip_list);
    pb_shape2D_free(&rect);
}
END_TEST

START_TEST(is_ear_non_contained_reflex)
{
    /* Input: Shape as defined below (vertex (0, 0.8 is the first vertex proceeding CCW),
     *        Vector of earclip_point: {pt = CONVEX, point_idx = 0}, {pt = CONVEX, point_idx = 1},
     *                                 {pt = REFLEX, point_idx = 2}, {pt = CONVEX, point_idx = 3},
     *                                 {pt = CONVEX, point_idx = 4}, {pt = CONVEX, point_idx = 5}
     *        Index: 1
     *        Expected output: non-zero (is an ear)
     *
     *               (0.5, 1.5)
     *                   *
     *                  / \
     *                 /   * (0.7, 1.3)
     *                /   /
     *               /   /
     *              /   * (0.4, 1.0)
     *    (0, 0.8) *    \
     *              \    \
     *               \    \
     *                \    \
     *                 \    \
     *                  \    \
     *                   \    * (1.1, 0.3)
     *                    \   /
     *                     \ /
     *                      * (0.9, 0)
     */

    pb_shape2D shape;
    pb_point2D shape_points[] = {{0, 0.8f}, {0.9f, 0}, {1.1f, 0.3f}, {0.4f, 1.f}, {0.7f, 1.3f}, {0.5f, 1.5f}};
    char unused = 1;

    pb_vector earclip_list;
    pb_earclip_point earclip_points[] = {{CONVEX, 0}, {CONVEX, 1}, {CONVEX, 2},
                                         {REFLEX, 3}, {CONVEX, 4}, {CONVEX, 5}};

    unsigned i;

    pb_shape2D_init(&shape, 6);
    for(i = 0; i < 6; ++i) {
        pb_vector_push_back(&shape.points, &shape_points[i]);
        pb_vector_push_back(&shape.connected, &unused);
    }

    pb_vector_init(&earclip_list, sizeof(pb_earclip_point), 4);
    for(i = 0; i < 6; ++i) {
        pb_vector_push_back(&earclip_list, &earclip_points[i]);
    }

    ck_assert_msg(pb_earclip_is_ear(&earclip_list, 1, &shape), "item 1 should have been an ear.");

    pb_vector_free(&earclip_list);
    pb_shape2D_free(&shape);
}
END_TEST

START_TEST(is_ear_contained_reflex)
{
    /* Input: Same shape and point list as is_ear_non_contained_reflex; index is 0
     * Expected output: 0 (not an ear) */

    pb_shape2D shape;
    pb_point2D shape_points[] = {{0, 0.8f}, {0.9f, 0}, {1.1f, 0.3f}, {0.4f, 1.f}, {0.7f, 1.3f}, {0.5f, 1.5f}};
    char unused = 1;

    pb_vector earclip_list;
    pb_earclip_point earclip_points[] = {{CONVEX, 0}, {CONVEX, 1}, {CONVEX, 2},
                                         {REFLEX, 3}, {CONVEX, 4}, {CONVEX, 5}};

    unsigned i;

    pb_shape2D_init(&shape, 6);
    for(i = 0; i < 6; ++i) {
        pb_vector_push_back(&shape.points, &shape_points[i]);
        pb_vector_push_back(&shape.connected, &unused);
    }

    pb_vector_init(&earclip_list, sizeof(pb_earclip_point), 4);
    for(i = 0; i < 6; ++i) {
        pb_vector_push_back(&earclip_list, &earclip_points[i]);
    }

    ck_assert_msg(!pb_earclip_is_ear(&earclip_list, 0, &shape), "item 0 should not have been an ear.");

    pb_vector_free(&earclip_list);
    pb_shape2D_free(&shape);
}
END_TEST

START_TEST(triangulate_triangle)
{
    /* Input: Shape with three vertices (don't have to be populated)
     * Expected output: array with elements [0, 1, 2] */

    size_t expected[] = {0, 1, 2};
    pb_shape2D triangle;
    unsigned i;

    pb_shape2D_init(&triangle, 3);
    triangle.points.size = 3;

    /* We shouldn't actually look at the points in this case since we should return early */
    size_t* results;
    results = pb_triangulate(&triangle);

    for(i = 0; i < 3; ++i) {
        ck_assert_msg(expected[i] == results[i], "expected %lu was %lu, should have been %lu", i, results[i], expected[i]);
    }

    pb_shape2D_free(&triangle);
    free(results);
}
END_TEST

START_TEST(triangulate_convex_polygon)
{
    /* Input: rectangle with coordinates (0, 0), (1, 0), (1, 2), (0, 2)
     * Expected output: results array with elements [0, 1, 2, 0, 2, 3] */

    size_t expected[] = {0, 1, 2, 0, 2, 3};
    pb_shape2D rectangle;
    pb_point2D points[] = {{0.f, 0.f}, {1.f, 0.f}, {1.f, 2.f}, {0.f, 2.f}};
    unsigned i;
    size_t* results;
    pb_shape2D_init(&rectangle, 4);
    for(i = 0; i < 4; ++i) {
        pb_vector_push_back(&rectangle.points, &points[i]);
    }

    results = pb_triangulate(&rectangle);
    for(i = 0; i < 6; ++i) {
        ck_assert_msg(results[i] == expected[i], "results[%u] was %lu, should have been %lu", i, results[i], expected[i]);
    }

    pb_shape2D_free(&rectangle);
    free(results);
}
END_TEST

START_TEST(triangulate_simple_polygon)
{
    /* Input: Shape as defined in is_ear_non_contained_reflex
     * Expected output: triangle indices [{4, 5, 0}, {3, 4, 0}, {3, 0, 1}, {1, 2, 3}] */
    pb_shape2D shape;
    pb_point2D shape_points[] = {{0, 0.8f}, {0.9f, 0}, {1.1f, 0.3f}, {0.4f, 1.f}, {0.7f, 1.3f}, {0.5f, 1.5f}};
    char unused = 1;

    unsigned i;

    size_t* results;
    size_t expected[] = {4, 5, 0, 3, 4, 0, 3, 0, 1, 1, 2, 3};

    pb_shape2D_init(&shape, 6);
    for(i = 0; i < 6; ++i) {
        pb_vector_push_back(&shape.points, &shape_points[i]);
        pb_vector_push_back(&shape.connected, &unused);
    }

    results = pb_triangulate(&shape);

    for(i = 0; i < 4; i += 3) {
        ck_assert_msg(results[i * 3] == expected [i * 3],
                      "Triangle %lu index 0 should have been %lu, was %lu",
                      i, expected [i * 3], results[i * 3]);

        ck_assert_msg(results[i * 3 + 1] == expected [i * 3 + 1],
                      "Triangle %lu index 0 should have been %lu, was %lu",
                      i, expected [i * 3 + 1], results[i * 3 + 1]);
        ck_assert_msg(results[i * 3 + 2] == expected [i * 3 + 2],
                      "Triangle %lu index 0 should have been %lu, was %lu",
                      i, expected [i * 3 + 2], results[i * 3 + 2]);
    }

    pb_shape2D_free(&shape);
    free(results);
}
END_TEST

Suite* make_triangulate_suite(void) {
    Suite* s;
    TCase* tc_num_tris;
    TCase* tc_tri_contains;
    TCase* tc_is_convex;
    TCase* tc_is_ear;
    TCase* tc_triangulate;

    s = suite_create("libpb Triangulation");

    tc_num_tris = tcase_create("Determine number of triangles in polygon");
    suite_add_tcase(s, tc_num_tris);
    tcase_add_test(tc_num_tris, triangulate_get_num_tris);

    tc_tri_contains = tcase_create("Triangle point containment tests");
    suite_add_tcase(s, tc_tri_contains);
    tcase_add_test(tc_tri_contains, triangle_contains_inside);
    tcase_add_test(tc_tri_contains, triangle_contains_edge);
    tcase_add_test(tc_tri_contains, triangle_contains_corner);
    tcase_add_test(tc_tri_contains, triangle_contains_outside);

    tc_is_convex = tcase_create("Vertex convexness tests");
    suite_add_tcase(s, tc_is_convex);
    tcase_add_test(tc_is_convex, is_convex_convex);
    tcase_add_test(tc_is_convex, is_convex_180_degrees);
    tcase_add_test(tc_is_convex, is_convex_reflex);

    tc_is_ear = tcase_create("Vertex earness tests");
    suite_add_tcase(s, tc_is_ear);
    tcase_add_test(tc_is_ear, is_ear_simple);
    tcase_add_test(tc_is_ear, is_ear_non_contained_reflex);
    tcase_add_test(tc_is_ear, is_ear_contained_reflex);

    tc_triangulate = tcase_create("Triangulation tests");
    suite_add_tcase(s, tc_triangulate);
    tcase_add_test(tc_triangulate, triangulate_triangle);
    tcase_add_test(tc_triangulate, triangulate_convex_polygon);
    tcase_add_test(tc_triangulate, triangulate_simple_polygon);

    return s;
}