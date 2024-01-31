#include "modbus_rt_platform_file.h"

#if (MODBUS_P2P_ENABLE) 
#include "modbus_rt_platform_memory.h"
static modbus_rt_file_t *p_file_info = NULL;

static int modbus_rt_file_add(FILE *fp)
{
    int fd = 0;
    modbus_rt_file_t *file_temp = modbus_rt_malloc(sizeof(struct modbus_rt_file));
    if(NULL == file_temp) {
        return - MODBUS_RT_ENOMEM;
    }
    memset(file_temp, 0,  sizeof(struct modbus_rt_file));
    if(NULL == p_file_info) {
        p_file_info = file_temp;
        fd++;
    } else {
        modbus_rt_file_t *temp = p_file_info;
        fd = temp->fd;
        while(NULL != temp->next) {
            fd = temp->fd;
            temp = temp->next;
        }
        temp->next = file_temp;
        file_temp->pre = temp;
        fd++;
    }
    file_temp->fp = fp;
    file_temp->fd = fd;
    return fd;
}

static FILE * modbus_rt_file_get(int fd)
{
    modbus_rt_file_t *file_temp = p_file_info;
    while( NULL != file_temp) {
        if(fd == file_temp->fd){
            return file_temp->fp;
        } 
        file_temp = file_temp->next;
    }
    return NULL;
}

//根据得到的文件信息创建文件,并以写的方式打开
int modbus_rt_file_write_info(uint8_t *data, size_t len, modbus_rt_file_info_t * file_info) {
    FILE *fp = NULL;
    if((NULL == data) || (sizeof(modbus_rt_file_info_t) != len)) {
        return -MODBUS_RT_ERROR;
    }
    memcpy(file_info, data, len);
    fp = fopen((char *)file_info->file_name, "wb");
    if(NULL == fp) {
        return -MODBUS_RT_ERROR;
    }
    return modbus_rt_file_add(fp);
}

int modbus_rt_file_read_info(uint8_t *data, size_t len, modbus_rt_file_info_t * file_info) {
    FILE *fp = NULL;
    if((NULL == data) || (sizeof(modbus_rt_file_info_t) != len)) {
        return -MODBUS_RT_ERROR;
    }
    memcpy(file_info, data, len);
    fp = fopen((char *)file_info->file_name, "rb");
    if(NULL == fp) {
        return -MODBUS_RT_ERROR;
    }
    fseek(fp, 0, SEEK_END);
    size_t file_len = ftell(fp);
    file_info->file_size = file_len;
    file_info->file_buf_len = 1024;
    fseek(fp, 0, SEEK_SET);
    return modbus_rt_file_add(fp);
}

int modbus_rt_file_write_file(int fd, uint8_t *data, size_t size) {
    int ret = MODBUS_RT_EOK;
    FILE *fp = modbus_rt_file_get(fd);
    if((NULL == data) ||(NULL == fp) || (0 >= size)) {
        return -MODBUS_RT_EINVAL;
    }
    return fwrite(data, size, 1, fp);
}

//根据文件的目录，得到文件的信息，并以读的方式打开
int modbus_rt_file_get_info(char *file_dev, char *file_master, modbus_rt_file_info_t * file_info) {
    FILE *fp = NULL;
    if((NULL == file_dev) || (NULL == file_master) || (NULL == file_info)) {
        return -MODBUS_RT_ERROR;
    }
    fp = fopen(file_master, "rb");
    if(NULL == fp) {
        return -MODBUS_RT_ERROR;
    }
    fseek(fp, 0, SEEK_END);
    size_t file_len = ftell(fp);
    int name_len = strlen(file_dev);
    memcpy(file_info->file_name, file_dev, name_len);
    file_info->file_name[name_len] = 0;
    file_info->file_size = file_len;
    file_info->file_buf_len = 1024;
    fseek(fp, 0, SEEK_SET);
    return modbus_rt_file_add(fp);
}

int modbus_rt_file_wb_open(char *file_dev) {
    FILE *fp = fopen(file_dev, "wb");
    if(NULL == fp) {
        return -MODBUS_RT_ERROR;
    }
    return modbus_rt_file_add(fp);
}

int modbus_rt_file_read_file(int fd, uint8_t *data, size_t size) {
    int ret = MODBUS_RT_EOK;
    FILE *fp = modbus_rt_file_get(fd);
    if((NULL == data) ||(NULL == fp) || (0 >= size)) {
        return -MODBUS_RT_EINVAL;
    }
    return fread(data, 1, size, fp);
}


int modbus_rt_file_close(int fd)
{
    int ret = MODBUS_RT_EOK;
    modbus_rt_file_t *file_temp = p_file_info;
    while( NULL != file_temp) {
        if(fd == file_temp->fd){
            break;
        } 
        file_temp = file_temp->next;
    }
    if(NULL == file_temp) {
        return -MODBUS_RT_ERROR;
    }
    FILE *fp = file_temp->fp;
    ret = fclose(fp);
    if(NULL == file_temp->pre) {
        if(NULL == file_temp->next) {
            p_file_info = NULL;
            modbus_rt_free(file_temp);
        } else {
            p_file_info = file_temp->next;
            modbus_rt_free(file_temp);
        }
    } else if(NULL == file_temp->next) {
        file_temp->pre->next = NULL;
        modbus_rt_free(file_temp);
    } else {
        file_temp->pre->next = file_temp->next;
        file_temp->next->pre = file_temp->pre;
        modbus_rt_free(file_temp);
    }
    return ret;
}

#endif
