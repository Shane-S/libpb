set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})

add_subdirectory(util)
add_subdirectory(internal)

# The header files won't show up in Visual Studio (and probably XCode) if they're not added to the source list
set(HEADERS ${PB_API_INCLUDE_DIR}/pb/sq_house.h
            ${PB_API_INCLUDE_DIR}/pb/floor_plan.h
            ${PB_API_INCLUDE_DIR}/pb/extrusion.h
            ${PB_API_INCLUDE_DIR}/pb/exports.h)

set(SOURCES extrusion.c
            sq_house.c floor_plan.c ../include/pb/simple_extruder.h simple_extruder.c)
            
add_library(pb ${SOURCES} ${HEADERS}
            $<TARGET_OBJECTS:pb_internal>)

if (UNIX)
    target_compile_options(pb PRIVATE -fPIC)
    target_link_libraries(pb -lm)
endif()

target_link_libraries(pb pb_util)

install(TARGETS pb
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
