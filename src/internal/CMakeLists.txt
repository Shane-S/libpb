set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})

set(HEADERS ${PB_API_INCLUDE_DIR}/pb/internal/squarify.h
            ${PB_API_INCLUDE_DIR}/pb/internal/astar.h
            ${PB_API_INCLUDE_DIR}/pb/internal/sq_house_layout.h
            ${PB_API_INCLUDE_DIR}/pb/internal/sq_house_graph.h)

set(SOURCES squarify.c
            astar.c
            sq_house_layout.c
            sq_house_graph.c)

add_library(pb_internal OBJECT ${SOURCES} ${HEADERS})