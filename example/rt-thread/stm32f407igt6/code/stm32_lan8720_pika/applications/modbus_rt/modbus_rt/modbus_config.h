/**
 * @file    modbus_condig.h
 * @brief   modbus_rt 的配置头文件
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */

#ifndef __PKG_MODBUS_CONFIG_H_
#define __PKG_MODBUS_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   线程相关的定义，包括线程名称的长度，线程堆栈大小，线程优先级等
 */
#define MODBUS_RT_NAME_MAX                          16
#define MODBUS_THREAD_STACK_SIZE                    (4096)
#define MODBUS_THREAD_PRIO                          (8)


/**
 @verbatim
    modbus_rt支持的modbus协议有
    rtu slave, rtu master，tcp slave, tcp master
    ascii slave, ascii master
    modbus rtu over TCP/UDP(包括slave/master)
    modbus ascii over TCP/UDP(包括slave/master)
    modbus TCP for UDP（包括slave/master）
    
    modbus 数据寄存器数据默认采用小端模式，支持大小端模式转化，包括四种，包括
    小端模式： little-endian byte swap
    大端模式： big-endian byte swap
    内部大段，外部小端模式： little-endian
    内部小端，外部大端模式： big-endian
 @endverbatim
 */


/**
 @verbatim
    modbus 基本协议有 rtu slave, rtu master, tcp slave, tcp master。
    其他协议基本都是基于以上协议扩展而来，相机可以见文档
 @endverbatim
 */
#ifndef MODBUS_RTU_SLAVE_ENABLE
    #define MODBUS_RTU_SLAVE_ENABLE                 1
#endif

#ifndef MODBUS_RTU_MASTER_ENABLE
    #define MODBUS_RTU_MASTER_ENABLE                1
#endif

#ifndef MODBUS_TCP_SLAVE_ENABLE
    #define MODBUS_TCP_SLAVE_ENABLE                 1
#endif

#ifndef MODBUS_TCP_MASTER_ENABLE
    #define MODBUS_TCP_MASTER_ENABLE                1
#endif


/**
 @verbatim
    需要注意的是：modbus ascii是基于modbus rtu实现的，
    如果要实现modbus ascii,必须使能对应的modbus rtu
    单独使能modbus ascii将无效
 @endverbatim
 */
#ifndef MODBUS_ASCII_SLAVE_ENABLE
    #define MODBUS_ASCII_SLAVE_ENABLE           1
#endif

#ifndef MODBUS_ASCII_MASTER_ENABLE
    #define MODBUS_ASCII_MASTER_ENABLE          1
#endif


/**
 @verbatim
    modbus TCP for UDP（包括slave/master）基于modbus tcp实现
    所以使能OVER_TCP或者OVER_UDP必须先使能对应的modbus rtu或者modbus ascii
 @endverbatim
 */
#ifndef MODBUS_TCP_SLAVE_FOR_UDP_ENABLE
    #define MODBUS_TCP_SLAVE_FOR_UDP_ENABLE     1
#endif

#ifndef MODBUS_TCP_MASTER_FOR_UDP_ENABLE
    #define MODBUS_TCP_MASTER_FOR_UDP_ENABLE    1
#endif


/**
 @verbatim
    modbus rtu和modbus ascii等可以跑在TCP/UDP上， 
    如果需要运行for udp模式，需要先开启对应的modbus tcp slave或者modbus tcp master
 @endverbatim
 */
#ifndef MODBUS_SERIAL_OVER_TCP_ENABLE
    #define MODBUS_SERIAL_OVER_TCP_ENABLE           1
#endif

#ifndef MODBUS_SERIAL_OVER_UDP_ENABLE
    #define MODBUS_SERIAL_OVER_UDP_ENABLE           1
#endif


/**
 @verbatim
    modbus tcp master自动重连机制，
    该宏定义，主要定义了tcp master 的自动重连机制和重连时间间隔
 @endverbatim
 */
#if MODBUS_TCP_MASTER_ENABLE
    #ifndef MODBUS_TCP_MASTER_RECONNECT
        #define     MODBUS_TCP_MASTER_RECONNECT             1
        #define     MODBUS_TCP_MASTER_RECONNECT_TIME_OUT    (2000)
    #endif
#endif


/**
 @verbatim
    modbus slave 寄存器是否和硬件外设绑定，默认不绑定
    如果绑定，需要自己添加"device_data.h"头文件和以下函数：
    dev_write_bits：            写bits寄存器
    dev_read_bits：             读bits寄存器
    dev_write_regs：            写regs寄存器
    dev_read_regs：             读regs寄存器
    dev_data2modbus_slave：     寄存器初始化（绑定slave寄存器和硬件外设信息）
 @endverbatim
 */
#if (MODBUS_RTU_SLAVE_ENABLE) || (MODBUS_TCP_SLAVE_ENABLE)
    #ifndef SLAVE_DATA_DEVICE_BINDING
        #define     SLAVE_DATA_DEVICE_BINDING             1
    #endif
#endif


/**
 @verbatim
    如果开启MODBUS_DATA_TRANS_ENABLE，则可以使用大小端转化相关函数
 @endverbatim
 */
#ifndef MODBUS_DATA_TRANS_ENABLE
    #define MODBUS_DATA_TRANS_ENABLE                 1
#endif


/**
 @verbatim
    针对性能有限的处理器而言，限制socket的数量，以确保内存和性能满足要求。
    如果使能socket数量限制，需要先使能TCP_MODBUS_NUMS_ENABLE，
    此时socket的数量最高为TCP_MODBUS_NUMS，超过之后，所有modbus_rt创建soket会失败
 @endverbatim
 */
#ifndef TCP_MODBUS_NUMS_ENABLE
    #define TCP_MODBUS_NUMS_ENABLE                  1
#endif

#if MODBUS_TCP_SLAVE_ENABLE
    #ifndef TCP_MODBUS_NUMS
        #define TCP_MODBUS_NUMS                     6
    #endif
    extern int tcp_modbus_nums;
#endif

/**
 @verbatim
    modbus rtu和modbus ascii串口相关的参数，
    包括文件的串口设备名称的最大长度，串口接收数据超时（在一定时间内没有应答则会返回超时）
    以及串口通信中modbus允许的最大单帧时间间隔。
    注意：ModBus规定单帧时间间隔不得超过3.5个帧单位，
    大致是35~40个二进制位（参数不同，略有不同），所以一般取45个二进制周期
    这里为了防止数据丢包，每个字节的时间间隔只要不超过MODBUS_RTU_BYTE_TIME_OUT_MIN，
    就认为，该数据未一个数据包。
 @endverbatim
 */
#if (MODBUS_RTU_SLAVE_ENABLE) || defined(MODBUS_RTU_MASTER_ENABLE)
    #ifndef MODBUS_RTU_NAME_MAX
        #define MODBUS_RTU_NAME_MAX 24
    #endif
    #ifndef MODBUS_RTU_TIME_OUT
        #define     MODBUS_RTU_TIME_OUT    (1000)
    #endif
    #ifndef MODBUS_RTU_TIME_OUT_BITS
        #define     MODBUS_RTU_TIME_OUT_BITS    (45)
    #endif
    #ifndef MODBUS_RTU_BYTE_TIME_OUT_MIN
        #define     MODBUS_RTU_BYTE_TIME_OUT_MIN   (4)
    #endif
#endif


/**
 @verbatim
    modbus slave模式如果允许在tcp模式下，创建的是tcp server，
    该宏定义，主要定义了tcp server允许连接client的最大数量
 @endverbatim
 */
#if MODBUS_TCP_SLAVE_ENABLE
    #define SOCKET_CONNECT_NUMS         (5)     //最多支持连接到modbus tcp slave上的master的数量
#endif

/**
 @verbatim
    modbus_rt在基于UDP实现modbus的基础上，扩展实现是否支持UDP广播搜索功能，
    主要目的是：实现跨局域网的设备查找功能，即在不知道设备IP的情况下可以查找设备
    即，可以通过像255.255.255.255广播modbus命令，slave端会对发送数据的ip做判断，
    如果与本机不在同一个网段，会默认广播应答数据，以便master可以捕获设备的IP地址
    注意：在某些系统上（有多网卡的设备），开启UDP广播搜索功能需要指定设备的网卡，以获取本机的IP地址
        （即在初始化时，需要调用xxx_set_net或者xxx_set_ip函数指定为特定网卡的IP地址，
        而不能使用空字符串或者采用本机默认IP，否则可能无法接收到广播的数据）
        另外，该功能可能需要路由器或者交换机支持。
 @endverbatim
 */
#if (MODBUS_TCP_SLAVE_FOR_UDP_ENABLE) || (MODBUS_TCP_MASTER_FOR_UDP_ENABLE) || (MODBUS_SERIAL_OVER_UDP_ENABLE)
    #ifndef MODBUS_UDP_FOR_SEARCH
        #define MODBUS_UDP_FOR_SEARCH           1
    #endif
#endif

/**
 @verbatim
    modbus_rt函数返回值定义
 @endverbatim
 */
#define MODBUS_RT_EOK                          0                /**< 返回正常 */
#define MODBUS_RT_ERROR                        1                /**< 一个标准错误或者未定义错误 */
#define MODBUS_RT_ETIMEOUT                     2                /**< 超时 */
#define MODBUS_RT_EFULL                        3                /**< 数据已满 */
#define MODBUS_RT_EEMPTY                       4                /**< 数据为空 */
#define MODBUS_RT_ENOMEM                       5                /**< 内存空间不足 */
#define MODBUS_RT_ENOSYS                       6                /**< 系统错误 */
#define MODBUS_RT_EBUSY                        7                /**< 设备忙 */
#define MODBUS_RT_EIO                          8                /**< IO 错误 */
#define MODBUS_RT_EINTR                        9                /**< 系统中断错误 */
#define MODBUS_RT_EINVAL                       10               /**< 函数参数错误 */
#define MODBUS_RT_ISOPEN                       11               /**< 设备已经开启 */
#define MODBUS_RT_REMOTE                       12               /**< 远程主机错误，client无法连接 */
#define MODBUS_RT_HOST_ERROR                   13               /**< 主机域名解析错误 */


/**
 @verbatim
    modbus_rt主从机判断
 @endverbatim
 */
typedef enum {
    MODBUS_SLAVE = 0,
    MODBUS_MASTER = 1,
}modbus_mode_type;

/**
 @verbatim
    modbus_rt读取还是写入数据判断
 @endverbatim
 */
typedef enum {
    MODBUS_READ = 0x00,              //读
    MODBUS_WRITE = 0x01,             //写
}modbus_excuse_dir_t;


#ifdef __cplusplus
}
#endif


#endif /* __PKG_MODBUS_CONFIG_H_ */
