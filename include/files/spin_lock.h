#ifndef CUSTOM_FILE_LIBRARY_SPINLOCK
#define CUSTOM_FILE_LIBRARY_SPINLOCK

#include <atomic>
#include <cstddef>
#include <pthread.h>
#include <utility>

#include "backoff.h"
#include "defs.h"

FILE_NAMESPACE_BEGIN

template<typename Strategy>
class spin_lock
{
public:

    spin_lock() :
        M_free(true),
        M_recurse(0),
        M_holder(0)
    {
    }

    void
    lock()
    {
        pthread_t curr_thread = pthread_self();     
        bool want             = true;

        while (!M_free.compare_exchange_weak(want, false))
        {
            /*  For recursive lock.
            */
            if (M_holder == curr_thread)
            {
                M_recurse.fetch_add(1);

                return;
            }

            M_backoff.wait();

            want = true;
        }

        M_backoff.adjust();

        M_holder = pthread_self();
        M_recurse.fetch_add(1);
    }

    void
    unlock()
    {
        if (!M_free.load() && M_holder == pthread_self())
        {
            std::size_t want = 1;
            if (M_recurse.compare_exchange_weak(want, 0))
            {
                M_holder = 0;
                M_free = true;
            }
            else
            {
                M_recurse.fetch_sub(1);
            }
        }
    }

private:

    std::atomic<bool>        M_free;
    std::atomic<std::size_t> M_recurse;
    pthread_t                M_holder;
    backoff<Strategy>        M_backoff;

};

FILE_NAMESPACE_END

#endif
