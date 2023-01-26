cmake_minimum_required(VERSION 3.10.0)

project("mmap lib")

enable_testing()

# NOTE: have a subfolder in tests dir which compiles the
#       stuff which may not be portable, like futex and
#       checks to see that it works (builds)

set(include ${CMAKE_CURRENT_SOURCE_DIR}/code/include CACHE FILEPATH "include dir")
set(unit_tests_exe UNIT_TESTS CACHE STRING "unit testing exectuable")
set(lock_tests_exe LOCK_TESTS CACHE STRING "lock testing exectuable")
set(thourough_tests_exe THOUROUGH_TESTS CACHE STRING "unordered map requirements thourough testing exectuable")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3 -O0 -std=c++2a -Wall")

add_subdirectory(tests)

# add_test(NAME unit_tests COMMAND ${unit_tests_exe})
# add_test(NAME lock_tests COMMAND ${lock_tests_exe})
# add_test(NAME thourough_tests_exe COMMAND ${lock_tests_exe})
