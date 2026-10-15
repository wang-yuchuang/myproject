/**
 * @file common_utility.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <ctype.h>

#include "common_utility.h"
#include "sdrv_slimfs.h"
#include "sdrv_slimfs_impl.h"
#include "sdrv_slimfs_config.h"

/* =========================== marco define session ======================== */
#define hexstr(p) (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */
void slimfs_hexdump(const void *ptr, uint32_t len, uint64_t address)
{
#if defined(CONFIG_SLIMFS_TEST_WITH_PC)
    uint64_t addr = (uint64_t)ptr;
#else
    uint32_t addr = (uint32_t)ptr;
#endif
    uint32_t count;
    for (count = 0 ; count < len; count += 16) {
        union {
            uint32_t buf[4];
            uint8_t  cbuf[16];
        } u;
        uint32_t s = ROUNDUP(MIN(len - count, 16), 4);
        uint32_t i;
#if defined(CONFIG_SLIMFS_TEST_WITH_PC)
        printf("0x%08lx: ", address);
#else
        printf("0x%08llx: ", address);
#endif
        for (i = 0; i < s / 4; i++) {
            u.buf[i] = ((const uint32_t *)addr)[i];
            printf("%08x ", u.buf[i]);
        }
        for (; i < 4; i++) {
            printf("         ");
        }
        printf("|");

        for (i=0; i < 16; i++) {
            unsigned char c = u.cbuf[i];
            if (i < s && isprint(c)) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        printf("|\n");
        address += 16;
        addr += 16;
    }
}

void convert_bytes2unit(uint64_t bytes, char* buf)
{
    int32_t div_m = 0;
    int32_t div_k = 0;
    int32_t rem = 0;


    div_m = bytes / (1 << 20);
    rem = bytes % (1 << 20);
    div_k = rem / (1 << 10);
    rem = rem % (1 << 10);

    sprintf(buf, "%dM %dK %dbytes", div_m, div_k, rem);
}

void clear_regions(uint32_t*data, uint8_t startBit, uint8_t bitLen)
{
    // check parameter
    if ((startBit + bitLen) >= (sizeof(uint32_t) * 8)) {
        return;
    }

    // clear bit region
    uint32_t mask_win = 0;
    mask_win = (1 << bitLen) - 1;
    mask_win = mask_win << startBit;
    mask_win = ~mask_win;

    *data &= mask_win;
}

void set_regions(uint32_t* data, uint8_t startBit, uint8_t bitLen, uint32_t setData)
{
    // check para valid
    if (((startBit + bitLen) >= (sizeof(uint32_t) * 8)) ||
        (setData > (uint32_t)((1 << bitLen) - 1))) {
        return;
    }

    // clear bit region
    clear_regions(data, startBit, bitLen);

    // set bit region
    *data |= setData << startBit;
}

uint32_t get_regions(uint32_t data, uint8_t startBit, uint8_t bitLen)
{
    if ((startBit + bitLen) >= (sizeof(uint32_t) * 8)) {
        return 0;
    }

    uint32_t mask_win = 0;

    mask_win = (1 << bitLen) - 1;
    mask_win = mask_win << startBit;
    data &= mask_win;

    return (data >> startBit);
}

int32_t split_str(char* str, char c, char** subStrs)
{
    if (str == NULL || subStrs == NULL) {
        return -1;
    }
    char* p = str;
    char* sub_str = NULL;
    int32_t i = 0;

    while (*p == c) p++;

    sub_str = p;
    while (*p != '\0') {
        if (*p == c) {
            if (sub_str != NULL) {
                subStrs[i++] = sub_str;
                sub_str = NULL;
            }
            *p = '\0';
        }
        else if (sub_str == NULL) {
            sub_str = p;
        }
        p++;
    }

    if (sub_str != NULL) {
        subStrs[i++] = sub_str;
    }

    return i;
}

static int32_t radix_value( char c )
{
    if(c >= '0'  &&  c <= '9') return(c - '0');
    if(c >= 'a'  &&  c <= 'f') return(c - 'a' + 10);
    if(c >= 'A'  &&  c <= 'F') return(c - 'A' + 10);
    return( 37 );
}

int32_t strtodigit(const char *nptr, char **endptr, int base)
{
    const char	*p;
    const char	*startp;
    int 		digit;
    int32_t 	value;
    int32_t     factor = 1;

    if(endptr != NULL) *endptr = (char *)nptr;
    p = nptr;
    if(base == 0) {
        if(hexstr(p))       base = 16;
        else if(*p == '0')  base = 8;
        else		        base = 10;
    }
    if(base == 16) {
        if(hexstr(p))  p += 2;	/* skip over '0x' */
    }
    if (*p == '-') {
        p++;
        factor = -1;
    }

    startp = p;
    value = 0;
    for(;;) {
        digit = radix_value(*p);
        if(digit >= base) break;
        value = value * base + digit;
        ++p;
    }
    if(p == startp)  p = nptr;
    if(endptr != NULL) *endptr = (char *)p;
    return value * factor;
}

int32_t get_line(char* s, int32_t len)
{
    if (s == NULL) {
        return -1;
    }

    int32_t i = 0;
    int32_t ret = 0;

    char c = '\0';

    while ((c = getchar()) != '\n') {
        if (i < len - 1) {
            s[i++] = c;
        }
        else {
            ret = -1;
        }
    }
    s[i] = '\0';

    return ret;
}

void dump_item_mgr_info(slimfs_item_mgr_t* itemMgr, const char* prefix)
{
    if (itemMgr == NULL) {
        return;
    }

    char head_buf[50] = "";

    memset(head_buf, 0x00, sizeof(head_buf));
    sprintf(head_buf, "=============== %s ", itemMgr->name);
    uint8_t len = strlen(head_buf);
    memset(&head_buf[len], '=', 50 - len - 1);

    printf("%s%s\n", prefix == NULL ? "" : prefix, head_buf);
    printf("%sitem   name : %s\n", prefix == NULL ? "" : prefix, itemMgr->name);
    printf("%sitem     Id : 0x%08x\n", prefix == NULL ? "" : prefix, itemMgr->id);
    printf("%sitem length : %d\n", prefix == NULL ? "" : prefix, itemMgr->len);
    printf("%sitem  crcEn : %d\n", prefix == NULL ? "" : prefix, itemMgr->crc_en);
    if (itemMgr->crc_en) {
        printf("%sitem crcVal : %d\n", prefix == NULL ? "" : prefix, itemMgr->crc);
    }
    // printf("id is: 0x%08x, len: %d, crc_en: %d%s\n"
    //     , itemMgr->id
    //     , itemMgr->len
    //     , itemMgr->crc_en
    //     , ((itemMgr->crc_en == 1) ? (buf) : ("")));
    return;
}

void dump_slots_info(slot_info_t* slotInfo, const char* prefix)
{
    if (slotInfo == NULL) {
        return;
    }
    // char head_buf[70] = "";

    // memset(head_buf, 0x00, sizeof(head_buf));
    // sprintf(head_buf, "==================slot type: %d", slotInfo->slot_mgr.slot_type);
    // uint8_t len = strlen(head_buf);
    // memset(&head_buf[len], '=', 70 - len - 1);
    // printf("%s\n", head_buf);

    printf("%sslot    type : %d\n", prefix == NULL ? "" : prefix, slotInfo->slot_mgr.slot_type);
    printf("%sslot N_index : %d\n", prefix == NULL ? "" : prefix, slotInfo->n_index);
    printf("%sstart  block : %d\n", prefix == NULL ? "" : prefix, slotInfo->slot_mgr.start_block);
    printf("%sblock number : %d\n", prefix == NULL ? "" : prefix, slotInfo->slot_mgr.block_number);
    printf("%sslot  number : %d\n", prefix == NULL ? "" : prefix, slotInfo->slot_number);
    printf("%sslot    used : %d(%.2f)%%\n", prefix == NULL ? "" : prefix, slotInfo->slot_used, ((float)slotInfo->slot_used/(float)slotInfo->slot_number) * 100.0);

    return;
}

void dump_slimfs_info(slimfs_header_t* slimfsHeader, const char* prefix)
{
    if (slimfsHeader == NULL) {
        return;
    }

    printf("%smagic number: %s\n", prefix == NULL ? "" : prefix, slimfsHeader->magic);
    printf("%smajor number: %s\n", prefix == NULL ? "" : prefix, slimfsHeader->major);
    printf("%smanor number: %s\n", prefix == NULL ? "" : prefix, slimfsHeader->manor);
    printf("%sslot types number: %d\n", prefix == NULL ? "" : prefix, slimfsHeader->slot_num);

}

/* end of file common_utility.c */