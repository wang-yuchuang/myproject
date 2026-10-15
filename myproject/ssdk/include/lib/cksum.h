/**
 * @file chsum.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef __CKSUM_H
#define __CKSUM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <compiler.h>

#define MD5_LEN 16
typedef unsigned int __u32;

struct MD5Context {
    __u32 buf[4];
    __u32 bits[2];
    union {
        unsigned char in[64];
        __u32 in32[16];
    };
};

__BEGIN_CDECLS

/*
 * Computes the CRC-CCITT, starting with an initialization value.
 * buf: the data on which to apply the checksum
 * length: the number of bytes of data in 'buf' to be calculated.
 */
unsigned short crc16(const unsigned char *buf, unsigned int length);

/*
 * Computes an updated version of the CRC-CCITT from existing CRC.
 * crc: the previous values of the CRC
 * buf: the data on which to apply the checksum
 * length: the number of bytes of data in 'buf' to be calculated.
 */
unsigned short update_crc16(unsigned short crc, const unsigned char *buf, unsigned int len);

unsigned long crc32(unsigned long crc, const unsigned char *buf, unsigned int len);

unsigned long adler32(unsigned long adler, const unsigned char *buf, unsigned int len);

__END_CDECLS

/*
 * Calculate and store in 'output' the MD5 digest of 'len' bytes at
 * 'input'. 'output' must have enough space to hold 16 bytes.
 */
void md5 (const unsigned char *input, const int len, unsigned char output[16]);

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void MD5Init(struct MD5Context *ctx);

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void MD5Update(struct MD5Context *ctx, unsigned char const *buf, unsigned len);

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void MD5Final(unsigned char digest[16], struct MD5Context *ctx);

#if defined(__cplusplus)
}
#endif

#endif

