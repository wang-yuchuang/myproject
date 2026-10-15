/**
 * @file common_utility.h
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: common utility for slimfs
 *
 * Revision History:
 * -----------------
 */
#ifndef __COMMON_UTILITY_H__
#define __COMMON_UTILITY_H__
/* =========================== include file session ======================== */
#include <stdint.h>
#include "sdrv_slimfs.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== marco define session ======================== */
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef ROUNDUP
#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#endif
#ifndef ROUNDDOWN
#define ROUNDDOWN(a, b) ((a) & ~((b)-1))
#endif


/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */

/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */
void slimfs_hexdump(const void *ptr, uint32_t len, uint64_t address);
void convert_bytes2unit(uint64_t bytes, char* buf);
void clear_regions(uint32_t*data, uint8_t startBit, uint8_t bitLen);
void set_regions(uint32_t* data, uint8_t startBit, uint8_t bitLen, uint32_t setData);
uint32_t get_regions(uint32_t data, uint8_t startBit, uint8_t bitLen);
int32_t split_str(char* str, char c, char** subStrs);
int32_t strtodigit(const char *nptr, char **endptr, int base);
int32_t get_line(char* s, int32_t len);
void dump_slots_info(slot_info_t* slotInfo, const char* prefix);
void dump_slimfs_info(slimfs_header_t* slimfsHeader, const char* prefix);
void dump_item_mgr_info(slimfs_item_mgr_t* itemMgr, const char* prefix);
uint32_t slimfs_get_us_tick(void);
/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __COMMON_UTILITY_H__ */
/* end of header file common_utility.h */