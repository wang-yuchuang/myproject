/**
 * @file sdrv_slimfs_checksum.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: slimfs checksum implement
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdint.h>
#include <stdio.h>

#include "sdrv_slimfs_impl.h"
/* =========================== marco define session ======================== */
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */
#define CRC16_INIT_VALUE  0xFFFF
 /*
  * Computes an updated version of the CRC from existing CRC.
  * crc: the previous values of the CRC
  * buf: the data on which to apply the checksum
  * length: the number of bytes of data in 'buf' to be calculated.
  */
static unsigned short update_crc16(unsigned short crc, const unsigned char *buf,
			   unsigned int length)
{
	unsigned int i;
	for (i = 0; i < length; i++) {
		crc = (unsigned char) (crc >> 8) | (crc << 8);
		crc ^= buf[i];
		crc ^= (unsigned char) (crc & 0xff) >> 4;
		crc ^= (crc << 8) << 4;
		crc ^= ((crc & 0xff) << 4) << 1;
	}
	return crc;
}

 /*
  * Computes a CRC, starting with an initialization value.
  * buf: the data on which to apply the checksum
  * length: the number of bytes of data in 'buf' to be calculated.
  */
 __attribute__((weak)) unsigned short crc16(const unsigned char *buf, unsigned int length)
{
	unsigned short crc = CRC16_INIT_VALUE;
	return update_crc16(crc, buf, length);
}


/******************************************************************************
**@brief get checksum func
******************************************************************************/
checksum_fun_t get_check_func()
{
    return crc16;
}



// int32_t main(int32_t argc, const char* argv[])
// {
//     checksum_fun_t fun;

//     fun = get_check_func();

//     uint8_t buf[32];
//     for (int32_t i = 0; i < 32; i++) {
//         buf[i] = i;
//     }
//     buf[30] = 0xc5;

//     uint8_t buf2[32];
//     for (int32_t i = 0; i < 32; i++) {
//         buf2[i] = i;
//     }
//     buf2[30] = 0xc5;
//     buf2[31] = 0xc5;

//     uint16_t crc1 = fun(buf, 30);
//     uint16_t crc2 = fun(buf, 31);
//     uint16_t crc3 = fun(buf, 32);

//     uint16_t crc4 = fun(buf2, 30);
//     uint16_t crc5 = fun(buf2, 31);
//     uint16_t crc6 = fun(buf2, 32);

//     printf("crc1 = 0x%x\n", crc1);
//     printf("crc2 = 0x%x\n", crc2);
//     printf("crc3 = 0x%x\n", crc3);
//     printf("crc4 = 0x%x\n", crc4);
//     printf("crc5 = 0x%x\n", crc5);
//     printf("crc6 = 0x%x\n", crc6);
// }

/* end of file sdrv_slimfs_checksum.c */