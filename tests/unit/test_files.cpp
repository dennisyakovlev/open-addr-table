#include <vector>

#include <gtest/gtest.h>

#include <files/Files.h>
#include <files/Utils.h>
#include <unit_tests/CustomString.h>
#include <unit_tests/Funcs.h>
#include <unit_tests/Vars.h>

template<typename File>
class FileTest : public ::testing::Test
{
protected:

    using Key = typename File::key_type;

    File*            file;
    std::size_t      n_default;
    std::vector<Key> keys =
    {
        ";9 qusjiCT*&",
        ";LAKSHXC98B",
        "",
        "90UasjmkmlklNjOIoi",
        " ",
        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
    };

    FileTest()
        : n_default(32)
    {
        RemoveCreateDir(unit_test_dir);
        file = new File(unit_test_dir, unit_test_file, n_default);
    }

};

using MyTypes = testing::Types
<
    MmapFiles::vector_file<MyString<256>>
>;
TYPED_TEST_CASE(FileTest, MyTypes);

TYPED_TEST(FileTest, Erase)
{
    
}

TYPED_TEST(FileTest, Exists)
{
    // none of keys exist
    for (const auto& key : this->keys)
    {
        ASSERT_FALSE(this->file->contains(key));
    }
}

TYPED_TEST(FileTest, Insert)
{
    for (const auto& key : this->keys)
    {
        ASSERT_TRUE(this->file->Insert(key));
    }

    // all keys already exist
    for (const auto& key : this->keys)
    {
        ASSERT_FALSE(this->file->Insert(key));
    }
}

TYPED_TEST(FileTest, Remove)
{
    // none of keys exist
    for (const auto& key : this->keys)
    {
        ASSERT_FALSE(this->file->erase(key));
    }
}

TYPED_TEST(FileTest, Exists_Insert)
{
    for (const auto& key : this->keys)
    {
        ASSERT_TRUE(this->file->Insert(key));
    }

    for (const auto& key : this->keys)
    {
        ASSERT_TRUE(this->file->contains(key));
    }
}

TYPED_TEST(FileTest, Insert_Remove)
{
    for (const auto& key : this->keys)
    {
        ASSERT_TRUE(this->file->Insert(key));
    }

    for (const auto& key : this->keys)
    {
        ASSERT_TRUE(this->file->erase(key));
    }

    for (const auto& key : this->keys)
    {
        ASSERT_FALSE(this->file->erase(key));
    }

    // insert after removal is same as inserting new key
    for (const auto& key : this->keys)
    {
        ASSERT_TRUE(this->file->Insert(key));
    }
}

TYPED_TEST(FileTest, Exists_Insert_Remove)
{
    ASSERT_TRUE(this->file->Insert(this->keys[0]));
    ASSERT_TRUE(this->file->contains(this->keys[0]));
    ASSERT_TRUE(this->file->erase(this->keys[0]));
    ASSERT_FALSE(this->file->contains(this->keys[0]));

    ASSERT_TRUE(this->file->Insert(this->keys[1]));
    ASSERT_FALSE(this->file->contains(this->keys[0]));
    ASSERT_TRUE(this->file->contains(this->keys[1]));
    ASSERT_TRUE(this->file->erase(this->keys[1]));
    ASSERT_FALSE(this->file->erase(this->keys[0]));
}

TYPED_TEST(FileTest, KeepSize)
{
    auto keys = this->file->size(), free = this->file->size_free();
    auto err = this->file->resize(this->n_default);
    // never give error, ie do no work
    ASSERT_TRUE(err == MmapFiles::Errors::no_error);
    ASSERT_EQ(this->file->size(), keys);
    ASSERT_EQ(this->file->size_free(), free);
}

TYPED_TEST(FileTest, IncreaseKeys)
{
    auto grow_to = this->n_default * 2;

    this->file->resize(grow_to);
    ASSERT_EQ(this->file->size(), grow_to);
    ASSERT_EQ(this->file->size_free(), grow_to);
}

TYPED_TEST(FileTest, ReduceSize)
{
    auto shrink_to = this->n_default / 2;

    this->file->resize(shrink_to);
    ASSERT_EQ(this->file->size(), shrink_to);
    ASSERT_EQ(this->file->size_free(), shrink_to);
}
