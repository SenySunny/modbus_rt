/**
 * @file    modbus_lave.h
 * @brief   modbus slave 头文件
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#ifndef __PKG_MODBUS_SLAVE_H_
#define __PKG_MODBUS_SLAVE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "modbus_config.h"


#if (MODBUS_RTU_SLAVE_ENABLE) || (MODBUS_TCP_SLAVE_ENABLE)

#include "agile_modbus.h"
#include "modbus_slave_util.h"

/**
 * @brief   modbus slave 寄存器类型
 */
typedef enum {
    CIOLS = 0x00,              //线圈地址
    INPUTS = 0x01,             //离散输入
    INPUT_REGISTERS = 0x03,    //输入寄存器
    REGISTERS = 0x04,          //保持寄存器
}modbus_register_type_t;

#endif

#ifdef __cplusplus
}
#endif

#endif /* __PKG_MODBUS_SLAVE_H_ */
