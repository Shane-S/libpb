set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})


# The header files won't show up in Visual Studio (and probably XCode) if they're not added to the source list
set(HEADERS ${PB_API_INCLUDE_DIR}/pb/util/float_utils.h
            ${PB_API_INCLUDE_DIR}/pb/util/util_exports.h
            ${PB_API_INCLUDE_DIR}/pb/util/graph/graph.h
            ${PB_API_INCLUDE_DIR}/pb/util/geom/rect_utils.h
            ${PB_API_INCLUDE_DIR}/pb/util/geom/types.h
            ${PB_API_INCLUDE_DIR}/pb/util/geom/triangulate.h
            ${PB_API_INCLUDE_DIR}/pb/util/hashmap/hashmap.h
            ${PB_API_INCLUDE_DIR}/pb/util/hashmap/hash_utils.h
            ${PB_API_INCLUDE_DIR}/pb/util/hashmap/MurmurHash3.h
            ${PB_API_INCLUDE_DIR}/pb/util/heap/heap.h
            ${PB_API_INCLUDE_DIR}/pb/util/pair/pair.h
            ${PB_API_INCLUDE_DIR}/pb/util/vector/vector.h)

set(SOURCES hashmap/hashmap.c
            hashmap/hash_utils.c
            hashmap/MurmurHash3.c
            heap/heap.c
            graph/graph.c
            vector/vector.c
            geom/rect_utils.c
            geom/triangulate.c
            float_utils.c)
            
add_library(pb_util ${SOURCES} ${HEADERS})
target_compile_definitions(pb_util PRIVATE "BUILD_SHARED=0")

if(UNIX)
    target_link_libraries(pb_util -lm)
endif(UNIX)
