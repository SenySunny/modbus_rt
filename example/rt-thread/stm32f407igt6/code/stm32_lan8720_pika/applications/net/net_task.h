/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     SenyPC       the first version
 */
#ifndef APPLICATIONS_NET_NET_TASK_H_
#define APPLICATIONS_NET_NET_TASK_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <sys/socket.h>
#include <netdev.h>

struct net_addr_info{
    ip_addr_t ip_addr;
    ip_addr_t netmask;
    ip_addr_t gw;
    ip_addr_t dns_servers[NETDEV_DNS_SERVERS_NUM];
 };

typedef struct net_addr_info * net_addr_info_t;

struct net_eth_info{
    uint8_t up_flag;                //只是知否已经连接上网络
    struct netdev *netdev;          //存储网卡的信息
    uint16_t *hwaddr_len;           //存储硬件设备地址的长度
    uint8_t *hwaddr;                //存储设备的硬件地址（一般指的时MAC或者IMEI）
    net_addr_info_t addr_info;      //存储设备的ip地址等信息

    uint16_t *hwaddr_check_len;     //是否做硬件信息绑定
    uint8_t *hwaddr_check;          //绑定设备的硬件地址（一般指的时MAC或者IMEI）
    net_addr_info_t addr_change;    //需要修改的网络信息内容
    uint16_t *ip_set;               //IP地址设置标志
    uint16_t *dhcp_set;             //开启dhcp模式

};

void net_task_entry(void *parameter);
int net_set_ip(ip_addr_t *ip, ip_addr_t *netmask, ip_addr_t *gw);
int net_set_dns_servers(ip_addr_t *dns0, ip_addr_t *dns1);
int net_set_dhcp( void );

#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_NET_NET_TASK_H_ */
