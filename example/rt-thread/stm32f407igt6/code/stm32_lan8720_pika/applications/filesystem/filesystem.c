/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-04     SenyPC       the first version
 */
#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include <dfs_fs.h>
#include "fal.h"
#include "drv_common.h"

#define DBG_TAG "app.filesystem"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

/* 外部SPI Flash 分区挂载 */
int filesystem_mount(void) {
    if(rt_device_find(BLK_DEV_NAME) != RT_NULL)  {
        if (RT_EOK != dfs_mount(BLK_DEV_NAME, "/", "elm", 0, 0)) {
            if(RT_EOK != dfs_mkfs("elm", BLK_DEV_NAME)) {    //加载失败，可以尝试先格式化磁盘
                rt_kprintf(BLK_DEV_NAME" mkfs failed!\n");
                return -RT_ERROR;
            }
            //格式化之后重新加载设备
            if (RT_EOK != dfs_mount(BLK_DEV_NAME, "/", "elm", 0, 0)) {
                rt_kprintf(BLK_DEV_NAME" mount to '/' failed!\n");
                return -RT_ERROR;
            }
        }
        rt_kprintf(BLK_DEV_NAME" mount to '/'\n");
    }
    return RT_EOK;
}

/* 外部SPI Flash 分区卸载 */
int filesystem_unmount(void) {
    if(rt_device_find(BLK_DEV_NAME) != RT_NULL)  {
        if (RT_EOK == dfs_unmount("/")) {
           rt_kprintf(BLK_DEV_NAME"unmount ok!\n");
        } else {
           rt_kprintf(BLK_DEV_NAME"unmount fail!\n");
        }
    }
    return RT_EOK;
}

static int mount_init(void)
{
//    rt_thread_delay(RT_TICK_PER_SECOND);
    return filesystem_mount();    /* 挂载 SPI Flash 为 根目录 */
}
INIT_ENV_EXPORT(mount_init);


