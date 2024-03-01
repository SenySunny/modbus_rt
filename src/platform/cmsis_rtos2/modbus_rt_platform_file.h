#ifndef __PKG_MODBUS_RT_PLATFORM_FILE_H_
#define __PKG_MODBUS_RT_PLATFORM_FILE_H_

#include "../../modbus/modbus_config.h"

#if (MODBUS_P2P_ENABLE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct modbus_rt_file_info
{
    uint32_t			file_type;
    uint32_t 			file_size;                             //接收文件的总大小
    uint32_t 			file_buf_len;                          //缓冲区的包的长度
    uint8_t 			file_name[MODBUS_P2P_FILE_LEN];        //存储文件路径和名称
} modbus_rt_file_info_t;


extern modbus_rt_file_info_t g_modbus_rt_file_info;

int modbus_rt_file_write_info(uint8_t *data, size_t len, modbus_rt_file_info_t * file_info);
int modbus_rt_file_read_info(uint8_t *data, size_t len, modbus_rt_file_info_t * file_info);
int modbus_rt_file_write_file(int fd, uint8_t *data, size_t size);

int modbus_rt_file_get_info(char *file_dev, char *file_master, modbus_rt_file_info_t * file_info);
int modbus_rt_file_wb_open(char *file_dev);
int modbus_rt_file_read_file(int fd, uint8_t *data, size_t size);

int modbus_rt_file_close(int fd);


#ifdef __cplusplus
}
#endif

#endif

#endif /* __PKG_MODBUS_RT_PLATFORM_FILE_H_ */
