/*
 * File      : fal_cfg.h
 * This file is part of FAL (Flash Abstraction Layer) package
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 * 2020-03-20     ShineRoyal   change for stm32f411rc
 */
#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <rtconfig.h>
#include <board.h>

#define NOR_FLASH_DEV_NAME             "norflash0"
#define BLK_DEV_NAME                    "filesystem"

#define FLASH_SIZE_GRANULARITY_16K   (4 * 16 * 1024)
#define FLASH_SIZE_GRANULARITY_64K   (1 * 64 * 1024)
#define FLASH_SIZE_GRANULARITY_128K  (7 * 128 * 1024)
#define STM32_FLASH_START_ADRESS_16K  STM32_FLASH_START_ADRESS
#define STM32_FLASH_START_ADRESS_64K  (STM32_FLASH_START_ADRESS_16K + FLASH_SIZE_GRANULARITY_16K)
#define STM32_FLASH_START_ADRESS_128K (STM32_FLASH_START_ADRESS_64K + FLASH_SIZE_GRANULARITY_64K)

/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev stm32_onchip_flash_16k;
extern const struct fal_flash_dev stm32_onchip_flash_64k;
extern const struct fal_flash_dev stm32_onchip_flash_128k;
extern struct fal_flash_dev nor_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                         \
{                                                                   \
    &stm32_onchip_flash_16k,                                        \
    &stm32_onchip_flash_64k,                                        \
    &stm32_onchip_flash_128k,                                       \
    &nor_flash0,                                                    \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                              \
{                                                                   \
    {FAL_PART_MAGIC_WROD,   "bootloader",   "onchip_flash_16k",     0 ,             FLASH_SIZE_GRANULARITY_16K ,    0}, \
    {FAL_PART_MAGIC_WROD,   "param",        "onchip_flash_64k",     0 ,             FLASH_SIZE_GRANULARITY_64K ,    0}, \
    {FAL_PART_MAGIC_WROD,   "app",          "onchip_flash_128k",    0 ,             FLASH_SIZE_GRANULARITY_128K,    0}, \
    {FAL_PART_MAGIC_WORD,   "sp_reg",       NOR_FLASH_DEV_NAME,     0,              4*1024,                         0}, \
    {FAL_PART_MAGIC_WORD,   "sp_other",     NOR_FLASH_DEV_NAME,     4*1024,         1020*1024,                      0}, \
    {FAL_PART_MAGIC_WORD,   BLK_DEV_NAME,   NOR_FLASH_DEV_NAME,     1024*1024,      15*1024*1024,                   0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
