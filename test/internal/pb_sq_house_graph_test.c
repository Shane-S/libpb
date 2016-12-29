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

    pb_rect rect1;
    pb_rect rect2;

    pb_shape2D_to_pb_rect(&r1, &rect1);
    pb_shape2D_to_pb_rect(&r2, &rect2);

    result = pb_sq_house_get_shared_wall(&rect1, &rect2);
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

    pb_rect rect1;
    pb_rect rect2;

    pb_shape2D_to_pb_rect(&r1, &rect1);
    pb_shape2D_to_pb_rect(&r2, &rect2);

    result = pb_sq_house_get_shared_wall(&rect1, &rect2);
    ck_assert_msg(result == SQ_HOUSE_LEFT, "result should have been 0 (left), was %d", result);
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

    pb_rect rect1;
    pb_rect rect2;

    pb_shape2D_to_pb_rect(&r1, &rect1);
    pb_shape2D_to_pb_rect(&r2, &rect2);

    result = pb_sq_house_get_shared_wall(&rect1, &rect2);
    ck_assert_msg(result == SQ_HOUSE_TOP, "result should have been 3 (top), was %d", result);
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

    pb_rect rect1;
    pb_rect rect2;

    pb_shape2D_to_pb_rect(&r1, &rect1);
    pb_shape2D_to_pb_rect(&r2, &rect2);

    result = pb_sq_house_get_shared_wall(&rect1, &rect2);
    ck_assert_msg(result == SQ_HOUSE_BOTTOM, "result should have been 1 (bottom), was %d", result);
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

    pb_rect rect1;
    pb_rect rect2;

    pb_shape2D_to_pb_rect(&r1, &rect1);
    pb_shape2D_to_pb_rect(&r2, &rect2);

    result = pb_sq_house_get_shared_wall(&rect1, &rect2);
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
        rooms[i].name = room_names[i];
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
    rooms[0].name = room_name;

    rooms[1].shape = shape1;
    rooms[1].name = room_name;

    f.num_rooms = 2;
    f.rooms = &rooms[0];

    h_spec.door_size = 0.5f;

    result = pb_sq_house_generate_floor_graph(&h_spec, room_specs, &f);
    {
        pb_sq_house_room_conn* conn;
        pb_edge const* edge = pb_graph_get_edge(result, &rooms[0], &rooms[1]);
        ck_assert_msg(edge, "Edge should have existed between rooms 0 and 1");

        conn = (pb_sq_house_room_conn*)edge->data;
        ck_assert_msg(conn->has_door == 0, "There should not have been a door between rooms 0 and 1");

        edge = pb_graph_get_edge(result, &rooms[1], &rooms[0]);
        ck_assert_msg(edge, "Edge should have existed between rooms 1 and 0");

        conn = (pb_sq_house_room_conn*)edge->data;
        ck_assert_msg(conn->has_door == 0, "There should not have been a door between rooms 1 and 0.");
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
        /* Given a pb_floor with two rooms and a pb_graph holding the floor connectivity graph
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

    pb_graph_free(floor_graph);
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

    pb_graph_free(floor_graph);
    pb_graph_free(result);
}
END_TEST

START_TEST(get_hallways_room0_disconnected_simple)
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
    free(result);

}
END_TEST

START_TEST(get_hallways_room0_disconnected_one_wall_overlaps)
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
    free(result);
}
END_TEST

START_TEST(get_hallways_room0_disconnected_one_wall_small)
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
    free(result);
}
END_TEST

START_TEST(get_hallways_single_disconnected)
{
    /* Input:
     * -    A floor with three rooms occupying rectangles {(0, 0), 5, 10}, {(5, 0), 5, 5}, {(5, 5), 5, 5}
     * -    An internal floor graph with the points (5, 0), (5, 5), (5, 10), (10, 5)
     * -    A hashmap containing a pointer to room 2
     *
     * Expected output: a pb_vector of size 1, containing another pb_vector of size 2, with edges (5, 10)->(5, 5),
     * (5, 5)->(5, 0)*/
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
    pb_pair expected_edges[] = {{&points[2], &points[1]}, {&points[1], &points[0]}};
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
    pb_hashmap_put(disconnected, &rooms[2], &rooms[2]);

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
                      "hallway edge incorrect, had from point (%.2f, %.2f) instead of "
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
    free(result);
}
END_TEST

START_TEST(place_hallways_simple)
{
    size_t i, j;

    /* Input: - floor with two rooms, rects {{0, 0}, 5, 5}, {{5, 0}, 5, 5}
     *        - list of hallways with a single hallway containing one edge (the two points)
     *        - floor graph for this floor (generated by pb_sq_house_generate_floor_graph)
     *        - internal graph for this floor (generated by pb_sq_house_generate_internal_graph)
     *        - map of room specs containing 1 spec with 2 allowed instances, can connect to itself
     *        - house spec with hallway_width = 0.5
     *
     * Expected Output: - floor contains 3 rooms with shapes
     *                    [] {0, 5}, {0, 0}, {4.75, 0}, {4.75, 5}
     *                    [] {5.25, 5}, {5.25, 0}, {10, 0}, {10, 5}
     *                    [] {4.75, 5}, {4.75, 0}, {5.25, 0}, {5.25, 5}
     *                  - connection between hallway and room 0 with overlap points {4.75, 0}, {4.75, 5}, has door is true
     *                  - connection between hallway and room 1 with overlap points {5.25, 0} and {5.25, 5}, has door is true
     */

    char* adj[] = { "Room" };
    pb_sq_house_room_spec specs[1];
    specs[0].adjacent = &adj[0];
    specs[0].area = 25.f;
    specs[0].name = "Room";
    specs[0].max_instances = 2;
    specs[0].num_adjacent = 1;
    specs[0].priority = 1;

    pb_sq_house_house_spec h;
    h.hallway_width = 0.5f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, specs[0].name, &specs[0]);

    pb_room* rooms = malloc(sizeof(pb_room) * 2);
    pb_rect room0_rect = { { 0, 0 }, 5, 5 };
    pb_rect_to_pb_shape2D(&room0_rect, &rooms[0].shape);
    pb_vector_init(&rooms[0].walls, sizeof(int), 4);
    rooms[0].name = specs[0].name;

    pb_rect room1_rect = { { 5, 0 }, 5, 5 };
    pb_rect_to_pb_shape2D(&room1_rect, &rooms[1].shape);
    pb_vector_init(&rooms[1].walls, sizeof(int), 4);
    rooms[1].name = specs[0].name;

    pb_floor f;
    f.rooms = rooms;
    f.num_rooms = 2;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&h, room_spec_map, &f);
    pb_graph* internal_graph = pb_sq_house_generate_internal_graph(floor_graph);

    pb_vector hallways;
    pb_vector_init(&hallways, sizeof(pb_vector), 1);

    pb_vector hallway;
    pb_point2D start = { 5.f, 0.f };
    pb_point2D end = { 5.f, 5.f };

    pb_vector_init(&hallway, sizeof(pb_edge*), 1);
    pb_edge* edge = pb_graph_get_edge(internal_graph, &start, &end);
    pb_vector_push_back(&hallway, &edge);

    pb_vector_push_back(&hallways, &hallway);
    
    pb_sq_house_place_hallways(&f, &h, room_spec_map, floor_graph, internal_graph, &hallways);

    pb_point2D hallway0_expected_points[] = { {4.75f, 5.f}, {4.75f, 0.f}, {5.25f, 0.f}, {5.25f, 5.f} };
    int hallway0_expected_walls[] = { 1, 1, 1, 1 };
    
    ck_assert_msg(f.num_rooms == 3, "floor should have had 3 rooms, had %lu", f.num_rooms);
    
    /* Check that hallways were added with correct shapes */
    pb_room* hallway0_room = f.rooms + 2;
    pb_point2D* hallway0_room_points = (pb_point2D*)hallway0_room->shape.points.items;
    int* hallway0_room_walls = (int*)hallway0_room->walls.items;

    for (i = 0; i < 4; ++i) {
        pb_point2D* real = hallway0_room_points + i;
        pb_point2D* expected = &hallway0_expected_points[0] + i;
        ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                      "point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", expected->x, expected->y, real->x, real->y);
        ck_assert_msg(hallway0_room_walls[i] == hallway0_expected_walls[i], "wall %d should have been %d, was %d",
                      i, hallway0_expected_walls[i], hallway0_room_walls[i]);
    }

    /* Check that rooms have the correct shapes */
    pb_point2D expected_room0_points[] = { { 0.f, 5.f },
                                           { 0.f, 0.f },
                                           { 4.75f, 0.f },
                                           { 4.75f, 5.f } };
    pb_point2D expected_room1_points[] = { { 5.25f, 5.f },
                                           { 5.25f, 0.f },
                                           { 10.f, 0.f },
                                           { 10.f, 5.f } };
    size_t expected_room_point_counts[] = { sizeof(expected_room0_points) / sizeof(pb_point2D),
                                            sizeof(expected_room1_points) / sizeof(pb_point2D) };
    pb_point2D* expected_room_points[] = { &expected_room0_points[0],
                                           &expected_room1_points[0] };
    
    for (i = 0; i < sizeof(expected_room_points) / sizeof(pb_point2D*); ++i) {
        pb_room* room = f.rooms + i;
        pb_point2D* points = (pb_point2D*)room->shape.points.items;
        ck_assert_msg(room->shape.points.size == expected_room_point_counts[i], "room %d should have had %lu points, had %lu",
                      i, expected_room_point_counts[i], room->shape.points.size);
        for (j = 0; j < expected_room_point_counts[i]; ++j) {
            pb_point2D* real = points + j;
            pb_point2D* expected = expected_room_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "room %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
        }
    }

    /* Check room connections */
    pb_sq_house_room_conn expected_room0_conns[] = {
        /* room,     neighbour    overlap_start,  overlap_end,    wall,  can_connect, has_door*/
        { f.rooms,   &f.rooms[2], { 4.75f, 0.f }, { 4.75f, 5.f }, 2,     1,           1 }
     };

    pb_sq_house_room_conn expected_room1_conns[] = {
        /* room,       neighbour    overlap_start,  overlap_end,    wall,  can_connect, has_door*/
        { &f.rooms[1], &f.rooms[2], { 5.25f, 0.f }, { 5.25f, 5.f }, 0,     1,           1 },
    };

    pb_sq_house_room_conn expected_room2_conns[] = {
        /* room,       neighbour    overlap_start,  overlap_end,    wall,  can_connect, has_door*/
        { &f.rooms[2], &f.rooms[0], { 4.75f, 0.f }, { 4.75f, 5.f },  0,     1,           1 },
        { &f.rooms[2], &f.rooms[1], { 5.25f, 0.f }, { 5.25f, 5.f },  2,     1,           1 },
    };

    size_t expected_room_conn_counts[] = {
        sizeof(expected_room0_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room1_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room2_conns) / sizeof(pb_sq_house_room_conn),
    };
    pb_sq_house_room_conn* expected_room_conns[] = {
        &expected_room0_conns[0],
        &expected_room1_conns[0],
        &expected_room2_conns[0],
    };

    size_t total_edges = 0;
    for (i = 0; i < f.num_rooms; ++i) {
        total_edges += expected_room_conn_counts[i];
    }
    ck_assert_msg(floor_graph->edges->size == total_edges, "floor graph should have had %lu edges, had %lu", total_edges,
                  floor_graph->edges->size);

    for (i = 0; i < f.num_rooms; ++i) {
        pb_vertex const* vert = pb_graph_get_vertex(floor_graph, f.rooms + i);
        ck_assert_msg(vert->edges_size == expected_room_conn_counts[i],
                      "room %lu had %lu edges, should have had %lu",
                      i, vert->edges_size, expected_room_conn_counts[i]);

        for (j = 0; j < expected_room_conn_counts[i]; ++j) {
            pb_sq_house_room_conn const* expected = expected_room_conns[i] + j;
            pb_sq_house_room_conn const* actual = (pb_sq_house_room_conn*)vert->edges[j]->data;

            ck_assert_msg(actual->neighbour == expected->neighbour, "room %lu edge %lu had incorrect neighbour", i, j);
            ck_assert_msg(pb_point_eq(&actual->overlap_start, &expected->overlap_start),
                          "room %lu edge %lu: expected overlap start (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_start.x, expected->overlap_start.y, actual->overlap_start.x, actual->overlap_start.y);
            ck_assert_msg(pb_point_eq(&actual->overlap_end, &expected->overlap_end),
                          "room %lu edge %lu: expected overlap end (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_end.x, expected->overlap_end.y, actual->overlap_end.x, actual->overlap_end.y);
            ck_assert_msg(actual->can_connect == expected->can_connect,
                          "room %lu edge %lu: expected can_connect == %lu, was %lu",
                          i, j, expected->can_connect, actual->can_connect);
            ck_assert_msg(actual->has_door == expected->has_door,
                          "room %lu edge %lu: expected has_door %d, was %d",
                          i, j, expected->has_door, actual->has_door);
            ck_assert_msg(actual->wall == expected->wall,
                          "room %lu edge %lu: expected wall %d, was %d",
                          i, j, expected->wall, actual->wall);
        }
    }

    for (i = 0; i < f.num_rooms; ++i) {
        pb_shape2D_free(&f.rooms[i].shape);
        pb_vector_free(&f.rooms[i].walls);
    }
    for (i = 0; i < hallways.size; ++i) {
        pb_vector* hallway = ((pb_vector*)hallways.items) + i;
        pb_vector_free(hallway);
    }
    pb_vector_free(&hallways);

    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(room_spec_map);
    free(f.rooms);
}
END_TEST

START_TEST(place_hallways_corner)
{
    /* Input: - floor with three rooms, rects {{0, 0}, 5, 10}, {{5, 0}, 5, 5}, {{5, 5}, 5, 5}
     *         - list of hallways with a single hallway containing two edges
     *           []  (10, 5)->(5, 5)
     *           []  (5, 5)->(5, 10)
     *         - floor graph for this floor (generated by pb_sq_house_generate_floor_graph)
     *         - internal graph for this floor (generated by pb_sq_house_generate_internal_graph)
     *         - map of room specs containing 1 spec with 3 allowed instances, can connect to itself
     *         - house spec with hallway_width = 0.5
     *
     * Expected Output: - floor contains 5 rooms with shapes
     *                    [] {0, 10}, {0, 0}, {5, 0}, {5, 5}, {4.75, 5}, {4.75, 10}
     *                    [] {5, 4.75}, {5, 0}, {10, 0}, {10, 4.75}
     *                    [] {5.25, 10}, {5.25, 5.25}, {10, 5.25}, {10, 10}
     *                    [] {5.25, 5.25}, {5.25, 4.75}, {10, 4.75}, {10, 5.25}
     *                    [] {4.75, 10}, {4.75, 4.75}, {5.25, 4.75}, {5.25, 5.25}, {5.25, 10}
     *                  - connection between hallway 0 and room 1 with overlap points {5.25, 4.75} and {10, 4.75}
     *                  - connection between hallway 0 and room 2 with overlap points {5.25, 5.25} and {10, 5.25}
     *                  - connection between hallway 1 and room 0 with overlap points {4.75, 4.75}, {4.75, 10}
     *                  - connection between hallway 1 and room 2 with overlap points {5.25, 5.25} and {5.25, 10}
     */

    size_t i, j;

    char* adj[] = { "Room", "Big Room" };
    pb_sq_house_room_spec specs[2];
    specs[0].adjacent = &adj[0];
    specs[0].area = 25.f;
    specs[0].name = "Room";
    specs[0].max_instances = 2;
    specs[0].num_adjacent = 2;
    specs[0].priority = 1;

    specs[1].adjacent = &adj[0];
    specs[1].area = 50.f;
    specs[1].name = "Big Room";
    specs[1].max_instances = 1;
    specs[1].num_adjacent = 2;
    specs[1].priority = 1;

    pb_sq_house_house_spec h;
    h.hallway_width = 0.5f;
    h.door_size = 0.75f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, specs[0].name, &specs[0]);
    pb_hashmap_put(room_spec_map, specs[1].name, &specs[1]);

    pb_room* rooms = malloc(sizeof(pb_room) * 3);
    pb_rect room_rects[] = { { { 0.f, 0.f }, 5.f, 10.f },
                             { { 5.f, 0.f }, 5.f, 5.f },
                             { { 5.f, 5.f }, 5.f, 5.f} };

    pb_rect_to_pb_shape2D(&room_rects[0], &rooms[0].shape);
    pb_vector_init(&rooms[0].walls, sizeof(int), 4);
    rooms[0].name = specs[1].name;

    pb_rect_to_pb_shape2D(&room_rects[1], &rooms[1].shape);
    pb_vector_init(&rooms[1].walls, sizeof(int), 4);
    rooms[1].name = specs[0].name;

    pb_rect_to_pb_shape2D(&room_rects[2], &rooms[2].shape);
    pb_vector_init(&rooms[2].walls, sizeof(int), 4);
    rooms[2].name = specs[0].name;

    pb_floor f;
    f.rooms = rooms;
    f.num_rooms = 3;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&h, room_spec_map, &f);
    pb_graph* internal_graph = pb_sq_house_generate_internal_graph(floor_graph);

    pb_vector hallways;
    pb_vector_init(&hallways, sizeof(pb_vector), 1);

    pb_vector hallway;
    pb_point2D start = { 10.f, 5.f };
    pb_point2D middle = { 5.f, 5.f };
    pb_point2D end = { 5.f, 10.f };

    pb_edge* edge;
    pb_vector_init(&hallway, sizeof(pb_edge*), 2);
    edge = pb_graph_get_edge(internal_graph, &start, &middle);
    pb_vector_push_back(&hallway, &edge);
    edge = pb_graph_get_edge(internal_graph, &middle, &end);
    pb_vector_push_back(&hallway, &edge);

    pb_vector_push_back(&hallways, &hallway);

    pb_sq_house_place_hallways(&f, &h, room_spec_map, floor_graph, internal_graph, &hallways);

    pb_point2D hallway0_expected_points[] = { { 5.25f, 5.25f },
                                              { 5.25f, 4.75f },
                                              { 10.f, 4.75f },
                                              { 10.f, 5.25f } };
    int hallway0_expected_walls[] = { 0, 1, 1, 1 };

    pb_point2D hallway1_expected_points[] = { { 4.75f, 10.f },
                                              { 4.75f, 4.75f },
                                              { 5.25f, 4.75f },
                                              { 5.25f, 5.25f },
                                              { 5.25f, 10 } };
    int hallway1_expected_walls[] = { 1, 1, 0, 1, 1 };

    pb_point2D const* hallway_expected_points[] = { &hallway0_expected_points[0], &hallway1_expected_points[0] };
    int const* hallway_expected_walls[] = { &hallway0_expected_walls[0], &hallway1_expected_walls[0] };

    ck_assert_msg(f.num_rooms == 5, "floor should have had 5 rooms, had %lu", f.num_rooms);

    size_t num_hallways = sizeof(hallway_expected_walls) / sizeof(int*);
    size_t first_hallway_idx = 3;

    /* Check that hallways were added with correct shapes */
    for (i = 0; i < num_hallways; ++i) {
        
        pb_room* hallway_room = f.rooms + first_hallway_idx + i;
        pb_point2D* hallway_room_points = (pb_point2D*)hallway_room->shape.points.items;
        int* hallway_room_walls = (int*)hallway_room->walls.items;

        size_t j;
        for (j = 0; j < hallway_room->shape.points.size; ++j) {
            pb_point2D* real = hallway_room_points + j;
            pb_point2D* expected = hallway_expected_points[i] + j;
            int real_wall = hallway_room_walls[j];
            int expected_wall = hallway_expected_walls[i][j];
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "hallway %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
            ck_assert_msg(real_wall == expected_wall, "hallway %d wall %d should have been %d, was %d",
                         i, j, expected_wall, real_wall);
        }
    }

    /* Check that rooms have the correct shapes and connections */
    pb_point2D expected_room0_points[] = { { 0.f, 10.f },
                                           { 0.f, 0.f },
                                           { 5.f, 0.f },
                                           { 5.f, 4.75f },
                                           { 4.75f, 4.75f },
                                           { 4.75f, 10.f } };
    pb_point2D expected_room1_points[] = { { 5.f, 4.75f },
                                           { 5.f, 0.f },
                                           { 10.f, 0.f },
                                           { 10.f, 4.75f } };
    pb_point2D expected_room2_points[] = { { 5.25f, 10.f },
                                           { 5.25f, 5.25f },
                                           { 10.f, 5.25f },
                                           { 10.f, 10.f } };
    size_t expected_room_point_counts[] = { sizeof(expected_room0_points) / sizeof(pb_point2D),
                                            sizeof(expected_room1_points) / sizeof(pb_point2D),
                                            sizeof(expected_room2_points) / sizeof(pb_point2D) };
    pb_point2D* expected_room_points[] = { &expected_room0_points[0],
                                           &expected_room1_points[0],
                                           &expected_room2_points[0] };

    for (i = 0; i < sizeof(expected_room_points) / sizeof(pb_point2D*); ++i) {
        pb_room* room = f.rooms + i;
        pb_point2D* points = (pb_point2D*)room->shape.points.items;
        ck_assert_msg(room->shape.points.size == expected_room_point_counts[i], "room %d should have had %lu points, had %lu",
                      i, expected_room_point_counts[i], room->shape.points.size);
        for (j = 0; j < expected_room_point_counts[i]; ++j) {
            pb_point2D* real = points + j;
            pb_point2D* expected = expected_room_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "room %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
        }
    }

    /* Check room connections */
    pb_sq_house_room_conn expected_room0_conns[] = {
        /* room,     neighbour    overlap_start,    overlap_end,     wall,  can_connect, has_door*/
        { f.rooms,   &f.rooms[4], { 4.75f, 4.75f }, { 4.75f, 10.f }, 4,     1,           1 },
        { f.rooms,   &f.rooms[1], { 5.f, 0.f },     { 5.f, 4.75f },  2,     1,           1 },
    };

    pb_sq_house_room_conn expected_room1_conns[] = {
        /* room,        neighbour    overlap_start,    overlap_end,      wall,  can_connect, has_door*/
        { &f.rooms[1],  &f.rooms[3], { 5.25f, 4.75f }, { 10.f, 4.75f },  3,     1,           1 },
        { &f.rooms[1],  &f.rooms[4], { 5.f, 4.75f },   { 5.25f, 4.75f }, 3,     1,           0 },
        { &f.rooms[1],  &f.rooms[0], { 5.f, 0.f },     { 5.f, 4.75f },   0,     1,           1 },
    };

    pb_sq_house_room_conn expected_room2_conns[] = {
        /* room,       neighbour    overlap_start,    overlap_end,     wall,  can_connect, has_door*/
        { &f.rooms[2], &f.rooms[3], { 5.25f, 5.25f }, { 10.f, 5.25f }, 1,     1,           1 },
        { &f.rooms[2], &f.rooms[4], { 5.25f, 5.25f }, { 5.25f, 10.f }, 0,     1,           1 },
    };

    pb_sq_house_room_conn expected_room3_conns[] = {
        /* room,       neighbour     overlap_start,   overlap_end,     wall,  can_connect, has_door*/
        { &f.rooms[3], &f.rooms[1], { 5.25f, 4.75f }, { 10.f, 4.75f }, 1,     1,           1 },
        { &f.rooms[3], &f.rooms[2], { 5.25f, 5.25f }, { 10.f, 5.25f }, 3,     1,           1 },
    };

    pb_sq_house_room_conn expected_room4_conns[] = {
        /* room,       neighbour    overlap_start,    overlap_end,      wall,  can_connect, has_door*/
        { &f.rooms[4], &f.rooms[0], { 4.75f, 4.75f }, { 4.75f, 10.f },  0,     1,           1 },
        { &f.rooms[4], &f.rooms[1], { 5.f, 4.75f },   { 5.25f, 4.75f }, 1,     1,           0 },
        { &f.rooms[4], &f.rooms[2], { 5.25f, 5.25f }, { 5.25f, 10.f },  3,     1,           1 },
    };

    size_t expected_room_conn_counts[] = {
        sizeof(expected_room0_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room1_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room2_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room3_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room4_conns) / sizeof(pb_sq_house_room_conn),
    };
    pb_sq_house_room_conn* expected_room_conns[] = {
        &expected_room0_conns[0],
        &expected_room1_conns[0],
        &expected_room2_conns[0],
        &expected_room3_conns[0],
        &expected_room4_conns[0],
    };

    size_t total_edges = 0;
    for (i = 0; i < f.num_rooms; ++i) {
        total_edges += expected_room_conn_counts[i];
    }
    ck_assert_msg(floor_graph->edges->size == total_edges, "floor graph should have had %lu edges, had %lu", total_edges,
                  floor_graph->edges->size);

    for (i = 0; i < f.num_rooms; ++i) {
        pb_vertex const* vert = pb_graph_get_vertex(floor_graph, f.rooms + i);
        ck_assert_msg(vert->edges_size == expected_room_conn_counts[i],
                      "room %lu had %lu edges, should have had %lu",
                      i, vert->edges_size, expected_room_conn_counts[i]);

        for (j = 0; j < expected_room_conn_counts[i]; ++j) {
            pb_sq_house_room_conn const* expected = expected_room_conns[i] + j;
            pb_sq_house_room_conn const* actual = (pb_sq_house_room_conn*)vert->edges[j]->data;

            ck_assert_msg(actual->neighbour == expected->neighbour, "room %lu edge %lu had incorrect neighbour", i, j);
            ck_assert_msg(pb_point_eq(&actual->overlap_start, &expected->overlap_start),
                          "room %lu edge %lu: expected overlap start (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_start.x, expected->overlap_start.y, actual->overlap_start.x, actual->overlap_start.y);
            ck_assert_msg(pb_point_eq(&actual->overlap_end, &expected->overlap_end),
                          "room %lu edge %lu: expected overlap end (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_end.x, expected->overlap_end.y, actual->overlap_end.x, actual->overlap_end.y);
            ck_assert_msg(actual->can_connect == expected->can_connect,
                          "room %lu edge %lu: expected can_connect == %lu, was %lu",
                          i, j, expected->can_connect, actual->can_connect);
            ck_assert_msg(actual->has_door == expected->has_door,
                          "room %lu edge %lu: expected has_door %d, was %d",
                          i, j, expected->has_door, actual->has_door);
            ck_assert_msg(actual->wall == expected->wall,
                          "room %lu edge %lu: expected wall %d, was %d",
                          i, j, expected->wall, actual->wall);
        }
    }

    for (i = 0; i < f.num_rooms; ++i) {
        pb_shape2D_free(&f.rooms[i].shape);
        pb_vector_free(&f.rooms[i].walls);
    }
    for (i = 0; i < hallways.size; ++i) {
        pb_vector* hallway = ((pb_vector*)hallways.items) + i;
        pb_vector_free(hallway);
    }
    pb_vector_free(&hallways);

    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(room_spec_map);
    free(f.rooms);
}
END_TEST

START_TEST(place_hallways_2_corners_opposite_sides)
{
    /* Input: - floor with four rooms, rects
     *           []  {{0, 0}, 5, 2.5}
     *           []  {{0, 2.5}, 5, 7.5}
     *           []  {{5, 0}, 5, 7.5}
     *           []  {{5, 7.5}, 5, 2.5}
     *         - list of hallways with a single hallway containing three edges
     *           []  (0, 2.5)->(5, 2.5)
     *           []  (5, 2.5)->(5, 7.5)
     *           []  (5, 7.5)->(10, 7.5)
     *         - floor graph for this floor (generated by pb_sq_house_generate_floor_graph)
     *         - internal graph for this floor (generated by pb_sq_house_generate_internal_graph)
     *         - map of room specs containing 2 spec with 2 allowed instances each, can connect to each other
     *         - house spec with hallway_width = 0.5
     *
     * Expected Output: - floor contains 7 rooms with shapes
     *                    []  {0, 2.25}, {0, 0}, {5, 0}, {5, 2.25}
     *                    []  {0, 10}, {0, 2.75}, {4.75, 2.75}, {4.75, 7.75}, {5, 7.75}, {5, 10}
     *                    []  {5.25, 7.25}, {5.25, 2.25}, {5, 2.25}, {5, 0}, {10, 0}, {10, 7.25}
     *                    []  {5, 10}, {5, 7.75}, {10, 7.75}, {10, 10}
     *                    []  {0, 2.75}, {0, 2.25}, {4.75, 2.25}, {4.75, 2.75}
     *                    []  {4.75, 7.75}, {4.75, 2.75}, {4.75, 2.25}, {5.25, 2.25}, {5.25, 7.25}, {5.25, 7.75}
     *                    []  {5.25, 7.75}, {5.25, 7.25}, {10, 7.25}, {10, 7.75}
     *                  - connection between hallway 0 and room 1 with overlap points {5.25, 4.75} and {10, 4.75}
     *                  - connection between hallway 0 and room 2 with overlap points {5.25, 5.25} and {10, 5.25}
     *                  - connection between hallway 1 and room 0 with overlap points {4.75, 4.75}, {4.75, 10}
     *                  - connection between hallway 1 and room 2 with overlap points {5.25, 5.25} and {5.25, 10}
     */

    size_t i, j;

    char* adj[] = { "Small Room", "Big Room" };
    pb_sq_house_room_spec specs[2];
    specs[0].adjacent = &adj[0];
    specs[0].area = 12.5f;
    specs[0].name = "Small Room";
    specs[0].max_instances = 2;
    specs[0].num_adjacent = 2;
    specs[0].priority = 1;

    specs[1].adjacent = &adj[0];
    specs[1].area = 37.5f;
    specs[1].name = "Big Room";
    specs[1].max_instances = 2;
    specs[1].num_adjacent = 2;
    specs[1].priority = 2;

    pb_sq_house_house_spec h;
    h.hallway_width = 0.5f;
    h.door_size = 0.75f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, specs[0].name, &specs[0]);
    pb_hashmap_put(room_spec_map, specs[1].name, &specs[1]);

    pb_room* rooms = malloc(sizeof(pb_room) * 4);
    pb_rect room_rects[] = { { { 0.f, 0.f }, 5.f, 2.5f },
                             { { 0.f, 2.5f }, 5.f, 7.5f },
                             { { 5.f, 0.f }, 5.f, 7.5f },
                             { { 5.f, 7.5f }, 5.f, 2.5f } };

    pb_rect_to_pb_shape2D(&room_rects[0], &rooms[0].shape);
    pb_vector_init(&rooms[0].walls, sizeof(int), 4);
    rooms[0].name = specs[0].name;

    pb_rect_to_pb_shape2D(&room_rects[1], &rooms[1].shape);
    pb_vector_init(&rooms[1].walls, sizeof(int), 4);
    rooms[1].name = specs[1].name;

    pb_rect_to_pb_shape2D(&room_rects[2], &rooms[2].shape);
    pb_vector_init(&rooms[2].walls, sizeof(int), 4);
    rooms[2].name = specs[1].name;

    pb_rect_to_pb_shape2D(&room_rects[3], &rooms[3].shape);
    pb_vector_init(&rooms[3].walls, sizeof(int), 4);
    rooms[3].name = specs[0].name;

    pb_floor f;
    f.rooms = rooms;
    f.num_rooms = 4;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&h, room_spec_map, &f);
    pb_graph* internal_graph = pb_sq_house_generate_internal_graph(floor_graph);

    pb_vector hallways;
    pb_vector_init(&hallways, sizeof(pb_vector), 1);

    pb_vector hallway;
    pb_point2D input_hallway_points[] = { { 0.f, 2.5f }, { 5.f, 2.5f }, { 5.f, 7.5f },  { 10.f, 7.5f } };

    size_t num_points = sizeof(input_hallway_points) / sizeof(pb_point2D);

    pb_vector_init(&hallway, sizeof(pb_edge*), num_points - 1);
    for (i = 0; i < num_points - 1; ++i) {
        pb_edge* edge = pb_graph_get_edge(internal_graph, &input_hallway_points[i], &input_hallway_points[i + 1]);
        pb_vector_push_back(&hallway, &edge);
    }
    pb_vector_push_back(&hallways, &hallway);


    pb_sq_house_place_hallways(&f, &h, room_spec_map, floor_graph, internal_graph, &hallways);
    pb_point2D hallway0_expected_points[] = { { 0.f, 2.75f },
                                              { 0.f, 2.25f },
                                              { 4.75f, 2.25f },
                                              { 4.75f, 2.75f } };
    int hallway0_expected_walls[] = { 1, 1, 0, 1 };

    pb_point2D hallway1_expected_points[] = { { 4.75f, 7.75f },
                                              { 4.75f, 2.75f },
                                              { 4.75f, 2.25f },
                                              { 5.25f, 2.25f },
                                              { 5.25f, 7.25f },
                                              { 5.25f, 7.75f } };
    int hallway1_expected_walls[] = { 1, 0, 1, 1, 0, 1 };

    pb_point2D hallway2_expected_points[] = { { 5.25f, 7.75f },
                                              { 5.25f, 7.25f },
                                              { 10.f, 7.25f },
                                              { 10.f, 7.75f } };
    int hallway2_expected_walls[] = { 0, 1, 1, 1 };

    pb_point2D const* hallway_expected_points[] = { &hallway0_expected_points[0],
                                                    &hallway1_expected_points[0],
                                                    &hallway2_expected_points[0] };
    int const* hallway_expected_walls[] = { &hallway0_expected_walls[0],
                                            &hallway1_expected_walls[0],
                                            &hallway2_expected_walls[0] };

    ck_assert_msg(f.num_rooms == 7, "floor should have had 7 rooms, had %lu", f.num_rooms);

    /* Check that hallways were added with correct shapes */
    size_t num_hallways = sizeof(hallway_expected_walls) / sizeof(int*);
    size_t first_hallway_idx = 4;
    for (i = 0; i < num_hallways; ++i) {

        pb_room* hallway_room = f.rooms + first_hallway_idx + i;
        pb_point2D* hallway_room_points = (pb_point2D*)hallway_room->shape.points.items;
        int* hallway_room_walls = (int*)hallway_room->walls.items;

        size_t j;
        for (j = 0; j < hallway_room->shape.points.size; ++j) {
            pb_point2D* real = hallway_room_points + j;
            pb_point2D* expected = hallway_expected_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "hallway %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
            ck_assert_msg(hallway_room_walls[j] == hallway_expected_walls[i][j], "hallway %d wall %d should have been %d, was %d",
                i, j, hallway_expected_walls[i][j], hallway_room_walls[j]);
        }
    }

    /* Check that rooms have the correct shapes and connections */
    pb_point2D expected_room0_points[] = { { 0.f, 2.25f },
                                           { 0.f, 0.f },
                                           { 5.f, 0.f },
                                           { 5.f, 2.25f } };
    pb_point2D expected_room1_points[] = { { 0.f, 10.f },
                                           { 0.f, 2.75f },
                                           { 4.75f, 2.75f },
                                           { 4.75f, 7.75f },
                                           { 5.f, 7.75f },
                                           { 5.f, 10.f } };
    pb_point2D expected_room2_points[] = { { 5.25f, 2.25f },
                                           { 5.f, 2.25f },
                                           { 5.f, 0.f },
                                           { 10.f, 0.f },
                                           { 10.f, 7.25f },
                                           { 5.25f, 7.25f } };
    pb_point2D expected_room3_points[] = { { 5.f, 10.f },
                                           { 5.f, 7.75f },
                                           { 10.f, 7.75f },
                                           { 10.f, 10.f } };

    size_t expected_room_point_counts[] = { sizeof(expected_room0_points) / sizeof(pb_point2D),
                                            sizeof(expected_room1_points) / sizeof(pb_point2D),
                                            sizeof(expected_room2_points) / sizeof(pb_point2D),
                                            sizeof(expected_room3_points) / sizeof(pb_point2D) };
    pb_point2D* expected_room_points[] = { &expected_room0_points[0],
                                           &expected_room1_points[0],
                                           &expected_room2_points[0],
                                           &expected_room3_points[0] };

    for (i = 0; i < sizeof(expected_room_points) / sizeof(pb_point2D*); ++i) {
        pb_room* room = f.rooms + i;
        pb_point2D* points = (pb_point2D*)room->shape.points.items;
        ck_assert_msg(room->shape.points.size == expected_room_point_counts[i], "room %d should have had %lu points, had %lu",
                      i, expected_room_point_counts[i], room->shape.points.size);
        for (j = 0; j < expected_room_point_counts[i]; ++j) {
            pb_point2D* real = points + j;
            pb_point2D* expected = expected_room_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "room %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
        }
    }

    /* Check room connections */
    pb_sq_house_room_conn expected_room0_conns[] = {
        /* room,     neighbour    overlap_start,    overlap_end,      wall,  can_connect, has_door*/
        { f.rooms,   &f.rooms[4], { 0.f, 2.25f },   { 4.75f, 2.25f }, 3,     1,           1 },
        { f.rooms,   &f.rooms[5], { 4.75f, 2.25f }, { 5.f, 2.25f },   3,     1,           0 },
        { f.rooms,   &f.rooms[2], { 5.f, 0.f },     { 5.f, 2.25f },   2,     1,           1 },
    };

    pb_sq_house_room_conn expected_room1_conns[] = {
        /* room,       neighbour    overlap_start,    overlap_end,      wall,  can_connect, has_door*/
        { &f.rooms[1], &f.rooms[4], { 0.f, 2.75f },   { 4.75f, 2.75f }, 1,     1,           1 },
        { &f.rooms[1], &f.rooms[5], { 4.75f, 2.75f }, { 4.75f, 7.75f }, 2,     1,           1 },
        { &f.rooms[1], &f.rooms[3], { 5.f, 7.75f },   { 5.f, 10.f },    4,     1,           1 },
    };

    pb_sq_house_room_conn expected_room2_conns[] = {
        /* room,        neighbour    overlap_start,    overlap_end,      wall,  can_connect, has_door*/
        { &f.rooms[1],  &f.rooms[5], { 5.25f, 2.25f }, { 5.25f, 7.25f }, 5,     1,           1 },
        { &f.rooms[1],  &f.rooms[6], { 5.25f, 7.25f }, { 10.f, 7.25f },  4,     1,           1 },
        { &f.rooms[1],  &f.rooms[0], { 5.f, 0.f },     { 5.f, 2.25f },   1,     1,           1 },
    };

    pb_sq_house_room_conn expected_room3_conns[] = {
        /* room,       neighbour    overlap_start,    overlap_end,      wall,  can_connect, has_door*/
        { &f.rooms[3], &f.rooms[5], { 5.f, 7.75f },   { 5.25f, 7.75f }, 1,     1,           0 },
        { &f.rooms[3], &f.rooms[6], { 5.25f, 7.75f }, { 10.f, 7.75f },  1,     1,           1 },
        { &f.rooms[1], &f.rooms[1], { 5.f, 7.75f },   { 5.f, 10.f },    0,     1,           1 },
    };

    pb_sq_house_room_conn expected_room4_conns[] = {
        /* room,       neighbour    overlap_start,    overlap_end,    wall,  can_connect, has_door*/
        { &f.rooms[4], &f.rooms[0], { 0.f, 2.25f }, { 4.75f, 2.25f }, 1,     1,           1 },
        { &f.rooms[4], &f.rooms[1], { 0.f, 2.75f }, { 4.75f, 2.75f }, 3,     1,           1 },
    };

    pb_sq_house_room_conn expected_room5_conns[] = {
        /* room,       neighbour    overlap_start,    overlap_end,      wall,  can_connect, has_door*/
        { &f.rooms[5], &f.rooms[0], { 4.75f, 2.25f }, { 5.f, 2.25f },   2,     1,           0 },
        { &f.rooms[5], &f.rooms[1], { 4.75f, 2.75f }, { 4.75f, 7.75f }, 0,     1,           1 },
        { &f.rooms[5], &f.rooms[2], { 5.25f, 2.25f }, { 5.25f, 7.25f }, 3,     1,           1 },
        { &f.rooms[5], &f.rooms[3], { 5.f, 7.75f },   { 5.25f, 7.75f }, 5,     1,           0 },
    };

    pb_sq_house_room_conn expected_room6_conns[] = {
        /* room,       neighbour    overlap_start,    overlap_end,     wall,  can_connect, has_door*/
        { &f.rooms[6], &f.rooms[2], { 5.25f, 7.25f }, { 10.f, 7.25f }, 1,     1,           1 },
        { &f.rooms[6], &f.rooms[3], { 5.25f, 7.75f }, { 10.f, 7.75f }, 3,     1,           1 },
    };

    size_t expected_room_conn_counts[] = {
        sizeof(expected_room0_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room1_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room2_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room3_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room4_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room5_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room6_conns) / sizeof(pb_sq_house_room_conn),
    };
    pb_sq_house_room_conn* expected_room_conns[] = {
        &expected_room0_conns[0],
        &expected_room1_conns[0],
        &expected_room2_conns[0],
        &expected_room3_conns[0],
        &expected_room4_conns[0],
        &expected_room5_conns[0],
        &expected_room6_conns[0],
    };

    size_t total_edges = 0;
    for (i = 0; i < f.num_rooms; ++i) {
        total_edges += expected_room_conn_counts[i];
    }
    ck_assert_msg(floor_graph->edges->size == total_edges, "floor graph should have had %lu edges, had %lu", total_edges,
                  floor_graph->edges->size);

    for (i = 0; i < f.num_rooms; ++i) {
        pb_vertex const* vert = pb_graph_get_vertex(floor_graph, f.rooms + i);
        ck_assert_msg(vert->edges_size == expected_room_conn_counts[i],
                      "room %lu had %lu edges, should have had %lu",
                      i, vert->edges_size, expected_room_conn_counts[i]);

        for (j = 0; j < expected_room_conn_counts[i]; ++j) {
            pb_sq_house_room_conn const* expected = expected_room_conns[i] + j;
            pb_sq_house_room_conn const* actual = (pb_sq_house_room_conn*)vert->edges[j]->data;

            ck_assert_msg(actual->neighbour == expected->neighbour, "room %lu edge %lu had incorrect neighbour", i, j);
            ck_assert_msg(pb_point_eq(&actual->overlap_start, &expected->overlap_start),
                          "room %lu edge %lu: expected overlap start (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_start.x, expected->overlap_start.y, actual->overlap_start.x, actual->overlap_start.y);
            ck_assert_msg(pb_point_eq(&actual->overlap_end, &expected->overlap_end),
                          "room %lu edge %lu: expected overlap end (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_end.x, expected->overlap_end.y, actual->overlap_end.x, actual->overlap_end.y);
            ck_assert_msg(actual->can_connect == expected->can_connect,
                          "room %lu edge %lu: expected can_connect == %lu, was %lu",
                          i, j, expected->can_connect, actual->can_connect);
            ck_assert_msg(actual->has_door == expected->has_door,
                          "room %lu edge %lu: expected has_door %d, was %d",
                          i, j, expected->has_door, actual->has_door);
            ck_assert_msg(actual->wall == expected->wall,
                          "room %lu edge %lu: expected wall %d, was %d",
                          i, j, expected->wall, actual->wall);
        }
    }

    for (i = 0; i < f.num_rooms; ++i) {
        pb_shape2D_free(&f.rooms[i].shape);
        pb_vector_free(&f.rooms[i].walls);
    }
    for (i = 0; i < hallways.size; ++i) {
        pb_vector* hallway = ((pb_vector*)hallways.items) + i;
        pb_vector_free(hallway);
    }
    pb_vector_free(&hallways);

    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(room_spec_map);
    free(f.rooms);
}
END_TEST

START_TEST(place_hallways_2_corners_same_side)
{
    /* Input: - floor with four rooms, rects
     *           []  {{0, 0}, 5, 10}
     *           []  {{5, 0}, 5, 10/3}
     *           []  {{5, 10/3}, 5, 10/3}
     *           []  {{5, 20/3}, 5, 10/3}
     *         - list of hallways with a single hallway containing three edges
     *           []  (10, 10/3)->(5, 10/3)
     *           []  (5, 10/3)->(5, 20/3)
     *           []  (5, 20/3)->(10, 20/3)
     *         - floor graph for this floor (generated by pb_sq_house_generate_floor_graph)
     *         - internal graph for this floor (generated by pb_sq_house_generate_internal_graph)
     *         - map of room specs containing 2 spec with 3 allowed instances each, can connect to each other
     *         - house spec with hallway_width = 0.5
     *
     * Expected Output: - floor contains 7 rooms with shapes
     *                    []  {0, 10}, {0, 0}, {5, 0}, {5, 10/3 - 0.25}, {4.75, 10/3 - 0.25}, {4.75, 20/3 + 0.25}, {5, 20/3 + 0.25}, {5, 10}
     *                    []  {5, 10/3 - 0.25}, {5, 0}, {10, 0}, {10, 10/3 - 0.25}
     *                    []  {5.25, 20/3 - 0.25}, {5.25, 10/3 + 0.25}, {10, 10/3 + 0.25}, {10, 20/3 - 0.25}
     *                    []  {5, 10}, {5, 20/3 + 0.25}, {10, 20/3 + 0.25}, {10, 10}
     *                    []  {5.25, 10/3 + 0.25}, {5.25, 10/3 - 0.25}, {10, 10/3 - 0.25}, {10, 10/3 + 0.25}
     *                    []  {4.75, 20/3 + 0.25}, {4.75, 10/3 - 0.25}, {5.25, 10/3 - 0.25}, {5.25, 10/3 + 0.25}, {5.25, 20/3 - 0.25}, {5.25, 20/3 + 0.25}
     *                    []  {5.25, 20/3 + 0.25}, {5.25, 20/3 - 0.25}, {10, 20/3 - 0.25}, {10, 20/3 + 0.25}
     *                  - connection between hallway 0 and room 1 with overlap points {5.25, 4.75} and {10, 4.75}
     *                  - connection between hallway 0 and room 2 with overlap points {5.25, 5.25} and {10, 5.25}
     *                  - connection between hallway 1 and room 0 with overlap points {4.75, 4.75}, {4.75, 10}
     *                  - connection between hallway 1 and room 2 with overlap points {5.25, 5.25} and {5.25, 10}
     */

    size_t i, j;

    char* adj[] = { "Small Room", "Big Room" };
    pb_sq_house_room_spec specs[2];
    specs[0].adjacent = &adj[0];
    specs[0].area = 5.f * 10.f / 3.f;
    specs[0].name = "Small Room";
    specs[0].max_instances = 3;
    specs[0].num_adjacent = 2;
    specs[0].priority = 1;

    specs[1].adjacent = &adj[0];
    specs[1].area = 50.f;
    specs[1].name = "Big Room";
    specs[1].max_instances = 2;
    specs[1].num_adjacent = 2;
    specs[1].priority = 2;

    pb_sq_house_house_spec h;
    h.hallway_width = 0.5f;
    h.door_size = 0.75f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, specs[0].name, &specs[0]);
    pb_hashmap_put(room_spec_map, specs[1].name, &specs[1]);

    pb_room* rooms = malloc(sizeof(pb_room) * 4);
    pb_rect room_rects[] = { { {0.f, 0.f }, 5.f, 10.f },
                             { {5.f, 0.f }, 5.f, 10.f / 3.f },
                             { {5.f, 10.f / 3.f }, 5.f, 10.f / 3.f },
                             { {5.f, 20.f / 3.f }, 5.f, 10.f / 3.f } };
    
    pb_rect_to_pb_shape2D(&room_rects[0], &rooms[0].shape);
    pb_vector_init(&rooms[0].walls, sizeof(int), 4);
    rooms[0].name = specs[1].name;

    pb_rect_to_pb_shape2D(&room_rects[1], &rooms[1].shape);
    pb_vector_init(&rooms[1].walls, sizeof(int), 4);
    rooms[1].name = specs[0].name;

    pb_rect_to_pb_shape2D(&room_rects[2], &rooms[2].shape);
    pb_vector_init(&rooms[2].walls, sizeof(int), 4);
    rooms[2].name = specs[0].name;

    pb_rect_to_pb_shape2D(&room_rects[3], &rooms[3].shape);
    pb_vector_init(&rooms[3].walls, sizeof(int), 4);
    rooms[3].name = specs[0].name;

    pb_floor f;
    f.rooms = rooms;
    f.num_rooms = 4;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&h, room_spec_map, &f);
    pb_graph* internal_graph = pb_sq_house_generate_internal_graph(floor_graph);

    pb_vector hallways;
    pb_vector_init(&hallways, sizeof(pb_vector), 1);

    pb_vector hallway;
    pb_point2D input_hallway_points[] = { { 10.f, 10.f / 3.f},
                                          { 5.f, 10.f / 3.f},
                                          { 5.f, 20.f / 3.f },
                                          { 10.f, 20.f / 3.f } };

    size_t num_points = sizeof(input_hallway_points) / sizeof(pb_point2D);

    pb_vector_init(&hallway, sizeof(pb_edge*), num_points - 1);
    for (i = 0; i < num_points - 1; ++i) {
        pb_edge* edge = pb_graph_get_edge(internal_graph, &input_hallway_points[i], &input_hallway_points[i + 1]);
        pb_vector_push_back(&hallway, &edge);
    }
    pb_vector_push_back(&hallways, &hallway);


    pb_sq_house_place_hallways(&f, &h, room_spec_map, floor_graph, internal_graph, &hallways);
    pb_point2D hallway0_expected_points[] = { { 5.25f, 10.f / 3.f + 0.25f },
                                              { 5.25f, 10.f / 3.f - 0.25f },
                                              { 10.f, 10.f / 3.f - 0.25f },
                                              { 10.f, 10.f / 3.f + 0.25f } };
    int hallway0_expected_walls[] = { 0, 1, 1, 1 };

    pb_point2D hallway1_expected_points[] = { { 4.75f, 20.f / 3.f + 0.25f },
                                              { 4.75f, 10.f / 3.f - 0.25f },
                                              { 5.25f, 10.f / 3.f - 0.25f },
                                              { 5.25f, 10.f / 3.f + 0.25f },
                                              { 5.25f, 20.f / 3.f - 0.25f },
                                              { 5.25f, 20.f / 3.f + 0.25f } };
    int hallway1_expected_walls[] = { 1, 1, 0, 1, 0, 1 };

    pb_point2D hallway2_expected_points[] = { { 5.25f, 20.f / 3.f + 0.25f },
                                              { 5.25f, 20.f / 3.f - 0.25f },
                                              { 10.f, 20.f / 3.f - 0.25f },
                                              { 10.f, 20.f / 3.f + 0.25f } };
    int hallway2_expected_walls[] = { 0, 1, 1, 1 };

    pb_point2D const* hallway_expected_points[] = { &hallway0_expected_points[0],
                                                    &hallway1_expected_points[0],
                                                    &hallway2_expected_points[0] };
    int const* hallway_expected_walls[] = { &hallway0_expected_walls[0],
                                            &hallway1_expected_walls[0],
                                            &hallway2_expected_walls[0] };

    ck_assert_msg(f.num_rooms == 7, "floor should have had 7 rooms, had %lu", f.num_rooms);

    /* Check that hallways were added with correct shapes */
    size_t num_hallways = sizeof(hallway_expected_walls) / sizeof(int*);
    size_t first_hallway_idx = f.num_rooms - num_hallways;
    for (i = 0; i < num_hallways; ++i) {

        pb_room* hallway_room = f.rooms + first_hallway_idx + i;
        pb_point2D* hallway_room_points = (pb_point2D*)hallway_room->shape.points.items;
        int* hallway_room_walls = (int*)hallway_room->walls.items;

        size_t j;
        for (j = 0; j < hallway_room->shape.points.size; ++j) {
            pb_point2D* real = hallway_room_points + j;
            pb_point2D* expected = hallway_expected_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "hallway %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
            ck_assert_msg(hallway_room_walls[j] == hallway_expected_walls[i][j], "hallway %d wall %d should have been %d, was %d",
                i, j, hallway_expected_walls[i][j], hallway_room_walls[j]);
        }
    }

    /* Check that rooms have the correct shapes and connections */
    pb_point2D expected_room0_points[] = { { 0.f, 10.f },
                                           { 0.f, 0.f },
                                           { 5.f, 0.f },
                                           { 5.f, 10.f / 3.f - 0.25f },
                                           { 4.75f, 10.f / 3.f - 0.25f },
                                           { 4.75f, 20.f / 3.f + 0.25f },
                                           { 5.f, 20.f / 3.f + 0.25f },
                                           { 5.f, 10.f } };
    pb_point2D expected_room1_points[] = { { 5.f, 10.f / 3.f - 0.25f },
                                           { 5.f, 0.f },
                                           { 10.f, 0.f },
                                           { 10.f, 10.f / 3.f - 0.25f } };
    pb_point2D expected_room2_points[] = { { 5.25f, 10.f / 3.f + 0.25f },
                                           { 10.f, 10.f / 3.f + 0.25f },
                                           { 10.f, 20.f / 3.f - 0.25f },
                                           { 5.25f, 20.f / 3.f - 0.25f } };
    pb_point2D expected_room3_points[] = { { 5.f, 10.f },
                                           { 5.f, 20.f / 3.f + 0.25f },
                                           { 10.f, 20.f / 3.f + 0.25f },
                                           { 10.f, 10.f } };

    size_t expected_room_point_counts[] = { sizeof(expected_room0_points) / sizeof(pb_point2D),
                                            sizeof(expected_room1_points) / sizeof(pb_point2D),
                                            sizeof(expected_room2_points) / sizeof(pb_point2D),
                                            sizeof(expected_room3_points) / sizeof(pb_point2D) };
    pb_point2D* expected_room_points[] = { &expected_room0_points[0],
        &expected_room1_points[0],
        &expected_room2_points[0],
        &expected_room3_points[0] };

    for (i = 0; i < sizeof(expected_room_points) / sizeof(pb_point2D*); ++i) {
        pb_room* room = f.rooms + i;
        pb_point2D* points = (pb_point2D*)room->shape.points.items;
        ck_assert_msg(room->shape.points.size == expected_room_point_counts[i], "room %d should have had %lu points, had %lu",
            i, expected_room_point_counts[i], room->shape.points.size);
        for (j = 0; j < expected_room_point_counts[i]; ++j) {
            pb_point2D* real = points + j;
            pb_point2D* expected = expected_room_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "room %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
        }
    }

    /* Check room connections */
    pb_sq_house_room_conn expected_room0_conns[] = {
        /* room,     neighbour    overlap_start,                 overlap_end,                   wall,  can_connect, has_door*/
        { f.rooms,   &f.rooms[5], { 4.75f, 10.f / 3.f - 0.25f }, { 4.75f, 20.f / 3.f + 0.25f }, 4,     1,           1 },
        { f.rooms,   &f.rooms[1], { 5.f, 0.f },                  { 5.f, 10.f / 3.f - 0.25f },   2,     1,           1 },
        { f.rooms,   &f.rooms[3], { 5.f, 20.f / 3.f + 0.25f },   { 5.f, 10.f },                 6,     1,           1 },
    };

    pb_sq_house_room_conn expected_room1_conns[] = {
        /* room,       neighbour    overlap_start,    overlap_end,                                wall,  can_connect, has_door*/
        { &f.rooms[1], &f.rooms[4], { 5.25f, 10.f / 3.f - 0.25f }, { 10.f, 10.f / 3.f - 0.25f },  3,     1,           1 },
        { &f.rooms[1], &f.rooms[5], { 5.f, 10.f / 3.f - 0.25f },   { 5.25f, 10.f / 3.f - 0.25f }, 3,     1,           0 },
        { &f.rooms[1], &f.rooms[0], { 5.f, 0.f },                  { 5.f, 10.f / 3.f - 0.25f },   0,     1,           1 },
    };

    pb_sq_house_room_conn expected_room2_conns[] = {
        /* room,        neighbour    overlap_start,                 overlap_end,                   wall,  can_connect, has_door*/
        { &f.rooms[2],  &f.rooms[4], { 5.25f, 10.f / 3.f + 0.25f }, { 10.f, 10.f / 3.f + 0.25f },  0,     1,           1 },
        { &f.rooms[2],  &f.rooms[5], { 5.25f, 10.f / 3.f + 0.25f }, { 5.25f, 20.f / 3.f - 0.25f }, 3,     1,           1 },
        { &f.rooms[2],  &f.rooms[6], { 5.25f, 20.f / 3.f - 0.25f }, { 10.f, 20.f / 3.f - 0.25f },  2,     1,           1 },
    };

    pb_sq_house_room_conn expected_room3_conns[] = {
        /* room,       neighbour    overlap_start,                 overlap_end,                   wall,  can_connect, has_door*/
        { &f.rooms[3], &f.rooms[5], { 5.f, 20.f / 3.f + 0.25f },   { 5.25f, 20.f / 3.f + 0.25f }, 1,     1,           0 },
        { &f.rooms[3], &f.rooms[6], { 5.25f, 20.f / 3.f + 0.25f }, { 10.f, 20.f / 3.f + 0.25f },  1,     1,           1 },
        { &f.rooms[3], &f.rooms[0], { 5.f, 20.f / 3.f + 0.25f },   { 5.f, 10.f },                 0,     1,           1 },
    };

    pb_sq_house_room_conn expected_room4_conns[] = {
        /* room,       neighbour    overlap_start,                 overlap_end,                  wall,  can_connect, has_door*/
        { &f.rooms[4], &f.rooms[1], { 5.25f, 10.f / 3.f - 0.25f }, { 10.f, 10.f / 3.f - 0.25f }, 1,     1,           1 },
        { &f.rooms[4], &f.rooms[2], { 5.25f, 10.f / 3.f + 0.25f }, { 10.f, 10.f / 3.f + 0.25f }, 3,     1,           1 },
    };

    pb_sq_house_room_conn expected_room5_conns[] = {
        /* room,       neighbour    overlap_start,                 overlap_end,                   wall,  can_connect, has_door*/
        { &f.rooms[5], &f.rooms[0], { 4.75f, 10.f / 3.f - 0.25f }, { 4.75f, 20.f / 3.f + 0.25f }, 0,     1,           1 },
        { &f.rooms[5], &f.rooms[1], { 5.f, 10.f / 3.f - 0.25f },   { 5.25f, 10.f / 3.f - 0.25f }, 1,     1,           0 },
        { &f.rooms[5], &f.rooms[2], { 5.25f, 10.f / 3.f + 0.25f }, { 5.25f, 20.f / 3.f - 0.25f }, 3,     1,           1 },
        { &f.rooms[5], &f.rooms[3], { 5.f, 20.f / 3.f + 0.25f },   { 5.25f, 20.f / 3.f + 0.25f }, 5,     1,           0 },
    };

    pb_sq_house_room_conn expected_room6_conns[] = {
        /* room,       neighbour    overlap_start,                 overlap_end,                  wall,  can_connect, has_door*/
        { &f.rooms[6], &f.rooms[2], { 5.25f, 20.f / 3.f - 0.25f }, { 10.f, 20.f / 3.f - 0.25f }, 1,     1,           1 },
        { &f.rooms[6], &f.rooms[3], { 5.25f, 20.f / 3.f + 0.25f }, { 10.f, 20.f / 3.f + 0.25f }, 3,     1,           1 },
    };

    size_t expected_room_conn_counts[] = {
        sizeof(expected_room0_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room1_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room2_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room3_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room4_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room5_conns) / sizeof(pb_sq_house_room_conn),
        sizeof(expected_room6_conns) / sizeof(pb_sq_house_room_conn),
    };
    pb_sq_house_room_conn* expected_room_conns[] = {
        &expected_room0_conns[0],
        &expected_room1_conns[0],
        &expected_room2_conns[0],
        &expected_room3_conns[0],
        &expected_room4_conns[0],
        &expected_room5_conns[0],
        &expected_room6_conns[0],
    };

    for (i = 0; i < f.num_rooms; ++i) {
        pb_vertex const* vert = pb_graph_get_vertex(floor_graph, f.rooms + i);
        ck_assert_msg(vert->edges_size == expected_room_conn_counts[i],
                      "room %lu had %lu edges, should have had %lu",
                      i, vert->edges_size, expected_room_conn_counts[i]);

        for (j = 0; j < expected_room_conn_counts[i]; ++j) {
            pb_sq_house_room_conn const* expected = expected_room_conns[i] + j;
            pb_sq_house_room_conn const* actual = (pb_sq_house_room_conn*)vert->edges[j]->data;

            ck_assert_msg(actual->neighbour == expected->neighbour, "room %lu edge %lu had incorrect neighbour", i, j);
            ck_assert_msg(pb_point_eq(&actual->overlap_start, &expected->overlap_start),
                          "room %lu edge %lu: expected overlap start (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_start.x, expected->overlap_start.y, actual->overlap_start.x, actual->overlap_start.y);
            ck_assert_msg(pb_point_eq(&actual->overlap_end, &expected->overlap_end),
                          "room %lu edge %lu: expected overlap end (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_end.x, expected->overlap_end.y, actual->overlap_end.x, actual->overlap_end.y);
            ck_assert_msg(actual->can_connect == expected->can_connect,
                          "room %lu edge %lu: expected can_connect == %lu, was %lu",
                          i, j, expected->can_connect, actual->can_connect);
            ck_assert_msg(actual->has_door == expected->has_door,
                          "room %lu edge %lu: expected has_door %d, was %d",
                          i, j, expected->has_door, actual->has_door);
            ck_assert_msg(actual->wall == expected->wall,
                          "room %lu edge %lu: expected wall %d, was %d",
                          i, j, expected->wall, actual->wall);
        }
    }

    for (i = 0; i < f.num_rooms; ++i) {
        pb_shape2D_free(&f.rooms[i].shape);
        pb_vector_free(&f.rooms[i].walls);
    }
    for (i = 0; i < hallways.size; ++i) {
        pb_vector* hallway = ((pb_vector*)hallways.items) + i;
        pb_vector_free(hallway);
    }
    pb_vector_free(&hallways);

    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(room_spec_map);
    free(f.rooms);
}
END_TEST

START_TEST(place_hallways_t_intersection_y_axis)
{
    /* Input: - floor with three rooms, rects
     *           []  {{0, 0}, 5, 10}
     *           []  {{5, 0}, 5, 5}
     *           []  {{5, 5}, 5, 5}
     *         - list of hallways with two hallways
     *           []  hallway 0
     *               (+) (10, 5)->(5, 5)
     *               (+) (5, 5)->(5, 10)
     *           []  hallway 1
     *               (+) (5, 0)->(5, 5)
     *         - floor graph for this floor (generated by pb_sq_house_generate_floor_graph)
     *         - internal graph for this floor (generated by pb_sq_house_generate_internal_graph)
     *         - map of room specs containing 2 spec with 2 allowed instances each, can connect to each other
     *         - house spec with hallway_width = 0.5
     *
     * Expected Output: - floor contains 5 rooms with shapes
     *                    []  {0, 10}, {0, 0}, {4.75, 0}, {4.75, 10}
     *                    []  {5.25, 4.75}, {5.25, 0}, {10, 0}, {10, 4.75}
     *                    []  {5.25, 10}, {5.25, 5.25}, {10, 5.25}, {10, 10}
     *                    []  {4.75, 10}, {4.75, 0}, {5.25, 0}, {5.25, 4.75}, {5.25, 5.25}, {5.25, 10}
     *                    []  {5.25, 5.25}, {5.25, 4.75}, {10, 4.75}, {10, 5.25}
     *                  - connection between hallway 0 and room 1 with overlap points {5.25, 4.75} and {10, 4.75}
     *                  - connection between hallway 0 and room 2 with overlap points {5.25, 5.25} and {10, 5.25}
     *                  - connection between hallway 1 and room 0 with overlap points {4.75, 4.75}, {4.75, 10}
     *                  - connection between hallway 1 and room 2 with overlap points {5.25, 5.25} and {5.25, 10}
     */

    size_t i, j; /* We're going to need these a lot, so might as well just put them here... */

    char* adj[] = { "Small Room", "Big Room" };
    pb_sq_house_room_spec specs[2];
    specs[0].adjacent = &adj[0];
    specs[0].area = 25.f;
    specs[0].name = "Small Room";
    specs[0].max_instances = 2;
    specs[0].num_adjacent = 2;
    specs[0].priority = 1;

    specs[1].adjacent = &adj[0];
    specs[1].area = 50.f;
    specs[1].name = "Big Room";
    specs[1].max_instances = 2;
    specs[1].num_adjacent = 2;
    specs[1].priority = 2;

    pb_sq_house_house_spec h;
    h.hallway_width = 0.5f;
    h.door_size = 0.75f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, specs[0].name, &specs[0]);
    pb_hashmap_put(room_spec_map, specs[1].name, &specs[1]);

    pb_room* rooms = malloc(sizeof(pb_room) * 3);
    pb_rect room_rects[] = { { { 0, 0 }, 5, 10 },
                             { { 5, 0 }, 5, 5 },
                             { { 5, 5 }, 5, 5 } };

    pb_rect_to_pb_shape2D(&room_rects[0], &rooms[0].shape);
    pb_vector_init(&rooms[0].walls, sizeof(int), 4);
    rooms[0].name = specs[1].name;

    pb_rect_to_pb_shape2D(&room_rects[1], &rooms[1].shape);
    pb_vector_init(&rooms[1].walls, sizeof(int), 4);
    rooms[1].name = specs[0].name;

    pb_rect_to_pb_shape2D(&room_rects[2], &rooms[2].shape);
    pb_vector_init(&rooms[2].walls, sizeof(int), 4);
    rooms[2].name = specs[0].name;

    pb_floor f;
    f.rooms = rooms;
    f.num_rooms = 3;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&h, room_spec_map, &f);
    pb_graph* internal_graph = pb_sq_house_generate_internal_graph(floor_graph);

    pb_vector hallways;
    pb_vector_init(&hallways, sizeof(pb_vector), 2);

    pb_vector input_hallways[2];
    pb_point2D input_hallway0_points[] = { { 10.f, 5.f }, { 5.f, 5.f }, { 5.f, 10.f } };
    pb_point2D input_hallway1_points[] = { { 5.f, 0.f }, { 5.f, 5.f } };
    size_t input_hallway_point_counts[] = { sizeof(input_hallway0_points) / sizeof(pb_point2D),
                                            sizeof(input_hallway1_points) / sizeof(pb_point2D)};
    pb_point2D const* input_hallway_points[2] = { &input_hallway0_points[0], &input_hallway1_points[0] };
   
    for (i = 0; i < sizeof(input_hallways) / sizeof(pb_vector); ++i) {
        pb_vector_init(&input_hallways[i], sizeof(pb_edge*), input_hallway_point_counts[i] - 1);
        for (j = 0; j < input_hallway_point_counts[i] - 1; ++j) {
            pb_edge* edge = pb_graph_get_edge(internal_graph, &input_hallway_points[i][j], &input_hallway_points[i][j + 1]);
            pb_vector_push_back(&input_hallways[i], &edge);
        }
        pb_vector_push_back(&hallways, &input_hallways[i]);
    }

    pb_sq_house_place_hallways(&f, &h, room_spec_map, floor_graph, internal_graph, &hallways);
    pb_point2D hallway0_expected_points[] = { { 5.25f, 5.25f },
                                              { 5.25f, 4.75f },
                                              { 10.f, 4.75f },
                                              { 10.f, 5.25f } };
    int hallway0_expected_walls[] = { 0, 1, 1, 1 };

    pb_point2D hallway1_expected_points[] = { { 4.75f, 10.f },
                                              { 4.75f, 0.f },
                                              { 5.25f, 0.f },
                                              { 5.25f, 4.75f },
                                              { 5.25f, 5.25f },
                                              { 5.25f, 10.f } };
    int hallway1_expected_walls[] = { 1, 1, 1, 0, 1, 1 };

    pb_point2D const* hallway_expected_points[] = { &hallway0_expected_points[0],
                                                    &hallway1_expected_points[0] };
    int const* hallway_expected_walls[] = { &hallway0_expected_walls[0],
                                            &hallway1_expected_walls[0] };

    ck_assert_msg(f.num_rooms == 5, "floor should have had 5 rooms, had %lu", f.num_rooms);

    /* Check that hallways were added with correct shapes */
    size_t num_hallways = sizeof(hallway_expected_walls) / sizeof(int*);
    size_t first_hallway_idx = f.num_rooms - num_hallways;
    for (i = 0; i < num_hallways; ++i) {

        pb_room* hallway_room = f.rooms + first_hallway_idx + i;
        pb_point2D* hallway_room_points = (pb_point2D*)hallway_room->shape.points.items;
        int* hallway_room_walls = (int*)hallway_room->walls.items;

        size_t j;
        for (j = 0; j < hallway_room->shape.points.size; ++j) {
            pb_point2D* real = hallway_room_points + j;
            pb_point2D* expected = hallway_expected_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "hallway %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
            ck_assert_msg(hallway_room_walls[j] == hallway_expected_walls[i][j], "hallway %d wall %d should have been %d, was %d",
                i, j, hallway_expected_walls[i][j], hallway_room_walls[j]);
        }
    }

    /* Check that rooms have the correct shapes and connections */
    pb_point2D expected_room0_points[] = { { 0.f, 10.f },
                                           { 0.f, 0.f },
                                           { 4.75f, 0.f },
                                           { 4.75f, 10.f } };
    pb_point2D expected_room1_points[] = { { 5.25f, 0.f },
                                           { 10.f, 0.f },
                                           { 10.f, 4.75f },
                                           { 5.25f, 4.75f } };
    pb_point2D expected_room2_points[] = { { 5.25f, 10.f },
                                           { 5.25f, 5.25f },
                                           { 10.f, 5.25f },
                                           { 10.f, 10.f } };

    size_t expected_room_point_counts[] = { sizeof(expected_room0_points) / sizeof(pb_point2D),
        sizeof(expected_room1_points) / sizeof(pb_point2D),
        sizeof(expected_room2_points) / sizeof(pb_point2D)};
    pb_point2D* expected_room_points[] = { &expected_room0_points[0],
        &expected_room1_points[0],
        &expected_room2_points[0]};

    for (i = 0; i < sizeof(expected_room_points) / sizeof(pb_point2D*); ++i) {
        pb_room* room = f.rooms + i;
        pb_point2D* points = (pb_point2D*)room->shape.points.items;
        ck_assert_msg(room->shape.points.size == expected_room_point_counts[i], "room %d should have had %lu points, had %lu",
            i, expected_room_point_counts[i], room->shape.points.size);
        for (j = 0; j < expected_room_point_counts[i]; ++j) {
            pb_point2D* real = points + j;
            pb_point2D* expected = expected_room_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "room %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
        }
    }

    /* Check room connections */
    pb_sq_house_room_conn expected_room0_conns[] = {
            /* room,     neighbour    overlap_start,   overlap_end,    wall, can_connect, has_door*/
            { f.rooms,   &f.rooms[4], { 4.75f, 0.f }, { 4.75f, 10.f }, 2,    1,           1 },
    };

    pb_sq_house_room_conn expected_room1_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,      wall, can_connect, has_door*/
            { &f.rooms[1], &f.rooms[3], { 5.25f, 4.75f }, { 10.f, 4.75f },  2,    1,           1 },
            { &f.rooms[1], &f.rooms[4], { 5.25f, 0.f },   { 5.25f, 4.75f }, 3,    1,           1 },
    };

    pb_sq_house_room_conn expected_room2_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,     wall, can_connect, has_door*/
            { &f.rooms[2], &f.rooms[3], { 5.25f, 5.25f }, { 10.f, 5.25f }, 1,    1,           1 },
            { &f.rooms[2], &f.rooms[4], { 5.25f, 5.25f }, { 5.25f, 10.f }, 0,    1,           1 },
    };

    pb_sq_house_room_conn expected_room3_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,     wall, can_connect, has_door*/
            { &f.rooms[3], &f.rooms[1], { 5.25f, 4.75f }, { 10.f, 4.75f }, 1,    1,           1 },
            { &f.rooms[3], &f.rooms[2], { 5.25f, 5.25f }, { 10.f, 5.25f }, 3,    1,           1 },
    };

    pb_sq_house_room_conn expected_room4_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,      wall, can_connect, has_door*/
            { &f.rooms[4], &f.rooms[0], { 4.75f, 0.f },   { 4.75f, 10.f },  0,    1,           1 },
            { &f.rooms[4], &f.rooms[1], { 5.25f, 0.f },   { 5.25f, 4.75f }, 2,    1,           1 },
            { &f.rooms[4], &f.rooms[2], { 5.25f, 5.25f }, { 5.25f, 10.f },  4,    1,           1 },
    };

    size_t expected_room_conn_counts[] = {
            sizeof(expected_room0_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room1_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room2_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room3_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room4_conns) / sizeof(pb_sq_house_room_conn),
    };
    pb_sq_house_room_conn* expected_room_conns[] = {
            &expected_room0_conns[0],
            &expected_room1_conns[0],
            &expected_room2_conns[0],
            &expected_room3_conns[0],
            &expected_room4_conns[0],
    };

    for (i = 0; i < f.num_rooms; ++i) {
        pb_vertex const* vert = pb_graph_get_vertex(floor_graph, f.rooms + i);
        ck_assert_msg(vert->edges_size == expected_room_conn_counts[i],
                      "room %lu had %lu edges, should have had %lu",
                      i, vert->edges_size, expected_room_conn_counts[i]);

        for (j = 0; j < expected_room_conn_counts[i]; ++j) {
            pb_sq_house_room_conn const* expected = expected_room_conns[i] + j;
            pb_sq_house_room_conn const* actual = (pb_sq_house_room_conn*)vert->edges[j]->data;

            ck_assert_msg(actual->neighbour == expected->neighbour, "room %lu edge %lu had incorrect neighbour", i, j);
            ck_assert_msg(pb_point_eq(&actual->overlap_start, &expected->overlap_start),
                          "room %lu edge %lu: expected overlap start (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_start.x, expected->overlap_start.y, actual->overlap_start.x, actual->overlap_start.y);
            ck_assert_msg(pb_point_eq(&actual->overlap_end, &expected->overlap_end),
                          "room %lu edge %lu: expected overlap end (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_end.x, expected->overlap_end.y, actual->overlap_end.x, actual->overlap_end.y);
            ck_assert_msg(actual->can_connect == expected->can_connect,
                          "room %lu edge %lu: expected can_connect == %lu, was %lu",
                          i, j, expected->can_connect, actual->can_connect);
            ck_assert_msg(actual->has_door == expected->has_door,
                          "room %lu edge %lu: expected has_door %d, was %d",
                          i, j, expected->has_door, actual->has_door);
            ck_assert_msg(actual->wall == expected->wall,
                          "room %lu edge %lu: expected wall %d, was %d",
                          i, j, expected->wall, actual->wall);
        }
    }

    for (i = 0; i < f.num_rooms; ++i) {
        pb_shape2D_free(&f.rooms[i].shape);
        pb_vector_free(&f.rooms[i].walls);
    }
    for (i = 0; i < hallways.size; ++i) {
        pb_vector* hallway = ((pb_vector*)hallways.items) + i;
        pb_vector_free(hallway);
    }
    pb_vector_free(&hallways);

    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(room_spec_map);
    free(f.rooms);
}
END_TEST

START_TEST(place_hallways_t_intersection_x_axis)
{
    /* Input: - floor with four rooms, rects
     *           []  {{0, 0}, 5, 5}
     *           []  {{0, 5}, 5, 5}
     *           []  {{5, 0}, 5, 5}
     *           []  {{5, 5}, 5, 5}
     *         - list of hallways with two hallways
     *           []  hallway 0
     *               (+) (10, 5)->(5, 5)
     *               (+) (5, 5)->(0, 5)
     *           []  hallway 1
     *               (+) (5, 5)->(5, 10)
     *         - floor graph for this floor (generated by pb_sq_house_generate_floor_graph)
     *         - internal graph for this floor (generated by pb_sq_house_generate_internal_graph)
     *         - map of room specs containing 1 spec with 4 allowed instances each, can connect to each other
     *         - house spec with hallway_width = 0.5
     *
     * Expected Output: - floor contains 6 rooms with shapes
     *                    []  {0, 4.75}, {0, 0}, {5, 0}, {5, 4.75}
     *                    []  {0, 10}, {0, 5.25}, {4.75, 5.25}, {4.75, 10}
     *                    []  {5, 4.75}, {5, 0}, {10, 0}, {10, 4.75}
     *                    []  {5.25, 10}, {5.25, 5.25}, {10, 5.25}, {10, 10}
     *                    []  {0, 5.25}, {0, 4.75}, {10, 4.75}, {10, 5.25}, {5.25, 5.25}, {4.75, 5.25}
     *                    []  {4.75, 10}, {4.75, 5.25}, {5.25, 5.25}, {5.25, 10}
     *                  - connection between hallway 0 and room 1 with overlap points {5.25, 4.75} and {10, 4.75}
     *                  - connection between hallway 0 and room 2 with overlap points {5.25, 5.25} and {10, 5.25}
     *                  - connection between hallway 1 and room 0 with overlap points {4.75, 4.75}, {4.75, 10}
     *                  - connection between hallway 1 and room 2 with overlap points {5.25, 5.25} and {5.25, 10}
     */

    size_t i, j;

    char* adj[] = { "Small Room" };
    pb_sq_house_room_spec specs[2];
    specs[0].adjacent = &adj[0];
    specs[0].area = 25.f;
    specs[0].name = "Small Room";
    specs[0].max_instances = 4;
    specs[0].num_adjacent = 1;
    specs[0].priority = 1;

    pb_sq_house_house_spec h;
    h.hallway_width = 0.5f;
    h.door_size = 0.75f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, specs[0].name, &specs[0]);

    pb_room* rooms = malloc(sizeof(pb_room) * 4);
    pb_rect room_rects[] = { { { 0.f, 0.f }, 5.f, 5.f },
                             { { 0.f, 5.f }, 5.f, 5.f },
                             { { 5.f, 0.f }, 5.f, 5.f },
                             { { 5.f, 5.f }, 5.f, 5.f } };

    for (i = 0; i < 4; ++i) {
        pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
        pb_vector_init(&rooms[i].walls, sizeof(int), 4);
        rooms[i].name = specs[0].name;
    }

    pb_floor f;
    f.rooms = rooms;
    f.num_rooms = 4;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&h, room_spec_map, &f);
    pb_graph* internal_graph = pb_sq_house_generate_internal_graph(floor_graph);

    pb_vector hallways;
    pb_vector_init(&hallways, sizeof(pb_vector), 2);

    pb_vector input_hallways[2];
    pb_point2D input_hallway0_points[] = { { 10.f, 5.f },
                                           { 5.f, 5.f },
                                           { 0.f, 5.f } };
    pb_point2D input_hallway1_points[] = { { 5.f, 5.f },
                                           { 5.f, 10.f }};
    size_t input_hallway_point_counts[] = { sizeof(input_hallway0_points) / sizeof(pb_point2D),
                                            sizeof(input_hallway1_points) / sizeof(pb_point2D) };
    pb_point2D const* input_hallway_points[2] = { &input_hallway0_points[0], &input_hallway1_points[0] };

    for (i = 0; i < sizeof(input_hallways) / sizeof(pb_vector); ++i) {
        pb_vector_init(&input_hallways[i], sizeof(pb_edge*), input_hallway_point_counts[i] - 1);
        for (j = 0; j < input_hallway_point_counts[i] - 1; ++j) {
            pb_edge* edge = pb_graph_get_edge(internal_graph, &input_hallway_points[i][j], &input_hallway_points[i][j + 1]);
            pb_vector_push_back(&input_hallways[i], &edge);
        }
        pb_vector_push_back(&hallways, &input_hallways[i]);
    }

    pb_sq_house_place_hallways(&f, &h, room_spec_map, floor_graph, internal_graph, &hallways);
    pb_point2D hallway0_expected_points[] = { { 0.f, 5.25f },
                                              { 0.f, 4.75f },
                                              { 10.f, 4.75f },
                                              { 10.f, 5.25f },
                                              { 5.25f, 5.25f },
                                              { 4.75f, 5.25f } };
    int hallway0_expected_walls[] = { 1, 1, 1, 1, 0, 1 };

    pb_point2D hallway1_expected_points[] = { { 4.75f, 10.f },
                                              { 4.75f, 5.25f },
                                              { 5.25f, 5.25f },
                                              { 5.25f, 10.f } };
    int hallway1_expected_walls[] = { 1, 0, 1, 1 };

    pb_point2D const* hallway_expected_points[] = { &hallway0_expected_points[0],
                                                    &hallway1_expected_points[0] };
    int const* hallway_expected_walls[] = { &hallway0_expected_walls[0],
                                            &hallway1_expected_walls[0] };

    ck_assert_msg(f.num_rooms == 6, "floor should have had 6 rooms, had %lu", f.num_rooms);

    /* Check that hallways were added with correct shapes */
    size_t num_hallways = sizeof(hallway_expected_walls) / sizeof(int*);
    size_t first_hallway_idx = f.num_rooms - num_hallways;
    for (i = 0; i < num_hallways; ++i) {

        pb_room* hallway_room = f.rooms + first_hallway_idx + i;
        pb_point2D* hallway_room_points = (pb_point2D*)hallway_room->shape.points.items;
        int* hallway_room_walls = (int*)hallway_room->walls.items;

        for (j = 0; j < hallway_room->shape.points.size; ++j) {
            pb_point2D* real = hallway_room_points + j;
            pb_point2D* expected = hallway_expected_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "hallway %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
            ck_assert_msg(hallway_room_walls[j] == hallway_expected_walls[i][j], "hallway %d wall %d should have been %d, was %d",
                i, j, hallway_expected_walls[i][j], hallway_room_walls[j]);
        }
    }

    /* Check that rooms have the correct shapes and connections */
    pb_point2D expected_room0_points[] = { { 0.f, 4.75f },
                                           { 0.f, 0.f },
                                           { 5.f, 0.f },
                                           { 5.f, 4.75f } };
    pb_point2D expected_room1_points[] = { { 0.f, 10.f },
                                           { 0.f, 5.25f },
                                           { 4.75f, 5.25f },
                                           { 4.75f, 10.f } };
    pb_point2D expected_room2_points[] = { { 5.f, 4.75f },
                                           { 5.f, 0.f },
                                           { 10.f, 0.f },
                                           { 10.f, 4.75f } };
    pb_point2D expected_room3_points[] = { { 5.25f, 10.f },
                                           { 5.25f, 5.25f },
                                           { 10.f, 5.25f },
                                           { 10.f, 10.f } };

    size_t expected_room_point_counts[] = { sizeof(expected_room0_points) / sizeof(pb_point2D),
                                            sizeof(expected_room1_points) / sizeof(pb_point2D),
                                            sizeof(expected_room2_points) / sizeof(pb_point2D),
                                            sizeof(expected_room3_points) / sizeof(pb_point2D) };
    pb_point2D* expected_room_points[] = { &expected_room0_points[0],
                                           &expected_room1_points[0],
                                           &expected_room2_points[0],
                                           &expected_room3_points[0] };

    for (i = 0; i < sizeof(expected_room_points) / sizeof(pb_point2D*); ++i) {
        pb_room* room = f.rooms + i;
        pb_point2D* points = (pb_point2D*)room->shape.points.items;
        ck_assert_msg(room->shape.points.size == expected_room_point_counts[i], "room %d should have had %lu points, had %lu",
            i, expected_room_point_counts[i], room->shape.points.size);
        for (j = 0; j < expected_room_point_counts[i]; ++j) {
            pb_point2D* real = points + j;
            pb_point2D* expected = expected_room_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "room %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
        }
    }

    /* Check room connections */
    pb_sq_house_room_conn expected_room0_conns[] = {
            /* room,     neighbour    overlap_start,  overlap_end,    wall, can_connect, has_door*/
            { f.rooms,   &f.rooms[4], { 0.f, 4.75f }, { 5.f, 4.75f }, 3,    1,           1 },
            { f.rooms,   &f.rooms[2], { 5.f, 0.f },   { 5.f, 4.75f }, 2,    1,           1 },
    };

    pb_sq_house_room_conn expected_room1_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,      wall, can_connect, has_door*/
            { &f.rooms[1], &f.rooms[4], { 0.f, 5.25f },   { 4.75f, 5.25f }, 1,    1,           1 },
            { &f.rooms[1], &f.rooms[5], { 4.75f, 5.25f }, { 4.75f, 10.f },  2,    1,           1 },
    };

    pb_sq_house_room_conn expected_room2_conns[] = {
            /* room,        neighbour    overlap_start,  overlap_end,     wall,  can_connect, has_door*/
            { &f.rooms[2],  &f.rooms[4], { 5.f, 4.75f }, { 10.f, 4.75f }, 3,     1,           1 },
            { &f.rooms[2],  &f.rooms[0], { 5.f, 0.f },   { 5.f, 4.75f },  0,     1,           1 },
    };

    pb_sq_house_room_conn expected_room3_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,     wall,  can_connect, has_door*/
            { &f.rooms[3], &f.rooms[4], { 5.25f, 5.25f }, { 10.f, 5.25f }, 1,     1,           1 },
            { &f.rooms[3], &f.rooms[5], { 5.25f, 5.25f }, { 5.25f, 10.f }, 0,     1,           1 },
    };

    pb_sq_house_room_conn expected_room4_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,      wall, can_connect, has_door*/
            { &f.rooms[4], &f.rooms[0], { 0.f, 4.75f },   { 5.f, 4.75f },   1,    1,           1 },
            { &f.rooms[4], &f.rooms[1], { 0.f, 5.25f },   { 4.75f, 5.25f }, 5,    1,           1 },
            { &f.rooms[4], &f.rooms[2], { 5.f, 4.75f },   { 10.f, 4.75f },  1,    1,           1 },
            { &f.rooms[4], &f.rooms[3], { 5.25f, 5.25f }, { 10.f, 5.25f },  3,    1,           1 },
    };

    pb_sq_house_room_conn expected_room5_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,     wall, can_connect, has_door*/
            { &f.rooms[5], &f.rooms[1], { 4.75f, 5.25f }, { 4.75f, 10.f }, 0,    1,           1 },
            { &f.rooms[5], &f.rooms[3], { 5.25f, 5.25f }, { 5.25f, 10.f }, 2,    1,           1 },
    };

    size_t expected_room_conn_counts[] = {
            sizeof(expected_room0_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room1_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room2_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room3_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room4_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room5_conns) / sizeof(pb_sq_house_room_conn),
    };
    pb_sq_house_room_conn* expected_room_conns[] = {
            &expected_room0_conns[0],
            &expected_room1_conns[0],
            &expected_room2_conns[0],
            &expected_room3_conns[0],
            &expected_room4_conns[0],
            &expected_room5_conns[0],
    };

    for (i = 0; i < f.num_rooms; ++i) {
        pb_vertex const* vert = pb_graph_get_vertex(floor_graph, f.rooms + i);
        ck_assert_msg(vert->edges_size == expected_room_conn_counts[i],
                      "room %lu had %lu edges, should have had %lu",
                      i, vert->edges_size, expected_room_conn_counts[i]);

        for (j = 0; j < expected_room_conn_counts[i]; ++j) {
            pb_sq_house_room_conn const* expected = expected_room_conns[i] + j;
            pb_sq_house_room_conn const* actual = (pb_sq_house_room_conn*)vert->edges[j]->data;

            ck_assert_msg(actual->neighbour == expected->neighbour, "room %lu edge %lu had incorrect neighbour", i, j);
            ck_assert_msg(pb_point_eq(&actual->overlap_start, &expected->overlap_start),
                          "room %lu edge %lu: expected overlap start (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_start.x, expected->overlap_start.y, actual->overlap_start.x, actual->overlap_start.y);
            ck_assert_msg(pb_point_eq(&actual->overlap_end, &expected->overlap_end),
                          "room %lu edge %lu: expected overlap end (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_end.x, expected->overlap_end.y, actual->overlap_end.x, actual->overlap_end.y);
            ck_assert_msg(actual->can_connect == expected->can_connect,
                          "room %lu edge %lu: expected can_connect == %d, was %d",
                          i, j, expected->can_connect, actual->can_connect);
            ck_assert_msg(actual->has_door == expected->has_door,
                          "room %lu edge %lu: expected has_door %d, was %d",
                          i, j, expected->has_door, actual->has_door);
            ck_assert_msg(actual->wall == expected->wall,
                          "room %lu edge %lu: expected wall %d, was %d",
                          i, j, expected->wall, actual->wall);
        }
    }

    /* Check that room shapes were correctly adjusted */
    for (i = 0; i < f.num_rooms; ++i) {
        pb_shape2D_free(&f.rooms[i].shape);
        pb_vector_free(&f.rooms[i].walls);
    }
    for (i = 0; i < hallways.size; ++i) {
        pb_vector* hallway = ((pb_vector*)hallways.items) + i;
        pb_vector_free(hallway);
    }
    pb_vector_free(&hallways);

    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(room_spec_map);
    free(f.rooms);
}
END_TEST

START_TEST(place_hallways_4_way_intersection)
{
    /* Input: - floor with four rooms, rects
     *           []  {{0, 0}, 5, 5}
     *           []  {{0, 5}, 5, 5}
     *           []  {{5, 0}, 5, 5}
     *           []  {{5, 5}, 5, 5}
     *         - list of hallways with a two hallways
     *           []  hallway 0
     *               (+) (10, 5)->(5, 5)
     *               (+) (5, 5)->(0, 5)
     *           []  hallway 1
     *               (+) (5, 0)->(5, 5)
     *               (+) (5, 5)->(5, 10)
     *         - floor graph for this floor (generated by pb_sq_house_generate_floor_graph)
     *         - internal graph for this floor (generated by pb_sq_house_generate_internal_graph)
     *         - map of room specs containing 1 spec with 4 allowed instances each, can connect to each other
     *         - house spec with hallway_width = 0.5
     *
     * Expected Output: - floor contains 7 rooms with shapes
     *                    []  {0, 4.75}, {0, 0}, {4.75, 0}, {4.75, 4.25}
     *                    []  {0, 10}, {0, 5.25}, {4.75, 5.25}, {4.75, 10}
     *                    []  {5.25, 4.75}, {5.25, 0}, {10, 0}, {10, 4.75}
     *                    []  {5.25, 10}, {5.25, 5.25}, {10, 5.25}, {10, 10}
     *                    []  {0, 5.25}, {0, 4.75}, {4.75, 4.75}, {5.25, 4.75}
     *                    []  {5.25, 5.25}, {5.25, 4.75}, {10, 4.75}, {10, 5.25}
     *                    []  {4.75, 10}, {4.75, 5.25}, {4.75, 4.75}, {4.75, 0}, {5.25, 0}, {5.25, 4.75}, {5.25, 5.25}, {5.25, 10}
     *                  - connection between hallway 0 and room 1 with overlap points {5.25, 4.75} and {10, 4.75}
     *                  - connection between hallway 0 and room 2 with overlap points {5.25, 5.25} and {10, 5.25}
     *                  - connection between hallway 1 and room 0 with overlap points {4.75, 4.75}, {4.75, 10}
     *                  - connection between hallway 1 and room 2 with overlap points {5.25, 5.25} and {5.25, 10}
     */

    size_t i, j;

    char* adj[] = { "Small Room" };
    pb_sq_house_room_spec specs[2];
    specs[0].adjacent = &adj[0];
    specs[0].area = 25.f;
    specs[0].name = "Small Room";
    specs[0].max_instances = 4;
    specs[0].num_adjacent = 1;
    specs[0].priority = 1;

    pb_sq_house_house_spec h;
    h.hallway_width = 0.5f;
    h.door_size = 0.75f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, specs[0].name, &specs[0]);

    pb_room* rooms = malloc(sizeof(pb_room) * 4);
    pb_rect room_rects[] = { { { 0.f, 0.f }, 5.f, 5.f },
                             { { 0.f, 5.f }, 5.f, 5.f },
                             { { 5.f, 0.f }, 5.f, 5.f },
                             { { 5.f, 5.f }, 5.f, 5.f } };

    for (i = 0; i < 4; ++i) {
        pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
        pb_vector_init(&rooms[i].walls, sizeof(int), 4);
        rooms[i].name = specs[0].name;
    }

    pb_floor f;
    f.rooms = rooms;
    f.num_rooms = 4;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&h, room_spec_map, &f);
    pb_graph* internal_graph = pb_sq_house_generate_internal_graph(floor_graph);

    pb_vector hallways;
    pb_vector_init(&hallways, sizeof(pb_vector), 2);

    pb_vector input_hallways[2];
    pb_point2D input_hallway0_points[] = { { 10.f, 5.f }, { 5.f, 5.f }, { 0.f, 5.f } };
    pb_point2D input_hallway1_points[] = { { 5.f, 0.f }, { 5.f, 5.f }, { 5.f, 10.f } };
    size_t input_hallway_point_counts[] = { sizeof(input_hallway0_points) / sizeof(pb_point2D),
                                            sizeof(input_hallway1_points) / sizeof(pb_point2D) };
    pb_point2D const* input_hallway_points[2] = { &input_hallway0_points[0], &input_hallway1_points[0] };

    for (i = 0; i < sizeof(input_hallways) / sizeof(pb_vector); ++i) {
        pb_vector_init(&input_hallways[i], sizeof(pb_edge*), input_hallway_point_counts[i] - 1);
        for (j = 0; j < input_hallway_point_counts[i] - 1; ++j) {
            pb_edge* edge = pb_graph_get_edge(internal_graph, &input_hallway_points[i][j], &input_hallway_points[i][j + 1]);
            pb_vector_push_back(&input_hallways[i], &edge);
        }
        pb_vector_push_back(&hallways, &input_hallways[i]);
    }

    pb_sq_house_place_hallways(&f, &h, room_spec_map, floor_graph, internal_graph, &hallways);
    pb_point2D hallway0_expected_points[] = { { 0.f, 5.25f },
                                              { 0.f, 4.75f },
                                              { 4.75f, 4.75f },
                                              { 4.75f, 5.25f } };
    int hallway0_expected_walls[] = { 1, 1, 0, 1 };

    pb_point2D hallway1_expected_points[] = { { 5.25f, 5.25f },
                                              { 5.25f, 4.75f },
                                              { 10.f, 4.75f },
                                              { 10.f, 5.25f } };
    int hallway1_expected_walls[] = { 0, 1, 1, 1 };

    pb_point2D hallway2_expected_points[] = { { 4.75f, 10.f },
                                              { 4.75f, 5.25f },
                                              { 4.75f, 4.75f },
                                              { 4.75f, 0.f },
                                              { 5.25f, 0.f },
                                              { 5.25f, 4.75f },
                                              { 5.25f, 5.25f },
                                              { 5.25f, 10.f } };
    int hallway2_expected_walls[] = { 1, 0, 1, 1, 1, 0, 1, 1 };

    pb_point2D const* hallway_expected_points[] = { &hallway0_expected_points[0],
                                                    &hallway1_expected_points[0],
                                                    &hallway2_expected_points[0] };
    int const* hallway_expected_walls[] = { &hallway0_expected_walls[0],
                                            &hallway1_expected_walls[0],
                                            &hallway2_expected_walls[0] };

    ck_assert_msg(f.num_rooms == 7, "floor should have had 7 rooms, had %lu", f.num_rooms);

    /* Check that hallways were added with correct shapes */
    size_t num_hallways = sizeof(hallway_expected_walls) / sizeof(int*);
    size_t first_hallway_idx = f.num_rooms - num_hallways;
    for (i = 0; i < num_hallways; ++i) {

        pb_room* hallway_room = f.rooms + first_hallway_idx + i;
        pb_point2D* hallway_room_points = (pb_point2D*)hallway_room->shape.points.items;
        int* hallway_room_walls = (int*)hallway_room->walls.items;

        for (j = 0; j < hallway_room->shape.points.size; ++j) {
            pb_point2D* real = hallway_room_points + j;
            pb_point2D* expected = hallway_expected_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "hallway %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
            ck_assert_msg(hallway_room_walls[j] == hallway_expected_walls[i][j], "hallway %d wall %d should have been %d, was %d",
                i, j, hallway_expected_walls[i][j], hallway_room_walls[j]);
        }
    }

    /* Check that rooms have the correct shapes and connections */
    pb_point2D expected_room0_points[] = { { 0.f, 4.75f },
                                           { 0.f, 0.f },
                                           { 4.75f, 0.f },
                                           { 4.75f, 4.75f } };
    pb_point2D expected_room1_points[] = { { 0.f, 10.f },
                                           { 0.f, 5.25f },
                                           { 4.75f, 5.25f },
                                           { 4.75f, 10.f } };
    pb_point2D expected_room2_points[] = { { 5.25f, 0.f },
                                           { 10.f, 0.f },
                                           { 10.f, 4.75f },
                                           { 5.25f, 4.75f } };
    pb_point2D expected_room3_points[] = { { 5.25f, 10.f },
                                           { 5.25f, 5.25f },
                                           { 10.f, 5.25f },
                                           { 10.f, 10.f } };

    size_t expected_room_point_counts[] = { sizeof(expected_room0_points) / sizeof(pb_point2D),
                                            sizeof(expected_room1_points) / sizeof(pb_point2D),
                                            sizeof(expected_room2_points) / sizeof(pb_point2D),
                                            sizeof(expected_room3_points) / sizeof(pb_point2D) };
    pb_point2D* expected_room_points[] = { &expected_room0_points[0],
                                           &expected_room1_points[0],
                                           &expected_room2_points[0],
                                           &expected_room3_points[0] };

    for (i = 0; i < sizeof(expected_room_points) / sizeof(pb_point2D*); ++i) {
        pb_room* room = f.rooms + i;
        pb_point2D* points = (pb_point2D*)room->shape.points.items;
        ck_assert_msg(room->shape.points.size == expected_room_point_counts[i], "room %d should have had %lu points, had %lu",
            i, expected_room_point_counts[i], room->shape.points.size);
        for (j = 0; j < expected_room_point_counts[i]; ++j) {
            pb_point2D* real = points + j;
            pb_point2D* expected = expected_room_points[i] + j;
            ck_assert_msg(pb_float_approx_eq(real->x, expected->x, 5) && pb_float_approx_eq(real->y, expected->y, 5),
                "room %d point %d should have been (%.2f, %.2f), was (%.2f, %.2f)", i, j, expected->x, expected->y, real->x, real->y);
        }
    }

    /* Check room connections */
    pb_sq_house_room_conn expected_room0_conns[] = {
            /* room,     neighbour    overlap_start,  overlap_end,      wall, can_connect, has_door*/
            { f.rooms,   &f.rooms[4], { 0.f, 4.75f }, { 4.75f, 4.75f }, 3,    1,           1 },
            { f.rooms,   &f.rooms[6], { 4.75f, 0.f }, { 4.75f, 4.75f }, 2,    1,           1 },
    };

    pb_sq_house_room_conn expected_room1_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,      wall, can_connect, has_door*/
            { &f.rooms[1], &f.rooms[4], { 0.f, 5.25f },   { 4.75f, 5.25f }, 1,    1,           1 },
            { &f.rooms[1], &f.rooms[6], { 4.75f, 5.25f }, { 4.75f, 10.f },  2,    1,           1 },
    };

    pb_sq_house_room_conn expected_room2_conns[] = {
            /* room,        neighbour    overlap_start,    overlap_end,      wall,  can_connect, has_door*/
            { &f.rooms[2],  &f.rooms[5], { 5.25f, 4.75f }, { 10.f, 4.75f },  2,     1,           1 },
            { &f.rooms[2],  &f.rooms[6], { 5.25f, 0.f },   { 5.25f, 4.75f }, 3,     1,           1 },
    };

    pb_sq_house_room_conn expected_room3_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,     wall,  can_connect, has_door*/
            { &f.rooms[3], &f.rooms[5], { 5.25f, 5.25f }, { 10.f, 5.25f }, 1,     1,           1 },
            { &f.rooms[3], &f.rooms[6], { 5.25f, 5.25f }, { 5.25f, 10.f }, 0,     1,           1 },
    };

    pb_sq_house_room_conn expected_room4_conns[] = {
            /* room,       neighbour    overlap_start,  overlap_end,      wall, can_connect, has_door*/
            { &f.rooms[4], &f.rooms[0], { 0.f, 4.75f }, { 4.75f, 4.75f }, 1,    1,           1 },
            { &f.rooms[1], &f.rooms[1], { 0.f, 5.25f }, { 4.75f, 5.25f }, 3,    1,           1 },
    };

    pb_sq_house_room_conn expected_room5_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,     wall,  can_connect, has_door*/
            { &f.rooms[5], &f.rooms[2], { 5.25f, 4.75f }, { 10.f, 4.75f }, 1,     1,           1 },
            { &f.rooms[5], &f.rooms[3], { 5.25f, 5.25f }, { 10.f, 5.25f }, 3,     1,           1 },
    };

    pb_sq_house_room_conn expected_room6_conns[] = {
            /* room,       neighbour    overlap_start,    overlap_end,      wall, can_connect, has_door*/
            { &f.rooms[6], &f.rooms[0], { 4.75f, 0.f },   { 4.75f, 4.75f }, 2,    1,           1 },
            { &f.rooms[6], &f.rooms[1], { 4.75f, 5.25f }, { 4.75f, 10.f },  0,    1,           1 },
            { &f.rooms[6], &f.rooms[2], { 5.25f, 0.f },   { 5.25f, 4.75f }, 4,    1,           1 },
            { &f.rooms[6], &f.rooms[3], { 5.25f, 5.25f }, { 5.25f, 10.f },  6,    1,           1 },
    };

    size_t expected_room_conn_counts[] = {
            sizeof(expected_room0_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room1_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room2_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room3_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room4_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room5_conns) / sizeof(pb_sq_house_room_conn),
            sizeof(expected_room6_conns) / sizeof(pb_sq_house_room_conn),
    };
    pb_sq_house_room_conn* expected_room_conns[] = {
            &expected_room0_conns[0],
            &expected_room1_conns[0],
            &expected_room2_conns[0],
            &expected_room3_conns[0],
            &expected_room4_conns[0],
            &expected_room5_conns[0],
            &expected_room6_conns[0],
    };

    for (i = 0; i < f.num_rooms; ++i) {
        pb_vertex const* vert = pb_graph_get_vertex(floor_graph, f.rooms + i);
        ck_assert_msg(vert->edges_size == expected_room_conn_counts[i],
                      "room %lu had %lu edges, should have had %lu",
                      i, vert->edges_size, expected_room_conn_counts[i]);

        for (j = 0; j < expected_room_conn_counts[i]; ++j) {
            pb_sq_house_room_conn const* expected = expected_room_conns[i] + j;
            pb_sq_house_room_conn const* actual = (pb_sq_house_room_conn*)vert->edges[j]->data;

            ck_assert_msg(actual->neighbour == expected->neighbour, "room %lu edge %lu had incorrect neighbour", i, j);
            ck_assert_msg(pb_point_eq(&actual->overlap_start, &expected->overlap_start),
                          "room %lu edge %lu: expected overlap start (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_start.x, expected->overlap_start.y, actual->overlap_start.x, actual->overlap_start.y);
            ck_assert_msg(pb_point_eq(&actual->overlap_end, &expected->overlap_end),
                          "room %lu edge %lu: expected overlap end (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected->overlap_end.x, expected->overlap_end.y, actual->overlap_end.x, actual->overlap_end.y);
            ck_assert_msg(actual->can_connect == expected->can_connect,
                          "room %lu edge %lu: expected can_connect == %lu, was %lu",
                          i, j, expected->can_connect, actual->can_connect);
            ck_assert_msg(actual->has_door == expected->has_door,
                          "room %lu edge %lu: expected has_door %d, was %d",
                          i, j, expected->has_door, actual->has_door);
            ck_assert_msg(actual->wall == expected->wall,
                          "room %lu edge %lu: expected wall %d, was %d",
                          i, j, expected->wall, actual->wall);
        }
    }

    for (i = 0; i < f.num_rooms; ++i) {
        pb_shape2D_free(&f.rooms[i].shape);
        pb_vector_free(&f.rooms[i].walls);
    }
    for (i = 0; i < hallways.size; ++i) {
        pb_vector* hallway = ((pb_vector*)hallways.items) + i;
        pb_vector_free(hallway);
    }
    pb_vector_free(&hallways);

    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    pb_graph_free(internal_graph);
    pb_hashmap_free(room_spec_map);
    free(f.rooms);
}
END_TEST

START_TEST(place_doors_have_doors)
{
    /* Input:
     * - Floor containing two rooms with rects
     *   [] {{0.f, 0.f}, 5.f, 5.f}
     *   [] {{5.f, 0.f}, 5.f, 5.f}
     * - Floor graph (generated by pb_generate_floor_graph)
     * - House spec with door_size = 0.75f
     *
     * Expected output:
     * - Both rooms have 1 door
     * - Doors placed at:
     *   [] Room 0: start = {5.f, 2.125f}, end = {5.f, 2.875f}, wall = 2
     *   [] Room 1: start = {5.f, 2.125f}, end = {5.f, 2.875f}, wall = 0
     * - Floor has doors == NULL and num_doors == 0
     */

    size_t i, j;

    char const* name = "Room";
    pb_sq_house_room_spec room_spec;
    room_spec.name = name;
    room_spec.adjacent = &name;
    room_spec.num_adjacent = 1;
    room_spec.priority = 0;
    room_spec.max_instances = 2;
    room_spec.area = 25.f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, name, &room_spec);

    pb_sq_house_house_spec house_spec;
    house_spec.num_rooms = 2;
    house_spec.door_size = 0.75f;

    pb_rect room_rects[] = {{{0.f, 0.f}, 5.f, 5.f},
                            {{5.f, 0.f}, 5.f, 5.f}};
    pb_room rooms[2] = {0};

    for (i = 0; i < sizeof(rooms) / sizeof(pb_room); ++i) {
        rooms[i].name = name;
        pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
    }

    pb_floor f;
    f.rooms = &rooms[0];
    f.num_rooms = 2;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&house_spec, room_spec_map, &f);
    pb_sq_house_place_doors(&f, &house_spec, floor_graph, 0);

    pb_wall_structure expected_room0_doors[] = {
            { { 5.f, 2.125f }, { 5.f, 2.875f }, 2 },
    };
    pb_wall_structure expected_room1_doors[] = {
            { { 5.f, 2.125f }, { 5.f, 2.875f }, 0 },
    };
    pb_wall_structure* expected_doors[] = {
        &expected_room0_doors[0],
        &expected_room1_doors[0],
    };
    size_t expected_door_counts[] = {
            sizeof(expected_room0_doors) / sizeof(pb_wall_structure),
            sizeof(expected_room1_doors) / sizeof(pb_wall_structure),
    };

    size_t num_rooms = sizeof(rooms) / sizeof(pb_room);

    ck_assert_msg(f.num_doors == 0, "floor should have had 0 doors, had %lu", f.num_doors);
    ck_assert_msg(f.doors == NULL, "floor's doors array should have been initialised to NULL, was %p", f.doors);

    for (i = 0; i < num_rooms; ++i) {
        pb_room* r = f.rooms + i;

        ck_assert_msg(r->num_doors == expected_door_counts[i], "room %lu had %lu doors, expected %lu",
                      i, r->num_doors, expected_door_counts[i]);


        for (j = 0; j < f.rooms[i].num_doors; ++j) {
            pb_wall_structure* door = r->doors + j;
            ck_assert_msg(pb_point_eq(&door->start, &expected_doors[i][j].start),
                          "room %lu: door %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                           i, j, expected_doors[i][j].start.x, expected_doors[i][j].start.y, door->start.x, door->start.y);
            ck_assert_msg(pb_point_eq(&door->end, &expected_doors[i][j].end),
                          "room %lu: door %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_doors[i][j].end.x, expected_doors[i][j].end.y, door->end.x, door->end.y);
            ck_assert_msg(door->wall == expected_doors[i][j].wall, "room %lu: door %lu should have had wall %d, was %d",
                          i, j, expected_doors[i][j].wall, door->wall);
        }
    }

    for (i = 0; i < num_rooms; ++i) {
        pb_shape2D_free(&rooms[i].shape);
        pb_vector_free(&rooms[i].walls);
        free(rooms[i].doors);
    }
    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    free(f.doors);
    pb_hashmap_free(room_spec_map);
}
END_TEST

START_TEST(place_doors_no_doors)
{
    /* Input:
     * - Floor containing two rooms with rects
     *   [] {{0.f, 0.f}, 5.f, 5.f}
     *   [] {{5.f, 0.f}, 5.f, 5.f}
     * - Floor graph (generated by pb_generate_floor_graph)
     * - House spec with door_size = 0.75f
     * - Room spec that doesn't allow connection between the two currently placed rooms
     *
     * Expected output:
     * - Both rooms have 0 doors
     8 - The floor has doors = NULL and 0 doors
     */

    size_t i, j;

    char const* name = "Room";
    char const* adj_name = "Other Room";
    pb_sq_house_room_spec room_spec;
    room_spec.name = name;
    room_spec.adjacent = &adj_name;
    room_spec.num_adjacent = 1;
    room_spec.priority = 0;
    room_spec.max_instances = 2;
    room_spec.area = 25.f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, name, &room_spec);

    pb_sq_house_house_spec house_spec;
    house_spec.num_rooms = 2;
    house_spec.door_size = 0.75f;

    pb_rect room_rects[] = {{{0.f, 0.f}, 5.f, 5.f},
                            {{5.f, 0.f}, 5.f, 5.f}};
    pb_room rooms[2] = {0};

    for (i = 0; i < sizeof(rooms) / sizeof(pb_room); ++i) {
        rooms[i].name = name;
        pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
    }

    pb_floor f;
    f.rooms = &rooms[0];
    f.num_rooms = 2;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&house_spec, room_spec_map, &f);
    pb_sq_house_place_doors(&f, &house_spec, floor_graph, 0);

    size_t num_rooms = sizeof(rooms) / sizeof(pb_room);

    ck_assert_msg(f.num_doors == 0, "floor should have had 0 doors, had %lu", f.num_doors);
    ck_assert_msg(f.doors == NULL, "floor's doors array should have been initialised to NULL, was %p", f.doors);

    for (i = 0; i < num_rooms; ++i) {
        pb_room* r = f.rooms + i;
        ck_assert_msg(r->num_doors == 0, "room %lu should have had 0 doors, had %lu", i, r->num_doors);
        ck_assert_msg(r->doors == NULL, "room %lu should have had doors == NULL, had %p", r->doors);
    }

    for (i = 0; i < num_rooms; ++i) {
        pb_shape2D_free(&rooms[i].shape);
        pb_vector_free(&rooms[i].walls);
        free(rooms[i].doors);
    }
    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    free(f.doors);
    pb_hashmap_free(room_spec_map);
}
END_TEST

START_TEST(place_doors_one_room_floor0)
{
    /* Input:
     * - Floor containing one room with rect
     *   [] {{0.f, 0.f}, 5.f, 5.f}
     * - Floor graph (generated by pb_generate_floor_graph)
     * - House spec with door_size = 0.75f
     *
     * Expected output:
     * - Room has 1 door with start = {2.215f, 0.f}, end = {2.875f, 0.f}, wall = 1
     * - Floor has 1 door with start = {2.215f, 0.f}, end = {2.875f, 0.f}, wall = 1
     */

    size_t i, j;

    char const* name = "Room";
    pb_sq_house_room_spec room_spec;
    room_spec.name = name;
    room_spec.adjacent = &name;
    room_spec.num_adjacent = 1;
    room_spec.priority = 0;
    room_spec.max_instances = 2;
    room_spec.area = 25.f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, name, &room_spec);

    pb_sq_house_house_spec house_spec;
    house_spec.num_rooms = 2;
    house_spec.door_size = 0.75f;

    pb_rect room_rects[] = { {{0.f, 0.f}, 5.f, 5.f} };
    pb_room rooms[1] = {0};

    for (i = 0; i < sizeof(rooms) / sizeof(pb_room); ++i) {
        rooms[i].name = name;
        pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
    }
    size_t num_rooms = sizeof(rooms) / sizeof(pb_room);

    pb_floor f;
    f.rooms = &rooms[0];
    f.num_rooms = num_rooms;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&house_spec, room_spec_map, &f);
    pb_sq_house_place_doors(&f, &house_spec, floor_graph, 1);

    pb_wall_structure expected_room0_doors[] = {
            { { 2.125f, 0.f }, { 2.875f, 0.f }, 1 },
    };
    pb_wall_structure* expected_doors[] = {
            &expected_room0_doors[0],
    };
    size_t expected_door_counts[] = {
            sizeof(expected_room0_doors) / sizeof(pb_wall_structure),
    };

    for (i = 0; i < num_rooms; ++i) {
        pb_room* r = f.rooms + i;

        ck_assert_msg(r->num_doors == expected_door_counts[i], "room %lu had %lu doors, expected %lu",
                      i, r->num_doors, expected_door_counts[i]);


        for (j = 0; j < f.rooms[i].num_doors; ++j) {
            pb_wall_structure* door = r->doors + j;
            ck_assert_msg(pb_point_eq(&door->start, &expected_doors[i][j].start),
                          "room %lu: door %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_doors[i][j].start.x, expected_doors[i][j].start.y, door->start.x, door->start.y);
            ck_assert_msg(pb_point_eq(&door->end, &expected_doors[i][j].end),
                          "room %lu: door %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_doors[i][j].end.x, expected_doors[i][j].end.y, door->end.x, door->end.y);
            ck_assert_msg(door->wall == expected_doors[i][j].wall, "room %lu: door %lu should have had wall %d, was %d",
                          i, j, expected_doors[i][j].wall, door->wall);
        }
    }

    pb_wall_structure expected_floor_door = {{2.125f, 0.f}, {2.875f, 0.f}, 1};
    ck_assert_msg(f.num_doors == 1, "floor should have had 1 door, had %lu", f.num_doors);
    ck_assert_msg(pb_point_eq(&f.doors->start, &expected_floor_door.start),
                  "floor door start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                  expected_floor_door.start.x, expected_floor_door.start.y, f.doors->start.x, f.doors->start.y);
    ck_assert_msg(pb_point_eq(&f.doors->end, &expected_floor_door.end),
                  "floor door start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                  expected_floor_door.end.x, expected_floor_door.end.y, f.doors->end.x, f.doors->end.y);
    ck_assert_msg(f.doors->wall == expected_floor_door.wall, "room %lu: door %lu should have had wall %d, was %d",
                  i, j, expected_floor_door.wall, f.doors->wall);

    for (i = 0; i < num_rooms; ++i) {
        pb_shape2D_free(&rooms[i].shape);
        pb_vector_free(&rooms[i].walls);
        free(rooms[i].doors);
    }
    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    free(f.doors);
    pb_hashmap_free(room_spec_map);
}
END_TEST

START_TEST(place_doors_multi_room_floor0)
{
    /* Input:
     * - Floor containing two room with rects
     *   [] {{0.f, 0.f}, 7.5f, 5.f}
     *   [] {{7.5f, 0.f}, 2.5f, 5.f}
     * - Floor graph (generated by pb_generate_floor_graph)
     * - House spec with door_size = 0.75f
     *
     * Expected output:
     * - Room 0 has doors
     *   [] start = {7.5f, 2.215f}, end = {7.5f, 2.875f}, wall = 2
     *   [] start = {3.375f, 0.f}, end = {4.125f, 0.f}, wall = 1
     * - Room 1 has doors
     *   [] {7.5f, 2.215f}, end = {7.5f, 2.875f}, wall = 0
     * - Floor has doors
     *   [] start = {3.375f, 0.f}, end = {4.125f, 0.f}, wall = 1
     */

    size_t i, j;

    char const* name = "Room";
    pb_sq_house_room_spec room_spec;
    room_spec.name = name;
    room_spec.adjacent = &name;
    room_spec.num_adjacent = 1;
    room_spec.priority = 0;
    room_spec.max_instances = 2;
    room_spec.area = 25.f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, name, &room_spec);

    pb_sq_house_house_spec house_spec;
    house_spec.num_rooms = 2;
    house_spec.door_size = 0.75f;

    pb_rect room_rects[] = { {{0.f, 0.f}, 7.5f, 5.f},
                             {{7.5f, 0.f}, 2.5f, 5.f}};
    pb_room rooms[2] = {0};

    for (i = 0; i < sizeof(rooms) / sizeof(pb_room); ++i) {
        rooms[i].name = name;
        pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
    }
    size_t num_rooms = sizeof(rooms) / sizeof(pb_room);

    pb_floor f;
    f.rooms = &rooms[0];
    f.num_rooms = num_rooms;

    pb_graph* floor_graph = pb_sq_house_generate_floor_graph(&house_spec, room_spec_map, &f);
    pb_sq_house_place_doors(&f, &house_spec, floor_graph, 1);

    pb_wall_structure expected_room0_doors[] = {
            {{7.5f, 2.125f}, {7.5f, 2.875f}, 2},
            {{3.375f, 0.f}, {4.125f, 0.f}, 1},
    };
    pb_wall_structure expected_room1_doors[] = {
            {{7.5f, 2.125f}, {7.5f, 2.875f}, 0},
    };
    pb_wall_structure* expected_doors[] = {
            &expected_room0_doors[0],
            &expected_room1_doors[0],
    };
    size_t expected_door_counts[] = {
            sizeof(expected_room0_doors) / sizeof(pb_wall_structure),
            sizeof(expected_room1_doors) / sizeof(pb_wall_structure),
    };

    for (i = 0; i < num_rooms; ++i) {
        pb_room* r = f.rooms + i;

        ck_assert_msg(r->num_doors == expected_door_counts[i], "room %lu had %lu doors, expected %lu",
                      i, r->num_doors, expected_door_counts[i]);


        for (j = 0; j < f.rooms[i].num_doors; ++j) {
            pb_wall_structure* door = r->doors + j;
            ck_assert_msg(pb_point_eq(&door->start, &expected_doors[i][j].start),
                          "room %lu: door %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_doors[i][j].start.x, expected_doors[i][j].start.y, door->start.x, door->start.y);
            ck_assert_msg(pb_point_eq(&door->end, &expected_doors[i][j].end),
                          "room %lu: door %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_doors[i][j].end.x, expected_doors[i][j].end.y, door->end.x, door->end.y);
            ck_assert_msg(door->wall == expected_doors[i][j].wall, "room %lu: door %lu should have had wall %d, was %d",
                          i, j, expected_doors[i][j].wall, door->wall);
        }
    }

    pb_wall_structure expected_floor_door = {{3.375f, 0.f}, {4.125f, 0.f}, 1};
    ck_assert_msg(f.num_doors == 1, "floor should have had 1 door, had %lu", f.num_doors);
    ck_assert_msg(pb_point_eq(&f.doors->start, &expected_floor_door.start),
                  "floor door start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                  expected_floor_door.start.x, expected_floor_door.start.y, f.doors->start.x, f.doors->start.y);
    ck_assert_msg(pb_point_eq(&f.doors->end, &expected_floor_door.end),
                  "floor door start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                  expected_floor_door.end.x, expected_floor_door.end.y, f.doors->end.x, f.doors->end.y);
    ck_assert_msg(f.doors->wall == expected_floor_door.wall, "room %lu: door %lu should have had wall %d, was %d",
                  i, j, expected_floor_door.wall, f.doors->wall);

    for (i = 0; i < num_rooms; ++i) {
        pb_shape2D_free(&rooms[i].shape);
        pb_vector_free(&rooms[i].walls);
        free(rooms[i].doors);
    }
    pb_graph_for_each_edge(floor_graph, pb_graph_free_edge_data, NULL);
    pb_graph_free(floor_graph);
    free(f.doors);
    pb_hashmap_free(room_spec_map);
}
END_TEST

START_TEST(place_windows_simple)
{
    /* Input:
     * - Floor containing two rooms with rects
     *   [] {{0.f, 0.f}, 5.f, 5.f}
     *   [] {{5.f, 0.f}, 5.f, 5.f}
     * - Floor graph (generated by pb_generate_floor_graph)
     * - House spec with window_size = 0.5f
     *
     * Expected output:
     * - Both rooms have 3 windows
     * - Doors placed at:
     *   [] Room 0:
     *      (+) start = {0.f, 2.25f}, end = {0.f, 2.75f}, wall = 0
     *      (+) start = {2.25f, 0.f}, end = {2.75f, 0.f}, wall = 1
     *      (+) start = {2.25f, 5.f}, end = {2.75f, 5.f}, wall = 3
     *   [] Room 1:
     *      (+) start = {7.25.f, 0.f}, end = {7.75f, 0.f}, wall = 1
     *      (+) start = {10.f, 2.25f}, end = {10.f, 2.75f}, wall = 2
     *      (+) start = {7.25, 5.f}, end = {7.75f, 5.f}, wall = 3
     * - Floor has num_windows == 6 and the same windows as the rooms (in the same order)
     */

    size_t i, j;

    char const* name = "Room";
    pb_sq_house_room_spec room_spec;
    room_spec.name = name;
    room_spec.adjacent = &name;
    room_spec.num_adjacent = 1;
    room_spec.priority = 0;
    room_spec.max_instances = 2;
    room_spec.area = 25.f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, name, &room_spec);

    pb_sq_house_house_spec house_spec;
    house_spec.num_rooms = 2;
    house_spec.door_size = 0.75f;
    house_spec.window_size = 0.5f;

    pb_rect room_rects[] = {{{0.f, 0.f}, 5.f, 5.f},
                            {{5.f, 0.f}, 5.f, 5.f}};
    pb_room rooms[2] = {0};

    for (i = 0; i < sizeof(rooms) / sizeof(pb_room); ++i) {
        rooms[i].name = name;
        pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
    }

    pb_floor f;
    pb_rect floor_rect = {{0.f, 0.f}, 10.f, 5.f};
    f.rooms = &rooms[0];
    f.num_rooms = 2;
    pb_rect_to_pb_shape2D(&floor_rect, &f.shape);

    pb_sq_house_place_windows(&f, &house_spec, 0);

    pb_wall_structure expected_room0_windows[] = {
            {{0.f, 2.25f}, {0.f, 2.75f}, 0},
            {{2.25f, 0.f}, {2.75f, 0.f}, 1},
            {{2.25f, 5.f}, {2.75f, 5.f}, 3},
    };
    pb_wall_structure expected_room1_windows[] = {
            {{7.25f, 0.f}, {7.75f, 0.f}, 1},
            {{10.f, 2.25f}, {10.f, 2.75f}, 2},
            {{7.25, 5.f}, {7.75f, 5.f}, 3},
    };
    pb_wall_structure* expected_windows[] = {
            &expected_room0_windows[0],
            &expected_room1_windows[0],
    };
    size_t expected_window_counts[] = {
            sizeof(expected_room0_windows) / sizeof(pb_wall_structure),
            sizeof(expected_room1_windows) / sizeof(pb_wall_structure),
    };

    size_t num_rooms = sizeof(rooms) / sizeof(pb_room);

    for (i = 0; i < num_rooms; ++i) {
        pb_room* r = f.rooms + i;

        ck_assert_msg(r->num_windows == expected_window_counts[i], "room %lu had %lu windows, expected %lu",
                      i, r->num_windows, expected_window_counts[i]);

        for (j = 0; j < f.rooms[i].num_windows; ++j) {
            pb_wall_structure* window = r->windows + j;
            ck_assert_msg(pb_point_eq(&window->start, &expected_windows[i][j].start),
                          "room %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_windows[i][j].start.x, expected_windows[i][j].start.y, window->start.x, window->start.y);
            ck_assert_msg(pb_point_eq(&window->end, &expected_windows[i][j].end),
                          "room %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_windows[i][j].end.x, expected_windows[i][j].end.y, window->end.x, window->end.y);
            ck_assert_msg(window->wall == expected_windows[i][j].wall, "room %lu: window %lu should have had wall %d, was %d",
                          i, j, expected_windows[i][j].wall, window->wall);
        }
    }

    pb_wall_structure expected_floor_windows[] = {
            {{0.f, 2.25f}, {0.f, 2.75f}, 0},
            {{2.25f, 0.f}, {2.75f, 0.f}, 1},
            {{2.25f, 5.f}, {2.75f, 5.f}, 3},
            {{7.25f, 0.f}, {7.75f, 0.f}, 1},
            {{10.f, 2.25f}, {10.f, 2.75f}, 2},
            {{7.25, 5.f}, {7.75f, 5.f}, 3},
    };
    size_t expected_num_floor_windows = sizeof(expected_floor_windows) / sizeof(pb_wall_structure);
    ck_assert_msg(f.num_windows == expected_num_floor_windows, "floor should have had %lu windows, had %lu",
                  expected_num_floor_windows, f.num_windows);
    for (i = 0; i < f.num_windows; ++i) {
        pb_wall_structure* window = f.windows + i;
        ck_assert_msg(pb_point_eq(&window->start, &expected_floor_windows[i].start),
                      "floor: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                      i, expected_floor_windows[i].start.x, expected_floor_windows[i].start.y, window->start.x, window->start.y);
        ck_assert_msg(pb_point_eq(&window->end, &expected_floor_windows[i].end),
                      "floor %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                      i, expected_floor_windows[i].end.x, expected_floor_windows[i].end.y, window->end.x, window->end.y);
        ck_assert_msg(window->wall == expected_floor_windows[i].wall, "room %lu: door %lu should have had wall %d, was %d",
                      i, expected_floor_windows[i].wall, window->wall);
    }

    for (i = 0; i < num_rooms; ++i) {
        pb_shape2D_free(&rooms[i].shape);
        pb_vector_free(&rooms[i].walls);
        free(rooms[i].windows);
    }

    free(f.windows);
    pb_shape2D_free(&f.shape);
    pb_hashmap_free(room_spec_map);
}
END_TEST

START_TEST(place_windows_one_room_floor0)
{
/* Input:
     * - Floor containing one room with rect
     *   [] {{0.f, 0.f}, 5.f, 5.f}
     * - Floor graph (generated by pb_generate_floor_graph)
     * - House spec with window_size = 0.5f
     * - is_first_floor = 1
     *
     * Expected output:
     * - Rooms has 3 windows
     * - Windows placed at:
     *   [] Room 0:
     *      (+) start = {0.f, 2.25f}, end = {0.f, 2.75f}, wall = 0
     *      (+) start = {5.f, 2.25f}, end = {5.f, 2.75f}, wall = 0
     *      (+) start = {2.25f, 5.f}, end = {2.75f, 5.f}, wall = 3
     * - Floor has num_windows == 3 and the same windows as the rooms (in the same order)
     */

        size_t i, j;

        char const* name = "Room";
        pb_sq_house_room_spec room_spec;
        room_spec.name = name;
        room_spec.adjacent = &name;
        room_spec.num_adjacent = 1;
        room_spec.priority = 0;
        room_spec.max_instances = 2;
        room_spec.area = 25.f;

        pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
        pb_hashmap_put(room_spec_map, name, &room_spec);

        pb_sq_house_house_spec house_spec;
        house_spec.num_rooms = 2;
        house_spec.door_size = 0.75f;
        house_spec.window_size = 0.5f;

        pb_rect room_rects[] = {{{0.f, 0.f}, 5.f, 5.f},};
        pb_room rooms[1] = {0};

        for (i = 0; i < sizeof(rooms) / sizeof(pb_room); ++i) {
            rooms[i].name = name;
            pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
        }

        pb_floor f;
        pb_rect floor_rect = {{0.f, 0.f}, 5.f, 5.f};
        f.rooms = &rooms[0];
        f.num_rooms = 1;
        pb_rect_to_pb_shape2D(&floor_rect, &f.shape);

        pb_sq_house_place_windows(&f, &house_spec, 1);

        pb_wall_structure expected_room0_windows[] = {
                {{0.f, 2.25f}, {0.f, 2.75f}, 0},
                {{5.f, 2.25f}, {5.f, 2.75f}, 2},
                {{2.25f, 5.f}, {2.75f, 5.f}, 3},
        };
        pb_wall_structure* expected_windows[] = {
                &expected_room0_windows[0],
        };
        size_t expected_window_counts[] = {
                sizeof(expected_room0_windows) / sizeof(pb_wall_structure),
        };

        size_t num_rooms = sizeof(rooms) / sizeof(pb_room);

        for (i = 0; i < num_rooms; ++i) {
            pb_room* r = f.rooms + i;

            ck_assert_msg(r->num_windows == expected_window_counts[i], "room %lu had %lu windows, expected %lu",
                          i, r->num_windows, expected_window_counts[i]);

            for (j = 0; j < f.rooms[i].num_windows; ++j) {
                pb_wall_structure* window = r->windows + j;
                ck_assert_msg(pb_point_eq(&window->start, &expected_windows[i][j].start),
                              "room %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                              i, j, expected_windows[i][j].start.x, expected_windows[i][j].start.y, window->start.x, window->start.y);
                ck_assert_msg(pb_point_eq(&window->end, &expected_windows[i][j].end),
                              "room %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                              i, j, expected_windows[i][j].end.x, expected_windows[i][j].end.y, window->end.x, window->end.y);
                ck_assert_msg(window->wall == expected_windows[i][j].wall, "room %lu: window %lu should have had wall %d, was %d",
                              i, j, expected_windows[i][j].wall, window->wall);
            }
        }

        pb_wall_structure expected_floor_windows[] = {
                {{0.f, 2.25f}, {0.f, 2.75f}, 0},
                {{5.f, 2.25f}, {5.f, 2.75f}, 2},
                {{2.25f, 5.f}, {2.75f, 5.f}, 3},
        };
        size_t expected_num_floor_windows = sizeof(expected_floor_windows) / sizeof(pb_wall_structure);
        ck_assert_msg(f.num_windows == expected_num_floor_windows, "floor should have had %lu windows, had %lu",
                      expected_num_floor_windows, f.num_windows);
        for (i = 0; i < f.num_windows; ++i) {
            pb_wall_structure* window = f.windows + i;
            ck_assert_msg(pb_point_eq(&window->start, &expected_floor_windows[i].start),
                          "floor: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, expected_floor_windows[i].start.x, expected_floor_windows[i].start.y, window->start.x, window->start.y);
            ck_assert_msg(pb_point_eq(&window->end, &expected_floor_windows[i].end),
                          "floor %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, expected_floor_windows[i].end.x, expected_floor_windows[i].end.y, window->end.x, window->end.y);
            ck_assert_msg(window->wall == expected_floor_windows[i].wall, "room %lu: door %lu should have had wall %d, was %d",
                          i, expected_floor_windows[i].wall, window->wall);
        }

        for (i = 0; i < num_rooms; ++i) {
            pb_shape2D_free(&rooms[i].shape);
            pb_vector_free(&rooms[i].walls);
            free(rooms[i].windows);
        }

        free(f.windows);
        pb_shape2D_free(&f.shape);
        pb_hashmap_free(room_spec_map);
}
END_TEST

START_TEST(place_windows_multi_room_floor0)
{
    /* Input:
     * - Floor containing two rooms with rects
     *   [] {{0.f, 0.f}, 5.f, 5.f}
     *   [] {{5.f, 0.f}, 5.f, 5.f}
     * - Floor graph (generated by pb_generate_floor_graph)
     * - House spec with window_size = 0.5f
     * - is_first_floor = 1
     *
     * Expected output:
     * - Both rooms have 3 windows
     * - Doors placed at:
     *   [] Room 0:
     *      (+) start = {0.f, 2.25f}, end = {0.f, 2.75f}, wall = 0
     *      (+) start = {2.25f, 5.f}, end = {2.75f, 5.f}, wall = 3
     *   [] Room 1:
     *      (+) start = {7.25.f, 0.f}, end = {7.75f, 0.f}, wall = 1
     *      (+) start = {10.f, 2.25f}, end = {10.f, 2.75f}, wall = 2
     *      (+) start = {7.25, 5.f}, end = {7.75f, 5.f}, wall = 3
     * - Floor has num_windows == 5 and the same windows as the rooms (in the same order)
     */

    size_t i, j;

    char const* name = "Room";
    pb_sq_house_room_spec room_spec;
    room_spec.name = name;
    room_spec.adjacent = &name;
    room_spec.num_adjacent = 1;
    room_spec.priority = 0;
    room_spec.max_instances = 2;
    room_spec.area = 25.f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, name, &room_spec);

    pb_sq_house_house_spec house_spec;
    house_spec.num_rooms = 2;
    house_spec.door_size = 0.75f;
    house_spec.window_size = 0.5f;

    pb_rect room_rects[] = {{{0.f, 0.f}, 5.f, 5.f},
                            {{5.f, 0.f}, 5.f, 5.f}};
    pb_room rooms[2] = {0};

    for (i = 0; i < sizeof(rooms) / sizeof(pb_room); ++i) {
        rooms[i].name = name;
        pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
    }

    pb_floor f;
    pb_rect floor_rect = {{0.f, 0.f}, 10.f, 5.f};
    f.rooms = &rooms[0];
    f.num_rooms = 2;
    pb_rect_to_pb_shape2D(&floor_rect, &f.shape);

    pb_sq_house_place_windows(&f, &house_spec, 1);

    pb_wall_structure expected_room0_windows[] = {
            {{0.f, 2.25f}, {0.f, 2.75f}, 0},
            {{2.25f, 5.f}, {2.75f, 5.f}, 3},
    };
    pb_wall_structure expected_room1_windows[] = {
            {{7.25f, 0.f}, {7.75f, 0.f}, 1},
            {{10.f, 2.25f}, {10.f, 2.75f}, 2},
            {{7.25, 5.f}, {7.75f, 5.f}, 3},
    };
    pb_wall_structure* expected_windows[] = {
            &expected_room0_windows[0],
            &expected_room1_windows[0],
    };
    size_t expected_window_counts[] = {
            sizeof(expected_room0_windows) / sizeof(pb_wall_structure),
            sizeof(expected_room1_windows) / sizeof(pb_wall_structure),
    };

    size_t num_rooms = sizeof(rooms) / sizeof(pb_room);

    for (i = 0; i < num_rooms; ++i) {
        pb_room* r = f.rooms + i;

        ck_assert_msg(r->num_windows == expected_window_counts[i], "room %lu had %lu windows, expected %lu",
                      i, r->num_windows, expected_window_counts[i]);

        for (j = 0; j < f.rooms[i].num_windows; ++j) {
            pb_wall_structure* window = r->windows + j;
            ck_assert_msg(pb_point_eq(&window->start, &expected_windows[i][j].start),
                          "room %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_windows[i][j].start.x, expected_windows[i][j].start.y, window->start.x, window->start.y);
            ck_assert_msg(pb_point_eq(&window->end, &expected_windows[i][j].end),
                          "room %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_windows[i][j].end.x, expected_windows[i][j].end.y, window->end.x, window->end.y);
            ck_assert_msg(window->wall == expected_windows[i][j].wall, "room %lu: window %lu should have had wall %d, was %d",
                          i, j, expected_windows[i][j].wall, window->wall);
        }
    }

    pb_wall_structure expected_floor_windows[] = {
            {{0.f, 2.25f}, {0.f, 2.75f}, 0},
            {{2.25f, 5.f}, {2.75f, 5.f}, 3},
            {{7.25f, 0.f}, {7.75f, 0.f}, 1},
            {{10.f, 2.25f}, {10.f, 2.75f}, 2},
            {{7.25, 5.f}, {7.75f, 5.f}, 3},
    };
    size_t expected_num_floor_windows = sizeof(expected_floor_windows) / sizeof(pb_wall_structure);
    ck_assert_msg(f.num_windows == expected_num_floor_windows, "floor should have had %lu windows, had %lu",
                  expected_num_floor_windows, f.num_windows);
    for (i = 0; i < f.num_windows; ++i) {
        pb_wall_structure* window = f.windows + i;
        ck_assert_msg(pb_point_eq(&window->start, &expected_floor_windows[i].start),
                      "floor: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                      i, expected_floor_windows[i].start.x, expected_floor_windows[i].start.y, window->start.x, window->start.y);
        ck_assert_msg(pb_point_eq(&window->end, &expected_floor_windows[i].end),
                      "floor %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                      i, expected_floor_windows[i].end.x, expected_floor_windows[i].end.y, window->end.x, window->end.y);
        ck_assert_msg(window->wall == expected_floor_windows[i].wall, "room %lu: door %lu should have had wall %d, was %d",
                      i, expected_floor_windows[i].wall, window->wall);
    }

    for (i = 0; i < num_rooms; ++i) {
        pb_shape2D_free(&rooms[i].shape);
        pb_vector_free(&rooms[i].walls);
        free(rooms[i].windows);
    }

    free(f.windows);
    pb_shape2D_free(&f.shape);
    pb_hashmap_free(room_spec_map);
}
END_TEST

START_TEST(place_windows_centre_room)
{
    /* Input:
 * - Floor containing five rooms with rects
 *   [] {{0.f, 0.f}, 5.f, 10.f}
 *   [] {{5.f, 0.f}, 5.f, 10.f / 3.f}
 *   [] {{5.f, 10.f / 3.f}, 5.f, 10.f / 3.f}
 *   [] {{5.f, 20.f / 3.f}, 5.f, 10.f / 3.f}
 *   [] {{10.f, 10.f}, 5.f, 10.f}
 * - Floor graph (generated by pb_generate_floor_graph)
 * - House spec with window_size = 0.5f
 * - is_first_floor = 0
 *
 * Expected output:
 * - Rooms have windows placed at:
 *   [] Room 0:
 *      (+) start = {0.f, 4.75f}, end = {0.f, 5.25f}, wall = 0
 *      (+) start = {2.25f, 0.f}, end = {2.75f, 0.f}, wall = 1
 *      (+) start = {2.25f, 10.f}, end = {2.75f, 10.f}, wall = 3
 *   [] Room 1:
 *      (+) start = {7.25.f, 0.f}, end = {7.75f, 0.f}, wall = 1
 *   [] Room 2: none
 *   [] Room 3:
 *      (+) start = {7.25.f, 10.f}, end = {7.75f, 10.f}, wall = 3
 *   [] Room 4:
 *      (+) start = {12.25f, 0.f}, end = {12.75f, 0.f}, wall = 1
 *      (+) start = {15.f, 7.25f}, end = {15.f, 7.75f}, wall = 2
 *      (+) start = {12.25f, 10.f}, end = {12.75f, 10.f}, wall = 3
 * - Floor has num_windows == 8 and the same windows as the rooms (in the same order)
 */

    size_t i, j;

    char const* name = "Room";
    pb_sq_house_room_spec room_spec;
    room_spec.name = name;
    room_spec.adjacent = &name;
    room_spec.num_adjacent = 1;
    room_spec.priority = 0;
    room_spec.max_instances = 2;
    room_spec.area = 25.f;

    pb_hashmap* room_spec_map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_hashmap_put(room_spec_map, name, &room_spec);

    pb_sq_house_house_spec house_spec;
    house_spec.num_rooms = 2;
    house_spec.door_size = 0.75f;
    house_spec.window_size = 0.5f;

    pb_rect room_rects[] = {
            {{0.f, 0.f}, 5.f, 10.f},
            {{5.f, 0.f}, 5.f, 10.f / 3.f},
            {{5.f, 10.f / 3.f}, 5.f, 10.f / 3.f},
            {{5.f, 20.f / 3.f}, 5.f, 10.f / 3.f},
            {{10.f, 0.f}, 5.f, 10.f},
    };
    pb_room rooms[5] = {0};
    size_t num_rooms = sizeof(rooms) / sizeof(pb_room);

    for (i = 0; i < num_rooms; ++i) {
        rooms[i].name = name;
        pb_rect_to_pb_shape2D(&room_rects[i], &rooms[i].shape);
    }

    pb_floor f;
    pb_rect floor_rect = {{0.f, 0.f}, 15.f, 10.f};
    f.rooms = &rooms[0];
    f.num_rooms = num_rooms;
    pb_rect_to_pb_shape2D(&floor_rect, &f.shape);

    pb_sq_house_place_windows(&f, &house_spec, 0);

    pb_wall_structure expected_room0_windows[] = {
            {{0.f, 4.75f},  {0.f, 5.25f}, 0},
            {{2.25f, 0.f},  {2.75f, 0.f}, 1},
            {{2.25f, 10.f}, {2.75f, 10.f}, 3},
    };
    pb_wall_structure expected_room1_windows[] = {
            {{7.25f, 0.f}, {7.75f, 0.f}, 1},
    };
    pb_wall_structure expected_room3_windows[] = {
            {{7.25f, 10.f}, {7.75f, 10.f}, 3},
    };
    pb_wall_structure expected_room4_windows[] = {
            {{12.25f, 0.f},  {12.75f, 0.f}, 1},
            {{15.f, 4.75f},  {15.f, 5.25f}, 2},
            {{12.25f, 10.f}, {12.75f, 10.f}, 3},
    };
    pb_wall_structure* expected_windows[] = {
            &expected_room0_windows[0],
            &expected_room1_windows[0],
            NULL,
            &expected_room3_windows[0],
            &expected_room4_windows[0],
    };
    size_t expected_window_counts[] = {
            sizeof(expected_room0_windows) / sizeof(pb_wall_structure),
            sizeof(expected_room1_windows) / sizeof(pb_wall_structure),
            0,
            sizeof(expected_room3_windows) / sizeof(pb_wall_structure),
            sizeof(expected_room4_windows) / sizeof(pb_wall_structure),
    };

    for (i = 0; i < num_rooms; ++i) {
        pb_room* r = f.rooms + i;

        ck_assert_msg(r->num_windows == expected_window_counts[i], "room %lu had %lu windows, expected %lu",
                      i, r->num_windows, expected_window_counts[i]);

        for (j = 0; j < f.rooms[i].num_windows; ++j) {
            pb_wall_structure* window = r->windows + j;
            ck_assert_msg(pb_point_eq(&window->start, &expected_windows[i][j].start),
                          "room %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_windows[i][j].start.x, expected_windows[i][j].start.y, window->start.x, window->start.y);
            ck_assert_msg(pb_point_eq(&window->end, &expected_windows[i][j].end),
                          "room %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                          i, j, expected_windows[i][j].end.x, expected_windows[i][j].end.y, window->end.x, window->end.y);
            ck_assert_msg(window->wall == expected_windows[i][j].wall, "room %lu: window %lu should have had wall %d, was %d",
                          i, j, expected_windows[i][j].wall, window->wall);
        }
    }

    pb_wall_structure expected_floor_windows[] = {
            {{0.f, 4.75f},  {0.f, 5.25f}, 0},
            {{2.25f, 0.f},  {2.75f, 0.f}, 1},
            {{2.25f, 10.f}, {2.75f, 10.f}, 3},
            {{7.25f, 0.f}, {7.75f, 0.f}, 1},
            {{7.25f, 10.f}, {7.75f, 10.f}, 3},
            {{12.25f, 0.f},  {12.75f, 0.f}, 1},
            {{15.f, 4.75f},  {15.f, 5.25f}, 2},
            {{12.25f, 10.f}, {12.75f, 10.f}, 3},
    };
    size_t expected_num_floor_windows = sizeof(expected_floor_windows) / sizeof(pb_wall_structure);
    ck_assert_msg(f.num_windows == expected_num_floor_windows, "floor should have had %lu windows, had %lu",
                  expected_num_floor_windows, f.num_windows);
    for (i = 0; i < f.num_windows; ++i) {
        pb_wall_structure* window = f.windows + i;
        ck_assert_msg(pb_point_eq(&window->start, &expected_floor_windows[i].start),
                      "floor: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                      i, expected_floor_windows[i].start.x, expected_floor_windows[i].start.y, window->start.x, window->start.y);
        ck_assert_msg(pb_point_eq(&window->end, &expected_floor_windows[i].end),
                      "floor %lu: window %lu start should have been (%.2f, %.2f), was (%.2f, %.2f)",
                      i, expected_floor_windows[i].end.x, expected_floor_windows[i].end.y, window->end.x, window->end.y);
        ck_assert_msg(window->wall == expected_floor_windows[i].wall, "room %lu: door %lu should have had wall %d, was %d",
                      i, expected_floor_windows[i].wall, window->wall);
    }

    for (i = 0; i < num_rooms; ++i) {
        pb_shape2D_free(&rooms[i].shape);
        pb_vector_free(&rooms[i].walls);
        free(rooms[i].windows);
    }

    free(f.windows);
    pb_shape2D_free(&f.shape);
    pb_hashmap_free(room_spec_map);
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

    //tc_sq_house_find_hallways = tcase_create("Hallway finding tests");
    //suite_add_tcase(s, tc_sq_house_find_hallways);
    //tcase_add_test(tc_sq_house_find_hallways, get_hallways_room0_disconnected_simple);
    //tcase_add_test(tc_sq_house_find_hallways, get_hallways_room0_disconnected_one_wall_overlaps);
    //tcase_add_test(tc_sq_house_find_hallways, get_hallways_room0_disconnected_one_wall_small);
    //tcase_add_test(tc_sq_house_find_hallways, get_hallways_single_disconnected);

    //tc_sq_house_place_hallways = tcase_create("Hallway placement tests");
    //suite_add_tcase(s, tc_sq_house_place_hallways);
    //tcase_add_test(tc_sq_house_place_hallways, place_hallways_simple);
    //tcase_add_test(tc_sq_house_place_hallways, place_hallways_corner);
    //tcase_add_test(tc_sq_house_place_hallways, place_hallways_2_corners_opposite_sides);
    //tcase_add_test(tc_sq_house_place_hallways, place_hallways_2_corners_same_side);
    //tcase_add_test(tc_sq_house_place_hallways, place_hallways_t_intersection_y_axis);
    //tcase_add_test(tc_sq_house_place_hallways, place_hallways_t_intersection_x_axis);
    //tcase_add_test(tc_sq_house_place_hallways, place_hallways_4_way_intersection);

    tc_sq_house_place_doors = tcase_create("Door placement tests");
    suite_add_tcase(s, tc_sq_house_place_doors);
    tcase_add_test(tc_sq_house_place_doors, place_doors_have_doors);
    tcase_add_test(tc_sq_house_place_doors, place_doors_no_doors);
    tcase_add_test(tc_sq_house_place_doors, place_doors_one_room_floor0);
    tcase_add_test(tc_sq_house_place_doors, place_doors_multi_room_floor0);

    tc_sq_house_place_windows = tcase_create("Window placement tests");
    suite_add_tcase(s, tc_sq_house_place_windows);
    tcase_add_test(tc_sq_house_place_windows, place_windows_simple);
    tcase_add_test(tc_sq_house_place_windows, place_windows_one_room_floor0);
    tcase_add_test(tc_sq_house_place_windows, place_windows_multi_room_floor0);
    tcase_add_test(tc_sq_house_place_windows, place_windows_centre_room);
    return s;
}