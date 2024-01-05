/**
 * @file    modbus_data_trans.c
 * @brief   modbus寄存器数据大小端转换函数
 * @author  SenySunny (senysunny@163.com)
 * @date    2023-05-14
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */
#include "modbus_data_trans.h"

#if  MODBUS_DATA_TRANS_ENABLE

/**
 * @brief   modbus_data_reg2reg:            uint16_t数据大小端转换函数：调用则大小端转换1次
 * @param   _Src:                           需要做转换的函数
 * @return  uint16_t：                      转换之后的数据
 *       
 */
uint16_t modbus_data_reg2reg(uint16_t _Src) {
     return (_Src << 8) | (_Src >> 8);
}


void modbus_data_regs2regs(uint16_t *_Dst, uint16_t *_Src, int size) {
    if(0 >= size) {
        return ;
    }
    for(int i = 0; i < size; i++) {
        _Dst[i] = (_Src[i] << 8) | (_Src[i] >> 8);
    }
}

void modbus_data_regs2bytes(modbus_endian_mode mode, uint8_t *_Dst, uint16_t *_Src, int size) {
    if(0 >= size) {
        return ;
    }
    switch(mode) {
        case LITTLE_ENDIAL_SWAP:
        case BIG_ENDIAL: {
            memcpy(_Dst, _Src, size * 2);
        } break;
        case BIG_ENDIAL_SWAP:
        case LITTLE_ENDIAL:  {
            uint8_t *strTemp = (uint8_t *)_Src;
            int len = size * 2;
             for(int i = 0; i < len; i += 2) {
                _Dst[i] = strTemp[i +1];
                _Dst[i + 1] = strTemp[i];
             }
        } break;
    }
}
void modbus_data_regs2str(modbus_endian_mode mode, char *_Dst, uint16_t *_Src, int size) {
    //_Dst指向的内存空间的长度应该大于size * 2，应该为(size * 2 + 1),以确保能够放下字符串末尾的0 
    if(0 >= size) {
        return ;
    }
    switch(mode) {
        case LITTLE_ENDIAL_SWAP: 
        case BIG_ENDIAL:{
            memcpy(_Dst, _Src, size * 2);
        } break;
        case BIG_ENDIAL_SWAP: 
        case LITTLE_ENDIAL: {
            char *strTemp = (char *)_Src;
            int len = size * 2;
             for(int i = 0; i < len; i += 2) {
                _Dst[i] = strTemp[i + 1];
                _Dst[i + 1] = strTemp[i];
             }
        } break;
    }
}
void modbus_data_regs2signed(modbus_endian_mode mode, int16_t *_Dst, uint16_t *_Src, int size) {
    if(0 >= size) {
        return ;
    }
    switch(mode) {
        case LITTLE_ENDIAL_SWAP: 
        case BIG_ENDIAL:{
            memcpy(_Dst, _Src, size * 2);
        } break;
        case BIG_ENDIAL_SWAP: 
        case LITTLE_ENDIAL:{
            for(int i = 0; i < size; i++) {
                _Dst[i] = (_Src[i] << 8) | (_Src[i] >> 8);
            }
        } break;
    }
}
void modbus_data_regs2int(modbus_endian_mode mode, int32_t *_Dst, uint16_t *_Src, int size) {
    if(0 >= size) {
        return ;
    }
    switch(mode) {
        case LITTLE_ENDIAL_SWAP: {
            memcpy(_Dst, _Src, size * 2);
        } break;
        case BIG_ENDIAL_SWAP: {
            uint8_t *strTemp = (uint8_t *)_Src;
            int len = size * 2;
            if (1 == size) {
                _Dst[0] = ((((int32_t)strTemp[0]) << 8) | ((int32_t)strTemp[1]));
                break;
            }
            for(int i = 0; i < len; i += 4) {
                _Dst[i/4] = ((((int32_t)strTemp[i + 0]) << 24) | (((int32_t)strTemp[i + 1]) << 16) | 
                (((int32_t)strTemp[i + 2]) << 8) | ((int32_t)strTemp[i + 3]));
            }
            if(size % 2) {
                _Dst[size/2] = ((((int32_t)strTemp[len - 2]) << 8) | ((int32_t)strTemp[len -1]));
            }
        } break;
        case LITTLE_ENDIAL: {
            uint8_t *strTemp = (uint8_t *)_Src;
            int len = size * 2;
            if (1 == size) {
                _Dst[0] = ((((int32_t)strTemp[0]) << 8) | ((int32_t)strTemp[1]));
                break;
            }
            for(int i = 0; i < len; i += 4) {
                _Dst[i/4] = ((((int32_t)strTemp[i + 2]) << 24) | (((int32_t)strTemp[i + 3]) << 16) | 
                (((int32_t)strTemp[i + 0]) << 8) | ((int32_t)strTemp[i + 1]));
            }
            if(size % 2) {
                _Dst[size/2] = ((((int32_t)strTemp[len - 2]) << 8) | ((int32_t)strTemp[len -1]));
            }
        } break;
        case BIG_ENDIAL: {
            uint8_t *strTemp = (uint8_t *)_Src;
            int len = size * 2;
            if (1 == size) {
                _Dst[0] = ((((int32_t)strTemp[1]) << 8) | ((int32_t)strTemp[0]));
                break;
            }
            for(int i = 0; i < len; i += 4) {
                _Dst[i/4] = ((((int32_t)strTemp[i + 1]) << 24) | (((int32_t)strTemp[i + 0]) << 16) | 
                (((int32_t)strTemp[i + 3]) << 8) | ((int32_t)strTemp[i + 2]));
            }
            if(size % 2) {
                _Dst[size/2] = ((((int32_t)strTemp[len - 1]) << 8) | ((int32_t)strTemp[len -2]));
            }
        } break;
    }
}
void modbus_data_regs2uint(modbus_endian_mode mode, uint32_t *_Dst, uint16_t *_Src, int size) {
    modbus_data_regs2int(mode, (int32_t *)_Dst, _Src, size);
}
void modbus_data_regs2long(modbus_endian_mode mode, int64_t *_Dst, uint16_t *_Src, int size) {
    switch(mode) {
        case LITTLE_ENDIAL_SWAP: {
            memcpy(_Dst, _Src, size * 2);
        } break;
        case BIG_ENDIAL_SWAP: {
            uint8_t *strTemp = (uint8_t *)_Src;
            int len = size * 2;
            if (4  < size) {
                for(int i = 0 ; i < len; i += 2) {
                    _Dst[0] = _Dst[0] << 16;
                    _Dst[0] |= ((((int64_t)strTemp[i]) << 8) | ((int64_t)strTemp[i + 1]));
                }
                break;
            }
            for(int i = 0; i < len; i += 8) {
                _Dst[i/4] = ((((int64_t)strTemp[0]) << 56) | (((int64_t)strTemp[1]) << 48) | 
                (((int64_t)strTemp[2]) << 40) | (((int64_t)strTemp[3]) << 32) || 
                (((int64_t)strTemp[4]) << 24) | (((int64_t)strTemp[5]) << 16) || 
                (((int64_t)strTemp[6]) << 8) | ((int64_t)strTemp[7]));
            }
            if(size % 4) {
                int end_len = ((size % 4) * 2);
                for(int i = end_len ; i >= 0; i -= 2) {
                    _Dst[size/4] = _Dst[0] << 16;
                    _Dst[size/4] |= ((((int64_t)strTemp[len - i - 2]) << 8) | ((int64_t)strTemp[len - i - 1]));
                }
            }
        } break;
        case LITTLE_ENDIAL: {
            uint8_t *strTemp = (uint8_t *)_Src;
            int len = size * 2;
            if (4  < size) {
                for(int i = len ; i >= 0; i -= 2) {
                    _Dst[0] = _Dst[0] << 16;
                    _Dst[0] |= ((((int64_t)strTemp[i - 1]) << 8) | ((int64_t)strTemp[i]));
                }
                break;
            }
            for(int i = 0; i < len; i += 8) {
                _Dst[i/4] = ((((int64_t)strTemp[i + 6]) << 56) | (((int64_t)strTemp[i + 7]) << 48) | 
                (((int64_t)strTemp[i + 4]) << 40) | (((int64_t)strTemp[i + 5]) << 32) || 
                (((int64_t)strTemp[i + 2]) << 24) | (((int64_t)strTemp[i + 3]) << 16) || 
                (((int64_t)strTemp[i + 0]) << 8) | ((int64_t)strTemp[i + 1]));
            }
            if(size % 4) {
                int end_len = ((size % 4) * 2);
                for(int i = 0 ; i < end_len; i += 2) {
                    _Dst[size/4] = _Dst[0] << 16;
                    _Dst[size/4] |= ((((int64_t)strTemp[len - i - 2]) << 8) | ((int64_t)strTemp[len - i - 1]));
                }
            }
        } break;
        case BIG_ENDIAL: {
            uint8_t *strTemp = (uint8_t *)_Src;
            int len = size * 2;
            if (4  < size) {
                for(int i = 0 ; i < len; i += 2) {
                    _Dst[0] = _Dst[0] << 16;
                    _Dst[0] |= ((((int64_t)strTemp[i + 1]) << 8) | ((int64_t)strTemp[i]));
                }
                break;
            }
            for(int i = 0; i < len; i += 8) {
                _Dst[i/4] = ((((int64_t)strTemp[i + 1]) << 56) | (((int64_t)strTemp[i + 0]) << 48) | 
                (((int64_t)strTemp[i + 3]) << 40) | (((int64_t)strTemp[i + 2]) << 32) || 
                (((int64_t)strTemp[i + 5]) << 24) | (((int64_t)strTemp[i + 4]) << 16) || 
                (((int64_t)strTemp[i + 7]) << 8) | ((int64_t)strTemp[i + 6]));
            }
            if(size % 4) {
                int end_len = ((size % 4) * 2);
                for(int i = end_len ; i >= 0; i -= 2) {
                    _Dst[size/4] = _Dst[0] << 16;
                    _Dst[size/4] |= ((((int64_t)strTemp[len - i - 1]) << 8) | ((int64_t)strTemp[len - i - 2]));
                }
            }
        } break;
    }
}
void modbus_data_regs2ulong(modbus_endian_mode mode, uint64_t *_Dst, uint16_t *_Src, int size) {
    modbus_data_regs2long(mode, (int64_t *)_Dst, _Src, size);
}
void modbus_data_regs2float(modbus_endian_mode mode, float *_Dst, uint16_t *_Src, int size) {
    modbus_data_regs2int(mode, (int32_t *)_Dst, _Src, size);
}
void modbus_data_regs2double(modbus_endian_mode mode, double *_Dst, uint16_t *_Src, int size) {
    modbus_data_regs2long(mode, (int64_t *)_Dst, _Src, size);
}


void modbus_data_bytes2regs(modbus_endian_mode mode, uint16_t *_Dst, uint8_t *_Src, int size) {
    if(0 >= size) {
        return ;
    }
    switch(mode) {
        case LITTLE_ENDIAL_SWAP:
        case BIG_ENDIAL: {
            memcpy(_Dst, _Src, size);
        } break;
        case BIG_ENDIAL_SWAP:
        case LITTLE_ENDIAL: {
            uint8_t *strTemp = (uint8_t *)_Src;
            if(1 == size) {
                _Dst[0] = (((uint16_t)strTemp[0]) << 8);
            }
            int len = size / 2;
            for (int i = 0; i < len; i++) {
                _Dst[i] = ((((uint16_t)strTemp[(i<<1) + 0]) << 8) + ((uint16_t)strTemp[(i<<1) + 1]));
            }
            if(size%2) {
                  _Dst[size/2] = (((uint16_t)strTemp[size - 1]) << 8);
            }
        } break;
    }
}
void modbus_data_str2regs(modbus_endian_mode mode, uint16_t *_Dst, char *_Src, int size) {
    modbus_data_bytes2regs(mode, _Dst, (uint8_t *)_Src, size);
}
void modbus_data_signed2regs(modbus_endian_mode mode, uint16_t *_Dst, int16_t *_Src, int size) {
    if(0 >= size) {
        return ;
    }
    switch(mode) {
        case LITTLE_ENDIAL_SWAP: 
        case BIG_ENDIAL:{
            memcpy(_Dst, _Src, size * 2);
        } break;
        case BIG_ENDIAL_SWAP: 
        case LITTLE_ENDIAL:{
            for(int i = 0; i < size; i++) {
                _Dst[i] = (_Src[i] << 8) | (_Src[i] >> 8);
            }
        } break;
    }
}
void modbus_data_int2regs(modbus_endian_mode mode, uint16_t *_Dst, int32_t *_Src, int size) {
    if(0 >= size) {
        return ;
    }
    switch(mode) {
        case LITTLE_ENDIAL_SWAP: {
            memcpy(_Dst, _Src, size * 4);
        } break;
        case BIG_ENDIAL_SWAP: {
            uint8_t *strTemp = (uint8_t *)_Src;
            for(int i = 0; i < size; i++) {
                _Dst[(i<<1)] = ((((uint16_t)strTemp[(i<<2) + 2]) << 8) + ((uint16_t)strTemp[(i<<2) + 3]));
                _Dst[(i<<1) + 1] = ((((uint16_t)strTemp[(i<<2) + 0]) << 8) + ((uint16_t)strTemp[(i<<2) + 1]));
            }
        } break;
        case LITTLE_ENDIAL: {
            uint8_t *strTemp = (uint8_t *)_Src;
            for(int i = 0; i < size; i++) {
                _Dst[(i<<1)] = ((((uint16_t)strTemp[(i<<2) + 0]) << 8) + ((uint16_t)strTemp[(i<<2) + 1]));
                _Dst[(i<<1) + 1] = ((((uint16_t)strTemp[(i<<2) + 2]) << 8) + ((uint16_t)strTemp[(i<<2) + 3]));
            }
        } break;
        case BIG_ENDIAL: {
            uint8_t *strTemp = (uint8_t *)_Src;
            for(int i = 0; i < size; i++) {
                _Dst[(i<<1)] = ((((uint16_t)strTemp[(i<<2) + 3]) << 8) + ((uint16_t)strTemp[(i<<2) + 2]));
                _Dst[(i<<1) + 1] = ((((uint16_t)strTemp[(i<<2) + 1]) << 8) + ((uint16_t)strTemp[(i<<2) + 0]));
            }
        } break;
    }
}
void modbus_data_uint2regs(modbus_endian_mode mode, uint16_t *_Dst, uint32_t *_Src, int size) {
    modbus_data_int2regs(mode, _Dst, (int32_t *)_Src, size);
}
void modbus_data_long2regs(modbus_endian_mode mode, uint16_t *_Dst, int64_t *_Src, int size) {
    if(0 >= size) {
        return ;
    }
    switch(mode) {
        case LITTLE_ENDIAL_SWAP: {
            memcpy(_Dst, _Src, size * 8);
        } break;
        case BIG_ENDIAL_SWAP: {
            uint8_t *strTemp = (uint8_t *)_Src;
            for(int i = 0; i < size; i++) {
                _Dst[(i<<2)] = ((((uint16_t)strTemp[(i<<3) + 6]) << 8) + ((uint16_t)strTemp[(i<<3) + 7]));
                _Dst[(i<<2) + 1] = ((((uint16_t)strTemp[(i<<3) + 4]) << 8) + ((uint16_t)strTemp[(i<<3) + 5]));
                _Dst[(i<<2) + 2] = ((((uint16_t)strTemp[(i<<3) + 2]) << 8) + ((uint16_t)strTemp[(i<<3) + 3]));
                _Dst[(i<<2) + 2] = ((((uint16_t)strTemp[(i<<3) + 0]) << 8) + ((uint16_t)strTemp[(i<<3) + 1]));
            }
        } break;
        case LITTLE_ENDIAL: {
            uint8_t *strTemp = (uint8_t *)_Src;
            for(int i = 0; i < size; i++) {
                _Dst[(i<<2)] = ((((uint16_t)strTemp[(i<<3) + 0]) << 8) + ((uint16_t)strTemp[(i<<3) + 1]));
                _Dst[(i<<2) + 1] = ((((uint16_t)strTemp[(i<<3) + 2]) << 8) + ((uint16_t)strTemp[(i<<3) + 3]));
                _Dst[(i<<2) + 2] = ((((uint16_t)strTemp[(i<<3) + 4]) << 8) + ((uint16_t)strTemp[(i<<3) + 5]));
                _Dst[(i<<2) + 2] = ((((uint16_t)strTemp[(i<<3) + 6]) << 8) + ((uint16_t)strTemp[(i<<3) + 7]));
            }
        } break;
        case BIG_ENDIAL: {
            uint8_t *strTemp = (uint8_t *)_Src;
            for(int i = 0; i < size; i++) {
                _Dst[(i<<2)] = ((((uint16_t)strTemp[(i<<3) + 7]) << 8) + ((uint16_t)strTemp[(i<<3) + 6]));
                _Dst[(i<<2) + 1] = ((((uint16_t)strTemp[(i<<3) + 5]) << 8) + ((uint16_t)strTemp[(i<<3) + 4]));
                _Dst[(i<<2) + 2] = ((((uint16_t)strTemp[(i<<3) + 3]) << 8) + ((uint16_t)strTemp[(i<<3) + 2]));
                _Dst[(i<<2) + 2] = ((((uint16_t)strTemp[(i<<3) + 1]) << 8) + ((uint16_t)strTemp[(i<<3) + 0]));
            }
        } break;
    }
}
void modbus_data_ulong2regs(modbus_endian_mode mode, uint16_t *_Dst, uint64_t *_Src, int size) {
    modbus_data_long2regs(mode, _Dst, (int64_t *)_Src, size);
}
void modbus_data_float2regs(modbus_endian_mode mode, uint16_t *_Dst, float *_Src, int size) {
    modbus_data_int2regs(mode, _Dst, (int32_t *)_Src, size);
}
void modbus_data_double2regs(modbus_endian_mode mode, uint16_t *_Dst, double *_Src, int size) {
    modbus_data_long2regs(mode, _Dst, (int64_t *)_Src, size); 
}

#endif
