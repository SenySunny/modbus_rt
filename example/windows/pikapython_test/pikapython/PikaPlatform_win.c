#ifdef PIKA_PLATFORM_NO_WEAK

#include "pikaScript.h"
#include "PikaPlatform.h"
#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) && !defined(CROSS_BUILD)
#include <Windows.h>
#ifndef HAVE_STRUCT_TIMESPEC
    #define HAVE_STRUCT_TIMESPEC
#endif
#include <pthread.h>
#endif

#if defined(QT_STD)
#include "qt_std.h"
#endif

#ifdef _WIN32

#ifdef WIN32_LEAN_AND_MEAN
#include <winioctl.h>
struct timeval
{
    long tv_sec;     // 秒
    long tv_usec;    // 微秒
};
#endif

typedef struct pika_platform_thread_win {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} pika_platform_thread_win_t;


void  usleep(unsigned long usec){
    HANDLE timer;
    LARGE_INTEGER interval;
    interval.QuadPart = (10 * usec);
    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &interval, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}

int gettimeofday(struct timeval *tp, void *tzp){
  time_t clock;
  struct tm tm;
  SYSTEMTIME wtm;
  GetLocalTime(&wtm);
  tm.tm_year   = wtm.wYear - 1900;
  tm.tm_mon   = wtm.wMonth - 1;
  tm.tm_mday   = wtm.wDay;
  tm.tm_hour   = wtm.wHour;
  tm.tm_min   = wtm.wMinute;
  tm.tm_sec   = wtm.wSecond;
  tm. tm_isdst  = -1;
  clock = mktime(&tm);
  tp->tv_sec = clock;
  tp->tv_usec = wtm.wMilliseconds * 1000;
  return (0);
}

void timeradd(struct timeval *a, struct timeval *b, struct timeval *res){
    res->tv_sec = a->tv_sec + b->tv_sec;
    res->tv_usec = a->tv_usec + b->tv_usec;
    if (res->tv_usec >= 1000000) {
        res->tv_sec += res->tv_usec / 1000000;
        res->tv_usec %= 1000000;
    }
}

void timersub(struct timeval *a, struct timeval *b, struct timeval *res){
    res->tv_sec = a->tv_sec - b->tv_sec;
    res->tv_usec = a->tv_usec - b->tv_usec;
    if (res->tv_usec < 0) {
        res->tv_sec -= 1;
        res->tv_usec += 1000000;
    }
}

#endif

PIKA_WEAK void pika_platform_disable_irq_handle(void) {
    /* disable irq to support thread */
}

PIKA_WEAK void pika_platform_enable_irq_handle(void) {
    /* disable irq to support thread */
}

PIKA_WEAK void* pika_platform_malloc(size_t size) {
    return malloc(size);
}

PIKA_WEAK void* pika_platform_realloc(void* ptr, size_t size) {
    return realloc(ptr, size);
}

PIKA_WEAK void* pika_platform_calloc(size_t num, size_t size) {
    return calloc(num, size);
}

PIKA_WEAK void pika_platform_free(void* ptr) {
    free(ptr);
}

PIKA_WEAK void* pika_user_malloc(size_t size) {
    return pika_platform_malloc(size);
}

PIKA_WEAK void pika_user_free(void* ptr, size_t size) {
    pika_platform_free(ptr);
}

PIKA_WEAK void pika_platform_error_handle() {
    return;
}

PIKA_WEAK void pika_platform_panic_handle() {
    while (1) {
    };
}

PIKA_WEAK uint8_t pika_is_locked_pikaMemory(void) {
    return 0;
}

PIKA_WEAK int64_t pika_platform_get_tick(void) {
#if PIKA_FREERTOS_ENABLE
    return platform_uptime_ms();
#elif defined(__linux)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#elif defined(_WIN32)
    LARGE_INTEGER frequency;        // ticks per second
    LARGE_INTEGER t;                // ticks
    int64_t tick;

    // get ticks per second
    QueryPerformanceFrequency(&frequency);

    // get current time
    QueryPerformanceCounter(&t);

    // convert the time to milliseconds
    tick = (int64_t)((t.QuadPart * 1000.0) / frequency.QuadPart);

    return tick;
#else
    return -1;
#endif
}

PIKA_WEAK int pika_platform_fflush(void* stream) {
#if PIKA_UNBUFFERED_ENABLE
    return fflush(stream);
#else
    return 0;
#endif
}

PIKA_WEAK int pika_platform_putchar(char ch) {
#if defined(QT_STD)
    return qt_putchar(ch);
#else
    return putchar(ch);
#endif

}

#ifndef pika_platform_printf
PIKA_WEAK void pika_platform_printf(char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    pika_vprintf(fmt, args);
    va_end(args);
}
#endif

PIKA_WEAK char* pika_platform_strdup(const char* src) {
    char* dst = (char*)pika_platform_malloc(strlen(src) + 1);
    if (dst) {
        strcpy(dst, src);
    }
    return dst;
}

PIKA_WEAK size_t pika_platform_tick_from_millisecond(size_t ms) {
    return ms;
}

PIKA_WEAK int pika_platform_vsnprintf(char* buff,
                                      size_t size,
                                      const char* fmt,
                                      va_list args) {
    return vsnprintf(buff, size, fmt, args);
}

PIKA_WEAK void pika_platform_wait(void) {
    while (1) {
    };
}

PIKA_WEAK void* pika_platform_memset(void* mem, int ch, size_t size) {
    return memset(mem, ch, size);
}

PIKA_WEAK void* pika_platform_memcpy(void* dir, const void* src, size_t size) {
    return memcpy(dir, src, size);
}

PIKA_WEAK int pika_platform_memcmp(const void* s1, const void* s2, size_t n) {
    return memcmp(s1, s2, n);
}

PIKA_WEAK void* pika_platform_memmove(void* s1, void* s2, size_t n) {
    return memmove(s1, s2, n);
}

PIKA_WEAK char pika_platform_getchar(void) {
#if defined(__linux) || defined(_WIN32)
#if defined(QT_STD)
    return qt_getchar();
#else
    return getchar();
#endif
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR_LOWLEVEL();
#endif
}

/* fopen */
PIKA_WEAK FILE* pika_platform_fopen(const char* filename, const char* modes) {
#if defined(__linux) || defined(_WIN32)
    return fopen(filename, modes);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR_LOWLEVEL();
#endif
}

/* fclose */
PIKA_WEAK int pika_platform_fclose(FILE* stream) {
#if defined(__linux) || defined(_WIN32)
    return fclose(stream);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR_LOWLEVEL();
#endif
}

/* fwrite */
PIKA_WEAK size_t pika_platform_fwrite(const void* ptr,
                                      size_t size,
                                      size_t n,
                                      FILE* stream) {
    pika_assert(NULL != stream);
#if defined(__linux) || defined(_WIN32)
    return fwrite(ptr, size, n, stream);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR_LOWLEVEL();
#endif
}

/* fread */
PIKA_WEAK size_t pika_platform_fread(void* ptr,
                                     size_t size,
                                     size_t n,
                                     FILE* stream) {
#if defined(__linux) || defined(_WIN32)
    return fread(ptr, size, n, stream);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR_LOWLEVEL(_);
#endif
}

/* fseek */
PIKA_WEAK int pika_platform_fseek(FILE* stream, long offset, int whence) {
#if defined(__linux) || defined(_WIN32)
    return fseek(stream, offset, whence);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR_LOWLEVEL(_);
#endif
}

/* ftell */
PIKA_WEAK long pika_platform_ftell(FILE* stream) {
#if defined(__linux) || defined(_WIN32)
    return ftell(stream);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR_LOWLEVEL(_);
#endif
}

PIKA_WEAK void pika_hook_instruct(void) {
    return;
}

PIKA_WEAK PIKA_BOOL pika_hook_arg_cache_filter(void* self) {
    return PIKA_TRUE;
}

PIKA_WEAK void pika_thread_idle_hook(void) {
    return;
}

PIKA_WEAK void pika_platform_thread_yield(void) {
    pika_thread_idle_hook();
#if PIKA_FREERTOS_ENABLE
    vTaskDelay(1);
#elif defined(__linux) || defined(_WIN32)
    return;
#else
    return;
#endif
}

PIKA_WEAK void pika_platform_sleep_ms(uint32_t ms) {
#if defined(__linux)
    usleep(ms * 1000);
#elif defined(_WIN32) && !defined(CROSS_BUILD)
    Sleep(ms);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR_LOWLEVEL(_);
#endif
}

/* Thread Support */
PIKA_WEAK pika_platform_thread_t* pika_platform_thread_init(
    const char* name,
    void (*entry)(void*),
    void* const param,
    unsigned int stack_size,
    unsigned int priority,
    unsigned int tick) {
#ifdef __linux
    int res;
    pika_platform_thread_t* thread;
    void* (*thread_entry)(void*);

    thread_entry = (void* (*)(void*))entry;
    thread = pikaMalloc(sizeof(pika_platform_thread_t));

    res = pthread_create(&thread->thread, NULL, thread_entry, param);
    if (res != 0) {
        pikaFree(thread, sizeof(pika_platform_thread_t));
    }

    thread->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    thread->cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    return thread;
#elif _WIN32
    int res;
    pika_platform_thread_t* thread;

    void* (*thread_entry)(void*);

    thread_entry = (void* (*)(void*))entry;
    thread = pikaMalloc(sizeof(pika_platform_thread_t));
    thread->platform_data = pikaMalloc(sizeof(pika_platform_thread_win_t));

    pika_platform_thread_win_t *thread_win = (pika_platform_thread_win_t *)(thread->platform_data);
    res = pthread_create(&thread_win->thread, NULL, thread_entry, param);
    if (res != 0) {
        pikaFree(thread->platform_data, sizeof(pika_platform_thread_win_t));
        pikaFree(thread, sizeof(pika_platform_thread_t));
    }

    thread_win->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    thread_win->cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    return thread;
#elif PIKA_FREERTOS_ENABLE
    pika_platform_thread_t* thread;

    thread = pikaMalloc(sizeof(pika_platform_thread_t));
#if PIKA_THREAD_MALLOC_STACK_ENABLE
    thread->thread_stack_size = stack_size;
    thread->thread_stack = pikaMalloc(thread->thread_stack_size);
#endif

    (void)tick;

#if PIKA_THREAD_MALLOC_STACK_ENABLE
    thread->thread =
        xTaskCreateStatic(entry, name, stack_size, param, priority,
                          thread->thread_stack, &thread->task_buffer);
#else
    int err =
        xTaskCreate(entry, name, stack_size, param, priority, &thread->thread);
#endif

#if PIKA_THREAD_MALLOC_STACK_ENABLE
    if (NULL == thread->thread) {
        pikaFree(thread->thread_stack, thread->thread_stack_size);
        pikaFree(thread, sizeof(pika_platform_thread_t));
        return NULL;
    }
#else
    if (pdPASS != err) {
        pikaFree(thread, sizeof(pika_platform_thread_t));
        return NULL;
    }
#endif

    return thread;
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return NULL;
#endif
}

PIKA_WEAK uint64_t pika_platform_thread_self(void) {
#ifdef __linux
    return (uint64_t)pthread_self();
#elif _WIN32
    return (uint64_t)(pthread_self().p);
#elif PIKA_FREERTOS_ENABLE
    return (uint64_t)xTaskGetCurrentTaskHandle();
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return 0;
#endif
}

PIKA_WEAK void pika_platform_thread_startup(pika_platform_thread_t* thread) {
    (void)thread;
}

PIKA_WEAK void pika_platform_thread_stop(pika_platform_thread_t* thread) {
#ifdef __linux
    pthread_mutex_lock(&(thread->mutex));
    pthread_cond_wait(&(thread->cond), &(thread->mutex));
    pthread_mutex_unlock(&(thread->mutex));
#elif _WIN32
    pika_platform_thread_win_t *thread_win = (pika_platform_thread_win_t *)(thread->platform_data);
    pthread_mutex_lock(&(thread_win->mutex));
    pthread_cond_wait(&(thread_win->cond), &(thread_win->mutex));
    pthread_mutex_unlock(&(thread_win->mutex));
#elif PIKA_FREERTOS_ENABLE
    vTaskSuspend(thread->thread);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
#endif
}

PIKA_WEAK void pika_platform_thread_start(pika_platform_thread_t* thread) {
#ifdef __linux
    pthread_mutex_lock(&(thread->mutex));
    pthread_cond_signal(&(thread->cond));
    pthread_mutex_unlock(&(thread->mutex));
#elif _WIN32
    pika_platform_thread_win_t *thread_win = (pika_platform_thread_win_t *)(thread->platform_data);
    pthread_mutex_lock(&(thread_win->mutex));
    pthread_cond_signal(&(thread_win->cond));
    pthread_mutex_unlock(&(thread_win->mutex));
#elif PIKA_FREERTOS_ENABLE
    vTaskResume(thread->thread);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
#endif
}

PIKA_WEAK void pika_platform_thread_destroy(pika_platform_thread_t* thread) {
#ifdef __linux
    if (NULL != thread) {
        pthread_detach(thread->thread);
        // pthread_join(thread->thread, NULL);
        pikaFree(thread, sizeof(pika_platform_thread_t));
        thread = NULL;
        return;
    }
#elif _WIN32
    if (NULL != thread) {
        pika_platform_thread_win_t *thread_win = (pika_platform_thread_win_t *)(thread->platform_data);
        pthread_detach(thread_win->thread);
        // pthread_join(thread->thread, NULL);
        pikaFree(thread_win, sizeof(pika_platform_thread_win_t));
        pikaFree(thread, sizeof(pika_platform_thread_t));
        thread = NULL;
        return;
    }
#elif PIKA_FREERTOS_ENABLE
    if (NULL != thread) {
        vTaskDelete(thread->thread);
        pikaFree(thread, sizeof(pika_platform_thread_t));
        return;
    }
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
#endif
}

PIKA_WEAK void pika_platform_thread_exit(pika_platform_thread_t* thread) {
#if defined(__linux) || defined(_WIN32)
    pika_platform_thread_destroy(thread);
#elif PIKA_FREERTOS_ENABLE
    // vTaskDelete(NULL);  // test on esp32c3
    if (NULL == thread) {
        vTaskDelete(NULL);
        return;
    }
    vTaskDelete(thread->thread);
    return;
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
#endif
}

PIKA_WEAK int pika_platform_thread_mutex_init(pika_platform_thread_mutex_t* m) {
#if defined(__linux) || defined(_WIN32)
    return pthread_mutex_init((pthread_mutex_t *)(&(m->mutex)), NULL);
#elif PIKA_FREERTOS_ENABLE
    m->mutex = xSemaphoreCreateMutex();
    return 0;
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return -1;
#endif
}

PIKA_WEAK int pika_platform_thread_mutex_lock(pika_platform_thread_mutex_t* m) {
#if defined(__linux) || defined(_WIN32)
    return pthread_mutex_lock((pthread_mutex_t *)(&(m->mutex)));
#elif PIKA_FREERTOS_ENABLE
    return xSemaphoreTake(m->mutex, portMAX_DELAY);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return -1;
#endif
}

PIKA_WEAK int pika_platform_thread_mutex_trylock(
    pika_platform_thread_mutex_t* m) {
#if defined(__linux) || defined(_WIN32)
    return pthread_mutex_trylock((pthread_mutex_t *)(&(m->mutex)));
#elif PIKA_FREERTOS_ENABLE
    return xSemaphoreTake(m->mutex, 0);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return -1;
#endif
}

PIKA_WEAK int pika_platform_thread_mutex_unlock(
    pika_platform_thread_mutex_t* m) {
#if defined(__linux) || defined(_WIN32)
    return pthread_mutex_unlock((pthread_mutex_t *)(&(m->mutex)));
#elif PIKA_FREERTOS_ENABLE
    return xSemaphoreGive(m->mutex);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return -1;
#endif
}

PIKA_WEAK int pika_platform_thread_mutex_destroy(
    pika_platform_thread_mutex_t* m) {
#if defined(__linux) || defined(_WIN32)
    return pthread_mutex_destroy((pthread_mutex_t *)(&(m->mutex)));
#elif PIKA_FREERTOS_ENABLE
    vSemaphoreDelete(m->mutex);
    return 0;
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return -1;
#endif
}

PIKA_WEAK void pika_platform_thread_timer_init(pika_platform_timer_t* timer) {
#ifdef __linux
    timer->time = (struct timeval){0, 0};
#elif _WIN32
    timer->platform_data = pikaMalloc(sizeof(struct timeval));
    *((struct timeval *)(timer->platform_data)) = (struct timeval){0, 0};
 #elif PIKA_FREERTOS_ENABLE
    timer->time = 0;
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
#endif
}

PIKA_WEAK void pika_platform_thread_timer_cutdown(pika_platform_timer_t* timer,
                                                  unsigned int timeout) {
#ifdef __linux
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
    timeradd(&now, &interval, &timer->time);
#elif _WIN32
    struct timeval *timer_temp = (struct timeval *)timer->platform_data;
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
    timeradd(&now, &interval, timer_temp);
#elif PIKA_FREERTOS_ENABLE
    timer->time = platform_uptime_ms();
    timer->time += timeout;
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
#endif
}

PIKA_WEAK char pika_platform_thread_timer_is_expired(
    pika_platform_timer_t* timer) {
#ifdef __linux
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->time, &now, &res);
    return ((res.tv_sec < 0) || (res.tv_sec == 0 && res.tv_usec <= 0));
#elif _WIN32
    struct timeval *timer_temp = (struct timeval *)timer->platform_data;
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(timer_temp, &now, &res);
    return ((res.tv_sec < 0) || (res.tv_sec == 0 && res.tv_usec <= 0));
#elif _WIN32
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub((struct timeval *)timer->platform_data, &now, &res);
    return ((res.tv_sec < 0) || (res.tv_sec == 0 && res.tv_usec <= 0));
#elif PIKA_FREERTOS_ENABLE
    return platform_uptime_ms() > timer->time ? 1 : 0;
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return 1;
#endif
}

PIKA_WEAK int pika_platform_thread_timer_remain(pika_platform_timer_t* timer) {
#ifdef __linux
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->time, &now, &res);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
#elif _WIN32
    struct timeval *timer_temp = (struct timeval *)timer->platform_data;
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(timer_temp, &now, &res);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
#elif PIKA_FREERTOS_ENABLE
    uint32_t now;
    now = platform_uptime_ms();
    if (timer->time <= now) {
        return 0;
    }
    return timer->time - now;
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return -1;
#endif
}

PIKA_WEAK unsigned long pika_platform_thread_timer_now(void) {
#if defined(__linux) || defined(_WIN32)
    return (unsigned long)time(NULL);
#elif PIKA_FREERTOS_ENABLE
    return (unsigned long)platform_uptime_ms();
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return 1;
#endif
}

PIKA_WEAK void pika_platform_thread_timer_usleep(unsigned long usec) {
#if defined(__linux) || defined(_WIN32)
    usleep(usec);
#elif PIKA_FREERTOS_ENABLE
    TickType_t tick = 1;
    if (usec != 0) {
        tick = usec / portTICK_PERIOD_MS;

        if (tick == 0)
            tick = 1;
    }
    vTaskDelay(tick);
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
#endif
}

PIKA_WEAK int pika_platform_thread_timer_destroy(
    pika_platform_timer_t* timer) {
#ifdef __linux
    return 0;
#elif _WIN32
    struct timeval *timer_temp = (struct timeval *)timer->platform_data;
    pikaFree(timer_temp, sizeof(struct timeval));
    return 0;
#elif PIKA_FREERTOS_ENABLE
    return 0;
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
    return -1;
#endif
}

PIKA_WEAK void pika_platform_reboot(void) {
#if defined(__linux) || defined(_WIN32)
    pika_platform_printf("reboot\n");
#else
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
#endif
}

PIKA_WEAK void pika_platform_clear(void) {
    WEAK_FUNCTION_NEED_OVERRIDE_ERROR();
}

PIKA_WEAK void pika_platform_abort_handler(void) {
    return;
}

#endif

