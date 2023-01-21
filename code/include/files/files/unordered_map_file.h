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
    typename IsFree, typename HashComp, typename KeyComp, typename ElemTransfer>
std::pair<Sz, bool>
open_address_emplace_index(Cont& cont, const Arg& k, Sz key_hash, Sz buckets)
{
    /*  what if we try to insert an item into an already chained sequence

        say somthing like

        0
        1
        2 I
        3 I
        4 I
        5

        where I all have the same hash of 2, and we try to insert at 3 or 4

        what should happen is the thing we try to insert gets put AFTER all of these
            right now we move everything down for this, which we DONT want to do

        what about and we try to insert 4
        0
        1
        2 2
        3 2
        4 2 
        5 2
        6 3
        7 3
        8
        9

        insert 4
        0
        1
        2 2
        3 2
        4 2   <- start here 
        5 3
        6 5   <- insert 4 here
        7 5
        8 
        9

    */

    auto index = key_hash % buckets;

    /*  NOTE: i dont think we need this if, would prolly be pretty
              efficient to just remove ths
    */
    if (!IsFree()(cont, index))
    {
        /*  NOTE: need to account for if there a table full of the
                  same index

            or do i? do we ALWAYS assume there is ATLEAST one space available ?
                i think yes
                since max_load_factor is 1 at most, we'd just rehash if there
                was no space

            conslusion, do not need to account for full table
        */

        /*  NOTE: optimization
                  right now when we insert with collision, we insert into
                  first "correct" place, if we iterate until the end
                    ie go as far as possible in the collision
                  would have to copy less
        */

        const auto start_index = index;
        auto hash_comp_res = HashComp()(cont, index, start_index);
        for (; !IsFree()(cont, index) && hash_comp_res < 2;)
        {
            /*  For key A,B know (A equals B) => (hash(A) == hash(b))

                Can optimize to not do unecessary key compare when
                key hashes unequal.
            */
            if (hash_comp_res == 1 && KeyComp()(cont, index, k))
            {
                return { index,false };
            }

            increment_wrap(index, buckets);
            hash_comp_res = HashComp()(cont, index, start_index);
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

/*  what if we have table of all same hashes, will iterate infinetly
*/

/**
 * @brief See \ref open_address_emplace_index
 *
 * @tparam HashEq(curr,num) compairson of modded hash values of
 *                          curr index with number num
 *                          0) curr < num
 *                          1) curr = num
 *                          2) curr > num
 * @tparam Deconstruct(curr) destruct the element at index curr
 * @return Sz index of removed element
 */
template<
    typename Cont,
    typename Arg, typename Sz,
    typename IsFree, typename HashComp, typename KeyComp, typename ElemTransfer,
    typename HashEq, typename Deconstruct>
Sz
open_address_erase_index(Cont& cont, const Arg& k, Sz key_hash, Sz buckets)
{
    /*  deconstruct in here or in funcs ?
            in funcs

        original func deallocs the pair but keeps hash
            not necisarily, so long as hashcomp does what we need

        if original func does NOT dealloc we'd return pair<sz,bool>
            we never need sz, we cannot return next iter, would be O(n) time

        we might still want sz, say we want sz where sz index which
        would be removed and would be buckets if nothing to remove
            - but we want to dealloc, how would we do this if we returned index
            - say like we have have a chain, would move the chain back, but
              then would just replace the current elem, and not dealloc it

            conclusion is we MUST dealloc the elem before manipulating
            the rest of the container
            question becomes in the original func or here

        dealloc in here
            - need dealloc()(cont, index)
            - caller original loses control
        dealloc in original func
            - forced to check whether to dealloc or not
        i think the point in dealloc in func is the decider, this puts logic of
        erase into the original caller, which i dont want

        for insert above is okay because conditional logic is not added,
        theyre simply given an index to shove elem into, and do as told

        dealloc within the func
        return sz
            sz is buckets if nothing deleted
            sz is the index of deleted
        


    */

    auto index = key_hash % buckets;

    /*  Terminate the loop when
            - there is a free element
            - hash value at index modded by buckets
              is greater than or equal to the value
              of the initial index to search at 
            - looped through all the buckets
    */
    const auto orig_index = index;
    Sz i = 0;
    while (!IsFree()(cont, index) &&
           HashEq()(cont, index, orig_index) == 0 &&
           i != buckets) 
    {
        increment_wrap(index, buckets);
        ++i;


        /*  need to keep looping until
                - found free
                - the hash of an index

            say we look to remove key with modded hash of 2
            0
            1 1
            2 1   <- start looking here
            3 1
            4 2   (what if no 2, but a 3)
            5 3
            6 6   <- we stop here
            7
            8

            remove 5a
            0 4
            1 5a
            2 5
            3
            4 4
            5 4   <- start looking here

            we will eaither get empty OR a number whose modded hash is
            equal to the index

            we need to know when a hash is diff and compare AGAINST a specific num
                need another template param
                no
                    hash comp should just be replaced by a (index,num) where we compares
                    modded hash at index against num

            IsFree(curr), KeyComp(curr,k), Deconstruct(curr) 

            is it possible to have a (modded hash >= curr index) AND we have to
            search
                yes, if collision overflows past end


        */
    }

    /*  If any of the following hold, there is no
        key to be erased.
            - index is free
                trivial
            - loop through all buckets
                no element was found whose hash value
                could result in key hash we're looking
                for
            - hash values at index modded by buckets
              is greater than hash of key we're looking
              for
                collision chains are non-decreasing
                ignoring wrap around. if current is
                greater, past end of a potenital chain
    */
    if (IsFree()(cont, index) ||
        i == buckets ||
        HashEq()(cont, index, key_hash) == 2)
    {
        return buckets;
    }

    // at this point, we are on a set of hashes which have same modded value as
    // the key hash we're looking for
    // or
    // the ENTIRE container is filled with hashed that were looking for

    /*  this assumes key will be found
            if key looking for doesnt exist
    */

    const auto temp = index;
    while (KeyComp()(cont, index, k))
    {
        increment_wrap(index, buckets);
    }

    Deconstruct()(cont, index);


    /*  curr - deconstructed element, where we move into
        next - element to take from, check against

        want to keep iterating while
            next is free
            and
            non-decreasing sequence
        but "non-decreasing" sequence can cause issues with something
        like "D"
        but we must account for something like A which flows past end

            and not hash equals index

    */

    auto curr = index, next = index;
    increment_wrap(next, buckets);
    while (!IsFree()(cont, next) && 
           HashComp()(cont, curr, next) < 2 &&
           HashEq()(cont, next, next) != 0)
    {
        ElemTransfer()(cont, curr, next);
        
        curr = next;
        increment_wrap(next, buckets);
    }

    

    /*  NOTE: should we return the index of where the removed
                element now is?
    */

    // if we're here then any of the above conditions is true, ie
    // nothing needs to be done, can just continue and return normally

    return curr;
}

/**
 * @brief See \ref open_address_erase_index
 * 
 * @return Sz index of element in cont whose key compares equal
 *            to k, buckets otherwise
 */
template<
    typename Cont,
    typename Arg, typename Sz,
    typename IsFree, typename KeyComp,
    typename HashEq>
Sz
open_address_find(const Cont& cont, const Arg& k, Sz key_hash, Sz buckets)
{
    auto index = key_hash % buckets;

    /*  Terminate the loop when
            - there is a free element
            - hash value at index modded by buckets
              is greater than or equal to the value
              of the initial index to search at 
            - looped through all the buckets
    */
    const auto orig_index = index;
    Sz i = 0;
    while (!IsFree()(cont, index) &&
           HashEq()(cont, index, orig_index) == 0 &&
           i != buckets) 
    {
        increment_wrap(index, buckets);
        ++i;
    }

    /*  If any of the following hold, there is no
        key to be erased.
            - index is free
                trivial
            - loop through all buckets
                no element was found whose hash value
                could result in key hash we're looking
                for
            - hash values at index modded by buckets
              is greater than hash of key we're looking
              for
                collision chains are non-decreasing
                ignoring wrap around. if current is
                greater, past end of a potenital chain
    */
    if (IsFree()(cont, index) ||
        i == buckets ||
        HashEq()(cont, index, key_hash) == 2)
    {
        return buckets;
    }

    /*  At this point, we are on a set of hashes or arbitrary size, 
        possibly filling the entier container, which have same modded
        value asthe key hash we're looking for.
    */  

    /*  Loop through all the hashes whose modded value
        is equal to the modded hash value of key k.
    */
    const auto temp = index;
    Sz j = 0;
    while (!IsFree(cont, index) &&
           HashEq()(cont, index, temp) == 1 &&
           j != buckets)
    {
        if (KeyComp()(cont, index, k))
        {
            return index;
        }

        increment_wrap(index, buckets);
    }

    return buckets;
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

    struct is_free
    {
        bool
        operator()(const element* blk) const
        {
            return get<0>(*blk);
        }
    };

    using testicle = unordered_map_file<Key, Value, Hash>;

    struct Is_Free_Test
    {
        bool
        operator()(const testicle& cont, size_type index) const
        {
            return is_free()(cont.M_file + index);
        }
    };

    struct Hash_Comp_Test
    {
        size_type
        operator()(const testicle& cont, size_type curr, size_type against) const
        {
            const auto modded_curr    = cont._hash(curr) % cont.M_buckets;
            const auto modded_against = cont._hash(against) % cont.M_buckets;

            return (modded_curr >= modded_against) * (1 + (modded_curr > modded_against));
        }
    };

    template<typename Uh>
    struct Key_Comp_Test
    {
        /*  NOTE: should this be a perfect forward ?
        */
        bool
        operator()(const testicle& cont, size_type curr, Uh k) const
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
            cont._is_free(from) = true;
        }
    };

    using iterator       = bidirectional_openaddr<
        std::pair<const Key, Value>,
        element,
        unordered_map_file<Key, Value, Hash>,
        convert, is_free>;
    using const_iterator = bidirectional_openaddr<
        const std::pair<const Key, Value>,
        element,
        unordered_map_file<Key, Value, Hash>,
        convert, is_free>;
    using difference_type = typename iterator::difference_type;

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

    reference_key
    _key(size_type index)
    {
        return get<2>(_block(index)).first;
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

                return Hash_Comp_Test()(
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
                return Key_Comp_Test<Key>()(
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
                local_is_free, local_hash_comp, local_key_comp, local_elem_transfer
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

    /*  === HERE ===
        fix find

        as file state is correct but find isnt

        perhaps have find as external func ?
            prolly not (maybe yes acually)

        we've pretty much implemented it in erase in one of the loops
    */

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
            _is_free(index) = true;
            --M_elem;
        }

        if (empty())
        {
            return end();
        }

        increment_wrap(index, M_buckets);
        return iterator(M_file + index);
    }

    size_type
    erase(const_reference_key k)
    {
        struct local_deconstruct
        {
            void operator()(testicle& cont, size_type curr)
            {
                std::allocator_traits<allocator>::destroy
                (
                    cont.M_alloc,
                    std::addressof(cont._block(curr))
                );
            }
        };
        
        struct local_hash_eq
        {
            size_type operator()(testicle& cont, size_type curr, size_type num)
            {
                const auto modded_curr = cont._hash(curr) % cont.M_buckets;

                return (modded_curr >= num) * (1 + (modded_curr > num));
            }
        };

        auto res = open_address_erase_index<
            decltype(*this),
            key_type, size_type,
            Is_Free_Test, Hash_Comp_Test, Key_Comp_Test<key_type>, Elem_Move_Test,
            local_hash_eq, local_deconstruct>
        (*this, k, Hash()(k), M_buckets);

        if (res == M_buckets)
        {
            return 0;
        }

        --M_elem;
        _is_free(res) = true;

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
