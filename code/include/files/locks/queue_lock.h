#ifndef CUSTOM_FILE_LIBRARY_QUEUELOCK
#define CUSTOM_FILE_LIBRARY_QUEUELOCK

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#include <utility>

#include <files/Defs.h>
#include "backoff.h"

FILE_NAMESPACE_BEGIN

#ifndef SAFE_BUILD

/**
 * @brief A lock which gives a strong promise for fair access to
 *        requests. Not recursive.
 *
 * @note  How to define "fair"? Here defined as
 *          """
 *          any request to obtain the lock will be honored on a first
 *          come first serve basis
 *          """
 *        Importantly this does not put any gaurentees on time limits on the
 *        owner once the lock is obtained. That is, not preemptive.
 *        This is distinct from a different idea of "fair", as in every thread
 *        will have an equal amount of time owning to the lock.
 *
 * @note Is the lock fully fair, no. A fully fair - as defined above - lock
 *       would have to some form of timestmp + tracking system and verify that
 *       lock requests are given out equally. This would be much slower.
 *       Instead this lock minimizes the chances of starvation. It is minized
 *       quite well to the point where it is no longer something to consider.
 *
 */
template<typename Strategy>
class queue_lock
{
public:

    /*  Must be unsigned. Unsigned type will give defined overflow behaviour
        which will allow the tail/head relationship to work.

        Is it technically possible for the number of unserved lock requests
        to become so large that head wraps around and reaches tail. But this
        means max_value(BASE) number of different threads have made an
        unserved request to lock the lock. Which is impossibly large.
    */
    using Base = std::size_t;
    using Atom = std::atomic<Base>;

    queue_lock() :
        M_head{ 0 },
        M_tail{ 0 },
        M_backoff()
    {
    }

    void
    lock()
    {
        /*  This is a bit iffy since != and == are not defined for
            atomic variables. Compiler will just do a comparison of
            memory. Might work might not.

            For example, if I use Base for wanted_head on my machine
            it fails to correctly compare, causing lock to not work.

            But this gives extra perforamnce since M_tail is not
            atomic and the loop comparison is non-atomic.
        */

        Atom wanted_head(M_head.fetch_add(1, std::memory_order_relaxed));
        while (wanted_head != M_tail)
        {
            M_backoff.wait();
        }

        M_backoff.adjust();

    }

    void
    unlock()
    {
        ++M_tail;
    }

private:

    Atom              M_head;
    Base              M_tail;
    backoff<Strategy> M_backoff;

};

#else

/**
 * @brief A safe build which trades off performance for portability. While
 *        the above lock has some undetermined behaviour, this one is
 *        gaurentted to work. All the same docs as above apply. Is about
 *        5x slower on my machine.
 *
 * @tparam Strategy
 */
template<typename Strategy>
class queue_lock
{
public:

    using Base = std::size_t;
    using Atom = std::atomic<Base>;

    queue_lock() :
        M_head{ 0 },
        M_tail{ 0 },
        M_backoff()
    {
    }

    void
    lock()
    {
        Base wanted_head = M_head.fetch_add(1);
        Base original    = wanted_head;
        while (!M_tail.compare_exchange_strong(wanted_head, wanted_head))
        {
            M_backoff.wait();

            wanted_head = original;
        }

        M_backoff.adjust();
    }

    void
    unlock()
    {
        ++M_tail;
    }

private:

    Atom              M_head, M_tail;
    backoff<Strategy> M_backoff;

};

#endif

FILE_NAMESPACE_END

#endif
