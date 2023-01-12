#include <gtest/gtest.h>

#include <files/locks.h>

template<typename Lock>
class LockTests : public testing::Test
{
protected:

    Lock lock;

};

using MyTypes = testing::Types
<
    MmapFiles::spin_lock,
    MmapFiles::queue_lock,
    MmapFiles::mutex_lock
>;
TYPED_TEST_CASE(LockTests, MyTypes);

TYPED_TEST(LockTests, Lock)
{
    ASSERT_TRUE(this->lock.lock());
}

TYPED_TEST(LockTests, LockUnlock)
{
    ASSERT_TRUE(this->lock.lock());
    ASSERT_TRUE(this->lock.unlock());
}
