#ifndef CUSTOM_FILE_LIBRARY_SPINLOCK
#define CUSTOM_FILE_LIBRARY_SPINLOCK

#include <atomic>
#include <cstddef>
#include <pthread.h>
#include <utility>

#include <files/Defs.h>

#if defined(__x86_64__) || defined(__i386__)

  #define SPIN_PAUSE() \
    __asm__ __volatile__ ("pause")

#elif defined(__ia64__)

  #if (defined(__HP_cc__) || defined(__HP_aCC__))

    #define SPIN_PAUSE() \
        _Asm_hint(_HINT_PAUSE)

  #else

    #define SPIN_PAUSE() \
        __asm__ __volatile__ ("hint @pause")

  #endif

#elif defined(__arm__)

  #ifdef __CC_ARM

    #define SPIN_PAUSE() \
        __yield()

  #else

    #define SPIN_PAUSE() \
        __asm__ __volatile__ ("yield")

  #endif

#else

    static_assert(0, "Need \"pause\" instruction or similar defined.");

#endif

FILE_NAMESPACE_BEGIN

class spin_lock
{
public:

    spin_lock()
        : M_free(true), M_recurse(0), M_estimate(32), M_holder(0)
    {
    }

    std::pair<bool, Errors>
    lock()
    {
        pthread_t curr_thread = pthread_self();     
        bool want             = true;
        std::size_t times     = 0;

        while (!M_free.compare_exchange_weak(want, false))
        {
            /*  For recursive lock.
            */
            if (M_holder == curr_thread)
            {
                M_recurse.fetch_add(1);

                return { true,Errors::no_error };
            }

            std::size_t local_estimate = M_estimate;
            for (std::size_t i = 0; i != local_estimate; ++i)
            {
                SPIN_PAUSE();
            }

            want = true;
            ++times;
        }

        if (times == 0)
        {
            M_estimate /= 2;
        }
        else
        {
            M_estimate = (1 + ((M_estimate & 0xFF) + (M_estimate / 4))) & 0xFF;
        }

        M_holder = pthread_self();
        M_recurse.fetch_add(1);

        return { times == 0,Errors::no_error };
    }

    std::pair<bool, Errors>
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
            
            return { true,Errors::no_error };
        }

        return { false,Errors::no_error };        
    }

private:

    std::atomic<bool>        M_free;
    std::atomic<std::size_t> M_recurse;
    std::size_t              M_estimate;
    pthread_t                M_holder;

};

FILE_NAMESPACE_END

#endif
