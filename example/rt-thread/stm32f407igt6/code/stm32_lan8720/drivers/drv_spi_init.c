/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-24     SenyPC       the first version
 */
#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_spi.h"

static int rt_hw_spi_flash_init(void)
{
    __HAL_RCC_GPIOG_CLK_ENABLE();
    rt_hw_spi_device_attach("spi3", "spi30", GPIOG, GPIO_PIN_6);
    if (RT_NULL == rt_sfud_flash_probe("norflash0", "spi30"))  //注册块设备，这一步可以将外部flash抽象为系统的块设备
    {
        return -RT_ERROR;
    }
    return RT_EOK;
}
/* 导出到自动初始化,组件初始化 */
INIT_DEVICE_EXPORT(rt_hw_spi_flash_init);
