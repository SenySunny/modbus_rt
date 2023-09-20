/**
 * @file    modbus_slave_util.h
 * @brief   Agile Modbus 软件包提供的简易从机接入头文件
 * @author  马龙伟 (2544047213@qq.com)
 * @date    2022-07-28
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */

#ifndef __PKG_MODBUS_SLAVE_UTIL_H
#define __PKG_MODBUS_SLAVE_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "modbus_config.h"

#if (MODBUS_RTU_SLAVE_ENABLE) || (MODBUS_TCP_SLAVE_ENABLE)

#include "agile_modbus.h"
#include <stdint.h>
#include <string.h>

//数据一次行最多处理的数据量
#define AGILE_MODBUS_UTIL_MAX_BITS_LEN           250
#define AGILE_MODBUS_UTIL_MAX_REGISTERS_LEN      125

/** @addtogroup UTIL
 * @{
 */

/** @addtogroup SLAVE_UTIL
 * @{
 */

/** @defgroup SLAVE_UTIL_Exported_Types Slave Util Exported Types
 * @{
 */

/**
 * @brief   从机寄存器映射结构体
 */
typedef struct agile_modbus_slave_util_map {
    int start_addr;                                       /**<起始地址 */
    void *data;                                          /**<数据的地址>*/
    int len;                                            /**<寄存器长度> */

    int (*get)(void *data_addr, int len, void *data);   /**<获取寄存器数据接口 */
    int (*set)(void *data_addr, int len, void *data);   /**<设置寄存器数据接口 */

    struct agile_modbus_slave_util_map *pre;            /**<指向前一个数据的指针 > */
    struct agile_modbus_slave_util_map *next;           /**<指向后一个数据的指针 > */
} agile_modbus_slave_util_map_t;

/**
 * @brief   从机功能结构体
 */
typedef struct agile_modbus_slave_util {
    agile_modbus_slave_util_map_t *tab_bits, *tab_bits_tail;                        /**<线圈寄存器定义 */
    agile_modbus_slave_util_map_t *tab_input_bits, *tab_input_bits_tail;            /**<离散量输入寄存器定义 */
    agile_modbus_slave_util_map_t *tab_registers, *tab_registers_tail;              /**<保持寄存器定义 */
    agile_modbus_slave_util_map_t *tab_input_registers, *tab_input_registers_tail;  /**<输入寄存器定义 */


    int (*addr_check)(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info,
                      const void *data);                                                  /**<地址检查接口 */
    int (*special_function)(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info); /**<特殊功能码处理接口 */
    int (*done)(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info,
                const void *data, int ret);                                               /**<处理结束接口 */
    
    int (*pre_ans_callback)(agile_modbus_t *ctx, int slave, int function,int addr, int quantity);                /**<添加上层的回调函数接口，应答之前 */                  
    int (*done_callback)(agile_modbus_t *ctx, int slave, int function,int addr, int quantity);                   /**<添加上层的回调函数接口，应答之后 */  
} agile_modbus_slave_util_t;

/**
 * @}
 */

/** @addtogroup SLAVE_UTIL_Exported_Functions
 * @{
 */
const agile_modbus_slave_util_map_t *get_map_by_addr(const agile_modbus_slave_util_map_t *maps, int address);
int agile_modbus_slave_util_callback(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info, const void *data);
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif

#ifdef __cplusplus
}
#endif

#endif /* __PKG_AGILE_MODBUS_SLAVE_UTIL_H */
