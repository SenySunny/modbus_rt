/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-04     luobeihai    first version
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <rtthread.h>
#include <rthw.h>
#ifdef RT_USING_DEVICE
    #include <rtdevice.h>
#endif

#include "apm32_msp.h"
#include "apm32e10x_gpio.h"
#include "apm32e10x_rcm.h"
#include "apm32e10x_misc.h"
#include "apm32e10x_rcm.h"
#include "apm32e10x_eint.h"
#include "apm32e10x_usart.h"
#include "apm32e10x_dma.h"
#include "apm32e10x_fmc.h"
#include "apm32e10x_dmc.h"

#if defined(RT_USING_ADC)
    #include "apm32e10x_adc.h"
#endif
#if defined(RT_USING_DAC)
    #include "apm32e10x_dac.h"
#endif
#if defined(RT_USING_RTC)
    #include "apm32e10x_rtc.h"
    #include "apm32e10x_pmu.h"
#endif
#if defined(RT_USING_SPI)
    #include "apm32e10x_spi.h"
#endif
#if defined(RT_USING_HWTIMER) || defined(RT_USING_PWM)
    #include "apm32e10x_tmr.h"
#endif
#if defined(RT_USING_WDT)
    #include "apm32e10x_iwdt.h"
    #include "apm32e10x_wwdt.h"
#endif
#if defined(RT_USING_SDIO)
    #include "apm32e10x_sdio.h"
#endif
#if defined(RT_USING_CAN)
    #include "apm32e10x_can.h"
#endif

#include "drv_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APM32_FLASH_START_ADRESS       ROM_START
#define APM32_FLASH_SIZE               ROM_SIZE
#define APM32_FLASH_END_ADDRESS        ROM_END

#define APM32_SRAM_SIZE                RAM_SIZE
#define APM32_SRAM_START               RAM_START
#define APM32_SRAM_END                 RAM_END

#if defined(__ARMCC_VERSION)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="CSTACK"
#define HEAP_BEGIN      (__segment_end("CSTACK"))
#else
extern int __bss_end;
#define HEAP_BEGIN      ((void *)&__bss_end)
#endif

#define HEAP_END        APM32_SRAM_END

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
}
#endif

#endif
