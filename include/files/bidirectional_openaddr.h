#ifndef CUSTOM_FILE_LIBRARY_BIDIRECTIONAL_OPENADDR_ITER
#define CUSTOM_FILE_LIBRARY_BIDIRECTIONAL_OPENADDR_ITER

#include <iterator>
#include <cstddef>

#include "defs.h"

FILE_NAMESPACE_BEGIN

template<
    typename Val, typename Underlying,
    typename Cont,
    typename UtoV, typename IsFree>
class bidirectional_openaddr
{
public:

    using iterator_category = std::bidirectional_iterator_tag;
    using reference         = Val&;
    using difference_type   = std::ptrdiff_t;

    using value_type        = Underlying;
    using pointer           = Underlying*;

    /**
     * @brief
     *
     * @param curr current pointer
     * @param begin one before begin
     * @param end one past end
     */
    bidirectional_openaddr(pointer curr, pointer end) :
        M_data(curr), M_end(end)
    {
    }

    operator bidirectional_openaddr<const Val, Underlying, Cont, UtoV, IsFree>() const
    {
        return bidirectional_openaddr<const Val, Underlying, Cont, UtoV, IsFree>(M_data, M_end);
    }

    reference
    operator*() const
    {
        return *(UtoV()(M_data));
    }

    Val*
    operator->() const
    {
        return UtoV()(M_data);
    }

    bidirectional_openaddr<Val, Underlying, Cont, UtoV, IsFree>&
    operator++()
    {
        do
        {
            ++M_data;
        } while (IsFree()(M_data) && M_data != M_end);

        return *this;
    }

    bidirectional_openaddr<Val, Underlying, Cont, UtoV, IsFree>
    operator++(int)
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    bidirectional_openaddr<Val, Underlying, Cont, UtoV, IsFree>&
    operator--()
    {
        do
        {
            --M_data;
        } while (IsFree()(M_data));

        return *this;
    }

    bidirectional_openaddr<Val, Underlying, Cont, UtoV, IsFree>
    operator--(int)
    {
        auto temp = *this;
        --(*this);
        return temp;
    }

    pointer M_data, M_end;

};

template<
    typename Val1, typename Val2,
    typename Underlying,
    typename Cont,
    typename UtoV, typename IsFree>
bool
operator==(
    const bidirectional_openaddr<Val1, Underlying, Cont, UtoV, IsFree>l,
    const bidirectional_openaddr<Val2, Underlying, Cont, UtoV, IsFree>r)
{
    return l.M_data == r.M_data;
}

template<
    typename Val1, typename Val2, typename Underlying,
    typename Cont,
    typename UtoV, typename IsFree>
bool
operator!=(
    const bidirectional_openaddr<Val1, Underlying, Cont, UtoV, IsFree>l,
    const bidirectional_openaddr<Val2, Underlying, Cont, UtoV, IsFree>r)
{
    return l.M_data != r.M_data;
}

template<
    typename Val1, typename Val2, typename Underlying,
    typename Cont,
    typename UtoV, typename IsFree>
bool
operator<(
    const bidirectional_openaddr<Val1, Underlying, Cont, UtoV, IsFree>l,
    const bidirectional_openaddr<Val2, Underlying, Cont, UtoV, IsFree>r)
{
    return l.M_data < r.M_data;
}

template<
    typename Val1, typename Val2, typename Underlying,
    typename Cont,
    typename UtoV, typename IsFree>
bool
operator<=(
    const bidirectional_openaddr<Val1, Underlying, Cont, UtoV, IsFree>l,
    const bidirectional_openaddr<Val2, Underlying, Cont, UtoV, IsFree>r)
{
    return l.M_data <= r.M_data;
}

template<
    typename Val1, typename Val2, typename Underlying,
    typename Cont,
    typename UtoV, typename IsFree>
bool
operator>(
    const bidirectional_openaddr<Val1, Underlying, Cont, UtoV, IsFree>l,
    const bidirectional_openaddr<Val2, Underlying, Cont, UtoV, IsFree>r)
{
    return l.M_data > r.M_data;
}

template<
    typename Val1, typename Val2, typename Underlying,
    typename Cont,
    typename UtoV, typename IsFree>
bool
operator>=(
    const bidirectional_openaddr<Val1, Underlying, Cont, UtoV, IsFree>l,
    const bidirectional_openaddr<Val2, Underlying, Cont, UtoV, IsFree>r)
{
    return l.M_data >= r.M_data;
}

template<
    typename Val, typename Underlying,
    typename Cont,
    typename UtoV, typename IsFree>
typename bidirectional_openaddr<Val, Underlying, Cont, UtoV, IsFree>::pointer
iter_data(bidirectional_openaddr<Val, Underlying, Cont, UtoV, IsFree> iter)
{
    return iter.M_data;
}

FILE_NAMESPACE_END

#endif
