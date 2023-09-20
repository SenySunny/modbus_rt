/**
 * @file    modbus_ascii.h
 * @brief   modbus ascii 头文件
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#ifndef __PKG_MODBUS_ASCII_H_
#define __PKG_MODBUS_ASCII_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "modbus_config.h"

#if MODBUS_ASCII_SLAVE_ENABLE || MODBUS_ASCII_MASTER_ENABLE
#include "modbus_rtu.h"     //modbus ascii本身基于modbus rtu实现


typedef struct rtu_modbus_device * ascii_modbus_device_t;

#if MODBUS_ASCII_SLAVE_ENABLE

typedef struct rtu_slave_data * ascii_slave_data_t;

#endif

#if MODBUS_ASCII_MASTER_ENABLE

typedef struct rtu_master_data * ascii_master_data_t;

#endif

/**
 * @brief   modbus ascii API函数，与modbus_rtu完全相同。
 */
ascii_modbus_device_t modbus_ascii(modbus_mode_type mode);
int modbus_ascii_set_serial(ascii_modbus_device_t dev, const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff);

int modbus_ascii_open(ascii_modbus_device_t dev);
int modbus_ascii_isopen(ascii_modbus_device_t dev);
int modbus_ascii_close(ascii_modbus_device_t dev);
int modbus_ascii_destroy(ascii_modbus_device_t * pos_dev);
int modbus_ascii_excuse(ascii_modbus_device_t dev, int dir_slave, int type_function, int addr, int quantity, void *ptr_data);

#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
    int modbus_ascii_set_over_type(ascii_modbus_device_t dev, modbus_serial_over_type_t over_type);
    int modbus_ascii_set_net(ascii_modbus_device_t dev, char * ipaddr, unsigned int port, int type);
    int modbus_ascii_set_ip(ascii_modbus_device_t dev, char * ipaddr);
    int modbus_ascii_set_port(ascii_modbus_device_t dev, unsigned int port);
    int modbus_ascii_set_type(ascii_modbus_device_t dev, int type);
#endif

#if MODBUS_ASCII_SLAVE_ENABLE
    int modbus_ascii_set_addr(ascii_modbus_device_t dev, int addr);
    int modbus_ascii_set_strict(ascii_modbus_device_t dev, uint8_t strict);
    int modbus_ascii_add_block(ascii_modbus_device_t dev, modbus_register_type_t type, int data_addr, void *data, int nums);
    int modbus_ascii_set_pre_ans_callback(ascii_modbus_device_t dev, int (*pre_ans)(agile_modbus_t *, int, int,int, int));
    int modbus_ascii_set_done_callback(ascii_modbus_device_t dev, int (*done)(agile_modbus_t *, int, int,int, int));
#endif

#if MODBUS_ASCII_MASTER_ENABLE
    #if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
        int modbus_ascii_set_server(ascii_modbus_device_t dev, char* saddr, unsigned int sport);
    #endif
        int modbus_ascii_excuse_ex(ascii_modbus_device_t dev, int slave, int function,int w_addr, int w_quantity, 
                                    void *ptr_w_data, int r_addr, int r_quantity, void *ptr_r_data);
#endif


#endif

#ifdef __cplusplus
}
#endif

#endif /* __PKG_MODBUS_ASCII_H_ */
