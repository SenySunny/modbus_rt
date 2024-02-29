/**
 * @file    modbus_slave.c
 * @brief   基于Agile Modbus 的应用层实现，slave寄存器处理函数
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */

#include "modbus_slave.h"

#if (MODBUS_RTU_SLAVE_ENABLE) || (MODBUS_TCP_SLAVE_ENABLE)
#include "modbus_rt_platform_memory.h"

#if SLAVE_DATA_DEVICE_BINDING
    #include "device_data.h"
#endif

/**
 * @brief   addr_check:                 modbus slave 在收到数据之后和应答数据之前的回调函数， 该函数主要作用用于slave地址的二次判断，
 *                                      以及用于做前期处理，这里可以用于协议转换函数使用，需要注意的是：转换函数的执行时间不能过长
 *                                      否则有可能会导致通信超时。
 * @param   slave_info:                 slave收到的数据信息结构体，内部存储发送端发送的地址，命令，数据等信息
 * @param   data                        slave的信息，即指向agile_modbus_slave_util_t的指针
 * @return  int:                        MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int addr_check(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info,
               const void *data) {
    (void)(ctx);
    int slave = slave_info->sft->slave;
    int function = slave_info->sft->function;
    int address = slave_info->address;
    int nb = slave_info->nb;
    const agile_modbus_slave_util_t *slave_util = (const agile_modbus_slave_util_t *)data;

//    if ((slave != ctx->slave) && (slave != AGILE_MODBUS_BROADCAST_ADDRESS) && (slave != 0xFF))
//        return -AGILE_MODBUS_EXCEPTION_UNKNOW;
    if(NULL != slave_util ->pre_ans_callback)
    {
        slave_util ->pre_ans_callback(ctx, slave, function, address, nb);
    }
    return MODBUS_RT_EOK;
}

/**
 * @brief   slave_done:                 modbus slave 在收到数据并处理完成之后的回调函数，该函数主要作用用于slave地址的二次判断，
 *                                      以及用于做后期处理，这里可以用于协议转换函数使用。需要注意的是：该函数执行的时候，并没有应答
 *                                      数据给master设备，所以转换函数的执行时间不能过长，否则有可能会导致master端提示超时，如果需要
 *                                      处理的事情较多，可以采用信号量的处理方式发送信号，在其他的线程中处理数据，以确保不会超时。
 * @param   slave_info:                 slave收到的数据信息结构体，内部存储发送端发送的地址，命令，数据等信息
 * @param   data                        slave的信息，即指向agile_modbus_slave_util_t的指针
 * @param   ret                         slave应答数据之后的返回值
 * @return  int:                        MODBUS_RT_EOK：成功，其他：失败
 *       
 */
int slave_done(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info,
               const void *data, int ret) {
    (void)(ctx);
    (void)(ret);
    // (void)(slave_info);
    int slave = slave_info->sft->slave;
    int function = slave_info->sft->function;
    int address = slave_info->address;
    int nb = slave_info->nb;
    const agile_modbus_slave_util_t *slave_util = (const agile_modbus_slave_util_t *)data;

//    if ((slave != ctx->slave) && (slave != AGILE_MODBUS_BROADCAST_ADDRESS) && (slave != 0xFF))
//        return -AGILE_MODBUS_EXCEPTION_UNKNOW;
    if(NULL != slave_util ->done_callback)
    {
        slave_util ->done_callback(ctx, slave,function, address,nb);
    }
    return MODBUS_RT_EOK;
}

/**
 * @brief   modbus_slave_insert_val:            把数据信息加载到slave的寄存器链表当中，详细信息查看
 *                                              modbus_slave_add_val函数，该函数主要被modbus_slave_add_val调用
 * @param   map_t:                              slave的寄存器链表
 * @param   util_tab：                          需要加载到链表当中的数据
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_slave_insert_val(agile_modbus_slave_util_map_t *map_t, agile_modbus_slave_util_map_t *util_tab) {
    while(util_tab != NULL) {
        //地址如果 存在重复，则直接报错
        if(((map_t->start_addr < util_tab->start_addr) &&
                ((map_t->start_addr + map_t->len) >  util_tab->start_addr)) ||
                ((map_t->start_addr >= util_tab->start_addr) &&
                (map_t->start_addr < (util_tab->start_addr + util_tab->len)))) {
            return -MODBUS_RT_ERROR;
        } else if(map_t->start_addr < util_tab->start_addr) {
            if(util_tab->pre != NULL) {
                map_t->pre = util_tab->pre;
                util_tab->pre->next = map_t;
            }
            map_t->next = util_tab;
            util_tab->pre = map_t;
            return MODBUS_RT_EOK;
        } else if(util_tab->next == NULL) {
            util_tab->next = map_t;
            map_t->pre = util_tab;
            return MODBUS_RT_EOK;
        } else {
            util_tab = util_tab->next;
        }
    }
    return -MODBUS_RT_ERROR;   //前面没有返回，表示util_tab本身就是NULL
}

/**
 * @brief   modbus_slave_write_bits:            写入BIT型寄存器，需要注意的是，如果设备本身有硬件信息绑定到
 *                                              slave的寄存器地址，可以在这里调用操作硬件相关的函数，这样便可以
 *                                              实现master端或者slave端的excuse函数直接操作设备硬件。可以参考注释的函数
 *                                              dev_write_bits，这个函数用户自己扩展，详情可以参考rt-thread的例程
 * @param   data_addr:                          指向需要操作的数据，即寄存器存储空间的指针。
 * @param   len:                                数据长度(指寄存器长度，非数据长度)
 * @param   data:                               需要写入的数据内容，即modbus处理之后的数据
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_slave_write_bits(void *data_addr, int len, void *data) {
    uint8_t *data_addr_temp = (uint8_t *)data_addr;
    uint8_t *data_temp = (uint8_t *)data;
#if SLAVE_DATA_DEVICE_BINDING    
    return dev_write_bits(data_addr_temp, len, data_temp);
#else
    memcpy(data_addr_temp, data_temp, len);
    return MODBUS_RT_EOK;
#endif    
}

/**
 * @brief   modbus_slave_read_bits:             读取BIT型寄存器，需要注意的是，如果设备本身有硬件信息绑定到
 *                                              slave的寄存器地址，可以在这里调用操作硬件相关的函数。
 * @param   data_addr:                          指向需要操作的数据，即寄存器存储空间的指针。
 * @param   len:                                数据长度(指寄存器长度，非数据长度)
 * @param   data:                               需要读取的数据内容空间，即需要把数据读取到data中以便slave设备做应答
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_slave_read_bits(void *data_addr, int len, void *data) {
    uint8_t *data_addr_temp = (uint8_t *)data_addr;
    uint8_t *data_temp = (uint8_t *)data;
#if SLAVE_DATA_DEVICE_BINDING 
    return dev_read_bits(data_addr_temp, len, data_temp);
#else
    memcpy(data_temp, data_addr_temp, len);
    return MODBUS_RT_EOK;
#endif
}

/**
 * @brief   modbus_slave_write_bits:            写入reg型寄存器(uint16_t)，需要注意的是，如果设备本身有硬件信息绑定到
 *                                              slave的寄存器地址，可以在这里调用操作硬件相关的函数，这样便可以
 *                                              实现master端或者slave端的excuse函数直接操作设备硬件。
 * @param   data_addr:                          指向需要操作的数据，即寄存器存储空间的指针。
 * @param   len:                                数据长度(指寄存器长度，非数据长度)
 * @param   data:                               需要写入的数据内容，即modbus处理之后的数据
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_slave_write_regs(void *data_addr, int len, void *data) {
    uint16_t *data_addr_temp = (uint16_t *)data_addr;
    uint16_t *data_temp = (uint16_t *)data;
#if SLAVE_DATA_DEVICE_BINDING 
    return dev_write_regs(data_addr_temp, len, data_temp);
#else
    memcpy(data_addr_temp, data_temp, len * 2);
    return MODBUS_RT_EOK;
#endif
}

/**
 * @brief   modbus_slave_read_regs:             写入reg型寄存器(uint16_t)，需要注意的是，如果设备本身有硬件信息绑定到
 *                                              slave的寄存器地址，可以在这里调用操作硬件相关的函数。
 * @param   data_addr:                          指向需要操作的数据，即寄存器存储空间的指针。
 * @param   len:                                数据长度(指寄存器长度，非数据长度)
 * @param   data:                               需要读取的数据内容空间，即需要把数据读取到data中以便slave设备做应答
 * @return  int:                                MODBUS_RT_EOK：成功，其他：失败
 *       
 */
static int modbus_slave_read_regs(void *data_addr, int len, void *data) {
    uint16_t *data_addr_temp = (uint16_t *)data_addr;
    uint16_t *data_temp = (uint16_t *)data;
#if SLAVE_DATA_DEVICE_BINDING 
    return dev_read_regs(data_addr_temp, len, data_temp);
#else
    memcpy(data_temp, data_addr_temp, len * 2);
    return MODBUS_RT_EOK;
#endif
}

/**
 * @brief   modbus_slave_util_init:             slave设备的util数据初始化，需要注意的是：如果设备本身有硬件信息绑定到
 *                                              slave的寄存器地址，可以在这里进行绑定操作。
 * @param   util:                               指向slave设备的util数据
 * @return  None
 *       
 */
void modbus_slave_util_init(agile_modbus_slave_util_t *util) {
    if(util != NULL) {
        util->tab_bits = NULL;
        util->tab_bits_tail = NULL;
        util->tab_input_bits = NULL;
        util->tab_input_bits_tail = NULL;
        util->tab_input_registers = NULL;
        util->tab_input_registers_tail = NULL;
        util->tab_registers = NULL;
        util->tab_registers_tail = NULL;
        util->addr_check = addr_check;
        util->special_function = NULL;
        util->done = slave_done;
        util->pre_ans_callback = NULL;
        util->done_callback = NULL;
    }
#if SLAVE_DATA_DEVICE_BINDING 
    dev_data2modbus_slave(util);
#endif
}

/**
 * @brief   modbus_slave_add_val:           增加绑定数据寄存器
 * @param   util:                           需要绑定在哪个modbus寄存器上
 * @param   type:                           需要绑定的数据属于modbus哪个寄存器
 * @param   data_addr:                      需要绑定到哪个地址上
 * @param   data:                           数据内容指针，注意该数据必须式全局可以访问的数据，否则可能导致错误
 * @param   nums:                           数据产长度
 * @return  int :                           绑定成功或者失败
 */
int modbus_slave_add_val(agile_modbus_slave_util_t *util, modbus_register_type_t type,
        int data_addr, void *data, int nums) {
    if(util == NULL) {
        return -MODBUS_RT_ERROR;
    }
    agile_modbus_slave_util_map_t *map_t = modbus_rt_malloc(sizeof(struct agile_modbus_slave_util_map));
    if(map_t == NULL) {
        return -MODBUS_RT_ERROR;
    }
    map_t->start_addr = data_addr;
    map_t->data = data;
    map_t->len = nums;
    map_t->pre = NULL;
    map_t->next = NULL;
    if(type == CIOLS) {
        map_t->get = modbus_slave_read_bits;
        map_t->set = modbus_slave_write_bits;
        if(util->tab_bits == NULL) {            //头部第一个数据
            util->tab_bits = map_t;
            util->tab_bits_tail = map_t;
            return  MODBUS_RT_EOK;
        } else if(modbus_slave_insert_val(map_t, util->tab_bits) != MODBUS_RT_EOK) { //判断是否有效,如果无效，则直接返回
            modbus_rt_free(map_t);
            return - MODBUS_RT_ERROR;
        }
        if((map_t->pre == NULL) && (map_t->next == util->tab_bits)) {
            util->tab_bits = map_t;
        } else if(map_t->next == NULL) {
            util->tab_bits_tail = map_t;
        }
        return  MODBUS_RT_EOK;
    } else if(type == INPUTS) {
        map_t->get = modbus_slave_read_bits;
        map_t->set = modbus_slave_write_bits;
        if(util->tab_input_bits == NULL) {            //头部第一个数据
            util->tab_input_bits = map_t;
            util->tab_input_bits_tail = map_t;
            return  MODBUS_RT_EOK;
        } else if(modbus_slave_insert_val(map_t, util->tab_input_bits) != MODBUS_RT_EOK) {  //判断是否有效,如果无效，则直接返回
            modbus_rt_free(map_t);
            return - MODBUS_RT_ERROR;
        }
        if((map_t->pre == NULL) && (map_t->next == util->tab_input_bits)){
           util->tab_input_bits = map_t;
        } else if(map_t->next == NULL) {
            util->tab_input_bits_tail = map_t;
        }
        return  MODBUS_RT_EOK;
    } else if(type == INPUT_REGISTERS) {
        map_t->get = modbus_slave_read_regs;
        map_t->set = modbus_slave_write_regs;
        if(util->tab_input_registers == NULL) {            //头部第一个数据
            util->tab_input_registers = map_t;
            util->tab_input_registers_tail = map_t;
            return  MODBUS_RT_EOK;
        } else if(modbus_slave_insert_val(map_t, util->tab_input_registers) != MODBUS_RT_EOK) {//判断是否有效,如果无效，则直接返回
            modbus_rt_free(map_t);
            return - MODBUS_RT_ERROR;
        }
        if((map_t->pre == NULL) && (map_t->next == util->tab_input_registers)) {
           util->tab_input_registers = map_t;
        } else if(map_t->next == NULL) {
            util->tab_input_registers_tail = map_t;
        }
        return  MODBUS_RT_EOK;
    } else if(type == REGISTERS) {
        map_t->get = modbus_slave_read_regs;
        map_t->set = modbus_slave_write_regs;
        if(util->tab_registers == NULL) {       //头部第一个数据
            util->tab_registers = map_t;
            util->tab_registers_tail = map_t;
            return  MODBUS_RT_EOK;
        } else if(modbus_slave_insert_val(map_t, util->tab_registers) != MODBUS_RT_EOK) { //判断是否有效,如果无效，则直接返回
            modbus_rt_free(map_t);
            return - MODBUS_RT_ERROR;
        }
        if((map_t->pre == NULL) && (map_t->next == util->tab_registers)) {
           util->tab_registers = map_t;
        } else if(map_t->next == NULL) {
            util->tab_registers_tail = map_t;
        }
        return  MODBUS_RT_EOK;
    }
    return - MODBUS_RT_ERROR;
}

/**
 * @brief   modbus_slave_clear_val:         清除绑定数据寄存器
 * @param   util:                           需要绑定在哪个modbus寄存器上
 * @return  int :                           清除成功或者失败
 */
int modbus_slave_clear_val(agile_modbus_slave_util_t *util) {
    //清除绑定CIOLS寄存器
    agile_modbus_slave_util_map_t *maps = util->tab_bits_tail;
    if(maps != NULL) {
        while(maps->pre != NULL) {
            agile_modbus_slave_util_map_t *maps_pre = maps->pre;
            modbus_rt_free(maps);
            maps = maps_pre;
        }
        modbus_rt_free(maps);
        util->tab_bits = NULL;
        util->tab_bits_tail = NULL;
    }
    //清除绑定INPUTS寄存器
    maps = util->tab_input_bits_tail;
    if(maps != NULL) {
        while(maps->pre != NULL) {
            agile_modbus_slave_util_map_t *maps_pre = maps->pre;
            modbus_rt_free(maps);
            maps = maps_pre;
        }
        modbus_rt_free(maps);
        util->tab_input_bits = NULL;
        util->tab_input_bits_tail = NULL;
    }
    //清除绑定INPUT_REGISTERS寄存器
    maps = util->tab_input_registers_tail;
    if(maps != NULL) {
        while(maps->pre != NULL) {
            agile_modbus_slave_util_map_t *maps_pre = maps->pre;
            modbus_rt_free(maps);
            maps = maps_pre;
        }
        modbus_rt_free(maps);
        util->tab_input_registers = NULL;
        util->tab_input_registers_tail = NULL;
    }
    //清除绑定REGISTERS寄存器
    maps = util->tab_registers_tail;
    if(maps != NULL) {
        while(maps->pre != NULL) {
            agile_modbus_slave_util_map_t *maps_pre = maps->pre;
            modbus_rt_free(maps);
            maps = maps_pre;
        }
        modbus_rt_free(maps);
        util->tab_registers = NULL;
        util->tab_registers_tail = NULL;
    }
    return MODBUS_RT_EOK;
}


/**
 * @brief   modbus_slave_read:          读取寄存器的值
 * @param   util:                       需要绑定在哪个modbus寄存器上
 * @param   type:                       需要操作的寄存器类型
 * @param   addr:                       寄存器地址
 * @param   quantity:                   读取寄存器的长度
 * @param   ptr_data:                   存储寄存器的数值
 * @return  int :                       读取成功或者失败
 */
int modbus_slave_read(agile_modbus_slave_util_t *util, modbus_register_type_t type, int addr, int quantity, void *ptr_data) {
    int ret = -MODBUS_RT_ERROR;
    int address = addr;
    int nb = quantity;
    agile_modbus_slave_util_map_t *maps = NULL;
    if((0 == quantity) || (NULL == ptr_data)) {
        ret = -MODBUS_RT_ERROR;
        return ret;
    }
    switch (type) {
        case CIOLS: {
            maps = util->tab_bits;
        } break;
        case INPUTS: {
            maps = util->tab_input_bits;
        } break;
        case INPUT_REGISTERS: {
            maps = util->tab_input_registers;
        } break;
        case REGISTERS: {
            maps = util->tab_registers;
        } break;
    }
    for (int now_address = address, i = 0; now_address < address + nb; now_address++, i++) {
        const agile_modbus_slave_util_map_t *map = get_map_by_addr(maps, now_address);
        if (map == NULL) {
            continue;
        }
        int map_len = map->start_addr + map->len - now_address;
        if (map->get) {
            int index = now_address - map->start_addr;
            int need_len = address + nb - now_address;
            if (need_len > map_len) {
                need_len = map_len;
            }
            if (type == CIOLS || type == INPUTS) {
                uint8_t *p = (uint8_t *)ptr_data;
                map->get(((uint8_t *)map->data) + index, need_len, &(p[now_address - address]));
            } else {
                uint16_t *p = (uint16_t *)ptr_data;
                map->get(((uint16_t *)map->data) + index, need_len, &(p[now_address - address]));
            }
        }
        now_address += map_len - 1;
        i += map_len - 1;
    }
    return  MODBUS_RT_EOK;
}


/**
 * @brief   modbus_slave_write:         写入寄存器的值
 * @param   util:                       需要绑定在哪个modbus寄存器上
 * @param   type:                       需要操作的寄存器类型
 * @param   addr:                       寄存器地址
 * @param   quantity:                   写入寄存器的长度
 * @param   ptr_data:                   需要写入的数值
 * @return  int :                       写入成功或者失败
 */
int modbus_slave_write(agile_modbus_slave_util_t *util, modbus_register_type_t type, int addr, int quantity, void *ptr_data) {
    int ret = -MODBUS_RT_ERROR;
    int address = addr;
    int nb = quantity;
    agile_modbus_slave_util_map_t *maps = NULL;
    if((0 == quantity) || (NULL == ptr_data)) {
        ret = -MODBUS_RT_ERROR;
        return ret;
    }
    switch (type) {
        case CIOLS: {
            maps = util->tab_bits;
        } break;
        case INPUTS: {
            maps = util->tab_input_bits;
        } break;
        case INPUT_REGISTERS: {
            maps = util->tab_input_registers;
        } break;
        case REGISTERS: {
            maps = util->tab_registers;
        } break;
    }
    for (int now_address = address, i = 0; now_address < address + nb; now_address++, i++) {
        const agile_modbus_slave_util_map_t *map = get_map_by_addr(maps, now_address);
        if (map == NULL) {
            continue;
        }
        int map_len = map->start_addr + map->len - now_address;
        if (map->set) {
            int index = now_address - map->start_addr;
            int need_len = address + nb - now_address;
            if (need_len > map_len) {
                need_len = map_len;
            }
            if (type == CIOLS || type == INPUTS) {
                uint8_t *p = (uint8_t *)ptr_data;
                map->set(((uint8_t *)map->data) + index, need_len, &(p[now_address - address]));
            } else {
                uint16_t *p = (uint16_t *)ptr_data;
                map->set(((uint16_t *)map->data) + index, need_len, &(p[now_address - address]));
            }
        }
        now_address += map_len - 1;
        i += map_len - 1;
    }
    return  MODBUS_RT_EOK;
}

#endif
