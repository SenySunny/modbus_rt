/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     SenyPC       the first version
 */
#ifndef DRIVERS_FAL_CFG_H_
#define DRIVERS_FAL_CFG_H_

#include <rtconfig.h>
#include <board.h>

/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev apm32_onchip_flash;
/* flash device table */
#define FAL_FLASH_DEV_TABLE                             \
{                                                       \
    &apm32_onchip_flash,                                \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                               \
{                                                                                    \
    {FAL_PART_MAGIC_WORD,   "bl",           "onchip_flash",         0,              64*1024,        0}, \
    {FAL_PART_MAGIC_WORD,   "app",          "onchip_flash",         64*1024,        444*1024,       0}, \
    {FAL_PART_MAGIC_WORD,   "sp_reg",       "onchip_flash",         508*1024,       2*1024,         0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */
#endif /* DRIVERS_FAL_CFG_H_ */
