#include <unit_tests/SpecialHash.h>

std::size_t SpecialHash::vals = 0;

std::ostream&
operator<<(std::ostream& out, const SpecialHash& spec)
{
    out << "{" << spec.first << "," << spec.second << "}";
    return out;
}

SpecialHash
gen_unique(std::size_t hash)
{
    return { hash,++SpecialHash::vals };
}

void
reset_gen()
{
    SpecialHash::vals = 0;
}
