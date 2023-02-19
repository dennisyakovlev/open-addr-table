#ifndef CUSTOM_FILE_LIBRARY_BACKOFF
#define CUSTOM_FILE_LIBRARY_BACKOFF

#include <cstddef>

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

/**
 * @brief No backoff, do nothing.
 * 
 *        Types of use cases
 *          - any amount of contention
 *          - contention times in nano seconds or single digit
 *            microseconds
 *          - using this will cause locks to raw loop.
 *            this maxes out the cpu until the lock becomes free
 *            which is a potential performance loss if done for
 *            too long
 *        Examples
 *          - modifying atomic counter
 * 
 */
struct backoff_none
{
};

/**
 * @brief Backoff is done entirely in userspace.
 * 
 *        Types of use cases
 *          - not a large amount of contention
 *          - contention times in the triple digit microseconds
 *          - using this will cause locks to smartly loop.
 *            most cpus have special instructions to notify of a
 *            userspace loop. this won't max out the cpu
 *        Examples
 *          - manipulating a container in memory
 */
struct backoff_userspace
{
};

/**
 * @brief A backoff strategy to be used when a lock must
 *        be acquired but is currently taken.
 * 
 * @tparam WaitingType strategy to use, one of the above
 */
template<typename WaitingType>
class backoff
{
};

template<>
class backoff<backoff_none>
{
public:

    void
    wait()
    {
    }

    void
    adjust()
    {
    }

};

template<>
class backoff<backoff_userspace>
{
public:

    backoff() :
        M_estimate(32),
        M_waits(0)
    {
    }

    void
    wait()
    {
        std::size_t local_estimate = M_estimate;
        for (std::size_t i = 0; i != local_estimate; ++i)
        {
            SPIN_PAUSE();
        }

        ++M_waits;
    }

    void
    adjust()
    {
        if (M_waits < 8)
        {
            M_estimate /= 2;
        }
        else
        {
            M_estimate = (1 + ((M_estimate & 0xFF) + (M_estimate / 4))) & 0xFF;
        }

        M_waits = 0;
    }

    std::size_t M_estimate, M_waits;

};

FILE_NAMESPACE_END

#endif
