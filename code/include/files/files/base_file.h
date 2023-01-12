#ifndef CUSTOM_FILE_LIBRARY_BASEFILE
#define CUSTOM_FILE_LIBRARY_BASEFILE

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utility>

#include "FileBlock.h"
#include <files/Allocators.h>
#include <files/Defs.h>
#include <files/Iterators.h>
#include <files/Utils.h>

FILE_NAMESPACE_BEGIN

/*  So base_file should have the inset emplace etc funcs where the key
    is the index to insert into

    the point of base_base is to encapsulate the filesystem

    should have the standard insert, emplace, erase etc. but this
    base_file is an array and the other files just manage it

    what about the hash table file with collisions ?
        will that be dealt with the hash file



    vector_file

    will we follow the semantics of a vector ?
        no
        if we erase elem, dont shift all the elements back

    Should not be <Key, Values...>, should be <Key, Val> and val
    can be FileBlock or whatever

    so is base_file just a mmap'ed unordered_map ???
        like what's point of vector_file
        if we make base_file an unordered_map then we have to deal with the collisions
*/

template<
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>
>
class base_file
{
public:

    using _block = block<Key, Value>

    using Allocator       = MmapAllocator<_block>;
    
    using value_type      = _block;
    using reference       = _block&;
    using const_reference = const _block&;
    using pointer         = _block*;
    using const_pointer   = const _block*;
    using size_type       = typename _block::size_type;
    using iterator        = Iterator<_block, base_file<Allocator, Key, Value>>;
    using const_iterator  = Iterator<const _block, base_file<Allocator, Key, Value>>;
    using difference_type = typename iterator::difference_type;

    using key_type            = Key;
    using reference_key       = Key&;
    using const_reference_key = const Key&;

    base_file(std::string name, size_type keys)
        : M_name(std::move(name)), M_full(M_name),
          M_total(keys),
          M_alloc()
    {
        M_file = M_alloc.allocate(M_total, OpenFile().GetReturn());
        ::truncate(M_full.c_str(), M_total * sizeof(value_type));
    }

    base_file(std::string path, std::string name, size_type keys)
        : M_path(std::move(path)), M_name(std::move(name)), M_full(M_path + "/" + M_name),
          M_total(keys),
          M_alloc()
    {
        M_file = M_alloc.allocate(M_total, OpenFile().GetReturn());
        ::truncate(M_full.c_str(), M_total * sizeof(value_type));
    }

    ~base_file()
    {
        M_alloc.deallocate(M_file, M_total);
    }

    bool
    ready() const
    {
        return ::access(M_full.c_str(), F_OK) == 0;
    }

    size_type
    size() const
    {
        return M_total;
    }

    const_iterator
    cbegin() const
    {
        return const_iterator(M_file);
    }

    iterator
    begin()
    {
        return iterator(M_file);
    }

    const_iterator
    cend() const
    {
        return begin() + M_total;
    }

    iterator
    end()
    {
        return begin() + M_total;
    }

    /**
     * @brief Set the number of key slots available.
     * 
     * @param num Number of total key slots to allow.
     * @return Errors
     */
    Errors
    resize(size_type n)
    {
        auto res = OpenFile();
        if
        (
            !res.Valid()
            ||
            ::truncate(M_full.c_str(), n * sizeof(value_type))
        )
        {
            return Errors::system;    
        }

        M_file  = M_alloc.reallocate(M_file, M_total, n, res.GetReturn());
        M_total = n;

        return Errors::no_error;
    }

    Errors
    FreeResources()
    {
        M_alloc.deallocate(M_file, M_total);

        if (ready())
        {
            if (::remove(M_full.c_str()))
            {
                return Errors::system;
            }
        }

        return Errors::no_error;
    }

protected:

    /**
     * @brief Open existing file or create file
     *        and open.
     * 
     * @note You can modify the file using the fd
     *       so technically this function shouldn't
     *       be const. I don't want to duplicate
     *       code.
     * 
     * @return Returned<int> fd for file. 
     */
    Returned<int>
    OpenFile()
    {
        int fd;

        if (ready())
        {
            fd = ::open(M_full.c_str(), O_RDWR, 0);
        }
        else
        {
            fd = ::open(M_full.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
        }

        if (fd == -1)
        {
            return { -1,Errors::system };
        }

        return { fd,Errors::no_error };
    }

    const std::string M_path, M_name, M_full; 
    size_type         M_total;
    Allocator         M_alloc;
    pointer           M_file;

};

FILE_NAMESPACE_END

#endif
