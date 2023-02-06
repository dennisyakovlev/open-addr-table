#ifndef CUSTOM_FILE_LIBRARY_BASIC_ALLOCATOR
#define CUSTOM_FILE_LIBRARY_BASIC_ALLOCATOR

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <files/Defs.h>

FILE_NAMESPACE_BEGIN

template<typename T>
class basic_allocator :
    public std::allocator<T>
{
public:
    using base = std::allocator<T>;

    using pointer   = typename base::pointer;
    using size_type = typename base::size_type;

    using base::allocator;

    basic_allocator(std::string)
    {
    }

    pointer
    reallocate(pointer old_addr, size_type n_old, size_type n)
    {
        auto ptr = this->allocate(n);
        for (size_type i = 0; i != std::min(n_old, n); ++i)
        {
            std::allocator_traits<base>::construct
            (
                *this,
                ptr + i,
                std::move(*(old_addr+ i))
            );
        }

        this->deallocate(old_addr, n_old);

        return ptr;
    }

};

FILE_NAMESPACE_END

#endif
