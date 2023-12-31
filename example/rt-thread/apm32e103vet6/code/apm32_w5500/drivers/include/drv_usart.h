/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-20     Abbcc        first version
 */

#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>
#include <drv_common.h>
#include <board.h>

struct apm32_usart
{
    const char *name;
    USART_T *usartx;
    IRQn_Type irq_type;
    struct rt_serial_device serial;
};

int rt_hw_usart_init(void);

#endif  /* __DRV_USART_H__ */
