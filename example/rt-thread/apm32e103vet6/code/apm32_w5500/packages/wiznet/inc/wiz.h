/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-26     chenyong     first version
 */

#ifndef __WIZ_H__
#define __WIZ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>

#define WIZ_SW_VERSION       "2.1.0"
#define WIZ_SW_VERSION_NUM   0x020100

#ifndef WIZ_SOCKETS_NUM
#define WIZ_SOCKETS_NUM      8
#endif

#ifndef WIZ_RX_MBOX_NUM
#define WIZ_RX_MBOX_NUM      10
#endif

/* WIZnet set chip MAC address */
void wiz_user_config_mac(char *mac_buf, rt_uint8_t buf_len);
/* WIZnet initialize device and network */
int wiz_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __WIZ_H__ */
