cmake_minimum_required (VERSION 2.6)

set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})

add_subdirectory(internal)

# The header files won't show up in Visual Studio (and probably XCode) if they're not added to the source list
set(HEADERS ${PB_API_INCLUDE_DIR}/pb/pb_generation.h
            ${PB_API_INCLUDE_DIR}/pb/pb_extrusion.h
            ${PB_API_INCLUDE_DIR}/pb/pb_hash.h
            ${PB_API_INCLUDE_DIR}/pb/pb_hash_utils.h
            ${PB_API_INCLUDE_DIR}/pb/pb_types.h
            ${PB_API_INCLUDE_DIR}/pb/pb_util.h
            ${PB_API_INCLUDE_DIR}/pb/pb_exports.h)

set(SOURCES pb_generation.c
            pb_extrusion.c
            pb_hash.c
            pb_hash_utils.c)

add_library(libpb SHARED ${SOURCES} ${HEADERS})
target_link_libraries(libpb libpb_internal)
