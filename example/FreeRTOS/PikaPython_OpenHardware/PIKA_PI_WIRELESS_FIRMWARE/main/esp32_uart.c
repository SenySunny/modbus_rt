/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-29     SenyPC       the first version
 */
#include "freertos_uart.h"
#include <ctype.h>
#include <string.h>

#define ESP32_UART_BUF_SIZE (1024)

#ifdef RTU_USING_UART0
    SemaphoreHandle_t _rx0_notice = NULL;
#endif
#ifdef RTU_USING_UART1
    SemaphoreHandle_t _rx1_notice = NULL;
#endif
#ifdef RTU_USING_UART2
    SemaphoreHandle_t _rx2_notice = NULL;
#endif

void uart_sem_give(uart_type_dev *serial);
/**
 * @brief   串口通信回调函数
 * @param   uart_id 串口id号
 * @param   data_len 接收的数据长度
 * @return  RT_EOK
 */
static void uart_event_task(void* pvParameters) {
    uart_type_dev *serial = (uart_type_dev *)pvParameters;
    if (NULL == serial) {
        vTaskDelete(NULL);
        return;
    }
    uart_event_t event;
    while(serial->event_thread_started) {
        if (xQueueReceive(serial->uart_queue, (void*)&event, (TickType_t)portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA: {
                    uart_sem_give(serial);
                } break;
                default: break;
            }
        }
    }
    uart_sem_give(serial);
    vTaskDelete(NULL);
}

/**
 * @brief   创建通信信号量
 * @param   serial 串口设备信息
 * @return  -RT_ERROR   创建失败
 *          RT_EOK      创建成功
 */
int uart_sem_create(uart_type_dev *serial) {
    if(NULL== serial) {
        return-1;
    }
#if defined(RTU_USING_UART0)
    if(serial->uartPort == 0) {
        _rx0_notice = xSemaphoreCreateBinary();
        if (_rx0_notice == NULL) {
            return -1;
        }
    }
#endif

#if defined(RTU_USING_UART1)
    if(serial->uartPort == 1) {
        _rx1_notice = xSemaphoreCreateBinary();
        if (_rx1_notice == NULL) {
            return -1;
        }
    }
#endif

#if defined(RTU_USING_UART2)
    if(serial->uartPort == 2) {
        _rx2_notice = xSemaphoreCreateBinary();
        if (_rx2_notice == NULL) {
            return -1;
        }
    }
#endif
    return 0;
}


/**
 * @brief   创建通信信号量
 * @param   serial 串口设备信息
 * @return  void
 */
void uart_sem_delete(uart_type_dev *serial) {
    if(NULL== serial) {
        return ;
    }
#if defined(RTU_USING_UART0)
    if(serial->uartPort == 0) {
        vSemaphoreDelete(_rx0_notice);
    }
#endif

#if defined(RTU_USING_UART1)
    if(serial->uartPort == 1) {
        vSemaphoreDelete(_rx1_notice);
    }
#endif

#if defined(RTU_USING_UART2)
    if(serial->uartPort == 2) {
        vSemaphoreDelete(_rx2_notice);
    }
#endif
}


/**
 * @brief   复位信号量
 * @param   serial 串口设备信息
 * @return  void
 */
void uart_sem_reset(uart_type_dev *serial) {
    if(NULL== serial) {
        return ;
    }
#if defined(RTU_USING_UART0)
    if(serial->uartPort == 0) {
        if(uxSemaphoreGetCount(_rx0_notice)) {
            xSemaphoreTake(_rx0_notice, 0);
        }
    }
#endif

#if defined(RTU_USING_UART1)
    if(serial->uartPort == 1) {
        if(uxSemaphoreGetCount(_rx1_notice)) {
            xSemaphoreTake(_rx1_notice, 0);
        }
    }
#endif

#if defined(RTU_USING_UART2)
    if(serial->uartPort == 2) {
        if(uxSemaphoreGetCount(_rx2_notice)) {
            xSemaphoreTake(_rx2_notice, 0);
        }
    }
#endif
}

/**
 * @brief   发送信号量
 * @param   serial  串口设备信息
 * @return  void
 */
void uart_sem_give(uart_type_dev *serial) {
    if(NULL== serial) {
        return ;
    }
#if defined(RTU_USING_UART0)
    if(serial->uartPort == 0) {
        xSemaphoreGive(_rx0_notice);
    }
#endif

#if defined(RTU_USING_UART1)
    if(serial->uartPort == 1) {
        xSemaphoreGive(_rx1_notice);
    }
#endif

#if defined(RTU_USING_UART2)
    if(serial->uartPort == 2) {
        xSemaphoreGive(_rx2_notice);
    }
#endif
}


/**
 * @brief   挂起信号量
 * @param   serial  串口设备信息
 * @param   timeout 超时时间
 * @return  void
 */
int uart_sem_take(uart_type_dev *serial, int32_t time) {
    if(NULL== serial) {
        return-1;
    }
#if defined(RTU_USING_UART0)
    if(serial->uartPort == 0) {
        return xSemaphoreTake(_rx0_notice, time);
    }
#endif

#if defined(RTU_USING_UART1)
    if(serial->uartPort == 1) {
        return xSemaphoreTake(_rx1_notice, time);
    }
#endif

#if defined(RTU_USING_UART2)
    if(serial->uartPort == 2) {
        return xSemaphoreTake(_rx2_notice, time);
    }
#endif
    return -1;
}

/**
 * @brief   设置通信管脚
 * @param   serial  串口设备信息
 * @param   timeout 超时时间
 * @return  void
 */
int uart_set_all_pins_default(uart_type_dev *serial) {
    if(NULL== serial) {
        return-1;
    }
#if defined(RTU_USING_UART0)
    if(serial->uartPort == 0) {
        serial->tx_port = UART0_TX_PIN;
        serial->rx_port = UART0_RX_PIN;
        serial->rts_port = UART0_RTS_PIN;
        serial->cts_port = UART0_CTS_PIN;
    }
#endif

#if defined(RTU_USING_UART1)
    if(serial->uartPort == 1) {
        serial->tx_port = UART1_TX_PIN;
        serial->rx_port = UART1_RX_PIN;
        serial->rts_port = UART1_RTS_PIN;
        serial->cts_port = UART1_CTS_PIN;
    }
#endif

#if defined(RTU_USING_UART2)
    if(serial->uartPort == 2) {
        serial->tx_port = UART2_TX_PIN;
        serial->rx_port = UART2_RX_PIN;
        serial->rts_port = UART2_RTS_PIN;
        serial->cts_port = UART2_CTS_PIN;
    }
#endif
    return -1;
}


/**
 * @brief   rtt_uart_open:          打开串口信息
 * @param   devname:                串口设备名称
 * @param   baudrate:               波特率
 * @param   parity:                 校验位：'N', 'E', 'O', 'M', 'S'
 * @param   stopbits:               停止位：1，2
 * @param   xonxoff:                控制流xonxoff开关，暂时不支持其他流控制模式
 * @return  uart_type_dev *:       返回成功打开的设备，如果没有打开，则返回RT_NULL
 *
 */
uart_type_dev *os_uart_open(const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff) {
    
    (void)(xonxoff);
    if(strlen(devname) != 5) {
        return NULL;
    } 
    char devname_temp[5] = {0};
    for(int i = 0; i < 4; i++) {
        devname_temp[i]=tolower(devname[i]);
    }
    if(strstr(devname_temp,"uart") != devname_temp) {
        return NULL;
    }
    uart_type_dev *serial = pvPortMalloc(sizeof(uart_type_dev));
    if(serial == NULL)  {
        return NULL;
    }
    serial->uartPort = devname[4] - '0';
    if (serial->uartPort < 0 || serial->uartPort >= UART_NUM_MAX) {
        vPortFree(serial);
        return NULL;
    }
    //Set UART pins default.
    uart_set_all_pins_default(serial);

    memset(&serial->uart_conf, 0, sizeof(serial->uart_conf));
    serial->uart_conf.baud_rate = baudrate;
    switch(bytesize) {
        case 5: {
            serial->uart_conf.data_bits =  UART_DATA_5_BITS;
        } break;
        case 6: {
            serial->uart_conf.data_bits =  UART_DATA_6_BITS;
        } break;
        case 7: {
            serial->uart_conf.data_bits =  UART_DATA_7_BITS;
        } break;
        case 8: {
            serial->uart_conf.data_bits =  UART_DATA_8_BITS;
        } break;
        default : {
            serial->uart_conf.data_bits =  UART_DATA_8_BITS;
        } break;
    }
    parity = toupper(parity);
    switch(parity) {
        case 'N': {
            serial->uart_conf.parity =  UART_PARITY_DISABLE;
        } break;
        case 'E': {
            serial->uart_conf.parity =  UART_PARITY_EVEN;
        } break;
        case 'O': {
            serial->uart_conf.parity =  UART_PARITY_ODD;
        } break;
        default : {
            serial->uart_conf.parity =  UART_PARITY_DISABLE;
        } break;
    }
    switch(stopbits) {
        case 1: {
            serial->uart_conf.stop_bits =  UART_STOP_BITS_1;
        } break;
        case 2: {
            serial->uart_conf.stop_bits =  UART_STOP_BITS_2;
        } break;
        case 3: {
            serial->uart_conf.stop_bits =  UART_STOP_BITS_1_5;
        } break;
        default : {
            serial->uart_conf.stop_bits =  UART_STOP_BITS_1;
        } break;
    }
    serial->uart_conf.flow_ctrl =  UART_HW_FLOWCTRL_DISABLE;
    serial->uart_conf.source_clk = UART_SCLK_DEFAULT;

    if(uart_sem_create(serial) != 0) {
        vPortFree(serial);
        return NULL;
    }

    //Install UART driver, and get the queue.
    uart_driver_install(serial->uartPort, ESP32_UART_BUF_SIZE * 2, ESP32_UART_BUF_SIZE * 2, 20, &serial->uart_queue, 0);
    uart_param_config(serial->uartPort, &serial->uart_conf);

    //Set UART pins.
    uart_set_pin(serial->uartPort, serial->tx_port, serial->rx_port, serial->rts_port, serial->cts_port);

    //Set uart rx event callback.
    uart_enable_rx_intr(serial->uartPort);

    serial->event_thread_started = true;
    xTaskCreate(uart_event_task, "uart_event_task", 4096, serial, 12, NULL);  
    return serial;
}

/**
 * @brief   rtt_uart_close:     关闭串口设备
 * @param   serial              串口设备信息
 * @return  int:                RT_EOK：成功，其他：失败
 *
 */
int os_uart_close(uart_type_dev *serial) {
    serial->event_thread_started = false;
    uart_sem_take(serial, (TickType_t)portMAX_DELAY);
    int ret = uart_driver_delete(serial->uartPort);
    uart_sem_delete(serial);
    vPortFree(serial);
    return ret;
}

/**
 * @brief   rtt_uart_send   发送数据函数
 * @param   serial          串口设备信息
 * @param   buf             接收数据缓冲区
 * @param   len             发送数据长度
 * @return  int             发送完成的数据长度
 */
int os_uart_send(uart_type_dev *serial, uint8_t *buf, int len) {
    uart_write_bytes(serial->uartPort, buf, len);
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
int os_uart_receive(uart_type_dev *serial, uint8_t *buf, int bufsz, int timeout, int bytes_timeout) {
    int len = 0;
    while(1) {
        uart_sem_reset(serial);
        int rc = uart_read_bytes(serial->uartPort, buf + len, bufsz, 0);
        if (rc > 0) {
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
        if(uart_sem_take(serial, timeout/portTICK_PERIOD_MS) != 1) {
            break;
        }
        timeout = bytes_timeout;
    }
    return len;
}
