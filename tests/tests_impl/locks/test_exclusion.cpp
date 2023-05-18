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
        thread_manager<Lock>(test_cpu_cores, test_iterations)
    {
    }

};

using MyTypes = ::testing::Types<
    queue_lock<backoff_none>,
    queue_lock<backoff_userspace>,
    spin_lock<backoff_none>,
    spin_lock<backoff_userspace>
>;
TYPED_TEST_SUITE(CorrectnessTest, MyTypes);

TYPED_TEST(CorrectnessTest, Exclusion)
{
    this->start();
    this->wait();

    ASSERT_EQ(this->arg().total, test_cpu_cores * test_iterations);
}

#endif
