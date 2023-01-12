#include <cstddef>
#include <type_traits>
#include <ftw.h>
#include <string>

#include <gtest/gtest.h>

#include <files/Files.h>
#include <unit_tests/CustomString.h>
#include <unit_tests/Funcs.h>
#include <unit_tests/Vars.h>

/**
 * @tparam Size integral constant.
 */
template<typename Size>
class BaseFileTest : public testing::Test
{
protected:

    using Key      = MyString<Size::value>;
    using Val      = std::size_t;
    using FileType = MmapFiles::base_file
    <
        Key,                        // key
        MyString<Size::value * 32>, // some padding
        Val                         // a value
    >;

    std::size_t n_default;
    FileType*   file;

    BaseFileTest()
        : n_default(32)
    {
        RemoveCreateDir(unit_test_dir);
        file = new FileType(unit_test_dir, unit_test_file, n_default);
    }

    typename FileType::value_type
    CreateBlock(Val i)
    {
        return
        {
            std::to_string(i).c_str(),
            "",
            i
        };
    }

};

using MyTypes = testing::Types
<
    std::integral_constant<std::size_t, 16>,
    std::integral_constant<std::size_t, 128>,
    std::integral_constant<std::size_t, 1024>,
    std::integral_constant<std::size_t, 65536>
>;
TYPED_TEST_CASE(BaseFileTest, MyTypes);

TYPED_TEST(BaseFileTest, Iterate)
{
    // Create a bunch of unique blocks in the file using
    // a counter. Verify the stored blocks are the expected
    // values.

    std::size_t i = 0;
    for (auto& block : *this->file)
    {
        block = this->CreateBlock(i++);
    }

    ASSERT_EQ(i, this->n_default);
    
    i = 0;
    for (const auto& block : *this->file)
    {
        ASSERT_TRUE(block == this->CreateBlock(i++));
    }
}

TYPED_TEST(BaseFileTest, Edges)
{
    // Force to use the ending block. Its possible
    // that "end" is implemented incorrectly.

    auto iter = this->file->begin();
    *iter = this->CreateBlock(10);
    iter += this->n_default - 1;
    *iter = this->CreateBlock(-1);

    iter = this->file->begin();
    ASSERT_TRUE(*iter == this->CreateBlock(10));
    iter += this->n_default - 1;
    ASSERT_TRUE(*iter == this->CreateBlock(-1));
}
