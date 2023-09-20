/**
 * @file    modbus_rtu.c
 * @brief   基于Agile Modbus 的modbus rtu应用层实现，包含slave和master
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#include "modbus_rtu.h"


#if (MODBUS_RTU_SLAVE_ENABLE) || (MODBUS_RTU_MASTER_ENABLE)

/**
 @verbatim
    如果开启了modbus slave的支持，则需要添加一些函数声明，函数实现在modbus_slave.c中
    因为这些函数无需对外，所以，不需要开放给用户使用
 @endverbatim
 */
#if MODBUS_RTU_SLAVE_ENABLE
    void modbus_slave_util_init(agile_modbus_slave_util_t *util);
    int modbus_slave_add_val(agile_modbus_slave_util_t *util, modbus_register_type_t type,
                            int data_addr, void *data, int nums);
    int modbus_slave_clear_val(agile_modbus_slave_util_t *util);
    int modbus_slave_read(agile_modbus_slave_util_t *util, modbus_register_type_t type, int addr, int quantity, void *ptr_data);
    int modbus_slave_write(agile_modbus_slave_util_t *util, modbus_register_type_t type, int addr, int quantity, void *ptr_data);
#endif

/**
 @verbatim
    如果使能modbus ascii的支持，这里采用了morbus ascii数据和rtu数据转换函数的形式
    对收发的数据进行转换，从而实现对modbus ascii的支持
 @endverbatim
 */
#if MODBUS_ASCII_SLAVE_ENABLE || MODBUS_ASCII_MASTER_ENABLE
    void modbus_ascii2rtu(uint8_t *data, int *data_len);
    void modbus_rtu2ascii(uint8_t *data, int *data_len);
#endif

/**
 * @brief   modbus_rtu_dev_init:    modbus rtu初始化数据函数，把modbus rtu的数据赋值到全局指针当中
 * @param   pos_dev:                指向rtu_modbus_device_t的指针，
 * @param   mode                    模式，为SLAVE/MASTER两种情况
 * @param   data                    设备数据指针
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_rtu_dev_init(rtu_modbus_device_t * pos_dev, modbus_mode_type mode, void *data) {
    int ret = 0;
    if(NULL != (*pos_dev)) {
        return -MODBUS_RT_EINVAL;
    }

    *pos_dev = modbus_rt_malloc(sizeof(struct rtu_modbus_device));
    if(NULL == (*pos_dev)) {
        return -MODBUS_RT_ENOMEM;
    }
    rtu_modbus_device_t dev = *pos_dev;

    memset(dev,0,sizeof(struct rtu_modbus_device));
    dev->mode = mode;
    ret = modbus_rt_sem_init(&dev->sem);
    if(MODBUS_RT_EOK != ret) {
        modbus_rt_free(dev);
        return ret;
    }
    ret = modbus_rt_mutex_init(&dev->mutex);
    if(MODBUS_RT_EOK != ret) {
        modbus_rt_sem_destroy(&(dev->sem));
        modbus_rt_free(dev);
        return ret;
    } 
    dev->data = data;
    return MODBUS_RT_EOK;
}


#if MODBUS_RTU_SLAVE_ENABLE

/**
 * @brief   modbus_rtu_slave_data_create:   rtu_slave_data_t数据创建函数
 * @param   None
 * @return  rtu_slave_data_t:               rtu_slave_data_t 数据指针
 *       
 */
static rtu_slave_data_t  modbus_rtu_slave_data_create(void) {
    rtu_slave_data_t data = modbus_rt_calloc(1, sizeof(struct rtu_slave_data));
    if(NULL == data) {
        return NULL;
    }
    memset(data,0,sizeof(struct rtu_slave_data));
    return data;
}


#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
/**
 * @brief   rtu_slave_close_client_sock:    关闭连接到modbus Slave上的设备socket
 * @param   p_sock:                         指向client socket数组的指针
 * @return  None
 *       
 */
static void rtu_slave_close_client_sock( int *p_sock ) {
    //清除被链接的socket资源
    for(int i = 0; i < SOCKET_CONNECT_NUMS ; i++) {
      if(-1 == p_sock[i]) {
          modbus_rt_net_close(p_sock[i]);
          p_sock[i] = -1;
#if TCP_MODBUS_NUMS_ENABLE         
          tcp_modbus_nums--;
#endif 
      }
    }
}

/**
 * @brief   modbus_rtu_slave_net_entry:     modbus rtu slave over net的入口函数
 * @param   parameter:                      入口参数， rtu_modbus_device_t 设备
 * @return  无
 *       
 */
static void modbus_rtu_slave_net_entry(rtu_modbus_device_t dev) {
    if((NULL == dev) || (OVER_NET != dev->over_type) || (0 >= dev->port) || 
    ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type))) {
        return ;
    }
    rtu_slave_data_t    data = (rtu_slave_data_t)dev->data;
    int send_len = dev->send_len;
    int read_len = dev->read_len;  
    agile_modbus_t *ctx = dev->ctx; 

    fd_set sock_all_set, sock_read_set;                 //存储socket的索引，用于select参数使用
    int cfd = 0, maxfd = 0, i = 0, nready = 0, ret = 0;                     //零时存储数据
    int sock_client[SOCKET_CONNECT_NUMS] = {0};               //连接到slave上的socket
    struct sockaddr_in client_addr = {0};                 //存储连接客户端的数据信息
    socklen_t client_addr_len = sizeof(client_addr);        //存储链接到改server上的client的IP地址信息

    struct timeval timeout= {0};                    //用于设置select的超时值

    //初始化 maxfd 等于 sock_server
    maxfd = dev->sock;
    //清空fdset
    FD_ZERO(&sock_all_set);
    //把sfd文件描述符添加到集合中
    FD_SET(dev->sock, &sock_all_set);
    if(SOCK_STREAM == dev->type) {
        //初始化客户端fd的集合
        for(i = 0; i < SOCKET_CONNECT_NUMS ; i++) {
            //初始化为-1
            sock_client[i] = -1;
        }
    }
    while(1) {
        //如果接收到关闭的命令
        if(0 == dev->thread_flag) {
            //清除连接到tcp server上的tcp client数据
            if(SOCK_STREAM == dev->type) {
                rtu_slave_close_client_sock(sock_client);
            }

            /* 结束线程运行标志 */
            modbus_rt_mutex_lock(&(dev->mutex));
            dev->thread_flag = -1;           //线程运行
            modbus_rt_mutex_unlock(&(dev->mutex));

            modbus_rt_sem_post(&(dev->sem));
            return ;
        }
        //设置超时时间为100ms
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        //每次select返回之后，fd_set集合就会变化，再select时，就不能使用，
        //所以我们要保存设置fd_set 和 读取的fd_set
        sock_read_set = sock_all_set;
        nready = select(maxfd + 1, &sock_read_set, NULL, NULL, &timeout);
        //没有超时机制，不会返回0
        if(0 > nready) {
            //系统错误，线程异常退出
            printf("0 > nready:%d.\n", nready);
            //清除连接到tcp server上的tcp client数据
            if(SOCK_STREAM == dev->type) {
                rtu_slave_close_client_sock(sock_client);
            }

            /* 结束线程运行标志 */
            modbus_rt_mutex_lock(&(dev->mutex));
            dev->thread_flag = -1;           //线程运行
            modbus_rt_mutex_unlock(&(dev->mutex));

            modbus_rt_sem_post(&(dev->sem));
            return ;
        } else if (0 == nready) {
            continue;       // 超时，继续等待
        }
#if MODBUS_SERIAL_OVER_TCP_ENABLE
        if(SOCK_STREAM == dev->type) {
            //判断监听的套接字是否有数据
            if(FD_ISSET(dev->sock, &sock_read_set)) {
                //有客户端进行连接了
               cfd = modbus_rt_net_accept(dev->sock, (struct sockaddr *)&client_addr, &client_addr_len);
               if(cfd < 0) {
                   continue;  //继续select
               }
#if TCP_MODBUS_NUMS_ENABLE  
               if(tcp_modbus_nums >= TCP_MODBUS_NUMS) {
                   modbus_rt_net_close(cfd);
                   continue;
               }
#endif
                //把新的cfd 保存到cfds集合中
               for(i = 0; i < SOCKET_CONNECT_NUMS ; i++) {
                  if(sock_client[i] == -1) {
                      sock_client[i] = cfd;
#if TCP_MODBUS_NUMS_ENABLE                       
                      tcp_modbus_nums++;
 #endif
                      break;
                  }
               }

                //如果i超过SOCKET_CONNECT_NUMS，表示client已经超标
                if(i >= SOCKET_CONNECT_NUMS) {
                    modbus_rt_net_close(cfd);
                } else {
                    //把新的cfd 添加到fd_set集合中
                    FD_SET(cfd, &sock_all_set);
                    //更新要select的maxfd
                    maxfd = (cfd > maxfd)?cfd:maxfd;
                    //没有其他套接字需要处理：这里防止重复工作，就不去执行其他任务
                    if(--nready == 0) {
                        continue;   //继续select
                    }
                }
            }
            //遍历所有的客户端文件描述符
            for(i = 0; i < SOCKET_CONNECT_NUMS ; i++) {
                // 如果不是该socket，继续遍历
                if(sock_client[i] == -1) {
                    continue;
                }
                //判断是否在fd_set集合里面
                if(FD_ISSET(sock_client[i], &sock_read_set)) {
                    read_len = recv(sock_client[i], ctx->read_buf, ctx->read_bufsz, 0);
                    if(read_len <= 0) {
                        modbus_rt_net_close(sock_client[i]);
                        //从集合里面清除
                        FD_CLR(sock_client[i], &sock_all_set);
                        //当前的客户端fd 赋值为-1
                        sock_client[i] = -1;
#ifdef TCP_MODBUS_NUMS  
                        tcp_modbus_nums--;
#endif

                        continue;
                    }
#if MODBUS_ASCII_SLAVE_ENABLE
                    if(dev->ascii_flag) {
                        modbus_ascii2rtu(ctx->read_buf, &read_len);
                    }
#endif                    
                    send_len = agile_modbus_slave_handle(ctx, read_len, data->slave_strict, agile_modbus_slave_util_callback, &data->util, NULL);
                    if (send_len > 0) {
#if MODBUS_ASCII_SLAVE_ENABLE
                        if(dev->ascii_flag) {
                            modbus_rtu2ascii(ctx->send_buf, &send_len);
                        }
#endif
                        ret = send(sock_client[i], ctx->send_buf, send_len, 0);
                        if(ret <= 0) {
                            continue;       //发送失败，有可能时client端断开了，在下一次循环的select中会关闭该socket
                        }
                    }
                }
            }
        }
#endif
#if MODBUS_SERIAL_OVER_UDP_ENABLE
    if(SOCK_DGRAM == dev->type) {
            //处理udp的数据
            read_len = recvfrom(dev->sock, ctx->read_buf, ctx->read_bufsz, 0, (struct sockaddr*)&client_addr, &client_addr_len);
            if (read_len <= 0) {
                continue;
            }
#if MODBUS_ASCII_SLAVE_ENABLE
            if(dev->ascii_flag) {
                modbus_ascii2rtu(ctx->read_buf, &read_len);
            }
#endif             
            send_len = agile_modbus_slave_handle(ctx, read_len, data->slave_strict, agile_modbus_slave_util_callback, &data->util, NULL);
            if (send_len > 0) {   
#if MODBUS_UDP_FOR_SEARCH       //跨网段设备广播检测
                if(modbus_rt_net_segment(dev->ipaddr, client_addr.sin_addr.s_addr) == 0) {
                    client_addr.sin_addr.s_addr = 0xffffffff;
                }
#endif
#if MODBUS_ASCII_SLAVE_ENABLE
                if(dev->ascii_flag) {
                    modbus_rtu2ascii(ctx->send_buf, &send_len);
                }
#endif
                ret = sendto(dev->sock, ctx->send_buf, send_len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
                if(ret <= 0) {
                    printf("send error.\n");
                    continue;
                }
            }
        }
#endif
    }
}
#endif

/**
 * @brief   modbus_rtu_slave_entry:         modbus rtu slave的入口函数
 * @param   parameter:                      入口参数， rtu_modbus_device_t 设备
 * @return  无
 *       
 */
static void modbus_rtu_slave_entry(void *parameter) {
    rtu_modbus_device_t dev = (rtu_modbus_device_t)parameter; 
    rtu_slave_data_t    data = (rtu_slave_data_t)dev->data;
    int send_len = dev->send_len;
    int read_len = dev->read_len;      
    agile_modbus_t *ctx = dev->ctx;

    agile_modbus_rtu_init(&(dev->ctx_rtu), dev->ctx_send_buf, send_len, dev->ctx_read_buf, read_len);
    agile_modbus_set_slave(ctx, data->addr);
    /* 启动线程运行标志 */
    modbus_rt_mutex_lock(&(dev->mutex));
    dev->thread_flag = 1;           //线程运行
    modbus_rt_mutex_unlock(&(dev->mutex));
    if (OVER_NONE == dev->over_type) {
        while(1) {
            //如果接收到关闭的命令
            if(0 == dev->thread_flag) {
                /* 结束线程运行标志 */
                modbus_rt_mutex_lock(&(dev->mutex));
                dev->thread_flag = -1;           //线程运行
                modbus_rt_mutex_unlock(&(dev->mutex));

                modbus_rt_sem_post(&(dev->sem));
                return ;
            }
            read_len = modbus_rt_serial_receive(dev->serial, ctx->read_buf, ctx->read_bufsz, MODBUS_RTU_TIME_OUT, dev->byte_timeout);
            if (read_len <= 0) {
                continue;
            }
#if MODBUS_ASCII_SLAVE_ENABLE
            if(dev->ascii_flag) {
                modbus_ascii2rtu(ctx->read_buf, &read_len);
            }
#endif 
            send_len = agile_modbus_slave_handle(ctx, read_len, data->slave_strict, agile_modbus_slave_util_callback, &data->util, NULL);
            if (send_len > 0) {
#if MODBUS_ASCII_SLAVE_ENABLE
                if(dev->ascii_flag) {
                    modbus_rtu2ascii(ctx->send_buf, &send_len);
                }
#endif    
                modbus_rt_serial_send(dev->serial, ctx->send_buf, send_len);
            }
        }

    } else if(OVER_NET == dev->over_type) {
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
        modbus_rtu_slave_net_entry(dev);
#endif
    }
}
#endif


#if MODBUS_RTU_MASTER_ENABLE
/**
 * @brief   modbus_rtu_master_data_create:  tcp_master_data_t数据创建函数
 * @param   None
 * @return  rtu_master_data_t:              rtu_master_data_t 数据指针
 *       
 */
static rtu_master_data_t  modbus_rtu_master_data_create(void) {
    rtu_master_data_t data = modbus_rt_calloc(1, sizeof(struct rtu_master_data));
    if(NULL == data) {
        return NULL;
    }
    memset(data,0,sizeof(struct rtu_master_data));
    return data;
}

#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
/**
 * @brief   modbus_rtu_master_net_txrx:     rtu_master over net数据发送接收
 * @param   dev:                            rtu_modbus_device_t 设备
 * @param   ctx:                            modbus句柄
 * @param   send_len:                       需要发送数据的长度
 * @return  int:                            MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_rtu_master_net_txrx(rtu_modbus_device_t dev, agile_modbus_t *ctx, int send_len) {
    int read_len = 0;
    fd_set sock_read_set;                                 //存储socket的索引，用于select参数使用
    int maxfd, nready;                                     //零时存储数据
    struct timeval timeout= {0};                            //用于设置select的超时值
    /* 清空可读事件描述符列表 */
    FD_ZERO(&sock_read_set);
    /* 将需要监听可读事件的描述符加入列表 */
    FD_SET(dev->sock, &sock_read_set);
    /* 获取需要监听的描述符号最大值 */
    maxfd = dev->sock;

    //设置超时时间为1s
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000000;
    if(dev->type == SOCK_STREAM) {
#if MODBUS_ASCII_SLAVE_ENABLE
        int send_len_temp = send_len;
        uint8_t send_buf_temp[(AGILE_MODBUS_MAX_ADU_LENGTH + 1) * 2] = {0};
        if(dev->ascii_flag) {
            memcpy(send_buf_temp, ctx->send_buf, send_len_temp);
            modbus_rtu2ascii(send_buf_temp, &send_len_temp);
            send(dev->sock, send_buf_temp, send_len_temp, 0);
        } else {
            send(dev->sock, ctx->send_buf, send_len, 0);
        }
#else
        send(dev->sock, ctx->send_buf, send_len, 0);
#endif 
        nready = select(maxfd + 1, &sock_read_set, NULL, NULL, &timeout);
        if(nready < 0) {      //错误
            return -MODBUS_RT_ERROR;
        }
        if(nready == 0) {         //超时
            return -MODBUS_RT_ETIMEOUT;
        }
        read_len = recv(dev->sock, ctx->read_buf, ctx->read_bufsz, 0);
        if (read_len <= 0) {     //sock断开了
            return -MODBUS_RT_EBUSY;
        }
#if MODBUS_ASCII_SLAVE_ENABLE
        if(dev->ascii_flag) {
            modbus_ascii2rtu(ctx->read_buf, &read_len);
        }
#endif 
    }
#if MODBUS_SERIAL_OVER_UDP_ENABLE
    else if(dev->type == SOCK_DGRAM) {
        struct sockaddr_in server_addr = {0};                 //存储连接udp server 的数据信息
        socklen_t server_addr_len = sizeof(server_addr);        //存储链接到改server上的client的IP地址信息

        rtu_master_data_t    data = (rtu_master_data_t)dev->data;         //tcp_master数据
        /* 设置服务端地址 */
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(data->sport);
        server_addr.sin_addr.s_addr = inet_addr(data->saddr);
        memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
#if MODBUS_ASCII_SLAVE_ENABLE
        int send_len_temp = send_len;
        uint8_t send_buf_temp[(AGILE_MODBUS_MAX_ADU_LENGTH + 1) * 2] = {0};
        if(dev->ascii_flag) {
            memcpy(send_buf_temp, ctx->send_buf, send_len_temp);
            modbus_rtu2ascii(send_buf_temp, &send_len_temp);
            sendto(dev->sock, send_buf_temp, send_len_temp, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        } else {
            sendto(dev->sock, ctx->send_buf, send_len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        }
#else
        sendto(dev->sock, ctx->send_buf, send_len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
#endif 
        nready = select(maxfd + 1, &sock_read_set, NULL, NULL, &timeout);
        if(nready < 0) {
            return -MODBUS_RT_ERROR;
        }
        if(nready == 0) {
            return -MODBUS_RT_ETIMEOUT;
        }
        read_len = recvfrom(dev->sock, ctx->read_buf, ctx->read_bufsz, 0, (struct sockaddr*)&server_addr, &server_addr_len);
        if (read_len <= 0) {
            return -MODBUS_RT_EBUSY;
        }
#if MODBUS_UDP_FOR_SEARCH
        if(0 == strcmp(data->saddr, "255.255.255.255")) {
            inet_ntop(AF_INET, &(server_addr.sin_addr), data->saddr, INET_ADDRSTRLEN);
        }
#endif
#if MODBUS_ASCII_SLAVE_ENABLE
        if(dev->ascii_flag) {
            modbus_ascii2rtu(ctx->read_buf, &read_len);
        }
#endif 
    }
#endif
    return read_len;
}
#endif

/**
 * @brief   modbus_rtu_master_txrx:         rtu_master数据发送接收
 * @param   dev:                            rtu_modbus_device_t 设备
 * @param   ctx:                            modbus句柄
 * @param   send_len:                       需要发送数据的长度
 * @return  int:                            MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_rtu_master_txrx(rtu_modbus_device_t dev, agile_modbus_t *ctx, int send_len) {
    if (OVER_NONE == dev->over_type) {
        int read_len = 0;
#if MODBUS_ASCII_SLAVE_ENABLE
        int send_len_temp = send_len;
        uint8_t send_buf_temp[(AGILE_MODBUS_MAX_ADU_LENGTH + 1) * 2] = {0};
        if(dev->ascii_flag) {
            memcpy(send_buf_temp, ctx->send_buf, send_len_temp);
            modbus_rtu2ascii(send_buf_temp, &send_len_temp);
            modbus_rt_serial_send(dev->serial, send_buf_temp, send_len_temp);
        } else {
            modbus_rt_serial_send(dev->serial, ctx->send_buf, send_len);
        }
#else
        modbus_rt_serial_send(dev->serial, ctx->send_buf, send_len);
#endif  
        read_len = modbus_rt_serial_receive(dev->serial, ctx->read_buf, ctx->read_bufsz, MODBUS_RTU_TIME_OUT, dev->byte_timeout);
        if (read_len <= 0) {     //接收超时
            return -MODBUS_RT_ETIMEOUT;
        }
#if MODBUS_ASCII_SLAVE_ENABLE
        if(dev->ascii_flag) {
            modbus_ascii2rtu(ctx->read_buf, &read_len);
        }
#endif 
        return read_len;
    } else if(OVER_NET == dev->over_type) {
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
        return modbus_rtu_master_net_txrx(dev, ctx, send_len);
#endif
    }
    return -MODBUS_RT_ERROR;
}

/**
 * @brief   modbus_rtu_master_excuse_run:   rtu_master执行执行函数
 * @param   dev:                            rtu_modbus_device_t设备
 * @return  int:                            MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_rtu_master_excuse_run(rtu_modbus_device_t dev) {
    int send_len = 0;             
    int read_len = 0; 
    agile_modbus_t *ctx = dev->ctx;
    rtu_master_data_t data = (rtu_master_data_t)dev->data;
 
    if((AGILE_MODBUS_FC_REPORT_SLAVE_ID !=  data->function) && (0x20 > data->function)) {
        if((0 == data->quantity) || (NULL == data->ptr_data)) {
            data->ret = -MODBUS_RT_ERROR;
            data->function = 0;
            return data->ret;
        }
    }
    agile_modbus_set_slave(ctx, data->slave_addr);
    switch (data->function) {
        case AGILE_MODBUS_FC_READ_COILS: {
            send_len = agile_modbus_serialize_read_bits(ctx, data->data_addr, data->quantity);
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if(read_len < 0) {
                data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_read_bits(ctx, read_len, data->ptr_data);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;    
        } break;
        case AGILE_MODBUS_FC_READ_DISCRETE_INPUTS: {
            send_len = agile_modbus_serialize_read_input_bits(ctx, data->data_addr, data->quantity);
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if(read_len < 0) {
                data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_read_input_bits(ctx, read_len, data->ptr_data);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        case AGILE_MODBUS_FC_READ_HOLDING_REGISTERS: {
            send_len = agile_modbus_serialize_read_registers(ctx, data->data_addr, data->quantity);
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if(read_len < 0) {
                data->ret = read_len;   
                break;
            }
            int rc = agile_modbus_deserialize_read_registers(ctx, read_len, data->ptr_data);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        case AGILE_MODBUS_FC_READ_INPUT_REGISTERS: {
            send_len = agile_modbus_serialize_read_input_registers(ctx, data->data_addr, data->quantity);
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if(read_len < 0) {
                data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_read_input_registers(ctx, read_len, data->ptr_data);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        case AGILE_MODBUS_FC_WRITE_SINGLE_COIL: {
            //高电平写FF00，低电平写0, 再agile_modbus_serialize_write_bit已经区分了
            send_len = agile_modbus_serialize_write_bit(ctx, data->data_addr, *((int *)data->ptr_data));
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if(read_len < 0) {
                data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_write_bit(ctx, read_len);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        case AGILE_MODBUS_FC_WRITE_SINGLE_REGISTER: {
            send_len = agile_modbus_serialize_write_register(ctx, data->data_addr, *((uint16_t *)data->ptr_data));
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if (read_len < 0) {
                data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_write_register(ctx, read_len);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        case AGILE_MODBUS_FC_WRITE_MULTIPLE_COILS: {
            send_len = agile_modbus_serialize_write_bits(ctx, data->data_addr, data->quantity, (uint8_t *)data->ptr_data);
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if (read_len < 0) {
                data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_write_bits(ctx, read_len);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        case AGILE_MODBUS_FC_WRITE_MULTIPLE_REGISTERS: {
            send_len = agile_modbus_serialize_write_registers(ctx, data->data_addr, data->quantity, (uint16_t *)data->ptr_data);
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if (read_len < 0) {
                 data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_write_registers(ctx, read_len);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        case AGILE_MODBUS_FC_REPORT_SLAVE_ID: {
            send_len = agile_modbus_serialize_report_slave_id(ctx);
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if (read_len < 0) {
                 data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_report_slave_id(ctx, read_len,data->quantity,  (uint8_t *)data->ptr_data);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        case AGILE_MODBUS_FC_MASK_WRITE_REGISTER: {
            send_len = agile_modbus_serialize_mask_write_register(ctx, data->data_addr,
                    *((uint16_t *)data->ptr_data), *(((uint16_t *)data->ptr_data) + 1));
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if (read_len < 0) {
                 data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_mask_write_register(ctx, read_len);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        case AGILE_MODBUS_FC_WRITE_AND_READ_REGISTERS:
        {
            send_len = agile_modbus_serialize_write_and_read_registers(ctx, data->data_addr, data->quantity,
                    (uint16_t *)data->ptr_data, data->read_addr, data->read_quantity);
            read_len = modbus_rtu_master_txrx(dev,ctx,send_len);
            if (read_len < 0) {
                 data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_write_and_read_registers(ctx, read_len, (uint16_t *)data->ptr_read_data);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
        default:
        {
            data->ret =-MODBUS_RT_ERROR;
        } break;
    }
    data->function = 0;
    return data->ret;
}

#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
/**
 * @brief   modbus_rtu_master_net_entry:    rtu_master over net的入口函数
 * @param   parameter:                      入口参数，rtu_modbus_device_t设备
 * @return  无
 *       
 */
static void modbus_rtu_master_net_entry(rtu_modbus_device_t dev) {
    if((NULL == dev) || (OVER_NET != dev->over_type) || (0 > dev->port) || 
    ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type))) {
        return ;
    }
    rtu_master_data_t    data = (rtu_master_data_t)dev->data;
    static int sock_timeout = 0;
    fd_set sock_read_set;                           //存储socket的索引，用于select参数使用
    int maxfd, nready;                              //零时存储数据
    struct timeval timeout= {0};                    //用于设置select的超时值
    while(1) {
        if(0 == dev->thread_flag) {
            /* 结束线程运行标志 */
            modbus_rt_mutex_lock(&(dev->mutex));
            dev->thread_flag = -1;           //线程运行
            modbus_rt_mutex_unlock(&(dev->mutex));

            modbus_rt_sem_post(&(dev->sem));
            return ;
        }
        if(0 < data->function) {  
            //如果socket被远程tcp server断开，当有指令时，尝试重新连接
            if((SOCK_STREAM == dev->type) && (0 >= dev->sock)) {
                dev->sock = modbus_rt_tcp_client_init(NULL, dev->port, data->saddr, data->sport);
                if(0 >= dev->sock) {
                    data->ret = -MODBUS_RT_REMOTE;
                    data->function = 0;

                    /* fuction执行完毕 */
                    modbus_rt_sem_post(&(data->completion));
                    continue;
                }
            }
            sock_timeout = 0;
            //连接成功之后，执行rtu master指令
            modbus_rtu_master_excuse_run(dev);
            /* fuction执行完毕 */
            modbus_rt_sem_post(&(data->completion));
        } else if ((SOCK_STREAM == dev->type) && (0 < dev->sock)) {
            sock_timeout = 0;
            //检错socket是不是被远程的服务器端断开,仅限TCP使用
            int read_len = 0;
            uint8_t read_buf[AGILE_MODBUS_MAX_ADU_LENGTH] = {0};
            /* 清空可读事件描述符列表 */
            FD_ZERO(&sock_read_set);
            /* 将需要监听可读事件的描述符加入列表 */
            FD_SET(dev->sock, &sock_read_set);
            /* 获取需要监听的描述符号最大值 */
            maxfd = dev->sock;

            //设置超时时间为50ms
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;

            nready = select(maxfd + 1, &sock_read_set, NULL, NULL, &timeout);
            //负数表示select错误
            if(0 > nready) {
                //系统错误，线程异常退出

                /* 结束线程运行标志 */
                modbus_rt_mutex_lock(&(dev->mutex));
                dev->thread_flag = -1;           //线程运行
                modbus_rt_mutex_unlock(&(dev->mutex));

                modbus_rt_sem_post(&(dev->sem));
                return ;

            } else if (0 == nready) {
                // 超时，继续等待
                continue;
            }
            read_len = recv(dev->sock, read_buf, AGILE_MODBUS_MAX_ADU_LENGTH, 0);       //尝试读取数据
            if (0 > read_len) {    
                //系统错误，线程异常退出
                printf("recv:0 > read_le.\n");
                /* 结束线程运行标志 */
                modbus_rt_mutex_lock(&(dev->mutex));
                dev->thread_flag = -1;           //线程运行
                modbus_rt_mutex_unlock(&(dev->mutex));

                modbus_rt_sem_post(&(dev->sem));
                return ;
            } else if (0 == read_len) {     
                //等于0表示服务端断开，下次会短线自动重连
                modbus_rt_net_close(dev->sock);
                dev->sock = 0;
                continue;
            }
        }
#if MODBUS_TCP_MASTER_RECONNECT 
        else if(0 >= dev->sock) {
            modbus_rt_thread_sleep(1);        //休眠1ms，让线程调度,2s重连一次
            if((++sock_timeout) >= MODBUS_TCP_MASTER_RECONNECT_TIME_OUT) {
                sock_timeout = 0;
                //如果socket被远程tcp server断开，当有指令时，尝试重新连接
                if((SOCK_STREAM == dev->type) && (0 >= dev->sock)) {
                    dev->sock = modbus_rt_tcp_client_init(NULL, dev->port, data->saddr, data->sport);
                    if(0 >= dev->sock) {
                        continue;
                    }
                }
            }
        }
#endif
    }
}
#endif

/**
 * @brief   modbus_rtu_master_entry:        rtu_master的入口函数
 * @param   parameter:                      入口参数，rtu_modbus_device_t设备
 * @return  无
 *       
 */
static void modbus_rtu_master_entry(void *parameter) {
    rtu_modbus_device_t dev = (rtu_modbus_device_t)parameter; 
    rtu_master_data_t    data = (rtu_master_data_t)dev->data;
    int send_len = dev->send_len;
    int read_len = dev->read_len;      
    agile_modbus_rtu_init(&(dev->ctx_rtu), dev->ctx_send_buf, send_len, dev->ctx_read_buf, read_len);
    /* 启动线程运行标志 */
    modbus_rt_mutex_lock(&(dev->mutex));
    dev->thread_flag = 1;           //线程运行
    modbus_rt_mutex_unlock(&(dev->mutex));
     if (OVER_NONE == dev->over_type) {
        while(1) {
            if(0 == dev->thread_flag) {
                /* 结束线程运行标志 */
                modbus_rt_mutex_lock(&(dev->mutex));
                dev->thread_flag = -1;           //线程运行
                modbus_rt_mutex_unlock(&(dev->mutex));

                modbus_rt_sem_post(&(dev->sem));
                return ;
            }
            //串口通信
            if(0 < data->function) { 
                //连接成功之后，执行rtu master指令
                modbus_rtu_master_excuse_run(dev);
                /* fuction执行完毕 */
                modbus_rt_sem_post(&(data->completion));
            } else {
                modbus_rt_thread_sleep(1);        //休眠1ms，让线程调度,2s重连一次
            }
        }
     } else if(OVER_NET == dev->over_type) {
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
        modbus_rtu_master_net_entry(dev);
#endif       
     }
}
#endif

/**
 * @brief   modbus_rtu:             创造modbus rtu设备
 * @param   mode:                   modbus类型： MODBUS_SLAVE/MODBUS_MASTER
 * @return  rtu_modbus_device_t:    rtu_modbus_device_t数据指针
 *       
 */
rtu_modbus_device_t modbus_rtu(modbus_mode_type mode) {
    int ret = 0;
    rtu_modbus_device_t dev = NULL;
    if((MODBUS_SLAVE != mode) && (MODBUS_MASTER != mode)) {
        return NULL;
    }
#if MODBUS_RTU_SLAVE_ENABLE
#if !MODBUS_RTU_MASTER_ENABLE
    if(MODBUS_MASTER == mode) {
        return NULL;
    }
#endif
    if(MODBUS_SLAVE == mode) {
        rtu_slave_data_t data = modbus_rtu_slave_data_create();
        if(NULL == data) {
            return NULL;
        }
        data->addr = 1;             ///默认地址为1
        data->slave_strict = 1;     //默认开启地址匹配功能
        //初始化modbus tcp slave的设备信息
        ret = modbus_rtu_dev_init(&dev, mode, data);
        if(MODBUS_RT_EOK != ret) {
            modbus_rt_free(data);
            return NULL;
        }
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
        if (OVER_NET == dev->over_type) {
            dev->port = 502;                //默认的端口号为502
            dev->type = SOCK_STREAM;        //默认为TCP模式
        }
#endif
        modbus_slave_util_init(&data->util);
    }
#endif   
#if MODBUS_RTU_MASTER_ENABLE
#if !MODBUS_RTU_SLAVE_ENABLE
    if(MODBUS_SLAVE == mode) {
        return NULL;
    }
#endif
    if(MODBUS_MASTER == mode) {
        rtu_master_data_t data = modbus_rtu_master_data_create();
        if(NULL == data) {
            return NULL;
        }
        ret = modbus_rtu_dev_init(&dev, mode, data);
        if(MODBUS_RT_EOK != ret) {
            modbus_rt_free(data);
            return NULL;
        }
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
        if (OVER_NET == dev->over_type) {
            data->sport = 502;          //服务器端默认端口0
            dev->port = 0;              //Master默认的端口号为随机
            dev->type = SOCK_STREAM;   //默认为TCP模式
        }
#endif
        //初始化master执行指令的完成信号量
        ret = modbus_rt_sem_init(&(data->completion));
        if(MODBUS_RT_EOK != ret) {
            modbus_rt_free(data);
            return NULL;
        }
    }
#endif
    dev->ctx = &(dev->ctx_rtu._ctx);  
    return dev;
}

/**
 * @brief   modbus_rtu_set_serial:  设置modbus rtu的串口信息
 * @param   dev:                    rtu_modbus_device_t设备
 * @param   devname:                串口设备名称
 * @param   baudrate:               波特率
 * @param   parity:                 校验位：'N', 'E', 'O', 'M', 'S' 
 * @param   stopbits:               停止位：1，2
 * @param   xonxoff:                控制流xonxoff开关，暂时不支持其他流控制模式
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_set_serial(rtu_modbus_device_t dev, const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff) {
    if((NULL == dev) || (NULL == devname)) {
        return -MODBUS_RT_EINVAL;
    }
    int len = strlen(devname);
    if((MODBUS_RTU_NAME_MAX <= len) || (5  > bytesize) || (8 < bytesize)) {
         return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    memcpy(dev->serial_info.devname, devname, len);
    dev->serial_info.devname[len] = 0;
    dev->serial_info.baudrate = baudrate;
    dev->serial_info.bytesize = bytesize;
    dev->serial_info.parity = parity;
    dev->serial_info.stopbits = stopbits;
    dev->serial_info.xonxoff = xonxoff;
    return MODBUS_RT_EOK;
}

#if MODBUS_ASCII_SLAVE_ENABLE || MODBUS_ASCII_MASTER_ENABLE
/**
 * @brief   modbus_rtu_set_ascii_flag:  modbus ascii开关设置
 * @param   dev:                    tcp_modbus_device_t设备
 * @param   flag:                   0：rtu模式，1：ascii模式
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_rtu_set_ascii_flag(rtu_modbus_device_t dev, int flag) {
    if((NULL == dev) || ((0 != flag) && (1 != flag))) {
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    dev->ascii_flag = flag;
    return MODBUS_RT_EOK;
}
#endif

/**
 * @brief   modbus_rtu_open:    开启modbus rtu启动通信相关的线程 
 * @param   dev:                rtu_modbus_device_t设备
 * @return  int:                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_open(rtu_modbus_device_t dev) {
    int id_thread = 0;
    char str_name[MODBUS_RT_NAME_MAX] = {0};
    if((NULL == dev) || (NULL == dev->data)) {
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
#if MODBUS_RTU_SLAVE_ENABLE
#if !MODBUS_RTU_MASTER_ENABLE
    if(MODBUS_MASTER == dev->mode) {
        return -MODBUS_RT_EINVAL;
    }
#endif   
#endif   
#if MODBUS_RTU_MASTER_ENABLE
#if !MODBUS_RTU_SLAVE_ENABLE
    if(MODBUS_SLAVE == dev->mode) {
        return -MODBUS_RT_EINVAL;
    }
#endif
    rtu_master_data_t data_master = NULL;
    if(MODBUS_MASTER == dev->mode) {
        data_master = (rtu_master_data_t)dev->data;
    }
#endif
    if (OVER_NONE == dev->over_type) {
        struct modbus_rt_serial_info info = dev->serial_info;
        dev->byte_timeout = (MODBUS_RTU_TIME_OUT_BITS * 1000000)/info.baudrate;
        if(dev->byte_timeout < (MODBUS_RTU_BYTE_TIME_OUT_MIN * 1000)) {         //转化为ms。此处注意，建议最短的字符间距设置不低于3ms(1750us)。
            dev->byte_timeout = MODBUS_RTU_BYTE_TIME_OUT_MIN;
        } else {
            dev->byte_timeout = dev->byte_timeout/1000 + 1;
        }
        int serial = modbus_rt_serial_open(info.devname, info.baudrate, info.bytesize, 
        info.parity, info.stopbits, info.xonxoff);
        if(0  > serial) {
             return serial;
        }
        id_thread = serial;
        dev->serial = serial;
    } else if (OVER_NET == dev->over_type) {
 #if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
        int sock = -1;
#if MODBUS_RTU_MASTER_ENABLE
        int64_t net_addr_temp = (int64_t)inet_addr(data_master->saddr);
        if(net_addr_temp < 0) {
            return -MODBUS_RT_EINVAL;
        }
#endif      
 #if TCP_MODBUS_NUMS_ENABLE
        if(tcp_modbus_nums >= TCP_MODBUS_NUMS) {
            return -MODBUS_RT_EINVAL;
        }
#endif
        if(SOCK_STREAM == dev->type) {
#if MODBUS_RTU_SLAVE_ENABLE
            if(MODBUS_SLAVE == dev->mode) {
                sock = modbus_rt_tcp_server_init(dev->ipaddr, dev->port, SOCKET_CONNECT_NUMS);
            }
#endif
#if MODBUS_RTU_MASTER_ENABLE
            if(MODBUS_MASTER == dev->mode) {
                sock = modbus_rt_tcp_client_init(dev->ipaddr, dev->port, data_master->saddr, data_master->sport);
            }
#endif
        } else if(SOCK_DGRAM == dev->type) {
#if MODBUS_SERIAL_OVER_UDP_ENABLE 
            sock = modbus_rt_udp_socket_init(dev->ipaddr, dev->port);
#else
        return -MODBUS_RT_EINVAL;
#endif
        }
        if (0 > sock) {
            return sock;
        }
        id_thread = sock;
        dev->sock = sock;
#else
        return -MODBUS_RT_EINVAL;
#endif              
    }
    dev->send_len = AGILE_MODBUS_MAX_ADU_LENGTH;
    dev->read_len = AGILE_MODBUS_MAX_ADU_LENGTH;
    dev->ctx_send_buf = modbus_rt_calloc(1, dev->send_len);
    if(NULL == dev->ctx_send_buf) {
        if (OVER_NONE == dev->over_type) {
            modbus_rt_serial_close(dev->serial);
            dev->serial = 0;
        }else if (OVER_NET == dev->over_type) {
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
            modbus_rt_net_close(dev->sock);                      /* 关闭socket */  
            dev->sock = 0;
#endif
        }
        return -MODBUS_RT_ENOMEM;
    }
    dev->ctx_read_buf = modbus_rt_calloc(1, dev->read_len);
    if(NULL == dev->ctx_read_buf) {
        modbus_rt_free(dev->ctx_send_buf);
        if (OVER_NONE == dev->over_type) {
            modbus_rt_serial_close(dev->serial);
            dev->serial = 0;
        }else if (OVER_NET == dev->over_type) {
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
            modbus_rt_net_close(dev->sock);                      /* 关闭socket */  
            dev->sock = 0;
#endif
        }
        return -MODBUS_RT_ENOMEM;
    }
    //创建线程
#if MODBUS_RTU_SLAVE_ENABLE
    if(MODBUS_SLAVE == dev->mode) {
        sprintf(str_name,"rtu_ts%d",id_thread);
        dev->thread = modbus_rt_thread_init(str_name, modbus_rtu_slave_entry, 
                dev, MODBUS_THREAD_STACK_SIZE, MODBUS_THREAD_PRIO, 10);
    }
#endif
#if MODBUS_RTU_MASTER_ENABLE
    if(MODBUS_MASTER == dev->mode) {
        sprintf(str_name,"rtu_tm%d",id_thread);
        dev->thread = modbus_rt_thread_init(str_name, modbus_rtu_master_entry, 
                dev, MODBUS_THREAD_STACK_SIZE, MODBUS_THREAD_PRIO, 10);
    }
#endif
    if(NULL != dev->thread) {
        modbus_rt_thread_startup(dev->thread);
    } else {
        modbus_rt_free(dev->ctx_send_buf);
        modbus_rt_free(dev->ctx_read_buf);
        if (OVER_NONE == dev->over_type) {
            modbus_rt_serial_close(dev->serial);
            dev->serial = 0;
        }else if (OVER_NET == dev->over_type) {
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
            modbus_rt_net_close(dev->sock);                      /* 关闭socket */  
            dev->sock = 0;
#endif
        }
        return -MODBUS_RT_EINVAL;
    }
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE    
#if TCP_MODBUS_NUMS_ENABLE
    tcp_modbus_nums++;
#endif
#endif
    dev->status = 1;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_isopen:   关闭modbus rtu启动通信相关的线程 
 * @param   dev:                rtu_modbus_device_t设备
 * @return  int:                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_isopen(rtu_modbus_device_t dev) {
    if ((NULL == dev)) { 
        return 0;
    }
    return dev->status;
}

/**
 * @brief   modbus_rtu_close:   关闭modbus rtu启动通信相关的线程 
 * @param   dev:                rtu_modbus_device_t设备
 * @return  int:                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_close(rtu_modbus_device_t dev) {
    if(!modbus_rtu_isopen(dev))
    {
        return -MODBUS_RT_ERROR;
    }
    if(1 == dev->thread_flag) {
         /* 尝试结束线程 */
        modbus_rt_mutex_lock(&(dev->mutex));
        dev->thread_flag = 0;           //尝试结束线程
        modbus_rt_mutex_unlock(&(dev->mutex));

        /* 等待设备线程结束 */
        modbus_rt_sem_wait(&(dev->sem));
    }
    modbus_rt_free(dev->ctx_send_buf);
    modbus_rt_free(dev->ctx_read_buf);
    if (OVER_NONE == dev->over_type) {
        if(0 < dev->serial) {
            modbus_rt_serial_close(dev->serial);            /* 关闭socket */ 
            dev->serial = 0;
        }
    } else if(OVER_NET == dev->over_type) {
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
        modbus_rt_net_close(dev->sock); 
#if TCP_MODBUS_NUMS_ENABLE         
        tcp_modbus_nums--;
#endif
        dev->sock = 0; 
#else
        return -MODBUS_RT_EINVAL;
#endif 
    }
    dev->status = 0;
    modbus_rt_thread_destroy(dev->thread);

    dev->thread = NULL;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_destroy: modbus rtu销毁函数
 * @param   pos_dev:            指向rtu_modbus_device_t的指针，
 *                              这里主要目的需要通过指针删除相应的动态分配数据内容
 * @return  int:                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_destroy(rtu_modbus_device_t * pos_dev) {
    if((NULL == pos_dev) || (NULL == (*pos_dev))) {
        return -MODBUS_RT_EINVAL;
    }
    rtu_modbus_device_t dev = *pos_dev; 
    if (NULL == dev) { 
        return -MODBUS_RT_EINVAL;
    }  
    if(dev->thread != 0) {
        modbus_rtu_close(dev);
    }
#if MODBUS_RTU_SLAVE_ENABLE
    if(MODBUS_SLAVE == dev->mode) {
        rtu_slave_data_t data = (rtu_slave_data_t)dev->data; 
        modbus_slave_clear_val(&(data->util));
        modbus_rt_free(data);
    }
#endif   
#if MODBUS_RTU_MASTER_ENABLE
    if(MODBUS_MASTER == dev->mode) {
        rtu_master_data_t data = (rtu_master_data_t)dev->data;  
        modbus_rt_sem_destroy(&(data->completion));
        modbus_rt_free(data);
    }
#endif
    modbus_rt_mutex_destroy(&(dev->mutex));
    modbus_rt_sem_destroy(&(dev->sem));
    modbus_rt_free(dev);
    *pos_dev = NULL;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_excuse:  modbus rtu执行函数：读取或者写入寄存器的值
 *          该函数为阻塞函数，执行完成或者超时后返回(采用信号量机制，不影响其他线程运行)。
 *          注意：不存在的寄存器读取和写入不会报错，但是永远为0。
 * 
 * @param   dev:                rtu_modbus_device_t设备
 * @param   dir_slave:          针对slave设备：操作方向：MODBUS_READ（读寄存器）； MODBUS_WRITE（写寄存器）
 *                              针对master设备：modbus slave的地址
 * @param   type_function:      针对slave设备：寄存器类型: CIOLS(0), INPUTS(1), INPUT_REGISTERS(3), REGISTERS(4)四种
 *                              针对master设备：modbus命令
 * @param   addr:               需要操作的寄存器地址
 * @param   quantity:           寄存器的数量
 * @param   ptr_data:           需要读取或者写入的内容，不过不需要则为NULL
 * @return  int:                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_excuse(rtu_modbus_device_t dev, int dir_slave, int type_function, int addr, int quantity, void *ptr_data) {
#if MODBUS_RTU_SLAVE_ENABLE
    if(MODBUS_SLAVE == dev->mode) {
    rtu_slave_data_t data = NULL;
    if((NULL == dev) || (NULL == dev->data) || 
        ((MODBUS_READ  != dir_slave) && (MODBUS_WRITE != dir_slave)) || (1 != dev->status)) {
        return -MODBUS_RT_EINVAL;
    }
    data = (rtu_slave_data_t)dev->data;
    modbus_rt_mutex_lock(&(dev->mutex));
    if(MODBUS_READ == dir_slave) {
        modbus_slave_read(&(data->util), type_function, addr, quantity, ptr_data);
    } else if(MODBUS_WRITE == dir_slave) {
        modbus_slave_write(&(data->util), type_function, addr, quantity, ptr_data);
    }
    modbus_rt_mutex_unlock(&(dev->mutex));
    return MODBUS_RT_EOK;
    }
#endif   
#if MODBUS_RTU_MASTER_ENABLE
    if(MODBUS_MASTER == dev->mode) {
        rtu_master_data_t data = NULL;
        if((NULL == dev) || (NULL == dev->data) || (1 != dev->status)) {
            return -MODBUS_RT_EINVAL;
        }
        data = (rtu_master_data_t)dev->data;

        modbus_rt_mutex_lock(&(dev->mutex));
        data->ret = -MODBUS_RT_ERROR;
        data->slave_addr = dir_slave;
        data->data_addr = addr;
        data->quantity = quantity;
        data->ptr_data = ptr_data;
        data->function = type_function;
        modbus_rt_mutex_unlock(&(dev->mutex));

        modbus_rt_sem_wait(&(data->completion));
        return data->ret;
    }
#endif
    return -MODBUS_RT_ERROR;
}

#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
/**
 * @brief   modbus_rtu_set_over_type:   开启modbus rtu over type功能数
 * @param   dev:                        rtu_modbus_device_t 设备
 * @param   over_type:                  OVER_NONE：不开启；OVER_NET：开启over tcp/udp功能
 * @return  int:                        MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_set_over_type(rtu_modbus_device_t dev, modbus_serial_over_type_t over_type) {
    if((NULL == dev) || ((OVER_NONE != over_type) && (OVER_NET != over_type))) {
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    dev->over_type = over_type;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_set_net:     修改modbus rtu的网络信息
 * @param   dev:                    rtu_modbus_device_t 设备
 * @param   ipaddr:                 设备ip地址，设置为“”表示默认为netdev
 * @param   port:                   端口号
 * @param   type:                   socket类型,TCP(SOCK_STREAM)或者UDP(SOCK_DGRAM)
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_rtu_set_net(rtu_modbus_device_t dev, char * ipaddr, unsigned int port, int type) {
    if((NULL == dev) || (OVER_NET != dev->over_type) || (0 >= port) || 
    ((SOCK_STREAM  != type) && (SOCK_DGRAM != type))) {
        return -MODBUS_RT_EINVAL;
    }
     if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    if(NULL != ipaddr) {
        int len_ip = strlen(ipaddr);
        memcpy(dev->ip_buf, ipaddr,len_ip);
        dev->ip_buf[len_ip] = 0;
        dev->ipaddr = dev->ip_buf;
    } else {
        dev->ipaddr = NULL;
    }
    dev->port = port;
    dev->type = type;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_set_ip:      修改modbus rtu的IP地址
 * @param   dev:                    rtu_modbus_device_t 设备
 * @param   ipaddr:                 需要绑定的IP地址，“”表示默认为netdev
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_rtu_set_ip(rtu_modbus_device_t dev, char * ipaddr) {
    if((NULL == dev) || (OVER_NET != dev->over_type)) {
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    if(NULL != ipaddr) {
        int len_ip = strlen(ipaddr);
        memcpy(dev->ip_buf, ipaddr,len_ip);
        dev->ip_buf[len_ip] = 0;
        dev->ipaddr = dev->ip_buf;
    } else {
        dev->ipaddr = NULL;
    }
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_set_port:    修改modbus rtu的通信端口号
 * @param   dev:                    tcp_modbus_device_t设备
 * @param   port:                   端口号
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_rtu_set_port(rtu_modbus_device_t dev, unsigned int port) {
    if((NULL == dev) || (OVER_NET != dev->over_type) || (0 >= port)) {
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    dev->port = port;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_set_type:    修改modbus rtu的通信类型
 * @param   dev:                    rtu_modbus_device_t 设备
 * @param   type:                   socket类型,TCP(SOCK_STREAM)或者UDP(SOCK_DGRAM)
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_rtu_set_type(rtu_modbus_device_t dev, int type) {
    if((NULL == dev) || (OVER_NET != dev->over_type) || (NULL == dev->data) || ((SOCK_STREAM  != type) && (SOCK_DGRAM != type))) {
        return -MODBUS_RT_EEMPTY;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    dev->type = type;
    return MODBUS_RT_EOK;
}

#endif

#if MODBUS_RTU_SLAVE_ENABLE
/**
 * @brief   modbus_rtu_set_addr:    修改modbus salve的通信地址
 * @param   dev:                    rtu_modbus_device_t 设备
 * @param   addr:                   modbus slave的地址
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_rtu_set_addr(rtu_modbus_device_t dev, int addr) {
    if((NULL == dev) || (NULL == dev->data) || (MODBUS_SLAVE != dev->mode)) {
        return -MODBUS_RT_EEMPTY;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    ((rtu_slave_data_t)dev->data)->addr = addr;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_set_strict:  修改modbus slave的地址匹配功能，注意：只能再开启之前修改
 * @param   dev:                    rtu_modbus_device_t 设备
 * @param   strict:                 地址匹配标识符,0: 不匹配地址；1: 匹配地址
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_set_strict(rtu_modbus_device_t dev, uint8_t strict) {
    if((NULL == dev) || (NULL == dev->data) || (MODBUS_SLAVE != dev->mode) ||
    ((0 != strict) && (1 != strict))) { 
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }

    ((rtu_slave_data_t)dev->data)->slave_strict = strict;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_add_block:   给modbus slave添加数据
 * @param   dev:                    rtu_modbus_device_t 设备
 * @param   type:                   modbus slave 的寄存器类型，包括CIOLS(0), INPUTS(1), INPUT_REGISTERS(3), REGISTERS(4)四种
 * @param   data_addr:              modbus slave 的地址
 * @param   data:                   modbus slave 添加的数据内容指针
 * @param   nums:                   modbus slave 数据地址长度(注意是地址长度，根据寄存器类型不同，数据的内容和长度不一致)
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_add_block(rtu_modbus_device_t dev, modbus_register_type_t type, int data_addr, void *data, int nums) {
    if((NULL == dev) || (NULL == dev->data)) {
        return -MODBUS_RT_EINVAL;
    }
    return modbus_slave_add_val(&(((rtu_slave_data_t)dev->data)->util), type, data_addr, data, nums);
}

/**
 * @brief   modbus_rtu_set_pre_ans_callback:    设置slave的应答前回调函数
 * @param   dev:                                rtu_modbus_device_t 设备
 * @param   pre_ans:                            回调函数
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_set_pre_ans_callback(rtu_modbus_device_t dev, int (*pre_ans)(agile_modbus_t *, int, int,int, int)) {
    if((NULL == dev) || (NULL == dev->data) || (MODBUS_SLAVE != dev->mode)) { 
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    ((rtu_slave_data_t)dev->data)->util.pre_ans_callback = pre_ans;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_set_done_callback:       设置slave的应答前回调函数
 * @param   dev:                                rtu_modbus_device_t 设备
 * @param   done:                               回调函数
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_set_done_callback(rtu_modbus_device_t dev, int (*done)(agile_modbus_t *, int, int,int, int)) {
    if((NULL == dev) || (NULL == dev->data) || (MODBUS_SLAVE != dev->mode)) { 
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    ((rtu_slave_data_t)dev->data)->util.done_callback = done;
    return MODBUS_RT_EOK;
}
#endif

#if MODBUS_RTU_MASTER_ENABLE
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
/**
 * @brief   modbus_rtu_set_server:              modbus master修改slave端的地址和端口信息
 * @param   dev:                                rtu_modbus_device_t 设备
 * @param   saddr:                              服务端的地址
 * @param   sport:                              服务端端口号
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_set_server(rtu_modbus_device_t dev, char* saddr, unsigned int sport) {
    rtu_master_data_t data = NULL;
    if((NULL == dev) || (NULL == dev->data) || ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type)) || 
    (NULL == saddr) || (0 >= sport)) {
        return -MODBUS_RT_EINVAL;
    }

    data = (rtu_master_data_t)dev->data;
    if(SOCK_STREAM  == dev->type) {
        if(0 < dev->status) {
            return -MODBUS_RT_ISOPEN;
        }
        if(MODBUS_RT_EOK != modbus_rt_net_addr2ip(saddr, data->saddr)) {
            return -MODBUS_RT_HOST_ERROR;
        }
        data->sport = sport;
    } 
 #if MODBUS_SERIAL_OVER_UDP_ENABLE   
    else if(SOCK_DGRAM == dev->type) {
        if(MODBUS_RT_EOK != modbus_rt_net_addr2ip(saddr, data->saddr)) {
            return -MODBUS_RT_HOST_ERROR;
        }
        data->sport = sport;
    }
#endif
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_rtu_get_saddr:               modbus rtu master获取slave端设备的IP地址
 *                                              需要注意的是：每次广播都需要重新用modbus_tcp_set_server
 *                                              设置slave的地址为：255.255.255.255。支持一条excuse指令
 *                                              之后。saddr会自动变为设备IP。下次广播需要重新设置广播ip
 * @param   dev:                                rtu_modbus_device_t设备
 * @param   saddr:                              存储设备的ip地址的空间，建议长度不小于INET_ADDRSTRLEN
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *
 */
int modbus_rtu_get_saddr(rtu_modbus_device_t dev, char* saddr){
    rtu_master_data_t data = NULL;
    if((NULL == dev) || (NULL == dev->data) || ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type)) ||
    (NULL == saddr)) {
        return -MODBUS_RT_EINVAL;
    }
    data = (rtu_master_data_t)dev->data;
    int len = strlen(data->saddr);
    memcpy(saddr, data->saddr,len);
    saddr[len] = 0;
    return MODBUS_RT_EOK;
}

#endif

/**
 * @brief   modbus_rtu_excuse_ex:               modbus rtu master特殊执行函数：
 *          主要针对：(0x17)AGILE_MODBUS_FC_WRITE_AND_READ_REGISTERS命令
 *          该函数为阻塞函数，执行完成或者超时后返回(采用信号量机制，不影响其他线程运行)。
 * @param   dev:                                rtu_modbus_device_t 设备
 * @param   slave:                              modbus slave的地址
 * @param   function:                           modbus命令
 * @param   w_addr:                             需要写入数据的寄存器地址
 * @param   w_quantity:                         写入数据的寄存器的数量
 * @param   ptr_w_data:                         需要写入的内容
 * @param   r_addr:                             需要读取入数据的寄存器地址
 * @param   r_quantity:                         读取数据的寄存器的数量
 * @param   ptr_w_data:                         存放读取的内容
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_rtu_excuse_ex(rtu_modbus_device_t dev, int slave, int function,int w_addr, int w_quantity, 
                        void *ptr_w_data, int r_addr, int r_quantity, void *ptr_r_data) {
    rtu_master_data_t data = NULL;
    if((NULL == dev) || (NULL == dev->data) || (1 != dev->status)) {
        return -MODBUS_RT_EINVAL;
    }
    data = (rtu_master_data_t)dev->data;

    modbus_rt_mutex_lock(&(dev->mutex));
    data->ret = -MODBUS_RT_ERROR;
    data->slave_addr = slave;
    data->data_addr = w_addr;
    data->quantity = w_quantity;
    data->ptr_data = ptr_w_data;
    data->read_addr = r_addr;
    data->read_quantity = r_quantity;
    data->ptr_read_data = ptr_r_data;
    data->function = function;
    modbus_rt_mutex_unlock(&(dev->mutex));

    modbus_rt_sem_wait(&(data->completion));
    return data->ret;
}

#endif

#endif


