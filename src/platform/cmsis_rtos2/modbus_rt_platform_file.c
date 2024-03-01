#include "modbus_rt_platform_file.h"

#if (MODBUS_P2P_ENABLE) 


//根据得到的文件信息创建文件,并以写的方式打开
int modbus_rt_file_write_info(uint8_t *data, size_t len, modbus_rt_file_info_t * file_info) {
    int fd = 0;
    if((NULL == data) || (sizeof(modbus_rt_file_info_t) != len)) {
        return -MODBUS_RT_ERROR;
    }
    memcpy(file_info, data, len);
    fd = open((char *)file_info->file_name, O_WRONLY | O_CREAT);
    if(0 >= fd) {
        return -MODBUS_RT_ERROR;
    }
    return fd;
}

int modbus_rt_file_read_info(uint8_t *data, size_t len, modbus_rt_file_info_t * file_info) {
    int ret = MODBUS_RT_EOK;
    int fd = 0;
    struct stat file_stat;
    if((NULL == data) || (sizeof(modbus_rt_file_info_t) != len)) {
        return -MODBUS_RT_ERROR;
    }
    memcpy(file_info, data, len);
    fd = open((char *)file_info->file_name, O_RDONLY);
    if(0 >= fd) {
        return -MODBUS_RT_ERROR;
    }
    ret = stat((char *)file_info->file_name, &file_stat);
    if(MODBUS_RT_EOK != ret) {
        return -MODBUS_RT_ERROR;
    }
    size_t file_len = file_stat.st_size;
    file_info->file_size = file_len;
    file_info->file_buf_len = 1024;
    return fd;
}

int modbus_rt_file_write_file(int fd, uint8_t *data, size_t size) {
    if((NULL == data) ||(0 >= fd) || (0 >= size)) {
        return -MODBUS_RT_EINVAL;
    }
    return write(fd, data, size);
}

//根据文件的目录，得到文件的信息，并以读的方式打开
int modbus_rt_file_get_info(char *file_dev, char *file_master, modbus_rt_file_info_t * file_info) {
    int ret = MODBUS_RT_EOK;
    int fd = 0;
    struct stat file_stat;
    if((NULL == file_dev) || (NULL == file_master) || (NULL == file_info)) {
        return -MODBUS_RT_ERROR;
    }
    fd = open(file_master, O_RDONLY);
    if(0 >= fd) {
        return -MODBUS_RT_ERROR;
    }
    ret = stat(file_master, &file_stat);
    if(MODBUS_RT_EOK != ret) {
        return -MODBUS_RT_ERROR;
    }
    size_t file_len = file_stat.st_size;
    file_info->file_size = file_len;
    file_info->file_buf_len = 1024;
    return fd;
}

int modbus_rt_file_wb_open(char *file_dev) {
    return open(file_dev, O_WRONLY | O_CREAT);
}

int modbus_rt_file_read_file(int fd, uint8_t *data, size_t size) {
    if((NULL == data) ||(0 >= fd) || (0 >= size)) {
        return -MODBUS_RT_EINVAL;
    }
    return read(fd, data, size);
}

int modbus_rt_file_close(int fd) {
    return close(fd);
}

#endif
