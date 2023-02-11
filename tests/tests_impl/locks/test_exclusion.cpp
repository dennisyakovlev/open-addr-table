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

#include <tests_support/thread_manager.h>
#include <tests_support/Vars.h>
#include <files/locks.h>

using namespace MmapFiles;

template<typename Lock>
class CorrectnessTest :
    public testing::Test,
    public thread_manager<Lock>
{
protected:

    CorrectnessTest() :
        thread_manager<Lock>(TESTS_NUM_THREADS, TESTS_NUM_ITERATS)
    {
    }

};

using MyTypes = ::testing::Types<
    queue_lock<backoff_none>,
    queue_lock<backoff_userspace>,
    queue_lock<backoff_futex>,
    spin_lock<backoff_none>,
    spin_lock<backoff_userspace>,
    spin_lock<backoff_futex>
>;
TYPED_TEST_CASE(CorrectnessTest, MyTypes);

TYPED_TEST(CorrectnessTest, Exclusion)
{
    this->start();
    this->wait();

    ASSERT_EQ(this->arg().total, TESTS_NUM_THREADS * TESTS_NUM_ITERATS);

    ASSERT_TRUE(this->state())
        << "test was in invalid state, some error occured";
}

#endif
