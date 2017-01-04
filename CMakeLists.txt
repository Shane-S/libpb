cmake_minimum_required (VERSION 3.1)
project (pb)

set (libpb_VERSION_MAJOR 0)
set (libpb_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (BUILD_SHARED_LIBS)
    set(PB_BUILD_SHARED_LIBS 1)
else (BUILD_SHARED_LIBS)
    set(PB_BUILD_SHARED_LIBS 0)
endif()

# If we're not building shared libraries, we don't want to define any of the DECLSPEC stuff
# This will add a BUILD_SHARED_LIBS preprocessor define for all targets, allowing them to figure
# out if they're shared
add_definitions(-DPB_BUILD_SHARED_LIBS=${PB_BUILD_SHARED_LIBS})

add_subdirectory(src)
add_subdirectory(test)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        PATTERN "internal/*" EXCLUDE)

if (BUILD_SHARED_LIBS AND WIN32)
    # DLLs are installed in the RUNTIME dir, which we've set as <prefix>/bin
    # On non-DLL platforms (i.e. everything that isn't Windows), shared libraries
    # are installed in the LIBRARY directory, which in this case is <prefix>/lib
    set(PB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:pb>)
else (BUILD_SHARED_LIBS AND WIN32)
    set(PB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:pb>)
endif (BUILD_SHARED_LIBS AND WIN32)

if (BUILD_SHARED_LIBS)
    set(PB_LIB_TYPE "SHARED")
else (BUILD_SHARED_LIBS)
    set(PB_LIB_TYPE "STATIC")
endif()

set(PB_UTIL_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:pb_util>)
set(PB_INSTALLED_INCLUDE_PATH ${CMAKE_INSTALL_PREFIX}/include)

configure_file(${CMAKE_SOURCE_DIR}/pb-config.cmake.in ${CMAKE_BINARY_DIR}/pb-config.cmake.at_configured @ONLY)
file (GENERATE OUTPUT ${CMAKE_BINARY_DIR}/pb-config.cmake
      INPUT ${CMAKE_BINARY_DIR}/pb-config.cmake.at_configured)

install(FILES ${CMAKE_BINARY_DIR}/pb-config.cmake DESTINATION cmake)