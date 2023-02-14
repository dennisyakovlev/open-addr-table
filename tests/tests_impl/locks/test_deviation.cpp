#include <algorithm>
#include <cstddef>
#include <memory>
#include <pthread.h>

#include <gtest/gtest.h>

#include <tests_support/Vars.h>
#include <tests_support/thread_manager.h>
#include <files/locks.h>

#include <mutex>

using namespace MmapFiles;

/**
 * @brief For this test need an atomic total. See "thread_deviation"
 *        for why.
 * 
 * @tparam Lock 
 */
template<typename Lock>
struct deviation_arg :
    public thread_arg<Lock>
{

    deviation_arg(std::size_t iterations) :
        thread_arg<Lock>(iterations),
        atomic_total(0)
    {
    }

    /**
     * @brief 
     * 
     */
    std::atomic<std::size_t> atomic_total;

};

template<typename Lock>
void*
thread_deviation(void* arg)
{
    deviation_arg<Lock>* typed_arg = static_cast<deviation_arg<Lock>*>(arg);

    while (!typed_arg->begin.load());

    std::size_t* times = new std::size_t(0);
    /*  If just used "typed_arg->total" would get undefined behaviour. So need
        an atomic total counter.
    */
    while (typed_arg->atomic_total.load() < TESTS_NUM_ITERATS * TESTS_NUM_THREADS)
    {
        typed_arg->lock.lock();
        
        ++*times;
        ++typed_arg->total;
        ++typed_arg->atomic_total;
        
        typed_arg->lock.unlock();
    }

    --typed_arg->dead;
    pthread_exit(times);
}

template<typename Lock>
class QueueDeviationTest :
    public testing::Test,
    public thread_manager<Lock, deviation_arg<Lock>>
{
protected:

    QueueDeviationTest() :
        thread_manager<Lock, deviation_arg<Lock>>(0, TESTS_NUM_ITERATS)
    {
        for (int i = 0; i != TESTS_NUM_THREADS; ++i)
        {
            M_tids.push_back(this->add_thread(thread_deviation<Lock>));
        }
    }

    std::size_t
    off_by(pthread_t tid)
    {
        return std::labs(this->template return_val<std::size_t>(tid) - TESTS_NUM_ITERATS);
    }

    std::vector<pthread_t>
        M_tids;

};

/*  So fore SOME reason, no idea why but having more than one type skews results.

    We need to somehow have MyTyped be a single type each time. This is impossible
    unless we screw with gtest internals.

    So what can do is in one file (Vars.h) can have a "MyTypes" which takes from
    a tuple depending on a macro for which element to take. Macro would be given
    when building.

    Okay but how does builder know what to set macro to? Can run a program which
    give a number which says macros should have values [0,n), then builder
    can build by setting macro in a build loop or sum.
*/

using MyTypes = testing::Types<
    queue_lock<backoff_none>,
    queue_lock<backoff_userspace>
>;
TYPED_TEST_SUITE(QueueDeviationTest, MyTypes);

TYPED_TEST(QueueDeviationTest, Deviation)
{
    this->start();
    this->wait();

    std::size_t total = 0;
    for (auto tid : this->M_tids)
    {
        total += this->off_by(tid);
    }

    /*  If total is greater than 0.01, then the number of times each thread
        obtained the lock relative to each other is greater than a 1% difference.
    */
    ASSERT_LE(total, 0.01 * (TESTS_NUM_ITERATS * TESTS_NUM_THREADS));

}
