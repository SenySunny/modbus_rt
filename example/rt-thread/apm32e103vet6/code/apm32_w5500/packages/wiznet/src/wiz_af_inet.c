/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-26     chenyong     first version
 */

#include <rtthread.h>
#include <rthw.h>

#ifdef RT_USING_SAL
#include <sal_netdb.h>
#include <sal_low_lvl.h>
#endif /* RT_USING_SAL */

#include <netdev.h>

#include <wiz.h>
#include <wiz_socket.h>

#ifdef SAL_USING_POSIX
#include <poll.h>
#endif

#ifdef SAL_USING_POSIX
static int wiz_poll(struct dfs_fd *file, struct rt_pollreq *req)
{
    int mask = 0;
    struct wiz_socket *sock;
    struct sal_socket *sal_sock;

    sal_sock = sal_get_socket((int) file->data);
    if(!sal_sock)
    {
        return -1;
    }

    sock = wiz_get_socket((int)sal_sock->user_data);
    if (sock != NULL)
    {
        rt_base_t level;

        rt_poll_add(&sock->wait_head, req);

        level = rt_hw_interrupt_disable();
        if (sock->rcvevent)
        {
            mask |= POLLIN;
        }
        if (sock->sendevent)
        {
            mask |= POLLOUT;
        }
        if (sock->errevent)
        {
            mask |= POLLERR;
        }
        rt_hw_interrupt_enable(level);
    }

    return mask;
}
#endif

static const struct sal_socket_ops wiz_socket_ops =
{
    wiz_socket,
    wiz_closesocket,
    wiz_bind,
    wiz_listen,
    wiz_connect,
    wiz_accept,
    wiz_sendto,
    wiz_recvfrom,
    wiz_getsockopt,
    wiz_setsockopt,
    wiz_shutdown,
    NULL,
    NULL,
    NULL,
#ifdef SAL_USING_POSIX
    wiz_poll,
#endif /* SAL_USING_POSIX */
};

static const struct sal_netdb_ops wiz_netdb_ops =
{
    wiz_gethostbyname,
    NULL,
    wiz_getaddrinfo,
    wiz_freeaddrinfo,
};


static const struct sal_proto_family wiz_inet_family =
{
    AF_WIZ,
    AF_INET,
    &wiz_socket_ops,
    &wiz_netdb_ops,
};

/* Set wiz network interface device protocol family information */
int sal_wiz_netdev_set_pf_info(struct netdev *netdev)
{
    RT_ASSERT(netdev);

    netdev->sal_user_data = (void *) &wiz_inet_family;
    return 0;
}
