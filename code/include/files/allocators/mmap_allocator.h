#ifndef CUSTOM_FILE_LIBRARY_ALLOCATOR
#define CUSTOM_FILE_LIBRARY_ALLOCATOR

#include <cstddef>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include <files/Defs.h>

#ifndef _SC_PAGESIZE
    #define _SC_PAGESIZE PAGE_SIZE
#endif

FILE_NAMESPACE_BEGIN

template<typename T>
class mmap_allocator
{
public:

    using value_type = T;
    using pointer    = T*;
    using size_type  = std::size_t; 

private:

    bool
    _ready() const
    {
        return ::access(M_file.c_str(), F_OK) == 0;
    }

    int
    _open_or_create()
    {
        int fd;

        /*  NOTE: do we need the seperate ifs ?
        */
        if (_ready())
        {
            fd = ::open(M_file.c_str(), O_RDWR, 0);
        }
        else
        {
            fd = ::open(M_file.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
        }

        return fd;
    }

    size_type
    _page_aligned(size_type n)
    {
        const auto sz = n * sizeof(value_type);
        const auto page_sz = sysconf(_SC_PAGESIZE);
        return sz + (page_sz - (sz % page_sz));
    }

    pointer
    _mmap(size_type sz)
    {
        int fd = _open_or_create();
        if (fd == -1)
        {
            /*  NOTE: return nullptr ???
                        thing is you *can* have nullptr in rare cases
                
                same for mremap
            */
            return reinterpret_cast<pointer>(-1);
        }

        ::ftruncate64(fd, sz);

        /*  NOTE: maybe give hint of random usage to not preload file
        */
        return static_cast<pointer>
        (
            ::mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
        );
    }

    pointer
    _mremap(pointer old_addr, size_type old_sz, size_type sz)
    {
        int fd = _open_or_create();
        if (fd == -1)
        {
            return reinterpret_cast<pointer>(-1);
        }

        ::ftruncate64(fd, sz);

        /*  NOTE: maybe give hint of random usage to not preload file
        */
        return static_cast<pointer>
        (
            ::mremap(old_addr, old_sz, sz, MREMAP_MAYMOVE)
        );
    }

public:

    /**
     * @brief Not usable if default constructed. Is just here
     *        if need be.
     * 
     */
    mmap_allocator() = default;

    mmap_allocator(const std::string& file)
        : M_file(file),
          M_least(false)
    {
    }

    pointer
    allocate(size_type n)
    {
        M_least = false;
        return _mmap(n * sizeof(value_type));
    }

    std::pair<pointer, size_type>
    allocate_at_least(std::size_t n)
    {
        M_least = true;
        const auto sz = _page_aligned(n);
        return {_mmap(sz), sz / n};
    }

    pointer
    reallocate(pointer old_addr, size_type n_old, size_type n)
    {
        auto sz_old = n_old * sizeof(value_type);
        if (M_least)
        {
            sz_old = _page_aligned(n_old);
        }

        M_least = false;
        return _mremap(old_addr, sz_old, n * sizeof(value_type));
    }

    std::pair<pointer, size_type>
    reallocate_at_lest(pointer old_addr, size_type n_old, size_type n)
    {
        auto sz_old = n_old * sizeof(value_type);
        if (M_least)
        {
            sz_old = _page_aligned(n_old);
        }

        M_least = true;
        const auto sz = _page_aligned(n);
        return {_mremap(old_addr, sz_old, sz), sz / n};
    }

    /*  NOTE: do we truncate the file to 0 here ?
    */

    void
    deallocate(pointer addr, size_type n)
    {
        auto sz_old = n * sizeof(value_type);
        if (M_least)
        {
            sz_old = _page_aligned(n);
        }

        ::msync(addr, n * sizeof(value_type), MS_SYNC);
        ::munmap(addr, n);
    }

private:

    const std::string M_file;
    bool              M_least;

};

FILE_NAMESPACE_END

#endif
