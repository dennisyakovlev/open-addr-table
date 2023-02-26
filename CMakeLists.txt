cmake_minimum_required(VERSION 3.10.0)

project("mmap lib")

include(CTest)
enable_testing()
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(include ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE FILEPATH "include dir")

if (NOT DEFINED CPU_CORES)
    execute_process(
        COMMAND "sh" "-c" "cat /proc/cpuinfo | grep -i cores | head -n 1 | egrep -o [0-9]+ | tr -d '\n'"
        OUTPUT_VARIABLE CPU_CORES
    )
    if (${CPU_CORES} MATCHES "^[0-9]+$")
        message(STATUS "Found ${CPU_CORES} physical CPU cores")
    else()
        message(FATAL_ERROR "Could not get number of cpu cores")
    endif()
endif()

if(TESTING EQUAL 1)
    message(STATUS "Testing requested")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

    # TESTS_NUM_THREADS and FAST_TESTS must be defined to valid values
    # by build system
    #   TESTS_NUM_THREADS - whole positive number
    #   FAST_TESTS - true for fast tests, false for no (see "Vars.h")
    add_compile_definitions(TESTS_NUM_THREADS=${CPU_CORES})
    if (DEFINED FAST_TESTS)
        add_compile_definitions(FAST_TESTS=${FAST_TESTS})
    else()
        add_compile_definitions(FAST_TESTS=true)
        message(STATUS "Fast tests on by default")
    endif()

    set(build_tests BUILD_TESTS CACHE STRING "build test executable")
    set(tests TESTS CACHE STRING "testing exectuable")
    
    add_subdirectory(tests/tests_build)
    add_subdirectory(tests/tests_support)
    add_subdirectory(tests/tests_impl)

    add_test(NAME tests COMMAND $<TARGET_FILE:${tests}>)
endif()
