/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-26     chenyong     first version
 */

#include <stdlib.h>

#include <wiz.h>
#include <wiz_socket.h>
#include <W5500/w5500.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME               "wiz.dev"
#ifdef WIZ_DEBUG
#define DBG_LEVEL                      DBG_LOG
#else
#define DBG_LEVEL                      DBG_INFO
#endif /* WIZ_DEBUG */
#define DBG_COLOR
#include <rtdbg.h>

struct rt_spi_device *wiz_device = RT_NULL;

#ifndef WIZ_SPI_FREQ_MAX
#define WIZ_SPI_FREQ_MAX 50000000
#endif

static int wiz_spi_init(const char *spi_dev_name)
{
    RT_ASSERT(spi_dev_name);

    if (wiz_device != RT_NULL)
    {
        return 0;
    }

    wiz_device = (struct rt_spi_device *) rt_device_find(spi_dev_name);
    if (wiz_device == RT_NULL)
    {
        LOG_E("You should attach [%s] into SPI bus firstly.", spi_dev_name);
        return -RT_ENOSYS;
    }

    /* check SPI device type */
    RT_ASSERT(wiz_device->parent.type == RT_Device_Class_SPIDevice);

    /* configure SPI device*/
    {
        struct rt_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;  /* SPI Compatible Modes 0 */
        cfg.max_hz = WIZ_SPI_FREQ_MAX;                          /* SPI Interface with Clock Speeds Up to 40 MHz */
        rt_spi_configure(wiz_device, &cfg);
    }

    if (rt_device_open((rt_device_t) wiz_device, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        LOG_E("open WIZnet SPI device %s error.", spi_dev_name);
        return -RT_ERROR;
    }

    return RT_EOK;
}

int wiz_device_init(const char *spi_dev_name)
{
    int result = RT_EOK;

    /* WIZnet SPI device initialize */
    result = wiz_spi_init(spi_dev_name);
    if (result != RT_EOK)
    {
        LOG_E("WIZnet SPI device initialize failed.");
        return result;
    }

    return RT_EOK;
}
