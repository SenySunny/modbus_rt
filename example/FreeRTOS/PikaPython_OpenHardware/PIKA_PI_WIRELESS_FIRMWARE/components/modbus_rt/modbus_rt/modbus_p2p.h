/**
 * @file    modbus_lave.h
 * @brief   modbus slave 头文件
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#ifndef __PKG_MODBUS_P2P_H_
#define __PKG_MODBUS_P2P_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "modbus_config.h"

#if  MODBUS_P2P_ENABLE
#include "agile_modbus.h"                   //agile_modbus的头文件
#include "modbus_slave_util.h"              //agile_modbus的从机接口头文件
#include "modbus_rt_platform_file.h"
#include "modbus_rt_platform_thread.h"

#define     P2P_SLAVE_BUF_MAX_LEN         (2048)    //定义文件传输时的缓冲区大小
#define     P2P_SLAVE_BUF_LEN             (1024)    //文件传输时，每次传输的数据大小，理论上缓冲区大小需要比传输文件大

#define AGILE_MODBUS_FC_TRANS_FILE  0x78                //自定义传输文件的命令
#define AGILE_MODBUS_FC_READ_FILE   0x79                //自定义传输文件的命令
#define TRANS_FILE_CMD_START       0x0001               //传输文件信息        
#define TRANS_FILE_CMD_DATA        0x0002               //传输文件内容
#define TRANS_FILE_FLAG_END        0x00                 //文件末尾标识符，已经达到文件末尾
#define TRANS_FILE_FLAG_NOT_END    0x01                 //文件末尾标识符，没有达到文件末尾

#if MODBUS_P2P_MASTER_ENABLE
/**
 * @brief   modbus p2p master结构体
 */
typedef struct modbus_p2p_master_info {
        modbus_excuse_dir_t dir;                 //定义需要发送还是接收的数据

        int fd;                                 //文件操作指针
        char file_name[MODBUS_P2P_FILE_LEN];  //本地存储文件路径和名称
        uint32_t write_file_size;                    //已经发送或者接收到的文件大小

        modbus_rt_thread_t *thread;             //发送文件的线程
        modbus_rt_sem_t   sem;                  //文件发送或者读取的线程信号量
        uint8_t raw_req[P2P_SLAVE_BUF_MAX_LEN];    //文件发送或者读取的数据缓冲区
        int raw_req_len;                        //文件发送或者读取的长度变量缓冲区

#if MODBUS_P2P_RECV_ENABLE
        modbus_rt_file_info_t file_info;        //需要发送到slave端的的数据信息
        int read_file_serial;                   //发送文件的序号，发送多少次
#endif
}modbus_p2p_master_info_t;

extern modbus_p2p_master_info_t g_modbus_p2p_master_info;

#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* __PKG_MODBUS_P2P_H_ */
