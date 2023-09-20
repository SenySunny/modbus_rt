/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-26     chenyong     first version
 */

#ifndef __WIZ_SOCKET_H__
#define __WIZ_SOCKET_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>

#include <netdb.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/* WIZnet socket magic word */
#define WIZ_SOCKET_MAGIC               0x3120

/* WIZnet Socket address family */
#ifndef AF_WIZ
#define AF_WIZ                         46
#endif

struct wiz_socket;
/* A callback prototype to inform about events for wiznet socket */
typedef void (* wiz_socket_callback)(struct wiz_socket *conn, int event, uint16_t len);

struct wiz_clnt_info
{
    int socket;
    int state;
    rt_slist_t list;
};

struct wiz_svr_info
{
    int backlog;
    rt_timer_t conn_tmr;
    rt_mailbox_t conn_mbox;
    rt_slist_t clnt_list;
};

struct wiz_socket
{
    /* WIZnet socket magic word */
    uint32_t magic;

    int socket;
    uint16_t port;
    /* type of the WIZnet socket (TCP, UDP or RAW) */
    uint8_t type;
    /* current state of the WIZnet socket */
    uint8_t state;
    /* receive semaphore, received data release semaphore */
    rt_sem_t recv_notice;
    rt_mutex_t recv_lock;

    /* timeout to wait for send or receive data in milliseconds */
    int32_t recv_timeout;
    int32_t send_timeout;

    /* A callback function that is informed about events for this AT socket */
    wiz_socket_callback callback;

    struct sockaddr *remote_addr;
    /* number of times data was received, set by event_callback() */
    uint16_t rcvevent;
    /* number of times data was ACKed (free send buffer), set by event_callback() */
    uint16_t sendevent;
    /* error happened for this socket, set by event_callback() */
    uint16_t errevent;

    /* server socket information */
    struct wiz_svr_info *svr_info;

#ifdef SAL_USING_POSIX
    rt_wqueue_t wait_head;
#endif
};

int wiz_socket(int domain, int type, int protocol);
int wiz_closesocket(int socket);
int wiz_shutdown(int socket, int how);
int wiz_listen(int socket, int backlog);
int wiz_bind(int socket, const struct sockaddr *name, socklen_t namelen);
int wiz_connect(int socket, const struct sockaddr *name, socklen_t namelen);
int wiz_accept(int socket, struct sockaddr *addr, socklen_t *addrlen);
int wiz_sendto(int socket, const void *dwiza, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
int wiz_send(int socket, const void *dwiza, size_t size, int flags);
int wiz_recvfrom(int socket, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
int wiz_recv(int socket, void *mem, size_t len, int flags);
int wiz_getsockopt(int socket, int level, int optname, void *optval, socklen_t *optlen);
int wiz_setsockopt(int socket, int level, int optname, const void *optval, socklen_t optlen);
struct hostent *wiz_gethostbyname(const char *name);
int wiz_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
void wiz_freeaddrinfo(struct addrinfo *ai);

/* get WIZnet socket object */
struct wiz_socket *wiz_get_socket(int socket);
/* WIZnet chip TCP/IP protocol register */
int wiz_inet_init(void);

#ifdef  __cplusplus
    }
#endif

#endif /* __WIZ_SOCKET_H__ */
