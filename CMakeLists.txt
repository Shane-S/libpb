cmake_minimum_required (VERSION 3.1)
project (libpb)

set (libpb_VERSION_MAJOR 0)
set (libpb_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(src)
add_subdirectory(test)