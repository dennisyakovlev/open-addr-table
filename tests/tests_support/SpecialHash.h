#ifndef UNIT_TESTS_SPECIALHASH
#define UNIT_TESTS_SPECIALHASH

#include <functional>
#include <iostream>
#include <utility>

/**
 * @brief Class which lets different object have the same hash
 *        values but not compare equal. Used as a key when
 *        hash conflicts are wanted.
 * 
 */
class SpecialHash : public std::pair<std::size_t, std::size_t>
{
public:

    using std::pair<std::size_t, std::size_t>::pair;

    ~SpecialHash()
    {
        first  = 0;
        second = 0;
    }

    static std::size_t vals;
};

std::ostream&
operator<<(std::ostream& out, const SpecialHash& spec);

/**
 * @brief Generate a SpecialHash with a unique value
 *        but a specified hash value.
 * 
 * @param hash 
 * @return SpecialHash 
 */
SpecialHash
gen_unique(std::size_t hash);

/**
 * @brief Reset the generation of SpecialHash's so that
 *        a new (but not distinct from old) set of values
 *        is used. 
 */
void
reset_gen();

template<>
struct std::hash<SpecialHash>
{
    std::size_t operator()(const SpecialHash& spec) const
    {
        return spec.first;
    }
};

#endif
