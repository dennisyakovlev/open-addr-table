#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <files/unordered_map_lru.h>
#include <unit_tests/CustomString.h>

using MmapFiles::unordered_map_lru;
using std::string;

template<typename Key>
class UnorderedMapLruTest : public ::testing::Test
{
protected:

    unordered_map_lru<Key, bool> map;

    UnorderedMapLruTest()
        : map(4)
    {
    }

};

/**
 * @brief Types to test. Be careful with the sizes given
 *        to custom strings. Their sizes must be at minimum
 *        the size of the strings used in the test.
 * 
 */
using MyTypes = testing::Types
<
    string,
    MyString<128>,
    CollisonString<64>
>;
TYPED_TEST_CASE(UnorderedMapLruTest, MyTypes);

TYPED_TEST(UnorderedMapLruTest, InsertPopsLeastRecent)
{
    ASSERT_TRUE(this->map.insert({"a", false}).second);
    ASSERT_TRUE(this->map.insert({"b", false}).second);
    ASSERT_TRUE(this->map.insert({"c", false}).second);
    ASSERT_TRUE(this->map.insert({"d", false}).second);

    // every insertion after this should remove
    // least recently used key

    // removes a
    ASSERT_TRUE(this->map.insert({"e", false}).second);
    ASSERT_FALSE(this->map.contains("a"));
    ASSERT_TRUE(this->map.contains("b"));
    ASSERT_TRUE(this->map.contains("c"));
    ASSERT_TRUE(this->map.contains("d"));

    // removes b
    ASSERT_TRUE(this->map.insert({"f", false}).second);
    ASSERT_FALSE(this->map.contains("a"));
    ASSERT_FALSE(this->map.contains("b"));
    ASSERT_TRUE(this->map.contains("c"));
    ASSERT_TRUE(this->map.contains("d"));

    // removes c
    ASSERT_TRUE(this->map.insert({"g", false}).second);
    ASSERT_FALSE(this->map.contains("a"));
    ASSERT_FALSE(this->map.contains("b"));
    ASSERT_FALSE(this->map.contains("c"));
    ASSERT_TRUE(this->map.contains("d"));

    // removes d
    ASSERT_TRUE(this->map.insert({"h", false}).second);
    ASSERT_FALSE(this->map.contains("a"));
    ASSERT_FALSE(this->map.contains("b"));
    ASSERT_FALSE(this->map.contains("c"));
    ASSERT_FALSE(this->map.contains("d"));
}

TYPED_TEST(UnorderedMapLruTest, Erase)
{
    this->map.emplace("a", false);
    auto iter = this->map.emplace("b", false).first;
    this->map.emplace("c", false);
    ASSERT_EQ(this->map.erase("a"), 1);
    ASSERT_EQ(this->map.erase(this->map.begin()), iter);
    ASSERT_EQ(this->map.erase(iter), this->map.end());
}

TYPED_TEST(UnorderedMapLruTest, LruOperations)
{
    // check for functionality of "least recent" part
    // of the container

    this->map.insert({"1", false});
    this->map.insert({"  ;", false});

    // resize to less than current size with
    // removed keys being empty keys (container
    // stays same)
    this->map.reserve(2);
    ASSERT_TRUE(this->map.contains("1"));
    ASSERT_TRUE(this->map.contains("  ;"));

    // insert_or_assign will change order,
    // forcing "1" to be used more recently
    this->map.insert_or_assign("1", true); // used more recently
    this->map.insert({"'90  8239p=.", false});
    ASSERT_TRUE(this->map.contains("1"));
    ASSERT_TRUE(this->map.contains("'90  8239p=."));
    ASSERT_FALSE(this->map.contains("  ;"));

    // resize to current size (container stays
    // same)
    this->map.reserve(2);
    ASSERT_TRUE(this->map.contains("1"));
    ASSERT_TRUE(this->map.contains("'90  8239p=."));

    // resize to greater than current size with
    // new insertions being empty (container
    // stays same)
    this->map.reserve(4);
    ASSERT_TRUE(this->map.contains("1"));
    ASSERT_TRUE(this->map.contains("'90  8239p=."));

    this->map.insert({"", false});
    this->map.insert({"  a一个字节流   ", false});

    // resize to less than current size with
    // removed keys being in use keys. the
    // least recently used key will be removed
    // until the size is met
    this->map.reserve(3);
    ASSERT_TRUE(this->map.contains("  a一个字节流   "));
    ASSERT_TRUE(this->map.contains(""));
    ASSERT_TRUE(this->map.contains("'90  8239p=."));
    ASSERT_FALSE(this->map.contains("1"));

    // LRU key is removed as expected
    this->map.insert_or_assign("", false);
    this->map.reserve(5);
    this->map.erase("");
    ASSERT_TRUE(this->map.contains("  a一个字节流   "));
    ASSERT_TRUE(this->map.contains("'90  8239p=."));
    ASSERT_FALSE(this->map.contains(""));

    this->map.insert({"a", false});
    this->map.insert({"b", false});
    this->map.insert({"c", false});
    ASSERT_TRUE(this->map.contains("  a一个字节流   "));
    ASSERT_TRUE(this->map.contains("'90  8239p=."));

    this->map.insert({"d", false});
    ASSERT_TRUE(this->map.contains("  a一个字节流   "));
    ASSERT_FALSE(this->map.contains("'90  8239p=."));

    this->map.insert({"e", false});
    ASSERT_FALSE(this->map.contains("  a一个字节流   "));
}
