#ifndef CUSTOM_FILE_LIBRARY_BASIC_ALLOCATOR
#define CUSTOM_FILE_LIBRARY_BASIC_ALLOCATOR

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <files/Defs.h>

FILE_NAMESPACE_BEGIN

/**
 * @brief Extension of basic std allocator with reallocate.
 *        Container requires reallocate, so added here.
 *
 * @tparam T
 */
template<typename T>
class basic_allocator :
    public std::allocator<T>
{
public:
    using base = std::allocator<T>;

    using pointer   = typename base::pointer;
    using size_type = typename base::size_type;

    using base::allocator;

    /**
     * @brief For compatibility with mmap allocator. Just
     *        disregard the naming request.
     *
     */
    basic_allocator(std::string)
    {
    }

    /**
     * @brief Forcefully reallocate some memory by allocating
     *        new memory, moving old memory into new memory,
     *        and deallocating old memory
     *
     * @param old_addr old address returned from allocate
     * @param n_old old number of elememts given to allocate
     * @param n new number of elements to allocate
     * @return pointer start of new memory
     */
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

    /**
     * @brief For compatibility with mmap allocator. Just
     *        disregard the naming request.
     *
     */
    void
    wipe()
    {
    }

};

FILE_NAMESPACE_END

#endif
