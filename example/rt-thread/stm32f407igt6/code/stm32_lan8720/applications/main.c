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

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define    SERIAL_NAME      "uart3"

rtu_modbus_device_t rs = NULL;
int rtu_slave_done_callback(agile_modbus_t *ctx, int slave, int function,int addr, int quantity) {
    (void)(ctx);
    rt_kprintf("rtu_slave done. addr: %d, function: %d, addr: %d, quantity: %d.\n", slave, function, addr, quantity);
    return 0;
}

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
    if(MODBUS_RT_EOK != (ret = modbus_rtu_set_done_callback(rs,rtu_slave_done_callback))) {
        rt_kprintf("modbus_rtu_set_done_callback error, code is: %d.\n", ret);
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
