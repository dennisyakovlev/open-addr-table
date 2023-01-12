#include <iostream>
#include <type_traits>

template<bool cond, typename T = void>
struct enable
{

};

template<typename T>
struct enable<true, T>
{
    using type = T;
};

template<typename, typename = void>
struct S;

template<typename T>
struct S<T, std::enable_if_t<std::is_same<T, int>::value>>
{

};

template<typename T>
struct S<T, std::enable_if_t<std::is_same<T, float>::value>>
{

};

/*
    main
    
    S<int> s;
    S<float> s2;
    S<double> stinky;

*/