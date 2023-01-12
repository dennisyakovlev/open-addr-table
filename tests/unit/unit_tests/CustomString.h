#ifndef UNIT_TESTS_CUSTOMSTRING
#define UNIT_TESTS_CUSTOMSTRING

#include <algorithm>
#include <cstddef>
#include <string>
#include <string.h>
#include <iostream>

template<std::size_t sz>
struct MyString
{

    MyString() = default;

    MyString(const char* ptr)
    {
        ::strcpy(M_str, ptr);
    }

    char M_str[sz];

};

template<std::size_t A, std::size_t B>
bool
operator==(const MyString<A>& l, const MyString<B>& r)
{
    return ::strcmp(l.M_str, r.M_str) == 0;
}

template<std::size_t A>
bool
operator==(const MyString<A>& l, const char* r)
{
    return ::strcmp(l.M_str, r) == 0;
}
template<std::size_t A>
bool
operator==(const char* l, const MyString<A>& r)
{
    return ::strcmp(l, r.M_str) == 0;
}

template<std::size_t A, std::size_t B>
bool
operator<(const MyString<A>& l, const MyString<B>& r)
{
    return ::strcmp(l.M_str, r.M_str) < 0;
}

template<std::size_t A>
bool
operator<(const MyString<A>& l, const char* r)
{
    return ::strcmp(l.M_str, r) < 0;
}

template<std::size_t A>
bool
operator<(const char* l, const MyString<A>& r)
{
    return ::strcmp(l, r.M_str) < 0;
}

template<std::size_t A, std::size_t B>
MyString<A + B>
operator+(const MyString<A>& l, const MyString<B>& r)
{
    std::string s = std::string(l.M_str) + std::string(r.M_str);
    return MyString<A + B>(s.c_str());
}

template<std::size_t A, std::size_t B>
MyString<A + B>
operator+(const MyString<A>& l, const char (&r)[B])
{
    std::string s = std::string(l.M_str) + std::string(r);
    return MyString<A + B>(s.c_str());
}

template<std::size_t A, std::size_t B>
MyString<A + B>
operator+(const char (&l)[A], const MyString<B>& r)
{
    std::string s = std::string(l) + std::string(r.M_str);
    return MyString<A + B>(s.c_str());
}

template<std::size_t sz>
std::ostream& operator<<(std::ostream& out, const MyString<sz>& s)
{
    out << std::string(s.M_str);
    return out;
}

template<std::size_t sz>
struct std::hash<MyString<sz>>
{
    std::size_t operator()(const MyString<sz>& s) const
    {
        return std::hash<string>{}(s.M_str);
    }
};

template<std::size_t sz, std::size_t hash>
struct collison
{
    std::size_t operator()(const MyString<sz>& s) const
    {
        return hash;
    }
};

#endif
