
#include "modbus_rt_platform_memory.h"
#include "FreeRTOS.h"

void *modbus_rt_malloc(size_t size) {
    return pvPortMalloc(size);
}

void *modbus_rt_calloc(size_t num, size_t size) {
    return pvPortMalloc(num * size);
}

void modbus_rt_free(void *ptr) {
    vPortFree(ptr);
}
