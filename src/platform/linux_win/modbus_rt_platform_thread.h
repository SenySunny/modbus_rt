#ifndef __PKG_MODBUS_RT_PLATFORM_THREAD_H_
#define __PKG_MODBUS_RT_PLATFORM_THREAD_H_

#ifdef __linux__
#include <unistd.h>
#elif defined(_WIN32)
#include <Windows.h>
    #ifndef     HAVE_STRUCT_TIMESPEC
        #define HAVE_STRUCT_TIMESPEC
    #endif
#endif

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct modbus_rt_thread {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} modbus_rt_thread_t;


typedef struct modbus_rt_mutex {
    pthread_mutex_t mutex;
} modbus_rt_mutex_t;

typedef struct modbus_rt_sem {
    sem_t sem;
} modbus_rt_sem_t;


modbus_rt_thread_t *modbus_rt_thread_init( const char *name,
                                           void (*entry)(void *),
                                           void * param,
                                           unsigned int stack_size,
                                           unsigned int priority,
                                           unsigned int tick);
void modbus_rt_thread_startup(modbus_rt_thread_t* thread);
void modbus_rt_thread_stop(modbus_rt_thread_t* thread);
void modbus_rt_thread_start(modbus_rt_thread_t* thread);
void modbus_rt_thread_destroy(modbus_rt_thread_t* thread);

void modbus_rt_thread_sleep(unsigned int ms);


int modbus_rt_mutex_init(modbus_rt_mutex_t* m);
int modbus_rt_mutex_lock(modbus_rt_mutex_t* m);
int modbus_rt_mutex_trylock(modbus_rt_mutex_t* m);
int modbus_rt_mutex_unlock(modbus_rt_mutex_t* m);
int modbus_rt_mutex_destroy(modbus_rt_mutex_t* m);

int modbus_rt_sem_init(modbus_rt_sem_t* m);
int modbus_rt_sem_getvalue(modbus_rt_sem_t* m,
                           int * sval);
int modbus_rt_sem_wait(modbus_rt_sem_t* m);
int modbus_rt_sem_post(modbus_rt_sem_t* m);
int modbus_rt_sem_destroy(modbus_rt_sem_t* m);


#ifdef __cplusplus
}
#endif

#endif /* __PKG_MODBUS_RT_PLATFORM_THREAD_H_ */
