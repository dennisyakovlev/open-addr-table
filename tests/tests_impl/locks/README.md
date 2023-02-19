# Lock Tests

Test the different kinds of locks for the functionality promised.

Tests will work if one thread is the maximum number of threads.

## test_acquire.cpp

Test that the queue lock is actually "fair". What "fair" is and how its tested is docuemented in "queue_lock.h" and "test_acquire.cpp" respectively.

Create some number of threads which will greedily request the lock. Then have one thread which also greedily requests the lock but also keeps track of of quickly it was able to obtain the lock.

The idea is if the lock is "fair", then the time taken to obtain the lock is bound by the number of threads.

## test_deviation.cpp

Test the queue lock is actually "fair". What "fair" is docuemented in "queue_lock.h".

Create the maximum number of threads which greedily request the lock. Each thread keeps track of number of times it obtained lock.

The idea is if the lock is "fair", then the number of times each thread obtained the locks should be about the same.

## test_exclusion.cpp

Locks should actually give exclusion to a resource.

Create the maximum number of threads and increment a regular variable in every thread a specified number of times. Before every variable increment, gain access to the lock and release afterwards. The final variable value should be (#pipelines * #iterations).

The idea is if locks did not provide proper exclusion there would be atleast one instance of a race condition on the variable. That is, two or more threads increment the variable at the same, in the end incrementing the variable by only one. This would be refelcted in the final result being smaller than expected.

This test does not gaurentee the locks are working. However, as the number of times this test is run and the number of iterations increases the chance the locks are correct increases. Running this test once with 4+ CPU cores should be enough.

## test_recursive_lock.cpp

Locks should meet recursive requirements.

These tests are sort of useless but I keep them. Reason is using a recursive lock wrong is undefined behaviour, not a certain failure.

## Strange Behaviour

Some strange behaviour I cannot explain even after looking. When used typed tests in gtest (`TYPED_TEST_SUIT`), and more than one type, the performance of **all** the tested types goes down. For example in "test_acquire.cpp" using
```
using MyTypes = testing::Types<
    queue_lock<backoff_none>,
    queue_lock<backoff_userspace>
>;
```
instead of
```
using MyTypes = testing::Types<
    queue_lock<backoff_none>
>;
```
will decrease ther performance significantly for both the types in the first snippet.

To deal with this I raise the tolerence for error with notes in the tests as to what the expected tolerence should be if using the second snippet.

Even after eveything is accounted for, the acquire and deviation tests will sometimes fail because queue lock does not provide a 100% gaurentee, only a very strong promise (see "queue_lock.h").