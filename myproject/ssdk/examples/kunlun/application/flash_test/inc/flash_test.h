/************************************************************************
 * Copyright(c) 2026  Semidrive
 * All rights reserved.
 ************************************************************************/
#ifndef __FLASH_TEST_H__
#define __FLASH_TEST_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* OSPI2 NOR XIP 擦写最小化验证 CLI:
 *   flash_test    -> 备份->擦->写回安全区 sector，验证 XIP 下从 RAM 执行擦写是否崩溃
 *
 * 验证点（一次排除3个中高风险点）:
 *   Q3  中断/异常在 flash，擦写时从 RAM 执行是否崩溃
 *   自包含：擦写闭包是否会漏调 flash 里的函数（如 ssdk_printf）
 *   cache : 写回后校验数据是否被 cache 污染
 */
int flash_test_cmd(int argc, char *argv[]);

/* NOR flash 擦/写 RAM 闭包（XIP 下安全，从 RAM 执行；addr 为 flash offset，非 CPU 地址）。
 * 供 gpt_slot_switch 等应用复用。闭包内禁用中断，调用方需在非擦写期间调用。
 * 闭包内无 printf（格式串在 flash .rodata，擦写期间读 flash 返回垃圾）。 */
int flash_test_erase(uint32_t addr);
int flash_test_write(uint32_t addr, const uint8_t *buf, uint32_t size);
int flash_test_program_range(uint32_t flash_off, const uint8_t *buf, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_TEST_H__ */
