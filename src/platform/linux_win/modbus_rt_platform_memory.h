#ifndef __PKG_MODBUS_RT_PLATFORM_MEMORY_H_
#define __PKG_MODBUS_RT_PLATFORM_MEMORY_H_

#include <stdlib.h>
#include <string.h>
#include <signal.h>


#ifdef __cplusplus
extern "C" {
#endif


void *modbus_rt_malloc(size_t size);
void *modbus_rt_calloc(size_t num, size_t size);
void modbus_rt_free(void *ptr);


#ifdef __cplusplus
}
#endif

#endif /* __PKG_MODBUS_RT_PLATFORM_MEMORY_H_ */
