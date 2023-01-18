#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include <files/Files.h>
#include <unit_tests/Funcs.h>
#include <unit_tests/SpecialHash.h>
#include <unit_tests/Vars.h>

// NOTE: rename file something like linear_probe

class UnorderedMapExtraTest : public testing::Test
{
protected:

    MmapFiles::unordered_map_file<SpecialHash,std::size_t> cont;

    UnorderedMapExtraTest()
        : cont(unit_test_file)
    {
        reset_gen();
        RemoveCreateDir(unit_test_file);
    }

};

TEST_F(UnorderedMapExtraTest, A)
{
    /*  have a container 

        0 4
        1 5
        2 5
        3
        4 4
        5 4
    */

    cont.reserve(6);

    std::vector<SpecialHash> vec = {
        gen_unique(4),
        gen_unique(4),
        gen_unique(4),
        gen_unique(5),
        gen_unique(5)
    };
    std::for_each(vec.cbegin(), vec.cend(), [&](const SpecialHash& spec){
        ASSERT_TRUE(cont.emplace(spec, 0).second);
    });

    ASSERT_EQ(6, cont.size());
}
