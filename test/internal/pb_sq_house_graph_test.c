#include <libcompat.h>
#include <check.h>
#include <pb/pb_sq_house.h>
#include <pb/internal/pb_sq_house_graph.h>
#include <pb/util/pb_hash_utils.h>

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

START_TEST(generate_floor_graph_multi_room)
{
    pb_graph* result;

    /* A room spec consists of
    *  char const* name;
    *  char const** adjacent;
    *  size_t num_adjacent;
    *  float area;
    *  unsigned int max_instances;
    *  unsigned priority;
    */
    char const* adj_lists[][4] = {
        { "Kitchen", "Bedroom", "Bathroom", PB_SQ_HOUSE_OUTSIDE },
        { "Living Room", NULL, NULL, NULL },
        { "Living Room", NULL, NULL, NULL },
        { "Living Room", NULL, NULL, NULL }
    };
    pb_sq_house_room_spec specs[4] = {
        { "Living Room", &adj_lists[0], 4, 11.f, 1, 0 },
        { "Bedroom", &adj_lists[1], 1, 5.f, 2, 1 },
        { "Bathroom", &adj_lists[2], 1, 4.f, 3, 3 },
        { "Kitchen", &adj_lists[3], 1, 7.f, 1, 2 },
    };
    pb_hash* room_specs = pb_hash_create(pb_str_hash, pb_str_eq);

    char const* room_names[] = { "Living Room", "Bedroom", "Bathroom", "Kitchen", "Bathroom", "Bedroom" };

    /* The set of rectangles that these rooms will occupy (as output from pb_squarify using 6x6 outer rect) */
    pb_rect rects[] = {
        { { 0.f, 0.f }, 4.125f, 8.f / 3.f },
        { { 4.125f, 0.f }, 1.875f, 8.f / 3.f },
        { { 0.f, 8.f / 3.f }, 3.30000019f, 1.21212113f },
        { { 0.f, 3.87878799f }, 3.30000019f, 2.12121201f },
        { { 3.30000019f, 8.f / 3.f }, 2.69999981f, 1.48148155f },
        { { 3.30000019f, 4.14814854f }, 2.70000029f, 1.85185170f }
    };
    pb_shape shapes[6];

    pb_room rooms[6];
    pb_floor f;

    /* Connection format: 
     * pb_room* neighbour;
     * pb_point overlap_start;
     * pb_point overlap_end;
     * side wall;
     * int door;
     */
    pb_sq_house_room_conn conns[] = {
        { &rooms[1], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_RIGHT, 1 },
        { &rooms[2], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_TOP, 1 },
        { &rooms[4], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_TOP, 1 },
    };

    /* Create the hash map of room specs */
    unsigned i;
    for (i = 0; i < 4; ++i) {
        pb_hash_put(room_specs, (void*)specs[i].name, (void*)&specs[i]);
    }

    /* Populate the floor's rooms */
    for (i = 0; i < 6; ++i) {
        pb_rect_to_pb_shape(&rects[i], &shapes[i]);
        rooms[i].room_shape = shapes[i];
        rooms[i].data = room_names[i];
    }

    f.num_rooms = 6;
    f.rooms = &rooms[0];

    /* Generate the floor graph for this floor */
    result = pb_sq_house_generate_floor_graph(room_specs, &f);

    /* Check the living room connections as a sanity check */
    /* I'm not checking the overlap points because we've already shown that to be working in the get_overlap tests */
    for (i = 0; i < 3; ++i) {
        pb_edge const* edge = pb_graph_get_edge(result, &rooms[0], conns[i].neighbour);
        pb_sq_house_room_conn* c;
        ck_assert_msg(edge != NULL, "Edge from Living Room to %s didn't exist.", (char*)(conns[i].neighbour->data));
        
        c = (pb_sq_house_room_conn*)edge->data;
        ck_assert_msg(c->door == conns[i].door, "Edge from Living Room to %s should have had a door but did not.", (char*)(conns[i].neighbour->data));
    }

    /* Free the connections */
    /* I should really make some sort of "Apply function to all full hash entries" function */
    for (i = 0; i < result->edges->cap; ++i) {
        if (result->edges->states[i] == FULL) {
            free(((pb_edge*)result->edges->entries[i].val)->data);
        }
    }

    /* Free the shapes we created */
    for (i = 0; i < 6; ++i) {
        pb_shape_free(&rooms[i].room_shape);
    }

    /* Free the generated graph and the room specs hash map */
    pb_graph_free(result);
    pb_hash_free(room_specs);

    /* Die a little inside */
}
END_TEST

Suite *make_pb_sq_house_graph_suite(void)
{
    Suite* s;
    TCase* tc_sq_house_get_shared_wall;
    TCase* tc_sq_house_get_wall_overlap;
    TCase* tc_sq_house_generate_floor_graph;

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

    tc_sq_house_generate_floor_graph = tcase_create("Floor graph creation tests");
    suite_add_tcase(s, tc_sq_house_generate_floor_graph);
    tcase_add_test(tc_sq_house_generate_floor_graph, generate_floor_graph_multi_room);

    return s;
}