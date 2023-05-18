#ifndef UNIT_TESTS_VARS 
#define UNIT_TESTS_VARS

#include <climits>
#include <functional>
#include <type_traits>

#include <files/Allocators.h>
#include <files/Files.h>
#include <tests_support/config.h>

constexpr const char* unit_test_dir   = "UNIT_TEST_TEMP_FILES";
constexpr const char* unit_test_file  = "unit_test_file";
constexpr std::size_t test_iterations = 100000;
constexpr std::size_t test_cpu_cores  = TESTS_NUM_THREADS;

/**
 * @brief Used to determine whether to use "fast" or "slow" file
 *        for testing. The difference is in the allocator. The
 *        actual memory map allocator is creates, deletes, modifies
 *        a true file on disk. Whereas the basic allocator is just
 *        the normal allocator with some extra functionality. So
 *        the basic allocator is significantly quicker.
 *
 *        Using the "fast" tests means the basic allocator will be
 *        used. Which means a thourough test is not done.
 *
 *        There *is* a difference between the two. For example, I
 *        found a resource leak with unclosed file descriptors when
 *        tests gave a segfault because I was using the mmap
 *        allocator, which would not have been found otherwise.
 *
 * @tparam Fast true for fast, false for slow
 * @tparam Ts types to give to file
 */
template<bool Fast, typename... Ts>
struct test_file_type
{
};

template<typename Key, typename Value>
struct test_file_type<false, Key, Value>
{
    using file = MmapFiles::unordered_map_file<
        Key,
        Value>;
};

template<typename Key, typename Value>
struct test_file_type<true, Key, Value>
{
    using file = MmapFiles::unordered_map_file<
        Key,
        Value,
        std::hash<Key>,
        MmapFiles::basic_allocator>;
};

template<typename Key, typename Value, typename Hash>
struct test_file_type<false, Key, Value, Hash>
{
    using file = MmapFiles::unordered_map_file<
        Key,
        Value,
        Hash>;
};

template<typename Key, typename Value, typename Hash>
struct test_file_type<true, Key, Value, Hash>
{
    using file = MmapFiles::unordered_map_file<
        Key,
        Value,
        Hash,
        MmapFiles::basic_allocator>;
};

#endif
