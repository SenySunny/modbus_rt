/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-04     luobeihai    first version
 */

#ifndef __APM32_MSP_H__
#define __APM32_MSP_H__

typedef enum {
    APM32F0,
    APM32F1,
    APM32E1,
    APM32F4,
}MCUTypedef;

void apm32_usart_init(void);
void apm32_msp_spi_init(void *Instance);
void apm32_msp_timer_init(void *Instance);
void apm32_msp_sdio_init(void *Instance);
void apm32_msp_can_init(void *Instance);

void apm32_msp_get_mcuid(uint32_t *id, MCUTypedef type);

#endif /* __APM32_MSP_H__ */
