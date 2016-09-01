#include <libcompat.h>
#include <check.h>
#include <string.h>
#include <pb/pb_sq_house.h>
#include <pb/internal/pb_sq_house_internal.h>
#include <pb/util/pb_hash.h>
#include <pb//util/pb_hash_utils.h>

START_TEST(single_room_selection)
{
    pb_sq_house_room_spec specs[1];
    pb_hash* rooms = pb_hash_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;
    char** result;

    specs[0].max_instances = 1;
    specs[0].name = "Closet";

    pb_hash_put(rooms, (void*)"Closet", (void*)&specs[0]);
    spec.num_rooms = 1;

    result = pb_sq_house_choose_rooms(rooms, &spec);

    ck_assert_msg(strcmp(result[0], specs[0].name) == 0, "Result should contain closet, but instead contained %s", result[0]);

    pb_hash_free(rooms);
    free(result);
}
END_TEST

START_TEST(multi_room_selection)
{
    pb_sq_house_room_spec specs[2];
    pb_hash* rooms = pb_hash_create(pb_str_hash, pb_str_eq);
    pb_sq_house_house_spec spec;

    pb_hash* instances = pb_hash_create(pb_str_hash, pb_str_eq); /* Stores the number of each room type from the result array */
    void* spec0_instances;
    void* spec1_instances;
    char** result;

    int i;

    specs[0].max_instances = 6;
    specs[0].name = "Closet";
    specs[1].max_instances = 6;
    specs[1].name = "Bathroom";

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

START_TEST(house_too_big_room_selection)
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

Suite *make_pb_sq_house_suite(void)
{
    Suite *s;
    TCase *tc_sq_house_choose_rooms;

    s = suite_create("Squarified house generation");

    tc_sq_house_choose_rooms = tcase_create("Room selection tests");
    suite_add_tcase(s, tc_sq_house_choose_rooms);
    tcase_add_test(tc_sq_house_choose_rooms, single_room_selection);
    tcase_add_test(tc_sq_house_choose_rooms, multi_room_selection);
    tcase_add_test(tc_sq_house_choose_rooms, house_too_big_room_selection);
    
    return s;
}
