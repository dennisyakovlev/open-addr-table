/**
 * @file test_unordered_map_req.cpp
 * @brief Test general requirements for unordered
 *        associative containers.
 * 
 *        As in, do not rigorously examine a type to meet
 *        ISO standard. Instead, check that a type is sane    
 *        in the way it meets requirements with the
 *        assumption the implementation details are
 *        correct.
 * 
 *        Look at essential functions, member declerations,
 *        and functionality.
 */

#include <algorithm>
#include <cassert>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <files/unordered_map_lru.h>
#include <files/Files.h>
#include <tests_support/Funcs.h>
#include <tests_support/CustomString.h>
#include <tests_support/Vars.h>

template<typename Cont>
class UnorderedMapReqTest :
    public testing::Test
{
public:

    // Should exist
    using key_type        = typename Cont::key_type;
    using mapped_type     = typename Cont::mapped_type;
    using value_type      = typename Cont::value_type;
    using size_type       = typename Cont::size_type;
    using difference_type = typename Cont::difference_type;
    using reference       = typename Cont::reference;
    using const_reference = typename Cont::const_reference;
    using pointer         = typename Cont::pointer;
    using const_pointer   = typename Cont::const_pointer;
    using iterator        = typename Cont::iterator;
    using const_iterator  = typename Cont::const_iterator;

    Cont cont;

    UnorderedMapReqTest()
        : cont(8)
    {
    }

};

/* Use <Key, Value> where
    - Key is some string type which can be construced
      from cont char*
    - Value is some signed integral type
*/
using MyTypes = testing::Types
<
    MmapFiles::unordered_map_lru<std::string, int>,
    MmapFiles::unordered_map_lru<MyString<88>, long>,
    MmapFiles::unordered_map_lru<MyString<257>, long, collison<257, 28>>,
    typename test_file_type<FAST_TESTS, MyString<128>, int>::file,
    typename test_file_type<FAST_TESTS, MyString<156>, long>::file,
    typename test_file_type<FAST_TESTS, MyString<67>, short, collison<67, 0>>::file,
    typename test_file_type<FAST_TESTS, MyString<953>, long, collison<953, std::numeric_limits<std::size_t>::max()>>::file,
    typename test_file_type<FAST_TESTS, std::string, long>::file
>;
TYPED_TEST_SUITE(UnorderedMapReqTest, MyTypes);

TYPED_TEST(UnorderedMapReqTest, Insert)
{
    auto pair_res = this->cont.insert({ "  ;",5 });
    ASSERT_TRUE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "  ;");
    ASSERT_EQ(pair_res.first->second, 5);

    pair_res = this->cont.insert(std::make_pair("  一个字节流   ", 5));
    ASSERT_TRUE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "  一个字节流   ");
    ASSERT_EQ(pair_res.first->second, 5);

    const auto pair = std::make_pair("90  8239p=.", 14);
    pair_res = this->cont.insert(pair);
    ASSERT_TRUE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "90  8239p=.");
    ASSERT_EQ(pair_res.first->second, 14);

    auto pair2 = pair;
    pair_res = this->cont.insert(std::move(pair2));
    ASSERT_FALSE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "90  8239p=.");
    ASSERT_EQ(pair_res.first->second, 14);

    pair_res = this->cont.insert(std::make_pair("  一个字节流   ", 50));
    ASSERT_FALSE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "  一个字节流   ");
    ASSERT_EQ(pair_res.first->second, 5);

    pair_res = this->cont.insert({ "  ;",0 });
    ASSERT_FALSE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "  ;");
    ASSERT_EQ(pair_res.first->second, 5);
}

TYPED_TEST(UnorderedMapReqTest, InsertOrAssign)
{
    auto pair_res = this->cont.insert_or_assign("  一个字节流   ", 5);
    ASSERT_TRUE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "  一个字节流   ");
    ASSERT_EQ(pair_res.first->second, 5);

    pair_res = this->cont.insert_or_assign("  一个字节流   ", 57);
    ASSERT_FALSE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "  一个字节流   ");
    ASSERT_EQ(pair_res.first->second, 57);

    pair_res = this->cont.insert_or_assign("90  8239p=.", 5);
    ASSERT_TRUE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "90  8239p=.");
    ASSERT_EQ(pair_res.first->second, 5);
}

TYPED_TEST(UnorderedMapReqTest, Emplace)
{
    auto pair_res = this->cont.emplace("", 19);
    ASSERT_TRUE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "");
    ASSERT_EQ(pair_res.first->second, 19);

    pair_res = this->cont.emplace("", 1);
    ASSERT_FALSE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, "");
    ASSERT_EQ(pair_res.first->second, 19);

    pair_res = this->cont.emplace(" ", 19);
    ASSERT_TRUE(pair_res.second);
    ASSERT_EQ(pair_res.first->first, " ");
    ASSERT_EQ(pair_res.first->second, 19);
}

TYPED_TEST(UnorderedMapReqTest, Empty)
{
    ASSERT_TRUE(this->cont.empty());

    this->cont.emplace("", 0);
    ASSERT_FALSE(this->cont.empty());
    this->cont.erase("");
    ASSERT_TRUE(this->cont.empty());
}

TYPED_TEST(UnorderedMapReqTest, Size)
{
    ASSERT_EQ(this->cont.size(), 0);

    this->cont.emplace("", 0);
    ASSERT_EQ(this->cont.size(), 1);
    this->cont.insert_or_assign("980auc", 0);
    this->cont.insert_or_assign("980auc", 5);
    this->cont.insert({"980auc",8});
    ASSERT_EQ(this->cont.size(), 2);
    this->cont.erase("");
    ASSERT_EQ(this->cont.size(), 1);
}

TYPED_TEST(UnorderedMapReqTest, Clear)
{
    this->cont.insert({"a",1});
    this->cont.insert({"b",1});
    this->cont.insert({"c",1});
    this->cont.insert({"d",1});
    this->cont.insert({"e",1});
    ASSERT_NE(this->cont.size(), 0);
    this->cont.clear();
    ASSERT_EQ(this->cont.size(), 0);
}

TYPED_TEST(UnorderedMapReqTest, Erase)
{
    ASSERT_EQ(this->cont.erase("a"), 0);
    this->cont.insert({"a",1});
    ASSERT_EQ(this->cont.erase("a"), 1);

    this->cont.insert({"b",1});
    this->cont.insert({"c",1});
    ASSERT_NE(this->cont.erase(this->cont.find("c")), this->cont.end());
    ASSERT_EQ(this->cont.erase(this->cont.find("b")), this->cont.end());
}

TYPED_TEST(UnorderedMapReqTest, Find)
{
    auto pair = std::make_pair("\"  一个字节流   \"", 0);
    ASSERT_EQ(this->cont.find(pair.first), this->cont.cend());
    this->cont.insert(pair);
    auto iter = this->cont.find(pair.first);
    ASSERT_EQ(iter->first, pair.first);
    ASSERT_EQ(iter->second, pair.second);
}

TYPED_TEST(UnorderedMapReqTest, Contains)
{
    auto pair = std::make_pair(" ", 1);
    ASSERT_FALSE(this->cont.contains(pair.first));
    this->cont.insert(pair);
    ASSERT_TRUE(this->cont.contains(pair.first));
}

TYPED_TEST(UnorderedMapReqTest, Range)
{
    // can't access key_type in the defined class
    // because of how gtest works
    using key_type = typename std::remove_const<decltype(this->cont.begin()->first)>::type;

    std::vector<key_type> v = { "a","b","c","d","e","f","g","h" };
    for (const auto& s : v)
    {
        this->cont.emplace(s, 0);
    }

    for (const auto& p : this->cont)
    {
        auto iter = std::find(v.begin(), v.end(), p.first);
        ASSERT_NE(iter, v.end()); 
        v.erase(iter);
    }
    
    ASSERT_TRUE(v.empty());
}

TYPED_TEST(UnorderedMapReqTest, BucketCount)
{
    ASSERT_GE(this->cont.bucket_count(), 1);
}

TYPED_TEST(UnorderedMapReqTest, BucketSize)
{
    ASSERT_EQ(0, this->cont.bucket_size(7));
    ASSERT_EQ(0, this->cont.bucket_size(2));    
}

TYPED_TEST(UnorderedMapReqTest, Bucket)
{
    using key_type = typename std::remove_const<decltype(this->cont.begin()->first)>::type;

    std::vector<key_type> v = {
        "key 1",
        "key two",
        "3",
        "four",
        "key-five",
        "6_key",
        "S E V E N",
        "_8_"
    };

    for (const auto& k : v)
    {
        this->cont.insert({k, 97});

        auto bucket = this->cont.bucket(k);
        ASSERT_LT(bucket, this->cont.max_bucket_count());
    }
}
