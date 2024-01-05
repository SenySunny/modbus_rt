/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-07     lyon       the first version
 */

#include <rtthread.h>
#include "PikaPlatform.h"
#include "device_config.h"
#include <unistd.h>
#include <fcntl.h>

struct rt_memheap ccm_heap;

static int pika_mem_init(void)
{
    rt_memheap_init(&ccm_heap, "ccmheap", (void *)0x10000000, 64*1024);
    rt_kprintf("ccmheap memheap init ok!\n");
    return RT_EOK;
}
INIT_ENV_EXPORT(pika_mem_init);

char __platform_getchar(void){
    char ch = 0;
    if(read(STDIN_FILENO, &ch, 1) > 0) {
        return ch;
    } else {
        return -1; /* EOF */
    }
    return -1;
}


int __platform_vsprintf(char* buff, char* fmt, va_list args){
    return rt_vsprintf(buff, fmt, args);
}
int __platform_vsnprintf(char* buff, size_t size, const char* fmt, va_list args){
    return rt_vsnprintf(buff, size, fmt, args);
}
void* __platform_malloc(size_t size) {
    return rt_memheap_alloc(&ccm_heap, size);
//    return rt_malloc(size);
}

void* __platform_realloc(void* ptr, size_t size) {
    return rt_memheap_realloc(&ccm_heap, ptr, size);
//    return rt_realloc(ptr, size);
}

void* __platform_calloc(size_t num, size_t size) {
    return rt_memheap_alloc(&ccm_heap, size);
//    return rt_calloc(num, size);
}

void __platform_free(void* ptr) {
    rt_memheap_free(ptr);
//    rt_free(ptr);
}
void* __platform_memset(void* mem, int ch, size_t size) {
    return rt_memset(mem, ch, size);
}
void* __platform_memcpy(void* dir, const void* src, size_t size) {
    return rt_memcpy(dir, src, size);
}

FILE *pika_platform_fopen(const char *filename, const char *modes) {
  return fopen(filename, modes);
}

int pika_platform_fclose(FILE *fp) {
    return fclose(fp);
}

int pika_platform_fseek(FILE *fp, long offset, int whence) {
  return fseek(fp, offset, whence);
}

long pika_platform_ftell(FILE *fp) {
    return ftell(fp);
}

size_t pika_platform_fread(void *ptr, size_t size, size_t count, FILE *fp) {
  return fread(ptr, size, count, fp);
}

size_t pika_platform_fwrite(const void *ptr, size_t size, size_t count,
                            FILE *fp) {
  return fwrite(ptr, size, count, fp);
}


int gethostname(char *name, size_t len)
{
    int dev_name_len= strlen(DEV_NAME);
    if(len >= 16) {
        memcpy(name, DEV_NAME, dev_name_len);
        name[dev_name_len] = 0;
    }
    return dev_name_len;
}

void pika_platform_sleep_ms(uint32_t ms) {
    rt_thread_mdelay(ms);
}


//void pikaRTThread_Thread_mdelay(PikaObj *self, int ms){
//    rt_thread_mdelay(ms);
//}
//
//void pikaRTThread_Task_platformGetTick(PikaObj *self){
//    obj_setInt(self, "tick", rt_tick_get());
//}

