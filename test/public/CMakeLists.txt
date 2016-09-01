# Build the test executable for the public API
set(SOURCES pb_extrusion_test.c
            pb_public_test_main.c
            ../test_util.c)
set(HEADERS pb_public_test.h ../test_util.h)
add_executable(pb_public_test ${SOURCES} ${HEADERS})
target_link_libraries(pb_public_test libpb check compat)