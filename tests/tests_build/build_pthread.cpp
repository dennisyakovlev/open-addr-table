#include <pthread.h>

#include <tests_build/Funcs.h>

void
build_pthread()
{
    pthread_mutexattr_settype(nullptr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutexattr_destroy(nullptr);
    pthread_mutex_destroy(nullptr);
    pthread_mutex_lock(nullptr);
    pthread_mutex_unlock(nullptr);
    pthread_self();
}
