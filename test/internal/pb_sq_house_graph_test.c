#include "../test_util.h"
#include <check.h>
#include <pb/sq_house.h>
#include <pb/internal/sq_house_graph.h>
#include <pb/util/hashmap/hash_utils.h>
#include <pb/util/geom/rect_utils.h>
#include <pb/floor_plan.h>
#include <pb/util/pair/pair.h>
#include <pb/util/graph/graph.h>
#include <pb/util/float_utils.h>

START_TEST(get_shared_wall_right)
{
    /* Given a room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}} and a second room with points {{10, 15}, {10, 0}, {25, 0}, {25, 15}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be RIGHT
     */
    pb_point2D points1[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };
    pb_point2D points2[] = { { 10.f, 15.f }, { 10.f, 0.f }, { 25.f, 0.f }, { 25.f, 15.f } };

    pb_room r1;
    pb_room r2;

    int result;
    
    r1.shape.points.items = &points1[0];
    r1.shape.points.size = 4;

    r2.shape.points.items = &points2[0];
    r2.shape.points.size = 4;

    result = pb_sq_house_get_shared_wall(&r1, &r2);
    ck_assert_msg(result == SQ_HOUSE_RIGHT, "result should have been 2 (right), was %d", result);
}
END_TEST

START_TEST(get_shared_wall_left)
{
    /* Given a room with points {{10, 15}, {10, 0}, {25, 0}, {25, 15}} and a second room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be LEFT */

    pb_point2D points1[] = { { 10.f, 15.f }, { 10.f, 0.f }, { 25.f, 0.f }, { 25.f, 15.f } };
    pb_point2D points2[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };

    pb_room r1;
    pb_room r2;

    int result;

    r1.shape.points.items = &points1[0];
    r1.shape.points.size = 4;

    r2.shape.points.items = &points2[0];
    r2.shape.points.size = 4;

    result = pb_sq_house_get_shared_wall(&r1, &r2);
    ck_assert_msg(result == SQ_HOUSE_LEFT, "result should have been 3 (left), was %d", result);
}
END_TEST

START_TEST(get_shared_wall_top)
{
    /* Given a room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}} and a second room with points {{0, 30}, {0, 10}, {18, 10}, {18, 30}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be TOP */

    pb_point2D points1[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };
    pb_point2D points2[] = { { 0.f, 30.f }, { 0.f, 10.f }, { 18.f, 10.f }, { 18.f, 30.f } };

    pb_room r1;
    pb_room r2;

    int result;

    r1.shape.points.items = &points1[0];
    r1.shape.points.size = 4;

    r2.shape.points.items = &points2[0];
    r2.shape.points.size = 4;

    result = pb_sq_house_get_shared_wall(&r1, &r2);
    ck_assert_msg(result == SQ_HOUSE_TOP, "result should have been 1 (top), was %d", result);
}
END_TEST

START_TEST(get_shared_wall_bottom)
{
    /* Given a room with points {{0, 30}, {0, 10}, {18, 10}, {18, 30}} and a second room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be BOTTOM */

    pb_point2D points1[] = { { 0.f, 30.f }, { 0.f, 10.f }, { 18.f, 10.f }, { 18.f, 30.f } };
    pb_point2D points2[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };

    pb_room r1;
    pb_room r2;

    int result;

    r1.shape.points.items = &points1[0];
    r1.shape.points.size = 4;

    r2.shape.points.items = &points2[0];
    r2.shape.points.size = 4;

    result = pb_sq_house_get_shared_wall(&r1, &r2);
    ck_assert_msg(result == SQ_HOUSE_BOTTOM, "result should have been 4 (bottom), was %d", result);
}
END_TEST

START_TEST(get_shared_wall_none)
{
    /* Given a room with points {{0, 30}, {0, 15}, {18, 15}, {18, 30}} and a second room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}}
     * When I invoke pb_sq_house_get_shared_wall
     * The result should be 0 (none) */

    pb_point2D points1[] = { { 0.f, 30.f }, { 0.f, 15.f }, { 18.f, 15.f }, { 18.f, 30.f } };
    pb_point2D points2[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };

    pb_room r1;
    pb_room r2;

    int result;

    r1.shape.points.items = &points1[0];
    r1.shape.points.size = 4;

    r2.shape.points.items = &points2[0];
    r2.shape.points.size = 4;

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

    pb_point2D points1[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };
    pb_point2D points2[] = { { 5.f, 30.f }, { 5.f, 10.f }, { 18.f, 10.f }, { 18.f, 30.f } };

    pb_point2D start;
    pb_point2D end;

    pb_point2D s_expected = points2[1];
    pb_point2D e_expected = points1[3];

    pb_room r1;
    pb_room r2;

    r1.shape.points.items = &points1[0];
    r1.shape.points.size = 4;

    r2.shape.points.items = &points2[0];
    r2.shape.points.size = 4;

    pb_sq_house_get_wall_overlap(&r1, &r2, SQ_HOUSE_TOP, &start, &end);

    ck_assert_msg(start.x == s_expected.x && start.y == s_expected.y,
                  "Start should have been {%.3f, %.3f}, was {%.3f, %.3f}",
                  s_expected.x, s_expected.y, start.x, start.y);
    ck_assert_msg(end.x == e_expected.x && end.y == e_expected.y,
                  "Start should have been {%.3f, %.3f}, was {%.3f, %.3f}",
                  e_expected.x, e_expected.y, end.x, end.y);
}
END_TEST

START_TEST(get_wall_overlap_right)
{
    /* Given a room with points {{0, 10}, {0, 0}, {10, 0}, {10, 10}} a second room with points {{10, 15}, {10, 0}, {25, 0}, {25, 15}},
     * and RIGHT as the overlapping wall
     * When I invoke pb_sq_house_get_wall_overlap
     * start should contain {10, 0} and end should contain {10, 10} */

    pb_point2D points1[] = { { 0.f, 10.f }, { 0.f, 0.f }, { 10.f, 0.f }, { 10.f, 10.f } };
    pb_point2D points2[] = { { 10.f, 15.f }, { 10.f, 0.f }, { 25.f, 0.f }, { 25.f, 15.f } };

    pb_point2D start;
    pb_point2D end;

    pb_point2D s_expected = points1[2];
    pb_point2D e_expected = points1[3];

    pb_room r1;
    pb_room r2;

    r1.shape.points.items = &points1[0];
    r1.shape.points.size = 4;

    r2.shape.points.items = &points2[0];
    r2.shape.points.size = 4;

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
    pb_hashmap* room_specs = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec h_spec;

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
    pb_shape2D shapes[6];

    pb_room rooms[6];
    pb_floor f;

    /* Connection format: 
     * pb_room* neighbour;
     * pb_shape2D overlap_start;
     * pb_shape2D overlap_end;
     * side wall;
     * int can_connect;
     */
    pb_sq_house_room_conn conns[] = {
        { &rooms[0], &rooms[1], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_RIGHT, 1 },
        { &rooms[0], &rooms[2], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_TOP, 1 },
        { &rooms[0], &rooms[4], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_TOP, 1 },
    };

    /* Create the hash map of room specs */
    unsigned i;
    for (i = 0; i < 4; ++i) {
        pb_hashmap_put(room_specs, (void*)specs[i].name, (void*)&specs[i]);
    }

    /* Populate the floor's rooms */
    for (i = 0; i < 6; ++i) {
        pb_rect_to_pb_shape2D(&rects[i], &shapes[i]);
        rooms[i].shape = shapes[i];
        rooms[i].data = room_names[i];
    }

    f.num_rooms = 6;
    f.rooms = &rooms[0];

    h_spec.door_size = 0.5f;

    /* Generate the floor graph for this floor */
    result = pb_sq_house_generate_floor_graph(&h_spec, room_specs, &f);

    /* Check the living room connections as a sanity check */
    /* I'm not checking the overlap points because we've already shown that to be working in the get_overlap tests */
    for (i = 0; i < 3; ++i) {
        pb_edge const* edge = pb_graph_get_edge(result, &rooms[0], conns[i].neighbour);
        pb_sq_house_room_conn* c;
        ck_assert_msg(edge != NULL, "Edge from Living Room to %s didn't exist.", (char*)(conns[i].neighbour->data));
        
        c = (pb_sq_house_room_conn*)edge->data;
        ck_assert_msg(c->can_connect == conns[i].can_connect,
                      "Edge from Living Room to %s should have had a door but did not.",
                      (char*)(conns[i].neighbour->data));
    }

    /* Free the connections */
    pb_graph_for_each_edge(result, pb_graph_free_edge_data, NULL);

    /* Free the shapes we created */
    for (i = 0; i < 6; ++i) {
        pb_shape2D_free(&rooms[i].shape);
    }

    /* Free the generated graph and the room specs hash map */
    pb_graph_free(result);
    pb_hashmap_free(room_specs);

    /* Die a little inside */
}
END_TEST

START_TEST(generate_floor_graph_no_door_space)
{
    pb_graph* result;

    char const* room_name = "Room";
    char const* adj_lists[]= { room_name };
    pb_sq_house_room_spec specs[] = {
            { room_name, &adj_lists[0], 2, 11.f, 1, 0 },
    };
    pb_hashmap* room_specs = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec h_spec;

    /* Make two rooms that only have 0.2 overlapping space */
    pb_rect rect0 = {{0.f, 0.f}, 5.f, 5.f};
    pb_rect rect1 = {{5.f, 4.8f}, 5.f, 5.f};

    pb_shape2D shape0;
    pb_shape2D shape1;

    pb_room rooms[2];
    pb_shape2D shapes[2];

    pb_floor f;

    pb_sq_house_room_conn conns[] = {
            { &rooms[0], &rooms[1], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_RIGHT, 1 },
            { &rooms[0], &rooms[2], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_TOP, 1 },
            { &rooms[0], &rooms[4], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_TOP, 1 },
    };

    unsigned i;

    pb_rect_to_pb_shape2D(&rect0, &shape0);
    pb_rect_to_pb_shape2D(&rect1, &shape1);
    pb_hashmap_put(room_specs, (void*)room_name, (void*)&specs[0]);

    rooms[0].shape = shape0;
    rooms[0].data = (void*)room_name;

    rooms[1].shape = shape1;
    rooms[1].data = (void*)room_name;

    f.num_rooms = 2;
    f.rooms = &rooms[0];

    h_spec.door_size = 0.5f;

    result = pb_sq_house_generate_floor_graph(&h_spec, room_specs, &f);
    {
        pb_sq_house_room_conn* conn;
        pb_edge const* edge = pb_graph_get_edge(result, &rooms[0], &rooms[1]);
        ck_assert_msg(edge, "Edge should have existed between rooms 0 and 1");

        conn = (pb_sq_house_room_conn*)edge->data;
        ck_assert_msg(conn->has_door == 0, "There should not have been a connection between rooms 0 and 1");

        edge = pb_graph_get_edge(result, &rooms[1], &rooms[0]);
        ck_assert_msg(edge, "Edge should have existed between rooms 1 and 0");

        conn = (pb_sq_house_room_conn*)edge->data;
        ck_assert_msg(conn->has_door == 0, "There should not have been a connection between rooms 1 and 0.");
    }

    pb_shape2D_free(&shape0);
    pb_shape2D_free(&shape1);

    pb_graph_for_each_edge(result, pb_graph_free_edge_data, NULL);
    pb_graph_free(result);
    pb_hashmap_free(room_specs);

}
END_TEST

START_TEST(find_disconnected_rooms_basic)
{
    /* Given a pb_floor with three rooms and a pb_graph holding the floor connectivity graph with a connection between rooms 0 and 1
     * When I invoke pb_sq_house_find_disconnected_rooms(graph, floor)
     * Then the result should be a pb_hashmap containing a pointer to floor.rooms[2] */

    pb_room fake_rooms[3] = { 0 }; /* We just need the addresses; don't need to populate this at all */
    pb_floor fake_floor;
    pb_graph* floor_graph = pb_graph_create(pb_pointer_hash, pb_pointer_eq);
    pb_sq_house_room_conn conns[] = {
        { &fake_rooms[0], &fake_rooms[1], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_RIGHT, 1, 1 },
        { &fake_rooms[1], &fake_rooms[0], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_LEFT, 1, 1 },
        { &fake_rooms[0], &fake_rooms[2], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_RIGHT, 0, 0 },
        { &fake_rooms[2], &fake_rooms[0], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_LEFT, 0, 0 },
    };
    pb_hashmap* result;
    pb_room* room;

    unsigned i;
    for (i = 0; i < 3; ++i) {
        pb_graph_add_vertex(floor_graph, &fake_rooms[i], &fake_rooms[i]);
    }

    fake_floor.num_rooms = 3;
    fake_floor.rooms = &fake_rooms[0];

    /* Add the fake edges */
    pb_graph_add_edge(floor_graph, &fake_rooms[0], &fake_rooms[1], 0.f, &conns[0]);
    pb_graph_add_edge(floor_graph, &fake_rooms[1], &fake_rooms[0], 0.f, &conns[1]);
    pb_graph_add_edge(floor_graph, &fake_rooms[0], &fake_rooms[2], 0.f, &conns[2]);
    pb_graph_add_edge(floor_graph, &fake_rooms[2], &fake_rooms[0], 0.f, &conns[3]);

    result = pb_sq_house_find_disconnected_rooms(floor_graph, &fake_floor);
    ck_assert_msg(result->size == 1, "result should contain one element, has %lu", result->size);

    pb_hashmap_get(result, &fake_rooms[2], (void**)&room);
    ck_assert_msg(room = &fake_rooms[2], "result should have contained &fake_rooms[2], had %p", room);

    pb_hashmap_free(result);
    pb_graph_free(floor_graph);
}
END_TEST

START_TEST(find_disconnected_rooms_one_sided_connection)
{
    /* Given a pb_floor with two rooms and a pb_graph holding the floor connectivity graph with can_connect from room 0 to 1 but not 1 to 0
     * When I invoke pb_sq_house_find_disconnected_rooms(graph, floor)
     * Then the result should be a pb_hashmap with size 0, and the connection from 1 to 0 should have can_connect == 1*/

    pb_room fake_rooms[2] = { 0 }; /* We just need the addresses; don't need to populate this at all */
    pb_floor fake_floor;
    pb_graph* floor_graph = pb_graph_create(pb_pointer_hash, pb_pointer_eq);
    pb_sq_house_room_conn conns[] = {
        { &fake_rooms[0], &fake_rooms[1], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_RIGHT, 1, 1 },
        { &fake_rooms[1], &fake_rooms[0], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_LEFT, 0, 0 }
    };
    pb_hashmap* result;

    unsigned i;
    for (i = 0; i < 2; ++i) {
        pb_graph_add_vertex(floor_graph, &fake_rooms[i], &fake_rooms[i]);
    }

    fake_floor.num_rooms = 2;
    fake_floor.rooms = &fake_rooms[0];

    /* Add the fake edges */
    pb_graph_add_edge(floor_graph, &fake_rooms[0], &fake_rooms[1], 0.f, &conns[0]);
    pb_graph_add_edge(floor_graph, &fake_rooms[1], &fake_rooms[0], 0.f, &conns[1]);

    result = pb_sq_house_find_disconnected_rooms(floor_graph, &fake_floor);
    ck_assert_msg(result->size == 0, "result should contain 0 elements, has %lu", result->size);
    ck_assert_msg(conns[1].can_connect == 1, "conns[1].can_connect should have been set to 1, was %d", conns[1].can_connect);

    pb_hashmap_free(result);
    pb_graph_free(floor_graph);
}
END_TEST

START_TEST(find_disconnected_rooms_outside_single_disconnected)
{
    /* Given a pb_floor with a single room and a pb_graph holding the floor connectivity graph
     * When I invoke pb_sq_house_find_disconnected_rooms(graph, floor)
     * Then the result should be a pb_hashmap with size 1 */

    pb_room fake_rooms[1] = { 0 }; /* We just need the addresses; don't need to populate this at all */
    pb_floor fake_floor;
    pb_graph* floor_graph = pb_graph_create(pb_pointer_hash, pb_pointer_eq);
    pb_hashmap* result;

    pb_graph_add_vertex(floor_graph, &fake_rooms[0], &fake_rooms[0]);

    fake_floor.num_rooms = 1;
    fake_floor.rooms = &fake_rooms[0];

    result = pb_sq_house_find_disconnected_rooms(floor_graph, &fake_floor);
    ck_assert_msg(result->size == 1, "result should contain 1 element, has %lu", result->size);

    pb_hashmap_free(result);
    pb_graph_free(floor_graph);
}
END_TEST

START_TEST(find_disconnected_rooms_outside_multi_disconnected)
    {
        /* Given a pb_floor with a two rooms and a pb_graph holding the floor connectivity graph
         * When I invoke pb_sq_house_find_disconnected_rooms(graph, floor)
         * Then the result should be a pb_hashmap with size 1 containing the room that doesn't connect to outside */

        pb_room fake_rooms[2] = { 0 }; /* We just need the addresses; don't need to populate this at all */
        pb_floor fake_floor;
        pb_graph* floor_graph = pb_graph_create(pb_pointer_hash, pb_pointer_eq);
        pb_sq_house_room_conn conns[2] = { { &fake_rooms[0], &fake_rooms[1], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_RIGHT, 0 },
                                           { &fake_rooms[1], &fake_rooms[0], { 0.f, 0.f }, { 0.f, 0.f }, SQ_HOUSE_LEFT, 0 } };
        pb_hashmap* result;

        unsigned i;
        for (i = 0; i < 2; ++i) {
            pb_graph_add_vertex(floor_graph, &fake_rooms[i], &fake_rooms[i]);
        }

        fake_floor.num_rooms = 2;
        fake_floor.rooms = &fake_rooms[0];

        /* Add the fake edges */
        pb_graph_add_edge(floor_graph, &fake_rooms[0], &fake_rooms[1], 0.f, &conns[0]);
        pb_graph_add_edge(floor_graph, &fake_rooms[1], &fake_rooms[0], 0.f, &conns[1]);

        result = pb_sq_house_find_disconnected_rooms(floor_graph, &fake_floor);
        ck_assert_msg(result->size == 1, "result should contain 1 element, has %lu", result->size);
        ck_assert_msg(pb_hashmap_get(result, &fake_rooms[0], (void**)&fake_rooms[0]) == -1, "floor.rooms[0] shouldn't have been in result set.");
        pb_hashmap_free(result);
        pb_graph_free(floor_graph);
    }
END_TEST

START_TEST(internal_graph_simple)
{
    /* Input: graph representing 10x10 house evenly divided into 4 rooms
     *           (5, 10)
     *         _____.______
     *        |     |      |
     *        |  2  |  3   |
     * (0, 5) *-----.------* (10, 5)
     *        |     |      |
     *        |  0  |  1   |
     *        |_____.______|
     *            (5, 0)
     *
     * Expected output: graph with 5 points (centre point and four cardinal points) with appropriate edges */

    pb_room rooms[4] = {0};               /* room,     neighbour  overlap_start, overlap_end, ignored*/
    pb_sq_house_room_conn room_conns[8] = {{&rooms[0], &rooms[1], {5.f, 0.f},    {5.f, 5.f}, (side) 0, 0, 0},
                                           {&rooms[0], &rooms[2], {0.f, 5.f},    {5.f, 5.f}, (side) 0, 0, 0},
                                           {&rooms[1], &rooms[0], {5.f, 0.f},    {5.f, 5.f}, (side) 0, 0, 0},
                                           {&rooms[1], &rooms[3], {5.f, 5.f},    {10.f, 5.f}, (side) 0, 0, 0},
                                           {&rooms[2], &rooms[0], {0.f, 5.f},    {5.f, 5.f}, (side) 0, 0, 0},
                                           {&rooms[2], &rooms[3], {5.f, 5.f},    {5.f, 10.f}, (side) 0, 0, 0},
                                           {&rooms[3], &rooms[1], {5.f, 5.f},    {10.f, 5.f}, (side) 0, 0, 0},
                                           {&rooms[3], &rooms[2], {5.f, 5.f},    {5.f, 10.f}, (side) 0, 0, 0}};

    pb_graph* floor_graph = pb_graph_create(pb_pointer_hash, pb_pointer_eq);

    pb_graph* result;
    pb_point2D expected_points[] = {{0, 5}, {5, 0}, {5, 5}, {5, 10}, {10, 5}};
    pb_pair expected_edge_points[] = {{&expected_points[0], &expected_points[2]},
                                      {&expected_points[1], &expected_points[2]},
                                      {&expected_points[3], &expected_points[2]},
                                      {&expected_points[4], &expected_points[2]},
                                      {&expected_points[2], &expected_points[0]},
                                      {&expected_points[2], &expected_points[1]},
                                      {&expected_points[2], &expected_points[3]},
                                      {&expected_points[2], &expected_points[4]}};
    /* Unfortunately there are two possible room connections for each edge because the graph's internal vertex order
     * isn't defined */
    pb_sq_house_room_conn* expected_room_conns[] = {&room_conns[1], &room_conns[4],
                                                    &room_conns[0], &room_conns[2],
                                                    &room_conns[5], &room_conns[7],
                                                    &room_conns[3], &room_conns[6],
                                                    &room_conns[1], &room_conns[4],
                                                    &room_conns[0], &room_conns[2],
                                                    &room_conns[5], &room_conns[7],
                                                    &room_conns[3], &room_conns[6]};

    unsigned i;
    for(i = 0; i < 4; ++i) {
        pb_graph_add_vertex(floor_graph, &rooms[i], &rooms[i]);
    }
    for(i = 0; i < 8; ++i) {
        pb_graph_add_edge(floor_graph, room_conns[i].room, room_conns[i].neighbour, 0, &room_conns[i]);
    }

    result = pb_sq_house_generate_internal_graph(floor_graph);
    ck_assert_msg(result->vertices->size == 5, "result graph should have had 5 vertices, had %lu", result->vertices->size);
    ck_assert_msg(result->edges->size == 8, "result graph should have had 8 edges, had %lu", result->edges->size);

    for(i = 0; i < 5; ++i) {
        pb_vertex const* vert = pb_graph_get_vertex(result, &expected_points[i]);
        ck_assert_msg(vert, "graph should have contained point (%.1f, %.1f)", expected_points[i].x, expected_points[i].y);
    }

    for(i = 0; i < 8; i += 2) {
        pb_edge const* edge = pb_graph_get_edge(result, expected_edge_points[i].first, expected_edge_points[i].second);
        pb_sq_house_room_conn* rconn = (pb_sq_house_room_conn*)edge->data;
        unsigned conn_idx = i * 2;

        ck_assert_msg(edge, "graph should have had edge between (%.1f, %.1f) and (%.1f, %.1f)",
                      ((pb_point2D*)expected_edge_points[i].first)->x, ((pb_point2D*)expected_edge_points[i].second)->y,
                      ((pb_point2D*)expected_edge_points[i].second)->x, ((pb_point2D*)expected_edge_points[i].second)->y);

        ck_assert_msg(edge->weight == 25, "all edge weights should be 25");

        ck_assert_msg(rconn == expected_room_conns[conn_idx] || rconn == expected_room_conns[conn_idx + 1],
                      "edge %u should have had connection %u or %u", i, conn_idx, conn_idx + 1);
    }

    pb_graph_free(result);
}
END_TEST

START_TEST(internal_graph_multiple_overlap)
{
    /* Input: graph representing 10x10 house evenly divided into 4 rooms
     *                (5, 10)
     *         __________.___________
     *        |  Room 1  |           |
     *        |          |  Room 4   |
     *        |(5, 20/3) *-----------* (10, 20/3)
     *        |          |  Room 3   |
     * (0, 5) *----------* (5, 5)    |
     *        |          .-----------* (10, 10/3)
     *        |(5, 10/3) |           |
     *        | Room 0   |  Room 2   |
     *        |__________.___________|
     *                (5, 0)
     *
     * Expected output: graph with the 8 points above and the 14 bi-directional edges connecting them */

    pb_room rooms[5] = {0};                /* room,     neighbour  overlap_start, overlap_end,    ignored*/
    pb_sq_house_room_conn room_conns[14] = {{&rooms[0], &rooms[1], {0.f, 5.f},    {5.f, 5.f},     (side) 0, 0, 0},
                                            {&rooms[0], &rooms[2], {5.f, 0.f},    {5.f, 10/3.f},  (side) 0, 0, 0},
                                            {&rooms[0], &rooms[3], {5.f, 10/3.f}, {5.f, 5.f},     (side) 0, 0, 0},
                                            {&rooms[1], &rooms[0], {0.f, 5.f},    {5.f, 5.f},     (side) 0, 0, 0},
                                            {&rooms[1], &rooms[3], {5.f, 5.f},    {5.f, 20/3.f},  (side) 0, 0, 0},
                                            {&rooms[1], &rooms[4], {5.f, 20/3.f}, {5.f, 10.f},    (side) 0, 0, 0},
                                            {&rooms[2], &rooms[0], {5.f, 0.f},    {5.f, 10/3.f},  (side) 0, 0, 0},
                                            {&rooms[2], &rooms[3], {5.f, 10/3.f}, {10.f, 10/3.f}, (side) 0, 0, 0},
                                            {&rooms[3], &rooms[0], {5.f, 10/3.f}, {5.f, 5.f},     (side) 0, 0, 0},
                                            {&rooms[3], &rooms[1], {5.f, 5.f},    {5.f, 20/3.f},  (side) 0, 0, 0},
                                            {&rooms[3], &rooms[2], {5.f, 10/3.f}, {10.f, 10/3.f}, (side) 0, 0, 0},
                                            {&rooms[3], &rooms[4], {5.f, 20/3.f}, {10.f, 20/3.f}, (side) 0, 0, 0},
                                            {&rooms[4], &rooms[1], {5.f, 20/3.f}, {5.f, 10.f},    (side) 0, 0, 0},
                                            {&rooms[4], &rooms[3], {5.f, 20/3.f}, {10.f, 20/3.f}, (side) 0, 0, 0}};

    pb_graph* floor_graph = pb_graph_create(pb_pointer_hash, pb_pointer_eq);

    pb_graph* result;
    pb_point2D expected_points[] = {{0, 5}, {5, 0}, {5, 10/3.f}, {5, 5},
                                    {5, 20/3.f}, {5, 10}, {10, 10/3.f}, {10, 20/3.f}};
    pb_pair expected_edge_points[] = {{&expected_points[0], &expected_points[3]},
                                      {&expected_points[1], &expected_points[2]},
                                      {&expected_points[2], &expected_points[1]},
                                      {&expected_points[2], &expected_points[3]},
                                      {&expected_points[2], &expected_points[6]},
                                      {&expected_points[3], &expected_points[0]},
                                      {&expected_points[3], &expected_points[2]},
                                      {&expected_points[3], &expected_points[4]},
                                      {&expected_points[4], &expected_points[3]},
                                      {&expected_points[4], &expected_points[5]},
                                      {&expected_points[4], &expected_points[7]},
                                      {&expected_points[5], &expected_points[4]},
                                      {&expected_points[6], &expected_points[2]},
                                      {&expected_points[7], &expected_points[4]}};

    pb_sq_house_room_conn* expected_room_conns[] = {&room_conns[0],  &room_conns[3],
                                                    &room_conns[1],  &room_conns[6],
                                                    &room_conns[1],  &room_conns[6],
                                                    &room_conns[2],  &room_conns[8],
                                                    &room_conns[7],  &room_conns[10],
                                                    &room_conns[0],  &room_conns[3],
                                                    &room_conns[2],  &room_conns[8],
                                                    &room_conns[4],  &room_conns[9],
                                                    &room_conns[4],  &room_conns[9],
                                                    &room_conns[5],  &room_conns[12],
                                                    &room_conns[11], &room_conns[13],
                                                    &room_conns[5],  &room_conns[12],
                                                    &room_conns[7],  &room_conns[10],
                                                    &room_conns[11], &room_conns[13]};

    float weights[] = {25.f, 100/9.f, 100/9.f, 25/9.f, 25.f, 25.f, 25/9.f,
                       25/9.f, 25/9.f, 100/9.f, 25.f, 100/9.f, 25.f, 25.f};

    unsigned i;
    for(i = 0; i < 5; ++i) {
        pb_graph_add_vertex(floor_graph, &rooms[i], &rooms[i]);
    }
    for(i = 0; i < 14; ++i) {
        pb_graph_add_edge(floor_graph, room_conns[i].room, room_conns[i].neighbour, 0, &room_conns[i]);
    }

    result = pb_sq_house_generate_internal_graph(floor_graph);
    ck_assert_msg(result->vertices->size == 8, "result graph should have had 8 vertices, had %lu", result->vertices->size);
    ck_assert_msg(result->edges->size == 14, "result graph should have had 14 edges, had %lu", result->edges->size);

    for(i = 0; i < 8; ++i) {
        pb_vertex const* vert = pb_graph_get_vertex(result, &expected_points[i]);
        ck_assert_msg(vert, "graph should have contained point (%.1f, .1f)", expected_points[i].x, expected_points[i].y);
    }

    for(i = 0; i < 14; i += 2) {
        pb_edge const* edge = pb_graph_get_edge(result, expected_edge_points[i].first, expected_edge_points[i].second);
        pb_sq_house_room_conn* rconn = (pb_sq_house_room_conn*)edge->data;
        unsigned conn_idx = i * 2;

        ck_assert_msg(edge, "graph should have had edge between (%.1f, %.1f) and (%.1f, %.1f)",
                      ((pb_point2D*)expected_edge_points[i].first)->x, ((pb_point2D*)expected_edge_points[i].second)->y,
                      ((pb_point2D*)expected_edge_points[i].second)->x, ((pb_point2D*)expected_edge_points[i].second)->y);

        ck_assert_msg(pb_float_approx_eq(edge->weight, weights[i], 5), "edge %u should have weight %.2f, has %.2f",
                      i, weights[i], edge->weight);

        ck_assert_msg(rconn == expected_room_conns[conn_idx] || rconn == expected_room_conns[conn_idx + 1],
                      "edge %u should have had connection %u or %u", i, conn_idx, conn_idx + 1);
    }

    pb_graph_free(result);
}
END_TEST

START_TEST(room0_disconnected_simple)
{
    /* Input:
     * -    A floor with three rooms occupying rectangles {(0, 0), 5, 5}, {(0, 5), 5, 5}, and
     *      {(5, 0), 5, 10}
     * -    An internal floor graph with the points (0, 5), (5, 0), (5, 5), (5, 10) and appropriate edges
     * -    A hashmap containing a pointer to room 0
     *
     * Expected output: a pb_vector of size 1, containing another pb_vector of size 1, with edge (5, 0)->(5, 5) */
    pb_floor f;
    pb_graph* floor_graph = pb_graph_create(pb_pointer_hash, pb_pointer_eq);
    pb_graph* internal_graph;
    pb_rect rects[] = {{{0, 0}, 5, 5}, {{0, 5}, 5, 5}, {{5, 0}, 5, 10}};
    pb_rect frect = {{0, 0}, 10, 10};
    pb_point2D points[] = {{0, 5}, {5, 0}, {5, 5}, {5, 10}};
    pb_room rooms[3] = {0};
    pb_sq_house_room_conn conns[] = {{&rooms[0], &rooms[1], {0.f, 5.f}, {5.f, 5.f},  (side) 0, 0, 0},
                                     {&rooms[0], &rooms[2], {5.f, 0.f}, {5.f, 5.f},  (side) 0, 0, 0},
                                     {&rooms[1], &rooms[0], {0.f, 5.f}, {5.f, 5.f},  (side) 0, 0, 0},
                                     {&rooms[1], &rooms[2], {5.f, 5.f}, {5.f, 10.f}, (side) 0, 0, 0},
                                     {&rooms[2], &rooms[0], {5.f, 0.f}, {5.f, 5.f},  (side) 0, 0, 0},
                                     {&rooms[2], &rooms[1], {5.f, 5.f}, {5.f, 10.f}, (side) 0, 0, 0}};
    pb_pair expected_edge = {&points[1], &points[2]};
    pb_hashmap* disconnected = pb_hashmap_create(pb_pointer_hash, pb_pointer_eq);

    pb_vector* result;
    pb_vector* hallway;
    pb_edge** hallway_edges;

    /* Set up the data */
    int i;
    for(i = 0; i < 3; ++i) {
        pb_rect_to_pb_shape2D(&rects[i], &rooms[i].shape);
    }
    for(i = 0; i < 3; ++i) {
        pb_graph_add_vertex(floor_graph, &rooms[i], &rooms[i]);
    }
    for(i = 0; i < 6; ++i) {
        pb_graph_add_edge(floor_graph, conns[i].room, conns[i].neighbour, 0, &conns[i]);
    }
    internal_graph = pb_sq_house_generate_internal_graph(floor_graph);
    pb_hashmap_put(disconnected, &rooms[0], &rooms[0]);

    f.rooms = &rooms[0];
    pb_rect_to_pb_shape2D(&frect, &f.shape);

    /* Generate the hallways */
    result = pb_sq_house_get_hallways(&f, floor_graph, internal_graph, disconnected);

    ck_assert_msg(result->size == 1, "result should have only had one hallway, had %lu", result->size);
    hallway = (pb_vector*)result->items;
    hallway_edges = (pb_edge**)hallway->items;
    ck_assert_msg(hallway->size == 1, "hallway should have had one edge, had %lu", hallway->size);

    {
        pb_vertex const *e_from = pb_graph_get_vertex(internal_graph, expected_edge.first);
        pb_vertex const *e_to = pb_graph_get_vertex(internal_graph, expected_edge.second);

        pb_point2D *from_point = (pb_point2D *) hallway_edges[0]->from->data;
        pb_point2D *e_from_point = (pb_point2D *) e_from->data;

        pb_point2D *to_point = (pb_point2D *) hallway_edges[0]->to->data;
        pb_point2D *e_to_point = (pb_point2D *) e_to->data;

        ck_assert_msg(hallway_edges[0]->from == e_from,
                      "hallway edge incorrect, had from point (%.2f, %.2f) instead of"
                              "(%.2f, %.2f)", from_point->x, from_point->y, e_from_point->x, e_from_point->y);

        ck_assert_msg(hallway_edges[0]->to == e_to, "hallway edge incorrect, had to point (%.2f, %.2f) instead of"
                "(%.2f, %.2f)", to_point->x, to_point->y, e_to_point->x, e_to_point->y);
    }

    for(i = 0; i < 3; ++i) {
        pb_shape2D_free(&rooms[i].shape);
    }
    pb_shape2D_free(&f.shape);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(disconnected);
    pb_vector_free(hallway);
    pb_vector_free(result);

}
END_TEST

START_TEST(room0_disconnected_one_wall_overlaps)
{
    /* Input:
     * -    A floor with three rooms occupying rectangles {(0, 0), 5, 10}, {(5, 0), 5, 5}, {(5, 5), 5, 5}
     * -    An internal floor graph with the points (5, 0), (5, 5), (5, 10), (10, 5)
     * -    A hashmap containing a pointer to room 0
     *
     * Expected output: a pb_vector of size 1, containing another pb_vector of size 2, with edge (5, 0)->(5, 10) and
     * (5, 5)->(5, 10) */
    pb_floor f;
    pb_graph* floor_graph = pb_graph_create(pb_pointer_hash, pb_pointer_eq);
    pb_graph* internal_graph;
    pb_rect rects[] = {{{0, 0}, 5, 10}, {{5, 0}, 5, 5}, {{5, 5}, 5, 5}};
    pb_rect frect = {{0, 0}, 10, 10};
    pb_point2D points[] = {{5, 0}, {5, 5}, {5, 10}, {10, 5}};
    pb_room rooms[3] = {0};
    pb_sq_house_room_conn conns[] = {{&rooms[0], &rooms[1], {5.f, 0.f}, {5.f, 5.f},  (side) 0, 0, 0},
                                     {&rooms[0], &rooms[2], {5.f, 5.f}, {5.f, 10.f}, (side) 0, 0, 0},
                                     {&rooms[1], &rooms[0], {5.f, 0.f}, {5.f, 5.f},  (side) 0, 0, 0},
                                     {&rooms[1], &rooms[2], {5.f, 5.f}, {10.f, 5.f}, (side) 0, 0, 0},
                                     {&rooms[2], &rooms[0], {5.f, 5.f}, {5.f, 10.f}, (side) 0, 0, 0},
                                     {&rooms[2], &rooms[1], {5.f, 5.f}, {10.f, 5.f}, (side) 0, 0, 0}};
    pb_pair expected_edges[] = {{&points[0], &points[1]}, {&points[1], &points[2]}};
    pb_hashmap* disconnected = pb_hashmap_create(pb_pointer_hash, pb_pointer_eq);

    pb_vector* result;
    pb_vector* hallway;
    pb_edge** hallway_edges;

    /* Set up the data */
    int i;
    for(i = 0; i < 3; ++i) {
        pb_rect_to_pb_shape2D(&rects[i], &rooms[i].shape);
    }
    for(i = 0; i < 3; ++i) {
        pb_graph_add_vertex(floor_graph, &rooms[i], &rooms[i]);
    }
    for(i = 0; i < 6; ++i) {
        pb_graph_add_edge(floor_graph, conns[i].room, conns[i].neighbour, 0, &conns[i]);
    }
    internal_graph = pb_sq_house_generate_internal_graph(floor_graph);
    pb_hashmap_put(disconnected, &rooms[0], &rooms[0]);

    f.rooms = &rooms[0];
    pb_rect_to_pb_shape2D(&frect, &f.shape);

    /* Generate the hallways */
    result = pb_sq_house_get_hallways(&f, floor_graph, internal_graph, disconnected);

    ck_assert_msg(result->size == 1, "result should have had 1 hallway, had %lu", result->size);
    hallway = (pb_vector*)result->items;
    hallway_edges = (pb_edge**)hallway->items;
    ck_assert_msg(hallway->size == 2, "hallway should have had 2 edges, had %lu", hallway->size);

    for(i = 0; i < 2; ++i) {
        pb_vertex const *e_from = pb_graph_get_vertex(internal_graph, expected_edges[i].first);
        pb_vertex const *e_to = pb_graph_get_vertex(internal_graph, expected_edges[i].second);

        pb_point2D *from_point = (pb_point2D *) hallway_edges[i]->from->data;
        pb_point2D *e_from_point = (pb_point2D *) e_from->data;

        pb_point2D *to_point = (pb_point2D *) hallway_edges[i]->to->data;
        pb_point2D *e_to_point = (pb_point2D *) e_to->data;

        ck_assert_msg(hallway_edges[i]->from == e_from,
                      "hallway edge incorrect, had from point (%.2f, %.2f) instead of"
                              "(%.2f, %.2f)", from_point->x, from_point->y, e_from_point->x, e_from_point->y);

        ck_assert_msg(hallway_edges[i]->to == e_to, "hallway edge incorrect, had to point (%.2f, %.2f) instead of"
                "(%.2f, %.2f)", to_point->x, to_point->y, e_to_point->x, e_to_point->y);
    }

    for(i = 0; i < 3; ++i) {
        pb_shape2D_free(&rooms[i].shape);
    }
    pb_shape2D_free(&f.shape);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(disconnected);
    pb_vector_free(hallway);
    pb_vector_free(result);
}
END_TEST

START_TEST(room0_disconnected_one_wall_small)
{
    /* Input:
     * -    A floor with two rooms occupying rectangles {(0, 0), 10, 5}, {(10, 0), 5, 5}
     * -    An internal floor graph with the points (10, 0), (10, 5)
     * -    A hashmap containing a pointer to room 0
     *
     * Expected output: a pb_vector of size 1, containing another pb_vector of size 1, with edge (10, 0)->(10, 5) */
    pb_floor f;
    pb_graph* floor_graph = pb_graph_create(pb_pointer_hash, pb_pointer_eq);
    pb_graph* internal_graph;
    pb_rect rects[] = {{{0, 0}, 10, 5}, {{10, 0}, 5, 5}};
    pb_rect frect = {{0, 0}, 15, 5};
    pb_point2D points[] = {{10, 0}, {10, 5}};
    pb_room rooms[3] = {0};
    pb_sq_house_room_conn conns[] = {{&rooms[0], &rooms[1], {10.f, 0.f}, {10.f, 5.f}, (side) 0, 0, 0},
                                     {&rooms[1], &rooms[0], {10.f, 0.f}, {10.f, 5.f}, (side) 0, 0, 0}};
    pb_pair expected_edges[] = {{&points[0], &points[1]}};
    pb_hashmap* disconnected = pb_hashmap_create(pb_pointer_hash, pb_pointer_eq);

    pb_vector* result;
    pb_vector* hallway;
    pb_edge** hallway_edges;

    /* Set up the data */
    int i;
    for(i = 0; i < 2; ++i) {
        pb_rect_to_pb_shape2D(&rects[i], &rooms[i].shape);
    }
    for(i = 0; i < 2; ++i) {
        pb_graph_add_vertex(floor_graph, &rooms[i], &rooms[i]);
    }
    for(i = 0; i < 2; ++i) {
        pb_graph_add_edge(floor_graph, conns[i].room, conns[i].neighbour, 0, &conns[i]);
    }
    internal_graph = pb_sq_house_generate_internal_graph(floor_graph);
    pb_hashmap_put(disconnected, &rooms[0], &rooms[0]);

    f.rooms = &rooms[0];
    pb_rect_to_pb_shape2D(&frect, &f.shape);

    /* Generate the hallways */
    result = pb_sq_house_get_hallways(&f, floor_graph, internal_graph, disconnected);

    ck_assert_msg(result->size == 1, "result should have had 1 hallway, had %lu", result->size);
    hallway = (pb_vector*)result->items;
    hallway_edges = (pb_edge**)hallway->items;
    ck_assert_msg(hallway->size == 1, "hallway should have had 1 edge, had %lu", hallway->size);

    for(i = 0; i < 1; ++i) {
        pb_vertex const *e_from = pb_graph_get_vertex(internal_graph, expected_edges[i].first);
        pb_vertex const *e_to = pb_graph_get_vertex(internal_graph, expected_edges[i].second);

        pb_point2D *from_point = (pb_point2D *) hallway_edges[i]->from->data;
        pb_point2D *e_from_point = (pb_point2D *) e_from->data;

        pb_point2D *to_point = (pb_point2D *) hallway_edges[i]->to->data;
        pb_point2D *e_to_point = (pb_point2D *) e_to->data;

        ck_assert_msg(hallway_edges[i]->from == e_from,
                      "hallway edge incorrect, had from point (%.2f, %.2f) instead of"
                              "(%.2f, %.2f)", from_point->x, from_point->y, e_from_point->x, e_from_point->y);

        ck_assert_msg(hallway_edges[i]->to == e_to, "hallway edge incorrect, had to point (%.2f, %.2f) instead of"
                "(%.2f, %.2f)", to_point->x, to_point->y, e_to_point->x, e_to_point->y);
    }

    for(i = 0; i < 2; ++i) {
        pb_shape2D_free(&rooms[i].shape);
    }
    pb_shape2D_free(&f.shape);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(disconnected);
    pb_vector_free(hallway);
    pb_vector_free(result);
}
END_TEST

Suite *make_pb_sq_house_graph_suite(void)
{
    Suite* s;
    TCase* tc_sq_house_get_shared_wall;
    TCase* tc_sq_house_get_wall_overlap;
    TCase* tc_sq_house_generate_floor_graph;
    TCase* tc_sq_house_find_disconnected;
    TCase* tc_sq_house_internal_graph;
    TCase* tc_sq_house_find_hallways;
    TCase* tc_sq_house_place_hallways;
    TCase* tc_sq_house_place_doors;
    TCase* tc_sq_house_place_windows;

    s = suite_create("Squarified house generation graph algorithms");

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
    tcase_add_test(tc_sq_house_generate_floor_graph, generate_floor_graph_no_door_space);

    tc_sq_house_find_disconnected = tcase_create("Disconnected room finding tests");
    suite_add_tcase(s, tc_sq_house_find_disconnected);
    tcase_add_test(tc_sq_house_find_disconnected, find_disconnected_rooms_basic);
    tcase_add_test(tc_sq_house_find_disconnected, find_disconnected_rooms_one_sided_connection);
    tcase_add_test(tc_sq_house_find_disconnected, find_disconnected_rooms_outside_single_disconnected);
    tcase_add_test(tc_sq_house_find_disconnected, find_disconnected_rooms_outside_multi_disconnected);

    tc_sq_house_internal_graph = tcase_create("Internal graph generation tests");
    suite_add_tcase(s, tc_sq_house_internal_graph);
    tcase_add_test(tc_sq_house_internal_graph, internal_graph_simple);
    tcase_add_test(tc_sq_house_internal_graph, internal_graph_multiple_overlap);

    tc_sq_house_find_hallways = tcase_create("Hallway finding tests");
    suite_add_tcase(s, tc_sq_house_find_hallways);
    tcase_add_test(tc_sq_house_find_hallways, room0_disconnected_simple);
    tcase_add_test(tc_sq_house_find_hallways, room0_disconnected_one_wall_overlaps);
    tcase_add_test(tc_sq_house_find_hallways, room0_disconnected_one_wall_small);

    return s;
}