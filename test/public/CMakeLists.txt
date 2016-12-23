# Build the test executable for the public API
set(SOURCES pb_extrusion_test.c
            pb_public_test_main.c
            ../test_util.c)
set(HEADERS pb_public_test.h ../test_util.h pb_sq_house_test.c)
add_executable(pb_public_test ${SOURCES} ${HEADERS})
target_link_libraries(pb_public_test pb check)

if( CHECK_FOUND )
    if( CHECK_CMAKE_TARGET_FOUND )
        target_link_libraries(pb_public_test pb check)
    else( CHECK_CMAKE_TARGET_FOUND )
        target_link_libraries(pb_public_test pb ${CHECK_LIBRARIES})
        target_include_directories(pb_public_test PRIVATE ${CHECK_INCLUDE_DIR})
    endif()
endif()
