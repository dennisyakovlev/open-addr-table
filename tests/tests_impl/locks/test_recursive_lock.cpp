#include <gtest/gtest.h>

#include <files/locks.h>

using namespace MmapFiles;

template<typename Lock>
class RecursiveLockTest :
    public testing::Test
{
protected:

    Lock lock;

};

using MyTypes = ::testing::Types
<
    spin_lock<backoff_none>,
    spin_lock<backoff_userspace>
>;
TYPED_TEST_CASE(RecursiveLockTest, MyTypes);

TYPED_TEST(RecursiveLockTest, Unlock)
{
    this->lock.unlock();
    this->lock.unlock();
    this->lock.unlock();
}

TYPED_TEST(RecursiveLockTest, RecursiveLock)
{
    this->lock.lock();
    this->lock.lock();
    this->lock.lock();
}

TYPED_TEST(RecursiveLockTest, RecursiveLockUnlock)
{
    this->lock.lock();    
    this->lock.lock();
    this->lock.unlock();
    this->lock.lock();
    this->lock.unlock();
    this->lock.unlock();
    this->lock.unlock();
    this->lock.lock();
}
