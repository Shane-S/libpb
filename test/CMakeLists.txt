set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(Findcheck)

if (NOT CHECK_FOUND)
    message(WARNING "Couldn't find check libraries; tests will not be built for this project.")
else (NOT CHECK_FOUND)
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
endif (NOT CHECK_FOUND)