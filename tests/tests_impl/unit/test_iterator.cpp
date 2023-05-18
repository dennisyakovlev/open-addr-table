#include <cstddef>

#include <gtest/gtest.h>

#include <files/unordered_map.h>
#include <tests_support/Vars.h>

using File = typename test_file_type<FAST_TESTS, std::size_t, std::size_t>::file;

class MapIteratorTest :
    public testing::Test
{
protected:

    File cont;

    MapIteratorTest()
    {
        MmapFiles::destruct_is_wipe(cont, true);
    }

};

TEST_F(MapIteratorTest, Begin)
{
    /*  insert value which is not at beginning of array.
        the begin iter should have this value, not the actual
        beginning
    */

    cont.bucket_choices({6});
    cont.rehash(6);

    cont.emplace(5, 0);

    auto beg = cont.cbegin();
    ASSERT_EQ(5, beg->first);
}

TEST_F(MapIteratorTest, ToEnd)
{
    /*  incrementing should bring to end even if there are
        empty spaces in array
    */
    
    cont.bucket_choices({9});
    cont.rehash(9);

    cont.emplace(4, 0);

    auto beg = cont.begin();
    ++beg;
    ASSERT_EQ(beg, cont.cend());
}

TEST_F(MapIteratorTest, ToBegin)
{
    cont.bucket_choices({9});
    cont.rehash(9);

    cont.emplace(4, 0);

    auto end = cont.end();
    --end;
    ASSERT_EQ(end, cont.cbegin());
}

TEST_F(MapIteratorTest, One)
{
    cont.bucket_choices({1});
    cont.rehash(1);

    cont.emplace(5, 0);

    auto beg = cont.begin();
    ++beg;
    ASSERT_EQ(beg, cont.cend());
}

TEST_F(MapIteratorTest, Empty)
{
    ASSERT_EQ(cont.begin(), cont.end());
}
