/**
 * @file    modbus_data_trans.h
 * @brief   modbus 大小端转换函数 头文件
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#ifndef __PKG_MODBUS_DATA_TRANS_H_
#define __PKG_MODBUS_DATA_TRANS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "modbus_config.h"

#if  MODBUS_DATA_TRANS_ENABLE
#include <stdint.h>
#include <string.h>

/**
 * @brief   modbus 大小端模式定义
 */
typedef enum {
    LITTLE_ENDIAL_SWAP = 0,                 //小端模式
    BIG_ENDIAL_SWAP = 1,                    //大端模式
    LITTLE_ENDIAL = 2,                      //内部大端，外部小端
    BIG_ENDIAL = 3,                       //内部小端，外部大端
}modbus_endian_mode;

/**
 * @brief   modbus 大小端 API函数
 */
uint16_t modbus_data_reg2reg(uint16_t _Src);

void modbus_data_regs2regs(uint16_t *_Dst, uint16_t *_Src, int size);

void modbus_data_regs2bytes(modbus_endian_mode mode, uint8_t *_Dst, uint16_t *_Src, int size);
void modbus_data_regs2str(modbus_endian_mode mode, char *_Dst, uint16_t *_Src, int size);
void modbus_data_regs2signed(modbus_endian_mode mode, int16_t *_Dst, uint16_t *_Src, int size);
void modbus_data_regs2int(modbus_endian_mode mode, int32_t *_Dst, uint16_t *_Src, int size);
void modbus_data_regs2uint(modbus_endian_mode mode, uint32_t *_Dst, uint16_t *_Src, int size);
void modbus_data_regs2long(modbus_endian_mode mode, int64_t *_Dst, uint16_t *_Src, int size);
void modbus_data_regs2ulong(modbus_endian_mode mode, uint64_t *_Dst, uint16_t *_Src, int size);
void modbus_data_regs2float(modbus_endian_mode mode, float *_Dst, uint16_t *_Src, int size);
void modbus_data_regs2double(modbus_endian_mode mode, double *_Dst, uint16_t *_Src, int size);

void modbus_data_bytes2regs(modbus_endian_mode mode, uint16_t *_Dst, uint8_t *_Src, int size);
void modbus_data_str2regs(modbus_endian_mode mode, uint16_t *_Dst, char *_Src, int size);
void modbus_data_signed2regs(modbus_endian_mode mode, uint16_t *_Dst, int16_t *_Src, int size);
void modbus_data_int2regs(modbus_endian_mode mode, uint16_t *_Dst, int32_t *_Src, int size);
void modbus_data_uint2regs(modbus_endian_mode mode, uint16_t *_Dst, uint32_t *_Src, int size);
void modbus_data_long2regs(modbus_endian_mode mode, uint16_t *_Dst, int64_t *_Src, int size);
void modbus_data_ulong2regs(modbus_endian_mode mode, uint16_t *_Dst, uint64_t *_Src, int size);
void modbus_data_float2regs(modbus_endian_mode mode, uint16_t *_Dst, float *_Src, int size);
void modbus_data_double2regs(modbus_endian_mode mode, uint16_t *_Dst, double *_Src, int size);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __PKG_MODBUS_DATA_TRANS_H_ */
