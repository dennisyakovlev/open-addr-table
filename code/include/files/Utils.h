#ifndef CUSTOM_FILE_LIBRARY_UTILS
#define CUSTOM_FILE_LIBRARY_UTILS

#include <files/Defs.h>

FILE_NAMESPACE_BEGIN

enum class Errors : int
{
    no_error = 0, // okay, no errors
    inavlid_args, // arguements to function were invalid
    system,       // system call failed with error
    cant_lock,    // cannot obtain lock
    not_found,    // requested hash/key value not found
    cant_get,     // cannot get the resource
    full          // resource cannot have more data added
};

/**
 * @brief Wrapper around return value.
 * 
 * @tparam T Value type to return.
 */
template<typename T>
class _ret_impl : public std::pair<T, Errors>
{
public:

    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;

    using std::pair<T, Errors>::pair;

    const_reference
    GetReturn() const
    {
        return this->first;
    }

    reference
    GetReturn()
    {
        return this->first;
    }

    Errors
    GetError() const
    {
        return this->second;
    }

    bool
    Valid() const
    {
        return this->second == Errors::no_error;
    }

};

/**
 * @brief Class to make access simple to returned data
 *        of the form (data, error).
 * 
 * @note If there is an error, return type is invalid.
 * 
 * @tparam Ret data return type 
 */
template<typename T>
class Returned : public _ret_impl<T>
{
public:

    using _base = _ret_impl<T>; 

    using value_type      = typename _base::value_type;
    using reference       = typename _base::reference;
    using const_reference = typename _base::const_reference;

    using _ret_impl<T>::_ret_impl;

};

template<>
class Returned<bool> : public _ret_impl<bool>
{
public:

    using _base = _ret_impl<bool>; 

    using value_type      = typename _base::value_type;
    using reference       = typename _base::reference;
    using const_reference = typename _base::const_reference;

    using _ret_impl<bool>::_ret_impl;

    /**
     * @brief Implicit converion to boolean.
     * 
     * @return true  If and only if return true and no error.
     * @return false Otherwise.
     */
    operator bool() const
    {
        if (this->Valid())
        {
            return this->GetReturn();
        }

        return false;
    }

};

FILE_NAMESPACE_END

#endif