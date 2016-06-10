cmake_minimum_required(VERSION 2.8)

# This will only work if compiling for a desktop - if you wanted to compile for say, Windows Phone, then you'd have to
# put the Check libraries for Windows phone somewhere and invoke CMake with -DCHECK_INSTALL_DIR:STRING="the path" where "the path"
# points to the Check libraries compiled for Windows Phone
if (WIN32)

    # Get the target architecture on windows so that we can try to set CHECK_INSTALL_DIR to something reasonable
    include(GetTargetArch)
    target_architecture(ARCH)
    
    set(IS_64BIT_MACHINE (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64" OR
                          ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "IA64" OR
                          ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "EM64T"))
    
    if(${ARCH} STREQUAL "x86_64")
        set(CHECK_INSTALL_DIR "C:\\Program Files\\check")
    else(${ARCH} STREQUAL "x86_64")
        # The target architecture is 32-bit, but it could be that we're still on a 64-bit machine
        # If we are, then "Program Files" will be "Program Files (x86)"
        if(IS_64BIT_MACHINE)
            set(CHECK_INSTALL_DIR "C:\\Program Files (x86)\\check")
        else(IS_64BIT_MACHINE)
            set(CHECK_INSTALL_DIR "C:\\Program Files\\check")
        endif(IS_64BIT_MACHINE)
    endif(${ARCH} STREQUAL "x86_64")
    
    
    # Import the check and compat libraries using the CMake-generated scripts, which should use the appropriate (MinSizeRel, Release, etc.) versions
    # for each build type
    include(${CHECK_INSTALL_DIR}/cmake/check.cmake)
    include(${CHECK_INSTALL_DIR}/cmake/compat.cmake)
else(WIN32)
    # Add the check include directories and libraries to search paths
    find_package(check REQUIRED)
    include_directories("${CHECK_INCLUDE_DIRS}")
    link_directories("${CHECK_INSTALL_DIR}/lib")    
endif(WIN32)

set(PB_API_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PB_API_INCLUDE_DIR})

# Build the test executable for the internal API
set(PB_INTERNAL_TEST_SOURCES pb_graph_test.c
                             pb_vertex_test.c
                             pb_squarify_test.c
                             pb_internal_test_main.c
                             test_util.c)
set(PB_INTERNAL_TEST_HEADERS pb_internal_test.h test_util.h)
add_executable(pb_internal_test ${PB_INTERNAL_TEST_SOURCES} ${PB_INTERNAL_TEST_HEADERS})
target_link_libraries(pb_internal_test libpb_internal check compat)

# Build the test executable for the public API
set(PB_PUBLIC_TEST_SOURCES pb_generation_test.c
                           pb_extrusion_test.c
                           pb_hash_test.c
                           pb_public_test_main.c
                           test_util.c)
set(PB_PUBLIC_TEST_HEADERS pb_public_test.h test_util.h)
add_executable(pb_public_test ${PB_PUBLIC_TEST_SOURCES} ${PB_PUBLIC_TEST_HEADERS})
target_link_libraries(pb_public_test libpb check compat)

# Run the two sets of test suites
enable_testing()
add_test(NAME internal COMMAND pb_internal_test)
add_test(NAME public COMMAND pb_public_test)