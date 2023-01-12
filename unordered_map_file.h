#ifndef CUSTOM_FILE_LIBRARY_UNORDEREDMAPFILE
#define CUSTOM_FILE_LIBRARY_UNORDEREDMAPFILE

#include <fcntl.h>
#include <limits>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <files/Allocators.h>
#include <files/Iterators.h>
#include <files/Defs.h>
#include "FileBlock.h"

/*  NOTE: use open addressing with linear probing

    account for iterator invalidation
*/

/*  NOTE: truncate should be shoved into the allocator ?
*/

/*  NOTE: must fix iterator to allow for different tags
*/

FILE_NAMESPACE_BEGIN

template<
    typename Key,
    typename Value,
    typename Hash  = std::hash<Key>>
class unordered_map_file
{
public:

    using _blk = block<std::size_t, Key, Value>;
    using allocator = MmapAllocator<_blk>;

    using value_type      = std::pair<const Key, Value>;
    using reference       = std::pair<const Key, Value>&;
    using const_reference = const std::pair<const Key, Value>&;
    using pointer         = std::pair<const Key, Value>*;
    using const_pointer   = const std::pair<const Key, Value>*;
    using size_type       = std::size_t;

    struct convert
    {
        pointer operator()(_blk* blk)
        {
            return reinterpret_cast<pointer>(std::addressof(get<1>(*blk)));
        } 
    };

    struct is_free
    {
        bool operator()(_blk* blk) const
        {
            return get<0>(*blk) == std::numeric_limits<size_type>::max();
        }
    };

    using iterator       = bidirectional_openaddr<
        std::pair<const Key, Value>,
        _blk,
        unordered_map_file<Key, Value, Hash>,
        convert, is_free>;
    using const_iterator = bidirectional_openaddr<
        const std::pair<const Key, Value>,
        _blk,
        unordered_map_file<Key, Value, Hash>,
        convert, is_free>;
    using difference_type = typename iterator::difference_type;

    using key_type            = Key;
    using reference_key       = Key&;
    using const_reference_key = const Key&;
    using pointer_key         = Key*;
    using const_pointer_key   = const Key*;
    using mapped_type         = Value;

private:

    bool
    _ready() const
    {
        return ::access(M_name.c_str(), F_OK) == 0;
    }

    int
    _open_or_create()
    {
        int fd;

        /*  NOTE: do we need the seperate ifs ?
        */
        if (_ready())
        {
            fd = ::open(M_name.c_str(), O_RDWR, 0);
        }
        else
        {
            fd = ::open(M_name.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
        }

        return fd;
    }

    /**
     * @brief Not very good for many reasons, but
     *        good enough.
     */
    std::string
    _gen_random(const std::string::size_type len)
    {
        ::srand(::time(nullptr));
        static const char alphanum[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
        ;
        std::string tmp_s;
        tmp_s.reserve(len);

        for (std::string::size_type i = 0; i != len; ++i)
        {
            tmp_s += alphanum[::rand() % (sizeof(alphanum) - 1)];
        }
        
        return tmp_s;
    }

    _blk&
    _block(size_type index)
    {
        return M_file[index];
    }

    const _blk&
    _block(size_type index) const
    {
        return M_file[index];
    }

    size_type&
    _hash(size_type index)
    {
        return get<0>(_block(index));
    }

    size_type
    _hash(size_type index) const
    {
        return get<0>(_block(index));
    }

    reference_key
    _key(size_type index)
    {
        return get<1>(_block(index));
    }

    const_reference_key
    _key(size_type index) const
    {
        return get<1>(_block(index));
    }

    void
    _init()
    {
        int fd = _open_or_create();
        if (fd == -1)
        {
            return;
        }

        M_file = M_alloc.allocate(M_buckets, fd);
        ::truncate(M_name.c_str(), M_buckets * sizeof(_blk));

        for (size_type i = 0; i != M_buckets; ++i)
        {
            _hash(i) = std::numeric_limits<size_type>::max();
        }
    }

public:

    unordered_map_file()
        : M_name(_gen_random(16)),
          M_buckets(0), M_elem(0),
          M_alloc(M_name)
    {
        _init();
    }

    unordered_map_file(std::string name)
        : M_name(std::move(name)),
          M_buckets(0), M_elem(0),
          M_alloc(M_name)
    {
        _init();
    }

    unordered_map_file(size_type buckets)
        : M_name(_gen_random(16)),
          M_buckets(buckets), M_elem(0),
          M_alloc(M_name)
    {
        _init();
    }

    unordered_map_file(size_type buckets, std::string name)
        : M_name(std::move(name)),
          M_buckets(buckets), M_elem(0),
          M_alloc(M_name)
    {
        _init();
    }

    size_type
    size() const
    {
        return M_elem;
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
        return const_iterator(M_file + M_buckets);
    }

    iterator
    end()
    {
        return iterator(M_file + M_buckets);
    }

    void
    reserve(size_type buckets)
    {
        int fd = _open_or_create();

        if
        (
            fd == -1
            ||
            ::truncate(M_name.c_str(), buckets * sizeof(_blk))
        )
        {
            return;    
        }

        M_file  = M_alloc.reallocate(M_file, M_buckets, buckets, fd);

        for (; M_buckets != buckets; ++M_buckets)
        {
            _hash(M_buckets) = std::numeric_limits<size_type>::max();
        }
    }

    iterator
    find(const_reference_key k)
    {
        return iterator(
            const_cast<typename iterator::pointer>(
                iter_data(
                    static_cast<const unordered_map_file<Key, Value, Hash>>(*this).find(k))));
    }

    const_iterator
    find(const_reference_key k) const
    {
        const auto hashed = Hash()(k) % M_buckets;
        auto index = hashed;
        for (; _hash(index) == hashed;)
        {
            if (_key(index) == k)
            {
                return const_iterator(M_file + index);
            }

            index = (index + 1) % M_buckets;
        }

        return cend();
    }

    /**
     * @brief For insert({x,y}) case.
     */
    std::pair<iterator, bool>
    insert(std::pair<Key, Value>&& v)
    {
        return emplace(v.first, v.second);
    }

    template<typename T, typename std::enable_if<std::is_lvalue_reference<T>::value, int>::type = 0>
    std::pair<iterator, bool>
    insert(T&& v)
    {
        return emplace(v.first, v.second);
    }

    template<typename T, typename std::enable_if<!std::is_lvalue_reference<T>::value, int>::type = 0>
    std::pair<iterator, bool>
    insert(T&& v)
    {
        return emplace(std::forward<decltype(v.first)>(v.first), std::forward<decltype(v.second)>(v.second));
    }

    template<typename Arg, typename... Args>
    std::pair<iterator, bool>
    emplace(Arg&& arg, Args&&... args)
    {
        // NOTE: must rehash and resize

        auto k = std::forward<Arg>(arg);
        const auto hashed = Hash()(k) % M_buckets;
        auto index = hashed;

        if (!is_free()(M_file + index))
        {
            for (; _hash(index) == hashed;)
            {
                if (_key(index) == k)
                {
                    return { iterator(M_file + index),false };
                }

                 index = (index + 1) % M_buckets;
            }

            if (!is_free()(M_file + index))
            {
                auto temp = index;
                for (; !is_free()(M_file + index);)
                {
                    index = (index + 1) % M_buckets;
                }
                for (; index != temp;)
                {
                    /*  NOTE: this actually lexigraphically constructs, need to change
                              block so that it assigns, not constructs
                              but do we actually want this ???
                    */
                    _block(index) = _block((index - 1) % M_buckets);
                    index = (index - 1) % M_buckets;
                }
            }
        }

        std::allocator_traits<allocator>::construct
        (
            M_alloc,
            std::addressof(_block(index)),
            hashed,
            std::forward<Arg>(arg),
            std::forward<Args>(args)...
        );
        
        ++M_elem;

        return { iterator(M_file + index),true };
    }

    template<typename T, typename U>
    std::pair<iterator, bool>
    insert_or_assign(T&& k, U&& val)
    {
        auto res = find(k);
        if (res == end())
        {
            return emplace(std::forward<T>(k), std::forward<U>(val));
        }

        // NOTE: should construct here ?
        //       like using allocator_traits
        // i think no since its "assignment"
        res->second = std::forward<U>(val);

        return { res,false };
    }

    /*  NOTE: do we actually erase ?
              ie call deconstruct ? or just leave as is ?
    */

    iterator
    erase(const_iterator iter)
    {
        auto index = iter_data(iter) - iter_data(cbegin());
        if (!is_free()(M_file + index))
        {
            _hash(index) = std::numeric_limits<size_type>::max();
            --M_elem;
        }

        if (empty())
        {
            return end();
        }

        return iterator(M_file + ((index + 1) % M_buckets));
    }

    size_type
    erase(const_reference_key k)
    {
        auto index = iter_data(find(k)) - iter_data(begin());
        if (index == M_buckets || is_free()(M_file + index))
        {
            return 0;
        }
        --M_elem;

        _hash(index) = std::numeric_limits<size_type>::max();

        return 1;
    }

    bool
    contains(const_reference_key k) const
    {
        return find(k) != cend();
    }

    bool
    empty() const
    {
        return M_elem == 0;
    }

    void
    clear()
    {
        for (size_type index = 0; index != M_buckets; ++index)
        {
            if (is_free()(M_file + index))
            {
                _hash(index) = std::numeric_limits<size_type>::max();
            }
        }

        M_elem = 0;
    }

private:

    const std::string M_name;
    size_type   M_buckets, M_elem;
    allocator   M_alloc;
    _blk*       M_file;

};

FILE_NAMESPACE_END

#endif
