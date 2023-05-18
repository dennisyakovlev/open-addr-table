#ifndef INCLUDE_GAURD_SCRIPTKEYCACHE
#define INCLUDE_GAURD_SCRIPTKEYCACHE

#include <list>
#include <unordered_map>
#include <utility>

#include "defs.h"

FILE_NAMESPACE_BEGIN

/**
 * @brief LRU cache with added remove and resize operations.
 * 
 * @tparam Key   Key type.
 * @tparam Value Value type.
 * @tparam Hash  Hash operator.
 * 
 * @note Hash type must meet requirements of std::hash.
 *          Must have const function call operator which takes
 *          const reference of type.
 * @note Key must meet requirements of std::unordered_map key type.
 *          Must have operator == and a "Hash" type
 */
template<
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>
>
class unordered_map_lru
{
public:

    using value_type      = std::pair<const Key, Value>;
    using reference       = std::pair<const Key, Value>&;
    using const_reference = const std::pair<const Key, Value>&;
    using pointer         = std::pair<const Key, Value>*;
    using const_pointer   = const std::pair<const Key, Value>*;
    using size_type       = typename std::list<std::pair<const Key, Value>>::size_type;
    using iterator        = typename std::list<std::pair<const Key, Value>>::iterator;
    using const_iterator  = typename std::list<std::pair<const Key, Value>>::const_iterator;
    using difference_type = typename iterator::difference_type;

    using key_type            = Key;
    using reference_key       = Key&;
    using const_reference_key = const Key&;
    using pointer_key         = Key*;
    using const_pointer_key   = const Key*;
    using mapped_type         = Value;

    unordered_map_lru() = default;

    unordered_map_lru(size_type n)
        : M_data(), M_cache(),
          M_max(n)
    {
    }

    size_type
    size() const
    {
        return M_data.size();
    }

    const_iterator
    cbegin() const
    {
        return M_data.cbegin();
    }

    iterator
    begin()
    {
        return M_data.begin();
    }

    const_iterator
    cend() const
    {
        return M_data.end();
    }

    iterator
    end()
    {
        return M_data.end();
    }

    /**
     * @brief Resize the maximum number of keys to n.
     *        Resizing to smaller will remove the
     *        required number of least recently used
     *        keys. Reszing to larger does not remove
     *        keys.
     * 
     * @param n Numbers of keys.
     */
    void
    reserve(size_type n)
    {
        if (n < M_data.size())
        {
            auto remove = M_data.size() - n;
            for (size_type i = 0; i != remove; ++i)
            {
                M_cache.erase(key(*least_recent()));
                M_data.erase(least_recent());
            }

        }

        M_max = n;
    }

    iterator
    find(const_reference_key k)
    {
        auto res = M_cache.find(k);
        if (res != M_cache.end())
        {
            return res->second;
        }

        return M_data.end();
    }

    const_iterator
    find(const_reference_key k) const
    {
        auto res = M_cache.find(k);
        if (res != M_cache.cend())
        {
            return res->second;
        }

        return M_data.cend();
    }

    /**
     * @brief For insert({x,y}) case.
     */
    std::pair<iterator, bool>
    insert(value_type&& v)
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
    emplace(Arg&& k, Args&&... args)
    {
        auto res = M_cache.emplace(std::forward<Arg>(k), M_data.end());
        if (!res.second)
        {
            return { res.first->second,false };
        }

        // NOTE: forces key to be copy contructible
        M_data.emplace_front(res.first->first, std::forward<Args>(args)...);
        res.first->second = M_data.begin();
        trim();

        return { M_data.begin(),true };
    }

    /**
     * @brief Store a key.
     * 
     * @note If a key already exists and is stored,
     *       its order is changed of the LRU.
     * 
     * @param key Key to store.
     * @return true  Key does not exist.
     * @return false Key already exists.
     */
    template<typename T, typename U>
    std::pair<iterator, bool>
    insert_or_assign(T&& k, U&& val)
    {
        auto res = M_cache.emplace(std::forward<T>(k), M_data.end());
        if (!res.second)
        {
            M_data.erase(res.first->second);
            M_data.emplace_front(res.first->first, std::forward<U>(val));
            res.first->second = M_data.begin();

            return { res.first->second,false };
        }

        M_data.emplace_front(res.first->first, std::forward<U>(val));
        res.first->second = M_data.begin();
        trim();

        return { M_data.begin(),true };
    }

    iterator
    erase(const_iterator iter)
    {
        if (iter == end())
        {
            return end();
        }

        M_cache.erase(iter->first);
        return M_data.erase(iter);
    }

    size_type
    erase(const_reference_key key)
    {
        auto info = M_cache.find(key);
        if (info != M_cache.end())
        {
            M_data.erase(info->second);
            M_cache.erase(info);

            return 1;
        }

        return 0;
    }

    bool
    contains(const_reference_key key) const
    {
        return M_cache.find(key) != M_cache.end();
    }

    bool
    empty() const
    {
        return M_data.empty();
    }

    void
    clear()
    {
        M_data.clear();
        M_cache.clear();
    }

    size_type
    bucket_count() const
    {
        return M_cache.bucket_count();
    }

    size_type
    max_bucket_count() const
    {
        return M_cache.max_bucket_count();
    }

    size_type
    bucket_size(size_type index) const
    {
        if (index > M_cache.size())
        {
            return 0;
        }

        return M_cache.bucket_size(index);
    }

    size_type
    bucket(const_reference_key k)
    {
        return M_cache.bucket(k);
    }

    float
    load_factor() const
    {
        return M_cache.load_factor();
    }

    float
    max_load_factor() const
    {
        return M_cache.max_load_factor();
    }

    void
    max_load_factor(float mzlf)
    {
        M_cache.max_load_factor(mzlf);
    }

protected:    

    const key_type&
    key(const value_type& v) const
    {
        return v.first;
    }

    Value&
    val(const value_type& v) const
    {
        return v.second;
    }

    iterator
    least_recent()
    {
        return --M_data.end();
    }

    /**
     * @brief remove least recent if necessary
     * 
     */
    void
    trim()
    {
        if (M_data.size() > M_max)
        {
            M_cache.erase(key(*least_recent()));
            M_data.erase(least_recent());
        }
    }

    std::list<value_type>                        M_data;
    std::unordered_map<key_type, iterator, Hash> M_cache;
    size_type                                    M_max;

};

FILE_NAMESPACE_END

#endif
