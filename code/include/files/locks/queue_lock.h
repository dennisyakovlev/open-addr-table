#ifndef CUSTOM_FILE_LIBRARY_QUEUELOCK
#define CUSTOM_FILE_LIBRARY_QUEUELOCK

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#include <utility>

#include <files/Defs.h>

FILE_NAMESPACE_BEGIN

class queue_lock
{
public:

    queue_lock()
        : M_head{ 0 }, M_tail{ 0 }, M_holder{ 0 }, M_locked(false)
    {
    }

    std::pair<bool, Errors>
    lock()
    {
        uint32_t curr_head = M_head.fetch_add(1);
        while (true)
        {
            uint32_t curr_tail = M_tail;
            if (curr_head == curr_tail)
            {
                break;
            }

            int res = syscall(SYS_futex, reinterpret_cast<uint32_t*>(&M_tail), FUTEX_WAIT, curr_tail, NULL, 0, 0);
            if (errno != EAGAIN && res == -1)
            {
                return { false,Errors::system };
            }
        }

        M_holder = pthread_self();
        M_locked = true;

        return { true,Errors::no_error };
    }

    std::pair<bool, Errors>
    unlock()
    {
        if (M_locked && M_holder == pthread_self())
        {
            /*  Order here matters. Once M_tail is incremented, other threads will have
                access to the lock if they are waiting. Must, unlock and reset holder
                before another thread has potential access.
            */
            M_holder = 0;
            M_locked = false;
            ++M_tail;

            /*  Must wake up all waiting threads.
            */
            long res = syscall(SYS_futex, reinterpret_cast<uint32_t*>(&M_tail), FUTEX_WAKE, INT32_MAX, NULL, 0, 0);
            if (res == -1)
            {
                return { true,Errors::system };
            }

            return { true,Errors::no_error };
        }

        return { false,Errors::no_error };
    }

private:

    std::atomic<uint32_t> M_head;
    uint32_t              M_tail;
    pthread_t             M_holder;
    bool                  M_locked;

};

FILE_NAMESPACE_END

#endif
