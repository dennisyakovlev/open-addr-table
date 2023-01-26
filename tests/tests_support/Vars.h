#ifndef UNIT_TESTS_VARS 
#define UNIT_TESTS_VARS

#include <climits>
#include <unistd.h>

#define TESTS_NUM_THREADS sysconf(_SC_NPROCESSORS_ONLN)

constexpr std::size_t TESTS_NUM_ITERATS = 100000;

constexpr const char* unit_test_dir = "UNIT_TEST_TEMP_FILES";
constexpr const char* unit_test_file = "unit_test_file";

#endif
