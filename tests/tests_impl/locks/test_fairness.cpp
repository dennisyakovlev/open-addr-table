#include <atomic>
#include <cstddef>
#include <pthread.h>

#include <gtest/gtest.h>

#include <tests_support/Vars.h>
#include <tests_support/thread_manager.h>
#include <files/locks.h>

using namespace MmapFiles;

template<typename Lock>
void*
func_nice(void* arg)
{
    thread_arg<Lock>* typed_arg = static_cast<thread_arg<Lock>*>(arg);

    while (!typed_arg->begin.load());

    /**
     * @brief the number of times this function successfully
     *        obtained the lock
     * 
     */
    std::size_t times     = 0;
    /**
     * @brief the total current "lag" of this function.
     * 
     *        Define lag as the number of "units" from right before the
     *        lock is obtained to right after. one "unit" is one increment
     *        of "total".
     *
     *        The idea is if the lock is truely fair then lag will be less
     *        then the number of threads everytime the lock is obtained
     * 
     */
    std::size_t lag       = 0;

    std::size_t expected = 1;
    while (!typed_arg->dead.compare_exchange_strong(expected, 1))
    {
        /**
         * @brief the previous total while the lock was held
         *
         */
        const auto old_total = typed_arg->total;
        typed_arg->lock.lock();

        /*  How long its been between pre lock and after lock. There
            is some margin for error as above two lines are not
            atomic, but error is quite small.
        */
        const auto curr_lag = typed_arg->total - old_total;

        /*  At most (TESTS_NUM_THREADS - 1) starve threads will
            be scheduled at the same time. There can be atmost
            TESTS_NUM_THREADS of lag which can be discarded. If
            the lag is greater than TESTS_NUM_THREADS, more
            than cycle of lock requests has passed, so the lag
            is counted.
        */
        if (curr_lag > TESTS_NUM_THREADS)
        {
            lag += curr_lag - TESTS_NUM_THREADS;
        }

        ++times;

        typed_arg->lock.unlock();

        expected = 1;
    }

    /*  This is the average number of increments after a "mod" that this
        thread was able to obtain this lock.
        If this is high, then the thread had to wait a long time to get
        the lock.
    */
    double* average_lag = new double(static_cast<double>(lag) / times);
    --typed_arg->dead;
    pthread_exit(average_lag);
}

template<typename Lock>
class QueueFairnessTest :
    public testing::Test,
    public thread_manager<Lock>
{
protected:

    QueueFairnessTest() :
        testing::Test(),
        thread_manager<Lock>(TESTS_NUM_THREADS - 1, TESTS_NUM_ITERATS)
    {
        M_tid = this->add_thread(func_nice<Lock>);
    }

    double
    lag()
    {
        return this->template return_val<double>(this->M_tid);
    }

    pthread_t M_tid;

};

using MyTypes = testing::Types<
    queue_lock<backoff_none>,
    queue_lock<backoff_userspace>
>;
TYPED_TEST_SUITE(QueueFairnessTest, MyTypes);

TYPED_TEST(QueueFairnessTest, Fairness)
{
    this->start();
    this->wait();

    /*  If the average lag is greater than 0.05 then from 20 requests,
        on average, the lock was obtained later than 1 "unit".

        See "lag" docuemntation in "func_nice"
    */
    ASSERT_LT(this->lag(), 0.05);
}
