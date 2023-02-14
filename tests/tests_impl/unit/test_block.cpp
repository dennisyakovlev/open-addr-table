#include <utility>

#include <gtest/gtest.h>

#include <files/Files.h>

using namespace MmapFiles;

/*  In most implementations (including this one), the only
    operators which contain implementation are == and
    either of < or >. The other operators are derived from
    those two. Will only test two assuming the others are
    trivial and correct.
*/

TEST(BlockTest, Equal)
{
    block<int,int,int,int> b1 = { 1,2,3,4 }, b2 = { 1,2,3,4 };
    ASSERT_TRUE(b1 == b2);
    ASSERT_TRUE(b2 == b1);
    ASSERT_TRUE(b1 == b1);

    block<int,int,int,int> b3 = { 1,2,3,4 }, b4 = { 1,2,2,4 };
    ASSERT_FALSE(b3 == b4);
    ASSERT_FALSE(b4 == b3);

    block<int> b5 = { -1 }, b6 = { -1 };
    ASSERT_TRUE(b5 == b6);
    ASSERT_TRUE(b6 == b5);

    block<int> b7 = { -1 }, b8 = { 0 };
    ASSERT_FALSE(b7 == b8);
    ASSERT_FALSE(b8 == b7);

    block<int,int,int> b9 = { 4,3,2 };
    block<int,int> b10    = { 4,3 };
    ASSERT_FALSE(b9 == b10);
    ASSERT_FALSE(b10 == b9);
}

TEST(Block, LessThan)
{
    block<int,int,int,int> b1 = { 1,2,3,4 }, b2 = { 1,2,3,4 };
    ASSERT_FALSE(b1 < b2);
    ASSERT_FALSE(b2 < b1);
    ASSERT_FALSE(b1 < b1);

    block<int,int,int,int> b3 = { 1,2,3,4 }, b4 = { 1,2,2,4 };
    ASSERT_TRUE(b4 < b3);
    ASSERT_FALSE(b3 < b4);

    block<int> b5 = { -1 }, b6 = { 0 };
    ASSERT_TRUE(b5 < b6);
    ASSERT_FALSE(b6 < b5);

    block<int,int,int> b7 = { 2,3,4 }; 
    block<int,int> b8     = { 2,3 };
    ASSERT_TRUE(b8 < b7);
    ASSERT_FALSE(b7 < b8);

    block<int,int,int> b9 = { 3,2,1 };
    block<int> b10        = { 1 };
    ASSERT_TRUE(b10 < b9);
    ASSERT_FALSE(b9 < b10);

}
