set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})

set(HEADERS ${PB_API_INCLUDE_DIR}/pb/internal/pb_squarify.h
            ${PB_API_INCLUDE_DIR}/pb/internal/pb_astar.h
            ${PB_API_INCLUDE_DIR}/pb/internal/pb_sq_house_layout.h
            ${PB_API_INCLUDE_DIR}/pb/internal/pb_sq_house_graph.h)

set(SOURCES pb_squarify.c
            pb_astar.c
            pb_sq_house_layout.c
            pb_sq_house_graph.c)

add_library(pb_internal STATIC ${SOURCES} ${HEADERS})
target_link_libraries(pb_internal pb_util )

if(UNIX)
    target_link_libraries(pb_internal -lm)
endif(UNIX)