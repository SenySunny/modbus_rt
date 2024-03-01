
#include "modbus_rt_platform_thread.h"
#include "modbus_rt_platform_memory.h"

#define UNUSED(x) (void)(x)

modbus_rt_thread_t *modbus_rt_thread_init( const char *name,
                                           void (*entry)(void *),
                                           void * param,
                                           unsigned int stack_size,
                                           unsigned int priority,
                                           unsigned int tick) {
    UNUSED(tick);
    modbus_rt_thread_t *thread = NULL;
    thread = modbus_rt_malloc(sizeof(modbus_rt_thread_t));
    if(NULL == thread) {
        return NULL;
    }

    osThreadAttr_t tAttr = {
        .name = name,
        .attr_bits = osThreadDetached,
        .cb_mem = NULL,
        .cb_size = 0u,
        .stack_mem = NULL,
        .stack_size = stack_size,
        .priority = priority,
        .tz_module = 0u,
        .reserved = 0u
    };
    /*modify thread creation method is dynamic creation, so thread exit rtos can recylcle the resource!*/
    thread->ptThread = osThreadNew(entry, param, (const osThreadAttr_t*)&tAttr);

    if (thread == NULL) {
        return NULL;
    } else {
        return thread;
    }
}

void modbus_rt_thread_startup(modbus_rt_thread_t *thread) {
    UNUSED(thread);
}
void modbus_rt_thread_stop(modbus_rt_thread_t *thread) {
    osThreadSuspend(thread->ptThread);
}
void modbus_rt_thread_start(modbus_rt_thread_t *thread) {
    osThreadResume(thread->ptThread);
}
void modbus_rt_thread_destroy(modbus_rt_thread_t *thread) {
    osThreadTerminate(thread->ptThread);
}

void modbus_rt_thread_sleep(unsigned int ms) {
    osDelay(ms);
}

int modbus_rt_mutex_init(modbus_rt_mutex_t *m) {
    m->ptMutex = osMutexNew(NULL);
    return 0;
}
int modbus_rt_mutex_lock(modbus_rt_mutex_t *m) {
    return osMutexAcquire(m->ptMutex, osWaitForever);
}
int modbus_rt_mutex_trylock(modbus_rt_mutex_t *m) {
    UNUSED(m);
    return 0;
}
int modbus_rt_mutex_unlock(modbus_rt_mutex_t *m) {
    return osMutexRelease(m->ptMutex);
}
int modbus_rt_mutex_destroy(modbus_rt_mutex_t *m) {
    return osMutexDelete(m->ptMutex);
}


int modbus_rt_sem_init(modbus_rt_sem_t *m) {
    m->ptSem = osSemaphoreNew(3, 0, NULL);
    return 0;
}
int modbus_rt_sem_wait(modbus_rt_sem_t *m) {
    return osSemaphoreAcquire(m->ptSem, osWaitForever);
}
int modbus_rt_sem_post(modbus_rt_sem_t *m) {
    return osSemaphoreRelease(m->ptSem);
}
int modbus_rt_sem_destroy(modbus_rt_sem_t *m) {
    return osSemaphoreDelete(m->ptSem);
}


