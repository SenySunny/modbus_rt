
#include "modbus_rt_platform_memory.h"

void *modbus_rt_malloc(size_t size) {
    return rt_malloc(size);
}

void *modbus_rt_calloc(size_t num, size_t size) {
    return rt_calloc(num, size);
}

void modbus_rt_free(void *ptr) {
    rt_free(ptr);
}
