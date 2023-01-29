#ifndef CUSTOM_TESTS_FUNCS
#define CUSTOM_TESTS_FUNCS

#include <algorithm>
#include <initializer_list>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

/**
 * @brief Remove file or directory and all contents.
 * 
 * @param path path to remove
 * @return int 0 for success, non-zero on failure
 */
int
RemoveRecursive(const char* path);


template<
    typename Cont,
    typename ContIn = std::initializer_list<std::size_t>>
std::vector<Cont>
all_permutations(ContIn lis)
{
    std::vector<Cont> res;
    Cont curr = lis;

    do
    {
        res.push_back(curr);
    } while (std::next_permutation(curr.begin(), curr.end()));
    
    return res;
}

/**
 * @brief Generate a type which gtest can use which is the
 *        set of all permuations of possible removal order
 *        from lis. lis will be inserted in the order it is
 *        given.
 * 
 * @param buckets the number of buckets the file container
 *                should reserve
 * @param lis unique hashes to insert
 * @return auto some form of gtest object
 */
template<
    typename Cont,
    typename ContIn = std::initializer_list<std::size_t>>
auto
permutated_insertions(std::size_t buckets, ContIn lis)
    -> decltype(
        testing::Combine(
            testing::ValuesIn(std::vector<Cont>{lis}),
            testing::ValuesIn(all_permutations<Cont>(Cont(lis.size()))),
            testing::ValuesIn(std::vector<Cont>{{buckets}})
       ))
{
    Cont range(lis.size());
    std::iota(range.begin(), range.end(), 0);

    return testing::Combine(
        testing::ValuesIn(std::vector<Cont>{lis}),
        testing::ValuesIn(all_permutations<Cont>(range)),
        testing::ValuesIn(std::vector<Cont>{{buckets}})
    );
}


#endif
