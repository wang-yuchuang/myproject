/**
 * @file sys_monitor_ota.c
 * @brief System Monitor — OTA 升级模块（R 核侧，staging 模式）
 *
 * 架构 (用户定稿 2026-09-02):
 *   - 阶段① 传输: A 核分块推送 ssystem_X + vbmeta-ssdk_X 双镜像 → R 核
 *     ExtMalloc 分配 DDR 缓存 (SAF 64MB 区) 暂存。传输期 flash 零接触，
 *     断电/重启/中断 → DDR 缓存丢弃, flash 无污染。
 *   - 阶段② 提交 (COMMIT): DDR CRC 双校验通过后 → 从缓存 STIG 擦写
 *     flash (ssystem_X → vbmeta-ssdk_X) → flash 回读 CRC 双校验 →
 *     切 NOR GPT。擦写窗口最短且无传输依赖。
 *
 * NOR 写入原语复用 flash_test (CONFIG_FLASH_TEST=y, 板上验证 13.5d):
 *   extern int flash_test_erase(uint32_t flash_off);
 *   extern int flash_test_write(uint32_t flash_off, const uint8_t *buf, uint32_t size);
 * GPT 解析/切槽位语义复刻 gpt_slot_switch.c (ab-slot 13.5c/l)。
 * DDR 缓存: ExtMalloc (FreeRTOS heap_ext, bmu05 main ExtHeapInit(SAF_MEMBASE, SAF_MEMSIZE))
 *
 * 关联变更: openspec/changes/rauc-xip-ota/
 */

#include "sys_monitor_ota.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cmsis_os2.h>

#include <arch/cache.h>
#include <reg.h>

#include <lib/cksum.h>             /* crc32: 仅用于 GPT entries/header CRC (EFI 标准, 与 gpt_slot_switch 一致) */
#include <lib/partition/partition_parser.h>  /* PART_ATT_* 官方位定义 */

/* sd_crc32 (poly 0x04C11DB7): 镜像数据 CRC, 与 A 核 ota-client/iccmsg 帧一致。
 * 头在 lib/sd_crc32/ (非 include 根), 符号已由 iccmsg 链接, 直接 extern */
extern uint32_t sd_crc32(uint32_t crc, const uint8_t *buffer, uint32_t len);
#include <lib/partition/partition_parser.h>  /* PART_ATT_* 官方位定义 */

#include "iccmsg.h"
#include "sys_monitor.h"      /* sys_monitor_get_ept() */

/* 实际运行槽判定 (定义于 sys_monitor_msg.c, 2026-09-03 根因修复) */
extern int sysmgr_running_slot(void);

/* DDR 缓存 (SAF 64MB ext heap, bmu05 main ExtHeapInit(SAF_MEMBASE, SAF_MEMSIZE)) */
extern void *ExtMalloc(size_t xWantedSize);
extern void ExtFree(void *pv);

/* ---- STIG 写原语（flash_test 导出，RAM 闭包，addr 为 flash offset） ---- */
extern int flash_test_erase(uint32_t flash_off);
extern int flash_test_write(uint32_t flash_off, const uint8_t *buf, uint32_t size);
extern int flash_test_program_range(uint32_t flash_off, const uint8_t *buf, uint32_t size);

/* [ssdk-standalone-boot-mode WDT fix] 长时 STIG 操作中喟狗 (sys_monitor.c 导出) */
extern void sys_monitor_wdt_ping(void);

/* ---- NOR GPT 布局（与 gpt_slot_switch.c 一致） ---- */
#define OTA_GPT_MIRROR_OFFSET   (0x2000)      /* GPT 镜像烧录起点 */
#define OTA_GPT_HDR_OFFSET      (OTA_GPT_MIRROR_OFFSET + 0x200)  /* "EFI PART" header @0x2200 */
#define OTA_GPT_ENTRY_LBA_OFF   72            /* header 内 partition_entry_lba */
#define OTA_GPT_ENTRY_SIZE      128
#define OTA_GPT_ENTRY_COUNT     128
#define OTA_GPT_ENTRIES_SIZE    (OTA_GPT_ENTRY_COUNT * OTA_GPT_ENTRY_SIZE)
#define OTA_GPT_ENTRIES_OFF     (0x400)       /* entries @0x2400 = mirror+0x400 */
#define OTA_GPT_ENTRY_NAME_OFF  56
#define OTA_GPT_ENTRY_ATTR_OFF  48
#define OTA_GPT_ENTRY_FIRST_LBA 32            /* entry+32: first_lba (8B LE) */
#define OTA_GPT_ENTRY_LAST_LBA  40            /* entry+40: last_lba (8B LE) */
#define OTA_OSPI2_XIP_BASE      0x08000000u
#define OTA_GPT_ACTIVE_VAL      PART_ATT_ACTIVE_VAL  /* 官方 1ULL<<50 */

/* 单槽镜像上限（ssystem 分区 960KB 内；BEGIN 时再对分区实际 size 校验） */
#define OTA_MAX_IMAGE_SIZE    (0xF0000u)
/* flash 擦写分块 (每块后 yield，保 tick/其它任务) */
#define OTA_WRITE_YIELD_GRAN   (64u * 1024u)
/* 回读校验块 */
#define OTA_VERIFY_CHUNK       1024

/* ---- OTA 运行状态 (staging) ---- */
typedef struct {
    volatile uint8_t  phase;             /* sysmgr_ota_phase_t */
    volatile uint8_t  error;
    uint8_t  target_slot;                /* 0=A 1=B (同一 OTA 会话固定) */
    uint8_t  cur_image;                  /* 当前接收 image_id */
    /* 双镜像 DDR 缓存 (ExtMalloc; cache[] 非空 = 已分配) */
    uint8_t *cache[2];                   /* [image_id] */
    uint32_t cache_alloc[2];
    uint32_t img_size[2];
    uint32_t img_crc[2];
    volatile uint32_t processed;         /* 当前镜像已收字节 */
    volatile uint8_t  received_mask;     /* bit_i: 镜像 i 收满 */
    volatile uint8_t  ota_active;
    volatile uint32_t last_activity;     /* tick: 传输阶段最后进展 (超时复位用) */
    uint32_t flash_off[2];               /* 每镜像目标 flash offset */
} ota_ctx_t;

#define OTA_XFER_TIMEOUT_MS  60000   /* 传输阶段无进展超时 (会话中断自愈) */

static ota_ctx_t          g_ota;
static osThreadId_t       g_ota_worker = NULL;
static osEventFlagsId_t   g_ota_ev     = NULL;

#define OTA_EV_FINALIZE   0x1

/* ---- GPT XIP direct 读（绕开 cospi_lock） ---- */
static int ota_gpt_xip_read(uint32_t flash_off, void *dst, uint32_t size)
{
    uint32_t addr = OTA_OSPI2_XIP_BASE + flash_off;
    arch_clean_invalidate_cache_range((addr_t)addr, ROUNDUP(size, 32));
    memcpy(dst, (void *)(uintptr_t)addr, size);
    return 0;
}

/* 在 entries buffer 按名字找 entry，返回偏移；找不到返回 -1 */
static int ota_gpt_find_name(const uint8_t *entries, const char *want,
                             uint32_t *entry_off)
{
    uint32_t i;
    for (i = 0; i < OTA_GPT_ENTRY_COUNT; i++) {
        const uint8_t *e = entries + i * OTA_GPT_ENTRY_SIZE;
        char name[37];
        unsigned j, n = 0;
        for (j = 0; j < 36 && n < 36; j += 2) {
            uint16_t c = e[OTA_GPT_ENTRY_NAME_OFF + j] |
                         (e[OTA_GPT_ENTRY_NAME_OFF + j + 1] << 8);
            if (c == 0 || c >= 0x80) break;
            name[n++] = (char)c;
        }
        name[n] = 0;
        if (strcmp(name, want) == 0) {
            *entry_off = i * OTA_GPT_ENTRY_SIZE;
            return 0;
        }
    }
    return -1;
}

/** 读当前 active slot（0=A 1=B），失败返回 -1 */
static int ota_get_active_slot(void)
{
    uint8_t hdr[92];
    uint32_t entry_lba, i;
    uint64_t attr_a = 0, attr_b = 0;

    memset(hdr, 0, sizeof(hdr));
    if (ota_gpt_xip_read(OTA_GPT_HDR_OFFSET, hdr, sizeof(hdr)) != 0) return -1;
    if (memcmp(hdr, "EFI PART", 8) != 0) return -1;
    entry_lba = *(uint32_t *)(hdr + OTA_GPT_ENTRY_LBA_OFF);

    for (i = 0; i < OTA_GPT_ENTRY_COUNT; i++) {
        uint8_t entry[OTA_GPT_ENTRY_SIZE];
        char name[37];
        unsigned j, n = 0;
        ota_gpt_xip_read(OTA_GPT_MIRROR_OFFSET + entry_lba * 512u + i * OTA_GPT_ENTRY_SIZE,
                         entry, OTA_GPT_ENTRY_SIZE);
        for (j = 0; j < 36 && n < 36; j += 2) {
            uint16_t c = entry[OTA_GPT_ENTRY_NAME_OFF + j] |
                         (entry[OTA_GPT_ENTRY_NAME_OFF + j + 1] << 8);
            if (c == 0 || c >= 0x80) break;
            name[n++] = (char)c;
        }
        name[n] = 0;
        if (strcmp(name, "ssystem_a") == 0) {
            attr_a = *(uint64_t *)(entry + OTA_GPT_ENTRY_ATTR_OFF);
        } else if (strcmp(name, "ssystem_b") == 0) {
            attr_b = *(uint64_t *)(entry + OTA_GPT_ENTRY_ATTR_OFF);
        }
    }
    if ((attr_a & OTA_GPT_ACTIVE_VAL) && !(attr_b & OTA_GPT_ACTIVE_VAL)) return 0;
    if ((attr_b & OTA_GPT_ACTIVE_VAL) && !(attr_a & OTA_GPT_ACTIVE_VAL)) return 1;
    return -1;
}

/** 解析 slot+image 分区 (ssystem_X / vbmeta-ssdk_X) 的 flash offset + 大小。失败返回 0 */
static uint32_t ota_resolve_slot_partition(uint8_t slot, uint8_t image_id,
                                           uint32_t *flash_off)
{
    uint8_t hdr[92];
    uint32_t entry_lba, i;
    const char *want;
    uint64_t first_lba, last_lba;

    if (slot > 1) return 0;
    if (image_id == SYSMGR_OTA_IMAGE_SSYSTEM)
        want = (slot == 0) ? "ssystem_a" : "ssystem_b";
    else if (image_id == SYSMGR_OTA_IMAGE_VBMETA_SSDK)
        want = (slot == 0) ? "vbmeta-ssdk_a" : "vbmeta-ssdk_b";
    else
        return 0;

    memset(hdr, 0, sizeof(hdr));
    if (ota_gpt_xip_read(OTA_GPT_HDR_OFFSET, hdr, sizeof(hdr)) != 0) return 0;
    if (memcmp(hdr, "EFI PART", 8) != 0) return 0;
    entry_lba = *(uint32_t *)(hdr + OTA_GPT_ENTRY_LBA_OFF);

    for (i = 0; i < OTA_GPT_ENTRY_COUNT; i++) {
        uint8_t entry[OTA_GPT_ENTRY_SIZE];
        char name[37];
        unsigned j, n = 0;
        ota_gpt_xip_read(OTA_GPT_MIRROR_OFFSET + entry_lba * 512u + i * OTA_GPT_ENTRY_SIZE,
                         entry, OTA_GPT_ENTRY_SIZE);
        for (j = 0; j < 36 && n < 36; j += 2) {
            uint16_t c = entry[OTA_GPT_ENTRY_NAME_OFF + j] |
                         (entry[OTA_GPT_ENTRY_NAME_OFF + j + 1] << 8);
            if (c == 0 || c >= 0x80) break;
            name[n++] = (char)c;
        }
        name[n] = 0;
        if (strcmp(name, want) == 0) {
            first_lba = *(uint64_t *)(entry + OTA_GPT_ENTRY_FIRST_LBA);
            last_lba  = *(uint64_t *)(entry + OTA_GPT_ENTRY_LAST_LBA);
            /* B5b 修复 (2026-09-04): GPT first_lba 是磁盘相对 LBA (磁盘起点=物理 0x2000 =
             * BM gpt_offset = erase_group 4096 × BOOT_SECTOR_IDX 2)，物理偏移须 +0x2000。
             * 原实现漏加 → OTA 写 ssystem_b/vbmeta-ssdk_b 偏前 0x2000 → t_loader 方案B
             * 验真 B 槽读到旧内容 → verify FAIL 回滚。GPT 表本身仍读 OTA_GPT_MIRROR_OFFSET。 */
            *flash_off = (uint32_t)(first_lba * 512u) + OTA_GPT_MIRROR_OFFSET;
            if (last_lba <= first_lba) return 0;
            return (uint32_t)((last_lba - first_lba + 1) * 512u);
        }
    }
    return 0;
}

/* ---- 响应发送（与 sys_monitor_msg.c send_response 同构） ---- */
static iccmsg_t *g_ota_ept = NULL;   /* B1b: 独立 OTA ept ("ota"@502); NULL=回退 sysmgr ept */

void sys_monitor_ota_set_ept(void *ept)
{
    g_ota_ept = (iccmsg_t *)ept;
}

static void ota_send_response(uint8_t msg_type, uint32_t seq,
                              const void *payload, uint16_t payload_len)
{
    iccmsg_t *ept = g_ota_ept ? g_ota_ept : sys_monitor_get_ept();
    if (ept == NULL) return;

    sysmgr_hdr_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.msg_type    = msg_type;
    hdr.payload_len = payload_len;
    hdr.seq         = seq;

    uint8_t buf[sizeof(sysmgr_hdr_t) + 64];
    if (payload_len > sizeof(buf) - sizeof(sysmgr_hdr_t)) {
        printf("[sys_monitor] ota: response too large (%u)\n", payload_len);
        return;
    }
    memcpy(buf, &hdr, sizeof(hdr));
    if (payload && payload_len > 0)
        memcpy(buf + sizeof(hdr), payload, payload_len);
    iccmsg_send(ept, 0, 0, buf, sizeof(hdr) + payload_len, ICCMSG_ACK_NO_NEED);
}

/* ==================== flash 写入辅助 (从 DDR 缓存) ==================== */

/** 从 DDR 缓存擦写一个 flash 分区（分批大闭包）。
 *  每 OTA_CLOSURE_BATCH (64KB) 一次大闭包 (单次进 RAM + 关中断 ~0.5s),
 *  批间 osDelay 恢复 tick/中断 (避免整区 9.6s 单次停摆损害 FreeRTOS)。返回 0=成功 */
#define OTA_CLOSURE_BATCH  (64u * 1024u)
static int ota_flash_write_part(uint32_t flash_off, const uint8_t *cache,
                                uint32_t size)
{
    uint32_t off;
    for (off = 0; off < size; off += OTA_CLOSURE_BATCH) {
        uint32_t chunk = size - off;
        if (chunk > OTA_CLOSURE_BATCH) chunk = OTA_CLOSURE_BATCH;
        if (flash_test_program_range(flash_off + off, cache + off, chunk) != 0) {
            printf("[sys_monitor] OTA: program range FAILED @0x%x\n", flash_off + off);
            return -1;
        }
        osDelay(1);   /* 批间恢复 tick/中断处理 */
        sys_monitor_wdt_ping();  /* [WDT fix] 每批写入后喟狗: 64KB×~0.5s×9批≈4.5s+verify≈9s 可超 WDT1 10s */
    }
    return 0;
}

/** 回读 CRC 校验一个 flash 分区。返回 0=匹配 */
static int ota_flash_verify_crc(uint32_t flash_off, uint32_t size, uint32_t expect)
{
    static uint8_t rb[OTA_VERIFY_CHUNK];
    uint32_t off, crc = 0;
    for (off = 0; off < size; off += OTA_VERIFY_CHUNK) {
        uint32_t sz = size - off;
        if (sz > OTA_VERIFY_CHUNK) sz = OTA_VERIFY_CHUNK;
        ota_gpt_xip_read(flash_off + off, rb, sz);
        crc = sd_crc32(crc, rb, sz);   /* 镜像数据: sd_crc32 (与 A 核一致) */
        sys_monitor_wdt_ping();  /* [WDT fix] 回读校验也可能耗时, 每块喟狗 */
    }
    return (crc == expect) ? 0 : -1;
}

/* ---- GPT 写公共 helper (ota_switch_gpt / mark_good 共用) ---- */

/** 组 GPT sector (改 attr + 双 CRC) 并 RAM STIG 写回 primary GPT (0x2000 4096B)。
 * entries 已含更新后的 ssystem_a/b attr；off_a/off_b 为 entries 内偏移。返回 0=成功 */
static int ota_gpt_commit(uint8_t *entries, uint32_t off_a, uint32_t off_b)
{
    static uint8_t sector[4096];
    memset(sector, 0, sizeof(sector));
    ota_gpt_xip_read(OTA_GPT_MIRROR_OFFSET, sector, 4096);
    memcpy(sector + OTA_GPT_ENTRIES_OFF + off_a,
           entries + off_a, OTA_GPT_ENTRY_SIZE);
    memcpy(sector + OTA_GPT_ENTRIES_OFF + off_b,
           entries + off_b, OTA_GPT_ENTRY_SIZE);
    *(uint32_t *)(sector + 0x200 + 88) =
        (uint32_t)crc32(0, entries, OTA_GPT_ENTRIES_SIZE);
    *(uint32_t *)(sector + 0x200 + 16) = 0;
    *(uint32_t *)(sector + 0x200 + 16) =
        (uint32_t)crc32(0, sector + 0x200, 92);

    if (flash_test_program_range(OTA_GPT_MIRROR_OFFSET, sector, 4096) != 0) {
        printf("[sys_monitor] OTA: GPT write FAILED\n");
        return -1;
    }
    __asm volatile("isb" ::: "memory");
    arch_invalidate_cache_range(OTA_OSPI2_XIP_BASE + OTA_GPT_MIRROR_OFFSET, 4096);
    return 0;
}

/** 读 primary GPT entries 并定位 ssystem_a/b。entries 缓冲≥16KB。返回 0=成功 */
static int ota_gpt_load(uint8_t *entries, uint32_t *off_a, uint32_t *off_b)
{
    uint8_t hdr[92];
    uint32_t entry_lba;
    memset(hdr, 0, sizeof(hdr));
    if (ota_gpt_xip_read(OTA_GPT_HDR_OFFSET, hdr, sizeof(hdr)) != 0) return -1;
    if (memcmp(hdr, "EFI PART", 8) != 0) return -1;
    entry_lba = *(uint32_t *)(hdr + OTA_GPT_ENTRY_LBA_OFF);
    memset(entries, 0, OTA_GPT_ENTRIES_SIZE);
    ota_gpt_xip_read(OTA_GPT_MIRROR_OFFSET + entry_lba * 512u,
                     entries, OTA_GPT_ENTRIES_SIZE);
    if (ota_gpt_find_name(entries, "ssystem_a", off_a) ||
        ota_gpt_find_name(entries, "ssystem_b", off_b))
        return -1;
    return 0;
}

/** NOR GPT 切槽到 target (复刻 gpt_switch_slot_xip, 官方 PART_ATT_*)。返回 0=成功 */
static int ota_switch_gpt(uint8_t slot)
{
    static uint8_t entries[OTA_GPT_ENTRIES_SIZE];
    uint32_t off_a = 0, off_b = 0;
    uint64_t new_a, new_b, attr_a, attr_b;

    if (slot > 1) return -1;
    if (ota_gpt_load(entries, &off_a, &off_b) != 0) return -1;
    attr_a = *(uint64_t *)(entries + off_a + OTA_GPT_ENTRY_ATTR_OFF);
    attr_b = *(uint64_t *)(entries + off_b + OTA_GPT_ENTRY_ATTR_OFF);

    new_a = attr_a;
    new_b = attr_b;
    if (slot == 0) {
        new_a = (new_a | PART_ATT_PRIORITY_VAL | PART_ATT_ACTIVE_VAL
                        | PART_ATT_MAX_RETRY_COUNT_VAL)
                & ~((uint64_t)PART_ATT_SUCCESSFUL_VAL)
                & ~((uint64_t)PART_ATT_UNBOOTABLE_VAL);
        /* B5c+P1 (2026-09-04/05): 非目标槽不清不设 UNBOOTABLE——但历史遗留 unboot (pre-B5c
         * 标的) 需清掉: 切槽时旧槽=刚成功运行的槽 (内容良好), 应恢复为回滚候选。 */
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
    *(uint64_t *)(entries + off_a + OTA_GPT_ENTRY_ATTR_OFF) = new_a;
    *(uint64_t *)(entries + off_b + OTA_GPT_ENTRY_ATTR_OFF) = new_b;
    return ota_gpt_commit(entries, off_a, off_b);
}

/* S3.3 (2026-09-05): mark-good——重置运行槽 GPT retry/successful，防止 DIL
 * ALWAYS_ROLL_BACK_CHECK 因无 mark-good 4 次 boot 后自动回滚已成功升级的新槽。
 * A 核 app_sysmgr 确认 R 核 boot 成功后发 SYSMGR_MARK_GOOD 触发。返回 0=成功。 */
int sys_monitor_ota_mark_good(void)
{
    static uint8_t entries[OTA_GPT_ENTRIES_SIZE];
    uint32_t off_a = 0, off_b = 0;
    uint64_t attr;
    int active = sysmgr_running_slot();
    uint32_t off;

    if (active < 0)
        active = ota_get_active_slot();
    if (active < 0 || active > 1)
        return -1;
    if (ota_gpt_load(entries, &off_a, &off_b) != 0)
        return -1;
    off = (active == 0) ? off_a : off_b;
    attr = *(uint64_t *)(entries + off + OTA_GPT_ENTRY_ATTR_OFF);
    /* 置 successful + retry 满 (MAX_RETRY_COUNT_VAL)，清 unbootable (自愈标记)。
     * active 位/priority 不动。ALWAYS_ROLL_BACK_CHECK 只认 retry: 每次 boot 递减,
     * mark-good 恢复 → 永不到 0 不回滚 (A 核起来 = 本次 boot verify PASS = 真成功)。 */
    attr |= PART_ATT_SUCCESSFUL_VAL | PART_ATT_MAX_RETRY_COUNT_VAL;
    attr &= ~((uint64_t)PART_ATT_UNBOOTABLE_VAL);
    if (active == 0)
        *(uint64_t *)(entries + off_a + OTA_GPT_ENTRY_ATTR_OFF) = attr;
    else
        *(uint64_t *)(entries + off_b + OTA_GPT_ENTRY_ATTR_OFF) = attr;
    printf("[sys_monitor] MARK_GOOD: slot %s retry reset (GPT)\n",
           active == 0 ? "A" : "B");
    return ota_gpt_commit(entries, off_a, off_b);
}

/** 释放缓存并复位会话（任何结束路径调用） */
static void ota_session_reset(uint8_t fail)
{
    int i;
    for (i = 0; i < 2; i++) {
        if (g_ota.cache[i]) {
            ExtFree(g_ota.cache[i]);
            g_ota.cache[i] = NULL;
        }
        g_ota.cache_alloc[i] = 0;
        g_ota.img_size[i]    = 0;
        g_ota.img_crc[i]     = 0;
        g_ota.flash_off[i]   = 0;
    }
    g_ota.received_mask = 0;
    g_ota.processed     = 0;
    g_ota.ota_active    = 0;
    g_ota.error         = fail ? g_ota.error : 0;
    /* fail=1: 置 FAILED 供 A 侧轮询; fail=0: 保留调用方已设的终态 phase (DONE)。
     * 修复 2026-09-04: 原实现 fail=0 → IDLE 会把 finalize 成功后的 DONE 立即清掉,
     * A 侧 ota-client COMMIT 后必 poll_status 等 DONE → 永远 poll 不到 → STATUS timeout */
    if (fail)
        g_ota.phase = SYSMGR_OTA_PHASE_FAILED;
}

/* ==================== OTA worker（异步 finalize: 校验+擦写+切槽） ==================== */

static void ota_do_finalize(void)
{
    int i;
    uint8_t slot = g_ota.target_slot;
    uint32_t t_fin = osKernelGetTickCount();   /* COMMIT 受理时刻 */

    printf("[sys_monitor] OTA: finalize target slot %c (ssystem %uB + vbmeta %uB) @t=%ums\n",
           slot == 0 ? 'A' : 'B', g_ota.img_size[SYSMGR_OTA_IMAGE_SSYSTEM],
           g_ota.img_size[SYSMGR_OTA_IMAGE_VBMETA_SSDK], t_fin);

    /* ① DDR 缓存 CRC 双校验 */
    g_ota.phase = SYSMGR_OTA_PHASE_VERIFYING;
    for (i = 0; i < 2; i++) {
        uint32_t crc = 0, off;
        if (!g_ota.cache[i] || !g_ota.img_size[i]) {
            printf("[sys_monitor] OTA: image %d cache missing\n", i);
            g_ota.error = 6;
            ota_session_reset(1);
            return;
        }
        for (off = 0; off < g_ota.img_size[i]; off += OTA_VERIFY_CHUNK) {
            uint32_t sz = g_ota.img_size[i] - off;
            if (sz > OTA_VERIFY_CHUNK) sz = OTA_VERIFY_CHUNK;
            crc = sd_crc32(crc, g_ota.cache[i] + off, sz);  /* 镜像数据: sd_crc32 (与 A 核一致) */
        }
        if (crc != g_ota.img_crc[i]) {
            printf("[sys_monitor] OTA: DDR CRC MISMATCH img=%d 0x%08x != 0x%08x, abort\n",
                   i, crc, g_ota.img_crc[i]);
            g_ota.error = 2;
            ota_session_reset(1);   /* flash 未动 */
            return;
        }
    }
    printf("[sys_monitor] OTA: DDR CRC OK (%ums elapsed), writing flash...\n",
           osKernelGetTickCount() - t_fin);

    /* ② flash 擦写: ssystem_X 再 vbmeta-ssdk_X */
    g_ota.phase = SYSMGR_OTA_PHASE_WRITING;
    for (i = 0; i < 2; i++) {
        uint32_t t_w = osKernelGetTickCount();
        if (ota_flash_write_part(g_ota.flash_off[i], g_ota.cache[i],
                                 g_ota.img_size[i]) != 0) {
            printf("[sys_monitor] OTA: flash write img=%d FAILED (slot %c partially written)\n",
                   i, slot == 0 ? 'A' : 'B');
            g_ota.error = 5;
            ota_session_reset(1);   /* 活动槽不变; 非活动槽残留可下次重擦 */
            return;
        }
        printf("[sys_monitor] OTA: flash write img=%d done (%ums, %uB)\n",
               i, osKernelGetTickCount() - t_w, g_ota.img_size[i]);
    }

    /* ③ flash 回读 CRC 双校验 */
    g_ota.phase = SYSMGR_OTA_PHASE_VERIFY_FLASH;
    for (i = 0; i < 2; i++) {
        uint32_t t_v = osKernelGetTickCount();
        if (ota_flash_verify_crc(g_ota.flash_off[i], g_ota.img_size[i],
                                 g_ota.img_crc[i]) != 0) {
            printf("[sys_monitor] OTA: flash verify img=%d FAILED\n", i);
            g_ota.error = 7;
            ota_session_reset(1);
            return;
        }
        printf("[sys_monitor] OTA: flash verify img=%d OK (%ums)\n",
               i, osKernelGetTickCount() - t_v);
    }
    printf("[sys_monitor] OTA: flash verify OK, switching GPT to slot %c...\n",
           slot == 0 ? 'A' : 'B');

    /* ④ 切 NOR GPT */
    g_ota.phase = SYSMGR_OTA_PHASE_SWITCHING;
    {
        uint32_t t_s = osKernelGetTickCount();
        if (ota_switch_gpt(slot) != 0) {
            printf("[sys_monitor] OTA: GPT switch FAILED\n");
            g_ota.error = 4;
            ota_session_reset(1);
            return;
        }
        printf("[sys_monitor] OTA: GPT switch done (%ums)\n",
               osKernelGetTickCount() - t_s);
    }
    g_ota.phase = SYSMGR_OTA_PHASE_DONE;
    printf("[sys_monitor] OTA: DONE — active slot now %c, reboot required (total %ums since COMMIT)\n",
           slot == 0 ? 'A' : 'B', osKernelGetTickCount() - t_fin);
    ota_session_reset(0);
}

static void ota_worker_task(void *arg)
{
    (void)arg;
    for (;;) {
        uint32_t flags = osEventFlagsWait(g_ota_ev, OTA_EV_FINALIZE,
                                          osFlagsWaitAny, 1000);  /* 1s 周期: 兼会话超时检查 */
        if (flags & OTA_EV_FINALIZE) {
            ota_do_finalize();
        } else if (g_ota.ota_active) {
            /* 传输阶段超时 (A 核中止/失联): 释放缓存回 IDLE, 允许重试 */
            if ((osKernelGetTickCount() - g_ota.last_activity) > OTA_XFER_TIMEOUT_MS) {
                printf("[sys_monitor] OTA: session timeout (no progress %us), resetting\n",
                       OTA_XFER_TIMEOUT_MS / 1000);
                g_ota.error = 8;
                ota_session_reset(1);
            }
        }
    }
}

/* ==================== 命令分发（rx_task 上下文） ==================== */

int sys_monitor_ota_handle(const sysmgr_hdr_t *hdr, const uint8_t *payload)
{
    uint32_t seq = hdr->seq;

    switch (hdr->msg_type) {

    case SYSMGR_OTA_BEGIN: {
        const sysmgr_ota_begin_t *b = (const sysmgr_ota_begin_t *)payload;
        sysmgr_ota_begin_ack_t ack;
        uint8_t rej = SYSMGR_OTA_REJ_NONE;
        uint32_t part_size;
        /* 实际运行槽优先 (自址判定); GPT attr 可能因 FDA 回滚失真 → 回退 attr */
        int active = sysmgr_running_slot();
        if (active < 0)
            active = ota_get_active_slot();

        memset(&ack, 0, sizeof(ack));

        /* 校验（无需 target active 检查：同会话后续镜像 BEGIN 时 active 未变，
         * 但首镜像必须 != 活动槽；多镜像同 target 由 ota_active 会话跟踪） */
        if (hdr->payload_len < sizeof(sysmgr_ota_begin_t))
            rej = SYSMGR_OTA_REJ_SIZE_INVALID;
        else if (b->target_slot > 1 || b->image_id > SYSMGR_OTA_IMAGE_VBMETA_SSDK)
            rej = SYSMGR_OTA_REJ_SIZE_INVALID;
        else if (b->image_size == 0 || b->image_size > OTA_MAX_IMAGE_SIZE)
            rej = SYSMGR_OTA_REJ_SIZE_INVALID;
        else if (active < 0)
            rej = SYSMGR_OTA_REJ_NOT_READY;
        else if ((part_size = ota_resolve_slot_partition(b->target_slot, b->image_id,
                                                         &g_ota.flash_off[b->image_id])) == 0)
            rej = SYSMGR_OTA_REJ_NOT_READY;
        else if (b->image_size > part_size)
            rej = SYSMGR_OTA_REJ_SIZE_INVALID;

        if (rej == SYSMGR_OTA_REJ_NONE) {
            if (!g_ota.ota_active) {
                /* 全新会话: 首镜像 target 必须 != 活动槽 */
                if (b->target_slot == (uint8_t)active) {
                    rej = SYSMGR_OTA_REJ_TARGET_ACTIVE;
                } else if (b->image_id != SYSMGR_OTA_IMAGE_SSYSTEM) {
                    rej = SYSMGR_OTA_REJ_SIZE_INVALID;   /* 会话须从 ssystem 开始 */
                }
            } else {
                /* 会话中: 仅允许下一顺序镜像 (ssystem 已收满 → vbmeta) */
                if (g_ota.target_slot != b->target_slot ||
                    b->image_id <= g_ota.cur_image ||
                    !(g_ota.received_mask & (1u << g_ota.cur_image))) {
                    rej = SYSMGR_OTA_REJ_BUSY;
                }
            }
        }

        if (rej != SYSMGR_OTA_REJ_NONE) {
            ack.ack           = 0;
            ack.phase         = g_ota.phase;
            ack.reject_reason = rej;
            printf("[sys_monitor] OTA_BEGIN rejected (reason=%u)\n", rej);
            ota_send_response(SYSMGR_RESPONSE_OTA_BEGIN_ACK, seq, &ack, sizeof(ack));
            return 0;
        }

        /* 受理: 分配 DDR 缓存 (首镜像分配 1MB 容纳; 复用已有缓存) */
        if (g_ota.cache[b->image_id] == NULL) {
            uint32_t want = b->image_size;
            uint8_t *buf = (uint8_t *)ExtMalloc(want);
            if (buf == NULL) {
                printf("[sys_monitor] OTA: ExtMalloc(%u) FAILED\n", want);
                ack.ack = 0;
                ack.reject_reason = SYSMGR_OTA_REJ_BUSY;
                ota_send_response(SYSMGR_RESPONSE_OTA_BEGIN_ACK, seq, &ack, sizeof(ack));
                return 0;
            }
            g_ota.cache[b->image_id]      = buf;
            g_ota.cache_alloc[b->image_id] = want;
        }
        g_ota.target_slot = b->target_slot;
        g_ota.cur_image   = b->image_id;
        g_ota.img_size[b->image_id]  = b->image_size;
        g_ota.img_crc[b->image_id]   = b->crc32;
        g_ota.processed    = 0;
        g_ota.error        = 0;
        g_ota.ota_active   = 1;
        g_ota.phase        = SYSMGR_OTA_PHASE_RECEIVING;
        g_ota.last_activity = osKernelGetTickCount();

        ack.ack   = 1;
        ack.phase = SYSMGR_OTA_PHASE_RECEIVING;
        printf("[sys_monitor] OTA_BEGIN: img=%u slot %c size=%u -> DDR cache @t=%ums\n",
               b->image_id, b->target_slot == 0 ? 'A' : 'B', b->image_size,
               (uint32_t)osKernelGetTickCount());
        ota_send_response(SYSMGR_RESPONSE_OTA_BEGIN_ACK, seq, &ack, sizeof(ack));
        return 0;
    }

    case SYSMGR_OTA_DATA: {
        const sysmgr_ota_data_t *d = (const sysmgr_ota_data_t *)payload;
        sysmgr_ota_data_ack_t ack;
        uint8_t  img = g_ota.cur_image;
        uint32_t len;

        memset(&ack, 0, sizeof(ack));
        ack.phase = g_ota.phase;
        if (hdr->payload_len < sizeof(sysmgr_ota_data_t)) {
            ota_send_response(SYSMGR_RESPONSE_OTA_DATA_ACK, seq, &ack, sizeof(ack));
            return 0;
        }
        len = d->len;
        if (len > hdr->payload_len - sizeof(sysmgr_ota_data_t)) {
            ack.ack = 0;
            ota_send_response(SYSMGR_RESPONSE_OTA_DATA_ACK, seq, &ack, sizeof(ack));
            return 0;
        }

        if (!g_ota.ota_active ||
            g_ota.phase != SYSMGR_OTA_PHASE_RECEIVING ||
            g_ota.cache[img] == NULL ||
            d->offset != g_ota.processed ||
            g_ota.processed + len > g_ota.img_size[img]) {
            ack.ack = 0;
            ack.written = g_ota.processed;
            ota_send_response(SYSMGR_RESPONSE_OTA_DATA_ACK, seq, &ack, sizeof(ack));
            return 0;
        }

        /* DDR 缓存写入 (memcpy, 快, flash 零接触) */
        memcpy(g_ota.cache[img] + d->offset, d->data, len);
        g_ota.processed += len;
        g_ota.last_activity = osKernelGetTickCount();
        if (g_ota.processed == g_ota.img_size[img]) {
            g_ota.received_mask |= (1u << img);
            g_ota.phase = SYSMGR_OTA_PHASE_RECEIVED;
            printf("[sys_monitor] OTA: img=%u fully received (%u B) into DDR @t=%ums\n",
                   img, g_ota.processed, (uint32_t)osKernelGetTickCount());
        }

        ack.ack     = 1;
        ack.written = g_ota.processed;
        ota_send_response(SYSMGR_RESPONSE_OTA_DATA_ACK, seq, &ack, sizeof(ack));
        return 0;
    }

    case SYSMGR_OTA_COMMIT: {
        sysmgr_ota_commit_ack_t ack;
        memset(&ack, 0, sizeof(ack));
        ack.phase = g_ota.phase;

        /* 最终提交: 双镜像都收满才受理 */
        if (!g_ota.ota_active ||
            g_ota.received_mask != 0x3 ||
            g_ota.phase != SYSMGR_OTA_PHASE_RECEIVED) {
            ack.ack = 0;
            ack.active_slot = 0xFF;
            ota_send_response(SYSMGR_RESPONSE_OTA_COMMIT_ACK, seq, &ack, sizeof(ack));
            return 0;
        }
        ack.ack   = 1;
        ack.phase = SYSMGR_OTA_PHASE_VERIFYING;
        printf("[sys_monitor] OTA_COMMIT accepted, finalizing...\n");
        /* ACK 必须先于 osEventFlagsSet 发出：worker(prio 25) 被唤醒即抢占进入
         * finalize（分批大闭包关中断），若后发 ACK 会被延迟到 finalize 完成后才
         * 上线路 → A 核 ota-client 5s 超时误报 FAILED（2026-09-03 根因）。*/
        ota_send_response(SYSMGR_RESPONSE_OTA_COMMIT_ACK, seq, &ack, sizeof(ack));
        osEventFlagsSet(g_ota_ev, OTA_EV_FINALIZE);
        return 0;
    }

    case SYSMGR_OTA_STATUS: {
        sysmgr_ota_status_t st;
        memset(&st, 0, sizeof(st));
        st.phase     = g_ota.phase;
        st.error     = g_ota.error;
        st.processed = g_ota.processed;
        st.total     = g_ota.img_size[g_ota.cur_image];
        ota_send_response(SYSMGR_RESPONSE_OTA_STATUS, seq, &st, sizeof(st));
        return 0;
    }

    default:
        return -1;   /* 非 OTA 命令 */
    }
}

void sys_monitor_ota_init(void)
{
    memset((void *)&g_ota, 0, sizeof(g_ota));
    g_ota.phase = SYSMGR_OTA_PHASE_IDLE;

    g_ota_ev = osEventFlagsNew(NULL);
    if (g_ota_ev == NULL) {
        printf("[sys_monitor] OTA: event flags alloc FAILED\n");
        return;
    }
    osThreadAttr_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.name       = "sysmgr_ota";
    attr.priority   = 25;    /* 数字优先级（避免 osPriorityAboveNormal 钳位 31） */
    attr.stack_size = 4096;
    g_ota_worker = osThreadNew(ota_worker_task, NULL, &attr);
    if (g_ota_worker == NULL) {
        printf("[sys_monitor] OTA: worker task create FAILED\n");
        return;
    }
    printf("[sys_monitor] OTA module ready (staging, idle)\n");
}
