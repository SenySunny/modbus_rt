#ifndef __PKG_MODBUS_RT_PLATFORM_SERIAL_H_
#define __PKG_MODBUS_RT_PLATFORM_SERIAL_H_

#include "modbus_config.h"

#if (MODBUS_RTU_SLAVE_ENABLE) || (MODBUS_RTU_MASTER_ENABLE)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "freertos_uart.h"

typedef struct modbus_rt_serial {
    uart_type_dev *serial_port;
    int serial;
    struct modbus_rt_serial * pre;
    struct modbus_rt_serial *next;
}modbus_rt_serial_t;

int modbus_rt_serial_open(const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff);
void modbus_rt_serial_close(int serial);
void modbus_rt_serial_send(int serial, void *buf, int len);
int modbus_rt_serial_receive(int serial, void *buf, int bufsz, const int timeout, const int bytes_timeout);



#ifdef __cplusplus
}
#endif

#endif

#endif /* __PKG_MODBUS_RT_PLATFORM_SERIAL_H_ */
