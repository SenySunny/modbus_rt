#include <stdio.h>
#include "modbus_rtu.h"
#include "modbus_tcp.h"

#define    SERIAL_NAME      "COM10"
#define    IP_ADDR          "192.168.28.150"

rtu_modbus_device_t rs = NULL;
tcp_modbus_device_t ts = NULL;
tcp_modbus_device_t tsu = NULL;
uint16_t reg_temp[10] = {1,2,3,4,5,6,7,8,9,10};

int rtu_slave_done_callback(agile_modbus_t *ctx, int slave, int function,int addr, int quantity) {
   (void)(ctx);
    printf("rtu_slave done. addr: %d, function: %d, addr: %d, quantity: %d.\n", slave, function, addr, quantity);
    return 0;
}

int tcp_slave_done_callback(agile_modbus_t *ctx, int slave, int function,int addr, int quantity) {
   (void)(ctx);
    printf("tcp_slave done. addr: %d, function: %d, addr: %d, quantity: %d.\n", slave, function, addr, quantity);
    return 0;
}

int udp_slave_done_callback(agile_modbus_t *ctx, int slave, int function,int addr, int quantity) {
   (void)(ctx);
    printf("udp_slave done. addr: %d, function: %d, addr: %d, quantity: %d.\n", slave, function, addr, quantity);
    return 0;
}

int modbus_rtu_slave_open_test( void ) {
    int ret = MODBUS_RT_EOK;
    if(NULL == (rs = modbus_rtu(MODBUS_SLAVE))) {
        printf("modbus_rtu_slave create error.\n");
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_rtu_set_serial(rs, SERIAL_NAME, 115200, 8, 'N', 1, 0))) {
        printf("modbus_rtu_set_serial error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_rtu_add_block(rs, REGISTERS,0,reg_temp, 10))) {
        printf("modbus_rtu_add_block error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_rtu_set_done_callback(rs,rtu_slave_done_callback))) {
        printf("modbus_rtu_set_done_callback error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_rtu_open(rs))) {
        printf("modbus_rtu_open error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    return ret;
}


int modbus_tcp_slave_open_test( void ) {
    int ret = MODBUS_RT_EOK;
    if(NULL == (ts = modbus_tcp(MODBUS_SLAVE))) {
        printf("modbus_tcp_slave create error.\n");
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_set_net(ts, IP_ADDR, 502, SOCK_STREAM))) {
        printf("modbus_tcp_set_net error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_add_block(ts, REGISTERS,0,reg_temp, 10))) {
        printf("modbus_tcp_add_block error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_set_done_callback(ts,tcp_slave_done_callback))) {
        printf("modbus_tcp_set_done_callback error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_open(ts))) {
        printf("modbus_tcp_open error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    return ret;
}

int modbus_tcp_slave_for_udp_open_test( void ) {
    int ret = MODBUS_RT_EOK;
    if(NULL == (tsu = modbus_tcp(MODBUS_SLAVE))) {
        printf("modbus_tcp_slave_for_udp create error.\n");
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_set_net(tsu, IP_ADDR, 502, SOCK_DGRAM))) {
        printf("modbus_tcp_set_net error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_add_block(tsu, REGISTERS,0,reg_temp, 10))) {
        printf("modbus_tcp_add_block error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_set_done_callback(tsu,udp_slave_done_callback))) {
        printf("modbus_tcp_set_done_callback error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    if(MODBUS_RT_EOK != (ret = modbus_tcp_open(tsu))) {
        printf("modbus_tcp_open error, code is: %d.\n", ret);
        return -MODBUS_RT_ERROR;
    }
    return ret;
}

int main(int argc, char *argv[]){
    int ret = MODBUS_RT_EOK;
    (void)(argc);
    (void)(argv);
    if(MODBUS_RT_EOK != (ret = modbus_rtu_slave_open_test())) {
        return ret;
    }
    printf("modbus_rtu_slave_open success.\n");
    if(MODBUS_RT_EOK != (ret = modbus_tcp_slave_open_test())) {
        return ret;  
    }
    printf("modbus_tcp_slave_open success.\n");
    if(MODBUS_RT_EOK != (ret = modbus_tcp_slave_for_udp_open_test())) {
        return ret;
    }
    printf("modbus_tcp_slave_for_udp_open success.\n");
    while(1) {
        modbus_rt_thread_sleep(1000);
    }
    return 0;
}
