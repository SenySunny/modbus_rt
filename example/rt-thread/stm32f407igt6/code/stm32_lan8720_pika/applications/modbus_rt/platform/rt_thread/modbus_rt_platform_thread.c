
#include "modbus_rt_platform_thread.h"
#include "modbus_rt_platform_memory.h"

#define UNUSED(x) (void)(x)

modbus_rt_thread_t *modbus_rt_thread_init( const char *name,
                                           void (*entry)(void *),
                                           void * param,
                                           unsigned int stack_size,
                                           unsigned int priority,
                                           unsigned int tick) {
    modbus_rt_thread_t *thread;
    thread = modbus_rt_calloc(sizeof(modbus_rt_thread_t), 1);
    if(RT_NULL == thread) {
        return RT_NULL;
    }

    /*modify thread creation method is dynamic creation, so thread exit rtos can recylcle the resource!*/
    thread->thread = rt_thread_create((const char *)name, entry, param, stack_size, priority, tick);

    if (thread->thread == RT_NULL) {
        return RT_NULL;
    } else {
        return thread;
    }
    return RT_NULL;
}

void modbus_rt_thread_startup(modbus_rt_thread_t* thread) {
    rt_thread_startup(thread->thread);
}
void modbus_rt_thread_stop(modbus_rt_thread_t* thread) {
    rt_thread_suspend(thread->thread);
}
void modbus_rt_thread_start(modbus_rt_thread_t* thread) {
    rt_thread_resume(thread->thread);
}
void modbus_rt_thread_destroy(modbus_rt_thread_t* thread) {
    modbus_rt_free(thread);
}

void modbus_rt_thread_sleep(unsigned int ms) {
    rt_thread_mdelay(ms);
}

int modbus_rt_mutex_init(modbus_rt_mutex_t* m) {
    m->mutex = rt_mutex_create("modbus_rt_mutex", RT_IPC_FLAG_PRIO);
    return 0;
}
int modbus_rt_mutex_lock(modbus_rt_mutex_t* m) {
    return rt_mutex_take((m->mutex), RT_WAITING_FOREVER);
}
int modbus_rt_mutex_trylock(modbus_rt_mutex_t* m) {
    return rt_mutex_trytake((m->mutex));
}
int modbus_rt_mutex_unlock(modbus_rt_mutex_t* m) {
    return rt_mutex_release((m->mutex));
}
int modbus_rt_mutex_destroy(modbus_rt_mutex_t* m) {
    return rt_mutex_delete((m->mutex));
}


int modbus_rt_sem_init(modbus_rt_sem_t* m) {
    m->sem = rt_sem_create("modbus_rt_sem", 0, RT_IPC_FLAG_PRIO);
    return 0;
}
int modbus_rt_sem_wait(modbus_rt_sem_t* m) {
    return rt_sem_take(m->sem, RT_WAITING_FOREVER);
}
int modbus_rt_sem_post(modbus_rt_sem_t* m) {
    return rt_sem_release(m->sem);
}
int modbus_rt_sem_destroy(modbus_rt_sem_t* m) {
    return rt_sem_delete(m->sem);
}


