#ifndef CUSTOM_FILE_LIBRARY_ALLOCATOR
#define CUSTOM_FILE_LIBRARY_ALLOCATOR

#include <cstddef>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include <files/Defs.h>

#ifndef _SC_PAGESIZE
    #ifdef PAGE_SIZE
        #define _SC_PAGESIZE PAGE_SIZE
    #else
        #define _SC_PAGESIZE -1
    #endif
#endif

/*  NOTE: mmap vs mmap64 
*/

FILE_NAMESPACE_BEGIN

template<typename T>
class mmap_allocator
{
public:

    using value_type = T;
    using pointer    = T*;
    using size_type  = std::size_t; 

private:

    int
    open_or_create()
    {
        int fd;

        if (::access(M_file.c_str(), F_OK) == 0)
        {
            fd = ::open(M_file.c_str(), O_RDWR, 0);
        }
        else
        {
            fd = ::open(M_file.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
        }

        return fd;
    }

    int
    close(int fd)
    {
        return ::close(fd);
    }

    size_type
    page_aligned(size_type n)
    {
        auto page_sz = sysconf(_SC_PAGESIZE);
        if (page_sz == -1)
        {
            page_sz = 4096;
        }

        const auto sz = n * sizeof(value_type);
        return sz + (page_sz - (sz % page_sz));
    }

    pointer
    mmap(size_type sz)
    {
        int fd = open_or_create();
        if (fd == -1)
        {
            return reinterpret_cast<pointer>(MAP_FAILED);
        }

        if (::ftruncate64(fd, sz))
        {
            close(fd);
            return reinterpret_cast<pointer>(MAP_FAILED);
        }

        void* ptr = ::mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        if (close(fd))
        {
            ::munmap(ptr, sz);

            return static_cast<pointer>(MAP_FAILED);
        }

        return static_cast<pointer>(ptr);
    }

    pointer
    mremap(pointer old_addr, size_type old_sz, size_type sz)
    {
        int fd = open_or_create();
        if (fd == -1)
        {
            return reinterpret_cast<pointer>(MAP_FAILED);
        }

        if (::ftruncate64(fd, sz))
        {
            close(fd);
            return reinterpret_cast<pointer>(MAP_FAILED);
        }

        void* ptr = ::mremap(old_addr, old_sz, sz, MREMAP_MAYMOVE);

        if (close(fd))
        {
            ::munmap(ptr, sz);

            return static_cast<pointer>(MAP_FAILED);
        }

        return static_cast<pointer>(ptr);
    }

public:

    // NOTE: delete this and add constructor which forces
    //       person to acknowladge that theyve constructed unusable object
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

    /**
     * @brief 
     * 
     * @param n must be greater than 0 
     * @return pointer 
     */
    pointer
    allocate(size_type n)
    {
        M_least = false;

        return mmap(n * sizeof(value_type));
    }

    std::pair<pointer, size_type>
    allocate_at_least(std::size_t n)
    {
        M_least = true;
        const auto sz = page_aligned(n);
        return {mmap(sz), sz / n};
    }

    pointer
    reallocate(pointer old_addr, size_type n_old, size_type n)
    {
        auto sz_old = n_old * sizeof(value_type);
        if (M_least)
        {
            sz_old = page_aligned(n_old);
        }

        M_least = false;
        return mremap(old_addr, sz_old, n * sizeof(value_type));
    }

    std::pair<pointer, size_type>
    reallocate_at_lest(pointer old_addr, size_type n_old, size_type n)
    {
        auto sz_old = n_old * sizeof(value_type);
        if (M_least)
        {
            sz_old = page_aligned(n_old);
        }

        M_least = true;
        const auto sz = page_aligned(n);
        return {mremap(old_addr, sz_old, sz), sz / n};
    }

    void
    deallocate(pointer addr, size_type n)
    {
        auto sz_old = n * sizeof(value_type);
        if (M_least)
        {
            sz_old = page_aligned(n);
        }

        ::msync(addr, sz_old * sizeof(value_type), MS_SYNC);
        ::munmap(addr, n);
    }

    /*  NOTE: destroy idea here is wrong 
    */
    int
    destory()
    {
        return ::remove(M_file.c_str());
    }

private:

    const std::string M_file;
    bool              M_least;

};

FILE_NAMESPACE_END

#endif
