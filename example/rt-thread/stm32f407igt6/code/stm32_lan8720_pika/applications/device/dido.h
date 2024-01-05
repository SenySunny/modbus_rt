/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     SenyPC       the first version
 */
#ifndef APPLICATIONS_DEVICE_DIDO_H_
#define APPLICATIONS_DEVICE_DIDO_H_

#ifdef __cplusplus
 extern "C" {
#endif


#include "drivers/pin.h"
#include <board.h>

#include "device_config.h"

extern uint8_t io_get_di(int pin);
extern uint8_t io_get_do(int pin);
extern void io_set_do(int pin, rt_base_t value);

extern int io_read_di(int start_addr, int nums, uint8_t *val);
extern int io_read_do(int start_addr, int nums, uint8_t *val);
extern int io_write_do(int start_addr, int nums, uint8_t *val);

#ifdef __cplusplus
}
#endif


#endif /* APPLICATIONS_DEVICE_DIDO_H_ */
