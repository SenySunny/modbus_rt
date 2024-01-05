/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-29     SenyPC       the first version
 */
#ifndef APPLICATIONS_RTT_UART_RTT_UART_H_
#define APPLICATIONS_RTT_UART_RTT_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_usart.h>

typedef struct stm32_uart  rtt_usart;


#define RTU_USING_UART1
//#define RTU_UART1_USING_RS485

#define RTU_USING_UART3
//#define RTU_UART2_USING_RS485

#define RTU_USING_UART4
#define RTU_UART4_USING_RS485

//IO 序号参考drv_gpio.c的定义
#if defined(RTU_USING_UART1) && defined(RTU_UART1_USING_RS485)
#define UART1_RS485_RE_PIN          GET_PIN(A, 8)                //PA8
#define uart1_rs485_tx_en()         rt_pin_write(UART1_RS485_RE_PIN, PIN_HIGH)
#define uart1_rs485_rx_en()         rt_pin_write(UART1_RS485_RE_PIN, PIN_LOW)
#endif

#if defined(RTU_USING_UART2) && defined(RTU_UART2_USING_RS485)
#define UART2_RS485_RE_PIN          GET_PIN(E, 5)                 //PE5
#define uart2_rs485_tx_en()         rt_pin_write(UART2_RS485_RE_PIN, PIN_HIGH)
#define uart2_rs485_rx_en()         rt_pin_write(UART2_RS485_RE_PIN, PIN_LOW)
#endif

#if defined(RTU_USING_UART3) && defined(RTU_UART3_USING_RS485)
#define UART3_RS485_RE_PIN          GET_PIN(D, 3)                //PD3
#define uart3_rs485_tx_en()         rt_pin_write(UART3_RS485_RE_PIN, PIN_HIGH)
#define uart3_rs485_rx_en()         rt_pin_write(UART3_RS485_RE_PIN, PIN_LOW)
#endif

#if defined(RTU_USING_UART4) && defined(RTU_UART4_USING_RS485)
#define UART4_RS485_RE_PIN          GET_PIN(H, 9)                //PH9
#define uart4_rs485_tx_en()         rt_pin_write(UART4_RS485_RE_PIN, PIN_HIGH)
#define uart4_rs485_rx_en()         rt_pin_write(UART4_RS485_RE_PIN, PIN_LOW)
#endif

#if defined(RTU_USING_UART5) && defined(RTU_UART5_USING_RS485)
#define UART5_RS485_RE_PIN          GET_PIN(D, 4)                //PD4
#define uart5_rs485_tx_en()         rt_pin_write(UART5_RS485_RE_PIN, PIN_HIGH)
#define uart5_rs485_rx_en()         rt_pin_write(UART5_RS485_RE_PIN, PIN_LOW)
#endif

#if defined(RTU_USING_UART6) && defined(RTU_UART6_USING_RS485)
#define UART6_RS485_RE_PIN          GET_PIN(C, 3)                //PC3
#define uart6_rs485_tx_en()         rt_pin_write(UART6_RS485_RE_PIN, PIN_HIGH)
#define uart6_rs485_rx_en()         rt_pin_write(UART6_RS485_RE_PIN, PIN_LOW)
#endif


#ifdef RTU_USING_UART1
    extern rt_sem_t _rx1_notice;
#endif
#ifdef RTU_USING_UART2
    extern rt_sem_t _rx2_notice;
#endif
#ifdef RTU_USING_UART3
    extern rt_sem_t _rx3_notice;
#endif
#ifdef RTU_USING_UART4
    extern rt_sem_t _rx4_notice;
#endif
#ifdef RTU_USING_UART5
    extern rt_sem_t _rx5_notice;
#endif
#ifdef RTU_USING_UART6
    extern rt_sem_t _rx6_notice;
#endif


typedef enum {
    DIR_RX = 0,         //RS485接收模式
    DIR_TX = 1,         //RS485阻塞发送模式: 遇到串口非就绪->等待串口就绪
}rs485_dir_type;

//extern rt_err_t uart_input(rt_device_t serial, rt_size_t size);
//extern void uart_rs485_dir_init(rt_device_t serial);
//extern void uart_rs485_dir(rt_device_t serial, rs485_dir_type dir);
//extern rt_err_t uart_sem_create(rt_device_t serial);
//extern void uart_sem_delete(rt_device_t serial);
//extern void uart_sem_reset(rt_device_t serial);
//extern rt_err_t uart_sem_take(rt_device_t serial, rt_int32_t timeout);

extern rt_device_t rtt_uart_open(const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff);
extern int rtt_uart_close(rt_device_t serial);
extern int rtt_uart_send(rt_device_t serial, uint8_t *buf, int len);
extern int rtt_uart_receive(rt_device_t serial, uint8_t *buf, int bufsz, int timeout, int bytes_timeout);


#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_RTT_UART_RTT_UART_H_ */
