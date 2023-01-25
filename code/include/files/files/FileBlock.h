#ifndef CUSTOM_FILE_LIBRARY_BLOCK
#define CUSTOM_FILE_LIBRARY_BLOCK

#include <cassert>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include <files/Defs.h>

FILE_NAMESPACE_BEGIN

template<std::size_t I, typename Arg, typename... Args>
struct _type
{
    using Elem = typename _type<I - 1, Args...>::Elem;
};

template<typename Arg, typename... Args>
struct _type<0, Arg, Args...>
{
    using Elem = Arg;
};

template<typename Arg, typename... Args>
struct _size_struct
{
    static constexpr std::size_t
    _s()
    {
        return 1 + _size_struct<Args...>::_s();
    }
};

template<>
struct _size_struct<void>
{
    static constexpr std::size_t
    _s()
    {
        return 0;
    }
};

template<typename... Args>
constexpr std::size_t
Length()
{
    return _size_struct<Args..., void>::_s();
}

template<std::size_t A, std::size_t B>
constexpr std::size_t
Min()
{
    return A < B ? A : B;
}

template<std::size_t A, std::size_t B>
constexpr std::size_t
Max()
{
    return A < B ? B : A;
}



template<typename... Values>
struct block;

template<std::size_t I, typename... Args>
const typename _type<I, Args...>::Elem&
get(const block<Args...>& fb);

template<std::size_t I, typename... Args>
typename _type<I, Args...>::Elem&
get(block<Args...>& fb);

template<std::size_t I, typename Arg, typename... Args>
void
set(block<Args...>& fb, Arg&& arg);



/**
 * @brief Recursive struct which unrolls equal to a
 *        loop going from 0-indexed [I,End).
 */
template<std::size_t End, std::size_t I = 0>
struct _loop
{

    /**
     * @brief When I is SIZE_MAX, indicates I was 0 and one was
     *        subtracted. The maximum length of a pack should
     *        be SIZE_MAX, ie max index is SIZE_MAX - 1.
     */
    static_assert(!(I == SIZE_MAX), "Cannot have empty block");

    template<typename... Orig, typename Arg, typename... Args>
    static void
    _init(block<Orig...>& fb, Arg&& arg, Args&&... args)
    {
        set<I>(fb, std::forward<Arg>(arg));
        _loop<End, I + 1>::_init(fb, std::forward<Args>(args)...);
    }

    template<typename... Args1, typename... Args2>
    static void
    _init(block<Args1...>& into, const block<Args1...>& from)
    {
        set<I>(into, get<I>(from));
        _loop<End, I + 1>::_init(into, from);
    }

    template<typename Arg, typename... Args>
    static constexpr std::size_t
    _sum_to()
    {
        return sizeof(Arg) + _loop<End, I + 1>::template _sum_to<Args...>();
    }

    template<typename From, typename... Froms>
    struct _convertible
    {
        template<typename To, typename... Tos>
        static constexpr bool
        _f()
        {
            return std::is_convertible<From, To>::value 
                && _loop<End, I + 1>::template _convertible<Froms...>::template _f<Tos...>();
        }
    };

    /**
     * @brief Not end of either sequence. 
     */
    template
    <
        typename... Args1, typename... Args2,
        typename std::enable_if<Length<Args1...>() - I != 0 && Length<Args2...>() - I != 0, bool>::type = true
    >
    static bool
    _less(const block<Args1...>& l, const block<Args2...>& r)
    {
        if (get<I>(l) < get<I>(r)) return true;
        if (get<I>(r) < get<I>(l)) return false;
        
        return _loop<End, I + 1>::_less(l, r);
    }

    /**
     * @brief End of "l" sequence. 
     */
    template
    <
        typename... Args1, typename... Args2,
        typename std::enable_if<Length<Args1...>() - I == 0, bool>::type = true
    >
    static bool
    _less(const block<Args1...>& l, const block<Args2...>& r)
    {
        return true;
    }

    /**
     * @brief End of "r" sequence. 
     */
    template
    <
        typename... Args1, typename... Args2,
        typename std::enable_if<Length<Args2...>() - I == 0, bool>::type = true
    >
    static bool
    _less(const block<Args1...>& l, const block<Args2...>& r)
    {
        return false;
    }

    template<typename... Args1, typename... Args2>
    static constexpr bool
    _eq(const block<Args1...>& l, const block<Args2...>& r)
    {
        return get<I>(l) == get<I>(r)
            && _loop<End, I + 1>::_eq(l, r);
    }

};

/**
 * @brief Base case.
 */
template<std::size_t I>
struct _loop<I, I>
{
    template<typename... Orig>
    static constexpr void
    _init(block<Orig...>& fb)
    {}

    template<typename... Args1, typename... Args2>
    static constexpr void
    _init(block<Args1...>& into, const block<Args1...>& from)
    {}

    template<typename... Args>
    static constexpr std::size_t
    _sum_to()
    {
        return 0;
    }

    template<typename... Froms>
    struct _convertible
    {
        template<typename... Tos>
        static constexpr bool
        _f()
        {
            return true;
        }
    };

    /**
     * @brief Equal length, equal elements. 
     */
    template<typename... Args1, typename... Args2>
    static constexpr bool
    _less(const block<Args1...>& l, const block<Args2...>& r)
    {
        return false;
    }

    template<typename... Args1, typename... Args2>
    static constexpr bool
    _eq(const block<Args1...>& l, const block<Args2...>& r)
    {
        return true;
    }

};

template<typename... Args>
constexpr std::size_t
SizeofTotal()
{
    return _loop<Length<Args...>()>
        ::template _sum_to<Args...>();
}

template<std::size_t I, typename... Args>
constexpr std::size_t
SizeofPartial()
{
    return _loop<I>
        ::template _sum_to<Args...>();
}
template<typename... Orig, typename... Args>
constexpr void
Unroll(block<Orig...>& fb, Args&&... args)
{
    _loop<Length<Orig...>()>::_init(fb, std::forward<Args>(args)...);
}

template<typename... Args1, typename... Args2>
constexpr void
CopyUnroll(block<Args1...>& into, const block<Args2...>& from)
{
    _loop<Length<Args1...>()>::_init(into, from);
}

/*  NOTE: should all the elements have trivial destructors ?
*/

/*  NOTE: define destructor
*/

/**
 * @brief A fixed size structure to represent arbitrary types
 *        stored in contiguous memory.
 * 
 * @tparam Key    Key type.
 * @tparam Values Value types.
 * 
 * @note Values types must meet requirements of std::tuple elements.
 */
template<typename... Vals>
struct block
{

    using size_type = std::size_t;

    /*  NOTE: default is not actually default, as in object wont be default initialzed here as M_data will just be given
              random values

              should they be?
    */
    block()
    {
        
    }

    template<typename... Args>
    block(const block<Args...>& other)
    {
        CopyUnroll(*this, other);
    }
    
    /* NOTE: this allows diff sizes? okay
    */

    /**
     * @brief Constrained forwarding. This constructor will
     *        attempt to be used when copy constructing. Can
     *        stop this by forcing the "Args" to be
     *        convertible to "Vals". 
     */
    template
    <
        typename... Args,
        typename = typename std::enable_if
        <
            _loop<Min<Length<Args...>(), Length<Vals...>()>()>
            ::template _convertible<Args...>
            ::template _f<Vals...>()
        >::type
    >
    block(Args&&... vals)
    {
        Unroll(*this, std::forward<Args>(vals)...);
    }

    char M_data[SizeofTotal<Vals...>()];
};

/**
 * @brief Get the I'th zero-indexed element out of Values.
 * 
 * @tparam I Index to get.
 * @return ElementType<I, fb>& Element.
 */
template<std::size_t I, typename... Args>
const typename _type<I, Args...>::Elem&
get(const block<Args...>& fb)
{
    return *reinterpret_cast<const typename _type<I, Args...>::Elem*>
    (
        fb.M_data + SizeofPartial<I, Args...>()
    );
}

template<std::size_t I, typename... Args>
typename _type<I, Args...>::Elem&
get(block<Args...>& fb)
{
    return *reinterpret_cast<typename _type<I, Args...>::Elem*>
    (
        fb.M_data + SizeofPartial<I, Args...>()
    );
}

/**
 * @brief Get the I'th zero-indexed element out of Values.
 * 
 * @tparam I Index to get.
 * @return ElementType<I, fb>& Element.
 */
template<std::size_t I, typename Arg, typename... Args>
void
set(block<Args...>& fb, Arg&& arg)
{
    // NOTE: should remove cv here ???
    //       Arg is forward ref so should not have ref ty
    //       we wont have const on Arg (we cannot)

    // ah we dont want to use Arg, we must use the final type
    //  ie use ElemType

    using ty = typename _type<I, Args...>::Elem;
    using alloc = std::allocator<ty>;
    alloc a;
    std::allocator_traits<alloc>::construct
    (
        a,
        reinterpret_cast<ty*>(fb.M_data + SizeofPartial<I, Args...>()),
        std::forward<Arg>(arg)
    );
}

template<typename... Args1, typename... Args2>
bool
operator==(const block<Args1...>& l, const block<Args2...>& r)
{
    constexpr auto sz_l = Length<Args1...>(); 
    constexpr auto sz_r = Length<Args2...>(); 
    return sz_l == sz_r
        && _loop<Min<sz_l, sz_r>()>::_eq(l, r);
}

template<typename... Args1, typename... Args2>
bool
operator!=(const block<Args1...>& l, const block<Args2...>& r)
{
    return !(l == r);
}

template<typename... Args1, typename... Args2>
bool
operator<(const block<Args1...>& l, const block<Args2...>& r)
{
    return _loop<Max<Length<Args1...>(), Length<Args2...>()>()>::_less(l, r);
}

template<typename... Args1, typename... Args2>
bool
operator<=(const block<Args1...>& l, const block<Args2...>& r)
{
    return (l < r) || (l == r);
}

template<typename... Args1, typename... Args2>
bool
operator>(const block<Args1...>& l, const block<Args2...>& r)
{
    return r < l;
}

template<typename... Args1, typename... Args2>
bool
operator>=(const block<Args1...>& l, const block<Args2...>& r)
{
    return r <= l;
}

FILE_NAMESPACE_END

#endif 
