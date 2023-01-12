#include <atomic>
#include <iostream>
#include <stddef.h>
#include <pthread.h>
#include <stdio.h>
#include <vector>

#include "Defs.h"
#include "QueueFairness.h"
#include <files/locks.h>

std::size_t           total;
MmapFiles::queue_lock lock;
std::atomic<bool>     thread_begin;

void*
func_starve(void* _)
{
    while (thread_begin.load());

    for (int i = 0; i != TESTS_NUM_ITERATS; ++i)
    {
        lock.lock();
        ++total;
        lock.unlock();
    }

    return nullptr;
}

void*
func_nice(void* _)
{
    while (thread_begin.load());

    std::size_t times = 0, lag = 0, old_total = 0, mod = 500;
    while (total < TESTS_NUM_ITERATS * (TESTS_NUM_THREADS - 1))
    {
        std::size_t temp = total;
        if (temp % mod == 0 && temp > old_total)
        {
            old_total = total;
            lock.lock();

            /*  How long its been between pre lock and after lock. There
                is some margin for error as above two lines are not
                atomic, but error is quite small.
            */
            long curr_lag = total - old_total;

            /*  At most (TESTS_NUM_THREADS - 1) starve threads will
                be scheduled at the same  time. There can be atmost
                TESTS_NUM_THREADS of lag which can be discarded. If
                the lag is greater than TESTS_NUM_THREADS, more
                than cycle of lock requests has passed, so the lag
                is counted.
            */
            if (curr_lag > TESTS_NUM_THREADS)
            {
                lag += curr_lag;
            }
            ++times;

            lock.unlock();
        }
    }

    /*  This is the average number of increments after a "mod" that this
        thread was able to obtain this lock.
        If this is high, then the thread had to wait a long time to get
        the lock.

        Try switching "lock" type to a different lock, say MutexLock and
        observe difference.
    */
    double average_lag = static_cast<double>(lag) / times;

    PRINT_RUN_TEST("Fairness");

    /*  If the average lag is grater than 0.01 then from 100 requests,
        on average, the lock was obtained later than 1 increment of
        "total".
    */
    int* ret = new int(0);
    if (average_lag > 0.01)
    {
        *ret = 1;
    }
    
    PRINT_FAIL_OR_SUCCESS(*ret == 0, "Fairness");
    if (*ret)
    {
        std::cout << "Expected average_lag < " << 0.01 << " and got " << average_lag << "\n";
    }

    return ret;
}

int
queue_fairness_test()
{
    PRINT_BEGIN_TESTS("QueueLockFairness");

    std::vector<pthread_t> threads(TESTS_NUM_THREADS);
    for (int i = 0; i != TESTS_NUM_THREADS - 1; ++i)
    {
        if (pthread_create(&threads[i], NULL, func_starve, NULL))
        {
            perror("create");
        }
    }
    if (pthread_create(&threads[TESTS_NUM_THREADS - 1], NULL, func_nice, NULL))
    {
        perror("create");
    }

    thread_begin.store(false);

    for (int i = 0; i != TESTS_NUM_THREADS - 1; ++i)
    {
        if (pthread_join(threads[i], NULL))
        {
            perror("join");
        }
    }
    int* res;
    if (pthread_join(threads.back(), reinterpret_cast<void**>(&res)))
    {
        perror("join");
    }

    PRINT_END_TESTS("QueueLockFairness");

    return *res;
}
