#include <unit_tests/SpecialHash.h>

std::size_t SpecialHash::vals = 0;

SpecialHash::~SpecialHash()
{
    first  = 0;
    second = 0;
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
