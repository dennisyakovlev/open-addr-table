cmake_minimum_required(VERSION 3.10.0)

project("mmap lib")

include(CTest)
enable_testing()
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)
# Is it required? no. You can safely remove this.
# I have here to stop me using newer stuff which gcc
# allows by default.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(include ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE FILEPATH "include dir")
set(include_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests CACHE FILEPATH "include tests dir")

if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    message(WARNING "build type not given, using defaults")
else()
    message(STATUS "given build type of ${CMAKE_BUILD_TYPE}")
endif()

if (TESTING STREQUAL "on" OR EXAMPLES STREQUAL "on")
    message(STATUS "Need executable tests for build and support")

    set(build_tests BUILD_TESTS CACHE STRING "build test executable")
    set(tests TESTS CACHE STRING "testing exectuable")

    add_subdirectory(tests/tests_build)
    add_subdirectory(tests/tests_support)
endif()

if(TESTING STREQUAL "on")
    message(STATUS "Testing requested")

    add_subdirectory(tests/tests_impl)

    add_test(NAME tests COMMAND $<TARGET_FILE:${tests}>)
endif()

if(EXAMPLES STREQUAL "on")
    message(STATUS "Examples requested")

    add_subdirectory(examples)
endif()

