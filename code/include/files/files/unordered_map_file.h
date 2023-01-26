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

/*  Discussion on ptrdiff_t vs size_t and why we can
    neglect it here.

    size_t
        Is the maximum possible size of a contiguous array.
    ptrdiff_t
        Is used on pointer arithmetic within the same
        contiguous array. It can represent the distance
        between any two indicies of the same array.

    With most implementations we will have
        max(size_t) / 2 = max(ptrdiff_t)                  (1)
    However, this is not gaurenteed.
    
    We know the minimum size of an element in the table is
        sizeof(size_t) + sizeof(Key) + sizeof(Value)
    which is >= 3. So maximum possible elements is
        max(size_t) / 3 > max(size_t) / 2 = max(ptrdiff_t)
    That is, max(ptrdiff_t) will be able to correctly
    represent any distance in the table when (1) holds.

    What about when (1) isn't true?

        """
        If an array is so large (greater than PTRDIFF_MAX
        elements, but less than SIZE_MAX bytes), that the
        difference between two pointers may not b
        representable as std::ptrdiff_t, the result of
        subtracting two such pointers is undefined.
        """
        https://en.cppreference.com/w/cpp/types/ptrdiff_t

    In conlusion, throughout the container it is okay
    to convert "size_type" (size_t) to "difference_type"
    (ptrdiff_t) and vice versa ONLY for indexing purposes.
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
 *        When collisions occur, the modded hashes of the
 *        keys will be in non-decreasing order.
 * 
 * @note Last four template params are types which
 *       must have function operators that take the
 *       cont as the first parameter
 * 
 * @tparam Cont 
 * @tparam Arg key type
 * @tparam Sz container size_type to use
 * @tparam IsFree(curr) true if the curr index be used written into
 *                      without overriding old data, otherwise false
 * @tparam HashComp(curr,against) comparison of modded hash values of curr
 *                                index with against index. return
 *                                0) curr < against
 *                                1) curr == against
 *                                2) curr > against
 * @tparam KeyComp(curr,k) true if key at curr index a k compare equal,
 *                         false otherwise
 * @tparam ElemTransfer(into,from) put contents of index from into index into
 * @tparam HashEq(curr,num) compairson of modded hash values of
 *                          curr index with number num
 *                          0) curr < num
 *                          1) curr = num
 *                          2) curr > num
 * @param cont container to manipulate
 * @param k key to insert
 * @param key_hash key hash, will begin searching from this index mod buckets
 * @param buckets numbers of buckets. assume this to equal to one plus the
 *                maximum valid index in cont (ie maximum number of elements)
 *                ie the wrap around value for iteration
 * @return std::pair<Sz,bool> Sz   - index of insertion
 *                            bool - true if inserted, false otherwise
 */
template<
    typename Cont,
    typename Arg, typename Sz,
    typename IsFree, typename HashComp, typename KeyComp, typename ElemTransfer,
    typename HashEq>
std::pair<Sz, bool>
open_address_emplace_index(Cont& cont, const Arg& k, Sz key_hash, Sz buckets)
{
    /*  The key is to make distinction between past the end collison
        and normal collison
    */

    /*  split into current index > than key_hash
        and < key_hash
    */

    // while(HashEq()(cont, index, key_hash) == 2)
    // {
    //     increment_wrap(index, buckets);
    // }

    /*  NOTE: tbh can use find here, assuming find returns the
              last possible valid index
    */

    auto res = open_address_find<
        Cont,
        Arg, Sz,
        IsFree, HashComp, KeyComp,
        HashEq
    >(cont, k, key_hash, buckets);

    auto index = res.first;

    if (res.second)
    {
        return { index,false };
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

    return  { index,true };
}

/**
 * @brief See \ref open_address_emplace_index
 *
 * @tparam Deconstruct(curr) destruct the element at index curr
 * @return Sz current index of removed element, buckets if no
 *            element was removed
 */
template<
    typename Cont,
    typename Arg, typename Sz,
    typename IsFree, typename HashComp, typename KeyComp, typename ElemTransfer,
    typename HashEq, typename Deconstruct>
Sz
open_address_erase_index(Cont& cont, const Arg& k, Sz key_hash, Sz buckets)
{
    auto res = open_address_find<
        Cont,
        Arg, Sz,
        IsFree, HashComp, KeyComp,
        HashEq
    >(cont, k, key_hash, buckets);

    if (!res.second)
    {
        return buckets;
    }

    auto index = res.first;

    Deconstruct()(cont, index);

    /*  NOTE: optimization to be used here, when we transfer elements back
              we do unecessary copies

        0
        1 1
        2 1   <- erase this guy
        3 2
        4 2
        
        now have this, we could just take index 4, and put into index 2
        0
        1 1
        2
        3 2
        4 2

        instead we move 3 -> 2 and 4 -> 3, extra ElemTransfer

    */

    /*  Determine whether to loop through a set of
        hashes.
    */
    auto next = index;
    increment_wrap(next, buckets);
    while (!IsFree()(cont, next) &&
           HashEq()(cont, next, next) != 1)
    {
        /*  Loop through all hashes which have the same
            modded hash value.
        */
        auto curr = index, start_same = next;
        // start_same issue, index or next
        //  thought about next, but idk didnt go with it
        while (!IsFree()(cont, next) &&
               HashComp()(cont, next, start_same) == 1)
        {
            ElemTransfer()(cont, curr, next);

            curr = next;
            increment_wrap(next, buckets);
        }

        index = curr;
    }

    return index;
}

/**
 * @brief See \ref open_address_erase_index
 * NOTE: update doc below 
 * @return Sz index of element in cont whose key compares equal
 *            to k, buckets otherwise
 */
template<
    typename Cont,
    typename Arg, typename Sz,
    typename IsFree, typename HashComp, typename KeyComp,
    typename HashEq>
std::pair<Sz, bool>
open_address_find(const Cont& cont, const Arg& k, Sz key_hash, Sz buckets)
{   
    auto index    = key_hash % buckets;
    bool iterated = false;

    /*  Collision overflow past end. Will not find the key
        here. Loop to the end of overflow.
    */
    while (!IsFree()(cont, index) &&
           HashEq()(cont, index, index) == 2)
    {
        increment_wrap(index, buckets);
        iterated = true;
    }

    /*  There can be no match for key "k" if end of
        overflow results in
            - free element
            - the starting index of the overflow
    */
    if (IsFree()(cont, index) ||
        (iterated && index == (key_hash % buckets)))
    {
        return { index,false };
    }

    /*  Can now search collision normally.
    */

    /*  The modded hash value form a non-decreasing
        sequence. Keep looping until find index whose
        modded hash value is greater than or equal to
        modded key hash.
    */
    Sz iterations = 0;
    while (!IsFree()(cont, index) &&
           HashEq()(cont, index, key_hash % buckets) == 0 &&
           iterations != buckets)
    {
        increment_wrap(index, buckets);
        ++iterations;
    }

    /*  There can be no match for key "k" if landed on
            - free element
            - the modded by buckets hash value at index
              os greater than the modded key hash. means
              skipped over potential indicies where key
              could exist
            - same element as started at
    */
    if (IsFree()(cont, index) ||
        HashEq()(cont, index, key_hash % buckets) == 2 ||
        iterations == buckets)
    {
        return { index,false };
    }

    /*  Loop through set of hashes which have the same
        modded value as key hash.
    */
    const auto start_same = index;
    iterations = 0;
    while (!IsFree()(cont, index) &&
           HashComp()(cont, index, start_same) == 1 &&
           iterations != buckets)
    {
        if (KeyComp()(cont, index, k))
        {
            return { index,true };
        }

        increment_wrap(index, buckets);
        ++iterations;
    }

    return { index, false };
}

template<
    typename Key,
    typename Value,
    typename Hash  = std::hash<Key>>
class unordered_map_file
{
public:

    /*  NOTE: we can use "const Key" because only "block" always construct
              and never copies.

        potential performance issue
    */
    using element = block<std::size_t, std::size_t, std::pair<const Key, Value>>;
    // using element = block<bool, std::size_t, std::pair<Key, Value>>;

    using allocator = mmap_allocator<element>;

    using value_type      = std::pair<const Key, Value>;
    using reference       = std::pair<const Key, Value>&;
    using const_reference = const std::pair<const Key, Value>&;
    using pointer         = std::pair<const Key, Value>*;
    using const_pointer   = const std::pair<const Key, Value>*;
    using size_type       = std::size_t;

    using key_type            = Key;
    using reference_key       = Key&;
    using const_reference_key = const Key&;
    using pointer_key         = Key*;
    using const_pointer_key   = const Key*;
    using mapped_type         = Value;

    struct convert
    {
        pointer
        operator()(element* blk)
        {
            return static_cast<pointer>(std::addressof(get<2>(*blk)));
        }
    };

    using testicle = unordered_map_file<Key, Value, Hash>;

    struct is_free
    {
        bool
        operator()(const testicle& cont, size_type index) const
        {
            return get<0>(cont._block(index));
        }
    };

    struct hash_comp
    {
        size_type
        operator()(const testicle& cont, size_type curr, size_type against) const
        {
            const auto modded_curr    = cont._hash(curr) % cont.M_buckets;
            const auto modded_against = cont._hash(against) % cont.M_buckets;

            return (modded_curr >= modded_against) * (1 + (modded_curr > modded_against));
        }
    };

    template<typename K>
    struct key_comp
    {
        /*  NOTE: should this be a perfect forward ?
        */
        bool
        operator()(const testicle& cont, size_type curr, K k) const
        {
            return cont._key(curr) == k;
        }
    };

    struct Elem_Move_Test
    {
        void
        operator()(testicle& cont, size_type to, size_type from)
        {
            cont._block(to) = cont._block(from); 
        }
    };

    struct is_free_iter
    {
        bool operator()(element* ptr)
        {
            return get<0>(*ptr);
        }
    };

    using iterator       = bidirectional_openaddr<
        std::pair<const Key, Value>,
        element,
        unordered_map_file<Key, Value, Hash>,
        convert, is_free_iter>;
    using const_iterator = bidirectional_openaddr<
        const std::pair<const Key, Value>,
        element,
        unordered_map_file<Key, Value, Hash>,
        convert, is_free_iter>;
    using difference_type = typename iterator::difference_type;

    struct local_hash_eq
    {
        size_type operator()(const testicle& cont, size_type curr, size_type num)
        {
            const auto modded_curr = cont._hash(curr) % cont.M_buckets;

            return (modded_curr >= num) * (1 + (modded_curr > num));
        }
    };

private:

    /**
     * @brief For use in the struct functors
     * 
     * @param ptr 
     */
    unordered_map_file(element* ptr, size_type buckets)
        : M_buckets(buckets),
          M_file(ptr)  
    {
    }

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

    element&
    _block(size_type index)
    {
        return *(M_file + index);
    }

    const element&
    _block(size_type index) const
    {
        return *(M_file + index);
    }

    size_type&
    _is_free(size_type index)
    {
        return get<0>(_block(index));
    }

    size_type
    _is_free(size_type index) const
    {
        return get<0>(_block(index));
    }

    size_type&
    _hash(size_type index)
    {
        return get<1>(_block(index));
    }

    size_type
    _hash(size_type index) const
    {
        return get<1>(_block(index));
    }

    reference
    obtain_value_type(size_type index)
    {
        return get<2>(_block(index));
    }

    const_reference
    obtain_value_type(size_type index) const
    {
        return get<2>(_block(index));
    }

    const_reference_key
    _key(size_type index) const
    {
        return get<2>(_block(index)).first;
    }

    void
    _init()
    {
        M_file = M_alloc.allocate(M_buckets);

        for (size_type i = 0; i != M_buckets; ++i)
        {
            _is_free(i) = true;
        }
    }

public:

    unordered_map_file()
        : M_name(_gen_random(16)),
          M_buckets(1), M_elem(0),
          M_alloc(M_name)
    {
        _init();
    }

    unordered_map_file(std::string name)
        : M_name(std::move(name)),
          M_buckets(1), M_elem(0),
          M_alloc(M_name)
    {
        _init();
    }

    /*  NOTE: buckets must be > 0 other mmap will
              freak out
    */
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
        size_type index = 0;
        while (is_free()(*this, index) &&
               index != M_buckets)
        {
            ++index;
        }

        return const_iterator(M_file + index);
    }

    iterator
    begin()
    {
        size_type index = 0;
        while (is_free()(*this, index) &&
               index != M_buckets)
        {
            ++index;
        }

        return iterator(M_file + index);
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
            _is_free(M_buckets) = true;
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
            [4,1,2,3,5,n,n,n,n,n]

            0 (4,14,3)
            1 (1,11,0)
            2 (2,2,1)
            3 (2,22,4)
            4 (3,13,2)

            0 
            1 (1,11)
            2 (2,2)
            3 (2,22)
            4 (3,13)
            5 (4,14)
            6
            7
            8
            9

             0 1 2 3 4 5 6 7 8 9
            [4,1,2,3,5,n,n,n,n,n]
            [n,1,2,3,0,4,n,n,n,n]

            now issue is when i insert (3,13), i move back (4,14), i must update
            the element in vector which has element.first = 4, it should be 5
                - use the pointer at the element we're moving will tell us
                  where to look
                - but we somehow need to know which element to access based of ptr
                - say we can have at end of vector an element which contains the
                  starting pointer and can subtract that to know which element
                  needs to be changed



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





            0 (0,0)
            1 (1,11)
            2 (2,12)  <- here stack should be 2,4,1 ie 4 copied to 1, 2 copied to 4
            3
            4 (4,9)

            0 (0,0)
            1 (1,9)
            2
            3 (3,11)
            4 (4,12)
            5
            6
            7

        */

        /*  v[i].first  - index i referencing a location in the current
                          mmap will be placed into the v[i].first location,
                          which references a location in the new mmap
            v[i].second - index i referencing a location in the new
                          mmap will be taken by the v[i].second pointer
                          which points to a valid element of the current
                          mmap
        */
        using local_cont = std::vector<std::pair<size_type, element*>>;

        /*  Can use this containers size type since 
                size_type -> ptrdiff_t
            is okay and
                ptrdiff_t -> Cont::size_type
            is also okay. So
                size_type -> Cont::size_type
            if okay
        */

        struct local_is_free
        {
            bool
            operator()(const local_cont& cont, size_type curr) const
            {
                return cont[curr].second == nullptr;
            }
        };

        struct local_hash_comp
        {
            size_type
            operator()(const local_cont& cont, size_type curr, size_type against) const
            {
                if (!cont[curr].second)
                {
                    return 2;
                }

                return hash_comp()(
                    {cont.back().second, cont.back().first},
                    curr, against
                );
            }
        };

        struct local_key_comp
        {
            size_type
            operator()(const local_cont& cont, size_type curr, const Key& k) const
            {
                // Assume KeyComp does not compare against a nullptr.
                return key_comp<Key>()(
                    {cont.back().second, cont.back().first},
                    curr, k
                );
            }
        };

        struct local_elem_transfer
        {
            void
            operator()(local_cont& cont, size_type to, size_type from)
            {
                const auto start = cont.back().second;
                cont[cont[from].second - start].first = to;

                std::swap(cont[to], cont[from]);
            }
        };

        struct bruh_hash_eq
        {
            size_type
            operator()(const local_cont& cont, size_type curr, size_type num)
            {
                return local_hash_eq()(
                    {cont.back().second, cont.back().first},
                    curr, num
                );
            }
        };

        constexpr auto invalid_index = std::numeric_limits<size_type>::max();
        local_cont vec(
            std::max(buckets, M_buckets) + 1,
            {invalid_index, nullptr}
        );
        /*  Keep reference to some data to be used in functors.
        */
        vec.back() = { buckets,M_file };

        /*  Insert pointers to all taken elements from the
            current containers into temporary container.
        */
        for (auto iter = cbegin(); iter != cend(); ++iter)
        {
            const auto data       = iter_data(iter);
            const size_type index = data - M_file;

            auto now_taken = open_address_emplace_index<
                local_cont,
                Key, size_type,
                local_is_free, local_hash_comp, local_key_comp, local_elem_transfer,
                bruh_hash_eq
            >(vec, _key(index), _hash(index), buckets).first;

            vec[index].first      = now_taken;
            vec[now_taken].second = M_file + index;
        }

        if (buckets > M_buckets)
        {
            reserve(buckets);
        }

        /*  NOTE: document
        */

        std::vector<size_type> stack;
        stack.reserve(4);
        for (size_type index = 0; index != M_buckets; ++index)
        {
            auto going_to = vec[index].first;
            /*  If an index is moved to itself can do nothing.
            */
            if (going_to != invalid_index && going_to != index)
            {
                auto prev = index;
                stack.push_back(prev);

                while (going_to != invalid_index)
                {
                    /*  Need to invalidate to account for
                        self loops. 
                    */
                    vec[prev].first = invalid_index;

                    stack.push_back(going_to);
                    prev = going_to;
                    going_to = vec[going_to].first;
                }

                while (stack.size() > 1 )
                {
                    auto end = stack.rbegin();
                    Elem_Move_Test()(*this, *end, *(end + 1));
                    stack.pop_back();
                }
                stack.pop_back();
            }
        }

        if (buckets < M_buckets)
        {
            reserve(buckets);
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
        const auto res = open_address_find<
            decltype(*this),
            key_type, size_type,
            is_free, hash_comp, key_comp<key_type>,
            local_hash_eq
        >(*this, k, Hash()(k), M_buckets);

        if (!res.second)
        {
            return const_iterator(M_file + M_buckets);
        }

        return const_iterator(M_file + res.first);
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

        Arg k(std::forward<Arg>(arg));
        const auto hashed = Hash()(k);

        auto res = open_address_emplace_index<
            decltype(*this),
            Arg, size_type,
            is_free, hash_comp, key_comp<Arg>, Elem_Move_Test,
            local_hash_eq
        >(*this, k, hashed, M_buckets);

        if (!res.second)
        {
            return { iterator(M_file + res.first),false };
        }

        std::allocator_traits<allocator>::construct
        (
            M_alloc,
            std::addressof(_block(res.first)),
            false,
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
 
        res->second = std::forward<U>(val);

        return { res,false };
    }

    iterator
    erase(const_iterator iter)
    {
        size_type index = iter_data(iter) - M_file;
        erase(_key(iter_data(iter) - M_file));

        if (empty())
        {
            return end();
        }

        return iterator(M_file + index);
    }

    size_type
    erase(const_reference_key k)
    {
        struct local_deconstruct
        {
            void operator()(testicle& cont, size_type curr)
            {
                /*  NOTE: this is wrong, we dont deconstruct the block,
                          deconstuct the value_type ONLY
                */
                using alloc = std::allocator<value_type>;
                alloc a;
                std::allocator_traits<alloc>::destroy
                (
                    a,
                    std::addressof(cont.obtain_value_type(curr))
                );
            }
        };

        auto res = open_address_erase_index<
            decltype(*this),
            key_type, size_type,
            is_free, hash_comp, key_comp<key_type>, Elem_Move_Test,
            local_hash_eq, local_deconstruct>
        (*this, k, Hash()(k), M_buckets);

        if (res == M_buckets)
        {
            return 0;
        }

        _is_free(res) = true;
        --M_elem;

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
            _is_free(index) = true;
        }

        M_elem = 0;
    }

// private:

    const std::string M_name;
    size_type         M_buckets, M_elem;
    allocator         M_alloc;
    element*          M_file;

};

FILE_NAMESPACE_END

#endif
