/**
 * @file    modbus_rtu.h
 * @brief   modbus rtu 头文件
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#ifndef __PKG_MODBUS_RTU_H_
#define __PKG_MODBUS_RTU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "modbus_config.h"

#if (MODBUS_RTU_SLAVE_ENABLE) || (MODBUS_RTU_MASTER_ENABLE)
#include "agile_modbus.h"                   //agile_modbus的头文件
#include "modbus_slave_util.h"        //agile_modbus的从机接口头文件
#include "modbus_rt_platform_memory.h"
#include "modbus_rt_platform_thread.h"
#include "modbus_rt_platform_serial.h"


#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
#include "modbus_rt_platform_net_socket.h"
#endif

/**
 * @brief   modbus rtu 串口参数结构体
 */
struct modbus_rt_serial_info {
    char devname[MODBUS_RTU_NAME_MAX];  //串口设备的名称
    int baudrate;                       //串口波特率                    
    int bytesize;                       //串口数据位
    char parity;                        //串口校验位
    int stopbits;                       //串口停止位
    int xonxoff;                        //串口流控制
};

/**
 * @brief   modbus rtu over 模式选择，目前可选择OVER_NET表示TCP/UDP网络模式
 */
typedef enum {
    OVER_NONE = 0x00,              //NONE
    OVER_NET = 0x01,              //over NET
}modbus_serial_over_type_t;

/**
 * @brief   modbus rtu通信结构体
 */
struct rtu_modbus_device {
    int serial;                                 //绑定的串口设备句柄
    struct modbus_rt_serial_info serial_info;   //串口信息
    int byte_timeout;                           //modbus超时时间，可以直接通过config计算得到   
#if MODBUS_ASCII_SLAVE_ENABLE || MODBUS_ASCII_MASTER_ENABLE
    int ascii_flag;                             //是否实现对modbus ascii的支持
#endif
    modbus_serial_over_type_t over_type;        //特殊寄存器覆写模式
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
    int sock;                                   //TCP_Modbus对应的socket
    int type;                                   //类型：SOCK_STREAM(1)：TCP;SOCK_DGRAM(2):UDP
    char *ipaddr;                               //设备的ip地址
    char ip_buf[INET_ADDRSTRLEN];               //设备端的ip地址
    unsigned int port;                          //端口号
#endif
    int status;                                 //设备状态，是否已经打开
    modbus_mode_type mode;                      //modbus rtu工作模式：SLAVE/MASTER
#if MODBUS_P2P_ENABLE
    int p2p_flag;                               //是否开启p2p功能，即通过modbus传输文件的功能
#endif
    agile_modbus_rtu_t ctx_rtu;                 //agile_modbus_tcp_t结构体
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
typedef struct rtu_modbus_device * rtu_modbus_device_t;

#if MODBUS_RTU_SLAVE_ENABLE
#include "modbus_slave.h"

/**
 * @brief   modbus rtu slave结构体
 */
struct rtu_slave_data {
    int addr;                           //modbus地址
    uint8_t slave_strict;               //是否对modbus地址做判断，由于modbus tcp可以通过IP来区分，所以理论可以不对地址做判断
    agile_modbus_slave_util_t util;     //用于绑定slave的寄存器的信息
};
typedef struct rtu_slave_data * rtu_slave_data_t;
#endif

#if MODBUS_RTU_MASTER_ENABLE
/**
 * @brief   modbus rtu master结构体
 */
struct rtu_master_data {

#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
    char saddr[INET_ADDRSTRLEN];             //slave端的ip地址
    unsigned int sport;                     //slave端的端口号
#endif
    //fuction是否执行完成的完成量，这里特别说明以下,由于tcp client的限制，
    //有可能式server端断开了，所以倒是socket没有及时关闭，会导致内存泄漏，
    //所以这里有需要一个线程检测server断开，所以不能用dev中的完成量作为执行完成标志
    modbus_rt_sem_t   completion;    //fuction是否执行完成的完成量
    int slave_addr;                         //modbus 从机地址
    int function;                           //Modbus的操作命令
    int data_addr;                          //Modbus的操作地址
    int quantity;                           //Modbus的操作地址长度
    int ret;                                //执行命令的返回值
    void *ptr_data;                       //Modbus的操作数据指针
     /*
     * 针对特殊命令AGILE_MODBUS_FC_WRITE_AND_READ_REGISTERS使用
     */
    int read_addr;
    int read_quantity;
    void *ptr_read_data;                 //Modbus的操作数据指针
};
typedef struct rtu_master_data * rtu_master_data_t;
#endif


/**
 * @brief   modbus rtu API函数
 */
rtu_modbus_device_t modbus_rtu(modbus_mode_type mode);
int modbus_rtu_set_serial(rtu_modbus_device_t dev, const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff);

#if MODBUS_ASCII_SLAVE_ENABLE || MODBUS_ASCII_MASTER_ENABLE
    int modbus_rtu_set_ascii_flag(rtu_modbus_device_t dev, int flag);       //理论上modbus ascii只是通过modbus rtu的该标志位来定义
#endif

int modbus_rtu_open(rtu_modbus_device_t dev);
int modbus_rtu_isopen(rtu_modbus_device_t dev);
int modbus_rtu_close(rtu_modbus_device_t dev);
int modbus_rtu_destroy(rtu_modbus_device_t * pos_dev);
int modbus_rtu_excuse(rtu_modbus_device_t dev, int dir_slave, int type_function, int addr, int quantity, void *ptr_data);
#if  MODBUS_P2P_ENABLE
    int modbus_rtu_set_p2p_flag(rtu_modbus_device_t dev, int flag);
#endif

#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
    int modbus_rtu_set_over_type(rtu_modbus_device_t dev, modbus_serial_over_type_t over_type);
    int modbus_rtu_set_net(rtu_modbus_device_t dev, char * ipaddr, unsigned int port, int type);
    int modbus_rtu_set_ip(rtu_modbus_device_t dev, char * ipaddr);
    int modbus_rtu_set_port(rtu_modbus_device_t dev, unsigned int port);
    int modbus_rtu_set_type(rtu_modbus_device_t dev, int type);
#endif

#if MODBUS_RTU_SLAVE_ENABLE
    int modbus_rtu_set_addr(rtu_modbus_device_t dev, int addr);
    int modbus_rtu_set_strict(rtu_modbus_device_t dev, uint8_t strict);
    int modbus_rtu_add_block(rtu_modbus_device_t dev, modbus_register_type_t type, int data_addr, void *data, int nums);
    int modbus_rtu_set_pre_ans_callback(rtu_modbus_device_t dev, int (*pre_ans)(agile_modbus_t *, int, int,int, int));
    int modbus_rtu_set_done_callback(rtu_modbus_device_t dev, int (*done)(agile_modbus_t *, int, int,int, int));
#if SLAVE_DATA_DEVICE_BINDING
    int modbus_rtu_set_dev_binding(rtu_modbus_device_t dev, int flag);
#endif
#endif

#if MODBUS_RTU_MASTER_ENABLE
    #if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
        int modbus_rtu_set_server(rtu_modbus_device_t dev, char* saddr, unsigned int sport);
        int modbus_rtu_get_saddr(rtu_modbus_device_t dev, char* saddr);
    #endif
    int modbus_rtu_excuse_ex(rtu_modbus_device_t dev, int slave, int function,int w_addr, int w_quantity, 
                            void *ptr_w_data, int r_addr, int r_quantity, void *ptr_r_data);
    #if  ((MODBUS_P2P_ENABLE) && (MODBUS_P2P_MASTER_ENABLE))
        int modbus_rtu_excuse_file(rtu_modbus_device_t dev, int slave, modbus_excuse_dir_t dir, char *file_dev, char *file_master);
    #endif
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* __PKG_MODBUS_RTU_H_ */
