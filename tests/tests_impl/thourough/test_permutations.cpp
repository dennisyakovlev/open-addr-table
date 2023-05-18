#include <cstddef>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include <files/unordered_map.h>
#include <tests_support/Funcs.h>
#include <tests_support/SpecialHash.h>
#include <tests_support/StrictOperation.h>
#include <tests_support/Vars.h>

using HashHolder = std::vector<std::size_t>;
using File = typename test_file_type<FAST_TESTS, SpecialHash, std::size_t>::file;

class PermutationTest : 
    public testing::TestWithParam<
        std::tuple<
            HashHolder,
            HashHolder,
            HashHolder
        >
    >,
    public StrictOperation<File>
{
protected:

    PermutationTest()
        : StrictOperation(File(unit_test_file))
    {
    }

    /**
     * @brief Parameter access.
     * 
     * @return HashHolder elements with hash values to be inserted into
     *                    file.
     */
    HashHolder
    elements()
    {
        return std::get<0>(GetParam());
    }

    /**
     * @brief Parameter access.
     * 
     * @return HashHolder container of indicies used to index given
     *                    elements
     */
    HashHolder
    erase_indicies()
    {
        return std::get<1>(GetParam());
    }

    /**
     * @brief Parameter access.
     * 
     * @return std::size_t number of buckets to reserve in the contatiner
     */
    std::size_t
    size()
    {
        return std::get<2>(GetParam()).front();
    }

};

TEST_P(PermutationTest, InsertThenErase)
{
    cont.reserve(size());
    insert(elements());

    for (auto index : erase_indicies())
    {
        erase_and_check(index);
    }
}

/*  Below tests have comments of format of two
    cols of numbers.
    
    First col is the indicies in the file container. The
    number of these corresponds to the first param in
    permutated_insertions.

    Second col is where, if working correctly, the
    hash values in the conatiner should be located.
    This col corresponds to second param in
    permutated_insertions.

    Each test will permutate the order of which elements
    are *erased*.
*/

/*  0 4
    1 5
    2 5
    3
    4 4
    5 4
*/
INSTANTIATE_TEST_SUITE_P
(
    A,
    PermutationTest,
    permutated_insertions<HashHolder>(6, {4,4,4,5,5})
);

/*  0 4
    1 4
    2 1
    3 1
    4 4
*/
INSTANTIATE_TEST_SUITE_P
(
    B,
    PermutationTest,
    permutated_insertions<HashHolder>(5, {4,4,4,1,1})
);

/*  0
    1 1
    2 1
    3 1
    4 3
    5 3
    6 6
    7
    8
*/
INSTANTIATE_TEST_SUITE_P
(
    C,
    PermutationTest,
    permutated_insertions<HashHolder>(9, {1,1,1,3,3,6})
);

/*  0
    1 1
    2 1
    3 1
    4 2
    5 3
    6 6
    7
    8
*/
INSTANTIATE_TEST_SUITE_P
(
    D,
    PermutationTest,
    permutated_insertions<HashHolder>(9, {1,1,1,3,2,6})
);

/*  0
    1
    2 2
    3 3
    4 4
    5 5
    6 6
*/
INSTANTIATE_TEST_SUITE_P
(
    E,
    PermutationTest,
    permutated_insertions<HashHolder>(7, {2,3,4,5,6})
);

/*  0 0 
    1 0
    2 0
*/
INSTANTIATE_TEST_SUITE_P
(
    F,
    PermutationTest,
    permutated_insertions<HashHolder>(4, {0,0,0,0})
);

/*  0
    1
    2 2
    3 2
    4 2
*/
INSTANTIATE_TEST_SUITE_P
(
    G,
    PermutationTest,
    permutated_insertions<HashHolder>(5, {2,2,2})
);

/*  0 6
    1 7
    2 1
    3 2
    4
    5
    6 6
    7 6
*/
INSTANTIATE_TEST_SUITE_P
(
    H,
    PermutationTest,
    permutated_insertions<HashHolder>(8, {6,6,7,6,2,1})
);
