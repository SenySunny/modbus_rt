#include "modbus_rt_platform_serial.h"

#if (MODBUS_RTU_SLAVE_ENABLE) || (MODBUS_RTU_MASTER_ENABLE)
#include "modbus_rt_platform_memory.h"
#include "modbus_rt_platform_thread.h"

static modbus_rt_serial_t *p_serial_info = NULL;

static int modbus_rt_serial_add_dev(struct sp_port *serial_port)
{
    int serial = 0;
    modbus_rt_serial_t *serial_port_temp = modbus_rt_malloc(sizeof(struct modbus_rt_serial));
    if(NULL == serial_port_temp) {
        return - MODBUS_RT_ENOMEM;
    }
    memset(serial_port_temp, 0,  sizeof(struct modbus_rt_serial));
    if(NULL == p_serial_info) {
        p_serial_info = serial_port_temp;
        serial++;
    } else {
        modbus_rt_serial_t *temp = p_serial_info;
        serial = temp->serial;
        while(NULL != temp->next) {
            temp = temp->next;
            serial = temp->serial;
        }
        temp->next = serial_port_temp;
        serial_port_temp->pre = temp;
        serial++;
    }
    serial_port_temp->serial_port = serial_port;
    serial_port_temp->serial = serial;
    return serial;
}

static struct sp_port * modbus_rt_serial_get_dev(int serial)
{
    modbus_rt_serial_t *serial_port_temp = p_serial_info;
    while( NULL != serial_port_temp) {
        if(serial == serial_port_temp->serial){
            return serial_port_temp->serial_port;
        } 
        serial_port_temp = serial_port_temp->next;
    }
    return NULL;
}

static void modbus_rt_serial_close_dev(int serial)
{
    modbus_rt_serial_t *serial_port_temp = p_serial_info;
    while( NULL != serial_port_temp) {
        if(serial == serial_port_temp->serial){
            break;
        } 
        serial_port_temp = serial_port_temp->next;
    }
    if(NULL == serial_port_temp) {
        return ;
    }
    struct sp_port *serial_port = serial_port_temp->serial_port;
    sp_close(serial_port);
    sp_free_port(serial_port);
    if(NULL == serial_port_temp->pre) {
        if(NULL == serial_port_temp->next) {
            p_serial_info = NULL;
            modbus_rt_free(serial_port_temp);
        } else {
            p_serial_info = serial_port_temp->next;
            modbus_rt_free(serial_port_temp);
        }
    } else if(NULL == serial_port_temp->next) {
        serial_port_temp->pre->next = NULL;
        modbus_rt_free(serial_port_temp);
    } else {
        serial_port_temp->pre->next = serial_port_temp->next;
        serial_port_temp->next->pre = serial_port_temp->pre;
        modbus_rt_free(serial_port_temp);
    }
}



int modbus_rt_serial_open(const char *devname, int baudrate, int bytesize, char parity, int stopbits, int xonxoff) {
    enum sp_return ret = SP_OK;
    struct sp_port *serial_port = NULL;
    
    ret = sp_get_port_by_name(devname, &serial_port);
    if(0 > ret) {
        return ret;
    }
    ret = sp_open(serial_port, SP_MODE_READ_WRITE);
    if(0 > ret) {
        return ret;
    }
    ret = sp_set_baudrate(serial_port, baudrate);
    if(0 > ret) {
        return ret;
    }
    ret = sp_set_bits(serial_port, bytesize);
    if(0 > ret) {
        return ret;
    }
    enum sp_parity parity_type =  SP_PARITY_INVALID;
    parity = toupper(parity);
    switch(parity) {
        case 'N': {
            parity_type =  SP_PARITY_NONE;
        } break;
        case 'E': {
            parity_type =  SP_PARITY_EVEN;  
        } break;
        case 'O': {
            parity_type =  SP_PARITY_ODD;  
        } break;
        case 'M': {
            parity_type =  SP_PARITY_MARK;
        } break;
        case 'S': {
           parity_type =  SP_PARITY_SPACE;
        } break;
        default : {
            parity_type =  SP_PARITY_NONE;
        } break;
    }
    ret = sp_set_parity(serial_port, parity_type);
    if(0 > ret) {
        return ret;
    }
    ret = sp_set_stopbits(serial_port, stopbits);
    if(0 == xonxoff) {
        ret = sp_set_flowcontrol(serial_port, SP_FLOWCONTROL_NONE);
    } else {
        ret = sp_set_flowcontrol(serial_port, SP_FLOWCONTROL_XONXOFF);
    }
    if(0 > ret) {
        return ret;
    }
    return modbus_rt_serial_add_dev(serial_port);
}

void modbus_rt_serial_close(int serial) {
    modbus_rt_serial_close_dev(serial);
}

void modbus_rt_serial_send(int serial, void *buf, int len) {
    struct sp_port *serial_port = modbus_rt_serial_get_dev(serial);
    sp_blocking_write(serial_port, buf, len, 0);
    sp_flush(serial_port,SP_BUF_BOTH);
}

int modbus_rt_serial_receive(int serial, void *buf, int bufsz, const int timeout, const int bytes_timeout) {
    struct sp_port *serial_port = modbus_rt_serial_get_dev(serial);
    uint8_t *buf_temp = (uint8_t *)buf;
    int len = 0;
    int len_recv = 0;
    int time = 0;
    while(timeout > time) {
        modbus_rt_thread_sleep(bytes_timeout);
        len_recv = sp_nonblocking_read(serial_port, buf_temp, bufsz);
        if(0 < len_recv){
            break;
        }
        time += bytes_timeout;
    }
    if(timeout <= time) {
        return -MODBUS_RT_ETIMEOUT;
    }
    
    len = len_recv;
    while(1) {
        modbus_rt_thread_sleep(bytes_timeout);
        len_recv = sp_nonblocking_read(serial_port, &buf_temp[len], bufsz - len);
        if(0 >= len_recv){
            return len;     //如果在一个bytes_timeout时间内没有数据，则表示读取数据结束
        } else {
            len += len_recv;
            if(bufsz <= len) {
                return -MODBUS_RT_EFULL;
            }
        }
    }
    return len;
}

#endif
