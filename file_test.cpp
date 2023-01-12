#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <unordered_map>
#include <memory>

#include <functional>

// #include <files/Files.h>
// #include <unit_tests/Funcs.h>
// #include <files/unordered_map_lru.h>

#include <unit_tests/CustomString.h>
#include <files/Files.h>

using namespace std;
using namespace MmapFiles;

struct SpecialInt
{

    SpecialInt() = default;

    SpecialInt(int i) : data(i)
    {
        cout << "construct" << endl;
    }

    SpecialInt(const SpecialInt& r)
    {
        cout << "copy construct" << endl;
        data = 1;
    }

    SpecialInt(SpecialInt&& r)
    {
        cout << "move construct" << endl;
        data = 2;
    }

    SpecialInt&
    operator=(const SpecialInt& r)
    {
        cout << "copy assignment operator" << endl;
        data = 3;
        return *this;
    }

    SpecialInt&
    operator=(SpecialInt&& r)
    {
        cout << "move assignment operator" << endl;
        data = 4;
        return *this;
    }

    int data;
};

bool operator==(SpecialInt l, SpecialInt r) { return l.data == r.data; }

template<>
struct std::hash<SpecialInt>
{
    bool operator()(const SpecialInt& i) const { return std::hash<int>{}(i.data); }
};

/*  i think whats happening is with the find(k) - begin() it uses
    distance of iterators, which is pairs, which is different than blocks

    this iterator should be legacy forward, but youre treating as
    random access
*/

struct Show
{
    // char arr[std::numeric_limits<std::size_t>::max() / 2];
    char arr;
};

int main(int argc, char const *argv[])
{
    unordered_map_file<std::size_t, std::size_t> f(5, "deez");
    f.insert({11, 100});
    f.insert({2, 100});
    f.insert({13, 100});
    f.insert({14, 100});
    f.insert({22, 100});

    f.rehash(10);


    // p (char*)&res.first->second - (char*)&res.first->first
    // unordered_map_file<MyString<156>, long> f(8, "deez");
    // unordered_map_file<MyString<2>, long> f(8, "deez");

    // cout << sizeof(unordered_map_file<MyString<2>, long>::value_type) << endl;

    // auto res = f.insert_or_assign("*", 8);
    // auto sz = sizeof(unordered_map_file<MyString<2>, long>::_blk);
    // auto a = res.first; // iterator
    // auto b = &get<1>(f.M_file[5]); // string
    // auto c = &get<2>(f.M_file[5]); // number

    /*  a.M_data = &f.M_file[5].M_data = &f.M_file[5]
            start of a block
        &b.M_str


        the iterator data is pointing to correct location
    */


    /*  res.first, the iter data is correctly aligned on the start of
            the third block
        res.first->first, the string is correctly algined
        res.first->second, number is incorrectly aligned
    */

    return 0;
}
