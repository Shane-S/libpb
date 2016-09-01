cmake_minimum_required(VERSION 2.8)

# If cross-compiling, invoke CMake with -DCHECK_INSTALL_DIR:STRING="the path" where "the path"
# points to the Check libraries compiled for the target platform
if(WIN32)

    # Determine whether we're compiling for 64-bit
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "x86_64")
    else(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "x86")
    endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    
    # Determine whether the machine itself is 64-bit
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

add_subdirectory(public)
add_subdirectory(internal)
add_subdirectory(util)

# Run the sets of test suites
enable_testing()
add_test(NAME internal COMMAND pb_internal_test)
add_test(NAME public COMMAND pb_public_test)
add_test(NAME util COMMAND pb_util_test)