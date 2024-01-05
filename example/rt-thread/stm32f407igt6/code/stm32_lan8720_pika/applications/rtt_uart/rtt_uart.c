/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-29     SenyPC       the first version
 */
#include "rtt_uart.h"
#include <ctype.h>

#ifdef RTU_USING_UART1
    rt_sem_t _rx1_notice = RT_NULL;
#endif
#ifdef RTU_USING_UART2
    rt_sem_t _rx2_notice = RT_NULL;
#endif
#ifdef RTU_USING_UART3
    rt_sem_t _rx3_notice = RT_NULL;
#endif
#ifdef RTU_USING_UART4
    rt_sem_t _rx4_notice = RT_NULL;
#endif
#ifdef RTU_USING_UART5
    rt_sem_t _rx5_notice = RT_NULL;
#endif
#ifdef RTU_USING_UART6
    rt_sem_t _rx6_notice = RT_NULL;
#endif



/**
 * @brief   串口通信回调函数
 * @param   serial 串口设备信息
 * @param   size 接收的数据长度
 * @return  RT_EOK
 */
rt_err_t uart_input(rt_device_t serial, rt_size_t size)
{
    rtt_usart *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct stm32_uart, serial);
#if defined(RTU_USING_UART1)
    if(uart->config->Instance == USART1)
    {
        rt_sem_release(_rx1_notice);
    }
#endif

#if defined(RTU_USING_UART2)
    if(uart->config->Instance == USART2)
    {
        rt_sem_release(_rx2_notice);
    }
#endif


#if defined(RTU_USING_UART3)
    if(uart->config->Instance == USART3)
    {
        rt_sem_release(_rx3_notice);
    }
#endif

#if defined(RTU_USING_UART4)
    if(uart->config->Instance == UART4)
    {
        rt_sem_release(_rx4_notice);
    }
#endif

#if defined(RTU_USING_UART5)
    if(uart->config->Instance == UART5)
    {
        rt_sem_release(_rx5_notice);
    }
#endif

#if defined(RTU_USING_UART6)
    if(uart->config->Instance == USART6)
    {
        rt_sem_release(_rx6_notice);
    }
#endif
    return RT_EOK;
}


/**
 * @brief   RS485接口 DIR引脚初始化
 * @param   serial 串口设备信息
 * @return  无
 */
void uart_rs485_dir_init(rt_device_t serial)
{
    rtt_usart *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct stm32_uart, serial);
#if defined(RTU_USING_UART1) && defined(RTU_UART1_USING_RS485)
    if(uart->config->Instance == USART1)
    {
        rt_pin_mode(UART1_RS485_RE_PIN, PIN_MODE_OUTPUT);
    }
#endif

#if defined(RTU_USING_UART2) && defined(RTU_UART2_USING_RS485)
    if(uart->config->Instance == USART2)
    {
        rt_pin_mode(UART2_RS485_RE_PIN, PIN_MODE_OUTPUT);
    }
#endif

#if defined(RTU_USING_UART3) && defined(RTU_UART3_USING_RS485)
    if(uart->config->Instance == USART3)
    {
        rt_pin_mode(UART3_RS485_RE_PIN, PIN_MODE_OUTPUT);
    }
#endif

#if defined(RTU_USING_UART4) && defined(RTU_UART4_USING_RS485)
    if(uart->config->Instance == UART4)
    {
        rt_pin_mode(UART4_RS485_RE_PIN, PIN_MODE_OUTPUT);
    }
#endif

#if defined(RTU_USING_UART5) && defined(RTU_UART5_USING_RS485)
    if(uart->config->Instance == UART5)
    {
        rt_pin_mode(UART5_RS485_RE_PIN, PIN_MODE_OUTPUT);
    }
#endif

#if defined(RTU_USING_UART6) && defined(RTU_UART6_USING_RS485)
    if(uart->config->Instance == USART6)
    {
        rt_pin_mode(UART6_RS485_RE_PIN, PIN_MODE_OUTPUT);
    }
#endif
}


/**
 * @brief   RS485接口 DIR引脚控制
 * @param   serial  串口设备信息
 * @param   dir     RS485通信方向，DIR_RX或者DIR_TX
 * @return  无
 */
void uart_rs485_dir(rt_device_t serial, rs485_dir_type dir)
{
    rtt_usart *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct stm32_uart, serial);

#if defined(RTU_USING_UART1) && defined(RTU_UART1_USING_RS485)
    if(uart->config->Instance == USART1)
    {
        if(dir == DIR_TX)
        {
            uart1_rs485_tx_en();
        }
        else
        {
            uart1_rs485_rx_en();
        }
    }
#endif

#if defined(RTU_USING_UART2) && defined(RTU_UART2_USING_RS485)
    if(uart->config->Instance == USART2)
    {
        if(dir == DIR_TX)
        {
            uart2_rs485_tx_en();
        }
        else
        {
            uart2_rs485_rx_en();
        }
    }
#endif

#if defined(RTU_USING_UART3) && defined(RTU_UART3_USING_RS485)
    if(uart->config->Instance == USART3)
    {
        if(dir == DIR_TX)
        {
            uart3_rs485_tx_en();
        }
        else
        {
            uart3_rs485_rx_en();
        }
    }
#endif

#if defined(RTU_USING_UART4) && defined(RTU_UART4_USING_RS485)
    if(uart->config->Instance == UART4)
    {
        if(dir == DIR_TX)
        {
            uart4_rs485_tx_en();
        }
        else
        {
            uart4_rs485_rx_en();
        }
    }
#endif

#if defined(RTU_USING_UART5) && defined(RTU_UART5_USING_RS485)
    if(uart->config->Instance == UART5)
    {
        if(dir == DIR_TX)
        {
            uart5_rs485_tx_en();
        }
        else
        {
            uart5_rs485_rx_en();
        }
    }
#endif

#if defined(RTU_USING_UART6) && defined(RTU_UART6_USING_RS485)
    if(uart->config->Instance == USART6)
    {
        if(dir == DIR_TX)
        {
            uart6_rs485_tx_en();
        }
        else
        {
            uart6_rs485_rx_en();
        }
    }
#endif
}

/**
 * @brief   创建通信信号量
 * @param   serial 串口设备信息
 * @return  -RT_ERROR   创建失败
 *          RT_EOK      创建成功
 */
rt_err_t uart_sem_create(rt_device_t serial)
{
    rtt_usart *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct stm32_uart, serial);
#if defined(RTU_USING_UART1)
    if(uart->config->Instance == USART1)
    {
        _rx1_notice = rt_sem_create("485_rx1", 0, RT_IPC_FLAG_FIFO);
        if (_rx1_notice == RT_NULL)
        {
            return -RT_ERROR;
        }
    }
#endif

#if defined(RTU_USING_UART2)
    if(uart->config->Instance == USART2)
    {
        _rx2_notice = rt_sem_create("485_rx2", 0, RT_IPC_FLAG_FIFO);
        if (_rx2_notice == RT_NULL)
        {
            return -RT_ERROR;
        }
    }
#endif

#if defined(RTU_USING_UART3)
    if(uart->config->Instance == USART3)
    {
        _rx3_notice = rt_sem_create("485_rx3", 0, RT_IPC_FLAG_FIFO);
        if (_rx3_notice == RT_NULL)
        {
            return -RT_ERROR;
        }
    }
#endif

#if defined(RTU_USING_UART4)
    if(uart->config->Instance == UART4)
    {
        _rx4_notice = rt_sem_create("485_rx4", 0, RT_IPC_FLAG_FIFO);
        if (_rx4_notice == RT_NULL)
        {
            return -RT_ERROR;
        }
    }
#endif

#if defined(RTU_USING_UART5)
    if(uart->config->Instance == UART5)
    {
        _rx5_notice = rt_sem_create("485_rx5", 0, RT_IPC_FLAG_FIFO);
        if (_rx5_notice == RT_NULL)
        {
            return -RT_ERROR;
        }
    }
#endif

#if defined(RTU_USING_UART6)
    if(uart->config->Instance == USART6)
    {
        _rx6_notice = rt_sem_create("485_rx6", 0, RT_IPC_FLAG_FIFO);
        if (_rx6_notice == RT_NULL)
        {
            return -RT_ERROR;
        }
    }
#endif
    return RT_EOK;
}


/**
 * @brief   创建通信信号量
 * @param   serial 串口设备信息
 * @return  void
 */
void uart_sem_delete(rt_device_t serial)
{
    rtt_usart *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct stm32_uart, serial);
#if defined(RTU_USING_UART1)
    if(uart->config->Instance == USART1)
    {
        rt_sem_delete(_rx1_notice);
    }
#endif

#if defined(RTU_USING_UART2)
    if(uart->config->Instance == USART2)
    {
        rt_sem_delete(_rx2_notice);
    }
#endif

#if defined(RTU_USING_UART3)
    if(uart->config->Instance == USART3)
    {
        rt_sem_delete(_rx3_notice);
    }
#endif

#if defined(RTU_USING_UART4)
    if(uart->config->Instance == UART4)
    {
        rt_sem_delete(_rx4_notice);
    }
#endif

#if defined(RTU_USING_UART5)
    if(uart->config->Instance == UART5)
    {
        rt_sem_delete(_rx5_notice);
    }
#endif

#if defined(RTU_USING_UART6)
    if(uart->config->Instance == USART6)
    {
        rt_sem_delete(_rx6_notice);
    }
#endif
}


/**
 * @brief   复位信号量
 * @param   serial 串口设备信息
 * @return  void
 */
void uart_sem_reset(rt_device_t serial)
{
    rtt_usart *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct stm32_uart, serial);
#if defined(RTU_USING_UART1)
    if(uart->config->Instance == USART1)
    {
        rt_sem_control(_rx1_notice, RT_IPC_CMD_RESET, RT_NULL);
    }
#endif

#if defined(RTU_USING_UART2)
    if(uart->config->Instance == USART2)
    {
        rt_sem_control(_rx2_notice, RT_IPC_CMD_RESET, RT_NULL);
    }
#endif

#if defined(RTU_USING_UART3)
    if(uart->config->Instance == USART3)
    {
        rt_sem_control(_rx3_notice, RT_IPC_CMD_RESET, RT_NULL);
    }
#endif

#if defined(RTU_USING_UART4)
    if(uart->config->Instance == UART4)
    {
        rt_sem_control(_rx4_notice, RT_IPC_CMD_RESET, RT_NULL);
    }
#endif

#if defined(RTU_USING_UART5)
    if(uart->usartx == UART5)
    {
        rt_sem_control(_rx5_notice, RT_IPC_CMD_RESET, RT_NULL);
    }
#endif

#if defined(RTU_USING_UART6)
    if(uart->config->Instance == USART6)
    {
        rt_sem_control(_rx6_notice, RT_IPC_CMD_RESET, RT_NULL);
    }
#endif
}


/**
 * @brief   挂起信号量
 * @param   serial  串口设备信息
 * @param   timeout 超时时间
 * @return  void
 */
rt_err_t uart_sem_take(rt_device_t serial, rt_int32_t time)
{
    rtt_usart *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct stm32_uart, serial);
#if defined(RTU_USING_UART1)
    if(uart->config->Instance == USART1)
    {
        return rt_sem_take(_rx1_notice, time);
    }
#endif

#if defined(RTU_USING_UART2)
    if(uart->config->Instance == USART2)
    {
        return rt_sem_take(_rx2_notice, time);
    }
#endif

#if defined(RTU_USING_UART3)
    if(uart->config->Instance == USART3)
    {
        return rt_sem_take(_rx3_notice, time);
    }
#endif

#if defined(RTU_USING_UART4)
    if(uart->config->Instance == UART4)
    {
        return rt_sem_take(_rx4_notice, time);
    }
#endif

#if defined(RTU_USING_UART5)
    if(uart->config->Instance == UART5)
    {
        return rt_sem_take(_rx5_notice, time);
    }
#endif

#if defined(RTU_USING_UART6)
    if(uart->config->Instance == USART6)
    {
        return rt_sem_take(_rx6_notice, time);
    }
#endif
    return -RT_ERROR;
}

/**
 * @brief   rtt_uart_open:          打开串口信息
 * @param   devname:                串口设备名称
 * @param   baudrate:               波特率
 * @param   parity:                 校验位：'N', 'E', 'O', 'M', 'S'
 * @param   stopbits:               停止位：1，2
 * @param   xonxoff:                控制流xonxoff开关，暂时不支持其他流控制模式
 * @return  rt_device_t:            返回成功打开的设备，如果没有打开，则返回RT_NULL
 *
 */
rt_device_t rtt_uart_open(const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff)
{
    rt_device_t serial = NULL;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;      /* 初始化配置参数 */

    (void)(xonxoff);
    serial = rt_device_find(devname);   /* 串口设备句柄 */
    if(NULL == serial) {
        return RT_NULL;
    }
    config.baud_rate = baudrate;
    config.data_bits = bytesize;
    parity = toupper(parity);
    switch(parity) {
        case 'N': {
            config.parity =  PARITY_NONE;
        } break;
        case 'E': {
            config.parity =  PARITY_EVEN;
        } break;
        case 'O': {
            config.parity =  PARITY_ODD;
        } break;
        default : {
            config.parity =  PARITY_NONE;
        } break;
    }
    config.stop_bits = stopbits;
    /* 1. 控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    /* 2. 485 DIR 引脚IO初始化,并设置为接收模式 */
    uart_rs485_dir_init(serial);
    uart_rs485_dir(serial, DIR_RX);
    /* 3. 初始化信号量 */
    if(uart_sem_create(serial) != RT_EOK) {
        return RT_NULL;
    }
    /* 4. 设置接收回调函数 */
    rt_device_set_rx_indicate(serial, uart_input);
    /* 6. 以中断接收及轮询发送模式打开串口设备 */
    if (rt_device_open(serial, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) != RT_EOK) {
        uart_sem_delete(serial);
        return RT_NULL;
    }
    return serial;
}

/**
 * @brief   rtt_uart_close:     关闭串口设备
 * @param   serial              串口设备信息
 * @return  int:                RT_EOK：成功，其他：失败
 *
 */
int rtt_uart_close(rt_device_t serial)
{
    int ret = rt_device_close(serial);
    uart_sem_delete(serial);
    return ret;
}

/**
 * @brief   rtt_uart_send   发送数据函数
 * @param   serial          串口设备信息
 * @param   buf             接收数据缓冲区
 * @param   len             发送数据长度
 * @return  int             发送完成的数据长度
 */
int rtt_uart_send(rt_device_t serial, uint8_t *buf, int len)
{
    uart_rs485_dir(serial, DIR_TX); //设置方向为发送
    rt_device_write(serial, 0, buf, len);
    uart_rs485_dir(serial, DIR_RX); //设置方向为发送
    return len;
}

/**
 * @brief   rtt_uart_receive        接收数据函数
 * @param   serial                  串口设备信息
 * @param   buf                     接收数据缓冲区
 * @param   bufsz                   接收的数据缓冲区长度
 * @param   timeout                 接收超时时间
 * @return  int                     接收的数据长度
 */
int rtt_uart_receive(rt_device_t serial, uint8_t *buf, int bufsz, int timeout, int bytes_timeout)
{
    int len = 0;
    while(1)
    {
        uart_sem_reset(serial);
        int rc = rt_device_read(serial, 0, buf + len, bufsz);
        if (rc > 0)
        {
            timeout = bytes_timeout;
            len += rc;
            bufsz -= rc;
            if (bufsz == 0)
            {
                break;
            }
            continue;
        }
        //挂起, 等待信号量, 即等待设备接收到数据, 在超时时间内没有接收到数据则直接返回超时(返回0), 如果接收到数据, 则把超时值修改为单字节超时数据
        if(uart_sem_take(serial, rt_tick_from_millisecond(timeout)) != RT_EOK)
        {
            break;
        }
        timeout = bytes_timeout;
    }
    return len;
}
