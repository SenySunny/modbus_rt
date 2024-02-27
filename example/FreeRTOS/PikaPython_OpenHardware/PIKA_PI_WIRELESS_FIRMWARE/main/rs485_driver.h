#ifndef _RS485_H_
#define _RS485_H_

#include "pika_hal.h"
#include <stdint.h>

extern int rs485_init(PIKA_HAL_UART_BAUDRATE baudrate);
extern int rs485_write(void *buf, size_t len);
extern int rs485_read(void *buf, size_t len);

#endif
