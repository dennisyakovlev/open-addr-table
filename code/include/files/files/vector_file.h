#ifndef CUSTOM_FILE_LIBRARY_ARRAYFILE
#define CUSTOM_FILE_LIBRARY_ARRAYFILE

/*  Key type now has same requirements as Cache key requirements
        plus assignment ???
*/

#include <stack>
#include <utility>

#include "base_file.h"
#include <files/Allocators.h>
#include <files/unordered_map_lru.h>
#include <files/Defs.h>

/*  transfer to "err_state" function which will give the current error state

*/

/*  instead of inheritance, since we dont really need (??? right) can have
    base_file has member and just manage the keys as such



    look at say insert
*/

/*  return std pair from insert with iterator being end
    if not valid, ie Returned.Valid() would be false
*/

FILE_NAMESPACE_BEGIN

template<typename Key, typename... Values>
class vector_file : public base_file<Key, bool, Values...>
{
protected:

    void
    KeyFileInit()
    {
        if (this->ready())
        {
            M_taken.reserve(this->size());

            size_type i = 0;
            for (const auto& block : *this)
            {
                if (InUse(block))
                {
                    M_taken.insert(std::make_pair(GetKey(block), i));
                    --M_available;
                }
                else
                {
                    M_free.push(i);
                }

                ++i;
            }
        }
    }

public:

    using _base = base_file<Key, bool, Values...>;

    using value_type      = typename _base::value_type;
    using reference       = typename _base::reference;
    using const_reference = typename _base::const_reference;
    using pointer         = typename _base::pointer;
    using const_pointer   = typename _base::const_pointer;
    using size_type       = typename _base::size_type;
    using iterator        = typename _base::iterator;
    using const_iterator  = typename _base::const_iterator;
    using difference_type = typename _base::difference_type;

    using key_type            = typename _base::key_type;
    using reference_key       = typename _base::reference_key;
    using const_reference_key = typename _base::const_reference_key;

    vector_file()
    {
        
    }

    vector_file(std::string name, size_type keys)
        : _base(std::move(name), keys),
          M_taken(keys), M_free(),
          M_available(keys)
    {
        KeyFileInit();
    }

    vector_file(std::string path, std::string name, size_type keys)
        : _base(std::move(path), std::move(name), keys),
          M_taken(0), M_free(),
          M_available(keys)
    {
        KeyFileInit();
    }

    size_type
    size_free()
    {
        return M_available;
    }

    Errors
    resize(size_type n)
    {
        if (n < this->size() - M_available)
        {
            /*  Resize to less than the number of
                in use keys.
            */
            return Errors::inavlid_args;
        }

        M_taken.reserve(n);

        if (n > this->size())
        {
            auto new_keys = n - this->size();
            for (size_type i = 0; i != new_keys; ++i)
            {
                M_free.push(this->size() + i);
            }
            M_available += new_keys;

            return _base::resize(n);
        }

        /*  Downsizing. Move all the keys to start
            of file and update data structues.
        */
        std::deque<size_type> free;
        size_type i = 0;
        for (auto& block : *this)
        {
            if (InUse(block))
            {
                if (!free.empty())
                {
                    auto earliest = free.front();
                    free.pop_front();

                    UseKey(this->begin() + earliest, GetKey(block));
                    SetUse(block, false);

                    free.emplace_back(i);
                }
            }
            else
            {
                free.emplace_back(i);
            }

            ++i;
        }

        M_taken.reserve(n);
        M_available = 0;
        while (!M_free.empty())
        {
            M_free.pop();
        }
        while (!free.empty() && free.front() < n)
        {
            M_free.push(free.front());
            free.pop_front();
            ++M_available;
        }

        return _base::resize(n);
    }

    /*  emplace args would be used to construct a file block
        but how ?

        with iterator we need to use =, which wouldnt be construction ?

        we want placement new but not really want uh to use allocator traitss?
    */

    Returned<bool>
    Insert(const Key& key)
    {
        if (contains(key))
        {
            return { false,Errors::no_error };
        }

        if (!M_available)
        {
            auto err = resize(this->M_total * 2);
            if (err != Errors::no_error)
            {
                return { false,err };
            }
        }

        UseKey(this->begin() + M_free.top(), key);
        --M_available;
        M_taken.insert(std::make_pair(key, M_free.top()));
        M_free.pop();

        return { true,Errors::no_error };
    }

    size_type
    erase(const_reference_key key)
    {
        auto iter = M_taken.find(key);
        if (iter == M_taken.end())
        {
            return 0;
        }

        auto index = iter->second;
        M_free.push(index);
        M_taken.erase(iter);
        SetUse(this->begin() + index, false);
        ++M_available;

        return 1;
    }

    bool
    contains(const Key& key) const
    {
        return M_taken.contains(key);
    }

private:

    bool
    InUse(const_reference iter)
    {
        return get<1>(iter);
    }

    void
    SetUse(iterator iter, bool use)
    {
        set<1>(*iter, use);
    }

    void
    SetUse(reference block, bool use)
    {
        set<1>(block, use);
    }

    const_reference_key
    GetKey(const_reference block)
    {
        return get<0>(block);
    }

    void
    UseKey(iterator iter, const_reference_key key)
    {
        set<0>(*iter, key);
        SetUse(iter, true);
    }

    unordered_map_lru<key_type, size_type> M_taken;
    std::stack<size_type>                  M_free;
    size_type                              M_available;

};

FILE_NAMESPACE_END

#endif
