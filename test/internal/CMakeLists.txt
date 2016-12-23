# Build the test executable for the internal API
set(SOURCES pb_squarify_test.c
            pb_astar_test.c
            pb_sq_house_layout_test.c
            pb_sq_house_graph_test.c
            pb_internal_test_main.c
            ../test_util.c)
set(HEADERS pb_internal_test.h 
            ../test_util.h)

add_executable(pb_internal_test ${SOURCES} ${HEADERS}
               $<TARGET_OBJECTS:pb_internal>)
target_link_libraries(pb_internal_test pb_util)

if (UNIX)
    target_link_libraries(pb_internal_test -lm)
endif()

if( CHECK_FOUND )
    if( CHECK_CMAKE_TARGET_FOUND )
        target_link_libraries(pb_internal_test check)
    else( CHECK_CMAKE_TARGET_FOUND)
        target_link_libraries(pb_internal_test ${CHECK_LIBRARIES})
        target_include_directories(pb_internal_test PRIVATE ${CHECK_INCLUDE_DIR})
    endif()
endif()
