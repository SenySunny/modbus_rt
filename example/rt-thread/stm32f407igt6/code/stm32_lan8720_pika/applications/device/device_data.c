/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     SenyPC       the first version
 */
#include "device_data.h"
#include "fal.h"
#include "dido.h"

#if MODBUS_TCP_SLAVE_ENABLE
    void modbus_slave_util_init(agile_modbus_slave_util_t *util);
    int modbus_slave_add_val(agile_modbus_slave_util_t *util, modbus_register_type_t type,
                            int data_addr, void *data, int nums);
    int modbus_slave_clear_val(agile_modbus_slave_util_t *util);
    int modbus_slave_read(agile_modbus_slave_util_t *util, modbus_register_type_t type, int addr, int quantity, void *ptr_data);
    int modbus_slave_write(agile_modbus_slave_util_t *util, modbus_register_type_t type, int addr, int quantity, void *ptr_data);
#endif

uint8_t dev_di_data[DEV_X_BIT_NUMS]       = {0};          //默认di数据
uint8_t dev_do_data[DEV_Y_BIT_NUMS]       = {0};          //默认do数据

uint8_t dev_bit_data[DEV_BIT_NUMS] = {0};                 //默认添加的0x数据
uint8_t dev_input_bit_data[DEV_BIT_NUMS] = {0};           //默认添加的1x数据

uint16_t dev_reg_data[DEV_REG_NUMS] = {0};                  //默认添加的4x数据
uint16_t dev_input_reg_data[DEV_REG_NUMS] = {0};            //默认添加的3x数据

uint16_t dev_sp_reg_data[DEV_SP_REG_NUMS] = {0};            //默认添加特殊4x数据，掉电不丢失
uint16_t dev_sp_input_reg_data[DEV_SP_REG_NUMS] = {0};      //默认添加特殊4x数据，掉电不丢失


static int user_fal_init(void) {
    fal_init();
    fal_blk_device_create(BLK_DEV_NAME);
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(user_fal_init);

static void set_dev_name(const struct fal_partition * dev_partition) {
    int dev_name_len = strlen(DEV_NAME);
    uint8_t *buf_temp = rt_calloc(1,FLASH_PAGE_SIZE);
    fal_partition_read(dev_partition, 0, buf_temp, FLASH_PAGE_SIZE);
    memcpy(&(buf_temp[0]), DEV_NAME, dev_name_len);
    buf_temp[dev_name_len] = 0;
    fal_partition_erase_all(dev_partition);
    fal_partition_write(dev_partition, 0, buf_temp, FLASH_PAGE_SIZE);
    rt_free(buf_temp);
}

static int dev_data_init(void) {
    //上电读取掉电保存数据
    int len = DEV_SP_REG_NUMS * 2;
    int dev_name_len= 0;
    char *dev_name = (char *)dev_sp_input_reg_data;
    const struct fal_partition * dev_partition = fal_partition_find("sp_reg");
    fal_partition_read(dev_partition, 0, (uint8_t *)dev_sp_input_reg_data,  len);
    fal_partition_read(dev_partition, 1024, (uint8_t *)dev_sp_reg_data,  len);
    dev_name_len = strlen(dev_name);
    if(dev_name_len < 16) {
        if(strcmp(dev_name, DEV_NAME) != 0) {
            set_dev_name(dev_partition);
            fal_partition_read(dev_partition, 0, (uint8_t *)dev_sp_input_reg_data,  len);
        }
    } else {
        set_dev_name(dev_partition);
        fal_partition_read(dev_partition, 0, (uint8_t *)dev_sp_input_reg_data,  len);
    }
    return RT_EOK;
}
INIT_ENV_EXPORT(dev_data_init);


uint8_t dev_read_bit(uint8_t *data_addr) {
    if((data_addr >= dev_di_data) && (data_addr <= (&dev_di_data[DEV_X_BIT_NUMS -1]))) {      //获取DI的值
        *data_addr = io_get_di((int)(data_addr - dev_di_data));
    } else if((data_addr >= dev_do_data) && (data_addr <= (&dev_do_data[DEV_Y_BIT_NUMS -1]))) { //获取DO的值

        *data_addr = io_get_do((int)(data_addr - dev_do_data));
    }
    return *data_addr;
}


void dev_write_bit(uint8_t *data_addr, uint8_t data){
    if((data_addr >= dev_do_data) && (data_addr <= (&dev_do_data[DEV_Y_BIT_NUMS -1]))) { //写入do的值
        *data_addr = data;
        io_set_do((int)(data_addr - dev_do_data), *data_addr);
    }
}

int dev_read_bits(uint8_t *data_addr, int len, uint8_t *data) {
    uint8_t *data_run = data_addr;      //处理的数据指针
    int serial_run = 0;                 //处理的数据长度
    int len_re = len;                   //剩余需要处理的内容

    while(len_re > 0) {
        int nums_temp = 0;
        if((data_run >= dev_di_data) && (data_run <= (&dev_di_data[DEV_X_BIT_NUMS  -1]))) {      //获取DI的值
            //有这个数据
            if((data_run + len_re) <= (&dev_di_data[DEV_X_BIT_NUMS  -1])) {
                nums_temp = len_re;
            } else {
                nums_temp = (data_run + len_re) - (&dev_di_data[DEV_X_BIT_NUMS  -1]);
            }
            io_read_di((int)(data_run - dev_di_data), nums_temp, &data[serial_run]);
            serial_run += nums_temp;
            data_run += nums_temp;
            len_re -= nums_temp;
            continue;
        } else if((data_run >= dev_do_data) && (data_run <= (&dev_do_data[DEV_Y_BIT_NUMS -1]))) { //获取DO的值
            //有这个数据
            if((data_run + len_re) <= (&dev_do_data[DEV_Y_BIT_NUMS  -1])) {
               nums_temp = len_re;
            } else {
               nums_temp = (data_run + len_re) - (&dev_do_data[DEV_Y_BIT_NUMS  -1]);
            }
            io_read_do((int)(data_run - dev_do_data), nums_temp, &data[serial_run]);
            serial_run += nums_temp;
            data_run += nums_temp;
            len_re -= nums_temp;
            continue;
        } else {
            nums_temp = len_re;
            //其他数据，直接读取就可以，不需要做任何操作。
            serial_run += nums_temp;
            data_run += nums_temp;
            len_re -= nums_temp;
            continue;
        }
    }
    return 1;
}

int dev_write_bits(uint8_t *data_addr, int len, uint8_t *data) {
    uint8_t *data_run = data_addr;      //处理的数据指针
    int serial_run = 0;                 //处理的数据长度
    int len_re = len;                   //剩余需要处理的内容

    while(len_re > 0) {
        int nums_temp = 0;
        if((data_run >= dev_di_data) && (data_run <= (&dev_di_data[DEV_X_BIT_NUMS  -1]))) {      //获取DI的值
            //有这个数据
            if((data_run + len_re) <= (&dev_di_data[DEV_X_BIT_NUMS  -1])) {
                nums_temp = len_re;
            } else {
                nums_temp = (data_run + len_re) - (&dev_di_data[DEV_X_BIT_NUMS  -1]);
            }
            //不能写入，所以写入值无效，相当于没有执行, 这里放一条空语句，表示不允许写入DI
            ;
            serial_run += nums_temp;
            data_run += nums_temp;
            len_re -= nums_temp;
            continue;
        } else if((data_run >= dev_do_data) && (data_run <= (&dev_do_data[DEV_Y_BIT_NUMS -1]))) { //获取DO的值
            //有这个数据
            if((data_run + len_re) <= (&dev_do_data[DEV_Y_BIT_NUMS  -1])) {
               nums_temp = len_re;
            } else {
               nums_temp = (data_run + len_re) - (&dev_do_data[DEV_Y_BIT_NUMS  -1]);
            }
            io_write_do((int)(data_run - dev_do_data), nums_temp, &data[serial_run]);
            serial_run += nums_temp;
            data_run += nums_temp;
            len_re -= nums_temp;
            continue;
        }else {
            nums_temp = len_re;
            //其他数据，直接写入即可，不需要做任何操作
            memcpy(data_run, &data[serial_run], nums_temp);
            serial_run += nums_temp;
            data_run += nums_temp;
            len_re -= nums_temp;
            continue;
        }
    }
    return 1;
}



uint16_t dev_read_reg(uint16_t *data_addr) {
    return 0;
}

void dev_write_reg(uint16_t *data_addr, uint16_t data) {

}

int dev_read_regs(uint16_t *data_addr, int len, uint16_t *data) {
    memcpy(data, data_addr, len * 2);
    return 0;
}

int dev_write_regs(uint16_t *data_addr, int len, uint16_t *data) {
    memcpy(data_addr, data, len * 2);
    return 0;
}


void dev_data2modbus_slave(agile_modbus_slave_util_t *util)
{
    //普通寄存器绑定
    modbus_slave_add_val(util, CIOLS, DEV_BIT_ADDR, dev_bit_data, DEV_BIT_NUMS);
    modbus_slave_add_val(util, INPUTS, DEV_BIT_ADDR, dev_input_bit_data, DEV_BIT_NUMS);

    //do, di的寄存器绑定
    modbus_slave_add_val(util, CIOLS, DEV_Y_BIT_ADDR, dev_do_data, DEV_Y_BIT_ADDR);
    modbus_slave_add_val(util, INPUTS, DEV_X_BIT_ADDR, dev_di_data, DEV_X_BIT_NUMS);

    //普通寄存器绑定
    modbus_slave_add_val(util, REGISTERS, DEV_REG_ADDR, dev_reg_data, DEV_REG_NUMS);
    modbus_slave_add_val(util, INPUT_REGISTERS, DEV_REG_ADDR, dev_input_reg_data, DEV_REG_NUMS);

    //特殊寄存器绑定
    modbus_slave_add_val(util, REGISTERS, DEV_SP_REG_ADDR, dev_sp_reg_data, DEV_SP_REG_NUMS);
    modbus_slave_add_val(util, INPUT_REGISTERS, DEV_SP_REG_ADDR, dev_sp_input_reg_data, DEV_SP_REG_NUMS);
}


