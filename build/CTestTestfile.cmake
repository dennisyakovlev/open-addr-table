# CMake generated Testfile for 
# Source directory: /home/dennisyakovlev/plan2
# Build directory: /home/dennisyakovlev/plan2/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(unit_tests "/home/dennisyakovlev/plan2/build/tests/unit/UNIT_TESTS")
set_tests_properties(unit_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/dennisyakovlev/plan2/CMakeLists.txt;13;add_test;/home/dennisyakovlev/plan2/CMakeLists.txt;0;")
add_test(lock_tests "/home/dennisyakovlev/plan2/build/tests/locks/LOCK_TESTS")
set_tests_properties(lock_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/dennisyakovlev/plan2/CMakeLists.txt;14;add_test;/home/dennisyakovlev/plan2/CMakeLists.txt;0;")
subdirs("tests")
