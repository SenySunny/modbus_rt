/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-09     SenyPC       the first version
 */
#include "dido.h"
#include "device_data.h"

static rt_base_t _io_di_pin[DEV_INNER_DI_NUMS] = {0};

static rt_base_t _io_do_pin[DEV_INNER_DO_NUMS] = {0};

static int io_init( void ) {
    _io_di_pin[0] = GET_PIN(A, 0);
    _io_di_pin[1] = GET_PIN(G, 2);
    _io_di_pin[2] = GET_PIN(C, 13);
    _io_di_pin[3] = GET_PIN(G, 3);
    _io_di_pin[4] = GET_PIN(G, 4);
    _io_di_pin[5] = GET_PIN(H, 2);
    _io_di_pin[6] = GET_PIN(H, 3);
    _io_di_pin[7] = GET_PIN(H, 8);
    _io_di_pin[8] = GET_PIN(F, 12);
    _io_di_pin[9] = GET_PIN(F, 13);
    _io_di_pin[10] = GET_PIN(F, 14);
    _io_di_pin[11] = GET_PIN(F, 15);
    _io_di_pin[12] = GET_PIN(G, 0);
    _io_di_pin[13] = GET_PIN(G, 1);
    _io_di_pin[14] = GET_PIN(D, 11);
    _io_di_pin[15] = GET_PIN(G, 5);
    _io_di_pin[16] = GET_PIN(D, 7);

    _io_do_pin[0] = GET_PIN(A, 15);
    _io_do_pin[1] = GET_PIN(E, 2);
    _io_do_pin[2] = GET_PIN(G, 15);
    _io_do_pin[3] = GET_PIN(B, 8);
    _io_do_pin[4] = GET_PIN(A, 5);
    _io_do_pin[5] = GET_PIN(C, 12);
    _io_do_pin[6] = GET_PIN(H, 7);
    _io_do_pin[7] = GET_PIN(D, 3);

    for(int i = 0; i < DEV_INNER_DI_NUMS; i++)
    {
        rt_pin_mode(_io_di_pin[i], PIN_MODE_INPUT);
    }
    for(int i = 0; i < DEV_INNER_DO_NUMS; i++)
    {
        rt_pin_mode(_io_do_pin[i], PIN_MODE_OUTPUT);
    }

    return RT_EOK;
}
INIT_BOARD_EXPORT(io_init);

uint8_t io_get_di(int pin) {
    if((pin >= 0) || (pin < DEV_INNER_DI_NUMS)) {
        return rt_pin_read(_io_di_pin[pin]);
    } else if(pin < DEV_X_BIT_NUMS) {
        return dev_di_data[pin];
    }
    return 0;
}


uint8_t io_get_do(int pin) {
    if((pin >= 0) || (pin < DEV_INNER_DO_NUMS)) {
        return rt_pin_read(_io_do_pin[pin]);
    } else if(pin < DEV_Y_BIT_NUMS) {
        return dev_do_data[pin];
    }
    return 0;

}

void io_set_do(int pin, rt_base_t value) {
    if((pin >= 0) || (pin < DEV_INNER_DO_NUMS)) {
        rt_pin_write(_io_do_pin[pin], value);
    } else if(pin < DEV_Y_BIT_NUMS) {
        dev_do_data[pin] = value;
    }
}


int io_read_di(int start_addr, int nums, uint8_t *val) {
    int i = 0;
    int end_addr = start_addr + nums;
    if((start_addr >= 0) || (start_addr < DEV_INNER_DI_NUMS)) {
        if(end_addr <= DEV_INNER_DI_NUMS) {
            for(i = 0; i < nums; i++) {
                val[i] = io_get_di(start_addr + i);
            }
        } else if(start_addr < DEV_INNER_DI_NUMS) {
            int inner_nums = DEV_INNER_DI_NUMS -start_addr;
            for(i = 0; i < inner_nums; i++) {
                val[i] = io_get_di(start_addr + i);
            }
            if(end_addr <= DEV_X_BIT_NUMS) {
                memcpy(&val[inner_nums], &dev_di_data[inner_nums], end_addr - inner_nums);
            } else {
                memcpy(&val[inner_nums], &dev_di_data[inner_nums], DEV_X_BIT_NUMS - inner_nums);
            }
        }
        return 1;
    } else if(start_addr < DEV_X_BIT_ADDR) {
        if(end_addr <= DEV_X_BIT_NUMS) {
            memcpy(&val[0], &dev_di_data[start_addr], end_addr - start_addr);
        } else {
            memcpy(&val[0], &dev_di_data[start_addr], DEV_X_BIT_NUMS - start_addr);
        }
        return 1;
    }
    return 0;
}

int io_read_do(int start_addr, int nums, uint8_t *val) {
    int i = 0;
    int end_addr = start_addr + nums;
    if((start_addr >= 0) || (start_addr < DEV_INNER_DO_NUMS)) {
        if(end_addr <= DEV_INNER_DO_NUMS) {
            for(i = 0; i < nums; i++) {
                val[i] = io_get_do(start_addr + i);
            }
        } else if(start_addr < DEV_INNER_DO_NUMS) {
            int inner_nums = DEV_INNER_DO_NUMS -start_addr;
            for(i = 0; i < inner_nums; i++) {
                val[i] = io_get_do(start_addr + i);
            }
            if(end_addr <= DEV_Y_BIT_NUMS) {
                memcpy(&val[inner_nums], &dev_do_data[inner_nums], end_addr - inner_nums);
            } else {
                memcpy(&val[inner_nums], &dev_do_data[inner_nums], DEV_Y_BIT_NUMS - inner_nums);
            }
        }
        return 1;
    } else if(start_addr < DEV_X_BIT_ADDR) {
        if(end_addr <= DEV_Y_BIT_NUMS) {
            memcpy(&val[0], &dev_do_data[start_addr], end_addr - start_addr);
        } else {
            memcpy(&val[0], &dev_do_data[start_addr], DEV_Y_BIT_NUMS - start_addr);
        }
        return 1;
    }
    return 0;
}

int io_write_do(int start_addr, int nums, uint8_t *val) {
    int i = 0;
    int end_addr = start_addr + nums;
    if((start_addr >= 0) || (start_addr < DEV_INNER_DO_NUMS)) {
        if(end_addr <= DEV_INNER_DO_NUMS) {
            for(i = 0; i < nums; i++) {
                io_set_do(start_addr + i, val[i]);
            }
        } else if(start_addr < DEV_INNER_DO_NUMS) {
            int inner_nums = DEV_INNER_DO_NUMS -start_addr;
            for(i = 0; i < inner_nums; i++) {
                io_set_do(start_addr + i, val[i]);
            }
            if(end_addr <= DEV_Y_BIT_NUMS) {
                memcpy(&dev_do_data[inner_nums], &val[inner_nums], end_addr - inner_nums);
            } else {
                memcpy(&dev_do_data[inner_nums], &val[inner_nums], DEV_Y_BIT_NUMS - inner_nums);
            }
        }
        return 1;
    } else if(start_addr < DEV_X_BIT_ADDR) {
        if(end_addr <= DEV_Y_BIT_NUMS) {
            memcpy(&dev_do_data[start_addr], &val[0], end_addr - start_addr);
        } else {
            memcpy(&dev_do_data[start_addr], &val[0], DEV_Y_BIT_NUMS - start_addr);
        }
        return 1;
    }
    return 0;
}
