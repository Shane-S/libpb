cmake_minimum_required (VERSION 2.6)
project (libpb)

set (libpb_VERSION_MAJOR 0)
set (libpb_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(src)
add_subdirectory(test)