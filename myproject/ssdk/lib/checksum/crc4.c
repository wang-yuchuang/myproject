/********************************************************
 *        Copyright(c) 2022    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/
#include "debug.h"
#include "crc4.h"


const uint8_t CRC4_Table[16]= {0,13,7,10,14,3,9,4,1,12,6,11,15,2,8,5};

uint8_t crc4_calculate(uint8_t initial_value, uint8_t const *message, uint8_t len)
{
    uint8_t crc_val = initial_value;
    int i = 0;
    for( ;i < len; i++)
    {
        crc_val = CRC4_Table[crc_val] ^ message[i];
    }
    crc_val = 0 ^ CRC4_Table[crc_val];
    return crc_val;
}
