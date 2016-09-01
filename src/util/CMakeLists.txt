cmake_minimum_required (VERSION 2.6)

set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})


# The header files won't show up in Visual Studio (and probably XCode) if they're not added to the source list
set(HEADERS ${PB_API_INCLUDE_DIR}/pb/util/pb_hash.h
            ${PB_API_INCLUDE_DIR}/pb/util/pb_hash_utils.h
            ${PB_API_INCLUDE_DIR}/pb/util/pb_heap.h
            ${PB_API_INCLUDE_DIR}/pb/util/pb_graph.h
            ${PB_API_INCLUDE_DIR}/pb/util/MurmurHash3.h
            ${PB_API_INCLUDE_DIR}/pb/util/pb_util_exports.h)

set(SOURCES pb_hash.c
            pb_hash_utils.c
            pb_heap.c
            pb_graph.c
            MurmurHash3.c)
            
add_library(libpb_util SHARED ${SOURCES} ${HEADERS})