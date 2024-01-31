/**
 * @file    modbus_tcp.h
 * @brief   modbus tcp 头文件
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#ifndef __PKG_MODBUS_TCP_H_
#define __PKG_MODBUS_TCP_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "modbus_config.h"

#if (MODBUS_TCP_SLAVE_ENABLE) || (MODBUS_TCP_MASTER_ENABLE)

#include "agile_modbus.h"                   //agile_modbus的头文件
#include "modbus_slave_util.h"              //agile_modbus的从机接口头文件
#include "modbus_rt_platform_memory.h"
#include "modbus_rt_platform_thread.h"
#include "modbus_rt_platform_net_socket.h"

/**
 * @brief   modbus tcp通信结构体
 */
struct tcp_modbus_device {

    int sock;                                   //TCP_Modbus对应的socket
    int type;                                   //类型：SOCK_STREAM(1)：TCP;SOCK_DGRAM(2):UDP
    char *ipaddr;                               //设备的ip地址
    char ip_buf[INET_ADDRSTRLEN];               //设备端的ip地址
    unsigned int port;                          //端口号
    int status;                                 //设备状态，是否已经打开
    modbus_mode_type mode;                      //modbus rtu工作模式：SLAVE/MASTER
#if MODBUS_P2P_ENABLE
    int p2p_flag;                               //是否开启p2p功能，即通过modbus传输文件的功能
#endif
    agile_modbus_tcp_t ctx_tcp;                 //agile_modbus_tcp_t结构体
    agile_modbus_t *ctx;                        //agile_modbus的句柄
    int send_len;                               //打包需要发送数据的长度
    int read_len;                               //串口接收到的数据的长度
    uint8_t *ctx_send_buf;                      //存储发送缓冲区的数据
    uint8_t *ctx_read_buf;                      //存储接收缓冲区的数据
    modbus_rt_thread_t *thread;                 //运行的线程标志
    modbus_rt_mutex_t mutex;                    //线程间数据访问的互斥量
    int thread_flag;                            //线程运行标志
    modbus_rt_sem_t   sem;                      //控制线程任务完成的的信号量

    void * data;                                //modbus的数据内容，更具SLAVE或者MASTER不同而不同
};
typedef struct tcp_modbus_device * tcp_modbus_device_t;

#if MODBUS_TCP_SLAVE_ENABLE
#include "modbus_slave.h"

/**
 * @brief   modbus tcp slave结构体
 */
struct tcp_slave_data {
    int addr;                                   //modbus地址
    uint8_t slave_strict;                       //是否对modbus地址做判断，由于modbus tcp可以通过IP来区分，所以理论可以不对地址做判断

    agile_modbus_slave_util_t util;             //用于绑定slave的寄存器的信息
};
typedef struct tcp_slave_data * tcp_slave_data_t;
#endif

#if MODBUS_TCP_MASTER_ENABLE
/**
 * @brief   modbus tcp master结构体
 */
struct tcp_master_data {
    //记录slave端的Ip和端口
    char saddr[INET_ADDRSTRLEN];                //slave端的ip地址
    unsigned int sport;                         //slave端的端口号

    //fuction是否执行完成的完成量，这里特别说明以下,由于tcp client的限制，
    //有可能式server端断开了，所以倒是socket没有及时关闭，会导致内存泄漏，
    //所以这里有需要一个线程检测server断开，所以不能用dev中的完成量作为执行完成标志
    modbus_rt_sem_t   completion;               //fuction是否执行完成的完成量

    int slave_addr;                             //modbus 从机地址
    int function;                               //Modbus的操作命令

    int data_addr;                              //Modbus的操作地址
    int quantity;                               //Modbus的操作地址长度

    int ret;                                    //执行命令的返回值

    void *ptr_data;                             //Modbus的操作数据指针

     /*
     * 针对特殊命令AGILE_MODBUS_FC_WRITE_AND_READ_REGISTERS使用
     */
    int read_addr;
    int read_quantity;
    void *ptr_read_data;                        //Modbus的操作数据指针
};
typedef struct tcp_master_data * tcp_master_data_t;
#endif

/**
 * @brief   modbus tcp API函数
 */
tcp_modbus_device_t modbus_tcp(modbus_mode_type mode);
int modbus_tcp_set_net(tcp_modbus_device_t dev, char * ipaddr, unsigned int port, int type);
int modbus_tcp_set_ip(tcp_modbus_device_t dev, char * ipaddr);
int modbus_tcp_set_port(tcp_modbus_device_t dev, unsigned int port);
int modbus_tcp_set_type(tcp_modbus_device_t dev, int type);
int modbus_tcp_open(tcp_modbus_device_t dev);
int modbus_tcp_isopen(tcp_modbus_device_t dev);
int modbus_tcp_close(tcp_modbus_device_t dev);
int modbus_tcp_destroy(tcp_modbus_device_t * pos_dev);
int modbus_tcp_excuse(tcp_modbus_device_t dev, int dir_slave, int type_function, int addr, int quantity, void *ptr_data);
#if  MODBUS_P2P_ENABLE
    int modbus_tcp_set_p2p_flag(tcp_modbus_device_t dev, int flag);
#endif

#if MODBUS_TCP_SLAVE_ENABLE
    int modbus_tcp_set_addr(tcp_modbus_device_t dev, int addr);
    int modbus_tcp_set_strict(tcp_modbus_device_t dev, uint8_t strict);
    int modbus_tcp_add_block(tcp_modbus_device_t dev, modbus_register_type_t type, int data_addr, void *data, int nums);
    int modbus_tcp_set_pre_ans_callback(tcp_modbus_device_t dev, int (*pre_ans)(agile_modbus_t *, int, int,int, int));
    int modbus_tcp_set_done_callback(tcp_modbus_device_t dev, int (*done)(agile_modbus_t *, int, int,int, int));
#if SLAVE_DATA_DEVICE_BINDING
    int modbus_tcp_set_dev_binding(tcp_modbus_device_t dev, int flag);
#endif
#endif

#if MODBUS_TCP_MASTER_ENABLE
    int modbus_tcp_set_server(tcp_modbus_device_t dev, char* saddr, unsigned int sport);
    int modbus_tcp_get_saddr(tcp_modbus_device_t dev, char* saddr);
    int modbus_tcp_excuse_ex(tcp_modbus_device_t dev, int slave, int function,int w_addr, int w_quantity, 
                            void *ptr_w_data, int r_addr, int r_quantity, void *ptr_r_data);
    #if  ((MODBUS_P2P_ENABLE) && (MODBUS_P2P_MASTER_ENABLE))
        int modbus_tcp_excuse_file(tcp_modbus_device_t dev, int slave, modbus_excuse_dir_t dir, char *file_dev, char *file_master);
    #endif
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* __PKG_MODBUS_TCP_H_ */
