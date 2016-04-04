cmake_minimum_required (VERSION 2.6)
project (libpb_internal)

set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR}/include)

set(HEADERS ${PB_API_INCLUDE_DIR}/pb/internal/pb_squarify.h)
set(HEADERS ${PB_API_INCLUDE_DIR}/pb/internal/pb_graph.h)

set(SOURCES pb_graph.c)
set(SOURCES ${SOURCES} pb_squarify.c)

add_library(libpb_internal STATIC ${SOURCES} ${HEADERS})
