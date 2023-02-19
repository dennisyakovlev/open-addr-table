cmake_minimum_required(VERSION 3.10.0)

project("mmap lib")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(include ${CMAKE_CURRENT_SOURCE_DIR}/code/include CACHE FILEPATH "include dir")

if(TESTING EQUAL 1)
    message(STATUS "Testing requested")

    enable_testing()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

    set(build_tests BUILD_TESTS CACHE STRING "build test executable")
    set(tests TESTS CACHE STRING "testing exectuable")
    
    add_subdirectory(tests/tests_build)
    add_subdirectory(tests/tests_support)
    add_subdirectory(tests/tests_impl)

    add_test(NAME tests COMMAND $<TARGET_FILE:${tests}>)
endif()
