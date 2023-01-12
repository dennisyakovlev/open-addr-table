/*  Queue lock should actually be a queue. Need to test that the
    lock is a queue. How?
    A queue should be "fair". That is no single thread can starve
    other threads. To see that the lock is fair can use a setup
    of the following.

    - A non-atomic global number
    - Many "starve" threads which attempt to starve other threads
    - A single "nice" thread which requests the lock sometimes.
*/

#ifndef QUEUEFAIRNESS_TEST
#define QUEUEFAIRNESS_TEST

int
queue_fairness_test();

#endif
