/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-29     SenyPC       the first version
 */
#ifndef _FREERTOS_UART_H_
#define _FREERTOS_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "modbus_rt_platform_thread.h"

#include "driver/gpio.h"
#include "driver/uart.h"

typedef struct esp32_uart {
    uart_port_t uartPort;
    uart_config_t uart_conf;
    QueueHandle_t uart_queue;
    gpio_num_t tx_port;
    gpio_num_t rx_port;
    gpio_num_t rts_port;
    gpio_num_t cts_port;
    bool event_thread_started;
} esp32_uart_t;

typedef esp32_uart_t  uart_type_dev;

//串口0，TTL
#define RTU_USING_UART0
//串口1，任意IO
#define RTU_USING_UART1
//串口3，485
#define RTU_USING_UART2


#ifdef RTU_USING_UART0
    #define UART0_TX_PIN    43
    #define UART0_RX_PIN    44
    #define UART0_RTS_PIN    UART_PIN_NO_CHANGE
    #define UART0_CTS_PIN    UART_PIN_NO_CHANGE
    extern modbus_rt_sem_t _rx0_notice;
#endif
#ifdef RTU_USING_UART1
    #define UART1_TX_PIN    18
    #define UART1_RX_PIN    17
    #define UART1_RTS_PIN    UART_PIN_NO_CHANGE
    #define UART1_CTS_PIN    UART_PIN_NO_CHANGE
    extern modbus_rt_sem_t _rx1_notice;
#endif
#ifdef RTU_USING_UART2
    #define UART2_TX_PIN    18
    #define UART2_RX_PIN    17
    #define UART2_RTS_PIN    UART_PIN_NO_CHANGE
    #define UART2_CTS_PIN    UART_PIN_NO_CHANGE
    extern modbus_rt_sem_t _rx2_notice;
#endif

extern uart_type_dev *os_uart_open(const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff);
extern int os_uart_close(uart_type_dev *serial);
extern int os_uart_send(uart_type_dev *serial, uint8_t *buf, int len);
extern int os_uart_receive(uart_type_dev *serial, uint8_t *buf, int bufsz, int timeout, int bytes_timeout);


#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_RTT_UART_RTT_UART_H_ */
