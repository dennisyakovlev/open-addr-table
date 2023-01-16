cmake_minimum_required(VERSION 3.10.0)

enable_testing()

set(include ${CMAKE_CURRENT_SOURCE_DIR}/code/include CACHE FILEPATH "Include dir")
set(unit_tests_exe UNIT_TESTS CACHE STRING "unit testing exectuable")
set(lock_tests_exe LOCK_TESTS CACHE STRING "lock testing exectuable")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3 -O0 -std=c++11 -Wall")

add_subdirectory(tests)

add_test(NAME unit_tests COMMAND ${unit_tests_exe})
add_test(NAME lock_tests COMMAND ${lock_tests_exe})
