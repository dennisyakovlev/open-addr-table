#include <pthread.h>

#include <tests_build/Funcs.h>

void
build_pthread()
{
    int i;
    void* ptr = &i;
    pthread_mutexattr_settype(reinterpret_cast<pthread_mutexattr_t*>(ptr), PTHREAD_MUTEX_RECURSIVE);
    pthread_mutexattr_destroy(reinterpret_cast<pthread_mutexattr_t*>(ptr));
    pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t*>(ptr));
    pthread_mutex_lock(reinterpret_cast<pthread_mutex_t*>(ptr));
    pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t*>(ptr));
    if (pthread_self()) return;
}
