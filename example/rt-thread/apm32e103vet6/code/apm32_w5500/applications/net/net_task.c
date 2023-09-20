/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     SenyPC       the first version
 */
#include <rtdevice.h>
#include "net_task.h"

#include "fal.h"
#include "device_data.h"
#include "modbus_tcp.h"

#define DBG_TAG "app.net"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define NET_CHANGE_ADDR_START   2008
#define NET_CHANGE_ADDR_LEN     24

#define NET_ADD_OFFSET           8



#define NETDEV_MAC_MAX_LEN    6
#define NETDEV_IMEI_MAX_LEN   8

#ifdef PKG_USING_WIZNET
//此处主要目的是为了防止在DHCP模式下，DHCP没有成功，这个时候想要修改静态IP时使用
int net_restart_status = 0;     //置1表示重启w5500。重启完成后归零,详细见net_restart函数

extern struct rt_work *dhcp_work;
extern int wiz_socket_init(void);
extern void DHCP_stop(void);
#endif

/**
 @verbatim
    dhcp_flag: 表示是否默认开启DHCP服务，默认不开启，也就是默认采用静态IP的方法
    后面定义的参数位默认的IP，子网掩码和网关信息
 @endverbatim
 */
static int dhcp_flag = 0;
#define     STATIC_IP           "192.168.27.166"
#define     STATIC_NETMASK      "255.255.255.0"
#define     STATIC_GW           "192.168.27.1"
#define     STATIC_DNS0         "192.168.27.1"
#define     STATIC_DNS1         "0.0.0.0"

/**
 @verbatim
    自定义网络相关通信数据
 @endverbatim
 */
struct net_eth_info g_net_info = {0};

void  mem_use_print( void ) {
    rt_size_t total;
    rt_size_t used;
    rt_size_t max_used;
    rt_memory_info(&total,&used,&max_used);
    rt_kprintf("used:%d\t ratio:%.2f%%\t total:%d\t max_used:%d.\n", used,((100.0*used)/total), total,max_used);
}

void eth_info_print( struct netdev *netdev) {
    rt_ubase_t index;
    rt_kprintf("Successfully connected the network.\r\n");
    rt_kprintf ("\t MAC: %02X-%02X-%02X-%02X-%02X-%02X\r\n",
            netdev->hwaddr[0],netdev->hwaddr[1],netdev->hwaddr[2],
            netdev->hwaddr[3],netdev->hwaddr[4],netdev->hwaddr[5]);
    rt_kprintf("\t ip address: %s\r\n", inet_ntoa(netdev->ip_addr));
    rt_kprintf("\t gw address: %s\r\n", inet_ntoa(netdev->gw));
    rt_kprintf("\t net mask  : %s\r\n", inet_ntoa(netdev->netmask));
    for (index = 0; index < NETDEV_DNS_SERVERS_NUM; index++) {
        rt_kprintf("\t dns server #%d: %s\r\n", index, inet_ntoa(netdev->dns_servers[index]));
    }
}

int net_hwaddr_changed(struct netdev *netdev, uint8_t *hwaddr) {
    if((netdev->hwaddr[0] != hwaddr[0]) || (netdev->hwaddr[1] != hwaddr[1]) ||
            (netdev->hwaddr[2] != hwaddr[2]) || (netdev->hwaddr[3] != hwaddr[3]) ||
           (netdev->hwaddr[4] != hwaddr[4]) || (netdev->hwaddr[5] != hwaddr[5])) {
        return 1;
    }
    return 0;
}

void net_save_addr_info( void ) {
    struct netdev *netdev = g_net_info.netdev;
    if(net_hwaddr_changed(netdev, g_net_info.hwaddr) ||
            (netdev->ip_addr.addr != g_net_info.addr_info->ip_addr.addr) ||
            (netdev->netmask.addr != g_net_info.addr_info->netmask.addr) ||
            (netdev->gw.addr != g_net_info.addr_info->gw.addr) ||
            (netdev->dns_servers[0].addr != g_net_info.addr_info->dns_servers[0].addr) ||
            (netdev->dns_servers[1].addr != g_net_info.addr_info->dns_servers[1].addr)){
        memcpy(g_net_info.hwaddr, netdev->hwaddr, NETDEV_MAC_MAX_LEN);
        g_net_info.addr_info->ip_addr = netdev->ip_addr;
        g_net_info.addr_info->netmask = netdev->netmask;
        g_net_info.addr_info->gw = netdev->gw;
        int offset = NET_ADD_OFFSET * 2;
        uint8_t *buf_temp = rt_calloc(1,FLASH_PAGE_SIZE);
        const struct fal_partition * dev_partition = fal_partition_find("sp_reg");
        fal_partition_read(dev_partition, 0, buf_temp, FLASH_PAGE_SIZE);
        memcpy(&(buf_temp[offset]), g_net_info.hwaddr_len, 2);
        memcpy(&(buf_temp[offset + 2]), g_net_info.hwaddr, NETDEV_HWADDR_MAX_LEN);
        memcpy(&(buf_temp[offset + 16]), g_net_info.addr_info, sizeof(struct net_addr_info));
        fal_partition_erase_all(dev_partition);
        fal_partition_write(dev_partition, 0, buf_temp, FLASH_PAGE_SIZE);
        rt_free(buf_temp);
    }
}



//重启网络，用于网线断开之后重新接入时使用
static void net_restart(int dhcp_status) {
    //等待网线连线
    while(netdev_is_link_up(g_net_info.netdev) == 0) {
        rt_thread_mdelay(100);
    }
    rt_thread_mdelay(100);         //检测到网线连接需要等待1s
    //如果默认不是dhcp，则直接关闭DHCP，并设置静态IP
    if(dhcp_status == 0) {       //如果是静态ip地址，则需要重新设置ip和端口号

        net_restart_status = 1;
        rt_thread_mdelay(100);
        while(net_restart_status != 0) {
           rt_thread_mdelay(100);
        }

        *g_net_info.ip_set = 5;
        net_set_ip(&(g_net_info.addr_change->ip_addr), &(g_net_info.addr_change->netmask), &(g_net_info.addr_change->gw));
        if(5 == *g_net_info.ip_set) {
            net_set_dns_servers(&(g_net_info.addr_change->dns_servers[0]), &(g_net_info.addr_change->dns_servers[1]));
        }
        *g_net_info.ip_set = 0;
    }
}


tcp_modbus_device_t ts = NULL;
tcp_modbus_device_t tsu = NULL;
//清除网络应用程序的缓存
int net_tcpiptask_clear( void ) {
    int ret = MODBUS_RT_EOK;
    if(NULL != ts) {
        ret = modbus_tcp_destroy(&ts);
        if(MODBUS_RT_EOK != ret) {
            rt_kprintf("modbus_tcp_destroy for tcp slave error,code:%d.\n", ret);
            return ret;
        }
        rt_kprintf("modbus_tcp_destroy for tcp slave ok.\n");
    }
    if(NULL != tsu) {
        ret = modbus_tcp_destroy(&tsu);
        if(MODBUS_RT_EOK != ret) {
            rt_kprintf("modbus_tcp_destroy for udp slave error,code:%d.\n", ret);
            return ret;
        }
        rt_kprintf("modbus_tcp_destroy for udp slave ok.\n");
    }
    mem_use_print();
    return ret;
}

int modbus_tcp_slave_open_test( void ) {
    int ret = MODBUS_RT_EOK;
    if(NULL == (ts = modbus_tcp(MODBUS_SLAVE))) {
        printf("modbus_tcp create error.\n");
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_set_net(ts, inet_ntoa(g_net_info.netdev->ip_addr), 502, SOCK_STREAM))) {
        printf("modbus_tco_set_net error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_open(ts))) {
        printf("modbus_tcp_open error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    return ret;
}

int modbus_tcp_slave_for_udp_open_test( void ) {
    int ret = MODBUS_RT_EOK;
    if(NULL == (tsu = modbus_tcp(MODBUS_SLAVE))) {
        printf("modbus_tcp create error.\n");
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_set_net(tsu, NULL, 502, SOCK_DGRAM))) {
        printf("modbus_tcp_set_net error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_set_strict(tsu, 0))) {
        printf("modbus_tcp_set_net error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_open(tsu))) {
        printf("modbus_tcp_open error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    return ret;
}

//网络应用程序重新启动
int net_tcpiptask_restart( void ) {
    int ret = MODBUS_RT_EOK;
    if(MODBUS_RT_EOK != (ret = modbus_tcp_slave_open_test())) {
        return ret;
    }
    printf("modbus_tcp_slave_open success.\n");
    if(MODBUS_RT_EOK != (ret = modbus_tcp_slave_for_udp_open_test())) {
        return ret;
    }
    printf("modbus_tcp_slave_for_udp_open success.\n");
    mem_use_print();
    return ret;
}

void net_task_entry(void *parameter) {
    g_net_info.up_flag = 0;                                 //上电初始化，表示未连接到网络上
    g_net_info.netdev = netdev_get_by_name("W5500");           //获取网卡信息
    if(NULL == g_net_info.netdev) {                         //网卡信息获取失败，直接退出
        rt_kprintf("netdev is NULL\r\n");
        return ;
    }
    g_net_info.hwaddr_len = (&dev_sp_input_reg_data[NET_ADD_OFFSET]);
    g_net_info.hwaddr = (uint8_t *)(&dev_sp_input_reg_data[NET_ADD_OFFSET + 1]);     //网关信息的地址
    g_net_info.addr_info = (net_addr_info_t)(&dev_sp_input_reg_data[NET_ADD_OFFSET + 8]);     //网关信息的地址

    memset(&dev_reg_data[NET_CHANGE_ADDR_START], 0, NET_CHANGE_ADDR_LEN << 1);
    g_net_info.hwaddr_check_len = (&dev_reg_data[NET_CHANGE_ADDR_START]);                        //是否做硬件信息绑定
    g_net_info.hwaddr_check = (uint8_t *)(&dev_reg_data[NET_CHANGE_ADDR_START + 1]);                 //绑定设备的硬件地址（一般指的时MAC或者IMEI）
    g_net_info.addr_change = (net_addr_info_t)(&dev_reg_data[NET_CHANGE_ADDR_START + 8]);            //modbus修改网络地址
    g_net_info.ip_set = &dev_reg_data[NET_CHANGE_ADDR_START + 18];                                    //修改网络标志位
    g_net_info.dhcp_set = &dev_reg_data[NET_CHANGE_ADDR_START + 19];                                  //修改网络标志位
    *g_net_info.hwaddr_len = NETDEV_MAC_MAX_LEN;
    if(0 == dhcp_flag) {
        //如果本身没有存储网络地址
        if((0 == g_net_info.addr_info->ip_addr.addr) || (0xffffffff == g_net_info.addr_info->ip_addr.addr)) {
            inet_aton(STATIC_IP, &(g_net_info.addr_change->ip_addr));
            inet_aton(STATIC_NETMASK, &(g_net_info.addr_change->netmask));
            inet_aton(STATIC_GW, &(g_net_info.addr_change->gw));
            inet_aton(STATIC_DNS0, &(g_net_info.addr_change->dns_servers[0]));
            inet_aton(STATIC_DNS1, &(g_net_info.addr_change->dns_servers[1]));
        } else {
            g_net_info.addr_change->ip_addr = g_net_info.addr_info->ip_addr;
            g_net_info.addr_change->netmask = g_net_info.addr_info->netmask;
            g_net_info.addr_change->gw = g_net_info.addr_info->gw;
            if(0xffffffff == g_net_info.addr_info->dns_servers[0].addr) {
                g_net_info.addr_info->dns_servers[0].addr = g_net_info.addr_info->gw.addr;
            }
            if(0xffffffff == g_net_info.addr_info->dns_servers[1].addr) {
                g_net_info.addr_info->dns_servers[1].addr = 0;
            }
            g_net_info.addr_change->dns_servers[0] = g_net_info.addr_info->dns_servers[0];
            g_net_info.addr_change->dns_servers[1] = g_net_info.addr_info->dns_servers[1];
        }
    }
    net_restart(dhcp_flag);                  //等待网线接入，网络重启
    while(1) {
        //网络断开(网线被拔掉)
        if(0 == netdev_is_link_up(g_net_info.netdev)) {      //检测到网线断开
            g_net_info.up_flag = 0;           //网络断开，则IP地址获取标志位清零
            net_tcpiptask_clear();            //清除网络应用程序的堆栈空间
            rt_thread_mdelay(100);
            net_restart(dhcp_flag);                  //等待网线接入，网络重启
        }
        //已经连接上网络（获取到IP地址）
        if((0 == g_net_info.up_flag) && (g_net_info.netdev->ip_addr.addr)) {
            rt_thread_mdelay(20);
            net_save_addr_info();
            eth_info_print(g_net_info.netdev);
            g_net_info.up_flag = 1;
            net_tcpiptask_restart();
        }
        //收到DHCP指令，表示开启网络DHCP功能
        if(1 == (*g_net_info.dhcp_set)) {
            if(NETDEV_MAC_MAX_LEN == (*g_net_info.hwaddr_check_len)) {
                if(net_hwaddr_changed(g_net_info.netdev, g_net_info.hwaddr_check)) {
                    rt_kprintf("MAC CHECK ERROR.\r\n");
                    memset(&dev_reg_data[NET_CHANGE_ADDR_START], 0, NET_CHANGE_ADDR_LEN << 1);
                } else {
                    rt_kprintf("set network dhcp for mac check.\r\n");
                    net_set_dhcp();
                    memset(&dev_reg_data[NET_CHANGE_ADDR_START], 0, NET_CHANGE_ADDR_LEN << 1);
                }
            } else {
                rt_kprintf("set network dhcp.\r\n");
                net_set_dhcp();
                memset(&dev_reg_data[NET_CHANGE_ADDR_START], 0, NET_CHANGE_ADDR_LEN << 1);
            }
        }
        //收到修改IP的命令
        if(0 < (*g_net_info.ip_set)) {
            if(NETDEV_MAC_MAX_LEN == (*g_net_info.hwaddr_check_len)) {
                if(net_hwaddr_changed(g_net_info.netdev, g_net_info.hwaddr_check)) {
                    rt_kprintf("MAC CHECK ERROR.\r\n");
                    memset(&dev_reg_data[NET_CHANGE_ADDR_START], 0, NET_CHANGE_ADDR_LEN << 1);
                } else {
                    rt_kprintf("set network static ip for mac check,ip:%s.\r\n", inet_ntoa(g_net_info.addr_change->ip_addr));
                    net_set_ip(&(g_net_info.addr_change->ip_addr), &(g_net_info.addr_change->netmask), &(g_net_info.addr_change->gw));
                    if(4 == *g_net_info.ip_set) {
                        net_set_dns_servers(&(g_net_info.addr_change->dns_servers[0]), NULL);
                    } else if(5 == *g_net_info.ip_set) {
                        net_set_dns_servers(&(g_net_info.addr_change->dns_servers[0]), &(g_net_info.addr_change->dns_servers[1]));
                    }
                    memset(&dev_reg_data[NET_CHANGE_ADDR_START], 0, NET_CHANGE_ADDR_LEN << 1);
                }
            } else {
                rt_kprintf("set network static ip:%s.\r\n", inet_ntoa(g_net_info.addr_change->ip_addr));
                net_set_ip(&(g_net_info.addr_change->ip_addr), &(g_net_info.addr_change->netmask), &(g_net_info.addr_change->gw));
                if(4 == *g_net_info.ip_set) {
                    net_set_dns_servers(&(g_net_info.addr_change->dns_servers[0]), NULL);
                } else if(5 == *g_net_info.ip_set) {
                    net_set_dns_servers(&(g_net_info.addr_change->dns_servers[0]), &(g_net_info.addr_change->dns_servers[1]));
                }
                memset(&dev_reg_data[NET_CHANGE_ADDR_START], 0, NET_CHANGE_ADDR_LEN << 1);
            }
        }
        //日常调度，1s检测一次
        rt_thread_mdelay(1000);
    }
}

int net_set_ip(ip_addr_t *ip, ip_addr_t *netmask, ip_addr_t *gw) {
    rt_err_t state = RT_EOK;
    ip_addr_t IP_update;
    ip_addr_t Mask_update;
    ip_addr_t GW_updata;
    dhcp_flag = 0;
    if(0 >= (*g_net_info.ip_set)) {
        return -RT_ERROR;
    }
    if(1 <= (*g_net_info.ip_set)) {
        if((NULL == ip) || (0 == ip->addr) || (0xffffffff == ip->addr)) {
            state = -RT_ERROR;
            return state;
        }
        IP_update = *ip;
        Mask_update.addr = 0x00ffffff;
        GW_updata = *ip;
        GW_updata.addr &= 0x00ffffff;
        GW_updata.addr |= 0x01000000;
    }
    if(2 <= (*g_net_info.ip_set)) {
        if((NULL == netmask) || (0 == netmask->addr) || (0xffffffff == netmask->addr)) {
            Mask_update.addr = 0x00ffffff;
        } else {
            Mask_update = *netmask;
        }
        GW_updata = *ip;
        GW_updata.addr &= 0x00ffffff;
        GW_updata.addr |= 0x01000000;
    }
    if(3 <= (*g_net_info.ip_set)) {
        if((NULL == gw) || (0 == gw->addr) || (0xffffffff == gw->addr)) {
            GW_updata = *ip;
            GW_updata.addr &= 0x00ffffff;
            GW_updata.addr |= 0x01000000;
        } else {
            GW_updata = *gw;
        }
    }

    net_tcpiptask_clear();            //断开网络前，先关闭网络应用

    netdev_set_down(g_net_info.netdev);             //断网
    g_net_info.up_flag = 0;                         //网络断开，则IP地址获取标志位清零
    //关闭dhcp
    if(netdev_is_dhcp_enabled(g_net_info.netdev)) {
        netdev_dhcp_enabled(g_net_info.netdev, RT_FALSE);
    }

    /* 如果只是关闭DHCP但是并没有关闭DHCP的工作队列，会导致如果上次开启DHCP之后，
        * 如果并没有动态获取到IP（连接的是交换机，而非路由器），会导致DHCP并没有完全关闭
        * 导致静态IP设置失败 */
    rt_work_cancel(dhcp_work);
    rt_thread_mdelay(10);

    netdev_set_ipaddr(g_net_info.netdev, &IP_update);
    netdev_set_netmask(g_net_info.netdev, &Mask_update);
    netdev_set_gw(g_net_info.netdev, &GW_updata);

    netdev_set_up(g_net_info.netdev);
    rt_thread_mdelay(100);
    return state;
}

int net_set_dns_servers(ip_addr_t *dns0, ip_addr_t *dns1) {
    rt_err_t state = RT_EOK;
    ip_addr_t dns;
    if(dns0 != NULL) {
        dns = *dns0;
        netdev_set_dns_server(g_net_info.netdev, 0, &dns);
        rt_thread_mdelay(10);
    }
    rt_thread_mdelay(100);
    if(dns1 != NULL) {
        dns = *dns1;
        netdev_set_dns_server(g_net_info.netdev, 1, &dns);
        rt_thread_mdelay(10);
    }
    return state;
}

int net_set_dhcp( void ) {
    ip_addr_t ip_zero = {0};

    net_tcpiptask_clear();            //断开网络前，先关闭网络应用

    if(dhcp_flag == 0) {
        dhcp_flag = 1;
    }

    netdev_set_down(g_net_info.netdev);      //断网
    g_net_info.up_flag = 0;                //网络断开，则IP地址获取标志位清零

    if(!netdev_is_dhcp_enabled(g_net_info.netdev)) {
        netdev_set_dns_server(g_net_info.netdev, 0, &ip_zero);
        netdev_set_dns_server(g_net_info.netdev, 1, &ip_zero);

        netdev_set_ipaddr(g_net_info.netdev, &ip_zero);
        netdev_set_netmask(g_net_info.netdev, &ip_zero);
        netdev_set_gw(g_net_info.netdev, &ip_zero);

        netdev_dhcp_enabled(g_net_info.netdev, RT_TRUE);

        wiz_socket_init();                              /* wizenet socket initialize */
        DHCP_stop();                                    //DHCP停止,主要目的时防止内存泄漏
        rt_work_submit(dhcp_work, RT_WAITING_NO);       //重启启动DHCP
    }

    netdev_set_up(g_net_info.netdev);               //上网
    return RT_EOK;
}

static int set_ip(int argc, char **argv) {
    rt_base_t level;
    rt_err_t ret = RT_EOK;
    int para_nums = 0;
    if (argc < 2) {
        return -RT_ERROR;
    }
    if(2 <= argc) {
        if(inet_aton(argv[1],&(g_net_info.addr_change->ip_addr)) != 0) {
            para_nums++;
        }
    }
    if(3 <= argc) {
        if(inet_aton(argv[2],&(g_net_info.addr_change->netmask)) != 0) {
            para_nums++;
        }
    }
    if(4 <= argc) {
        if(inet_aton(argv[3],&(g_net_info.addr_change->gw)) != 0) {
            para_nums++;
        }
    }
    if(5 <= argc) {
        if(inet_aton(argv[4],&(g_net_info.addr_change->dns_servers[0])) != 0) {
            para_nums++;
        }
    }
    if(6 <= argc) {
        if(inet_aton(argv[4],&(g_net_info.addr_change->dns_servers[1])) != 0) {
            para_nums++;
        }
    }
    if(para_nums > 0) {
        level = rt_hw_interrupt_disable();           //关闭中断
        *g_net_info.ip_set = para_nums;
        rt_hw_interrupt_enable(level);              //打开中断
    }
    return ret;
}


static int set_dhcp(int argc, char **argv) {
    rt_base_t level;
    rt_err_t ret = RT_EOK;
    if (argc != 1) {
        return -RT_ERROR;
    }

    //设置设置dhcp标志位
    level = rt_hw_interrupt_disable();           //关闭中断
    *g_net_info.dhcp_set = 1;
    rt_hw_interrupt_enable(level);              //打开中断

    return ret;
}

static int net_start(int argc, char **argv) {
    rt_err_t ret = RT_EOK;
    if (argc != 1) {
        return -RT_ERROR;
    }
    net_tcpiptask_restart();
    return ret;
}

static int net_stop(int argc, char **argv) {
    rt_err_t ret = RT_EOK;
    if (argc != 1) {
        return -RT_ERROR;
    }
    net_tcpiptask_clear();
    return ret;
}

#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT(set_ip,      netdev set ip);
MSH_CMD_EXPORT(set_dhcp,    netdev set dhcp);
MSH_CMD_EXPORT(net_start,   net start);
MSH_CMD_EXPORT(net_stop,    net_stop);
#endif /* FINSH_USING_MSH */




