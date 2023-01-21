#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>

#include <unordered_map>

#include <gtest/gtest.h>

#include <files/Files.h>
#include <unit_tests/Funcs.h>
#include <unit_tests/SpecialHash.h>
#include <unit_tests/Vars.h>

class LinearProbeTest : public testing::Test
{
protected:

    using file      = MmapFiles::unordered_map_file<SpecialHash,std::size_t>;
    // using file      = std::unordered_map<SpecialHash,std::size_t>;
    using hashes    = std::vector<SpecialHash>;
    using size_type = typename hashes::size_type;

    file              cont;
    hashes            vec;
    std::vector<bool> rel;

    LinearProbeTest()
        // : cont(),
        : cont(unit_test_file)
    {
        reset_gen();
    }

    ~LinearProbeTest()
    {
        RemoveRecursive(unit_test_file);
    }

    /**
     * @brief Erase from file container an element which
     *        exists and is equal to the index of element
     *        inserted with "inserted". Do some checks.
     * 
     * @param index index from hashes container of key
     *              to remove which is relative to the
     *              original size of keys
     */
    void
    erase_and_check(size_type index)
    {
        /*  Use "rel" which keeps track of which indicies
            were removed. Allows unit tests to use indicies
            relative the original size of "vec".
        */

        if (!rel[index])
        {
            throw std::invalid_argument(std::to_string(index) + " already removed");
        }
        rel[index] = false;

        ASSERT_EQ(cont.erase(vec[index]), 1);
        ASSERT_FALSE(cont.contains(vec[index]));

        for (size_type i = 0; i != vec.size(); ++i)
        {
            if (rel[i])
            {
                ASSERT_TRUE(cont.contains(vec[i]));
            }
        }
    }

    /**
     * @brief Insert set of hashes, each of which will be 
     *        transformed into a unique key.
     * 
     * @param lis set of hashes
     * @param pos position of keys array to insert into
     */
    void
    insert(std::initializer_list<std::size_t> lis, size_type pos = 0)
    {
        std::vector<bool> temp(lis.size(), true); 
        rel.insert(rel.begin() + pos, temp.cbegin(), temp.cend());

        size_type j = 0;
        for (std::size_t hash : lis)
        {
            auto spec = gen_unique(hash);
            ASSERT_TRUE(cont.emplace(spec, 0).second);
            vec.insert(vec.begin() + pos + j, spec);
            ++j;
        }
    }
};

TEST_F(LinearProbeTest, A)
{
    /*  0 4
        1 5
        2 5
        3
        4 4
        5 4
    */

    cont.reserve(6);
    insert({4,4,4,5,5});

    ASSERT_EQ(5, cont.size());

    erase_and_check(2);
    erase_and_check(3);
    erase_and_check(1);
    erase_and_check(4);
    erase_and_check(0);
}

// TEST_F(LinearProbeTest, B_1)
// {
//     /*  0 4
//         1 4
//         2 1
//         3 1
//         4 4
//     */

//     cont.reserve(5);
//     insert({4,4,4,1,1});

//     ASSERT_EQ(5, cont.size());

//     erase_and_check(0);
//     erase_and_check(1);
//     erase_and_check(2);
//     erase_and_check(3);
//     erase_and_check(4);
// }

// TEST_F(LinearProbeTest, B_2)
// {
//     /*  0 4
//         1 4
//         2 1
//         3 1
//         4 4
//     */

//     cont.reserve(5);
//     insert({1,1,4,4,4});

//     ASSERT_EQ(5, cont.size());

//     erase_and_check(0);
//     erase_and_check(1);
//     erase_and_check(2);
//     erase_and_check(3);
//     erase_and_check(4);
// }

// TEST_F(LinearProbeTest, C)
// {
//     /*  0
//         1 1
//         2 1
//         3 1
//         4 3
//         5 3
//         6 6
//         7
//         8
//     */

//     cont.reserve(9);
//     insert({1,1,1,3,3,6});

//     ASSERT_EQ(6, cont.size());

//     ASSERT_EQ(cont.erase(gen_unique(2)), 0);
//     ASSERT_EQ(cont.erase(gen_unique(5)), 0);
//     ASSERT_EQ(cont.erase(gen_unique(4)), 0);

//     erase_and_check(3);
//     erase_and_check(5);
// }

// TEST_F(LinearProbeTest, D)
// {
//     /*  0
//         1
//         2 2
//         3 3
//         4 4
//         5 5
//         6 6
//     */

//     cont.reserve(7);
//     insert({2,3,4,5,6});

//     ASSERT_EQ(5, cont.size());

//     erase_and_check(1);
//     erase_and_check(0);
//     erase_and_check(4);
//     erase_and_check(3);
//     erase_and_check(2);
// }

// TEST_F(LinearProbeTest, E)
// {
//     /*  0 1
//         1 1
//         2 1
//         3 1
//     */

//     cont.reserve(4);
//     insert({1,1,1,1});

//     ASSERT_EQ(4, cont.size());

//     ASSERT_EQ(cont.erase(gen_unique(2)), 0);
//     ASSERT_EQ(cont.erase(gen_unique(0)), 0);
//     ASSERT_EQ(cont.erase(gen_unique(1)), 0);
// }

// TEST_F(LinearProbeTest, F)
// {
//     /*  0 0 
//         1 0
//         2 0
//         4 0
//         5 0
//     */

//     cont.reserve(6);
//     insert({0,0,0,0,0,0});

//     ASSERT_EQ(6, cont.size());

//     ASSERT_EQ(cont.erase(gen_unique(0)), 0);
//     ASSERT_EQ(cont.erase(gen_unique(5)), 0);
//     ASSERT_EQ(cont.erase(gen_unique(2)), 0);
// }

// TEST_F(LinearProbeTest, G)
// {
//     /*  0 6
//         1 6
//         2 6
//         3 6
//         4 2
//         5 
//         6 6

//         0 6
//         1 6
//         2 6
//         3 1
//         4 2
//         5 3
//         6 6
//     */

//     cont.reserve(7);

//     insert({6,6,6,6,6,2});

//     erase_and_check(0);    
//     insert({1,3}, 6);
    
//     erase_and_check(4);
//     erase_and_check(6);
//     erase_and_check(1);
//     erase_and_check(2);
//     erase_and_check(3);
//     erase_and_check(7);
// }

TEST_F(LinearProbeTest, H)
{
    /*  0
        1
        2 2
        3 2
        4 2
    */

    cont.reserve(5);
    insert({2,2,2});

    ASSERT_EQ(cont.find(gen_unique(2)), cont.end());

    erase_and_check(2);
}
