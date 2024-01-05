/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     SenyPC       the first version
 */
#ifndef APPLICATIONS_DEVICE_DEVICE_DATA_H_
#define APPLICATIONS_DEVICE_DEVICE_DATA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "device_config.h"
#include "modbus_slave.h"

extern uint8_t dev_di_data[DEV_X_BIT_NUMS];          //默认di数据
extern uint8_t dev_do_data[DEV_Y_BIT_NUMS];          //默认do数据

extern uint8_t dev_bit_data[DEV_BIT_NUMS];
extern uint8_t dev_input_bit_data[DEV_BIT_NUMS];

extern uint16_t dev_reg_data[DEV_REG_NUMS];
extern uint16_t dev_input_reg_data[DEV_REG_NUMS];

extern uint16_t dev_sp_reg_data[DEV_SP_REG_NUMS];
extern uint16_t dev_sp_input_reg_data[DEV_SP_REG_NUMS];


void dev_write_bit(uint8_t *data_addr, uint8_t data);
uint8_t dev_read_bit(uint8_t *data_addr);
int dev_write_bits(uint8_t *data_addr, int len, uint8_t *data);
int dev_read_bits(uint8_t *data_addr, int len, uint8_t *data);

void dev_write_reg(uint16_t *data_addr, uint16_t data);
uint16_t dev_read_reg(uint16_t *data_addr);
int dev_write_regs(uint16_t *data_addr, int len, uint16_t *data);
int dev_read_regs(uint16_t *data_addr, int len, uint16_t *data);
void dev_data2modbus_slave(agile_modbus_slave_util_t *util);


#ifdef __cplusplus
}
#endif


#endif /* APPLICATIONS_DEVICE_DEVICE_DATA_H_ */
