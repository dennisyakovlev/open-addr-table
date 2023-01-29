#include <gtest/gtest.h>

#include <files/locks.h>

template<typename Lock>
class RecursiveLockTest : public ::testing::Test
{
protected:

    Lock lock;

};

using MyTypes = ::testing::Types
<
    MmapFiles::spin_lock,
    MmapFiles::mutex_lock
>;
TYPED_TEST_CASE(RecursiveLockTest, MyTypes);

TYPED_TEST(RecursiveLockTest, Unlock)
{
    ASSERT_FALSE(this->lock.unlock().first);
    ASSERT_FALSE(this->lock.unlock().first);
    ASSERT_FALSE(this->lock.unlock().first);
}

TYPED_TEST(RecursiveLockTest, RecursiveLock)
{
    ASSERT_TRUE(this->lock.lock().first);
    ASSERT_TRUE(this->lock.lock().first);
    ASSERT_TRUE(this->lock.lock().first);
}

TYPED_TEST(RecursiveLockTest, RecursiveLockUnlock)
{
    ASSERT_TRUE(this->lock.lock().first);    
    ASSERT_TRUE(this->lock.lock().first);
    ASSERT_TRUE(this->lock.unlock().first);
    ASSERT_TRUE(this->lock.lock().first);
    ASSERT_TRUE(this->lock.unlock().first);
    ASSERT_TRUE(this->lock.unlock().first);
    ASSERT_FALSE(this->lock.unlock().first);
    ASSERT_TRUE(this->lock.lock().first);
}
