#include <libcompat.h>
#include <check.h>
#include <string.h>
#include <pb/pb_sq_house.h>
#include <pb/internal/pb_sq_house_internal.h>
#include <pb/util/pb_hash.h>
#include <pb//util/pb_hash_utils.h>

START_TEST(choose_rooms_single_room)
{
    pb_sq_house_room_spec specs[1];
    pb_hash* rooms = pb_hash_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;
    char** result;

    char* adjacent[] = { PB_SQ_HOUSE_OUTSIDE, "Closet" };

    specs[0].max_instances = 1;
    specs[0].name = "Closet";
    specs[0].adjacent = &adjacent[0];
    specs[0].num_adjacent = 2;

    pb_hash_put(rooms, (void*)"Closet", (void*)&specs[0]);
    spec.num_rooms = 1;

    result = pb_sq_house_choose_rooms(rooms, &spec);

    ck_assert_msg(strcmp(result[0], specs[0].name) == 0, "Result should contain closet, but instead contained %s", result[0]);

    pb_hash_free(rooms);
    free(result);
}
END_TEST

START_TEST(choose_rooms_multiple_rooms)
{
    pb_sq_house_room_spec specs[2];
    pb_hash* rooms = pb_hash_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;

    pb_hash* instances = pb_hash_create(pb_str_hash, pb_str_eq); /* Stores the number of each room type from the result array */
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

    pb_hash_put(rooms, (void*)specs[0].name, (void*)&specs[0]);
    pb_hash_put(rooms, (void*)specs[1].name, (void*)&specs[1]);
    spec.num_rooms = 12;

    result = pb_sq_house_choose_rooms(rooms, &spec);

    pb_hash_put(instances, (void*)specs[0].name, (void*)0);
    pb_hash_put(instances, (void*)specs[1].name, (void*)0);

    for (i = 0; i < 12; ++i) {
        if (strcmp(result[i], specs[0].name) == 0) {
            void* num;
            pb_hash_get(instances, specs[0].name, &num);
            num = (void*)((int)num + 1);
            pb_hash_put(instances, specs[0].name, num);
        } else if(strcmp(result[i], specs[1].name) == 0) {
            void* num;
            pb_hash_get(instances, specs[1].name, &num);
            num = (void*)((int)num + 1);
            pb_hash_put(instances, specs[1].name, num);
        } else {
            ck_abort_msg("Result string didn't match either room specification.");
        }
    }

    pb_hash_get(instances, specs[0].name, &spec0_instances);
    pb_hash_get(instances, specs[1].name, &spec1_instances);
    ck_assert_msg(spec0_instances == 6, "Result should have 6 instances of closet, but instead contained %d", (int)spec0_instances);
    ck_assert_msg(spec1_instances == 6, "Result should have 6 instances of bathroom, but instead contained %d", (int)spec1_instances);

    pb_hash_free(rooms);
    pb_hash_free(instances);
    free(result);
}
END_TEST

START_TEST(choose_rooms_house_too_big)
{
    pb_sq_house_room_spec specs[2];
    pb_hash* rooms = pb_hash_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;
    char** result;
    
    specs[0].max_instances = 6;
    specs[0].name = "Closet";
    specs[1].max_instances = 6;
    specs[1].name = "Bathroom";

    pb_hash_put(rooms, (void*)specs[0].name, (void*)&specs[0]);
    pb_hash_put(rooms, (void*)specs[1].name, (void*)&specs[1]);
    spec.num_rooms = 24;

    result = pb_sq_house_choose_rooms(rooms, &spec);
    ck_assert_msg(result == NULL, "Result should have been NULL, was %p", result);

    pb_hash_free(rooms);
    free(result);
}
END_TEST

START_TEST(choose_rooms_no_outside)
{
    pb_sq_house_room_spec specs[1];
    pb_hash* rooms = pb_hash_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;
    char** result;

    char* adjacent[] = { "Closet", "Bathroom" };

    specs[0].max_instances = 6;
    specs[0].name = "Closet";
    specs[0].adjacent = &adjacent[0];
    specs[0].num_adjacent = 2;

    pb_hash_put(rooms, (void*)specs[0].name, (void*)&specs[0]);
    spec.num_rooms = 6;

    result = pb_sq_house_choose_rooms(rooms, &spec);
    ck_assert_msg(result == NULL, "Result should have been NULL, was %p", result);

    pb_hash_free(rooms);
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
    pb_hash* room_specs = pb_hash_create(pb_str_hash, pb_str_eq);
    pb_rect* result;
    pb_building house;

    h_spec.width = 10;
    h_spec.height = 25;
    h_spec.num_rooms = 1;
    h_spec.stair_room_width = 7.f;
    
    living_room.area = 250;
    living_room.name = "Living Room";

    pb_hash_put(room_specs, (void*)living_room.name, (void*)&living_room);

    result = pb_sq_house_layout_stairs(&rooms[0], room_specs, &h_spec, &house);
    ck_assert_msg(house.num_floors == 1, "House should have had one floor, but had %lu", house.num_floors);
    ck_assert_msg(house.floors[0].num_rooms == 1, "House's first floor should have had one room, but had %lu", house.floors[0].num_rooms);
    ck_assert_msg(result[0].bottom_left.x == 0.f && result[0].bottom_left.y == 0.f && result[0].w == 10 && result[0].h == 25,
                  "Result should have had bottom left {0.f, 0.f}, width of 10.f and height of 25.f, but instead had bottom left {%f, %f}, width %f, and height %f",
                  result[0].bottom_left.x, result[0].bottom_left.y, result[0].w, result[0].h);
}
END_TEST

Suite *make_pb_sq_house_suite(void)
{
    Suite *s;
    TCase *tc_sq_house_choose_rooms;
    TCase *tc_sq_house_layout_stairs;

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
    
    return s;
}
