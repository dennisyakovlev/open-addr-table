# Lock Tests

Test the different kinds of locks for the functionality promised.

Tests will work if one thread is the maximum number of threads.

There's a sortof assumption that is okay if it doesnt hold. Assumption is that

## test_exclusion.cpp

Locks should actually give exclusion to a resource.

Create the maximum number of threads and increment a regular variable in every thread a specified number of times. Before every variable increment, gain access to the lock and release afterwards. The final variable value should be (#pipelines * #iterations).

The idea is if locks did not provide proper exclusion there would be atleast one instance of a race condition on the variable. That is, two or more threads increment the variable at the same, in the end incrementing the variable by only one. This would be refelcted in the final result being smaller than expected.

This test does not gaurentee the locks are working. However, as the number of times this test is run and the number of iterations increases the chance the locks are correct increases. Running this test once with 4+ CPU cores should be enough.

## test_fairness.cpp

Test that the queue lock is actually "fair".

What "fair" is and how its tested is docuemented in "queue_lock.h" and "test_fairness.cpp" respectively.

