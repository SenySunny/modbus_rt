/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     SenyPC       the first version
 */
#ifndef APPLICATIONS_DEVICE_DEVICE_CONFIG_H_
#define APPLICATIONS_DEVICE_DEVICE_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <board.h>

#define    DEV_NAME                 "apm32_io"

#define     FLASH_PAGE_SIZE         0x800U

#define     DEV_INNER_DI_NUMS        (14)               //设备本机DI数量
#define     DEV_INNER_DO_NUMS        (10)               //设备本机DO数量


/*0x和1x寄存器的地址*/
#define     DEV_BIT_ADDR            (0)                 //0x和1x寄存器的地址
#define     DEV_X_BIT_ADDR          (10000)             //X寄存器，DI的值，1x寄存器
#define     DEV_Y_BIT_ADDR          (20000)             //Y寄存器，DO的值，0x寄存器


/*3x和4x寄存器的地址*/
#define     DEV_REG_ADDR            (0)                 //4x和3x寄存器的值
#define     DEV_SP_REG_ADDR         (8000)              //特殊4x和3x寄存器的地址

 /*0x和1x寄存器的的数量*/
#define     DEV_BIT_NUMS            (4096)              //默认设定的0x和1x寄存器的数量
#define     DEV_X_BIT_NUMS          (1024)              //最多支持DI的数量
#define     DEV_Y_BIT_NUMS          (1024)              //最多支持DO的数量

 /*3x和4x寄存器的数量*/
#define     DEV_REG_NUMS            (2048)              //默认设定的4x和3x寄存器的数量
#define     DEV_SP_REG_NUMS         (256)               //默认设置特殊4x和3x寄存器的数量

#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_DEVICE_DEVICE_CONFIG_H_ */
