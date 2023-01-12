#ifndef MY_TESTING_DEFINITIONS
#define MY_TESTING_DEFINITIONS

#include <cxxabi.h>
#include <iostream>
#include <unistd.h>

#define TESTS_NUM_THREADS sysconf(_SC_NPROCESSORS_ONLN)

constexpr std::size_t TESTS_NUM_ITERATS = 100000;

#define DEMANGLE_TYPEID_NAME(x) (abi::__cxa_demangle(typeid(x).name(), NULL, NULL, NULL))

#define PRINT_RUN_TEST(name) \
    std::cout \
        << "\033[1;32m[ RUN      ]\033[0m " \
        << name \
        << "\n"

#define PRINT_FAIL_OR_SUCCESS(b, test_name) \
    std::cout \
        << (b ? "\033[1;32m[       OK ]\033[0m " : "\033[1;31m[  FAILED  ]\033[0m ") \
        << (b ? test_name : "") \
        << "\n"

#define PRINT_BEGIN_TESTS(name) \
    std::cout << "\033[1;32m[----------]\033[0m " << name << "\n"

#define PRINT_END_TESTS(name) \
    std::cout << "\033[1;32m[----------]\033[0m " << name << " Done\n"

#endif
