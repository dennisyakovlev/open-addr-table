#include <utility>

#include <gtest/gtest.h>

#include <files/Files.h>
#include <tests_support/Funcs.h>
#include <tests_support/SpecialHash.h>
#include <tests_support/StrictOperation.h>
#include <tests_support/Vars.h>

using File = MmapFiles::unordered_map_file<SpecialHash,std::size_t>;

class LinearProbeTest : 
    public testing::Test,
    public StrictOperation<File>
{
protected:

    LinearProbeTest()
        : StrictOperation(File(unit_test_file))
    {
    }

};

/*  Below tests have comments of the format of two
    cols of numbers.

    First col is the indicies in the files container.

    Second col is where, if working correctly, the
    hash values in the container should be loacted.

*/

TEST_F(LinearProbeTest, InertFullNewHash)
{
    /*  0 6
        1 6
        2 6
        3 6
        4 2
        5 
        6 6

        0 6
        1 6
        2 6
        3 1
        4 2
        5 3
        6 6
    */

    cont.bucket_choices({7});
    cont.reserve(7);

    insert({6,6,6,6,6,2});

    erase_and_check(0);    
    insert({1,3}, 6);
    
    erase_and_check(4);
    erase_and_check(6);
    erase_and_check(1);
    erase_and_check(2);
    erase_and_check(3);
    erase_and_check(7);
}

TEST_F(LinearProbeTest, InersetOneSize)
{
    /*  0 0
    */

    cont.bucket_choices({1});
    cont.reserve(1);

    insert({0});

    ASSERT_EQ(cont.erase(gen_unique(0)), 0);

    erase_and_check(0);
}

TEST_F(LinearProbeTest, InsertZeroSize)
{
    /*  0 0
    */

    insert({0});

    ASSERT_EQ(cont.erase(gen_unique(0)), 0);

    erase_and_check(0);
}

TEST_F(LinearProbeTest, FullySame)
{
    /*  0 3
        1 3
        2 3
        3 3
        4 3
        5 3
    */

    cont.bucket_choices({6});
    cont.reserve(6);
    insert({3,3,3,3,3,3});

    ASSERT_EQ(cont.find(gen_unique(3)), cont.end());
    ASSERT_EQ(cont.find(gen_unique(4)), cont.end());

    erase_and_check(5);
    erase_and_check(2);
    erase_and_check(0);
    erase_and_check(4);
    erase_and_check(1);
    erase_and_check(3);
}

TEST_F(LinearProbeTest, KeyAlreadyExists)
{
    insert({7,3,9,14,2});

    ASSERT_FALSE(cont.insert({ keys()[2],10 }).second);
    auto iter = cont.find(keys()[2]);
    ASSERT_NE(iter, cont.cend());
    ASSERT_NE(10, iter->second);
}
