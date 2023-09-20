
#include "modbus_rt_platform_memory.h"

void *modbus_rt_malloc(size_t size) {
    return malloc(size);
}

void *modbus_rt_calloc(size_t num, size_t size) {
    return calloc(num, size);
}

void modbus_rt_free(void *ptr) {
    free(ptr);
}
