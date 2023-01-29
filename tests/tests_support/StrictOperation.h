#ifndef CUSTOM_TESTS_SCRIPTOPERATIONS
#define CUSTOM_TESTS_SCRIPTOPERATIONS

#include <exception>
#include <initializer_list>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <tests_support/SpecialHash.h>

template<
    typename Cont,
    std::enable_if<std::is_same<typename Cont::key_type, SpecialHash>::value, bool>::type = 0>
class StrictOperation
{
protected:

    using hashes    = std::vector<SpecialHash>;
    using size_type = typename hashes::size_type;

    Cont              cont;
    hashes            vec;
    std::vector<bool> rel;

    StrictOperation(Cont&& c)
        : cont(std::move(c))
    {
        reset_gen();
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

        /*  NOTE: order here in EQ matters ???
        */
        ASSERT_EQ(1, cont.erase(vec[index]))
            << "file failed to erase index\n    "
            << index << " = " << vec[index] << "\n";
        ASSERT_FALSE(cont.contains(vec[index]))
            << "file contains just erased index\n    "
            << index << " = " << vec[index] << "\n";

        for (size_type i = 0; i != vec.size(); ++i)
        {
            if (rel[i])
            {
                ASSERT_TRUE(cont.contains(vec[i]))
                    << "file is missing index\n    "
                    << i << " = " << vec[i] << "\n"
                    << "when erasing index\n    "
                    << index << "\n";
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
    template<typename ArrayLike = std::initializer_list<std::size_t>>
    void
    insert(ArrayLike lis, size_type pos = 0)
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

    const hashes&
    keys()
    {
        return vec;
    }

};

#endif
