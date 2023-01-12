/*  Locks should actually give exclusion to a resource.
    
    Create the maximum number of parallel pipelines and increment
    a non-atomic variable in every thread a specified number
    of times. Before every variable increment, gain access to the
    lock and release afterwards.
    The final variable value should be #pipelines * #iterations.

    This test does not gaurentee the locks are working. However, as
    the number of times this test is run and the number of iterations
    increases the chance the locks are correct increases.
*/

#ifndef CORRECTNESS_TEST
#define CORRECTNESS_TEST

#include <atomic>
#include <cstddef>
#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <tuple>
#include <vector>

#include "Defs.h"

template<typename T>
struct Arg
{
    T                 lock;
    std::size_t       total;
    std::atomic<bool> begin;
};

template<typename T>
void* thread_increment(void* arg)
{
    Arg<T>* typed_arg = reinterpret_cast<Arg<T>*>(arg);

    while (!typed_arg->begin.load());

    for (int i = 0; i != TESTS_NUM_ITERATS; ++i)
    {
        typed_arg->lock.lock();
        ++(typed_arg->total);
        typed_arg->lock.unlock();
    }

    return nullptr;
}

template<typename T>
class CorrectnessTest
{
public:

    void
    Init()
    {
        threads.resize(TESTS_NUM_THREADS);
        nums.resize(TESTS_NUM_THREADS);
        arg.total  = 0;
        arg.begin  = false; 

        for (int i = 0; i != TESTS_NUM_THREADS; ++i)
        {
            nums[i] = i;
            if (pthread_create(&threads[i], NULL, thread_increment<T>, reinterpret_cast<void*>(&arg)))
            {
                perror("\n\nthread create\n\n");
            }
        }
    }

    void
    Wait()
    {
        for (int i = 0; i != TESTS_NUM_THREADS; ++i)
        {
            if (pthread_join(threads[i], nullptr))
            {
                perror("join");
            }
        }
    }

    Arg<T>&
    GetArg()
    {
        return arg;
    }

private:

    std::vector<pthread_t>   threads;
    std::vector<std::size_t> nums;
    Arg<T>                   arg;

};

template<typename Tuple, std::size_t I>
struct TestLocks
{
    static int TestLock()
    {
        using Lock = typename std::tuple_element<I - 1, Tuple>::type;
        CorrectnessTest<Lock> test;
        test.Init();
        test.GetArg().begin.store(true);
        test.Wait();

        PRINT_RUN_TEST(DEMANGLE_TYPEID_NAME(Lock));
        int res = test.GetArg().total == (TESTS_NUM_THREADS * TESTS_NUM_ITERATS) ? 0 : 1;
        PRINT_FAIL_OR_SUCCESS(res == 0, DEMANGLE_TYPEID_NAME(Lock));

        return TestLocks<Tuple, I - 1>::TestLock() + res;
    }
};

template<typename Tuple>
struct TestLocks<Tuple, 0>
{
    static int TestLock()
    {
        return 0;
    }
};

int
correctness_tests();

#endif
