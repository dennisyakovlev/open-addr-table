#include <climits>
#include <unistd.h>

#define TESTS_NUM_THREADS sysconf(_SC_NPROCESSORS_ONLN)

constexpr std::size_t TESTS_NUM_ITERATS = 100000;
