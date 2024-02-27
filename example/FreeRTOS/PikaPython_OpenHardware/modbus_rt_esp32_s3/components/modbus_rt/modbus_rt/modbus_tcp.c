/**
 * @file    modbus_tcp.c
 * @brief   基于Agile Modbus 的modbus tcp应用层实现，包含基于TCP和UDP的slave和master
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */

#include "modbus_tcp.h"

/**
 @verbatim
    针对性能有限的处理器而言，限制socket的数量，以确保内存和性能满足要求
    tcp server端可以连接多个client，所以需要一个全局变量统计总共的socket数量
 @endverbatim
 */
#if TCP_MODBUS_NUMS_ENABLE
    int tcp_modbus_nums = 0;
#endif

#if (MODBUS_TCP_SLAVE_ENABLE) || (MODBUS_TCP_MASTER_ENABLE)

#if  MODBUS_P2P_ENABLE
    #include "modbus_p2p.h"
    int modbus_slave_special_callback(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info);
    uint8_t compute_meta_length_after_function_callback(agile_modbus_t *ctx, int function, agile_modbus_msg_type_t msg_type);
    int compute_data_length_after_meta_callback(agile_modbus_t *ctx, uint8_t *msg,int msg_length, agile_modbus_msg_type_t msg_type);
#endif

/**
 @verbatim
    如果开启了modbus slave的支持，则需要添加一些函数声明，函数实现在modbus_slave.c中
    因为这些函数无需对外，所以，不需要开放给用户使用
 @endverbatim
 */
#if MODBUS_TCP_SLAVE_ENABLE
    void modbus_slave_util_init(agile_modbus_slave_util_t *util);
    int modbus_slave_add_val(agile_modbus_slave_util_t *util, modbus_register_type_t type,
                            int data_addr, void *data, int nums);
    int modbus_slave_clear_val(agile_modbus_slave_util_t *util);
    int modbus_slave_read(agile_modbus_slave_util_t *util, modbus_register_type_t type, int addr, int quantity, void *ptr_data);
    int modbus_slave_write(agile_modbus_slave_util_t *util, modbus_register_type_t type, int addr, int quantity, void *ptr_data);
#if SLAVE_DATA_DEVICE_BINDING
    int modbus_slave_util_dev_binding(agile_modbus_slave_util_t *util, int flag);
#endif
#endif

/**
 * @brief   modbus_tcp_dev_init:             modbus tcp初始化数据函数，把modbus tcp的数据赋值到全局指针当中
 * @param   pos_dev:                        指向tcp_modbus_device_t的指针，
 * @param   mode                            模式，为SLAVE/MASTER两种情况
 * @param   data                            设备数据指针
 * @return  int:                            MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_tcp_dev_init(tcp_modbus_device_t * pos_dev, modbus_mode_type mode, void *data) {
    int ret = 0;
    if(NULL != (*pos_dev)) {
        return -MODBUS_RT_EINVAL;
    }

    *pos_dev = modbus_rt_malloc(sizeof(struct tcp_modbus_device));
    if(NULL == (*pos_dev)) {
        return -MODBUS_RT_ENOMEM;
    }
    tcp_modbus_device_t dev = *pos_dev;

    memset(dev,0,sizeof(struct tcp_modbus_device));
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

#if MODBUS_TCP_SLAVE_ENABLE
/**
 * @brief   modbus_tcp_slave_data_create:   tcp_slave_data_t数据创建函数
 * @param   None
 * @return  tcp_slave_data_t:               tcp_slave_data_t数据指针
 *       
 */
static tcp_slave_data_t  modbus_tcp_slave_data_create(void) {
    tcp_slave_data_t data = modbus_rt_calloc(1, sizeof(struct tcp_slave_data));
    if(NULL == data) {
        return NULL;
    }
    memset(data,0,sizeof(struct tcp_slave_data));
    return data;
}

/**
 * @brief   tcp_slave_close_client_sock:    关闭连接到modbus Slave上的设备socket
 * @param   p_sock:                         指向client socket数组的指针
 * @return  None
 *       
 */
static void tcp_slave_close_client_sock( int *p_sock ) {
    for(int i = 0; i < SOCKET_CONNECT_NUMS ; i++) {
      if(-1 != p_sock[i]) {
          modbus_rt_net_close(p_sock[i]);
          p_sock[i] = -1;
#if TCP_MODBUS_NUMS_ENABLE         
          tcp_modbus_nums--;
#endif 
      }
    }
}

/**
 * @brief   modbus_tcp_slave_entry:         modbus tcp slave的入口函数
 * @param   parameter:                      入口参数，tcp_modbus_device_t设备
 * @return  无
 *       
 */
static void modbus_tcp_slave_entry(void *parameter) {
    tcp_modbus_device_t dev = (tcp_modbus_device_t)parameter; 
    tcp_slave_data_t    data = (tcp_slave_data_t)dev->data;
    int send_len = dev->send_len;
    int read_len = dev->read_len;      
    agile_modbus_t *ctx = dev->ctx;

    agile_modbus_tcp_init(&(dev->ctx_tcp), dev->ctx_send_buf, send_len, dev->ctx_read_buf, read_len);
    agile_modbus_set_slave(ctx, data->addr);
#if MODBUS_P2P_ENABLE
    if(0 != dev->p2p_flag) {
        agile_modbus_set_compute_meta_length_after_function_cb(dev->ctx, compute_meta_length_after_function_callback);
        agile_modbus_set_compute_data_length_after_meta_cb(dev->ctx, compute_data_length_after_meta_callback);
    }
#endif

    /* 启动线程运行标志 */
    modbus_rt_mutex_lock(&(dev->mutex));
    dev->thread_flag = 1;           //线程运行
    modbus_rt_mutex_unlock(&(dev->mutex));

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
                tcp_slave_close_client_sock(sock_client);
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
                tcp_slave_close_client_sock(sock_client);
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
#if MODBUS_TCP_SLAVE_ENABLE
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
                    send_len = agile_modbus_slave_handle(ctx, read_len, data->slave_strict, agile_modbus_slave_util_callback, &data->util, NULL);
                    if (send_len > 0) {
                        ret = send(sock_client[i], ctx->send_buf, send_len, 0);
                        if(ret <= 0) {
                            continue;       //发送失败，有可能时client端断开了，在下一次循环的select中会关闭该socket
                        }
                    }
                }
            }
        }
#endif
#if MODBUS_TCP_SLAVE_FOR_UDP_ENABLE
        if(SOCK_DGRAM == dev->type) {
            //处理udp的数据
            read_len = recvfrom(dev->sock, ctx->read_buf, ctx->read_bufsz, 0, (struct sockaddr*)&client_addr, &client_addr_len);
            if (read_len <= 0) {
                continue;
            }
            send_len = agile_modbus_slave_handle(ctx, read_len, data->slave_strict, agile_modbus_slave_util_callback, &data->util, NULL);
            if (send_len > 0) {   
#if MODBUS_UDP_FOR_SEARCH       //跨网段设备广播检测
                if(modbus_rt_net_segment(dev->ipaddr, client_addr.sin_addr.s_addr) == 0) {
                    client_addr.sin_addr.s_addr = 0xffffffff;
                }
#endif
                ret = sendto(dev->sock, ctx->send_buf, send_len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
                if(ret <= 0) {
                    continue;
                }
            }
        }
#endif
    }

}
#endif

#if MODBUS_TCP_MASTER_ENABLE
/**
 * @brief   modbus_tcp_master_data_create:  tcp_master_data_t数据创建函数
 * @param   None
 * @return  tcp_master_data_t:              tcp_master_data_t数据指针
 *       
 */
static tcp_master_data_t  modbus_tcp_master_data_create(void) {
    tcp_master_data_t data = modbus_rt_calloc(1, sizeof(struct tcp_master_data));
    if(NULL == data) {
        return NULL;
    }
    memset(data,0,sizeof(struct tcp_master_data));
    return data;
}

/**
 * @brief   modbus_tcp_master_txrx:         tcp_master数据发送接收
 * @param   dev:                            tcp_modbus_device_t设备
 * @param   ctx:                            modbus句柄
 * @param   send_len:                       需要发送数据的长度
 * @return  int:                            MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_tcp_master_txrx(tcp_modbus_device_t dev, agile_modbus_t *ctx, int send_len) {
    int read_len = 0;
    fd_set sock_read_set;                                 //存储socket的索引，用于select参数使用
    int maxfd, nready;                                     //零时存储数据
    struct timeval timeout= {0};                    //用于设置select的超时值
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
        send(dev->sock, ctx->send_buf, send_len, 0);
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
    }
#if MODBUS_TCP_MASTER_FOR_UDP_ENABLE
    else if(dev->type == SOCK_DGRAM) {
        struct sockaddr_in server_addr = {0};                 //存储连接udp server 的数据信息
        socklen_t server_addr_len = sizeof(server_addr);        //存储链接到改server上的client的IP地址信息

        tcp_master_data_t    data = (tcp_master_data_t)dev->data;         //tcp_master数据
        /* 设置服务端地址 */
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(data->sport);
        server_addr.sin_addr.s_addr = inet_addr(data->saddr);
        memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

        sendto(dev->sock, ctx->send_buf, send_len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
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
    }
#endif
    return read_len;
}

/**
 * @brief   modbus_tcp_master_excuse_run:   tcp_master执行执行函数
 * @param   dev:                            tcp_modbus_device_t设备
 * @return  int:                            MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_tcp_master_excuse_run(tcp_modbus_device_t dev) {
    int send_len = 0;               //打包需要发送数据的长度
    int read_len = 0;               //串口接收到的数据的长度
    agile_modbus_t *ctx = dev->ctx;
    tcp_master_data_t data = (tcp_master_data_t)dev->data;
    
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
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
#if MODBUS_P2P_MASTER_ENABLE
#if MODBUS_P2P_SEND_ENABLE
        case AGILE_MODBUS_FC_TRANS_FILE: {
            modbus_p2p_master_info_t *p2p_info = &g_modbus_p2p_master_info;
            send_len = agile_modbus_serialize_raw_request(ctx, p2p_info->raw_req, p2p_info->raw_req_len);
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
            if (read_len < 0) {
                 data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_raw_response(ctx, read_len);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            int flag = ctx->read_buf[ctx->backend->header_length + 3];
            if(0x01 != flag) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            data->ret = MODBUS_RT_EOK;
        } break;
#endif
#if MODBUS_P2P_RECV_ENABLE
        case AGILE_MODBUS_FC_READ_FILE: {
            modbus_p2p_master_info_t *p2p_info = &g_modbus_p2p_master_info;
            send_len = agile_modbus_serialize_raw_request(ctx, p2p_info->raw_req, p2p_info->raw_req_len);
            read_len = modbus_tcp_master_txrx(dev,ctx,send_len);
            if (read_len < 0) {
                 data->ret = read_len;
                break;
            }
            int rc = agile_modbus_deserialize_raw_response(ctx, read_len);
            if (rc < 0) {
                data->ret = -MODBUS_RT_ERROR;
                break;
            }
            //把数据返回到上传程序线程操作
            int cmd_data_len = (ctx->read_buf[ctx->backend->header_length + 3] << 8) + ctx->read_buf[ctx->backend->header_length + 4];
            memcpy(&(p2p_info->raw_req[2]), &ctx->read_buf[ctx->backend->header_length + 1], cmd_data_len + 4);
            data->ret = MODBUS_RT_EOK;
        } break;
#endif
#endif
        default:
        {
            data->ret =-MODBUS_RT_ERROR;
        } break;
    }
    data->function = 0;
    return data->ret;
}

/**
 * @brief   modbus_tcp_master_entry:        modbus tcp master的入口函数
 * @param   parameter:                      入口参数，tcp_modbus_device_t设备
 * @return  无
 *       
 */
static void modbus_tcp_master_entry(void *parameter) {
    static int sock_timeout = 0;
    tcp_modbus_device_t dev = (tcp_modbus_device_t)parameter; 
    tcp_master_data_t    data = (tcp_master_data_t)dev->data;
    int send_len = dev->send_len;
    int read_len = dev->read_len;      
    agile_modbus_tcp_init(&(dev->ctx_tcp), dev->ctx_send_buf, send_len, dev->ctx_read_buf, read_len);
#if MODBUS_P2P_ENABLE
    if(0 != dev->p2p_flag) {
        agile_modbus_set_compute_meta_length_after_function_cb(dev->ctx, compute_meta_length_after_function_callback);
        agile_modbus_set_compute_data_length_after_meta_cb(dev->ctx, compute_data_length_after_meta_callback);
    }
#endif
    /* 启动线程运行标志 */
    modbus_rt_mutex_lock(&(dev->mutex));
    dev->thread_flag = 1;           //线程运行
    modbus_rt_mutex_unlock(&(dev->mutex));

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
            //连接成功之后，执行tcp master指令
            modbus_tcp_master_excuse_run(dev);
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
            if (0 >= read_len) {
                //接收异常，可能是服务单断开连接
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
 * @brief   modbus_tcp:             创造modbus tcp设备
 * @param   mode:                   modbus类型： MODBUS_SLAVE/MODBUS_MASTER
 * @return  tcp_modbus_device_t:    tcp_modbus_device_t数据指针
 *       
 */
tcp_modbus_device_t modbus_tcp(modbus_mode_type mode) {
    int ret = 0;
    tcp_modbus_device_t dev = NULL;
    if((MODBUS_SLAVE != mode) && (MODBUS_MASTER != mode)) {
        return NULL;
    }
#if MODBUS_TCP_SLAVE_ENABLE
#if !MODBUS_TCP_MASTER_ENABLE
    if(MODBUS_MASTER == mode) {
        return NULL;
    }
#endif
    if(MODBUS_SLAVE == mode) {
        tcp_slave_data_t data = modbus_tcp_slave_data_create();
        if(NULL == data) {
            return NULL;
        }
        data->addr = 1;             ///默认地址为1
        data->slave_strict = 1;     //默认开启地址匹配功能

        ret = modbus_tcp_dev_init(&dev, mode, data);
        if(MODBUS_RT_EOK != ret) {
            modbus_rt_free(data);
            return NULL;
        }
        dev->port = 502;                //默认的端口号为502
        dev->type = SOCK_STREAM;        //默认为TCP模式
        modbus_slave_util_init(&data->util);
    }
#endif   
#if MODBUS_TCP_MASTER_ENABLE
#if !MODBUS_TCP_SLAVE_ENABLE
    if(MODBUS_SLAVE == mode) {
        return NULL;
    }
#endif
    if(MODBUS_MASTER == mode) {
        tcp_master_data_t data = modbus_tcp_master_data_create();
        if(NULL == data) {
            return NULL;
        }
        data->sport = 502;          //服务器端默认端口503 
        ret = modbus_tcp_dev_init(&dev, mode, data);
        if(MODBUS_RT_EOK != ret) {
            modbus_rt_free(data);
            return NULL;
        }
        dev->port = 0;                  //Master默认的端口号为随机
        dev->type = SOCK_STREAM;        //默认为TCP模式
        //初始化master执行指令的完成信号量
        ret = modbus_rt_sem_init(&(data->completion));
        if(MODBUS_RT_EOK != ret) {
            modbus_rt_free(data);
            return NULL;
        }
    }
#endif
    dev->ctx = &(dev->ctx_tcp._ctx);
    return dev;
}

/**
 * @brief   modbus_tcp_set_net:     修改modbus tcp的网络信息
 * @param   dev:                    tcp_modbus_device_t设备
 * @param   ipaddr:                 设备ip地址，设置为“”表示默认为netdev
 * @param   port:                   端口号
 * @param   type:                   socket类型,TCP(SOCK_STREAM)或者UDP(SOCK_DGRAM)
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int  modbus_tcp_set_net(tcp_modbus_device_t dev, char * ipaddr, unsigned int port, int type) {
    if((NULL == dev) || ((SOCK_STREAM  != type) && (SOCK_DGRAM != type))) {
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
 * @brief   modbus_tcp_set_ip:      修改modbus tcp的IP地址
 * @param   dev:                    tcp_modbus_device_t设备
 * @param   ipaddr:                 需要绑定的IP地址，“”表示默认为netdev
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_tcp_set_ip(tcp_modbus_device_t dev, char * ipaddr) {
    if((NULL == dev)) {
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
 * @brief   modbus_tcp_set_port:    修改modbus tcp的通信端口号
 * @param   dev:                    tcp_modbus_device_t设备
 * @param   port:                   端口号
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_tcp_set_port(tcp_modbus_device_t dev, unsigned int port) {
    if((NULL == dev)) {
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    dev->port = port;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_tcp_set_type:    修改modbus tcp的通信类型
 * @param   dev:                    tcp_modbus_device_t设备
 * @param   type:                   socket类型,TCP(SOCK_STREAM)或者UDP(SOCK_DGRAM)
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_tcp_set_type(tcp_modbus_device_t dev, int type) {
#if !((MODBUS_TCP_SLAVE_FOR_UDP_ENABLE) || (MODBUS_TCP_MASTER_FOR_UDP_ENABLE))
    if(SOCK_DGRAM == type) {
        return -MODBUS_RT_EINVAL;
    }
#endif
    if((NULL == dev) || (NULL == dev->data) || ((SOCK_STREAM  != type) && (SOCK_DGRAM != type))) {
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    dev->type = type;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_tcp_open:    开启modbus tcp启动通信相关的线程 
 * @param   dev:                tcp_modbus_device_t设备
 * @return  int:                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_tcp_open(tcp_modbus_device_t dev) {
    int sock = -1;
    char str_name[MODBUS_RT_NAME_MAX] = {0};
    if((NULL == dev) || (NULL == dev->data) || ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type))) {
         return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
#if MODBUS_TCP_SLAVE_ENABLE
#if !MODBUS_TCP_MASTER_ENABLE
    if(MODBUS_MASTER == dev->mode) {
         return -MODBUS_RT_EINVAL;
    }
#endif
#if  MODBUS_P2P_ENABLE
    if(dev->p2p_flag) {

        ((tcp_slave_data_t)(dev->data))->util.special_function = modbus_slave_special_callback;
    }
#endif
#endif   
#if MODBUS_TCP_MASTER_ENABLE
#if !MODBUS_TCP_SLAVE_ENABLE
    if(MODBUS_SLAVE == dev->mode) {
         return -MODBUS_RT_EINVAL;
    }
#endif
    tcp_master_data_t data_master = NULL;
    (void)(data_master);
    if(MODBUS_MASTER == dev->mode) {
        data_master = (tcp_master_data_t)dev->data;
        int64_t net_addr_temp = (int64_t)inet_addr(data_master->saddr);
        if(net_addr_temp < 0) {
            return -MODBUS_RT_EINVAL;
        }
    }
#endif

#if TCP_MODBUS_NUMS_ENABLE
    if(tcp_modbus_nums >= TCP_MODBUS_NUMS) {
        return -MODBUS_RT_EINVAL;
    }
#endif
    if(SOCK_STREAM == dev->type) {
#if MODBUS_TCP_SLAVE_ENABLE
        if(MODBUS_SLAVE == dev->mode) {
            sock = modbus_rt_tcp_server_init(dev->ipaddr, dev->port, SOCKET_CONNECT_NUMS);
        }
#endif
#if MODBUS_TCP_MASTER_ENABLE
        if(MODBUS_MASTER == dev->mode) {
            sock = modbus_rt_tcp_client_init(dev->ipaddr, dev->port, data_master->saddr, data_master->sport);
        }
#endif
    } else if(SOCK_DGRAM == dev->type) {
#if MODBUS_TCP_SLAVE_FOR_UDP_ENABLE 
        sock = modbus_rt_udp_socket_init(dev->ipaddr, dev->port);
#else
    return -MODBUS_RT_EINVAL;
#endif
    }
    if (0 > sock) {
        return sock;
    }
    dev->sock = sock;
#if MODBUS_P2P_ENABLE
    if(0 == dev->p2p_flag) {
        dev->send_len = AGILE_MODBUS_MAX_ADU_LENGTH;
        dev->read_len = AGILE_MODBUS_MAX_ADU_LENGTH;
    } else {
        dev->send_len = P2P_SLAVE_BUF_MAX_LEN;
        dev->read_len = P2P_SLAVE_BUF_MAX_LEN;
    }
#else
    dev->send_len = AGILE_MODBUS_MAX_ADU_LENGTH;
    dev->read_len = AGILE_MODBUS_MAX_ADU_LENGTH;
#endif
    dev->ctx_send_buf = modbus_rt_calloc(1, dev->send_len);
    if(NULL == dev->ctx_send_buf) {
        modbus_rt_net_close(dev->sock);                      /* 关闭socket */  
        dev->sock= 0;
        return -MODBUS_RT_ENOMEM;
    }
    dev->ctx_read_buf = modbus_rt_calloc(1, dev->read_len);
    if(NULL == dev->ctx_read_buf) {
        modbus_rt_free(dev->ctx_send_buf);
        modbus_rt_net_close(dev->sock);                      /* 关闭socket */
        dev->sock = 0; 
        return -MODBUS_RT_ENOMEM;
    }
    //创建线程
#if MODBUS_TCP_SLAVE_ENABLE
    if(MODBUS_SLAVE == dev->mode) {
        sprintf(str_name,"tcp_ts%d",sock);
        dev->thread = modbus_rt_thread_init(str_name, modbus_tcp_slave_entry, 
                dev, MODBUS_THREAD_STACK_SIZE, MODBUS_THREAD_PRIO, 10);
    }
#endif
#if MODBUS_TCP_MASTER_ENABLE
    if(MODBUS_MASTER == dev->mode) {
        sprintf(str_name,"tcp_tm%d",sock);
        dev->thread = modbus_rt_thread_init(str_name, modbus_tcp_master_entry,
                dev, MODBUS_THREAD_STACK_SIZE, MODBUS_THREAD_PRIO, 10);
    }
#endif
    if(NULL != dev->thread) {
        modbus_rt_thread_startup(dev->thread);
    } else {
        modbus_rt_free(dev->ctx_send_buf);
        modbus_rt_free(dev->ctx_read_buf);
        modbus_rt_net_close(sock);
        sock = 0; 
        return -MODBUS_RT_EINVAL;
    }
#if TCP_MODBUS_NUMS_ENABLE
    tcp_modbus_nums++;
#endif
    dev->status = 1;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_tcp_isopen:  modbus tcp线程是否开启
 * @param   dev :               tcp_modbus_device_t设备
 * @return  int:                1: 线程开启；0：线程未开启
 *       
 */
int modbus_tcp_isopen(tcp_modbus_device_t dev) {
    if ((NULL == dev) || (0 >= dev->sock)) { 
        return 0;
    }
    return dev->status;
}

/**
 * @brief   modbus_tcp_close:   关闭modbus tcp启动通信相关的线程 
 * @param   dev:                tcp_modbus_device_t设备
 * @return  int:                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_tcp_close(tcp_modbus_device_t dev) {
    if(!modbus_tcp_isopen(dev))
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
    if(0 < dev->sock) {
        modbus_rt_net_close(dev->sock);                      /* 关闭socket */ 
    }
#if TCP_MODBUS_NUMS_ENABLE         
    tcp_modbus_nums--;
#endif 
    dev->status = 0;
    modbus_rt_thread_destroy(dev->thread);
    dev->sock = 0; 
    
    dev->thread = NULL;
    return MODBUS_RT_EOK;
}


/**
 * @brief   modbus_tcp_destroy: modbus tcp销毁函数
 * @param   pos_dev:            指向tcp_modbus_device_t的指针，
 *                              这里主要目的需要通过指针删除相应的动态分配数据内容
 * @return  int:                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_tcp_destroy(tcp_modbus_device_t * pos_dev) {
    if((NULL == pos_dev) || (NULL == (*pos_dev))) {
        return -MODBUS_RT_EINVAL;
    }
    tcp_modbus_device_t dev = *pos_dev; 
    if (NULL == dev) { 
        return -MODBUS_RT_EINVAL;
    }  
    if(dev->thread != 0) {
        modbus_tcp_close(dev);
    }
#if MODBUS_TCP_SLAVE_ENABLE
    if(MODBUS_SLAVE == dev->mode) {
        tcp_slave_data_t data = (tcp_slave_data_t)dev->data; 
        modbus_slave_clear_val(&(data->util));
        modbus_rt_free(data);
    }
#endif   
#if MODBUS_TCP_MASTER_ENABLE
    if(MODBUS_MASTER == dev->mode) {
        tcp_master_data_t data = (tcp_master_data_t)dev->data;  
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
 * @brief   modbus_tcp_excuse:  modbus tcp执行函数：读取或者写入寄存器的值
 *          该函数为阻塞函数，执行完成或者超时后返回(采用信号量机制，不影响其他线程运行)。
 *          注意：不存在的寄存器读取和写入不会报错，但是永远为0。
 * 
 * @param   dev:                tcp_modbus_device_t设备
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
int modbus_tcp_excuse(tcp_modbus_device_t dev, int dir_slave, int type_function, int addr, int quantity, void *ptr_data) {
#if MODBUS_TCP_SLAVE_ENABLE
    if(MODBUS_SLAVE == dev->mode) {
        tcp_slave_data_t data = NULL;
        if((NULL == dev) || (NULL == dev->data) || ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type)) ||
        ((MODBUS_READ  != dir_slave) && (MODBUS_WRITE != dir_slave)) || (1 != dev->status)) {
            return -MODBUS_RT_EINVAL;
        }
        data = (tcp_slave_data_t)dev->data;
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
#if MODBUS_TCP_MASTER_ENABLE
    if(MODBUS_MASTER == dev->mode) {
        tcp_master_data_t data = NULL;
        if((NULL == dev) || (NULL == dev->data) || ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type)) ||
        (1 != dev->status)) {
            return -MODBUS_RT_EINVAL;
        }
        data = (tcp_master_data_t)dev->data;

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

#if  MODBUS_P2P_ENABLE
/**
 * @brief   modbus_tcp_set_p2p_flag:    开启modbus p2p功能数
 * @param   dev:                        tcp_modbus_device_t设备
 * @param   flag:                       0：关闭p2p模式；1：开启p2p模式
 * @return  int:                        MODBUS_RT_EOK：成功，其他：失败
 *
 */
int modbus_tcp_set_p2p_flag(tcp_modbus_device_t dev, int flag) {
    if ((NULL == dev)) {
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    dev->p2p_flag = flag;
    return MODBUS_RT_EOK;
}
#endif

#if MODBUS_TCP_SLAVE_ENABLE
/**
 * @brief   modbus_tcp_set_addr:    修改modbus salve的通信地址
 * @param   dev:                    tcp_modbus_device_t设备
 * @param   addr:                   modbus slave的地址
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 */
int modbus_tcp_set_addr(tcp_modbus_device_t dev, int addr) {
    if((NULL == dev) || (NULL == dev->data) || (MODBUS_SLAVE != dev->mode)) {
        return -MODBUS_RT_EEMPTY;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    ((tcp_slave_data_t)dev->data)->addr = addr;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_tcp_set_strict:  修改modbus slave的地址匹配功能，注意：只能再开启之前修改
 * @param   dev:                    tcp_modbus_device_t设备
 * @param   strict:                 地址匹配标识符,0: 不匹配地址；1: 匹配地址
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_tcp_set_strict(tcp_modbus_device_t dev, uint8_t strict) {
    if((NULL == dev) || (NULL == dev->data) || (MODBUS_SLAVE != dev->mode) ||
    ((SOCK_STREAM != dev->type) && (SOCK_DGRAM != dev->type)) || 
    ((0 != strict) && (1 != strict))) {
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }

    ((tcp_slave_data_t)dev->data)->slave_strict = strict;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_tcp_add_block:   给modbus slave添加数据
 * @param   dev:                    tcp_modbus_device_t设备
 * @param   type:                   modbus slave 的寄存器类型，包括CIOLS(0), INPUTS(1), INPUT_REGISTERS(3), REGISTERS(4)四种
 * @param   data_addr:              modbus slave 的地址
 * @param   data:                   modbus slave 添加的数据内容指针
 * @param   nums:                   modbus slave 数据地址长度(注意是地址长度，根据寄存器类型不同，数据的内容和长度不一致)
 * @return  int:                    MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_tcp_add_block(tcp_modbus_device_t dev, modbus_register_type_t type, int data_addr, void *data, int nums) {
    if((NULL == dev) || (NULL == dev->data)) {
        return -MODBUS_RT_EINVAL;
    }
    return modbus_slave_add_val(&(((tcp_slave_data_t)dev->data)->util), type, data_addr, data, nums);
}

/**
 * @brief   modbus_tcp_set_pre_ans_callback:    设置slave的应答前回调函数
 * @param   dev:                                tcp_modbus_device_t设备
 * @param   pre_ans:                            回调函数
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_tcp_set_pre_ans_callback(tcp_modbus_device_t dev, int (*pre_ans)(agile_modbus_t *, int, int,int, int)) {
    if((NULL == dev) || (NULL == dev->data) || (0 >= dev->port) || (MODBUS_SLAVE != dev->mode) ||
    ((SOCK_STREAM != dev->type) && (SOCK_DGRAM != dev->type))) { 
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    ((tcp_slave_data_t)dev->data)->util.pre_ans_callback = pre_ans;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_tcp_set_done_callback:       设置slave的应答前回调函数
 * @param   dev:                                tcp_modbus_device_t设备
 * @param   done:                               回调函数
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_tcp_set_done_callback(tcp_modbus_device_t dev, int (*done)(agile_modbus_t *, int, int,int, int)) {
    if((NULL == dev) || (NULL == dev->data) || (0 >= dev->port) || (MODBUS_SLAVE != dev->mode) ||
    ((SOCK_STREAM != dev->type) && (SOCK_DGRAM != dev->type))) { 
        return -MODBUS_RT_EINVAL;
    }
    if(0 < dev->status) {
        return -MODBUS_RT_ISOPEN;
    }
    ((tcp_slave_data_t)dev->data)->util.done_callback = done;
    return MODBUS_RT_EOK;
}


#if SLAVE_DATA_DEVICE_BINDING
/**
 * @brief   modbus_tcp_set_dev_binding: 开启slave的设备绑定功能
 * @param   dev:                        tcp_modbus_device_t设备
 * @param   flag:                       0：关闭p2p模式；1：开启p2p模式
 * @return  int:                        MODBUS_RT_EOK：成功，其他：失败
 *
 */
int modbus_tcp_set_dev_binding(tcp_modbus_device_t dev, int flag) {
    if((NULL == dev) || (NULL == dev->data) || (MODBUS_SLAVE != dev->mode)) {
        return -MODBUS_RT_EINVAL;
    }
    return modbus_slave_util_dev_binding(&(((tcp_slave_data_t)dev->data)->util), flag);
}
#endif

#endif

#if MODBUS_TCP_MASTER_ENABLE
/**
 * @brief   modbus_tcp_set_server:              modbus tcp master修改slave端的地址和端口信息
 *                                              如果是广播，每次广播之前都需要调用一次。调用完成之后
 *                                              ip会自动修改为设备的IP，这会导致下次的指令为非广播指令
 *                                              所以每次广播前都需要重新修改设备的ip。
 * @param   dev:                                tcp_modbus_device_t设备
 * @param   saddr:                              服务端的地址
 * @param   sport:                              服务端端口号
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int modbus_tcp_set_server(tcp_modbus_device_t dev, char* saddr, unsigned int sport) {
    tcp_master_data_t data = NULL;
    if((NULL == dev) || (NULL == dev->data) || ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type)) || 
    (NULL == saddr) || (0 >= sport)) {
        return -MODBUS_RT_EINVAL;
    }

    data = (tcp_master_data_t)dev->data;
    if(SOCK_STREAM  == dev->type) {     
        if(0 < dev->status) {           //tcp只允许open之前设置
            return -MODBUS_RT_ISOPEN;
        }
        if(MODBUS_RT_EOK != modbus_rt_net_addr2ip(saddr, data->saddr)) {
            return -MODBUS_RT_HOST_ERROR;
        }
        data->sport = sport;
    } 
 #if MODBUS_TCP_MASTER_FOR_UDP_ENABLE   
    else if(SOCK_DGRAM == dev->type) {      //udp可以中途修改
        if(MODBUS_RT_EOK != modbus_rt_net_addr2ip(saddr, data->saddr)) {
            return -MODBUS_RT_HOST_ERROR;
        }
        data->sport = sport;
    }
#endif
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_tcp_get_saddr:               modbus tcp master获取slave端设备的IP地址
 *                                              需要注意的是：每次广播都需要重新用modbus_tcp_set_server
 *                                              设置slave的地址为：255.255.255.255。支持一条excuse指令
 *                                              之后。saddr会自动变为设备IP。下次广播需要重新设置广播ip
 * @param   dev:                                tcp_modbus_device_t设备
 * @param   saddr:                              存储设备的ip地址的空间，建议长度不小于INET_ADDRSTRLEN
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *
 */
int modbus_tcp_get_saddr(tcp_modbus_device_t dev, char* saddr){
    tcp_master_data_t data = NULL;
    if((NULL == dev) || (NULL == dev->data) || ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type)) ||
    (NULL == saddr)) {
        return -MODBUS_RT_EINVAL;
    }
    data = (tcp_master_data_t)dev->data;
    int len = strlen(data->saddr);
    memcpy(saddr, data->saddr,len);
    saddr[len] = 0;
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_tcp_excuse_ex:               modbus tcp master特殊执行函数：
 *          主要针对：(0x17)AGILE_MODBUS_FC_WRITE_AND_READ_REGISTERS命令
 *          该函数为阻塞函数，执行完成或者超时后返回(采用信号量机制，不影响其他线程运行)。
 * @param   dev:                                tcp_modbus_device_t设备
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
int modbus_tcp_excuse_ex(tcp_modbus_device_t dev, int slave, int function,int w_addr, int w_quantity, 
                        void *ptr_w_data, int r_addr, int r_quantity, void *ptr_r_data) {
    tcp_master_data_t data = NULL;
    if((NULL == dev) || (NULL == dev->data) || ((SOCK_STREAM  != dev->type) && (SOCK_DGRAM != dev->type)) ||
    (1 != dev->status) || (AGILE_MODBUS_FC_WRITE_AND_READ_REGISTERS != function)) {
        return -MODBUS_RT_EINVAL;
    }
    data = (tcp_master_data_t)dev->data;

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

#if  ((MODBUS_P2P_ENABLE) && (MODBUS_P2P_MASTER_ENABLE))
/**
 * @brief   modbus_tcp_master_file_entry:  tcp master 文件读写线程函数
 * @param   parameter: 入口参数，tcp_modbus_device_t设备
 * @return  无
 *
 */
static void modbus_tcp_master_file_entry(void *parameter) {
    tcp_modbus_device_t dev = (tcp_modbus_device_t)parameter;
    tcp_master_data_t    data = (tcp_master_data_t)dev->data;
    modbus_p2p_master_info_t *p2p_info = &g_modbus_p2p_master_info;
    uint8_t *raw_req = p2p_info->raw_req;
    int raw_req_len = p2p_info->raw_req_len;
    int info_len = sizeof(modbus_rt_file_info_t);
    int step = 0;
    while(1) {
        raw_req_len = 2;
#if MODBUS_P2P_SEND_ENABLE
#if (!MODBUS_P2P_RECV_ENABLE)
        if(MODBUS_READ == p2p_info->dir) {
            data->ret = -MODBUS_RT_EINVAL;
            modbus_rt_sem_post(&(p2p_info->sem));
            return ;
        }
#endif
        if(MODBUS_WRITE == p2p_info->dir) {
            switch(step) {
                case 0: {
                    modbus_rt_mutex_lock(&(dev->mutex));
                    data->ret = -MODBUS_RT_ERROR;
                    data->slave_addr = raw_req[0];

                    raw_req[raw_req_len++] = ((uint16_t)TRANS_FILE_CMD_START >> 8);
                    raw_req[raw_req_len++] = ((uint16_t)TRANS_FILE_CMD_START & 0xFF);
                    int nb = sizeof(modbus_rt_file_info_t);
                    raw_req[raw_req_len++] = nb >> 8;
                    raw_req[raw_req_len++] = nb & 0xFF;
                    memcpy(raw_req + raw_req_len, &(p2p_info->file_info), info_len);
                    raw_req_len += info_len;
                    p2p_info->raw_req_len = raw_req_len;
                    data->function = raw_req[1];
                    modbus_rt_mutex_unlock(&(dev->mutex));
                    //等待执行完毕
                    modbus_rt_sem_wait(&(data->completion));
                    if(MODBUS_RT_EOK != data->ret)
                    {
                        modbus_rt_sem_post(&(p2p_info->sem));
                        return ;
                    }
                    step = 1;
                } break;
                case 1: {
                    modbus_rt_mutex_lock(&(dev->mutex));
                    data->ret = -MODBUS_RT_ERROR;
                    data->slave_addr = raw_req[0];

                    raw_req[raw_req_len++] = ((uint16_t)TRANS_FILE_CMD_DATA >> 8);
                    raw_req[raw_req_len++] = ((uint16_t)TRANS_FILE_CMD_DATA & 0xFF);
                    int nb_pos = raw_req_len;
                    raw_req_len += 3;
                    int recv_bytes = modbus_rt_file_read_file(p2p_info->fd, raw_req + raw_req_len, P2P_SLAVE_BUF_LEN);
                    raw_req_len += recv_bytes;
                    p2p_info->write_file_size += recv_bytes;
                    int nb = recv_bytes + 1;
                    raw_req[nb_pos] = nb >> 8;
                    raw_req[nb_pos + 1] = nb & 0xFF;
                    if (recv_bytes < P2P_SLAVE_BUF_LEN) {
                        raw_req[nb_pos + 2] = TRANS_FILE_FLAG_END;
                        step = 2;
                    } else {
                        raw_req[nb_pos + 2] = TRANS_FILE_FLAG_NOT_END;
                    }
                    p2p_info->raw_req_len = raw_req_len;
                    data->function = raw_req[1];
                    modbus_rt_mutex_unlock(&(dev->mutex));
                    //等待执行完毕
                    modbus_rt_sem_wait(&(data->completion));
                    if(MODBUS_RT_EOK != data->ret)
                    {
                        modbus_rt_sem_post(&(p2p_info->sem));
                        return ;
                    }
                } break;
                default: {
                    modbus_rt_sem_post(&(p2p_info->sem));
                    return ;
                } break;
            }
        }
#endif

#if MODBUS_P2P_RECV_ENABLE
#if (!MODBUS_P2P_SEND_ENABLE)
        if(MODBUS_WRITE == p2p_info->dir) {
            data->ret = -MODBUS_RT_EINVAL;
            modbus_rt_sem_post(&(p2p_info->sem));
            return ;
        }
#endif
        if(MODBUS_READ == p2p_info->dir) {
            switch(step) {
                case 0: {
                    modbus_rt_mutex_lock(&(dev->mutex));
                    data->ret = -MODBUS_RT_ERROR;
                    data->slave_addr = raw_req[0];

                    raw_req[raw_req_len++] = ((uint16_t)TRANS_FILE_CMD_START >> 8);
                    raw_req[raw_req_len++] = ((uint16_t)TRANS_FILE_CMD_START & 0xFF);
                    int nb = sizeof(modbus_rt_file_info_t);
                    raw_req[raw_req_len++] = nb >> 8;
                    raw_req[raw_req_len++] = nb & 0xFF;
                    memcpy(raw_req + raw_req_len, &(p2p_info->file_info), info_len);
                    raw_req_len += info_len;
                    p2p_info->raw_req_len = raw_req_len;
                    data->function = raw_req[1];
                    modbus_rt_mutex_unlock(&(dev->mutex));
                    //等待执行完毕
                    modbus_rt_sem_wait(&(data->completion));
                    if(MODBUS_RT_EOK != data->ret)
                    {
                        modbus_rt_sem_post(&(p2p_info->sem));
                        return ;
                    }

                    uint16_t cmd_recv =  ((uint16_t)(raw_req[2] << 8)) + raw_req[3];
                    uint16_t len_recv =  ((uint16_t)(raw_req[4] << 8)) + raw_req[5];
                    if((TRANS_FILE_CMD_START != cmd_recv) || (nb != len_recv)) {
                        data->ret = -MODBUS_RT_ERROR;
                        modbus_rt_sem_post(&(p2p_info->sem));
                        return ;
                    }
                    p2p_info->fd = modbus_rt_file_wb_open(p2p_info->file_name);
                    if(0 >= p2p_info->fd) {
                        data->ret = -MODBUS_RT_ERROR;
                        modbus_rt_sem_post(&(p2p_info->sem));
                        return ;
                    }
                    memcpy(&(p2p_info->file_info), &(raw_req[6]), len_recv);
                    p2p_info->read_file_serial = 0;
                    step = 1;
                } break;
                case 1: {
                    modbus_rt_mutex_lock(&(dev->mutex));
                    data->ret = -MODBUS_RT_ERROR;
                    data->slave_addr = raw_req[0];

                    p2p_info->read_file_serial++;
                    raw_req[raw_req_len++] = ((uint16_t)TRANS_FILE_CMD_DATA >> 8);
                    raw_req[raw_req_len++] = ((uint16_t)TRANS_FILE_CMD_DATA & 0xFF);
                    int len = 0x0002;
                    raw_req[raw_req_len++] = len >> 8;
                    raw_req[raw_req_len++] = len & 0xFF;
                    raw_req[raw_req_len++] = (p2p_info->read_file_serial) >> 8;
                    raw_req[raw_req_len++] = (p2p_info->read_file_serial) & 0xFF;

                    p2p_info->raw_req_len = raw_req_len;
                    data->function = raw_req[1];
                    modbus_rt_mutex_unlock(&(dev->mutex));
                    //等待执行完毕
                    modbus_rt_sem_wait(&(data->completion));
                    if(MODBUS_RT_EOK != data->ret)
                    {
                        modbus_rt_sem_post(&(p2p_info->sem));
                        return ;
                    }
                    uint16_t cmd_recv =  ((uint16_t)(raw_req[2] << 8)) + raw_req[3];
                    uint16_t len_recv =  ((uint16_t)(raw_req[4] << 8)) + raw_req[5];
                    if((TRANS_FILE_CMD_DATA != cmd_recv) || (0 >= p2p_info->fd)) {
                        data->ret = -MODBUS_RT_ERROR;
                        modbus_rt_sem_post(&(p2p_info->sem));
                        return ;
                    }
                    int flag = raw_req[6];
                    int file_len = len_recv -1;
                    data->ret = modbus_rt_file_write_file(p2p_info->fd, &(raw_req[7]), file_len);
                    if(0 >= data->ret) {
                        data->ret = -MODBUS_RT_ERROR;
                        modbus_rt_sem_post(&(p2p_info->sem));
                        return ;
                    } else {
                        data->ret = MODBUS_RT_EOK;
                    }
                    p2p_info->write_file_size += file_len;
                    if (0 == flag) {
                        step = 2;
                    }
                } break;
                default: {
                    modbus_rt_sem_post(&(p2p_info->sem));
                    return ;
                } break;
            }
        }
#endif
    }
}

/**
 * @brief   modbus_tcp_excuse_file:  modbus tcp master的文件上传和下载函数
 *          该函数为阻塞函数，执行完成或者超时后返回(采用信号量机制，不影响其他线程运行)。
 *
 * @param   dev:                tcp_modbus_device_t设备
 * @param   slave:              modbus slave的地址
 * @param   dir:                MODBUS_WRITE（传输文件到slave设备）， MODBUS_READ（从slave设备中读取文件）；
 * @param   file_dev:           slave设备中的文件名称（绝对路径）
 * @param   file_master:        master设备中的文件名称（绝对路径）
 * @return  int:                MODBUS_RT_EOK：成功，其他：失败
 *
 */
int modbus_tcp_excuse_file(tcp_modbus_device_t dev, int slave, modbus_excuse_dir_t dir, char *file_dev, char *file_master) {
    int ret = 0;
    if((NULL == dev) || (NULL == dev->data) || ((MODBUS_READ != dir) && (MODBUS_WRITE != dir)) ||
    (1 != dev->status)) {
        return -MODBUS_RT_EINVAL;
    }
    tcp_master_data_t    data = (tcp_master_data_t)dev->data;
#if MODBUS_P2P_SEND_ENABLE
#if (!MODBUS_P2P_RECV_ENABLE)
    if(MODBUS_READ == dir) {
        return -MODBUS_RT_EINVAL;
    }
#endif
    if(MODBUS_WRITE == dir) {
        modbus_p2p_master_info_t *p2p_info = &g_modbus_p2p_master_info;
        if(0 < p2p_info->fd) {
            modbus_rt_file_close(p2p_info->fd);
            p2p_info->fd = 0;
        }
        p2p_info->dir = MODBUS_WRITE;
        p2p_info->fd = modbus_rt_file_get_info(file_dev, file_master, &(p2p_info->file_info));
        if(0 >= p2p_info->fd) {
            return -MODBUS_RT_EIO;
        }
        p2p_info->raw_req[0] = slave;
        p2p_info->raw_req[1] = AGILE_MODBUS_FC_TRANS_FILE;

        p2p_info->write_file_size = 0;
        ret = modbus_rt_sem_init(&(p2p_info->sem));
        if(MODBUS_RT_EOK != ret) {
            modbus_rt_file_close(p2p_info->fd);
            p2p_info->fd = 0;
            return -MODBUS_RT_ENOMEM;
        }
        char str_name[MODBUS_RT_NAME_MAX] = {0};
        sprintf(str_name,"s%d",p2p_info->fd);
        p2p_info->thread = modbus_rt_thread_init(str_name, modbus_tcp_master_file_entry,
            dev, MODBUS_THREAD_STACK_SIZE, MODBUS_THREAD_PRIO, 10);
        if(NULL != p2p_info->thread) {
            modbus_rt_thread_startup(p2p_info->thread);
        } else {
            modbus_rt_file_close(p2p_info->fd);
            p2p_info->fd = 0;
            modbus_rt_sem_destroy(&(p2p_info->sem));
            return -MODBUS_RT_EINVAL;
        }
        modbus_rt_sem_wait(&(p2p_info->sem));

        if(0 < p2p_info->fd) {
            modbus_rt_file_close(p2p_info->fd);
            p2p_info->fd = 0;
        }
        modbus_rt_sem_destroy(&(p2p_info->sem));
        modbus_rt_thread_destroy(p2p_info->thread);
        if(MODBUS_RT_EOK != data->ret)
        {
            return data->ret;
        }
        return MODBUS_RT_EOK;
    }
#endif
#if MODBUS_P2P_RECV_ENABLE
#if (!MODBUS_P2P_SEND_ENABLE)
    if(MODBUS_WRITE == dir) {
        return -MODBUS_RT_EINVAL;
    }
#endif
    if(MODBUS_READ == dir) {
        modbus_p2p_master_info_t *p2p_info = &g_modbus_p2p_master_info;
        if(0 < p2p_info->fd) {
            modbus_rt_file_close(p2p_info->fd);
            p2p_info->fd = 0;
        }
        if(NULL == file_dev) {
            return -MODBUS_RT_EINVAL;
        }
        p2p_info->dir = MODBUS_READ;
        int len_file = strlen(file_dev);
        memcpy(p2p_info->file_info.file_name, file_dev, len_file);
        p2p_info->file_info.file_name[len_file] = 0;
        if(NULL != file_master) {
            len_file = strlen(file_master);
            memcpy(p2p_info->file_name, file_master, len_file);
            p2p_info->file_name[len_file] = 0;
        } else {
            memcpy(p2p_info->file_name, file_dev, len_file);
            p2p_info->file_name[len_file] = 0;
        }

        p2p_info->raw_req[0] = slave;
        p2p_info->raw_req[1] = AGILE_MODBUS_FC_READ_FILE;

        p2p_info->write_file_size = 0;
        ret = modbus_rt_sem_init(&(p2p_info->sem));
        if(MODBUS_RT_EOK != ret) {
            p2p_info->fd = 0;
            return -MODBUS_RT_ENOMEM;
        }
        char str_name[MODBUS_RT_NAME_MAX] = {0};
        sprintf(str_name,"r%d",p2p_info->fd);
        p2p_info->thread = modbus_rt_thread_init(str_name, modbus_tcp_master_file_entry,
            dev, MODBUS_THREAD_STACK_SIZE, MODBUS_THREAD_PRIO, 10);
        if(NULL != p2p_info->thread) {
            modbus_rt_thread_startup(p2p_info->thread);
        } else {
            modbus_rt_sem_destroy(&(p2p_info->sem));
            return -MODBUS_RT_EINVAL;
        }
        modbus_rt_sem_wait(&(p2p_info->sem));
        if(0 < p2p_info->fd) {
            modbus_rt_file_close(p2p_info->fd);
            p2p_info->fd = 0;
        }
        modbus_rt_sem_destroy(&(p2p_info->sem));
        modbus_rt_thread_destroy(p2p_info->thread);
        if(MODBUS_RT_EOK != data->ret)
        {
            return data->ret;
        }
        return MODBUS_RT_EOK;
    }
#endif
    return MODBUS_RT_EOK;
}
#endif

#endif

#endif
