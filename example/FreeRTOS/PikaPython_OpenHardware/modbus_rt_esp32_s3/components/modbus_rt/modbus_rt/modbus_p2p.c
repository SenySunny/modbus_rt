/**
 * @file    modbus_p2p.c
 * @brief   基于Agile Modbus 的p2p应用层实现，即文件传输，包含slave和master
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#include "modbus_p2p.h"

#if  MODBUS_P2P_ENABLE

/**
 * @brief   modbus p2p slave结构体
 */
typedef struct modbus_p2p_slave_info{
        int fd;                               //文件操作句柄
        modbus_rt_file_info_t file_info;        //文件信息
        uint32_t write_file_size;                    //已经发送的文件大小
}modbus_p2p_slave_info_t;

/**
 @verbatim
    p2p接收文件，采用的全局变量操作，意味着，每个时刻只允许一个线程用于接收文件。
    如果同时多个modbus slave对象同时接收文件，会导致系统错误，这部分问题暂时没有考虑。
    需要套在使用时特别注意。
 @endverbatim
 */
modbus_p2p_slave_info_t g_modbus_p2p_slave_info;

/**
 * @brief   从机特殊指令的回调函数，在agile_modbus_slave_util_callback中调用
 * @param   ctx modbus 句柄
 * @param   slave_info 从机信息体
 * @param   data 私有数据
 * @return  =0:正常;
 *          <0:异常
 *             (-AGILE_MODBUS_EXCEPTION_UNKNOW(-255): 未知异常，从机不会打包响应数据)
 *             (其他负数异常码: 从机会打包异常响应数据)
 */
int modbus_slave_special_callback(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info)
{
    int function = slave_info->sft->function;
    int ret = MODBUS_RT_EOK;

    switch (function) {
#if MODBUS_P2P_SEND_ENABLE
        case AGILE_MODBUS_FC_TRANS_FILE: {      //发送文件
            modbus_p2p_slave_info_t *p2p_info = &g_modbus_p2p_slave_info;

            int send_index = slave_info->send_index;        //应答数据的数据指引 
//            int data_len = slave_info->nb;                  //
            uint8_t *data_ptr = slave_info->buf;
            int cmd = (data_ptr[0] << 8) + data_ptr[1];
            int cmd_data_len = (data_ptr[2] << 8) + data_ptr[3];
            uint8_t *cmd_data_ptr = data_ptr + 4;
            switch(cmd) {
                case TRANS_FILE_CMD_START: {        //传输文件信息
                    if(0 < p2p_info->fd) {
                        modbus_rt_file_close(p2p_info->fd);
                        p2p_info->fd = 0;
                    }
                    p2p_info->write_file_size  = 0;
                    p2p_info->fd = modbus_rt_file_write_info(cmd_data_ptr, cmd_data_len, &(p2p_info->file_info));
                    if(0 >= p2p_info->fd) {
                        ret = -MODBUS_RT_ERROR;
                        break;
                    }
                }break;
                case TRANS_FILE_CMD_DATA: {         //传输文件内容
                    if(0 >= p2p_info->fd) {
                        ret = -MODBUS_RT_EEMPTY;
                        break;
                    }
                    if(0 >= cmd_data_len) {
                        ret = -MODBUS_RT_ERROR;
                        break;
                    }
                    int flag = cmd_data_ptr[0];
                    int file_len = cmd_data_len - 1;
                    if(0 < file_len) {
                        ret = modbus_rt_file_write_file(p2p_info->fd, cmd_data_ptr + 1, file_len);
                        if(0 >= ret) {
                            ret = -MODBUS_RT_ERROR;
                            break;
                        } else {
                            ret = MODBUS_RT_EOK;
                        }
                    }
                    p2p_info->write_file_size += file_len;
                    if (TRANS_FILE_FLAG_END == flag) {
                        modbus_rt_file_close(p2p_info->fd);
                        p2p_info->fd = 0;
                        if(p2p_info->write_file_size != p2p_info->file_info.file_size) {
                            ret = -MODBUS_RT_ERROR;
                            break;
                        }
                    }
                }break;
            }
            ctx->send_buf[send_index++] = data_ptr[0];
            ctx->send_buf[send_index++] = data_ptr[1];
            ctx->send_buf[send_index++] = (ret == 0) ? 0x01 : 0x00;
            *(slave_info->rsp_length) = send_index;
        } break;
#endif
#if MODBUS_P2P_RECV_ENABLE
        case AGILE_MODBUS_FC_READ_FILE: {       //接收文件
            modbus_p2p_slave_info_t *p2p_info = &g_modbus_p2p_slave_info;

            int send_index = slave_info->send_index;        //应答数据的数据指引 
//            int data_len = slave_info->nb;                  //
            uint8_t *data_ptr = slave_info->buf;
            int cmd = (data_ptr[0] << 8) + data_ptr[1];
            int cmd_data_len = (data_ptr[2] << 8) + data_ptr[3];
            uint8_t *cmd_data_ptr = data_ptr + 4;
            switch(cmd) {
                case TRANS_FILE_CMD_START: {        //传输文件信息
                    if(0 < p2p_info->fd) {
                        modbus_rt_file_close(p2p_info->fd);
                        p2p_info->fd = 0;
                    }
                    p2p_info->write_file_size  = 0;
                    p2p_info->fd = modbus_rt_file_read_info(cmd_data_ptr, cmd_data_len, &(p2p_info->file_info));
                    if(0 >= p2p_info->fd) {      //返回文件打开失败
                        ctx->send_buf[send_index++] = data_ptr[0];
                        ctx->send_buf[send_index++] = data_ptr[1];
                        ctx->send_buf[send_index++] = 0x00;
                        ctx->send_buf[send_index++] = 0x01;
                        ctx->send_buf[send_index++] = 0x00;
                        *(slave_info->rsp_length) = send_index;
                        break;
                    }
                    ctx->send_buf[send_index++] = data_ptr[0];
                    ctx->send_buf[send_index++] = data_ptr[1];
                    ctx->send_buf[send_index++] = cmd_data_len >> 8;
                    ctx->send_buf[send_index++] = cmd_data_len & 0xff;
                    memcpy(&ctx->send_buf[send_index], &(p2p_info->file_info), cmd_data_len);
                    *(slave_info->rsp_length) = send_index + cmd_data_len;
                }break;
                case TRANS_FILE_CMD_DATA: {         //传输文件内容
                    if(0 >= p2p_info->fd) {
                        ret = -MODBUS_RT_EEMPTY;
                        break;
                    }
                    if(0 >= cmd_data_len) {
                        ret = -MODBUS_RT_ERROR;
                        break;
                    }
                    
                    int recv_bytes = modbus_rt_file_read_file(p2p_info->fd, &(ctx->send_buf[send_index + 5]), P2P_SLAVE_BUF_LEN);
                    if( 0 > recv_bytes) {
                        ret = -MODBUS_RT_ERROR;
                        break;
                    }
                    p2p_info->write_file_size += recv_bytes;

                    int ask_len = recv_bytes + 1;
                    ctx->send_buf[send_index++] = data_ptr[0];
                    ctx->send_buf[send_index++] = data_ptr[1];
                    ctx->send_buf[send_index++] = ask_len >> 8;
                    ctx->send_buf[send_index++] = ask_len & 0xff;
                    if (P2P_SLAVE_BUF_LEN > recv_bytes) {
                        ctx->send_buf[send_index++] = 0x00;       //最后一包数据
                        modbus_rt_file_close(p2p_info->fd);         //关闭文件
                        p2p_info->fd = 0;
                        if(p2p_info->write_file_size != p2p_info->file_info.file_size) {
                            ret = -MODBUS_RT_ERROR;
                            break;
                        }
                    } else {
                        ctx->send_buf[send_index++] = 0x01;       //不是最后一包数据
                    } 
                    *(slave_info->rsp_length) = send_index + recv_bytes;
                }break;
            }
            if(MODBUS_RT_EOK != ret) {
                break;
            }
        } break;
#endif
        default : {
            ret = -MODBUS_RT_ERROR;
        } break;
    }
    return ret;
}

/**
 * @brief   compute_meta_length_after_function_callback     设置特殊指令报文的数据源长度
 * @param   ctx                                             agile modbus 句柄
 * @param   function                                        modbus特殊功能码
 * @param   msg_type                                        消息类型.(包括主机端的请求消息 和 服务器端的请求消息)
 * @return  uint8_t                                         数据元长度
 */
uint8_t compute_meta_length_after_function_callback(agile_modbus_t *ctx, int function,
                                                    agile_modbus_msg_type_t msg_type)
{
    int length;
    if (msg_type == AGILE_MODBUS_MSG_INDICATION) {        //返回主机请求报文的数据元长度(uint8_t 类型)，不是特殊功能码必须返回 0。
        length = 0;
        if (function == AGILE_MODBUS_FC_TRANS_FILE) {
            length = 4;                                 //主机端:命令(2字节) + 数据长度(2字节)
        } else if (function == AGILE_MODBUS_FC_READ_FILE) {
            length = 4;                                 //主机端:命令(2字节) + 数据长度(2字节)
        }
    } else {                                            //返回从机响应报文的数据元长度(uint8_t 类型)，不是特殊功能码必须返回 1。
        /* MSG_CONFIRMATION */
        length = 1;
        if (function == AGILE_MODBUS_FC_TRANS_FILE) {
            length = 3;                                 //从机端:命令(2字节)+状态(1字节)
        } else if (function == AGILE_MODBUS_FC_READ_FILE) {
            length = 4;   //主机端:命令(2字节) + 数据长度(2字节)
        }
    }
    return length;
}


/**
 * @brief   compute_data_length_after_meta_callback         设置特殊指令报文的数据源后面的数据内容的长度信息
 * @param   ctx                                             agile modbus 句柄
 * @param   msg                                             消息内容
 * @param   msg_length                                      消息长度
 * @param   msg_type                                        消息类型.(包括主机端的请求消息 和 服务器端的请求消息)
 * @return  int                                             数据内容长度
 */
int compute_data_length_after_meta_callback(agile_modbus_t *ctx, uint8_t *msg,
                                            int msg_length, agile_modbus_msg_type_t msg_type)
{
    int function = msg[ctx->backend->header_length];
    int length;
    if (msg_type == AGILE_MODBUS_MSG_INDICATION) {        //返回主机请求报文数据元之后的数据长度,不是特殊功能码必须返回 0。
        length = 0;
        if (function == AGILE_MODBUS_FC_TRANS_FILE) {
            //数据元之后的长度为:数据长度(2字节)
            length = (msg[ctx->backend->header_length + 3] << 8) + msg[ctx->backend->header_length + 4];
        } else if (function == AGILE_MODBUS_FC_READ_FILE) {
            length = (msg[ctx->backend->header_length + 3] << 8) + msg[ctx->backend->header_length + 4];   //主机端:命令(2字节) + 数据长度(2字节)
        }
    } else {                                            //返回从机响应报文数据元之后的数据长度，不是特殊功能码必须返回 0。
        /* MSG_CONFIRMATION */
        length = 0;                                     //从机端数据元之后的数据长度为0,如果增加读取文件功能,则会又改功能
        if (function == AGILE_MODBUS_FC_TRANS_FILE) {
            //数据元之后的长度为:数据长度(2字节)
            length = 0;
        } else if (function == AGILE_MODBUS_FC_READ_FILE) { 
            length = (msg[ctx->backend->header_length + 3] << 8) + msg[ctx->backend->header_length + 4];    //主机端:命令(2字节) + 数据长度(2字节)
        }
	}
    return length;
}

#if MODBUS_P2P_MASTER_ENABLE
modbus_p2p_master_info_t g_modbus_p2p_master_info;


#endif

#endif
