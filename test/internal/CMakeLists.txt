# Build the test executable for the internal API
set(SOURCES pb_squarify_test.c
            pb_sq_house_test.c
            pb_internal_test_main.c
            ../test_util.c)
set(HEADERS pb_internal_test.h 
            ../test_util.h)

add_executable(pb_internal_test ${SOURCES} ${HEADERS})
target_link_libraries(pb_internal_test libpb_internal check compat)