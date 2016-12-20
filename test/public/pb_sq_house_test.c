#include <pb/sq_house.h>
#include <check.h>

#ifndef _WIN32
#include <time.h>
#include <pb/util/hashmap/hash_utils.h>
#include <stdio.h>
#include <stdlib.h>

#else
#endif

START_TEST(sq_house_performance_test)
{
    pb_sq_house_room_spec specs[8] = {0};

    char const* living_adj[] = {
            PB_SQ_HOUSE_OUTSIDE,
            PB_SQ_HOUSE_STAIRS,
            "Laundry room",
            "Kitchen",
            "Dining room",
            "Bathroom",
            "Bedroom",
            "Master bedroom",
    };
    specs[0].name = "Living room";
    specs[0].adjacent = &living_adj[0];
    specs[0].num_adjacent = sizeof(living_adj) / sizeof(char*);
    specs[0].priority = 0;
    specs[0].max_instances = 1;
    specs[0].area = 20.f;

    char const* kitchen_adj[] = {
            PB_SQ_HOUSE_OUTSIDE,
            "Pantry",
            "Laundry room",
            "Living room",
            "Dining room"
    };
    specs[1].name = "Kitchen";
    specs[1].adjacent = &kitchen_adj[0];
    specs[1].num_adjacent = sizeof(kitchen_adj) / sizeof(char*);
    specs[1].priority = 1;
    specs[1].max_instances = 1;
    specs[1].area = 15.f;

    char const* pantry_adj[] = {
            "Laundry room",
            "Kitchen"
    };
    specs[2].name = "Pantry";
    specs[2].adjacent = &pantry_adj[0];
    specs[2].num_adjacent = sizeof(pantry_adj) / sizeof(char*);
    specs[2].priority = 6;
    specs[2].max_instances = 1;
    specs[2].area = 5.f;

    char const* laundry_adj[] = {
            PB_SQ_HOUSE_STAIRS,
            "Kitchen",
            "Pantry",
    };
    specs[3].name = "Laundry room";
    specs[3].adjacent = &laundry_adj[0];
    specs[3].num_adjacent = sizeof(laundry_adj) / sizeof(char*);
    specs[3].priority = 4;
    specs[3].max_instances = 1;
    specs[3].area = 9.f;

    char const* dining_adj[] = {
            "Kitchen",
            "Living room",
            "Bathroom",
            "Bedroom",
            "Master bedroom",
    };
    specs[4].name = "Dining room";
    specs[4].adjacent = &dining_adj[0];
    specs[4].num_adjacent = sizeof(dining_adj) / sizeof(char*);
    specs[4].priority = 5;
    specs[4].max_instances = 1;
    specs[4].area = 15.f;

    char const* bathroom_adj[] = {
            PB_SQ_HOUSE_STAIRS,
            "Living room",
            "Dining room",
            "Bedroom",
            "Master bedroom",
    };
    specs[5].name = "Bathroom";
    specs[5].adjacent = &bathroom_adj[0];
    specs[5].num_adjacent = sizeof(bathroom_adj) / sizeof(char*);
    specs[5].priority = 2;
    specs[5].max_instances = 5;
    specs[5].area = 7.f;

    char const* bedroom_adj[] = {
            PB_SQ_HOUSE_STAIRS,
            "Living room",
            "Dining room",
            "Bathroom",
    };
    specs[6].name = "Bedroom";
    specs[6].adjacent = &bedroom_adj[0];
    specs[6].num_adjacent = sizeof(bedroom_adj) / sizeof(char*);
    specs[6].priority = 3;
    specs[6].max_instances = 5;
    specs[6].area = 10.f;

    char const* master_adj[] = {
            PB_SQ_HOUSE_STAIRS,
            "Living room",
            "Dining room",
            "Bathroom",
    };
    specs[7].name = "Master bedroom";
    specs[7].adjacent = &master_adj[0];
    specs[7].num_adjacent = sizeof(master_adj) / sizeof(char*);
    specs[7].priority = 7;
    specs[7].max_instances = 1;
    specs[7].area = 15.f;

    size_t i;
    pb_hashmap* room_specs = pb_hashmap_create(pb_str_hash, pb_str_eq);
    for (i = 0; i < 8; ++i) {
        pb_hashmap_put(room_specs, specs[i].name, &specs[i]);
    }

    pb_sq_house_house_spec hspec;
    hspec.num_rooms = 15;
    hspec.door_size = 0.75f;
    hspec.window_size = 0.5f;
    hspec.hallway_width = 0.75f;
    hspec.stair_room_width = 3.f;
    hspec.width = 15.f;
    hspec.height = 10.f;

    size_t const iters = 1000;
    float ms_sum = 0.f;
    for (i = 0; i < iters; ++i) {
#ifndef _WIN32
        struct timespec start;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
#else
        /* Query performance timer */
#endif
        pb_building* b = pb_sq_house(&hspec, room_specs);

#ifndef _WIN32
        struct timespec end;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

        struct timespec diff = {end.tv_sec - start.tv_sec, end.tv_nsec - start.tv_nsec};
        ms_sum += (diff.tv_sec * 1000.f) + (diff.tv_nsec / 1000000.f);
#else
        /* Query performance timer */
#endif

        pb_building_free(b, pb_sq_house_free_building, pb_sq_house_free_floor, pb_sq_house_free_room);
        free(b);
    }
    float avg_ms = ms_sum / iters;
    printf("Average number of milliseconds: %.4f\n", avg_ms);
    pb_hashmap_free(room_specs);
}
END_TEST

Suite *make_pb_generation_suite(void) {
    Suite *s;
    TCase *tc_sq_house_performance;

    s = suite_create("Squarified House creation tests");

    tc_sq_house_performance = tcase_create("Performance test");
    suite_add_tcase(s, tc_sq_house_performance);
    tcase_add_test(tc_sq_house_performance, sq_house_performance_test);

    return s;
}