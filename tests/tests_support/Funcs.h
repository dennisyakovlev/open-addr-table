#ifndef CUSTOM_TESTS_FUNCS
#define CUSTOM_TESTS_FUNCS

#include <algorithm>
#include <initializer_list>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

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
 * @example to illustrate use
 *              buckets=10
 *              lis=[6,7,8]
 *          would get a result of the form
 *              {
 *                  [6,7,8],
 *                  {
 *                     [0,1,2], 
 *                     [0,2,1], 
 *                     [1,0,2], 
 *                     [1,2,0], 
 *                     [2,1,0], 
 *                     [2,0,1]
 *                  },
 *                  [10]
 *              }
 * 
 * @tparam Cont final container type to use. semantics and
 *              usage should be similar to an array. Cont
 *              must be initializable by ContIn
 * @tparam ContIn parameter container type. semantics and
 *                usage should be similar to an array
 * @param buckets numbers of buckets. this has no effect on
 *                the permutations
 * @param lis data to permutate. every element must be
 *            unique
 * @return 
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
        testing::ValuesIn(std::vector<Cont>({lis})),
        testing::ValuesIn(all_permutations<Cont>(range)),
        testing::ValuesIn(std::vector<Cont>{{buckets}})
    );
}

#endif
