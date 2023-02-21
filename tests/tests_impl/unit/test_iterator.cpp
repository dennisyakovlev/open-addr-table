#include <cstddef>

#include <gtest/gtest.h>

#include <files/Files.h>
#include <tests_support/Vars.h>

using File = typename test_file_type<FAST_TESTS, std::size_t, std::size_t>::file;

class MapIteratorTest :
    public testing::Test
{
protected:

    File file;

};

TEST_F(MapIteratorTest, Begin)
{
    /*  insert value which is not at beginning of array.
        the begin iter should have this value, not the actual
        beginning
    */

    file.bucket_choices({6});
    file.rehash(6);

    file.emplace(5, 0);

    auto beg = file.cbegin();
    ASSERT_EQ(5, beg->first);
}

TEST_F(MapIteratorTest, ToEnd)
{
    /*  incrementing should bring to end even if there are
        empty spaces in array
    */
    
    file.bucket_choices({9});
    file.rehash(9);

    file.emplace(4, 0);

    auto beg = file.begin();
    ++beg;
    ASSERT_EQ(beg, file.cend());
}

TEST_F(MapIteratorTest, ToBegin)
{
    file.bucket_choices({9});
    file.rehash(9);

    file.emplace(4, 0);

    auto end = file.end();
    --end;
    ASSERT_EQ(end, file.cbegin());
}

TEST_F(MapIteratorTest, One)
{
    file.bucket_choices({1});
    file.rehash(1);

    file.emplace(5, 0);

    auto beg = file.begin();
    ++beg;
    ASSERT_EQ(beg, file.cend());
}

TEST_F(MapIteratorTest, Empty)
{
    ASSERT_EQ(file.begin(), file.end());
}
