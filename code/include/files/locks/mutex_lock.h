#ifndef CUSTOM_FILE_LIBRARY_MUTEXLOCK
#define CUSTOM_FILE_LIBRARY_MUTEXLOCK

#include <errno.h>
#include <pthread.h>
#include <utility>

#include <files/Defs.h>

FILE_NAMESPACE_BEGIN

class mutex_lock
{
public:

    mutex_lock()
        : M_valid(true)
    {
        pthread_mutexattr_t attr;

        if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
        {
            M_valid = false;
        }
        else if (pthread_mutex_init(&M_mutex, &attr))
        {
            M_valid = false;
        }

        pthread_mutexattr_destroy(&attr);
    }

    ~mutex_lock()
    {
        pthread_mutex_destroy(&M_mutex);
    }

    std::pair<bool, Errors>
    lock()
    {
        if (!M_valid)
        {
            return { false,Errors::system };
        }

        auto res = pthread_mutex_lock(&M_mutex);
        if (res)
        {
            return { false, Errors::system };
        }
        return { true,Errors::no_error };
    }

    std::pair<bool, Errors>
    unlock()
    {
        if (!M_valid)
        {
            return { false,Errors::system };
        }

        auto res = pthread_mutex_unlock(&M_mutex);
        if (EPERM == res)
        {
            return { false,Errors::no_error };
        }
        else if (res)
        {
            return { true,Errors::system };
        }

        return { true,Errors::no_error };
    }

private:

    pthread_mutex_t M_mutex;
    bool            M_valid;

};

FILE_NAMESPACE_END

#endif
