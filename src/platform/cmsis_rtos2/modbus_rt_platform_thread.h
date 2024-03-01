#ifndef __PKG_MODBUS_RT_PLATFORM_THREAD_H_
#define __PKG_MODBUS_RT_PLATFORM_THREAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "./port/CMSIS_RTOS_V2/cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct platform_thread {
    osThreadId_t ptThread;
} modbus_rt_thread_t;

typedef struct modbus_rt_mutex {
    osMutexId_t ptMutex;
} modbus_rt_mutex_t;

typedef struct modbus_rt_sem {
    osSemaphoreId_t ptSem;
} modbus_rt_sem_t;


modbus_rt_thread_t *modbus_rt_thread_init( const char *name,
                                           void (*entry)(void *),
                                           void * param,
                                           unsigned int stack_size,
                                           unsigned int priority,
                                           unsigned int tick);
void modbus_rt_thread_startup(modbus_rt_thread_t *thread);
void modbus_rt_thread_stop(modbus_rt_thread_t *thread);
void modbus_rt_thread_start(modbus_rt_thread_t *thread);
void modbus_rt_thread_destroy(modbus_rt_thread_t *thread);

void modbus_rt_thread_sleep(unsigned int ms);


int modbus_rt_mutex_init(modbus_rt_mutex_t *m);
int modbus_rt_mutex_lock(modbus_rt_mutex_t *m);
int modbus_rt_mutex_trylock(modbus_rt_mutex_t *m);
int modbus_rt_mutex_unlock(modbus_rt_mutex_t *m);
int modbus_rt_mutex_destroy(modbus_rt_mutex_t *m);

int modbus_rt_sem_init(modbus_rt_sem_t *m);
int modbus_rt_sem_wait(modbus_rt_sem_t *m);
int modbus_rt_sem_post(modbus_rt_sem_t *m);
int modbus_rt_sem_destroy(modbus_rt_sem_t *m);


#ifdef __cplusplus
}
#endif

#endif /* __PKG_MODBUS_RT_PLATFORM_THREAD_H_ */
