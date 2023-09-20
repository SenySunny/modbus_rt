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
#include <rtthread.h>

#include <sys/time.h>

#include <wiz_socket.h>
#include <W5500/w5500.h>

#ifdef RT_USING_SAL
#include <sal_netdb.h>
#include <sal_low_lvl.h>
#endif /* RT_USING_SAL */

#include <netdev.h>

#define Sn_PROTO(ch)         (0x001408 + (ch<<5))

#define WIZ_PING_DATA_LEN    32
#define WIZ_PING_HEAD_LEN    8

#define WIZ_PING_PORT        3000
#define WIZ_PING_REQUEST     8
#define WIZ_PING_REPLY       0
#define WIZ_PING_CODE        0
#define WIZ_PING_DELAY      (1 * RT_TICK_PER_SECOND)
#define WIZ_PING_TIMEOUT    (2 * RT_TICK_PER_SECOND)

struct wiz_ping_msg
{
    uint8_t   type;                    // 0 - Ping Reply, 8 - Ping Request
    uint8_t   code;                    // Always 0
    uint16_t  check_sum;               // Check sum
    uint16_t  id;                      // Identification
    uint16_t  seq_num;                 // Sequence Number
    int8_t    data[WIZ_PING_DATA_LEN]; // Ping Data  : 1452 = IP RAW MTU - sizeof(type+code+check_sum+id+seq_num)
};

/* calculate string check value */
static uint16_t wiz_checksum( uint8_t *src, uint32_t len )
{
    uint16_t sum, tsum, i, j;
    uint32_t lsum;

    j = len >> 1;
    lsum = 0;

    for (i = 0; i < j; i++)
    {
        tsum = src[i * 2];
        tsum = tsum << 8;
        tsum += src[i * 2 + 1];
        lsum += tsum;
    }

    if (len % 2)
    {
        tsum = src[i * 2];
        lsum += (tsum << 8);
    }

    sum = lsum;
    sum = ~(sum + (lsum >> 16));
    return (uint16_t) sum;
}

static int wiz_ping_request(int socket)
{
    int idx, send_len;
    uint16_t tmp_checksum;
    struct wiz_ping_msg ping_req;

    /* set request ping message object */
    ping_req.type = WIZ_PING_REQUEST;
    ping_req.code = WIZ_PING_CODE;
    ping_req.id = htons(rand() % 0xffff);
    ping_req.seq_num = htons(rand() % 0xffff);
    for (idx = 0; idx < WIZ_PING_DATA_LEN; idx++)
    {
        ping_req.data[idx] = (idx) % 8;
    }
    ping_req.check_sum = 0;
    /* calculate request ping message check value */
    tmp_checksum = wiz_checksum((uint8_t *) &ping_req, sizeof(ping_req));
    ping_req.check_sum = htons(tmp_checksum);

    /* send request ping message */
    send_len = wiz_send(socket, &ping_req, sizeof(ping_req), 0);
    if (send_len != sizeof(ping_req))
    {
        return -1;
    }

    return send_len - WIZ_PING_HEAD_LEN;
}

static int wiz_ping_reply(int socket, struct sockaddr *from)
{
    uint16_t tmp_checksum;
    uint8_t recv_buf[WIZ_PING_HEAD_LEN + WIZ_PING_DATA_LEN + 1];
    struct wiz_ping_msg ping_rep;
    rt_tick_t start_tick;
    int recv_len;
    int idx;

    start_tick = rt_tick_get();
    while(1)
    {
        if (rt_tick_get() - start_tick > WIZ_PING_TIMEOUT)
        {
            return -1;
        }

        if (getSn_RX_RSR(socket) <= 0)
        {
            rt_thread_mdelay(1);
            continue;
        }
        else
        {
            struct sockaddr *sin = (struct sockaddr *)from;
            socklen_t addr_len = sizeof(struct sockaddr_in);
            recv_len = wiz_recvfrom(socket, recv_buf, WIZ_PING_HEAD_LEN + WIZ_PING_DATA_LEN, 0, sin, &addr_len);
            if (recv_len < 0)
            {
                return -1;
            }
            break;
        }
    }

    if (recv_buf[0] == WIZ_PING_REPLY)
    {
        ping_rep.type        = recv_buf[0];
        ping_rep.code        = recv_buf[1];
        ping_rep.check_sum   = (recv_buf[3] << 8) + recv_buf[2];
        ping_rep.id          = (recv_buf[5] << 8) + recv_buf[4];
        ping_rep.seq_num     = (recv_buf[7] << 8) + recv_buf[6];
        for (idx = 0; idx < recv_len - 8; idx++)
        {
            ping_rep.data[idx] = recv_buf[8 + idx];
        }

        tmp_checksum = ~wiz_checksum(recv_buf, recv_len);
        if (tmp_checksum != 0xffff)
        {
            return -2;
        }
    }
    else if (recv_buf[0] == WIZ_PING_REQUEST)
    {
        ping_rep.code        = recv_buf[1];
        ping_rep.type        = recv_buf[2];
        ping_rep.check_sum   = (recv_buf[3] << 8) + recv_buf[2];
        ping_rep.id          = (recv_buf[5] << 8) + recv_buf[4];
        ping_rep.seq_num     = (recv_buf[7] << 8) + recv_buf[6];
        for (idx = 0; idx < recv_len - 8; idx++)
        {
            ping_rep.data[idx] = recv_buf[8 + idx];
        }

        tmp_checksum = ping_rep.check_sum;
        ping_rep.check_sum = 0;
        if (tmp_checksum != ping_rep.check_sum)
        {
            return -2;
        }
    }
    else
    {
        rt_kprintf("wiz_ping: unknown ping receive message.\n");
        return -1;
    }

    return recv_len - WIZ_PING_HEAD_LEN;
}

int wiz_ping(struct netdev *netdev, const char *host, size_t data_len, uint32_t times, struct netdev_ping_resp *ping_resp)
{
    int result = RT_EOK, socket;
    struct sockaddr_in server_addr;
    struct timeval timeout;
    struct in_addr ina;
    struct hostent *hostent;
    rt_tick_t recv_start_tick;
    struct sal_proto_family *pf = (struct sal_proto_family *) netdev->sal_user_data;

    /* get network interface socket operations */
    if (pf == RT_NULL || pf->skt_ops == RT_NULL)
    {
        rt_kprintf("wiz_ping: pf or pf->skt_ops is RT_NULL.\n");
        return -RT_FALSE;
    }

    hostent = (struct hostent *) pf->netdb_ops->gethostbyname(host);
    if (hostent == RT_NULL)
    {
        rt_kprintf("wiz_ping: hostent is RT_NULL.\n");
        return -RT_FALSE;
    }

    socket = -1;

    socket = wiz_socket(AF_WIZ, SOCK_RAW, 0);
    if (socket < 0)
    {
        rt_kprintf("wiz_ping: create ping socket(%d) failed.\n",socket);
        return -1;
    }
    /* set socket ICMP protocol */
    IINCHIP_WRITE(Sn_PROTO(socket), IPPROTO_ICMP);

    /* Check socket register */
    while(getSn_SR(socket) != SOCK_IPRAW);

    timeout.tv_sec = times / RT_TICK_PER_SECOND;
    timeout.tv_usec = (times % RT_TICK_PER_SECOND) * 1000 / RT_TICK_PER_SECOND;

    /* set receive and send timeout option */
    wiz_setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (void *) &timeout,
               sizeof(timeout));
    wiz_setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (void *) &timeout,
               sizeof(timeout));

    server_addr.sin_family = AF_WIZ;
    server_addr.sin_port = htons(WIZ_PING_PORT);
    server_addr.sin_addr = *((struct in_addr *)hostent->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    rt_memcpy(&ina, &server_addr.sin_addr, sizeof(ina));

    if (wiz_connect(socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0)
    {
        wiz_closesocket(socket);
        return -1;
    }

    if ((result = wiz_ping_request(socket)) > 0)
    {
        recv_start_tick = rt_tick_get();
        result = wiz_ping_reply(socket, (struct sockaddr *) &server_addr);
    }

    if(result > 0)
    {
        ping_resp->ip_addr.addr = ((struct in_addr *)hostent->h_addr)->s_addr;
        ping_resp->ticks = rt_tick_get() - recv_start_tick;
        ping_resp->data_len = data_len;
        ping_resp->ttl = getSn_TTL(socket);
    }

    wiz_closesocket(socket);

    return result;
}
