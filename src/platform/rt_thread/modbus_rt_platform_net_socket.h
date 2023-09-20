#ifndef __PKG_MODBUS_RT_PLATFORM_NET_SOCKET_H_
#define __PKG_MODBUS_RT_PLATFORM_NET_SOCKET_H_

#include "modbus_config.h"

#if (MODBUS_TCP_SLAVE_ENABLE) || (MODBUS_TCP_MASTER_ENABLE) || \
    (MODBUS_SERIAL_OVER_TCP_ENABLE) || (MODBUS_SERIAL_OVER_UDP_ENABLE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "rtdevice.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdev.h>

#define INET_ADDRSTRLEN  22

#define     NET_DEV_NAME        "W5500"

int modbus_rt_net_socket(int __domain, int __type, int __protocol);
int modbus_rt_net_close(int __fd);
int modbus_rt_net_accept(int __fd, struct sockaddr *addr, socklen_t *addrlen);
int modbus_rt_net_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);

#if MODBUS_TCP_SLAVE_ENABLE || (MODBUS_RTU_SLAVE_ENABLE && MODBUS_SERIAL_OVER_TCP_ENABLE)
    int modbus_rt_tcp_server_init(char* ipaddr, unsigned int port,int backlog);
#endif

#if MODBUS_TCP_MASTER_ENABLE || (MODBUS_RTU_MASTER_ENABLE && MODBUS_SERIAL_OVER_TCP_ENABLE)
    int modbus_rt_tcp_client_init(char* ipaddr, unsigned int port, char* saddr, unsigned int sport);
    int modbus_rt_net_addr2ip(char* saddr, char *ip);
#endif

#if MODBUS_TCP_SLAVE_FOR_UDP_ENABLE || MODBUS_TCP_MASTER_FOR_UDP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
    int modbus_rt_udp_socket_init(char* ipaddr, unsigned int port);
    int modbus_rt_net_segment(char *ipaddr, uint32_t saddr);
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif /* __PKG_MODBUS_RT_PLATFORM_NET_SOCKET_H_ */
