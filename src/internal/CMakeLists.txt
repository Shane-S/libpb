cmake_minimum_required (VERSION 2.6)

set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})

set(HEADERS ${PB_API_INCLUDE_DIR}/pb/internal/pb_squarify.h
            ${PB_API_INCLUDE_DIR}/pb/internal/pb_sq_house_internal.h)

set(SOURCES pb_squarify.c
            pb_sq_house_internal.c)


add_library(libpb_internal STATIC ${SOURCES} ${HEADERS})
target_link_libraries(libpb_internal libpb_util)