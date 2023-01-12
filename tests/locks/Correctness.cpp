#include <tuple>

#include <files/locks.h>
#include "Correctness.h"
#include "Defs.h"

int
correctness_tests()
{
    using Tuple = std::tuple
    <
        MmapFiles::spin_lock,
        MmapFiles::mutex_lock,
        MmapFiles::queue_lock
    >;

    PRINT_BEGIN_TESTS("LockCorrectnessTests");
    int res = TestLocks<Tuple, std::tuple_size<Tuple>::value>::TestLock();
    PRINT_END_TESTS("LockCorrectnessTests");

    return res;
}
