
#include "modbus_rt_platform_net_socket.h"

#if (MODBUS_TCP_SLAVE_ENABLE) || (MODBUS_TCP_MASTER_ENABLE) || \
    (MODBUS_SERIAL_OVER_TCP_ENABLE) || (MODBUS_SERIAL_OVER_UDP_ENABLE)

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")

static int modbus_rt_winsock_initialized = 0;

int modbus_rt_init_winsock() {
    if (0 == modbus_rt_winsock_initialized) {
        WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (res != 0) {
            printf("WSAStartup failed with error: %d\n", res);
            return 1;
        }
        modbus_rt_winsock_initialized = 1;
    } else if (0 < modbus_rt_winsock_initialized) {
        modbus_rt_winsock_initialized++;
    }
    return 0;
}

int modbus_rt_cleanup_winsock() {
    if (1 == modbus_rt_winsock_initialized) {
        WSACleanup();
        modbus_rt_winsock_initialized = 0;
    } else if (1 < modbus_rt_winsock_initialized) {
        modbus_rt_winsock_initialized--;
    }
    return 0;
}
#endif


int modbus_rt_net_socket(int __domain, int __type, int __protocol) {
#if defined(__linux__)
    return socket(__domain, __type, __protocol);
#elif defined(_WIN32)
    modbus_rt_init_winsock();
    return socket(__domain, __type, __protocol);
#endif
}


int modbus_rt_net_close(int __fd) {
#if defined(__linux__)
    shutdown(__fd, SHUT_RDWR);
    return close(__fd);
#elif defined(_WIN32)
    int ret = 0;
    shutdown(__fd, SD_BOTH);
    ret = closesocket(__fd);
    modbus_rt_cleanup_winsock();
    return ret;
#endif
}

int modbus_rt_net_accept(int __fd, struct sockaddr *addr, int *addrlen) {
#if defined(__linux__)
    return accept(__fd,  addr,  addrlen);
#elif defined(_WIN32)
    int ret = accept(__fd,  addr,  addrlen);
    if(ret > 0) {
        modbus_rt_init_winsock();
    }
    return ret;
#endif
}

int modbus_rt_net_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    return setsockopt(fd, level, optname, optval, optlen);
}

#if MODBUS_TCP_SLAVE_ENABLE || (MODBUS_RTU_SLAVE_ENABLE && MODBUS_SERIAL_OVER_TCP_ENABLE)
int modbus_rt_tcp_server_init(char* ipaddr, unsigned int port,int backlog)
{
    struct sockaddr_in server_addr;
    int sfd = modbus_rt_net_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(0 > sfd) {
        return sfd;
    }
    int optval = 1;
    modbus_rt_net_setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if((NULL != ipaddr) &&(0 == strlen(ipaddr))) {
        ipaddr = NULL;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port  = htons(port);
    server_addr.sin_addr.s_addr = (ipaddr == NULL) ? htonl(INADDR_ANY) : inet_addr(ipaddr);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    int ret = bind(sfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr));
    if(0 > ret) {
        modbus_rt_net_close(sfd);
        return ret;
    }

    ret = listen(sfd, backlog);
    if(0 > ret) {
        modbus_rt_net_close(sfd);
        return ret;
    }
    return sfd;
}
#endif

#if MODBUS_TCP_MASTER_ENABLE || (MODBUS_RTU_MASTER_ENABLE && MODBUS_SERIAL_OVER_TCP_ENABLE)
int modbus_rt_tcp_client_init(char* ipaddr, unsigned int port, char* saddr, unsigned int sport)
{
    int ret = -1;
    struct sockaddr_in client_addr = {0};
    struct addrinfo hints, *addr_list, *cur;
    char strPort[6] = {0};

    int cfd = modbus_rt_net_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(0 > cfd) {
        return cfd;
    }
    int optval = 1;
    modbus_rt_net_setsockopt(cfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if((NULL != ipaddr) || (0 != port)) {
        if((NULL != ipaddr) &&(0 == strlen(ipaddr))) {
            ipaddr = NULL;
        }
        client_addr.sin_family = AF_INET;
        client_addr.sin_port  = htons(port);
        client_addr.sin_addr.s_addr = (ipaddr == NULL) ? htonl(INADDR_ANY) : inet_addr(ipaddr);
        memset(&(client_addr.sin_zero), 0, sizeof(client_addr.sin_zero));
        ret = bind(cfd,(struct sockaddr*)&client_addr,sizeof(client_addr));
         if(0 > ret) {
            modbus_rt_net_close(cfd);
            return ret;
        }
    }
    sprintf(strPort, "%d", sport);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // 支持 IPv4
    hints.ai_socktype = SOCK_STREAM; // 套接字类型
    if (getaddrinfo(saddr, strPort, &hints, &addr_list) != 0) {
        return -MODBUS_RT_HOST_ERROR;
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        if (AF_INET == cur->ai_family) {
            ret = connect(cfd,cur->ai_addr,cur->ai_addrlen);
            if(0 > ret) {
                modbus_rt_net_close(cfd);
                freeaddrinfo(addr_list); // 释放资源
                return ret;
            }
            break;
        }
    }

    freeaddrinfo(addr_list); // 释放资源
    return cfd;
}

int modbus_rt_net_addr2ip(const char* saddr, char *ip)
{
#if defined(_WIN32)     //确保再没有创建socket的时候可以使用getaddrinfo函数
    modbus_rt_init_winsock();
#endif
    struct addrinfo hints, *addr_list, *cur;
    struct sockaddr_in* ipv4 = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // 支持 IPv4
    hints.ai_socktype = SOCK_STREAM; // 套接字类型

    if (0 != getaddrinfo(saddr, NULL, &hints, &addr_list)) {
        return -MODBUS_RT_HOST_ERROR;
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        if (AF_INET == cur->ai_family) {
            ipv4 = (struct sockaddr_in*)cur->ai_addr;
            break;
        }
    }

    if(NULL != ipv4) {
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
    }
    freeaddrinfo(addr_list);
#if defined(_WIN32)
    modbus_rt_cleanup_winsock();
#endif
    return MODBUS_RT_EOK;
}
#endif

#if MODBUS_TCP_SLAVE_FOR_UDP_ENABLE || MODBUS_TCP_MASTER_FOR_UDP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
int modbus_rt_udp_socket_init(char* ipaddr, unsigned int port){
    struct sockaddr_in udp_addr;
    int ufd = modbus_rt_net_socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(0 > ufd) {
       return ufd;
    }
    int optval = 1;
    modbus_rt_net_setsockopt(ufd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    int on_off = 1;	//允许
    modbus_rt_net_setsockopt(ufd, SOL_SOCKET, SO_BROADCAST, &on_off, sizeof(on_off));

     if((NULL != ipaddr) || (0 != port)) {
        if((NULL != ipaddr) &&(0 == strlen(ipaddr))) {
            ipaddr = NULL;
        }
       udp_addr.sin_family = AF_INET;
       udp_addr.sin_port  = htons(port);
       udp_addr.sin_addr.s_addr = (ipaddr == NULL) ? htonl(INADDR_ANY) : inet_addr(ipaddr);
       memset(&(udp_addr.sin_zero), 0, sizeof(udp_addr.sin_zero));
       int ret = bind(ufd,(struct sockaddr*)&udp_addr,sizeof(struct sockaddr));
       if(0 > ret) {
           modbus_rt_net_close(ufd);
           return ret;
       }
    }
    return ufd;
}

int modbus_rt_net_segment(char *ipaddr, uint32_t saddr){
    if(NULL != ipaddr) {
        uint32_t ip = inet_addr(ipaddr);
        uint32_t mask = 0x00FFFFFF;
        if((ip & mask) == (saddr & mask)) {
            return 1;
        }
    } else {
        uint32_t ip, mask;
#if defined(_WIN32)
        char hostname[256];
        mask = 0x00FFFFFF;
        if (gethostname(hostname, sizeof(hostname)) != 0) {
            return 0;
        }
        struct hostent *host_info = gethostbyname(hostname);
        if (host_info == NULL) {
            return 0;
        }
        IN_ADDR  **addr_list = (IN_ADDR **)host_info->h_addr_list;
        for (int i = 0; addr_list[i] != NULL; i++) {
            ip = addr_list[i]->s_addr;
            if((ip & mask) == (saddr & mask))
            {
                return 1;
            }
        }
#elif defined(__linux)
        struct ifaddrs *ifaddr, *ifa;
        mask = 0x00FFFFFF;
        if (getifaddrs(&ifaddr) == -1) {
            return 0;
        }

        int count = 0;
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL) {
                continue;
            }

            if (ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in *addr_local = (struct sockaddr_in *)ifa->ifa_addr;
                ip = addr_local->sin_addr.s_addr;

                if((ip & mask) == (saddr & mask)){
                    freeifaddrs(ifaddr);
                    return 1;
                }
            }
        }
        freeifaddrs(ifaddr);
#endif
    }
    return 0;
}
#endif

#endif
