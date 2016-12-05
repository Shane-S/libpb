# Tries to find the check library
# 
# First tries to find check in the CMake package registry
# Next, tries to use pkg-config
# If that fails and we're on Windows, checks the default cmake install directory for CMake files,
# Then tries to use find_library and find_path to get the library and include paths
# Finally ouputs a warning (or an error depending on whether it's required) if it's not found; the user has to configure manually
#
# Variables:
#    CHECK_FOUND:              True when check (CMake config or otherwise) is found.
#    CHECK_CMAKE_TARGET_FOUND: Set to true when the check CMake config files were found. "check" is a target in this case.
#    CHECK_INCLUDE_DIR:        The include directories for check. Set only if CHECK_CMAKE_TARGET_FOUND is false.
#    CHECK_LIBRARIES:          The libraries to be linked against. Set only if CHECK_CMAKE_TARGET_FOUND is false.

set( CHECK_FOUND FALSE )

# Check the package registry (I mean, it's worth a shot right?)
find_package( check QUIET CONFIG )
if( check_FOUND )
    set( CHECK_FOUND TRUE )
    set( CHECK_CMAKE_TARGET_FOUND TRUE )
endif( check_FOUND )

# Try pkg-config
# This is basically the same code as the original Findcheck.cmake file
if( NOT CHECK_FOUND )
    include( FindPkgConfig )
    if( PKG_CONFIG_FOUND )
        pkg_search_module( CHECK check )

        if( CHECK_FOUND AND WIN32 AND NOT CMAKE_CROSSCOMPILING AND NOT CHECK_INCLUDE_DIR )
            # pkg-config didn't set the check include dir on Windows last time I tried
            # see what we can do about that
            
            # properly stringify pkg-config output to handle directories with spaces
            # this probablt needs to happen for all platforms, but I currently have a path without spaces on Linux
            # so this will have to do for now
            string( REPLACE ";" " " CHECK_CFLAGS_STRING "${CHECK_CFLAGS}")
            string( REPLACE ";" " " CHECK_CFLAGS_OTHER_STRING "${CHECK_CFLAGS_OTHER}")
            set( CHECK_ALL_CFLAGS_STRING "${CHECK_CFLAGS_STRING} ${CHECK_CFLAGS_OTHER_STRING}")
            
            # get all include directories and find which one (if any) contains check.h
            string( REGEX MATCHALL "-I[^-]*" INCLUDE_DIRS ${CHECK_ALL_CFLAGS_STRING} ) 
            foreach( INCLUDE_DIR IN LISTS INCLUDE_DIRS )
                string( REGEX REPLACE "[/-]I\\s*" "" STRIPPED_DIR "${INCLUDE_DIR}")
                string( STRIP "${STRIPPED_DIR}" STRIPPED_DIR )
                if( EXISTS "${STRIPPED_DIR}/check.h" )
                    set( CHECK_INCLUDE_DIR "${STRIPPED_DIR}" )
                    set( TO_REMOVE "${INCLUDE_DIR}")
                    break()
                endif()
            endforeach( INCLUDE_DIR )

            # None of the include directories contained check
            # This probably shouldn't happen, but we'll leave it just in case
            if( NOT CHECK_INCLUDE_DIR )
                set( CHECK_FOUND FALSE )
            else( NOT CHECK_INCLUDE_DIR )
                # Remove the include from CFLAGS since it will be added anyway
                string( REPLACE "${TO_REMOVE}" "" CHECK_CFLAGS "${CHECK_CFLAGS}" )
                string( REPLACE "${TO_REMOVE}" "" CHECK_CFLAGS_OTHER "${CHECK_CFLAGS_OTHER}" )
            endif()
        endif()
    endif()
endif()

# Didn't find check CMake config or with pkg-config
# Time to start guessing and hope for the best
if( NOT CHECK_FOUND )

    # Take a look at the default Windows CMake installation directory for check
    if( WIN32 AND NOT CMAKE_CROSSCOMPILING )
        # Determine whether we're compiling for 64-bit
        if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
            set( SIXTY_FOUR_BIT_TARGET_ARCH TRUE )
        else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
            set( SIXTY_FOUR_BIT_TARGET_ARCH FALSE )
        endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        
        # Determine whether the machine itself is 64-bit
        set( IS_64BIT_MACHINE (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64" OR
                               ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "IA64" OR
                               ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "EM64T") )
        
        if( SIXTY_FOUR_BIT_TARGET_ARCH )
            set( CHECK_INSTALL_DIR "C:\\Program Files\\check" )
        else( SIXTY_FOUR_BIT_TARGET_ARCH )
            # The target architecture is 32-bit, but it could be that we're still on a 64-bit machine
            # If we are, then "Program Files" will be "Program Files (x86)"
            if( IS_64BIT_MACHINE )
                set( CHECK_INSTALL_DIR "C:\\Program Files (x86)\\check" )
            else( IS_64BIT_MACHINE )
                set( CHECK_INSTALL_DIR "C:\\Program Files\\check" )
            endif( IS_64BIT_MACHINE )
        endif( SIXTY_FOUR_BIT_TARGET_ARCH )

        if( EXISTS ${CHECK_INSTALL_DIR}/cmake/check.cmake )
            include( ${CHECK_INSTALL_DIR}/cmake/check.cmake )
            set( CHECK_FOUND TRUE )
            set( CHECK_CMAKE_TARGET_FOUND TRUE )
        endif()
    else( WIN32 AND NOT CMAKE_CROSSCOMPILING )
            # I stole this part from the check source. Some things have been changed to lowercase,
            # and I also removed the search for libcompat since it's an internal library and it
            # was making me sad

            # - Try to find the CHECK libraries
            #  This configuration file for finding libcheck is originally from
            #  the opensync project. The originally was downloaded from here:
            #  opensync.org/browser/branches/3rd-party-cmake-modules/modules/FindCheck.cmake
            #
            #  Copyright (c) 2007 Daniel Gollub <dgollub@suse.de>
            #  Copyright (c) 2007 Bjoern Ricks  <b.ricks@fh-osnabrueck.de>
            #
            #  Redistribution and use is allowed according to the terms of the New
            #  BSD license.
        if ( CHECK_INSTALL_DIR )
		    message ( STATUS "Using override CHECK_INSTALL_DIR to find check" )
		    set ( CHECK_INCLUDE_DIR  "${CHECK_INSTALL_DIR}/include" )
		    set ( CHECK_INCLUDE_DIRS "${CHECK_INCLUDE_DIR}" )
		    find_library( CHECK_LIBRARY NAMES check PATHS "${CHECK_INSTALL_DIR}/lib" )
		    set ( CHECK_LIBRARIES "${CHECK_LIBRARY}" )
	    else ( CHECK_INSTALL_DIR )
		    find_path( CHECK_INCLUDE_DIR check.h )
		    find_library( CHECK_LIBRARIES NAMES check )
	    endif ( CHECK_INSTALL_DIR )

	    if ( CHECK_INCLUDE_DIR AND CHECK_LIBRARIES )
		    set( CHECK_FOUND 1 )
		    if ( NOT Check_FIND_QUIETLY )
			    message ( STATUS "Found CHECK: ${CHECK_LIBRARIES}" )
		    endif ( NOT Check_FIND_QUIETLY )
	    else ( CHECK_INCLUDE_DIR AND CHECK_LIBRARIES )
		    if ( Check_FIND_REQUIRED )
			    message( FATAL_ERROR "Could NOT find CHECK" )
		    else ( Check_FIND_REQUIRED )
			    if ( NOT Check_FIND_QUIETLY )
				    message( STATUS "Could NOT find CHECK" )	
			    endif ( NOT Check_FIND_QUIETLY )
		    endif ( Check_FIND_REQUIRED )
	    endif ( CHECK_INCLUDE_DIR AND CHECK_LIBRARIES )
    endif( WIN32 AND NOT CMAKE_CROSSCOMPILING )
endif()

# Hide advanced variables from CMake GUIs
mark_as_advanced( CHECK_INCLUDE_DIR CHECK_LIBRARIES )
