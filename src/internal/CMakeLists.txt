cmake_minimum_required (VERSION 2.6)
project (libpb_internal)

set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})

set(HEADERS ${PB_API_INCLUDE_DIR}/pb/internal/pb_squarify.h
            ${PB_API_INCLUDE_DIR}/pb/internal/pb_graph.h
            ${PB_API_INCLUDE_DIR}/pb/internal/MurmurHash3.h)

set(SOURCES pb_graph.c
            pb_squarify.c
            MurmurHash3.c)

add_library(libpb_internal STATIC ${SOURCES} ${HEADERS})
