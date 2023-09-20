
#include "modbus_rt_platform_thread.h"
#include "modbus_rt_platform_memory.h"

#define UNUSED(x) (void)(x)

modbus_rt_thread_t *modbus_rt_thread_init( const char *name,
                                           void (*entry)(void *),
                                           void * param,
                                           unsigned int stack_size,
                                           unsigned int priority,
                                           unsigned int tick) {
    int res;
    modbus_rt_thread_t *thread;
    void *(*thread_entry) (void *);

    UNUSED(name);
    UNUSED(stack_size);
    UNUSED(priority);
    UNUSED(tick);

    thread_entry = (void *(*)(void*))entry;
    thread = modbus_rt_malloc(sizeof(modbus_rt_thread_t));
    if(NULL == thread) {
        return NULL;
    }

    res = pthread_create(&thread->thread, NULL, thread_entry, param);
    if(0 != res) {
        modbus_rt_free(thread);
        return NULL;
    }

    thread->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    thread->cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    return thread;

}
void modbus_rt_thread_startup(modbus_rt_thread_t* thread) {
    (void) thread;
}
void modbus_rt_thread_stop(modbus_rt_thread_t* thread) {
    pthread_mutex_lock(&(thread->mutex));
    pthread_cond_wait(&(thread->cond), &(thread->mutex));
    pthread_mutex_unlock(&(thread->mutex));
}
void modbus_rt_thread_start(modbus_rt_thread_t* thread) {
    pthread_mutex_lock(&(thread->mutex));
    pthread_cond_signal(&(thread->cond));
    pthread_mutex_unlock(&(thread->mutex));
}
void modbus_rt_thread_destroy(modbus_rt_thread_t* thread) {
    if (NULL != thread) {
        pthread_detach(thread->thread);
        modbus_rt_free(thread);
    }
}

void modbus_rt_thread_sleep(unsigned int ms) {
#if defined(__linux__)
    usleep(ms * 1000);
#elif defined(_WIN32)
    Sleep(ms);
#endif
}

int modbus_rt_mutex_init(modbus_rt_mutex_t* m) {
    return pthread_mutex_init(&(m->mutex), NULL);
}
int modbus_rt_mutex_lock(modbus_rt_mutex_t* m) {
    return pthread_mutex_lock(&(m->mutex));
}
int modbus_rt_mutex_trylock(modbus_rt_mutex_t* m) {
    return pthread_mutex_trylock(&(m->mutex));
}
int modbus_rt_mutex_unlock(modbus_rt_mutex_t* m) {
    return pthread_mutex_unlock(&(m->mutex));
}
int modbus_rt_mutex_destroy(modbus_rt_mutex_t* m) {
    return pthread_mutex_destroy(&(m->mutex));
}


int modbus_rt_sem_init(modbus_rt_sem_t* m) {
    return sem_init(&(m->sem), 0, 0);
}
int modbus_rt_sem_getvalue(modbus_rt_sem_t* m, int * sval) {
    return sem_getvalue(&(m->sem), sval);
}
int modbus_rt_sem_wait(modbus_rt_sem_t* m) {
    return sem_wait(&(m->sem));
}
int modbus_rt_sem_post(modbus_rt_sem_t* m) {
    return sem_post(&(m->sem));
}
int modbus_rt_sem_destroy(modbus_rt_sem_t* m) {
    return sem_destroy(&(m->sem));
}


