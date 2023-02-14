#ifndef CUSTOM_TESTS_THREADMANAGER
#define CUSTOM_TESTS_THREADMANAGER

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstring>
#include <memory>
#include <pthread.h>
#include <string>
#include <utility>

/**
 * @brief Type which contains necessary information for tests
 *        on locks.
 * 
 * @tparam Lock lock type to use
 */
template<typename Lock>
struct thread_arg
{

    thread_arg() = delete;

    thread_arg(std::size_t iterations) :
        lock(),
        total(0),
        num_iterations(iterations),
        begin(false),
        dead(0)
    {
    }

    /**
     * @brief lock to use
     * 
     */
    Lock lock;

    /**
     * @brief the value which is incremented by
     *        the all of the threads 
     */
    std::size_t total;
    
    /**
     * @brief number of times to request the lock and
     *        increment total by in a single thread
     * 
     */
    std::size_t num_iterations;
    
    /**
     * @brief "signal" to let the threads know
     *        when to start incrementing
     * 
     */
    std::atomic<bool> begin;

    /**
     * @brief number of threads which are not executing at 
     *        the current moment
     * 
     *        every thread has an obligation to decrement
     *        dead just before it exists
     */
    std::atomic<std::size_t> dead;

};

/**
 * @brief stupidly increment a variable using a lock for
 *        a set number of times. see \ref thread_arg for
 *        more
 * 
 * @tparam Lock lock type
 * @param arg a thread_arg
 * @return void* 
 */
template<typename Lock>
void*
thread_increment(void* arg)
{
    thread_arg<Lock>* typed_arg = static_cast<thread_arg<Lock>*>(arg);

    while (!typed_arg->begin.load());

    for (std::size_t i = 0; i != typed_arg->num_iterations; ++i)
    {
        typed_arg->lock.lock();

        ++typed_arg->total;
        
        typed_arg->lock.unlock();
    }

    --typed_arg->dead;
    pthread_exit(nullptr);
}

/**
 * @brief Manage threads, errors, and other data to make
 *        tests more readable.
 * 
 * @tparam Lock lock type
 */
template<typename Lock, typename Arg = thread_arg<Lock>>
class thread_manager
{
public:

    /**
     * @brief Setup variables, create and run threads. Created
     *        threads are assigned the default start function of
     *        thread_increment.
     *  
     * @param num number of threads to create with default function
     * @param iteratios number of iterations each thread will do
     *                  inside the default function
     */
    thread_manager(std::size_t num, std::size_t iterations) :
        M_thread_num(0),
        M_arg(iterations)
    {
        for (std::size_t i = 0; i != num; ++i)
        {
            add_thread(thread_increment<Lock>);
        }
    }

    template<typename... Args>
    thread_manager(std::size_t num, Args... args) :
        M_thread_num(0),
        M_arg(std::forward<Args>(args)...)
    {
        for (std::size_t i = 0; i != num; ++i)
        {
            add_thread(thread_increment<Lock>);
        }
    }

    /**
     * @brief Add a function which will be executed by a thread.
     *        Return thread id which can be used to retrieve
     *        value from function.
     * 
     * @tparam Arg arguement type
     * @param start_routine function which matches thread start routine
     *                      signature
     * @param arg arguement to pass to function
     * @return pthread_t unique thread id
     */
    template<typename FuncArg>
    pthread_t
    add_thread(void* (*start_routine)(void*), FuncArg arg)
    {
        if (M_thread_num == M_threads.size())
        {
            throw std::system_error(
                -1,
                std::generic_category(),
                "Ran out of room for threads"
            );

            return 0;
        }

        if (pthread_create(&M_threads[M_thread_num].first, NULL, start_routine, static_cast<void*>(arg)))
        {
            --M_arg.dead;
            
            throw std::system_error(
                -1,
                std::generic_category(),
                std::string("bad pthread create") + strerror(errno)
            );

            return 0;
        }

        ++M_arg.dead;
        ++M_thread_num;

        return M_threads[M_thread_num - 1].first;
    }

    pthread_t
    add_thread(void* (*start_routine)(void*))
    {
        return add_thread(start_routine, &M_arg);
    }

    /**
     * @brief sets arg.begin to true. will allow functions in
     *        threads to start their logical execution.
     */
    void
    start()
    {
        M_arg.begin.store(true);
    }

    /**
     * @brief Force a synchronus wait until all the threads
     *        added are done. Saves return value of threads.
     */
    void
    wait()
    {
        for (std::size_t i = 0; i != M_thread_num; ++i)
        {
            if (pthread_join(M_threads[i].first, &M_threads[i].second))
            {
                throw std::system_error(
                    -1,
                    std::generic_category(),
                    std::string("bad join pthread: ") + strerror(errno)
                );
            }
        }
    }

    /**
     * @brief Get returned value from a thread once it is
     *        done executing. Must be called after wait. Must
     *        be valid referenceble pointer returned from
     *        thread.
     * 
     * @tparam T 
     * @param id must be valid id returned from add_thread
     * @return T& value
     */
    template<typename T>
    T&
    return_val(pthread_t id)
    {
        return *static_cast<T*>
        (
            std::find_if(M_threads.cbegin(), M_threads.cend(),
            [=](const std::pair<pthread_t, void*>& pair)
            {
                return pair.first == id;
            })->second
        );
    }

private:

    /**
     * @brief This cannot be reallocated... something internal to
     *        pthreds demands the pointer given to pthread_create
     *        remains valid.
     * 
     */
    std::array<std::pair<pthread_t, void*>, 256>
                M_threads;
    std::size_t M_thread_num;
    Arg         M_arg;

};

#endif
