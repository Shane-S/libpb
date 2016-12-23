# Build the test executable for the public API
set(SOURCES pb_heap_test.c
            pb_hash_test.c
            pb_graph_test.c
            pb_vertex_test.c
            pb_vector_test.c
            pb_geom_test.c
            pb_util_test_main.c
            ../test_util.c triangulate_test.c)

set(HEADERS pb_util_test.h
            ../test_util.h)

add_executable(pb_util_test ${SOURCES} ${HEADERS})
target_link_libraries(pb_util_test pb_util check)

if( CHECK_FOUND )
    if( CHECK_CMAKE_TARGET_FOUND )
        target_link_libraries(pb_util_test check)
    else( CHECK_CMAKE_TARGET_FOUND )
        target_link_libraries(pb_util_test ${CHECK_LIBRARIES})
        target_include_directories(pb_util_test PRIVATE ${CHECK_INCLUDE_DIR})
    endif()
endif()

