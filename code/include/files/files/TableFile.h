#ifndef CUSTOM_FILE_LIBRARY_TABLEFILE
#define CUSTOM_FILE_LIBRARY_TABLEFILE

#include "BaseFile.h"
#include <files/Defs.h>
#include <files/Utils.h>

FILE_NAMESPACE_BEGIN

template
<
    typename    Key,
    typename... Values
>
class HashTableFile : public _file_impl<Key, Values...>
{
public:

    using _base = _file_impl<Key, Values...>;

    using value_type      = typename _base::value_type;
    using reference       = typename _base::reference;
    using const_reference = typename _base::const_reference;
    using pointer         = typename _base::pointer;
    using const_pointer   = typename _base::const_pointer;
    using size_type       = typename _base::size_type;
    using iterator        = typename _base::iterator;
    using const_iterator  = typename _base::const_iterator;
    using difference_type = typename _base::difference_type;

    using _file_impl<Key, Values...>::_file_impl;

    Errors
    SetSize(size_type num)
    {
        if (!this->IsInitialized())
        {
            auto err = _base::SetSize(num);
            if (err != Errors::no_error)
            {
                return err;
            }

            this->M_free = num;

            return Errors::no_error;
        }

        if (num == this->M_total)
        {
            return Errors::no_error;
        }

        HashTableFile<Key, Values...> new_file(this->M_path.c_str(), "/temp");
        new_file.SetSize(num);
        for (auto keys = this->Begin(); keys != this->End(); ++keys)
        {
            if (keys->Test(value_type::Bits::usage))
            {
                new_file.Insert(keys->Key());
            }
        }

        auto err = MoveResource(this->M_path + "/temp");
        if (err != Errors::no_error)
        {
            return err;
        }

        this->KeyFileInit();

        return Errors::no_error;
    }

    bool
    Exists(const std::string& key) const
    {
        auto index = GetIndex(key);
        auto iter  = this->Begin() + index;

        if (!iter->Test(value_type::Bits::usage))
        {
            return false;
        }

        if (iter->Test(value_type::Bits::collision))
        {
            return ArrayFile<T>(this->M_path, "/" + std::to_string(index)).Exists(key);
        }

        return ::strncmp(iter->Key(), key.c_str(), value_type::KeySize) == 0;
    }

    Returned<bool>
    Insert(const std::string& key)
    {
        if (Exists(key))
        {
            return { false,Errors::no_error };
        }

        auto index = GetIndex(key);
        auto iter  = this->Begin() + index;

        if (!iter->Test(value_type::Bits::usage))
        {
            ::strncpy(iter->Key(), key.c_str(), value_type::KeySize);
            iter->FlagSet(value_type::Bits::usage, true);
            return { true,Errors::no_error };
        }

        ArrayFile<T> collision(this->M_path, "/" + std::to_string(index));
        if (!iter->Test(value_type::Bits::collision))
        {
            auto err = collision.SetSize(4);
            if (err != Errors::no_error)
            {
                return { false,err };
            }

            auto ret = collision.Insert(iter->Key());
            if (!ret.Valid())
            {
                return ret;
            }
            iter->FlagSet(value_type::Bits::collision, true);
        }
        
        return collision.Insert(key);
    }

    Returned<bool>
    Remove(const std::string& key)
    {
        if (!Exists(key))
        {
            return { false,Errors::no_error };
        }

        auto index = GetIndex(key);
        auto iter  = this->Begin() + index;

        if (!iter->Test(value_type::Bits::collision))
        {
            iter->FlagSet(value_type::Bits::usage, false);
            return { true,Errors::no_error };
        }

        ArrayFile<T> collision(this->M_path, "/" + std::to_string(index));
        auto res = collision.Remove(key);
        if (!res.Valid())
        {
            return { false,res.GetError() };
        }

        if (1 == collision.NumKeys() - collision.NumFreeKeys())
        {
            auto err = collision.SetSize(1);
            if (err != Errors::no_error)
            {
                return { false,err };
            }

            ::memcpy(iter->Key(), collision.Begin()->Key(), value_type::KeySize);
            iter->FlagSet(value_type::Bits::collision, false);
            return { true,collision.FreeResources() };
        }

        return { true,Errors::no_error };
    }

private:

    template<typename U>
    size_type
    GetIndex(const std::string& key, U mod) const
    {
        auto hash = std::hash<std::string>{}(key);
        if (hash)
        {
            return static_cast<size_type>(hash % mod);
        }

        return 0;
    }

    size_type
    GetIndex(const std::string& key) const
    {   
        return GetIndex(key, this->M_total);
    }

    Errors
    MoveResource(const std::string& other)
    {
        auto err = this->FreeResources();
        if (err != Errors::no_error)
        {
            return err;
        }

        if (::rename(other.c_str(), this->M_full.c_str()))
        {
            return Errors::system;
        }

        return Errors::no_error;
    }

};

FILE_NAMESPACE_END

#endif
