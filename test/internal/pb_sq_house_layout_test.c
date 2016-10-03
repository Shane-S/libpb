#include <libcompat.h>
#include <check.h>
#include <string.h>
#include <pb/pb_sq_house.h>
#include <pb/internal/pb_sq_house_layout.h>
#include <pb/util/pb_hashmap.h>
#include <pb//util/pb_hash_utils.h>
#include "../test_util.h"

START_TEST(choose_rooms_single_room)
{
    pb_sq_house_room_spec specs[1];
    pb_hashmap* rooms = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;
    char** result;

    char* adjacent[] = { PB_SQ_HOUSE_OUTSIDE, "Closet" };

    specs[0].max_instances = 1;
    specs[0].name = "Closet";
    specs[0].adjacent = &adjacent[0];
    specs[0].num_adjacent = 2;

    pb_hashmap_put(rooms, (void*)"Closet", (void*)&specs[0]);
    spec.num_rooms = 1;

    result = pb_sq_house_choose_rooms(rooms, &spec);

    ck_assert_msg(strcmp(result[0], specs[0].name) == 0, "Result should contain closet, but instead contained %s", result[0]);

    pb_hashmap_free(rooms);
    free(result);
}
END_TEST

START_TEST(choose_rooms_multiple_rooms)
{
    pb_sq_house_room_spec specs[2];
    pb_hashmap* rooms = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;

    pb_hashmap* instances = pb_hashmap_create(pb_str_hash, pb_str_eq); /* Stores the number of each room type from the result array */
    void* spec0_instances;
    void* spec1_instances;
    char** result;

    int i;

    char* adjacent[] = { PB_SQ_HOUSE_OUTSIDE, "Closet", "Bathroom" };

    specs[0].max_instances = 6;
    specs[0].name = "Closet";
    specs[0].adjacent = &adjacent[0];
    specs[0].num_adjacent = 3;

    specs[1].max_instances = 6;
    specs[1].name = "Bathroom";
    specs[1].adjacent = &adjacent[0];
    specs[1].num_adjacent = 3;

    pb_hashmap_put(rooms, (void*)specs[0].name, (void*)&specs[0]);
    pb_hashmap_put(rooms, (void*)specs[1].name, (void*)&specs[1]);
    spec.num_rooms = 12;

    result = pb_sq_house_choose_rooms(rooms, &spec);

    pb_hashmap_put(instances, (void*)specs[0].name, (void*)0);
    pb_hashmap_put(instances, (void*)specs[1].name, (void*)0);

    for (i = 0; i < 12; ++i) {
        if (strcmp(result[i], specs[0].name) == 0) {
            void* num;
            pb_hashmap_get(instances, specs[0].name, &num);
            num = (void*)((int)num + 1);
            pb_hashmap_put(instances, specs[0].name, num);
        } else if(strcmp(result[i], specs[1].name) == 0) {
            void* num;
            pb_hashmap_get(instances, specs[1].name, &num);
            num = (void*)((int)num + 1);
            pb_hashmap_put(instances, specs[1].name, num);
        } else {
            ck_abort_msg("Result string didn't match either room specification.");
        }
    }

    pb_hashmap_get(instances, specs[0].name, &spec0_instances);
    pb_hashmap_get(instances, specs[1].name, &spec1_instances);
    ck_assert_msg((int)spec0_instances == 6, "Result should have 6 instances of closet, but instead contained %d", (int)spec0_instances);
    ck_assert_msg((int)spec1_instances == 6, "Result should have 6 instances of bathroom, but instead contained %d", (int)spec1_instances);

    pb_hashmap_free(rooms);
    pb_hashmap_free(instances);
    free(result);
}
END_TEST

START_TEST(choose_rooms_house_too_big)
{
    pb_sq_house_room_spec specs[2];
    pb_hashmap* rooms = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;
    char** result;
    
    specs[0].max_instances = 6;
    specs[0].name = "Closet";
    specs[1].max_instances = 6;
    specs[1].name = "Bathroom";

    pb_hashmap_put(rooms, (void*)specs[0].name, (void*)&specs[0]);
    pb_hashmap_put(rooms, (void*)specs[1].name, (void*)&specs[1]);
    spec.num_rooms = 24;

    result = pb_sq_house_choose_rooms(rooms, &spec);
    ck_assert_msg(result == NULL, "Result should have been NULL, was %p", result);

    pb_hashmap_free(rooms);
    free(result);
}
END_TEST

START_TEST(choose_rooms_no_outside)
{
    pb_sq_house_room_spec specs[1];
    pb_hashmap* rooms = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;
    char** result;

    char* adjacent[] = { "Closet", "Bathroom" };

    specs[0].max_instances = 6;
    specs[0].name = "Closet";
    specs[0].adjacent = &adjacent[0];
    specs[0].num_adjacent = 2;

    pb_hashmap_put(rooms, (void*)specs[0].name, (void*)&specs[0]);
    spec.num_rooms = 6;

    result = pb_sq_house_choose_rooms(rooms, &spec);
    ck_assert_msg(result == NULL, "Result should have been NULL, was %p", result);

    pb_hashmap_free(rooms);
    free(result);
}
END_TEST

START_TEST(layout_stairs_single_floor)
{
    /*
     *  Given a house specification with dimensions 10 by 25, 1 room and room specifications containing one room with one max instance and 250 area
     *  When I invoke pb_sq_house_layout_stairs
     *  Then the result should be one rectangle with the same dimensions and position as the house rectangle and a single floor with a single room
     */
    pb_sq_house_house_spec h_spec;
    pb_sq_house_room_spec living_room;
    char* rooms[] = { "Living Room" };
    pb_hashmap* room_specs = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_rect* result;
    pb_building house;

    h_spec.width = 10;
    h_spec.height = 25;
    h_spec.num_rooms = 1;
    h_spec.stair_room_width = 7.f;
    
    living_room.area = 250;
    living_room.name = "Living Room";

    pb_hashmap_put(room_specs, (void*)living_room.name, (void*)&living_room);

    result = pb_sq_house_layout_stairs(&rooms[0], room_specs, &h_spec, &house);
    ck_assert_msg(house.num_floors == 1, "House should have had one floor, but had %lu", house.num_floors);
    ck_assert_msg(house.floors[0].num_rooms == 1, "House's first floor should have had one room, but had %lu", house.floors[0].num_rooms);
    ck_assert_msg(result[0].bottom_left.x == 0.f && result[0].bottom_left.y == 0.f && result[0].w == 10 && result[0].h == 25,
                  "Result should have had bottom left {0.f, 0.f}, width of 10.f and height of 25.f, but instead had bottom left {%f, %f}, width %f, and height %f",
                  result[0].bottom_left.x, result[0].bottom_left.y, result[0].w, result[0].h);
}
END_TEST

START_TEST(layout_stairs_three_floors)
{
    /*
    *  Given a house specification with {w = 30, h = 30, num_rooms = 3, stair_width = 7} and room specifications containing one room {max_instances = 3, area = 690}
    *  When I invoke pb_sq_house_layout_stairs
    *  Then the result should be 3 rectangles with areas 690, 480, 690 and house with 3 floors containing 2, 3 and 2 rooms
    */
    pb_sq_house_house_spec h_spec;
    pb_sq_house_room_spec living_room;
    char* rooms[] = { "Living Room", "Living Room", "Living Room" };
    pb_hashmap* room_specs = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_rect* result;
    pb_building house;

    float expected_areas[3] = { 900.f, 900.f, 900.f };
    pb_rect temp;
    size_t expected_num_rooms[] = { 2, 3, 2 };
    unsigned int i;

    h_spec.width = 30.f;
    h_spec.height = 30.f;
    h_spec.num_rooms = 3;
    h_spec.stair_room_width = 7.f;

    living_room.area = 690.f;
    living_room.name = "Living Room";

    pb_hashmap_put(room_specs, (void*)living_room.name, (void*)&living_room);

    result = pb_sq_house_layout_stairs(&rooms[0], room_specs, &h_spec, &house);
    ck_assert_msg(house.num_floors == 3, "House should have had 3 floors, but had %lu", house.num_floors);

    /* The remaining areas will depend on the stairs which are assigned randomly */
    /* Assuming that the correct number of floors were created, we know how many stairs are on each floor and can add up their areas */
    pb_shape_to_pb_rect(&house.floors[0].rooms[0].room_shape, &temp);
    expected_areas[0] -= temp.w * temp.h;

    pb_shape_to_pb_rect(&house.floors[1].rooms[0].room_shape, &temp);
    expected_areas[1] -= temp.w * temp.h;
    pb_shape_to_pb_rect(&house.floors[1].rooms[1].room_shape, &temp);
    expected_areas[1] -= temp.w * temp.h;

    pb_shape_to_pb_rect(&house.floors[2].rooms[0].room_shape, &temp);
    expected_areas[2] -= temp.w * temp.h;

    for (i = 0; i < house.num_floors; ++i) {
        float area = result[i].w * result[i].h;
        ck_assert_msg(assert_close_enough(area, expected_areas[i], 5), "Area for rectangle %i should have been about %.3f, was %.3f", i, expected_areas[i], area);
        ck_assert_msg(house.floors[i].num_rooms == expected_num_rooms[i], "%ith floor should have had %lu rooms, had %lu rooms", i, expected_num_rooms[i], house.floors[i].num_rooms);
    }
}
END_TEST

START_TEST(layout_stairs_big_stairs)
{
    /*
     *  Given a house specification with {w = 30, h = 30, num_rooms = 2, stair_width = 9} and room specifications containing one room {max_instances = 2, area = 895}
     *  When I invoke pb_sq_house_layout_stairs
     *  Then pb_layout_stairs should resize stair_width to 7.5 (30 * 0.25), yielding 2 rectangles with areas of 675, and house with 2 floors containing 2 rooms each
     */
    pb_sq_house_house_spec h_spec;
    pb_sq_house_room_spec living_room;
    char* rooms[] = { "Living Room", "Living Room" };
    pb_hashmap* room_specs = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_rect* result;
    pb_building house;

    float expected_areas[3] = { 675.f, 675.f };
    size_t expected_num_rooms[] = { 2, 2 };
    unsigned int i;

    h_spec.width = 30.f;
    h_spec.height = 30.f;
    h_spec.num_rooms = 2;
    h_spec.stair_room_width = 9.f;

    living_room.area = 675.f;
    living_room.name = "Living Room";

    pb_hashmap_put(room_specs, (void*)living_room.name, (void*)&living_room);

    result = pb_sq_house_layout_stairs(&rooms[0], room_specs, &h_spec, &house);
    ck_assert_msg(house.num_floors == 2, "House should have had 2 floors, but had %lu", house.num_floors);
    for (i = 0; i < house.num_floors; ++i) {
        float area = result[i].w * result[i].h;
        ck_assert_msg(assert_close_enough(area, expected_areas[i], 5), "Area for rectangle %i should have been about %.f, was %.f", i, expected_areas[i], area);
        ck_assert_msg(house.floors[i].num_rooms == expected_num_rooms[i], "%ith floor should have had %lu rooms, had %lu rooms", i, expected_num_rooms[i], house.floors[i].num_rooms);
    }
}
END_TEST

START_TEST(fill_floor_finished_rect_has_children_x)
{
    /* Given a remaining floor rectangle with dimensions 20 by 40, and two children both widths of 20 and heights of 10 
     * When I invoke pb_sq_house_fill_remaining_floor 
     * Then both rectangles should have their heights adjusted to approximately 40 */
    pb_rect floor_rect = { { 0.f, 0.f }, 20.f, 40.f };
    int rect_has_children = 1;
    pb_rect children[2] = {
        { {0.f, 0.f}, 10.f, 20.f },
        { {10.f, 0.f}, 10.f, 20.f }
    };
    size_t last_row_size = 2;

    pb_sq_house_fill_remaining_floor(&floor_rect, rect_has_children, &children[0], last_row_size);

    ck_assert_msg(assert_close_enough(children[0].h, 40.f, 5), "First rectangle had height of %.3f, should have been around 40.0f", children[0].h);
    ck_assert_msg(assert_close_enough(children[1].h, 40.f, 5), "Second rectangle had height of %.3f, should have been around 40.0f", children[1].h);
}
END_TEST

START_TEST(fill_floor_finished_rect_has_children_y)
{
    /* Given a remaining floor rectangle with dimensions 40 by 20, and two children with both with widths of 10 and heights of 20
     * When I invoke pb_sq_house_fill_remaining_floor
     * Then both rectangles should have their widths adjusted to approximately 40 */
    pb_rect floor_rect = { { 0.f, 0.f }, 40.f, 20.f };
    int rect_has_children = 1;
    pb_rect children[2] = {
        { { 0.f, 0.f }, 20.f, 10.f },
        { { 0.f, 10.f }, 20.f, 10.f }
    };
    size_t last_row_size = 2;

    pb_sq_house_fill_remaining_floor(&floor_rect, rect_has_children, &children[0], last_row_size);

    ck_assert_msg(assert_close_enough(children[0].w, 40.f, 5), "First rectangle had width of %.3f, should have been around 40.0f", children[0].w);
    ck_assert_msg(assert_close_enough(children[1].w, 40.f, 5), "Second rectangle had width of %.3f, should have been around 40.0f", children[1].w);
}
END_TEST

START_TEST(fill_floor_finished_rect_no_children_top)
{
    /* Given a remaining floor rectangle with dimensions 40 by 20, bottom left corner at {0, 20}, no children, and the two rectangles in the previous row below the final rect
     * When I invoke pb_sq_house_fill_remaining_floor
     * Then both previous-row rectangles should have their heights adjusted to approximately 40 */
    pb_rect floor_rect = { { 0.f, 20.f }, 40.f, 20.f };
    int rect_has_children = 0;
    pb_rect children[2] = {
        { { 0.f, 0.f }, 20.f, 20.f },
        { { 20.f, 0.f }, 20.f, 20.f }
    };
    size_t last_row_size = 2;

    pb_sq_house_fill_remaining_floor(&floor_rect, rect_has_children, &children[0], last_row_size);

    ck_assert_msg(assert_close_enough(children[0].h, 40.f, 5), "First rectangle had width of %.3f, should have been around 40.0f", children[0].w);
    ck_assert_msg(assert_close_enough(children[1].h, 40.f, 5), "Second rectangle had width of %.3f, should have been around 40.0f", children[1].w);
}
END_TEST

START_TEST(fill_floor_finished_rect_no_children_right)
{
    /* Given a remaining floor rectangle with dimensions 20 by 40, bottom left corner at {20, 0}, no children, and the two rectangles in the previous row left of the final rect
     * When I invoke pb_sq_house_fill_remaining_floor
     * Then both previous-row rectangles should have their widths adjusted to approximately 40 */
    pb_rect floor_rect = { { 20.f, 0.f }, 20.f, 40.f };
    int rect_has_children = 0;
    pb_rect children[2] = {
        { { 0.f, 0.f }, 20.f, 20.f },
        { { 0.f, 20.f }, 20.f, 20.f }
    };
    size_t last_row_size = 2;

    pb_sq_house_fill_remaining_floor(&floor_rect, rect_has_children, &children[0], last_row_size);

    ck_assert_msg(assert_close_enough(children[0].w, 40.f, 5), "First rectangle had height of %.3f, should have been around 40.0f", children[0].h);
    ck_assert_msg(assert_close_enough(children[1].w, 40.f, 5), "Second rectangle had height of %.3f, should have been around 40.0f", children[1].h);
}
END_TEST

START_TEST(layout_floor_single_room)
{
    /* Given a floor with a single room
     * When I invoke pb_sq_house_layout_floor
     * The room should occupy the entire floor rectangle */
    char const* rooms[] = { "Living Room" };
    pb_sq_house_room_spec lr;
    pb_hashmap* map = pb_hashmap_create(pb_str_hash, pb_str_eq);
    pb_rect floor_rect = {
        { 0.f, 0.f },
        40.f,
        40.f
    };
    pb_floor f;
    pb_room f_rooms[1];
    pb_rect result;

    f.num_rooms = 1;
    f.rooms = &f_rooms[0];
    
    lr.area = 90.f;
    pb_hashmap_put(map, (void*)&rooms[0], (void*)&lr);
    pb_sq_house_layout_floor(&rooms[0], map, &f, 1, &floor_rect);

    pb_shape_to_pb_rect(&f.rooms[0].room_shape, &result);
    ck_assert_msg(assert_close_enough(result.w, floor_rect.w, 5), "Result's width should have been about %.3f, was %.3f", floor_rect.w, result.w);
    ck_assert_msg(assert_close_enough(result.h, floor_rect.h, 5), "Result's height should have been about %.3f, was %.3f", floor_rect.h, result.h);

    free(f.rooms[0].room_shape.points);
    pb_hashmap_free(map);
}
END_TEST

Suite *make_pb_sq_house_layout_suite(void)
{
    Suite* s;
    TCase* tc_sq_house_choose_rooms;
    TCase* tc_sq_house_layout_stairs;
    TCase* tc_sq_house_layout_floor;
    TCase* tc_sq_house_fill_floor;

    s = suite_create("Squarified house generation");

    tc_sq_house_choose_rooms = tcase_create("Room selection tests");
    suite_add_tcase(s, tc_sq_house_choose_rooms);
    tcase_add_test(tc_sq_house_choose_rooms, choose_rooms_single_room);
    tcase_add_test(tc_sq_house_choose_rooms, choose_rooms_multiple_rooms);
    tcase_add_test(tc_sq_house_choose_rooms, choose_rooms_house_too_big);
    tcase_add_test(tc_sq_house_choose_rooms, choose_rooms_no_outside);

    tc_sq_house_layout_stairs = tcase_create("Stair layout tests");
    suite_add_tcase(s, tc_sq_house_layout_stairs);
    tcase_add_test(tc_sq_house_layout_stairs, layout_stairs_single_floor);
    tcase_add_test(tc_sq_house_layout_stairs, layout_stairs_three_floors);
    tcase_add_test(tc_sq_house_layout_stairs, layout_stairs_big_stairs);

    tc_sq_house_layout_floor = tcase_create("Floor layout tests");
    suite_add_tcase(s, tc_sq_house_layout_floor);
    tcase_add_test(tc_sq_house_layout_floor, layout_floor_single_room);

    tc_sq_house_fill_floor = tcase_create("Fill remaining floor tests");
    suite_add_tcase(s, tc_sq_house_fill_floor);
    tcase_add_test(tc_sq_house_fill_floor, fill_floor_finished_rect_has_children_x);
    tcase_add_test(tc_sq_house_fill_floor, fill_floor_finished_rect_has_children_y);
    tcase_add_test(tc_sq_house_fill_floor, fill_floor_finished_rect_no_children_right);
    tcase_add_test(tc_sq_house_fill_floor, fill_floor_finished_rect_no_children_top);
    
    return s;
}
