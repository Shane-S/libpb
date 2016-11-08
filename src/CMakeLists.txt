set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})

add_subdirectory(util)
add_subdirectory(internal)

# The header files won't show up in Visual Studio (and probably XCode) if they're not added to the source list
set(HEADERS ${PB_API_INCLUDE_DIR}/pb/pb_sq_house.h
            ${PB_API_INCLUDE_DIR}/pb/pb_floor_plan.h
            ${PB_API_INCLUDE_DIR}/pb/pb_extrusion.h
            ${PB_API_INCLUDE_DIR}/pb/pb_exports.h)

set(SOURCES pb_extrusion.c
            pb_sq_house.c)
            
add_library(pb SHARED ${SOURCES} ${HEADERS})
target_link_libraries(pb pb_internal)
target_link_libraries(pb pb_util)
