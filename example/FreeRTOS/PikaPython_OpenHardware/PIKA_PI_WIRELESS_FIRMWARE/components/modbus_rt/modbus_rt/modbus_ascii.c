/**
 * @file    modbus_ascii.c
 * @brief   基于Agile Modbus 的modbus ascii应用层实现，包含slave和master
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#include "modbus_ascii.h"

#if MODBUS_ASCII_SLAVE_ENABLE || MODBUS_ASCII_MASTER_ENABLE

/**
 * @brief   modbus rtu crc16校验函数，实现在agile_modbus_rtu.c中
 */
uint16_t agile_modbus_rtu_crc16(uint8_t *buffer, uint16_t buffer_length);

/**
 * @brief   modbus_ascii_lrc:       modbus ascii的lrc校验函数
 * @param   buffer:                 需要校验的数据内容
 * @return  uint8_t:                校验后得到的数据
 *       
 */
static uint8_t modbus_ascii_lrc(uint8_t *buffer, uint16_t buffer_length)
{
    int i;
    uint8_t lrc = 0;
    for(i=0; i < buffer_length; i++) {
        lrc += buffer[i];
    }
    lrc = (~lrc)+1;
    return lrc;
}

/**
 * @brief   modbus_rtu2ascii:       modbus rtu数据转化位modbus ascii的数据
 * @param   data                    数据内容，转化后的数据也依然存储在该空间中
 * @param   data_len                传递过来的位modbus rtu数据长度，转换完成之后位modbus ascii的数据长度
 * @return  None
 *       
 */
void modbus_rtu2ascii(uint8_t *data, int *data_len){
    uint8_t buf[(AGILE_MODBUS_MAX_ADU_LENGTH + 1) * 2] = {0};
    int len = *data_len;
    if(( 0 > len) || (AGILE_MODBUS_MAX_ADU_LENGTH < len)) {
        return ;
    }
    memcpy(buf, data, len);
    uint8_t lrc = modbus_ascii_lrc(data, len - 2);
    len = len - 2;          //去掉后面的CRC数据内容
    data[0] = ':';
    for(int i = 0; i < len; i++){
        sprintf((char *)&data[1+ i*2], "%02X", buf[i]);
    }
    len = len * 2;      //校验时起始位不算在内
    len = len + 1;
    sprintf((char *)&data[len], "%02X", lrc);
    len = len + 2;
    data[len] = '\r';
    data[len + 1] = '\n';
    data[len + 2] = 0;
    *data_len = len + 3;
}

/**
 * @brief   modbus_rtu2ascii:       modbus ascii数据转化位modbus rtu的数据
 * @param   data                    数据内容，转化后的数据也依然存储在该空间中
 * @param   data_len                传递过来的位modbus ascii数据长度，转换完成之后位modbus rtu的数据长度
 * @return  None
 *       
 */
void modbus_ascii2rtu(uint8_t *data, int *data_len){
    char buf[(AGILE_MODBUS_MAX_ADU_LENGTH + 1) * 2] = {0};
    int len = *data_len;
    if(( 0 > len) || ((((AGILE_MODBUS_MAX_ADU_LENGTH + 1) * 2)) < len)) {
        return ;
    }
    memcpy(buf, data, len);
    len = strlen(buf) - 4;
    for(int i = 0; i < len; i += 2){
        sscanf(&buf[i + 1], "%2hhX", &data[i/2]);
    }
    len = len/2;
    uint16_t crc = agile_modbus_rtu_crc16(data, len);
    data[len] = (uint8_t)(crc >> 8);
    data[len + 1] = (uint8_t)crc;
    *data_len = len + 2;
}



/**
 * @brief   modbus_ascii:           创造modbus ascii设备，注意这里只是修改了ascii_flag的标志位
 *                                  理论直接用modbus_rtu设备，然后通过modbus_rtu_set_ascii_flag设置
 *                                  ascii_flag为1也可以实现一样的功能
 * @param   mode:                   modbus类型： MODBUS_SLAVE/MODBUS_MASTER
 * @return  ascii_modbus_device_t:  ascii_modbus_device_t 数据指针
 *       
 */
ascii_modbus_device_t modbus_ascii(modbus_mode_type mode) {
    rtu_modbus_device_t dev = modbus_rtu(mode);
    if(NULL != dev) {
        dev->ascii_flag = 1;
    }
    return dev;
}

int modbus_ascii_set_serial(ascii_modbus_device_t dev, const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff) {
    return modbus_rtu_set_serial(dev, devname, baudrate, bytesize, parity, stopbits, xonxoff);
}

int modbus_ascii_open(ascii_modbus_device_t dev) {
    return modbus_rtu_open(dev);
}

int modbus_ascii_isopen(ascii_modbus_device_t dev) {
    return modbus_rtu_isopen(dev);
}

int modbus_ascii_close(ascii_modbus_device_t dev) {
    return modbus_rtu_close(dev);
}

int modbus_ascii_destroy(ascii_modbus_device_t * pos_dev) {
    return modbus_rtu_destroy(pos_dev);
}

int modbus_ascii_excuse(ascii_modbus_device_t dev, int dir_slave, int type_function, int addr, int quantity, void *ptr_data) {
    return modbus_rtu_excuse(dev, dir_slave, type_function, addr, quantity, ptr_data);
}

#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
int modbus_ascii_set_over_type(ascii_modbus_device_t dev, modbus_serial_over_type_t over_type) {
    return modbus_rtu_set_over_type(dev, over_type);
}

int modbus_ascii_set_net(ascii_modbus_device_t dev, char * ipaddr, unsigned int port, int type) {
    return modbus_rtu_set_net(dev, ipaddr, port, type);
}

int modbus_ascii_set_ip(ascii_modbus_device_t dev, char * ipaddr) {
    return modbus_rtu_set_ip(dev,ipaddr);
}

int modbus_ascii_set_port(ascii_modbus_device_t dev, unsigned int port) {
    return modbus_rtu_set_port(dev, port);
}

int modbus_ascii_set_type(ascii_modbus_device_t dev, int type) {
    return modbus_rtu_set_type(dev,type);
}

#endif

#if MODBUS_ASCII_SLAVE_ENABLE
int modbus_ascii_set_addr(ascii_modbus_device_t dev, int addr) {
    return modbus_rtu_set_addr(dev, addr);
}

int modbus_ascii_set_strict(ascii_modbus_device_t dev, uint8_t strict) {
    return modbus_rtu_set_strict(dev,strict);
}

int modbus_ascii_add_block(ascii_modbus_device_t dev, modbus_register_type_t type, int data_addr, void *data, int nums) {
    return modbus_rtu_add_block(dev, type, data_addr, data, nums);
}
int modbus_ascii_set_pre_ans_callback(ascii_modbus_device_t dev, int (*pre_ans)(agile_modbus_t *, int, int,int, int)) {
    return modbus_rtu_set_pre_ans_callback(dev, pre_ans);
}
int modbus_ascii_set_done_callback(ascii_modbus_device_t dev, int (*done)(agile_modbus_t *, int, int,int, int)) {
    return modbus_rtu_set_done_callback(dev, done);
}
#if SLAVE_DATA_DEVICE_BINDING
int modbus_ascii_set_dev_binding(ascii_modbus_device_t dev, int flag) {
    return modbus_rtu_set_dev_binding(dev, flag);
}
#endif
#endif

#if MODBUS_ASCII_MASTER_ENABLE
#if MODBUS_SERIAL_OVER_TCP_ENABLE || MODBUS_SERIAL_OVER_UDP_ENABLE
int modbus_ascii_set_server(ascii_modbus_device_t dev, char* saddr, unsigned int sport) {
    return modbus_rtu_set_server(dev, saddr, sport);
}

#endif
int modbus_ascii_excuse_ex(ascii_modbus_device_t dev, int slave, int function,int w_addr, int w_quantity, 
void *ptr_w_data, int r_addr, int r_quantity, void *ptr_r_data) {
    return modbus_rtu_excuse_ex(dev, slave, function, w_addr, w_quantity, ptr_w_data, r_addr, r_quantity, ptr_r_data);
}
#endif


#endif

