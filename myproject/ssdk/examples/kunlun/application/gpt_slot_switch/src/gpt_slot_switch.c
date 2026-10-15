/************************************************************************
 * Copyright(c) 2026  Semidrive
 * All rights reserved.
 *
 * gpt_slot_switch.c — GPT A/B slot switch CLI example (kunlun / OSPI2 NOR)
 *
 * Purpose:
 *   Switch the A/B active slot flag in the NOR GPT partition table for the
 *   R-core (SEC domain) ssystem. This is a standalone example/application
 *   that reuses the existing ptdev (partition device) API to mark a slot
 *   active and write the attributes back to NOR GPT.
 *
 * Usage (from R-core FreeRTOS CLI):
 *   slot_switch status   -> show current active slot + slot attributes
 *   slot_switch a        -> mark slot A active (GPT active bit -> A)
 *   slot_switch b        -> mark slot B active (GPT active bit -> B)
 *
 * Note: After switching the active slot, a reboot is required for the FDA
 *       (t_loader) to boot the newly active slot.
 ************************************************************************/
#include <CLI.h>
#include <debug.h>

/* 用直接 printf（与 xip main.c Hello 一致），ssdk_printf 需 CONFIG_DEBUG 定义才输出，会吞掉命令结果 */
#define DBG(format, args...) printf("[GPT] " format, ##args)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arch/cache.h>
#include <reg.h>     /* readl/writel */
#include <lib/disk/disk.h>
#include <lib/disk/disk_norflash.h>
#include <lib/partition/partition_parser.h>
#include <lib/partition/ab_partition_parser.h>

#include "gpt_slot_switch.h"
#include <lib/cksum.h>      /* crc32 (标准 CRC-32/IEEE) */
#include <flash_test.h>     /* flash_test_erase/flash_test_write RAM 闭包 */

/* NOR GPT offset (all A/B slides partition table in SPI NOR starts at 0x2000) */
#ifndef NORFLASH_GPT_OFFSET
#define NORFLASH_GPT_OFFSET   (0x2000)
#endif

#define GPT_SSYSTEM_NAME      "ssystem"

/* GPT 镜像结构（Oracle 复核确认）：镜像@0x2000，header@0x2200("EFI PART"), entries@0x2400 */
#define GPT_MIRROR_OFFSET     (0x2000)      /* GPT 镜像烧录起点 */
#define GPT_HDR_OFFSET        (GPT_MIRROR_OFFSET + 0x200)   /* "EFI PART" header */
#define GPT_ENTRY_LBA_OFFSET  72            /* header 内 partition_entry_lba */
#define GPT_ENTRY_SIZE        128           /* partition entry 大小 */
#define GPT_ENTRY_NAME_OFF    56            /* entry 内名字（UTF16LE）*/
#define GPT_ENTRY_ATTR_OFF    48            /* entry 内 attribute_flag */
#define GPT_ACTIVE_BIT        50            /* PART_ATT_ACTIVE_BIT */
#define OSPI2_XIP_BASE        0x08000000u   /* OSPI2 AHB 窗 */

/* XIP direct 读 GPT（绕开 cospi_lock）。返回 0=成功。 */
static int gpt_xip_read(uint32_t flash_off, void *dst, uint32_t size)
{
    uint32_t addr = OSPI2_XIP_BASE + flash_off;
    arch_clean_invalidate_cache_range((addr_t)addr, ROUNDUP(size, 32));
    memcpy(dst, (void *)(uintptr_t)addr, size);
    return 0;
}

/* 解析 GPT 找 ssystem_a/b 的 attribute_flag。返回 0=找到, -1=未找到。 */
static int gpt_find_ssystem_attr(uint64_t *attr_a, uint64_t *attr_b)
{
    uint8_t hdr[92], entry[128];
    uint32_t entry_lba, entries_abs, i;
    /* ① 读 GPT header @0x2200 校验 "EFI PART" */
    memset(hdr, 0, sizeof(hdr));
    gpt_xip_read(GPT_HDR_OFFSET, hdr, sizeof(hdr));
    if (memcmp(hdr, "EFI PART", 8) != 0) {
        DBG("GPT header signature invalid @0x%x\n", GPT_HDR_OFFSET);
        return -1;
    }
    entry_lba = *(uint32_t *)(hdr + GPT_ENTRY_LBA_OFFSET);
    DBG("GPT header OK, entry_lba=%u\n", entry_lba);
    /* ② entries 起点 = GPT_MIRROR_OFFSET + entry_lba*512 */
    entries_abs = GPT_MIRROR_OFFSET + entry_lba * 512u;
    /* ③ 按名字匹配 ssystem_a / ssystem_b */
    *attr_a = 0; *attr_b = 0;
    for (i = 0; i < 128; i++) {
        char name[37];
        unsigned j, n = 0;
        gpt_xip_read(entries_abs + i * GPT_ENTRY_SIZE, entry, GPT_ENTRY_SIZE);
        for (j = 0; j < 36 && n < 36; j += 2) {
            uint16_t c = entry[GPT_ENTRY_NAME_OFF + j] | (entry[GPT_ENTRY_NAME_OFF + j + 1] << 8);
            if (c == 0 || c >= 0x80) break;
            name[n++] = (char)c;
        }
        name[n] = 0;
        if (strcmp(name, "ssystem_a") == 0) {
            *attr_a = *(uint64_t *)(entry + GPT_ENTRY_ATTR_OFF);
        } else if (strcmp(name, "ssystem_b") == 0) {
            *attr_b = *(uint64_t *)(entry + GPT_ENTRY_ATTR_OFF);
        }
    }
    if (*attr_a || *attr_b) {
        DBG("ssystem_a attr=0x%llx, ssystem_b attr=0x%llx\n", (unsigned long long)*attr_a, (unsigned long long)*attr_b);
        return 0;
    }
    DBG("ssystem_a/b not found in GPT entries\n");
    return -1;
}


/* 用 XIP direct 读 GPT 显示 active 槽（绕开 cospi_lock/ptdev）*/
static void gpt_show_slot_status(void)
    {
    uint64_t attr_a = 0, attr_b = 0;
    int active_slot = INVALID;

    if (gpt_find_ssystem_attr(&attr_a, &attr_b) != 0) {
        DBG("=== GPT A/B Slot Status: unable to read GPT ===\n");
        return;
    }
    /* ACTIVE_BIT=50 判断 active */
    if (attr_a & (1ULL << GPT_ACTIVE_BIT))
        active_slot = SLOT_A;
    else if (attr_b & (1ULL << GPT_ACTIVE_BIT))
        active_slot = SLOT_B;
    DBG("=== GPT A/B Slot Status (%s) ===\n", GPT_SSYSTEM_NAME);
    if (active_slot == SLOT_A || active_slot == SLOT_B)
        DBG("  Active slot   : %s\n", SUFFIX_SLOT(active_slot));
    else
        DBG("  Active slot   : INVALID (0x%x/0x%x)\n", attr_a, attr_b);
    DBG("  ssystem_a attr=0x%x\n", attr_a);
    DBG("  ssystem_b attr=0x%x\n", attr_b);
}

/* GPT entries 区大小 = 128 条目 × 128 字节 = 16KB（@flash 0x2400-0x6400） */
#define GPT_ENTRY_COUNT        128
#define GPT_ENTRIES_SIZE       (GPT_ENTRY_COUNT * GPT_ENTRY_SIZE)

#define GPT_ACTIVE_VAL         (1ULL << GPT_ACTIVE_BIT)   /* 1ULL<<50 */

/* 静态 RAM buffer（4KB 栈数组过大，用 static 放 .data/.bss） */
static uint8_t g_gpt_sector[4096];
static uint8_t g_gpt_entries[GPT_ENTRIES_SIZE];

/* 在 entries buffer 中按名字（UTF16LE）匹配，返回条目在 buffer 内偏移 */
static int gpt_find_entry_name(const uint8_t *entries, const char *want,
                               uint32_t *entry_off)
{
    uint32_t i;
    for (i = 0; i < GPT_ENTRY_COUNT; i++) {
        const uint8_t *e = entries + i * GPT_ENTRY_SIZE;
        char name[37];
        unsigned j, n = 0;
        for (j = 0; j < 36 && n < 36; j += 2) {
            uint16_t c = e[GPT_ENTRY_NAME_OFF + j] |
                         (e[GPT_ENTRY_NAME_OFF + j + 1] << 8);
            if (c == 0 || c >= 0x80) break;
            name[n++] = (char)c;
        }
        name[n] = 0;
        if (strcmp(name, want) == 0) {
            *entry_off = i * GPT_ENTRY_SIZE;
            return 0;
        }
    }
    return -1;
}

/* a/b 切槽：RAM STIG 闭包写 GPT attribute（绕开 cospi_lock）。
 * 只改 primary GPT（attr×2 + entries_crc32 + header_crc32，均在 0x2000-0x2FFF 一扇区）。 */
static int gpt_switch_slot_xip(int slot)
{
    uint64_t attr_a = 0, attr_b = 0, new_a, new_b;
    uint32_t off_a, off_b, entries_crc, header_crc;
    int was = INVALID;
    int ret;

    if (slot != SLOT_A && slot != SLOT_B) {
        DBG("invalid slot, use 'a' or 'b'\n");
        return 0;
    }

    /* ① 读当前 attr */
    if (gpt_find_ssystem_attr(&attr_a, &attr_b) != 0) {
        DBG("unable to read GPT slot attrs, abort\n");
        return 0;
    }
    if (attr_a & GPT_ACTIVE_VAL) was = SLOT_A;
    else if (attr_b & GPT_ACTIVE_VAL) was = SLOT_B;

    if (slot == was) {
        DBG("already active on %s, no need to switch\n", SUFFIX_SLOT(slot));
        return 0;
    }

    /* ② 读全量 entries (16KB) 到 RAM，按名字定位 ssystem_a/b */
    gpt_xip_read(GPT_MIRROR_OFFSET + 0x400, g_gpt_entries, GPT_ENTRIES_SIZE);
    if (gpt_find_entry_name(g_gpt_entries, "ssystem_a", &off_a) ||
        gpt_find_entry_name(g_gpt_entries, "ssystem_b", &off_b)) {
        DBG("ssystem_a/b not found in GPT entries, abort\n");
        return 0;
    }

    /* ③ activate 目标槽 / deactivate 另一槽（对齐 BM ptdev_activate_slot / ptdev_deactivate_slot 语义）。
     * activate = priority 最高 + active + retry count 满，清 successful/unbootable；
     * deactivate = 置 UNBOOTABLE + 清 SUCCESS/ACTIVE/PRIORITY/MAX_RETRY（复刻 BM ptdev_deactivate_slot）。否则 t_loader ptdev_find_active_slot 的
     * "first bootup" fallback（zeroed 计数跨 priority 迭代累加，count==2 强制 SLOT_A）
     * 会在 fresh flash（两槽 attr=0）下静默吞掉切 B。 */
    new_a = *(uint64_t *)(g_gpt_entries + off_a + GPT_ENTRY_ATTR_OFF);
    new_b = *(uint64_t *)(g_gpt_entries + off_b + GPT_ENTRY_ATTR_OFF);
    if (slot == SLOT_A) {
        new_a = (new_a | PART_ATT_PRIORITY_VAL | PART_ATT_ACTIVE_VAL
                        | PART_ATT_MAX_RETRY_COUNT_VAL)
                & ~((uint64_t)PART_ATT_SUCCESSFUL_VAL)
                & ~((uint64_t)PART_ATT_UNBOOTABLE_VAL);
        /* B5c+P1 (2026-09-04/05): 非目标槽不清历史遗留 unboot (pre-B5c 标的) → 切槽时
         * 旧槽=刚成功运行槽 (内容良好), 显式清 unboot 恢复为回滚候选。 */
        new_b = new_b
                & ~((uint64_t)PART_ATT_SUCCESSFUL_VAL)
                & ~((uint64_t)PART_ATT_ACTIVE_VAL)
                & ~((uint64_t)PART_ATT_PRIORITY_VAL)
                & ~((uint64_t)PART_ATT_MAX_RETRY_COUNT_VAL)
                & ~((uint64_t)PART_ATT_UNBOOTABLE_VAL);
    } else {
        new_b = (new_b | PART_ATT_PRIORITY_VAL | PART_ATT_ACTIVE_VAL
                        | PART_ATT_MAX_RETRY_COUNT_VAL)
                & ~((uint64_t)PART_ATT_SUCCESSFUL_VAL)
                & ~((uint64_t)PART_ATT_UNBOOTABLE_VAL);
        new_a = new_a
                & ~((uint64_t)PART_ATT_SUCCESSFUL_VAL)
                & ~((uint64_t)PART_ATT_ACTIVE_VAL)
                & ~((uint64_t)PART_ATT_PRIORITY_VAL)
                & ~((uint64_t)PART_ATT_MAX_RETRY_COUNT_VAL)
                & ~((uint64_t)PART_ATT_UNBOOTABLE_VAL);
    }

    *(uint64_t *)(g_gpt_entries + off_a + GPT_ENTRY_ATTR_OFF) = new_a;
    *(uint64_t *)(g_gpt_entries + off_b + GPT_ENTRY_ATTR_OFF) = new_b;

    /* ④ 读 4KB sector (0x2000-0x2FFF) 到 RAM */
    gpt_xip_read(GPT_MIRROR_OFFSET, g_gpt_sector, 4096);

    /* ⑤ 把修改后的 ssystem_a/b entry 同步进 sector buffer（两者均在 sector 内） */
    memcpy(g_gpt_sector + 0x400 + off_a, g_gpt_entries + off_a, GPT_ENTRY_SIZE);
    memcpy(g_gpt_sector + 0x400 + off_b, g_gpt_entries + off_b, GPT_ENTRY_SIZE);

    /* ⑥ entries CRC（128×128）写入 header entries_crc32 字段 @+88 */
    entries_crc = (uint32_t)crc32(0, g_gpt_entries, GPT_ENTRIES_SIZE);
    *(uint32_t *)(g_gpt_sector + 0x200 + 88) = entries_crc;

    /* ⑦ header CRC：先清零 header_crc32 字段 @+16，再对 92B 计算写回 */
    *(uint32_t *)(g_gpt_sector + 0x200 + 16) = 0;
    header_crc = (uint32_t)crc32(0, g_gpt_sector + 0x200, 92);
    *(uint32_t *)(g_gpt_sector + 0x200 + 16) = header_crc;

    /* ⑧ 擦 + 写回（RAM STIG 闭包）——B6 修复 (2026-09-04): 原 flash_test_erase+flash_test_write
     * 两次独立闭包调用 → per-sector closure 累积崩溃 (AGENTS 反模式, pc=0x08210c4c);
     * 改用与 ota_switch_gpt 相同的 flash_test_program_range 单次批闭包 (擦+写一体, 已验证多次成功) */
    ret = flash_test_program_range(GPT_MIRROR_OFFSET, g_gpt_sector, 4096);
    if (ret) {
        DBG("program GPT sector @0x%x FAILED ret=%d\n", GPT_MIRROR_OFFSET, ret);
        return 0;
    }

    /* ⑨ 验证 */
    __asm volatile("isb" ::: "memory");
    arch_invalidate_cache_range(OSPI2_XIP_BASE + GPT_MIRROR_OFFSET, 4096);
    if (gpt_find_ssystem_attr(&attr_a, &attr_b) != 0) {
        DBG("verify: unable to re-read GPT attrs\n");
        return 0;
    }
    DBG("GPT active slot switched to %s (was %s)\n", SUFFIX_SLOT(slot),
        (was == SLOT_A || was == SLOT_B) ? SUFFIX_SLOT(was) : "INVALID");
    DBG("  ssystem_a attr=0x%llx, ssystem_b attr=0x%llx\n",
        (unsigned long long)attr_a, (unsigned long long)attr_b);
    DBG("  NOTE: reboot required for FDA to boot the newly active slot\n");
    return 0;
}

int gpt_slot_switch_cmd(int argc, char *argv[])
{
    int slot = INVALID;

    DBG("[1] enter gpt_slot_switch_cmd argc=%d\n", argc);
    if (argc < 1) {
        DBG("[2] usage (argc<1)\n");
        DBG("usage: slot_switch [a|b|status]\n");
        return 0;/* CLI: MUST return 0 (non-zero = repeat, infinite loop) */
    }

    DBG("[3] subcommand='%s'\n", argv[0]);
    if (strcmp(argv[0], "status") == 0) {
        /* read-only: XIP direct 读 GPT（绕开 cospi_lock）*/
        gpt_show_slot_status();
        return 0;
    } else if (strcmp(argv[0], "a") == 0) {
        slot = SLOT_A;
    } else if (strcmp(argv[0], "b") == 0) {
        slot = SLOT_B;
    } else {
        DBG("[4] invalid subcommand '%s'\n", argv[0]);
        DBG("usage: slot_switch [a|b|status]\n");
        return 0;/* CLI: MUST return 0 (non-zero = repeat, infinite loop) */
    }

    /* a/b 切换：RAM 闭包 STIG 写 GPT attribute（绕开 cospi_lock） */
    DBG("[5] slot_switch %s: switch via RAM STIG write\n", argv[0]);
    gpt_switch_slot_xip(slot);
    return 0;
}

CLI_CMD("slot_switch", "GPT A/B slot switch: slot_switch [a|b|status]", gpt_slot_switch_cmd);
