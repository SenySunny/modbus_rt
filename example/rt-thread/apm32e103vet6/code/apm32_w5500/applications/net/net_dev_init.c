/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     SenyPC       the first version
 */
#include <stdio.h>
#include <board.h>
#include "drv_spi.h"

/**
 @verbatim
    定义一个全局变量存储网络的MAC地址。
 @endverbatim
 */
char mac_eth[18] = {0};

/**
 * @brief   get_mcu_id_for_mac:             读取设备的ID并作为设备的网络MAC，具有唯一性。
  *                                                                                     需要注意的是：该MAC经作为本地设备MAC使用。
 *                                          MAC地址最高字节（MSB）的低第二位（LSb）表示这个MAC地址是全局的还是本地的，
  *                                                                                     即U/L（Universal/Local）位，如果为0，表示是全局地址。所有的OUI这一位都是0。
 *                                          MAC地址最高字节（MSB）的低第一位(LSb），表示这个MAC地址是单播还是多播。0表示单播。
 * @param   void
 * @return  int:                            0
 *
 */
static int get_mcu_id_for_mac(void) {
    uint32_t id[3];
    uint8_t *p_mac = (uint8_t *)id;
    apm32_msp_get_mcuid(id, APM32E1);
    //设备ID为12字节(96位)，这里仅仅用后6字节。
    p_mac[6] |= 0x02;       //设置该mac为本地MAC
    p_mac[6] &= 0xfe;       //设置该MAC为单播
    sprintf(mac_eth,"%X-%X-%X-%X-%X-%X",p_mac[6], p_mac[7], p_mac[8], p_mac[9], p_mac[10], p_mac[11]);
    return 0;
}
/* 导出到自动初始化,组件初始化 */
INIT_COMPONENT_EXPORT(get_mcu_id_for_mac);


/**
 * @brief   rt_hw_spi_flash_init:            w5500的spi初始化
 * @param   无
 * @return  int：                                                        RT_EOK：成功，其他：失败
 *
 */
static int rt_hw_spi_flash_init(void)
{
    //初始化w5500的SPI
    RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_GPIOC);
    return rt_hw_spi_device_attach("spi1", "spi10", GPIOC, GPIO_PIN_4);
}
/* 导出到自动初始化 */
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);

/**
 * @brief   wiz_user_config_mac:            设置wiz库中W5500的MAC地址，该函数本身位wiz.c中的虚函数，这里重新实现。
  *                                                                                     用于用户自定义mac地址
 * @param   mac_buf:                        存储mac地址的存储区
 * @param   buf_len                         MAC存储区的长度
 * @return  void
 *
 */
void wiz_user_config_mac(char *mac_buf, rt_uint8_t buf_len) {
    RT_ASSERT(mac_buf != RT_NULL);
    RT_ASSERT(buf_len > 0);
    rt_memset(mac_buf, 0x0, buf_len);
    rt_strncpy(mac_buf, mac_eth, buf_len);
}
