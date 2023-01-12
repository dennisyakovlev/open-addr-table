#ifndef CUSTOM_FILE_LIBRARY_UNORDEREDMAPFILE
#define CUSTOM_FILE_LIBRARY_UNORDEREDMAPFILE

#include <limits>
#include <stdlib.h>
#include <time.h>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <unistd.h>
#include <vector>

#include <files/Allocators.h>
#include <files/Iterators.h>
#include <files/Defs.h>
#include "FileBlock.h"

/*  NOTE: use open addressing with linear probing

    account for iterator invalidation
*/

/*  NOTE: this shouldn't be part of this class should be own func

            and maybe named open_address_emplace or sum
*/

FILE_NAMESPACE_BEGIN

/**
 * @brief Increment with wrap around around mod.
 * 
 * @tparam Sz unsigned type
 * @param i num to increment
 * @param mod modulus
 * @return Sz a value in range [0,mod)
 */
template<
    typename Sz,
    typename std::enable_if<std::is_unsigned<Sz>::value, int>::type = 0>
void
increment_wrap(Sz& i, Sz mod)
{
    i = (i + mod + 1) % mod;
}

/**
 * @brief Decrememnt with wrap around around mod
 * 
 * @tparam Sz unsigned type
 * @param i num to decrement
 * @param mod modulus
 * @return Sz a value in range [0,mod)
 */
template<
    typename Sz,
    typename std::enable_if<std::is_unsigned<Sz>::value, int>::type = 0>
void
decrement_wrap(Sz& i, Sz mod)
{
    i = (i + mod - 1) % mod;
}

/**
 * @brief Open addressing with linear probing algorithm.
 * 
 * @note Last four template params are types which
 *       must have function operators that take the
 *       cont as the first parameter
 * 
 * @tparam Cont 
 * @tparam Arg key type
 * @tparam Sz container size_type to use
 * @tparam IsFree(index) true if the index be used written into
 *                       without overriding old data, otherwise false
 * @tparam HashComp(index, hashed) true if the hash value at index and hashed
 *                                 are equqal, false otherwise 
 * @tparam KeyComp(index, k) true if key at index a k compare equal, false
 *                           otherwise
 * @tparam ElemTransfer(into, from) put contents of index from into index into
 * @param cont container to manipulate
 * @param k key to insert
 * @param index begin searching from this index
 * @param buckets numbers of buckets in cont
 *                ie the mod applied to index
 * @return std::pair<Sz, bool> 
 */
template<
    typename Cont,
    typename Arg, typename Sz,
    typename IsFree, typename HashComp, typename KeyComp, typename ElemTransfer>
std::pair<Sz, bool>
open_address_emplace_index(Cont& cont, const Arg& k, Sz index, Sz buckets)
{
    const auto hashed  = index;
    if (!IsFree()(cont, index))
    {
        for (; HashComp()(cont, index, hashed);)
        {
            if (KeyComp()(cont, index, k))
            {
                return { index,false };
            }

            increment_wrap(index, buckets);
        }

        if (!IsFree()(cont, index))
        {
            auto temp = index;
            for (; !IsFree()(cont, index);)
            {
                increment_wrap(index, buckets);
            }
            for (; index != temp;)
            {
                auto decrement = index;
                decrement_wrap(decrement, buckets);
                ElemTransfer()(cont, index, decrement);
                decrement_wrap(index, buckets);
            }
        }
    }

    return  { index,true };
}

template<
    typename Cont,
    typename Sz,
    typename IsFree, typename ElemTransfer
>
std::pair<Sz, bool>
open_address_erase_index(Cont& cont, Sz index, Sz buckets)
{
    // this function would be called after the elem is deleted
    // then everything is moved over
}

/**
 * @brief 
 * 
 * @tparam Key 
 * @tparam Value 
 * @tparam Hash 
 */
template<
    typename Key,
    typename Value,
    typename Hash  = std::hash<Key>>
class unordered_map_file
{
public:

    using _blk = block<std::size_t, std::pair<Key, Value>>;
    using allocator = mmap_allocator<_blk>;

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
        bool operator()(const _blk* blk) const
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

    /**
     * @brief Not very good for many reasons, but
     *        good enough.
     */
    std::string
    _gen_random(const std::string::size_type len)
    {
        static const char alphanum[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        
        std::string tmp_s;
        tmp_s.reserve(len);
        ::srand(::time(nullptr));
        for (std::string::size_type i = 0; i != len; ++i)
        {
            tmp_s += alphanum[::rand() % (sizeof(alphanum) - 1)];
        }
        
        /*  NOTE: could prolly change this to be a global func in util or sum
        */
        while (::access(tmp_s.c_str(), F_OK) == 0)
        {
            ::srand(::time(nullptr) * tmp_s[0]);
            tmp_s.clear();

            for (std::string::size_type i = 0; i != len; ++i)
            {
                tmp_s += alphanum[::rand() % (sizeof(alphanum) - 1)];
            }
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
        return get<1>(_block(index)).first;
    }

    const_reference_key
    _key(size_type index) const
    {
        return get<1>(_block(index)).first;
    }

    void
    _init()
    {
        M_file = M_alloc.allocate(M_buckets);

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

    reference operator[](const Key& k)
    {
        /*  NOTE: implement
        */
    }

    const_reference operator[](const Key& k) const
    {

    }

    size_type
    size() const
    {
        return M_elem;
    }

    const_iterator
    cbegin() const
    {
        auto ptr = M_file;
        while (is_free()(ptr))
        {
            ++ptr;
        }

        return const_iterator(ptr);
    }

    iterator
    begin()
    {
        auto ptr = M_file;
        while (is_free()(ptr))
        {
            ++ptr;
        }

        return iterator(ptr);
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
        M_file  = M_alloc.reallocate(M_file, M_buckets, buckets);

        for (; M_buckets != buckets; ++M_buckets)
        {
            _hash(M_buckets) = std::numeric_limits<size_type>::max();
        }
    }

    void
    rehash(size_type buckets)
    {
        /*  (modded, hash, order of insertion)
            0 (2,22,4)
            1 (1,11,0)
            2 (2,2,1)
            3 (3,13,2)
            4 (3,9,3)

            0 (2,22,4)
            1 (1,11,0)
            2 (2,2,1)
            3 (3,13,2)
            4 (3,9,3)
            5
            6
            7
            8
            9

             0 1 2 3 4 5 6 7 8 9
            [n,1,0,2,3,n,n,n,n,4]
            i      is the new position
            arr[i] is the old position

            need to keep looping until find an empty spot
            once found empty spot, need to copy into it until unwinded loop
                use stack, but we need another array

            try to copy 0 -> 2, but is full. where does the old 2 go into new array ? (ie need our current array but inverse)
            copy 2 -> 3, but full
            copy 3 -> 4, but full
            copy 4 -> 9 okay, go back up

            push this all onto stack<pair<int,int>> and pop off to unwind


            (1) thing is we dont need second array if the stack will always contains a (a -> b -> b + 1 -> b + 2 -> ... -> b + n -> c) loop

            18*** what would happen ? since itd go into 3

             0  1  2  3  4  5  6  7  8  9 10 11 12 13 14      new position           ie indicies in the new hash table
            [n, n, n, n, n, 0, 7, 5, n, n, n, n, n, n, n]     old position (arr1)    ie indicies in the old hash table

             0  1  2  3  4  5  6  7  8  9 10 11 12 13 14      old position
            [5, n, n, n, n, 7, n, 6, n, n, n, n, n, n, n]     new position (arr2)

            arr1[5] = 0
            arr2[0] = 5

            (0,5),(5,7),(7,6)
            0,5,7,6     stack doesn't need to be pairs, can just be (top - 1) goes into (top)

            iterator through old positions
            if in use:
                follow the top of the stack, setting each arr2 to be invalid
                    
            when a pos is in use, then arr2[pos] will have a valid value






            0 (0,80)
            1
            2
            3
            4
            5 (5,37)
            6 
            7 (7,21)
            
            0
            1
            2
            3
            4
            5 (5,80)
            6 (6,21)
            7 (7,37)
            8
            9
            10
            11
            12
            13
            14

            0 -> 5
            5 -> 7
            7 -> 6


            (1) since we have a chain not like this, we need the stack and second array

            NOTE: reszing to something which would give similar mod values is a bad idea

             0 1 2 3 4 5 6 7 8,9
            [9,1,2,3,4,n,n,n,n,n]

            

            0 (4,14,3)
            1 (1,11,0)
            2 (2,2,1)
            3 (2,22,4)
            4 (3,13,2)

            0 
            1 (1,11)
            2 (2,22)
            3 (2,2)
            4 (3,13)
            5 
            6
            7
            8
            9 (9,9)

            now two issues
                - self loops ie stack would look like a,b,c,d,a or a,b,c,d,b
                    just set every element of arr to invalid whenever it gets added to the stack (see above)
                - how to know how to emplace
                    we want an emplace func which is like our current emplace just allows for generic container
                    and keep "is_free" to be generic. switch to use swap instead of =




            want to avoid copying the actual elements as much as possible, one copy per element

            - array which is same size as new number of buckets
            - insert into this array with same idea as emplace
            - iterate through the array copying elements in the container to their associated pos

            but if we overwrite element, need to "follow" the overwritten elements
        */

        /*  what if we use vector of _blk*, ie our data and just
            give special funcs ?
        */

        using Cont    = std::vector<std::pair<size_type, _blk*>>;
        using Cont_sz = typename Cont::size_type;

        /*  NOTE: should these be Cont_sz or size_type
        */

        struct IsFree
        {
            bool operator()(const Cont& c, Cont_sz i) const
            {
                return c[i].second == nullptr;
            }
        };

        struct HashComp
        {
            Cont_sz operator()(const Cont& c, Cont_sz i, Cont_sz hash) const
            {
                return get<0>(*c[i].second) == hash;
            }
        };

        struct KeyComp
        {
            Cont_sz operator()(const Cont& c, Cont_sz i, const Key& k) const
            {
                return get<1>(*c[i].second).first == k;
            }
        };

        struct ElemTransfer
        {
            void operator()(Cont& c, Cont_sz to, Cont_sz from)
            {
                return std::swap(c[to], c[from]);
            }
        };

        Cont vec(buckets, {0, nullptr});
        for (auto iter = cbegin(); iter != cend(); ++iter)
        {
            const auto data  = iter_data(iter);
            const auto index = data - M_file;
            vec[index] = 
            {
                open_address_emplace_index<
                    Cont,
                    Key, Cont_sz,
                    IsFree, HashComp, KeyComp, ElemTransfer
                >(vec, _key(index), index, buckets).first,
                data
            };
        }

        for (Cont_sz i = 0; i != vec.size(); ++i)
        {
            if (vec[i].second)
            {
                std::vector<Cont_sz> stack = { i };
                auto j = i;
                if (vec[j].first != j)
                {
                    for (; vec[j].second;)
                    {
                        stack.push_back(j);
                        vec[j].second = nullptr;
                        j = vec[j].first;
                    }
                }
                vec[j].second = nullptr;

                // NOTE: can use ElemTransfer defined in emplace for this
                while (!stack.empty())
                {
                    const auto temp = stack.back();
                    _block(j) = _block(temp);
                    j = temp;
                    stack.pop_back();
                }
            }
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

            // index = (index + 1) % M_buckets;
            increment_wrap(index, M_buckets);
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

        using testicle = unordered_map_file<Key, Value, Hash>;

        struct Is_Free_Test
        {
            size_type operator()(const testicle& cont, size_type index) const
            {
                return is_free()(cont.M_file + index);
            }
        };

        struct Hash_Comp_Test
        {
            size_type operator()(const testicle& cont, size_type index, size_type hash) const
            {
                return cont._hash(index) == hash;
            }
        };

        template<typename Uh>
        struct Key_Comp_Test
        {
            /*  NOTE: should this be a perfect forward ?
            */
            size_type operator()(const testicle& cont, size_type index, Uh k) const
            {
                return cont._key(index) == k;
            }
        };

        struct Elem_Move_Test
        {
            void operator()(testicle& cont, size_type to, size_type from)
            {
                cont._block(to) = cont._block(from); 
            }
        };

    template<typename Arg, typename... Args>
    std::pair<iterator, bool>
    emplace(Arg&& arg, Args&&... args)
    {
        // NOTE: must rehash and resize

        Arg k(std::forward<Arg>(arg));
        const auto hashed = Hash()(k) % M_buckets;

        auto res = open_address_emplace_index<
            decltype(*this),
            Arg, size_type,
            Is_Free_Test, Hash_Comp_Test, Key_Comp_Test<Arg>, Elem_Move_Test>
        (*this, k, hashed, M_buckets);

        if (!res.second)
        {
            return { iterator(M_file + res.first),false };
        }

        std::allocator_traits<allocator>::construct
        (
            M_alloc,
            std::addressof(_block(res.first)),
            hashed,
            std::make_pair(std::move(k), std::forward<Args>(args)...)
        );
 
        ++M_elem;

        return { iterator(M_file + res.first),true };
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

              yes should call deconstruct, since not our job to
              optimize that
    */

    /*  NOTE: erase is wrong, we need to move potenital elements back,
              just use the open_addr func

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

        increment_wrap(index, M_buckets);
        return iterator(M_file + index);
        // return iterator(M_file + ((index + 1) % M_buckets));
    }

    size_type
    erase(const_reference_key k)
    {
        /*  NOTE: why warning ?
        */
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

// private:

    const std::string M_name;
    size_type   M_buckets, M_elem;
    allocator   M_alloc;
    _blk*       M_file;

};

FILE_NAMESPACE_END

#endif
