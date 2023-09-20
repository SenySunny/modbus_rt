/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     RT-Thread    first version
 */

#include <rtthread.h>
#include "modbus_rtu.h"
#include "net_task.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define    SERIAL_NAME      "uart2"

rtu_modbus_device_t rs = NULL;
int modbus_rtu_slave_open_test( void ) {
    int ret = MODBUS_RT_EOK;
    if(NULL == (rs = modbus_rtu(MODBUS_SLAVE))) {
        rt_kprintf("modbus_rtu_slave create error.\n");
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_rtu_set_serial(rs, SERIAL_NAME, 115200, 8, 'N', 1, 0))) {
        rt_kprintf("modbus_rtu_set_serial error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_rtu_open(rs))) {
        rt_kprintf("modbus_rtu_open error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    return ret;
}

int main(void) {

    int ret = MODBUS_RT_EOK;
    if(MODBUS_RT_EOK != (ret = modbus_rtu_slave_open_test())) {
        return ret;
    }
    rt_kprintf("modbus_rtu_slave_open success.\n");

    //启动网络线程，网络连接成功后可以创建modbus tcp slave for udp
    rt_thread_t net_task = rt_thread_create("net_task",net_task_entry, NULL, 8192, 12, 10);
    if (net_task != RT_NULL)
    {
        rt_thread_startup(net_task);
    }
    while (1) {
        rt_thread_mdelay(1000);
    }
    return RT_EOK;
}


#ifdef FINSH_USING_MSH
#include <finsh.h>
static rt_err_t mem_use(int argc, char **argv)
{
    if (argc != 1)
    {
        LOG_E("mem_use para is error.");
        return -RT_ERROR;
    }
    rt_size_t total;
    rt_size_t used;
    rt_size_t max_used;

    rt_memory_info(&total,&used,&max_used);
    rt_kprintf("used:%d\t ratio:%.2f%%\t total:%d\t max_used:%d.\n", used,((100.0*used)/total), total,max_used);
    return RT_EOK;
}

MSH_CMD_EXPORT(mem_use, print the mem used.);
#endif
