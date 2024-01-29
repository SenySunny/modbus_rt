/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-26     chenyong     first version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wiz.h>
#include <wiz_socket.h>

#include <W5500/w5500.h>
#ifdef WIZ_USING_DHCP
#include <DHCP/wizchip_dhcp.h>
#endif

#include <arpa/inet.h>
#include <netdev.h>

#if !defined(WIZ_SPI_DEVICE) || !defined(WIZ_RST_PIN) || !defined(WIZ_IRQ_PIN)
#error "please config SPI device name, reset pin and irq pin in menuconfig."
#endif

#define DBG_ENABLE
#define DBG_SECTION_NAME               "wiz"
#ifdef WIZ_DEBUG
#define DBG_LEVEL                      DBG_LOG
#else
#define DBG_LEVEL                      DBG_INFO
#endif /* WIZ_DEBUG */
#define DBG_COLOR
#include <rtdbg.h>

#define IMR_SENDOK                     0x10
#define IMR_TIMEOUT                    0x08
#define IMR_RECV                       0x04
#define IMR_DISCON                     0x02
#define IMR_CON                        0x01
#define WIZ_DEFAULT_MAC                "00-E0-81-DC-53-1A"

#define WIZ_ID_LEN                     6
static char wiz_netdev_name[WIZ_ID_LEN];

#define WIZ_DHCP_SOCKET                7

extern struct rt_spi_device *wiz_device;
extern int wiz_device_init(const char *spi_dev_name);
extern int wiz_inet_init(void);
static int wiz_netdev_info_update(struct netdev *netdev, rt_bool_t reset);

rt_bool_t wiz_init_ok = RT_FALSE;
static wiz_NetInfo wiz_net_info;
static rt_timer_t  dns_tick_timer;

#ifdef WIZ_USING_DHCP
static rt_timer_t  dhcp_timer;
#endif
struct rt_work *dhcp_work = RT_NULL;
extern int wiz_recv_notice_cb(int socket);
extern int wiz_closed_notice_cb(int socket);

static rt_mailbox_t wiz_rx_mb = RT_NULL;

static void wiz_isr(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_mb_send(wiz_rx_mb, (rt_ubase_t) wiz_device);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void wiz_data_thread_entry(void *parameter)
{
#define IR_SOCK(ch)         (0x01 << ch)   /**< check socket interrupt */

    struct rt_spi_device* dev;

    while (1)
    {
        if (rt_mb_recv(wiz_rx_mb, (rt_ubase_t*) &dev, RT_WAITING_FOREVER) == RT_EOK)
        {
            uint8_t ir, sir, sn_ir;
            int8_t socket = -1;

            /* get IR data than clean IR */
            ir = getIR();
            setIR(ir);

            if ((ir & IR_CONFLICT) == IR_CONFLICT)
            {
                setIR(IR_CONFLICT);
            }

            if ((ir & IR_UNREACH) == IR_UNREACH)
            {
                setIR(IR_UNREACH);
            }

            /* get and process socket interrupt register */
            sir = getSIR();

            for (socket = 0; socket < 8; socket++)
            {
                sn_ir = 0;

                if (sir & IR_SOCK(socket))
                {
                    /* save interrupt value*/
                    sn_ir = getSn_IR(socket);

                    if (sn_ir & Sn_IR_CON)
                    {
                        setSn_IR(socket, Sn_IR_CON);
                    }
                    if (sn_ir & Sn_IR_DISCON)
                    {
                        wiz_closed_notice_cb(socket);
                        setSn_IR(socket, Sn_IR_DISCON);
                    }
                    if (sn_ir & Sn_IR_RECV)
                    {
                        wiz_recv_notice_cb(socket);
                        setSn_IR(socket, Sn_IR_RECV);
                    }
                    if (sn_ir & Sn_IR_TIMEOUT)
                    {
                        /* deal with timeout event in the wiznet ioLibrary */
                        //setSn_IR(socket, Sn_IR_TIMEOUT);
                    }
                }
            }
        }
    }
}

static void spi_write_byte(uint8_t data)
{
    struct rt_spi_message spi_msg;

    rt_memset(&spi_msg, 0x00, sizeof(spi_msg));

    spi_msg.send_buf = &data;
    spi_msg.length = 1;

    rt_spi_transfer_message(wiz_device, &spi_msg);
}

static uint8_t spi_read_byte(void)
{
    struct rt_spi_message spi_msg;
    uint8_t data;

    rt_memset(&spi_msg, 0x00, sizeof(spi_msg));

    spi_msg.recv_buf = &data;
    spi_msg.length = 1;

    rt_spi_transfer_message(wiz_device, &spi_msg);

    return data;
}

static void spi_write_burst(uint8_t *pbuf, uint16_t len)
{
    struct rt_spi_message spi_msg;

    rt_memset(&spi_msg, 0x00, sizeof(spi_msg));

    spi_msg.send_buf = pbuf;
    spi_msg.length = len;

    rt_spi_transfer_message(wiz_device, &spi_msg);
}

static void spi_read_burst(uint8_t *pbuf, uint16_t len)
{
    struct rt_spi_message spi_msg;

    rt_memset(&spi_msg, 0x00, sizeof(spi_msg));

    spi_msg.recv_buf = pbuf;
    spi_msg.length = len;

    rt_spi_transfer_message(wiz_device, &spi_msg);
}

static void spi_cris_enter(void)
{
    rt_spi_take_bus(wiz_device);
}

static void spi_cris_exit(void)
{
    rt_spi_release_bus(wiz_device);
}

static void spi_cs_select(void)
{
    rt_spi_take(wiz_device);
}

static void spi_cs_deselect(void)
{
    rt_spi_release(wiz_device);
}

/* register TCP communication related callback function */
static int wiz_callback_register(void)
{
    /* register critical section callback function */
    reg_wizchip_cris_cbfunc(spi_cris_enter, spi_cris_exit);

#if (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_) || (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_)
    /* register SPI device CS select callback function */
    reg_wizchip_cs_cbfunc(spi_cs_select, spi_cs_deselect);
#else
#if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
#error "Unknown _WIZCHIP_IO_MODE_"
#else
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
#endif
#endif
    /* register SPI device read/write data callback function */
    reg_wizchip_spi_cbfunc(spi_read_byte, spi_write_byte);
    reg_wizchip_spiburst_cbfunc(spi_read_burst, spi_write_burst);

    return RT_EOK;
}

/* initialize WIZnet chip configures */
static int wiz_chip_cfg_init(void)
{
#define    CW_INIT_MODE         2
#define    CW_INIT_SOCKETS      8
#define    CW_INIT_TIMEOUT      (2 * RT_TICK_PER_SECOND)

    uint8_t memsize[CW_INIT_MODE][CW_INIT_SOCKETS] = { 0 };

    /* reset WIZnet chip internal PHY, configures PHY mode. */
    if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) == -1)
    {
        LOG_E("WIZCHIP initialize failed.");
        return -RT_ERROR;
    }

    struct wiz_NetTimeout_t net_timeout;
    net_timeout.retry_cnt=5;
    net_timeout.time_100us=20000;
    ctlnetwork(CN_SET_TIMEOUT, (void*) &net_timeout);

    return RT_EOK;
}

/* WIZnet chip hardware reset */
static void wiz_reset(void)
{
    rt_pin_write(WIZ_RST_PIN, PIN_LOW);
    rt_thread_mdelay(2);

    rt_pin_write(WIZ_RST_PIN, PIN_HIGH);
    rt_thread_mdelay(2);
}

#ifdef WIZ_USING_DHCP
static void wiz_ip_assign(void)
{
    /* get the assigned IP address and reconfigure the IP address of the chip */
    getIPfromDHCP(wiz_net_info.ip);
    getGWfromDHCP(wiz_net_info.gw);
    getSNfromDHCP(wiz_net_info.sn);
    getDNSfromDHCP(wiz_net_info.dns);
    wiz_net_info.dhcp = NETINFO_DHCP;

    ctlnetwork(CN_SET_NETINFO, (void*) &wiz_net_info);
}

static void wiz_ip_conflict(void)
{
    /* deal with conflict IP for WIZnet DHCP  */
    LOG_D("conflict IP from DHCP.");
    RT_ASSERT(0);
}

static void wiz_dhcp_timer_entry(void *parameter)
{
    DHCP_time_handler();
}
#endif /* WIZ_USING_DHCP */

static int wiz_netstr_to_array(const char *net_str, uint8_t *net_array)
{
    int ret;
    unsigned int idx;

    RT_ASSERT(net_str);
    RT_ASSERT(net_array);

    if (strstr(net_str, "."))
    {
        int ip_addr[4];

        /* resolve IP address, gateway address or subnet mask */
        ret = sscanf(net_str, "%d.%d.%d.%d", ip_addr + 0, ip_addr + 1, ip_addr + 2, ip_addr + 3);
        if (ret != 4)
        {
            LOG_E("input address(%s) resolve error.", net_str);
            return -RT_ERROR;
        }

        for (idx = 0; idx < sizeof(ip_addr)/sizeof(ip_addr[0]); idx++)
        {
            net_array[idx] = ip_addr[idx];
        }
    }
    else
    {
        int mac_addr[6];

        /* resolve MAC address */
        if (strstr(net_str, ":"))
        {
            ret = sscanf(net_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr + 0, mac_addr + 1, mac_addr + 2,
                    mac_addr + 3,  mac_addr + 4,  mac_addr + 5);
        }
        else if (strstr(net_str, "-"))
        {
            ret = sscanf(net_str, "%02x-%02x-%02x-%02x-%02x-%02x", mac_addr + 0, mac_addr + 1, mac_addr + 2,
                    mac_addr + 3,  mac_addr + 4,  mac_addr + 5);
        }
        else
        {
            LOG_E("input MAC address(%s) format error.", net_str);
            return -RT_ERROR;
        }

        if (ret != 6)
        {
            LOG_E("input MAC address(%s) resolve error.", net_str);
            return -RT_ERROR;
        }

        for (idx = 0; idx < sizeof(mac_addr)/sizeof(mac_addr[0]); idx++)
        {
            net_array[idx] = mac_addr[idx];
        }
    }

    return RT_EOK;
}

/* set WIZnet device MAC address */
RT_WEAK void wiz_user_config_mac(char *mac_buf, rt_uint8_t buf_len)
{
    RT_ASSERT(mac_buf != RT_NULL);
    RT_ASSERT(buf_len > 0);

    rt_memset(mac_buf, 0x0, buf_len);
    rt_strncpy(mac_buf, WIZ_DEFAULT_MAC, buf_len);
}

static void wiz_set_mac(void)
{
    char mac_str[32];

    wiz_user_config_mac(mac_str, sizeof(mac_str));
    if (wiz_netstr_to_array(mac_str, wiz_net_info.mac) != RT_EOK)
    {
        wiz_netstr_to_array(WIZ_DEFAULT_MAC, wiz_net_info.mac);
    }
}

#ifdef WIZ_USING_DHCP
static int wiz_network_dhcp(struct netdev *netdev);
#endif

/* initialize WIZnet network configures */
static int wiz_network_init(rt_bool_t b_config)
{
    struct netdev * netdev;
    netdev = netdev_get_by_name(wiz_netdev_name);
    if (netdev == RT_NULL)
    {
        LOG_E("don`t find device(%s)", wiz_netdev_name);
        return -RT_ERROR;
    }

#ifndef WIZ_USING_DHCP
    if(wiz_netstr_to_array(WIZ_IPADDR, wiz_net_info.ip) != RT_EOK ||
            wiz_netstr_to_array(WIZ_MSKADDR, wiz_net_info.sn) != RT_EOK ||
                wiz_netstr_to_array(WIZ_GWADDR, wiz_net_info.dns) != RT_EOK ||
                    wiz_netstr_to_array(WIZ_GWADDR, wiz_net_info.gw) != RT_EOK)
    {
        netdev_low_level_set_status(netdev, RT_FALSE);
        netdev_low_level_set_link_status(netdev, RT_FALSE);
        return -RT_ERROR;
    }
    wiz_net_info.dhcp = NETINFO_STATIC;
#endif

    int result = RT_EOK;
    rt_bool_t b_status = b_config;

    /* set mac information */
    wiz_set_mac();
    /* set static WIZnet network information */
    ctlnetwork(CN_SET_NETINFO, (void*) &wiz_net_info);

#ifdef WIZ_USING_DHCP
    /* alloc IP address through DHCP */
    {
        result = wiz_network_dhcp(netdev);
        if (result != RT_EOK)
        {
            b_status = RT_FALSE;
            LOG_E("WIZnet network initialize failed, DHCP timeout.");
        }
        else
        {
            b_status = RT_TRUE;
            LOG_D("WIZnet network initialize success.");
        }
    }
#endif

    netdev_low_level_set_status(netdev, b_status);
    wiz_netdev_info_update(netdev, RT_FALSE);
    netdev_low_level_set_link_status(netdev, b_status);

    return result;
}

/* wizenet socket initialize */
int wiz_socket_init(void)
{
    int idx = 0;

    /* socket(0-7) initialize */
    setSIMR(0xff);

    /* set socket receive/send buffer size */
    for (idx = 0; idx < WIZ_SOCKETS_NUM; idx++)
    {
        setSn_RXBUF_SIZE(idx, 0x02);
        setSn_TXBUF_SIZE(idx, 0x02);
    }

    /* set socket ISR state support */
    for (idx = 0; idx < WIZ_SOCKETS_NUM; idx++)
    {
        setSn_IMR(idx, (IMR_TIMEOUT | IMR_RECV | IMR_DISCON));
    }

    return RT_EOK;
}

static void wiz_dns_time_handler(void *arg)
{
    extern void DNS_time_handler(void);
    DNS_time_handler();
}

static int wiz_netdev_info_update(struct netdev *netdev, rt_bool_t reset)
{
    wiz_NetInfo net_info;
    rt_memset(&net_info, 0, sizeof(net_info));

    if(reset == RT_FALSE)
    {
        ctlnetwork(CN_GET_NETINFO, (void *)&net_info);
    }
    else
    {
        /* clean dns server information */
        netdev->dns_servers->addr = 0;
        ctlnetwork(CN_SET_NETINFO, (void *)&net_info);
    }
    netdev_low_level_set_ipaddr(netdev, (const ip_addr_t *)&net_info.ip);
    netdev_low_level_set_gw(netdev, (const ip_addr_t *)&net_info.gw);
    netdev_low_level_set_netmask(netdev, (const ip_addr_t *)&net_info.sn);
    netdev_low_level_set_dns_server(netdev, 0, (const ip_addr_t *)&net_info.dns);
    memcpy(netdev->hwaddr, (const void *)&net_info.mac, netdev->hwaddr_len);
    /* 1 - Static, 2 - DHCP */
    netdev_low_level_set_dhcp_status(netdev, net_info.dhcp - 1);

    return RT_EOK;
}

static int wiz_netdev_set_up(struct netdev *netdev)
{
    netdev_low_level_set_status(netdev, RT_TRUE);
    return RT_EOK;
}

static int wiz_netdev_set_down(struct netdev *netdev)
{
    netdev_low_level_set_status(netdev, RT_FALSE);
    return RT_EOK;
}

static int wiz_netdev_set_addr_info(struct netdev *netdev, ip_addr_t *ip_addr, ip_addr_t *netmask, ip_addr_t *gw)
{
    rt_err_t result = RT_EOK;

    RT_ASSERT(netdev);
    RT_ASSERT(ip_addr || netmask || gw);

    ctlnetwork(CN_GET_NETINFO, (void *)&wiz_net_info);

    if (ip_addr)
        rt_memcpy(wiz_net_info.ip, &ip_addr->addr, sizeof(wiz_net_info.ip));

    if (netmask)
        rt_memcpy(wiz_net_info.sn, &netmask->addr, sizeof(wiz_net_info.sn));

    if (gw)
        rt_memcpy(wiz_net_info.gw, &gw->addr, sizeof(wiz_net_info.gw));

    if (ctlnetwork(CN_SET_NETINFO, (void *)&wiz_net_info) == RT_EOK)
    {
        if (ip_addr)
            netdev_low_level_set_ipaddr(netdev, ip_addr);

        if (netmask)
            netdev_low_level_set_netmask(netdev, netmask);

        if (gw)
            netdev_low_level_set_gw(netdev, gw);

        result = RT_EOK;
    }
    else
    {
        LOG_E("%s set addr info failed!", wiz_netdev_name);
        result = -RT_ERROR;
    }

    return result;
}

static int wiz_netdev_set_dns_server(struct netdev *netdev, uint8_t dns_num, ip_addr_t *dns_server)
{
    rt_err_t result = RT_EOK;

    RT_ASSERT(netdev);
    RT_ASSERT(dns_server);

    ctlnetwork(CN_GET_NETINFO, (void *)&wiz_net_info);

    rt_memcpy(wiz_net_info.dns, &dns_server->addr, sizeof(wiz_net_info.dns));

    if (ctlnetwork(CN_SET_NETINFO, (void *)&wiz_net_info) == RT_EOK)
    {
        netdev_low_level_set_dns_server(netdev, dns_num, (const ip_addr_t *)dns_server);
        result = RT_EOK;
    }
    else
    {
        LOG_E("%s set dns server failed!", wiz_netdev_name);
        result = -RT_ERROR;
    }

    return result;
}

static int wiz_netdev_set_dhcp(struct netdev *netdev, rt_bool_t is_enabled)
{
    rt_err_t result = RT_EOK;

    RT_ASSERT(netdev);

    ctlnetwork(CN_GET_NETINFO, (void *)&wiz_net_info);

    /* 1 - Static, 2 - DHCP */
    wiz_net_info.dhcp = (dhcp_mode)(is_enabled + 1);

    if (ctlnetwork(CN_SET_NETINFO, (void *)&wiz_net_info) == RT_EOK)
    {
        netdev_low_level_set_dhcp_status(netdev, is_enabled);
        result = RT_EOK;

        if(is_enabled == RT_FALSE)
        {
            if(dhcp_work != RT_NULL)
            {
                rt_work_cancel(dhcp_work);
            }
            LOG_D("wiznet(w5500) dhcp status is disable.");
        }
        else
        {
#ifndef WIZ_USING_DHCP
            LOG_W("wiznet(w5500) dhcp function haven't compiled.");
            result = -RT_ERROR;
#else
            if(dhcp_work != RT_NULL)
            {
                rt_work_submit(dhcp_work, RT_WAITING_NO);
                LOG_D("wiznet(w5500) dhcp status is enable.");
            }
#endif
        }
    }
    else
    {
        LOG_E("%s set dhcp info failed!", wiz_netdev_name);
        result = -RT_ERROR;
    }

    return result;
}

#ifdef RT_USING_FINSH
static int wiz_netdev_ping(struct netdev *netdev, const char *host, size_t data_len, uint32_t timeout, struct netdev_ping_resp *ping_resp)
{
    RT_ASSERT(netdev);
    RT_ASSERT(host);
    RT_ASSERT(ping_resp);

    extern int wiz_ping(struct netdev *netdev, const char *host, size_t data_len, uint32_t times, struct netdev_ping_resp *ping_resp);

    return wiz_ping(netdev, host, data_len, timeout, ping_resp);
}
#endif

void wiz_netdev_netstat(struct netdev *netdev)
{
    // TODO
    return;
}

const struct netdev_ops wiz_netdev_ops =
{
    wiz_netdev_set_up,
    wiz_netdev_set_down,

    wiz_netdev_set_addr_info,
    wiz_netdev_set_dns_server,
    wiz_netdev_set_dhcp,
#ifdef RT_USING_FINSH
    wiz_netdev_ping,
    wiz_netdev_netstat,
#endif
};

static struct netdev *wiz_netdev_add(const char *netdev_name)
{
#define ETHERNET_MTU        1472
#define HWADDR_LEN          6
    struct netdev *netdev = RT_NULL;

    netdev = (struct netdev *)rt_calloc(1, sizeof(struct netdev));
    if (netdev == RT_NULL)
    {
        return RT_NULL;
    }

    netdev->flags = 0;
    netdev->mtu = ETHERNET_MTU;
    netdev->ops = &wiz_netdev_ops;
    netdev->hwaddr_len = HWADDR_LEN;

#ifdef PKG_USING_WIZNET
    extern int sal_wiz_netdev_set_pf_info(struct netdev *netdev);
    /* set the network interface socket/netdb operations */
    sal_wiz_netdev_set_pf_info(netdev);
#endif

    netdev_register(netdev, netdev_name, RT_NULL);

    return netdev;
}

#ifdef WIZ_USING_DHCP
extern int net_restart_status;
static void wiz_dhcp_work(struct rt_work *dhcp_work, void *dhcp_work_data)
{
#define WIZ_DHCP_WORK_RETRY         3 /* DHCP will have 3 times handshake */
#define WIZ_DHCP_WORK_RETRY_TIME    (2 * RT_TICK_PER_SECOND)
    static int wiz_dhcp_retry_times = WIZ_DHCP_WORK_RETRY * 20;

    RT_ASSERT(dhcp_work_data != RT_NULL);

    struct netdev *netdev = (struct netdev *)dhcp_work_data;

    uint8_t dhcp_times = 0;
    uint32_t dhcp_work_times;
    static uint8_t data_buffer[1024];
    static uint32_t dhcp_status = DHCP_FAILED;

    if(dhcp_status == DHCP_FAILED)
    {
        DHCP_init(WIZ_DHCP_SOCKET, data_buffer);
        rt_timer_start(dhcp_timer);
    }
	

    while (1)
    {
        /* DHCP start, return DHCP_IP_LEASED is success. */
        dhcp_status = DHCP_run();

        switch (dhcp_status)
        {
        case DHCP_IP_ASSIGN:
        case DHCP_IP_CHANGED:
        {
            /* to update netdev information */
            wiz_netdev_info_update(netdev, RT_FALSE);
            break;
        }
        case DHCP_IP_LEASED:
        {
            int hour, min;
            (void)(hour);(void)(min);
            /* to update netdev information */
            wiz_netdev_info_update(netdev, RT_FALSE);

            /* reset the previous work configure */
            rt_work_cancel(dhcp_work);
            dhcp_work_times = (getDHCPTick1s() > getDHCPLeasetime() / 2) ? 
                0 : getDHCPLeasetime() / 2 - getDHCPTick1s();
            /* according to the DHCP leaset time, config next DHCP produce */
            rt_work_submit(dhcp_work, (dhcp_work_times+1) * RT_TICK_PER_SECOND);
            hour = getDHCPLeasetime() / 3600;
            min = (getDHCPLeasetime() % 3600) / 60;
            LOG_D("DHCP countdown to lease renewal [%dH: %dMin], retry time[%04d]", hour, min, dhcp_times);
            wiz_dhcp_retry_times = WIZ_DHCP_WORK_RETRY * 20;
			net_restart_status = 0;
            return;
        }
        case DHCP_STOPPED:
            dhcp_times = wiz_dhcp_retry_times;
            break;
        case DHCP_FAILED:
        {
            dhcp_times = wiz_dhcp_retry_times;
            LOG_E("dhcp handshake failed!");
            break;
        }
        default:
        {
            dhcp_times++;

            /* DHCP_RUNNING status, include don't receive data */
            rt_thread_mdelay(10);
            break;
        }
        }

        if (dhcp_times >= wiz_dhcp_retry_times)
        {
            LOG_D("DHCP work in %d seconds, [%03d|%03d]", WIZ_DHCP_WORK_RETRY_TIME / RT_TICK_PER_SECOND, dhcp_times, wiz_dhcp_retry_times);

            /* if dhcp service is too busy to manger IP, increase retry times */
            wiz_dhcp_retry_times = wiz_dhcp_retry_times + WIZ_DHCP_WORK_RETRY;

            DHCP_stop();
            dhcp_status = DHCP_FAILED;
            rt_timer_stop(dhcp_timer);

            rt_work_cancel(dhcp_work);

            /* according to WIZ_DHCP_WORK_RETRY_TIME, reconfigure in 2 seconds */
			if(net_restart_status > 0)
			{
			     net_restart_status++;
            }
            if(net_restart_status < 3)
            {
                rt_work_submit(dhcp_work, WIZ_DHCP_WORK_RETRY_TIME);
            }
            else
            {
                net_restart_status = 0;
            }
			break;
        }
    }
}

static int wiz_network_dhcp(struct netdev *netdev)
{
    if (netdev == RT_NULL)
        return -RT_EINVAL;

    /* set default MAC address for DHCP */
    setSHAR(wiz_net_info.mac);
    /* DHCP configure initialize, clear information other than MAC address */
    setSn_RXBUF_SIZE(WIZ_DHCP_SOCKET, 0x02);
    setSn_TXBUF_SIZE(WIZ_DHCP_SOCKET, 0x02);
    /* register to assign IP address and conflict callback */
    reg_dhcp_cbfunc(wiz_ip_assign, wiz_ip_assign, wiz_ip_conflict);

    dhcp_timer = rt_timer_create("wiz_dhcp", wiz_dhcp_timer_entry, RT_NULL, 1 * RT_TICK_PER_SECOND, RT_TIMER_FLAG_PERIODIC);
    if (dhcp_timer == RT_NULL)
        return -RT_ERROR;

    dhcp_work = (struct rt_work *)rt_calloc(1, sizeof(struct rt_work));
    if (dhcp_work == RT_NULL)
        return -RT_ENOMEM;
    rt_thread_mdelay(1800);
    rt_work_init(dhcp_work, wiz_dhcp_work, (void *)netdev);
    rt_work_submit(dhcp_work, WIZ_DHCP_WORK_RETRY_TIME);

    return RT_EOK;
}
#endif /* WIZ_USING_DHCP */

static void wiz_link_status_thread_entry(void *parameter)
{
#define WIZ_PHYCFGR_LINK_STATUS 0x01

    uint8_t phycfgr = 0;
    struct netdev *netdev = RT_NULL;

    netdev = netdev_get_by_name(wiz_netdev_name);
    if (netdev == RT_NULL)
    {
        LOG_E("don`t find device(%s)", wiz_netdev_name);
        return;
    }

    while (1)
    {
        /* Get PHYCFGR data */
        phycfgr = getPHYCFGR();

        /* If the register contents are different from the struct contents, the struct needs to be updated */
        if ((phycfgr & WIZ_PHYCFGR_LINK_STATUS) != ((netdev->flags & NETDEV_FLAG_LINK_UP) ? RT_TRUE : RT_FALSE))
        {
            if (phycfgr & WIZ_PHYCFGR_LINK_STATUS)
            {
                wiz_socket_init();
#ifdef WIZ_USING_DHCP
                if(dhcp_work)
                {
                    DHCP_stop();
                    rt_work_submit(dhcp_work, RT_WAITING_NO);
                }
#else
                wiz_network_init(RT_TRUE);
#endif
                netdev_low_level_set_link_status(netdev, phycfgr & WIZ_PHYCFGR_LINK_STATUS);
                wiz_netdev_info_update(netdev, RT_FALSE);
                LOG_I("%s netdev link status becomes link up", wiz_netdev_name);
            }
            else
            {
                netdev_low_level_set_link_status(netdev, phycfgr & WIZ_PHYCFGR_LINK_STATUS);
                if(dhcp_work)
                {
                    rt_work_cancel(dhcp_work);
                }
                wiz_netdev_info_update(netdev, RT_TRUE);
                LOG_I("%s netdev link status becomes link down", wiz_netdev_name);
            }
        }
        rt_thread_mdelay(1000);
    }
}

static int wiz_interrupt_init(rt_base_t isr_pin)
{
    rt_thread_t tid;

    /* initialize RX mailbox */
    wiz_rx_mb = rt_mb_create("wiz_mb", WIZ_RX_MBOX_NUM, RT_IPC_FLAG_FIFO);
    if (wiz_rx_mb == RT_NULL)
    {
        LOG_E("WIZnet create receive data mailbox error.");
        return -RT_ENOMEM;
    }

    /* create WIZnet SPI RX thread  */
    tid = rt_thread_create("wiz", wiz_data_thread_entry, RT_NULL, 2048, RT_THREAD_PRIORITY_MAX / 6, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    /* initialize interrupt pin */
    rt_pin_mode(isr_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(isr_pin, PIN_IRQ_MODE_FALLING, (void (*)(void*)) wiz_isr, RT_NULL);
    rt_pin_irq_enable(isr_pin, PIN_IRQ_ENABLE);

    return 0;
}

static int wiz_is_exist(void)
{
    wiz_NetInfo ni;
    int ret;

    wiz_set_mac();
    ctlnetwork(CN_SET_NETINFO, (void *)&wiz_net_info);
    ctlnetwork(CN_GET_NETINFO, (void *)&ni);

    ret = rt_memcmp(wiz_net_info.mac, ni.mac, sizeof(ni.mac));

    return (ret == 0);
}

/* #include "stm32f4xx_hal.h" */
/* WIZnet initialize device and network */
int wiz_init(void)
{
    int result = RT_EOK;
    rt_thread_t tid;

    if (wiz_init_ok == RT_TRUE)
    {
        LOG_I("RT-Thread WIZnet package is already initialized.");
        return RT_EOK;
    }

        /* initialize reset pin */
    rt_pin_mode(WIZ_RST_PIN, PIN_MODE_OUTPUT);

    /* I think you can attach w5500 into spi bus at here. You can use this function to realize.*/
    /* extern rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, GPIO_TypeDef *cs_gpiox, uint16_t cs_gpio_pin); */

    /* WIZnet SPI device and pin initialize */
    result = wiz_device_init(WIZ_SPI_DEVICE);
    if (result != RT_EOK)
    {
        goto __exit;
    }

    /* WIZnet SPI device reset */
    wiz_reset();
    /* set WIZnet device read/write data callback */
    wiz_callback_register();

    if (!wiz_is_exist())
    {
        result = -1;
        LOG_E("Wiznet chip not detected");
        goto __exit;
    }

    /* Add wiz to the netdev list */
    ctlwizchip(CW_GET_ID, (void *)wiz_netdev_name);
    wiz_netdev_add(wiz_netdev_name);

    /* WIZnet chip configure initialize */
    wiz_chip_cfg_init();

    /* WIZnet socket initialize */
    wiz_socket_init();
    /* WIZnet network initialize */
    result = wiz_network_init(RT_FALSE);
    if (result != RT_EOK)
    {
        goto __exit;
    }

    dns_tick_timer = rt_timer_create("dns_tick", wiz_dns_time_handler, RT_NULL, 1 * RT_TICK_PER_SECOND, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
    rt_timer_start(dns_tick_timer);

    /* create WIZnet link status Polling thread  */
    tid = rt_thread_create("wiz_stat", wiz_link_status_thread_entry, RT_NULL, 2048, RT_THREAD_PRIORITY_MAX - 4, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    wiz_interrupt_init(WIZ_IRQ_PIN);

__exit:
    if (result == RT_EOK)
    {
        wiz_init_ok = RT_TRUE;
        LOG_I("RT-Thread WIZnet package (V%s) initialize success.", WIZ_SW_VERSION);
    }
    else
    {
        LOG_E("RT-Thread WIZnet package (V%s) initialize failed(%d).", WIZ_SW_VERSION, result);
    }

    return result;
}
INIT_ENV_EXPORT(wiz_init);
