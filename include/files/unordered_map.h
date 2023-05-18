#ifndef CUSTOM_FILE_LIBRARY_UNORDEREDMAPFILE
#define CUSTOM_FILE_LIBRARY_UNORDEREDMAPFILE

#include <array>
#include <initializer_list>
#include <limits>
#include <stdlib.h>
#include <time.h>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <unistd.h>
#include <vector>

#include "mmap_allocator.h"
#include "bidirectional_openaddr.h"
#include "defs.h"
#include "file_block.h"

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

/*  NOTE: add note about picking ideal_buckets to reserve / rehash to
*/

FILE_NAMESPACE_BEGIN

/**
 * @brief Check whether a type has member function matching
 *        unordered_map_file::destruct_is_wipe.
 *
 * @note Taken from https://stackoverflow.com/a/3627243/10162890, thank you
 *
 * @tparam Type type to check
 */
template <class Type>
class type_has_wipe
{
private:

    template <typename T, T>
    struct TypeCheck;

    typedef char Yes;
    typedef long No;

    template <typename T>
    struct wipe
    {
        typedef void (T::*fptr)(bool);
    };

    template <typename T>
    static Yes HasWipe(TypeCheck<typename wipe<T>::fptr, &T::destruct_is_wipe>*);
    template <typename T>
    static No  HasWipe(...);

public:

    static bool const value = (sizeof(HasWipe<Type>(0)) == sizeof(Yes));

};

template<typename T, bool has_wipe = false>
struct wipe_specialize
{
    static void call(T& t, bool b)
    {
    }
};

template<typename T>
struct wipe_specialize<T, true>
{
    static void call(T& t, bool b)
    {
        t.destruct_is_wipe(b);
    }
};

/**
 * @brief Call destruct_is_wipe on a type if it exists, other do nothing.
 *
 *        Point of this is to allow compatibilituy with other types when
 *        using an unordered_map_file with mmap_allocator and default file
 *        name. This would allow deletion of the file on disk when the
 *        map object is destructed, giving same behaviour as a normal map.
 *
 * @tparam T any type
 * @param t type to attempt to use
 * @param b see destruct_is_wipe for unordered_map_file
 */
template<typename T>
void destruct_is_wipe(T& t, bool b)
{
    wipe_specialize<T, type_has_wipe<T>::value>::call(t, b);
}

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
 * @brief Open address find algorithm.
 *
 * @tparam Container container type
 * @tparam Key key type
 * @tparam Sz container size type to use
 * @tparam IsFree(curr) true if the curr index be used written into
 *                      without overriding old data, otherwise false
 * @tparam HashComp(curr,against) comparison of modded hash values of curr
 *                                index with against index. return
 *                                0) curr < against
 *                                1) curr == against
 *                                2) curr > against
 * @tparam KeyComp(curr,k) true if key at curr index a k compare equal,
 *                         false otherwise
 * @tparam HashEq(curr,num) compairson of modded hash values of
 *                          curr index with number num
 *                          0) curr < num
 *                          1) curr = num
 *                          2) curr > num
 * @param cont container to search
 * @param k key to look for
 * @param key_hash key hash, will begin searching from (key_hash % buckets)
 * @param buckets numbers of buckets. assume this to equal to one plus the
 *                maximum valid index in cont (ie maximum number of elements)
 *                ie the wrap around value for iteration
 * @return std::pair<Sz, bool> Sz   - (A) found index of key
 *                                    (B) first free index at or after key_hash // NOTE this is wrong, should be first insertable / free
 *                             bool - (A) true if found
 *                                    (B) false if not found
 */
template<
    typename Container,
    typename Key, typename Sz,
    typename IsFree, typename HashComp, typename KeyComp,
    typename HashEq>
std::pair<Sz, bool>
open_address_find(const Container& cont, const Key& k, Sz key_hash, Sz buckets)
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

/**
 * @brief Open addressing insert algorithm. See \ref open_address_find for
 *        docs which would be redundant here.
 *
 * @tparam ElemTransfer(into,from) put contents of index "from" into index "into"
 * @return std::pair<Sz,bool> Sz   - (A) index of insertion
 *                                   (B) index of existing key
 *                            bool - (A) true if inserted
 *                                   (B) false if not inserted
 */
template<
    typename Container,
    typename Key, typename Sz,
    typename IsFree, typename HashComp, typename KeyComp, typename ElemTransfer,
    typename HashEq>
std::pair<Sz, bool>
open_address_emplace_index(Container& cont, const Key& k, Sz key_hash, Sz buckets)
{
    auto res = open_address_find<
        Container,
        Key, Sz,
        IsFree, HashComp, KeyComp,
        HashEq>
    (cont, k, key_hash, buckets);

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

    return { index,true };
}

/**
 * @brief Open addressing erase algorithm. See \ref open_address_find for
 *        docs which would be redundant here.
 *
 * @tparam Deconstruct(curr) destruct the element at index curr
 * @return Sz current index of removed element, buckets if no
 *            element was removed
 */
template<
    typename Container,
    typename Key, typename Sz,
    typename IsFree, typename HashComp, typename KeyComp, typename ElemTransfer,
    typename HashEq, typename Deconstruct>
Sz
open_address_erase_index(Container& cont, const Key& k, Sz key_hash, Sz buckets)
{
    auto res = open_address_find<
        Container,
        Key, Sz,
        IsFree, HashComp, KeyComp,
        HashEq>
    (cont, k, key_hash, buckets);

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
    auto next = index, start_index = index;
    increment_wrap(next, buckets);
    while (!IsFree()(cont, next) &&
           HashEq()(cont, next, next) != 1 &&
           next != start_index)
    {
        /*  Loop through all hashes which have the same
            modded hash value.
        */
        auto curr = index, start_same = next;
        while (!IsFree()(cont, next) &&
               HashComp()(cont, next, start_same) == 1 &&
               HashEq()(cont, next, next) != 1 &&
               next != start_index)
        {
            ElemTransfer()(cont, curr, next);

            curr = next;
            increment_wrap(next, buckets);
        }

        index = curr;
    }

    return index;
}

/* NOTE: want to be able to give custom allocator
*/

/*  NOTE: the template template param Allocator and potential issues

    https://stackoverflow.com/questions/38721618/why-does-stdstack-not-use-template-template-parameter
*/

template<
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    template<typename...> typename Allocator = mmap_allocator>
class unordered_map_file
{
public:

    /*  NOTE: we can use "const Key" because only "block" always construct
              and never copies.

        potential performance issue
    */
    using element = block<std::size_t, std::size_t, std::pair<const Key, Value>>;

    using allocator = Allocator<element>;

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

    struct access
    {

        access() = delete;

        access(element* ptr) :
            M_ptr(ptr)
        {
        }

        access(element* ptr, size_type buckets) :
            M_ptr(ptr),
            M_buckets(buckets)
        {
        }

        ~access() = default;

        element&
        block(size_type index)
        {
            return M_ptr[index];
        }

        const element&
        block(size_type index) const
        {
            return M_ptr[index];
        }

        /**
         * @brief set free state of index
         *
         * @param index
         * @param free true for free, false otherwise
         */
        void
        set_free(size_type index, bool free)
        {
            get<0>(block(index)) = free;
        }

        bool
        is_free(size_type index) const
        {
            return get<0>(block(index));
        }

        size_type&
        hash(size_type index)
        {
            return get<1>(block(index));
        }

        size_type
        hash(size_type index) const
        {
            return get<1>(block(index));
        }

        const_reference_key
        key(size_type index) const
        {
            return get<2>(block(index)).first;
        }

        reference
        value_type(size_type index)
        {
            return get<2>(block(index));
        }

        size_type
        buckets() const
        {
            return M_buckets;
        }

        element*  M_ptr;
        size_type M_buckets;
    };

    struct convert
    {
        pointer
        operator()(element* blk)
        {
            return static_cast<pointer>(std::addressof(get<2>(*blk)));
        }
    };

    struct is_free
    {
        bool
        operator()(access cont, size_type index) const
        {
            return cont.is_free(index);
        }
    };

    struct hash_comp
    {
        size_type
        operator()(access cont, size_type curr, size_type against) const
        {
            const auto modded_curr    = cont.hash(curr) % cont.buckets();
            const auto modded_against = cont.hash(against) % cont.buckets();

            return (modded_curr >= modded_against) * (1 + (modded_curr > modded_against));
        }
    };

    template<typename K>
    struct key_comp
    {
        /*  NOTE: should this be a perfect forward ?
        */
        bool
        operator()(access cont, size_type curr, K k) const
        {
            return cont.key(curr) == k;
        }
    };

    struct elem_move
    {
        void
        operator()(access cont, size_type to, size_type from)
        {
            cont.block(to) = cont.block(from); 
        }
    };

    struct hash_eq
    {
        size_type operator()(access cont, size_type curr, size_type num)
        {
            const auto modded_curr = cont.hash(curr) % cont.buckets();

            return (modded_curr >= num) * (1 + (modded_curr > num));
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

private:

    /**
     * @brief Get the next preferred size of the container. this
     *        can be larger or smaller. One of bucket choices
     *        will be picked using load factor and given buckets.
     * 
     * @param wanted_buckets Number of raw buckets wanted. This
     *                       num doesn't consider load factor or
     *                       anything else.
     * @param mlf max load factor
     * @param larger true if want next larger size, false for
     *               next smaller
     * @return size_type next preffered size if can. otherwise
     *                   minimum buckets accounted for load factor
     *                   or max_size() + 1 if invalid params
     */
    size_type
    next_size(size_type wanted_buckets, float mlf, bool larger)
    {
        /* NOTE: careful with the ciel and >= here,
                 think what i have is right
        */
        const auto max_sz = max_size();
        if (wanted_buckets > max_sz ||
            M_elem > max_sz * mlf ||
            wanted_buckets > max_sz * mlf)
        {
            return max_sz + 1;
        }

        size_type min_buckets = wanted_buckets / mlf;
        if (larger)
        {
            for (size_type potential : M_bucket_choices)
            {
                if (potential >= min_buckets)
                {
                    return potential;
                }
            }
        }
        else
        {
            for (auto iter = M_bucket_choices.crbegin();
                 iter != M_bucket_choices.crend(); ++iter)
            {
                if (*iter <= min_buckets)
                {
                    return *iter;
                }
            }
        }

        return min_buckets;
    }

    /**
     * @brief Reserves elements according to @ref next_size
     * 
     * @param buckets number of requested buckets
     * @param mlf max load factor
     * @param realloc true to reallocate existing memory, false to
     *                allocate new memory
     * @return true reserved more space
     * @return false failed to reserve space
     */
    bool
    reserve_choice(size_type buckets,
                   float mlf,
                   bool realloc,
                   bool preserve,
                   bool larger)
    {
        auto new_buckets = next_size(buckets, mlf, larger);
        if (new_buckets != max_size() + 1)
        {
            if (realloc)
            {
                M_file  = M_alloc.reallocate(M_file, M_buckets, new_buckets);
            }
            else
            {
                M_file  = M_alloc.allocate(new_buckets);
            }

            if (larger)
            {
                if (preserve)
                {
                    for (; M_buckets != new_buckets; ++M_buckets)
                    {
                        if (!access(M_file).is_free(M_buckets))
                        {
                            ++M_elem;
                        }
                    }
                }
                else
                {
                    for (; M_buckets != new_buckets; ++M_buckets)
                    {
                        access(M_file).set_free(M_buckets, true);
                    }
                }
            }
            else
            {
                M_buckets = new_buckets;
            }

            return true;
        }

        return false;
    }

    iterator
    make_iter(size_type index)
    {
        return iterator(M_file + index, M_file + M_buckets);
    }

    const_iterator
    make_iter(size_type index) const
    {
        return const_iterator(M_file + index, M_file + M_buckets);
    }

public:

    unordered_map_file() :
        M_buckets(0), M_elem(0),
        M_alloc(),
        M_delete(false),
        M_load(1)
    {
        reserve_choice(0, M_load, false, false, true);
    }

    unordered_map_file(size_type buckets) :
        M_buckets(0), M_elem(0),
        M_alloc(),
        M_delete(false),
        M_load(1)
    {
        reserve_choice(buckets, M_load, false, false, true);
    }

    unordered_map_file(std::string name) :
        M_buckets(0), M_elem(0),
        M_alloc(std::move(name)),
        M_delete(false),
        M_load(1)
    {
        reserve_choice(0, M_load, false, false, true);
    }

    unordered_map_file(std::string name, size_type buckets, bool preserve) :
        M_buckets(0), M_elem(0),
        M_alloc(std::move(name)),
        M_delete(false),
        M_load(1)
    {
        reserve_choice(buckets, M_load, false, true, true);
    }

    unordered_map_file(size_type buckets, std::string name) :
        M_buckets(0), M_elem(0),
        M_alloc(std::move(name)),
        M_delete(false),
        M_load(1)
    {
        reserve_choice(buckets, M_load, false, false, true);
    }

    unordered_map_file(
        size_type buckets,
        std::string name,
        std::initializer_list<size_type> choices = {}
    ) :
        M_buckets(0), M_elem(0),
        M_alloc(std::move(name)),
        M_delete(false),
        M_load(1)
    {
        if (choices.size())
        {
            bucket_choices(choices);
        }

        reserve_choice(buckets, M_load, false, false, true);
    }

    unordered_map_file(unordered_map_file&& rv) :
        M_buckets(rv.M_buckets), M_elem(rv.M_elem),
        M_alloc(rv.M_alloc),
        M_delete(rv.M_delete),
        M_load(rv.M_load),
        M_file(rv.M_file)
    {
        rv.M_file = nullptr;
    }

    ~unordered_map_file()
    {
        if (M_file)
        {
            std::allocator_traits<allocator>::deallocate
            (
                M_alloc,
                M_file,
                M_buckets
            );
        }

        if (M_delete)
        {
            M_alloc.wipe();
        }
    }

    Value& operator[](const Key& k)
    {
        auto iter = find(k);
        if (iter != end())
        {
            return iter->second;
        }

        return emplace(k, Value()).first->second;
    }

    Value& operator[](Key&& k)
    {
        auto iter = find(k);
        if (iter != end())
        {
            return iter->second;
        }

        return emplace(k, Value()).first->second;
    }

    size_type
    size() const
    {
        return M_elem;
    }

    const_iterator
    cbegin() const
    {
        auto iter = make_iter(0);
        if (access(M_file, M_buckets).is_free(0))
        {
            return ++iter;
        }

        return iter;
    }

    iterator
    begin()
    {
        /*  NOTE: this is not constant time, should be
        */

        auto iter = make_iter(0);
        if (access(M_file, M_buckets).is_free(0))
        {
            return ++iter;
        }

        return iter;
    }

    const_iterator
    cend() const
    {
        return make_iter(M_buckets);
    }

    iterator
    end()
    {
        return make_iter(M_buckets);
    }

    void
    reserve(size_type buckets)
    {
        rehash(buckets);
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

        /*  Can use this containers size type since 
                size_type -> ptrdiff_t
            is okay and
                ptrdiff_t -> Cont::size_type
            is also okay. So
                size_type -> Cont::size_type
            if okay
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
                const auto orig_elem_ptr = cont[curr].second;

                if (!orig_elem_ptr || !cont[against].second)
                {
                    return 2;
                }

                return hash_comp()(
                    {cont.back().second, cont.back().first},
                    orig_elem_ptr - cont.back().second, cont[against].second - cont.back().second
                );
            }
        };

        struct local_key_comp
        {
            size_type
            operator()(const local_cont& cont, size_type curr, const Key& k) const
            {
                const auto orig_elem_ptr = cont[curr].second;

                if (!orig_elem_ptr)
                {
                    return false;
                }

                return key_comp<Key>()(
                    {cont.back().second, cont.back().first},
                    orig_elem_ptr - cont.back().second, k
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

                std::swap(cont[to].second, cont[from].second);
            }
        };

        struct bruh_hash_eq
        {
            size_type
            operator()(const local_cont& cont, size_type curr, size_type num)
            {
                const auto orig_elem_ptr = cont[curr].second;

                return hash_eq()(
                    {cont.back().second, cont.back().first},
                    orig_elem_ptr - cont.back().second, num
                );
            }
        };

        const auto new_buckets = next_size(buckets, M_load, buckets > M_buckets);

        if (new_buckets == M_buckets)
        {
            return;
        }

        constexpr auto invalid_index = std::numeric_limits<size_type>::max();
        local_cont vec(
            std::max(new_buckets, M_buckets) + 1,
            {invalid_index, nullptr}
        );
        /*  Keep reference to some data to be used in functors.
        */
        vec.back() = { new_buckets,M_file };

        /*  Insert pointers to all taken elements from the
            current containers into temporary container.
        */
        for (auto iter = cbegin(); iter != cend(); ++iter)
        {
            const auto data       = iter_data(iter);
            const size_type index = data - M_file;

            access temp(M_file);
            auto now_taken = open_address_emplace_index<
                local_cont,
                Key, size_type,
                local_is_free, local_hash_comp, local_key_comp, local_elem_transfer,
                bruh_hash_eq>
            (vec, temp.key(index), temp.hash(index), new_buckets).first;

            vec[index].first      = now_taken;
            vec[now_taken].second = M_file + index;
        }

        const size_type loop_to = M_buckets;
        if (new_buckets > M_buckets)
        {
            reserve_choice(new_buckets, M_load, true, false, true);
        }

        /*  NOTE: document
        */

        std::vector<size_type> stack;
        stack.reserve(4);
        for (size_type index = 0; index != loop_to; ++index)
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

                while (stack.size() > 1)
                {
                    auto end = stack.rbegin();
                    elem_move()(M_file, *end, *(end + 1));
                    access(M_file).set_free(*(end + 1), true);
                    stack.pop_back();
                }
                stack.pop_back();
            }
        }

        if (new_buckets < M_buckets)
        {
            reserve_choice(new_buckets, M_load, true, false, false);
        }
    }

    iterator
    find(const_reference_key k)
    {
        access temp(M_file, M_buckets);
        const auto res = open_address_find<
            access,
            key_type, size_type,
            is_free, hash_comp, key_comp<key_type>,
            hash_eq>
        (temp, k, Hash()(k), M_buckets);

        if (!res.second)
        {
            return make_iter(M_buckets);
        }

        return make_iter(res.first);
    }

    const_iterator
    find(const_reference_key k) const
    {
        access temp(M_file, M_buckets);
        const auto res = open_address_find<
            access,
            key_type, size_type,
            is_free, hash_comp, key_comp<key_type>,
            hash_eq>
        (temp, k, Hash()(k), M_buckets);

        if (!res.second)
        {
            return make_iter(M_buckets);
        }

        return make_iter(res.first);
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
        Key k(std::forward<Arg>(arg));
        const auto hashed = Hash()(k);

        if (M_elem == M_buckets)
        {
            reserve_choice(M_buckets + 1, M_load, true, false, true);
        }

        access temp(M_file, M_buckets);
        const auto res = open_address_emplace_index<
            access,
            Key, size_type,
            is_free, hash_comp, key_comp<Key>, elem_move,
            hash_eq>
        (temp, k, hashed, M_buckets);

        if (!res.second)
        {
            return { make_iter(res.first),false };
        }

        std::allocator_traits<allocator>::construct
        (
            M_alloc,
            M_file + res.first,
            false,
            hashed,
            std::make_pair(std::move(k), std::forward<Args>(args)...)
        );
 
        ++M_elem;

        return { make_iter(res.first),true };
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
        erase(access(M_file).key(index));

        if (empty())
        {
            return end();
        }

        return make_iter(index);
    }

    size_type
    erase(const_reference_key k)
    {
        struct local_deconstruct
        {
            void operator()(access cont, size_type curr)
            {
                using alloc = std::allocator<value_type>;
                alloc a;
                std::allocator_traits<alloc>::destroy
                (
                    a,
                    std::addressof(cont.value_type(curr))
                );
            }
        };

        access temp(M_file, M_buckets);
        auto res = open_address_erase_index<
            access,
            key_type, size_type,
            is_free, hash_comp, key_comp<key_type>, elem_move,
            hash_eq, local_deconstruct>
        (temp, k, Hash()(k), M_buckets);

        if (res == M_buckets)
        {
            return 0;
        }

        access(M_file).set_free(res, true);
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
            access(M_file).set_free(index, true);
        }

        M_elem = 0;
    }

    size_type
    max_size() const
    {
        return std::numeric_limits<size_type>::max() / sizeof(element);
    }

    size_type
    bucket_count() const
    {
        return M_buckets;
    }

    size_type
    max_bucket_count() const
    {
        return max_size();
    }

    size_type
    bucket_size(size_type index) const
    {
        return !is_free()(M_file, index);
    }

    size_type
    bucket(const_reference_key k) const
    {
        return Hash()(k) % M_buckets;
    }

    float
    load_factor() const
    {
        return M_elem / static_cast<float>(M_buckets);
    }

    float
    max_load_factor() const
    {
        return M_load;
    }

    void
    max_load_factor(float mzlf)
    {
        /*  NOTE: remains unimplemented
        */
    }

    /**
     * @brief Decide whether on destruct it is necessary to delete
     *        the associated file on disk.
     *
     * @param b true for yes, false for no. If false the data on
     *          disk will not be bound by the lifetime of the object.
     */
    void
    destruct_is_wipe(bool b)
    {
        M_delete = b;
    }

    /**
     * @brief The choices of the number of buckets the
     *        conatienr will allocate.
     * 
     * @return const std::vector<std::size_t>& 
     */
    const std::vector<std::size_t>&
    bucket_choices() const
    {
        return M_bucket_choices;
    }

    /**
     * @brief Sets the choices for number of buckets
     *        to use. Requirements are
     *          1) The minimum element of choices
     *             must be greater than 0.
     *          2) Elements must be increasing from
     *             beginning to end.
     * 
     * @tparam T container which can be copy assigned 
     *           to std::vector<std::size_t>
     * @param choices values to set as
     */
    template<typename T>
    void
    bucket_choices(T&& choices)
    {
        M_bucket_choices = std::forward<T>(choices);
    }

    /**
     * @brief See other bucket_choices
     */
    void
    bucket_choices(std::initializer_list<size_type> choices)
    {
        M_bucket_choices = choices;
    }

private:

    size_type         M_buckets, M_elem;
    allocator         M_alloc;
    bool              M_delete;
    float             M_load;
    element*          M_file;
    std::vector<std::size_t> M_bucket_choices =
    {
        1,
        7,
        17,
        73,
        181,
        431,
        1777,
        4721,
        10253,
        41017,
        140989, 
        487757,
        1028957
    };

};

FILE_NAMESPACE_END

#endif
