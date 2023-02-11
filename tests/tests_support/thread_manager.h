#ifndef CUSTOM_TESTS_THREADMANAGER
#define CUSTOM_TESTS_THREADMANAGER

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <pthread.h>
#include <utility>
#include <vector>

/**
 * @brief Type which contains necessary information for tests
 *        on locks.
 * 
 * @tparam Lock lock type to use
 */
template<typename Lock>
struct thread_arg
{

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
        ++(typed_arg->total);
        typed_arg->lock.unlock();
    }

    pthread_exit(nullptr);
}

/**
 * @brief Manage threads, errors, and other data to make
 *        tests more readable.
 * 
 * @tparam Lock lock type
 */
template<typename Lock>
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
        M_okay(true)
    {
        M_arg.total          = 0;
        M_arg.num_iterations = iterations;
        M_arg.begin          = false;

        for (std::size_t i = 0; i != num; ++i)
        {
            add_thread(thread_increment<Lock>, &M_arg);
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
    template<typename Arg>
    pthread_t
    add_thread(void* (*start_routine)(void*), Arg arg)
    {
        M_threads.push_back({0,nullptr});
        if (pthread_create(&M_threads.back().first, NULL, start_routine, static_cast<void*>(arg)))
        {
            perror("\n\nthread create\n\n");
            M_okay = false;
        }

        return M_threads.back().first;
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
        for (std::size_t i = 0; i != M_threads.size(); ++i)
        {
            if (pthread_join(M_threads[i].first, &(M_threads[i].second)))
            {
                perror("\n\njoin\n\n");
                M_okay = false;
            }
        }
    }

    /**
     * @brief Get the parameter which is passed to the default
     *        function.
     * 
     * @return thread_arg<Lock>& 
     */
    thread_arg<Lock>&
    arg()
    {
        return M_arg;
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
        return *static_cast<T*>(std::find_if(M_threads.cbegin(), M_threads.cend(), [=](const auto& pair) {
            return pair.first == id;
        })->second);
    }

    /**
     * @brief Determine whether the current objects state is
     *        valid. Is invalidated upon error.
     *        Should use this func at end of test to make sure
     *        the test results have meaning.
     * 
     * @return true valid 
     * @return false invalid
     */
    bool
    state()
    {
        return M_okay;
    }

    thread_arg<Lock> M_arg;
    std::vector<std::pair<pthread_t, void*>>
                     M_threads;
    bool             M_okay;

};

#endif
