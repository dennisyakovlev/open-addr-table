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

#include <gtest/gtest.h>

#include <lock_tests/Defs.h>
#include <files/locks.h>

template<typename T>
struct Arg
{

    using Lock = T;

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
class CorrectnessTest : public testing::Test
{
protected:

    CorrectnessTest()
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

    std::vector<pthread_t>   threads;
    std::vector<std::size_t> nums;
    Arg<T>                   arg;

};

using MyTypes = ::testing::Types<
    MmapFiles::spin_lock,
    MmapFiles::mutex_lock,
    MmapFiles::queue_lock
>;
TYPED_TEST_CASE(CorrectnessTest, MyTypes);

TYPED_TEST(CorrectnessTest, Exclusion)
{
    this->arg.begin.store(true);
    this->Wait();

    ASSERT_EQ(this->arg.total, TESTS_NUM_THREADS * TESTS_NUM_ITERATS);

}

#endif
