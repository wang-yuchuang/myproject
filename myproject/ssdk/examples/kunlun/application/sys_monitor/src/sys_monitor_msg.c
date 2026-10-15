/**
 * @file sys_monitor_msg.c
 * @brief System Monitor — iccmsg rx callback and command dispatch
 *
 * Dispatches incoming sysmgr protocol messages (GET_VERSION, PING,
 * PLANNED_REBOOT, GET_RCORE_STATUS, etc.) and builds responses.
 *
 * 关联变更: openspec/changes/add-system-mgmt-service/
 */

#include "sys_monitor.h"
#include "sys_monitor_channel.h"
#include "sys_monitor_task.h"
#include "sys_monitor_ota.h"
#include "sysmgr_protocol.h"
#include "ssdk_version.h"

#include <cmsis_os2.h>
#include <string.h>
#include <stdio.h>

#include "iccmsg.h"
#include <arch/cache.h>
#include <reg.h>

/* ---- NOR GPT XIP direct 读取（复刻 gpt_slot_switch.c，绕开 cospi_lock） ---- */
#ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
#define GPT_MIRROR_OFFSET      (0x2000)      /* GPT 镜像烧录起点 */
#define GPT_HDR_OFFSET         (GPT_MIRROR_OFFSET + 0x200)   /* "EFI PART" header */
#define GPT_ENTRY_LBA_OFFSET   72            /* header 内 partition_entry_lba */
#define GPT_ENTRY_SIZE         128           /* partition entry 大小 */
#define GPT_ENTRY_NAME_OFF     56            /* entry 内名字（UTF16LE）*/
#define GPT_ENTRY_ATTR_OFF     48            /* entry 内 attribute_flag */
#define GPT_ACTIVE_BIT         50            /* PART_ATT_ACTIVE_BIT */
#endif  /* NOR GPT layout (XIP only) */
#define OSPI2_XIP_BASE         0x08000000u   /* OSPI2 AHB 窗 */

/* XIP direct 读 GPT。返回 0=成功 */
#ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
static int sysmgr_gpt_xip_read(uint32_t flash_off, void *dst, uint32_t size)
{
    uint32_t addr = OSPI2_XIP_BASE + flash_off;
    arch_clean_invalidate_cache_range((addr_t)addr, ROUNDUP(size, 32));
    memcpy(dst, (void *)(uintptr_t)addr, size);
    return 0;
}
#endif  /* sysmgr_gpt_xip_read (XIP only) */

/* 读 NOR GPT active slot (0=A, 1=B)。失败返回 -1 */
#ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
static int sysmgr_get_active_slot(void)
{
    uint8_t hdr[92];
    uint32_t entry_lba, i;
    uint64_t attr_a = 0, attr_b = 0;

    memset(hdr, 0, sizeof(hdr));
    if (sysmgr_gpt_xip_read(GPT_HDR_OFFSET, hdr, sizeof(hdr)) != 0) return -1;
    if (memcmp(hdr, "EFI PART", 8) != 0) return -1;
    entry_lba = *(uint32_t *)(hdr + GPT_ENTRY_LBA_OFFSET);

    /* 扫描 entries 找 ssystem_a / ssystem_b */
    for (i = 0; i < 128; i++) {
        uint8_t entry[GPT_ENTRY_SIZE];
        char name[37];
        unsigned j, n = 0;
        sysmgr_gpt_xip_read(GPT_MIRROR_OFFSET + entry_lba * 512u + i * GPT_ENTRY_SIZE,
                            entry, GPT_ENTRY_SIZE);
        for (j = 0; j < 36 && n < 36; j += 2) {
            uint16_t c = entry[GPT_ENTRY_NAME_OFF + j] | (entry[GPT_ENTRY_NAME_OFF + j + 1] << 8);
            if (c == 0 || c >= 0x80) break;
            name[n++] = (char)c;
        }
        name[n] = 0;
        if (strcmp(name, "ssystem_a") == 0) {
            attr_a = *(uint64_t *)(entry + GPT_ENTRY_ATTR_OFF);
        } else if (strcmp(name, "ssystem_b") == 0) {
            attr_b = *(uint64_t *)(entry + GPT_ENTRY_ATTR_OFF);
        }
    }
    /* active bit 判断 */
    if ((attr_a & (1ULL << GPT_ACTIVE_BIT)) && !(attr_b & (1ULL << GPT_ACTIVE_BIT))) return 0;
    if ((attr_b & (1ULL << GPT_ACTIVE_BIT)) && !(attr_a & (1ULL << GPT_ACTIVE_BIT))) return 1;
    return -1;   /* 无有效 active 槽 */
}
#endif  /* CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP (NOR GPT active-slot read) */


/* ---- 实际运行槽判定 (2026-09-03 根因修复) ----
 * 本固件双链接 XIP (MEMBASE_A=0x081EB000 / MEMBASE_B=0x082DB000)。
 * NOR GPT attr 只是"上次切换"标记 —— FDA 回滚后可实际从另一槽启动且不持久化 attr
 * (曾出现 attr=B 而实际运行 A: hook 按 attr 取 target=1-active=A = 正在执行槽,
 * R 核擦写自执行 XIP 区 → 取指 0xFF 永久挂死)。
 * 以本代码自身执行地址判定真正运行的槽: 运行槽恒不可作为 OTA target。
 * 返回 0=A / 1=B / -1=非 XIP 窗口 (未知)。 */
#define SSYS_MEMBASE_A   0x081EB000u
#define SSYS_MEMBASE_B   0x082DB000u
#define SSYS_SLOT_SIZE   0x000F0000u     /* 960KB (ospi2.bpt ssystem ab) */
int sysmgr_running_slot(void)
{
    uintptr_t self = (uintptr_t)&sysmgr_running_slot;
    if (self >= SSYS_MEMBASE_A && self < SSYS_MEMBASE_A + SSYS_SLOT_SIZE) return 0;
    if (self >= SSYS_MEMBASE_B && self < SSYS_MEMBASE_B + SSYS_SLOT_SIZE) return 1;
    return -1;
}
/* ---- Helpers ---- */

/**
 * 填充 R 核版本串。主路径取构建期统一版本 SSDK_VERSION (auto_version.sh 生成,
 * 形如 "D9360-bmu05_t00-dev-<sha8>", 总长 ≤63 适配 version[64]);
 * SSDK_VERSION 为空时回退 "unknown"。
 */
static void sysmgr_get_build_version(char *out, size_t n)
{
    size_t len;

    if (out == NULL || n == 0)
        return;

    memset(out, 0, n);

    /* 主路径: 构建期统一版本串 (≤63B, 断言见 auto_version.sh) */
    if (SSDK_VERSION[0] != '\0') {
        len = 0;
        while (len < n - 1 && SSDK_VERSION[len] != '\0')
            len++;
        memcpy(out, SSDK_VERSION, len);
        out[len] = '\0';
        return;
    }

    /* 防御性回退: SSDK_VERSION 为空时返回 "unknown" */
    strncpy(out, "unknown", n - 1);
    out[n - 1] = '\0';
}

/* send_response 共享缓冲上限: 最大载荷为 sysmgr_version_t (version[64]);
 * rcore_status 等其余载荷均不大于它。 */
_Static_assert(sizeof(sysmgr_version_t) >= sizeof(sysmgr_rcore_status_t),
               "sysmgr_version_t must be at least as large as sysmgr_rcore_status_t");

/**
 * Send a response: header + payload in a single iccmsg frame.
 *
 * @param msg_type  Response message type (SYSMGR_RESPONSE_*)
 * @param seq       Sequence number from the request
 * @param payload   Pointer to payload struct (may be NULL if payload_len==0)
 * @param payload_len  Size of payload in bytes
 */
static void send_response(uint8_t msg_type, uint32_t seq,
                          const void *payload, uint16_t payload_len)
{
    iccmsg_t *ept = sys_monitor_get_ept();
    if (ept == NULL) {
        printf("[sys_monitor] send_response: no endpoint\n");
        return;
    }

    sysmgr_hdr_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.msg_type    = msg_type;
    hdr.payload_len = payload_len;
    hdr.seq         = seq;

    /* Build contiguous buffer: header + payload */
    uint32_t total = sizeof(sysmgr_hdr_t) + payload_len;
    uint8_t buf[sizeof(sysmgr_hdr_t) + sizeof(sysmgr_version_t)];
    if (total > sizeof(buf)) {
        printf("[sys_monitor] send_response: payload too large (%u)\n", total);
        return;
    }

    memcpy(buf, &hdr, sizeof(hdr));
    if (payload && payload_len > 0)
        memcpy(buf + sizeof(hdr), payload, payload_len);

    iccmsg_send(ept, 0, 0, buf, total, ICCMSG_ACK_NO_NEED);
}

/* PLANNED_REBOOT 速率限制 (Oracle Area 3): 10 分钟内最多接受 1 次。
 * 用 osKernelGetTickCount() 差值判定, uint32 减法天然处理 tick 回绕。 */
#define SYSMGR_PLANNED_REBOOT_MIN_INTERVAL_SEC (10 * 60)
static uint32_t g_reboot_last_accept_tick = 0;
static bool     g_reboot_accept_valid     = false;

/* ---- Main rx callback ---- */

int sys_monitor_rx_cb(void *user_ctx, const void *data, uint32_t len)
{
    (void)user_ctx;

    if (data == NULL || len < sizeof(sysmgr_hdr_t)) {
        printf("[sys_monitor] rx_cb: bad msg (len=%u)\n", len);
        return -1;
    }

    const sysmgr_hdr_t *hdr     = (const sysmgr_hdr_t *)data;
    const uint8_t      *payload = (const uint8_t *)data + sizeof(sysmgr_hdr_t);

#ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
    /* OTA 命令分发（独立模块: 擦除/STIG 写/CRC/切槽）— XIP NOR 专属 */
    if (sys_monitor_ota_handle(hdr, payload) == 0)
        return 0;
#endif

    switch (hdr->msg_type) {

    case SYSMGR_GET_VERSION: {
        sysmgr_version_t ver;
        memset(&ver, 0, sizeof(ver));
        sysmgr_get_build_version(ver.version, sizeof(ver.version));
#ifdef CONFIG_BOARD_NAME_STR
        strncpy(ver.board_name, CONFIG_BOARD_NAME_STR, sizeof(ver.board_name) - 1);
#endif
        send_response(SYSMGR_RESPONSE_VERSION, hdr->seq,
                      &ver, sizeof(ver));
        printf("[sys_monitor] GET_VERSION -> %s (board=%s)\n",
               ver.version, ver.board_name);
        break;
    }

    case SYSMGR_PING: {
        /* Update heartbeat / connection state */
        sys_monitor_on_ping();

        sysmgr_pong_t pong;
        memset(&pong, 0, sizeof(pong));
        pong.rcore_uptime_ms = (uint32_t)(osKernelGetTickCount() * 1000 / osKernelGetTickFreq());
        send_response(SYSMGR_RESPONSE_PONG, hdr->seq,
                      &pong, sizeof(pong));
        break;
    }

    case SYSMGR_GET_READY_STATUS: {
        sysmgr_ready_status_t st;
        memset(&st, 0, sizeof(st));
        st.ready          = 1;              /* TODO: check actual endpoint readiness */
        st.endpoint_count = (uint8_t)get_iccmsg_num();
        send_response(SYSMGR_RESPONSE_READY_STATUS, hdr->seq,
                      &st, sizeof(st));
        break;
    }

    case SYSMGR_PLANNED_REBOOT: {
        uint32_t now     = osKernelGetTickCount();
        uint32_t tick_hz = osKernelGetTickFreq();
        bool     accept  = true;

        /* 速率限制 (Oracle Area 3): 10 分钟内最多接受 1 次, 超出则拒绝且不进入 grace */
        if (g_reboot_accept_valid &&
            (uint32_t)((now - g_reboot_last_accept_tick) / tick_hz) <
                SYSMGR_PLANNED_REBOOT_MIN_INTERVAL_SEC) {
            accept = false;
        }

        sysmgr_reboot_ack_t ack;
        memset(&ack, 0, sizeof(ack));

        if (accept) {
            g_reboot_last_accept_tick = now;
            g_reboot_accept_valid     = true;
            sys_monitor_enter_grace();
            ack.ack               = 1;
            ack.grace_period_sec  = SYSMGR_GRACE_PERIOD;
        } else {
            /* 拒绝: 不进入 grace 模式, ack=0 通知 A 核 */
            ack.ack               = 0;
            ack.grace_period_sec  = 0;
            printf("[sys_monitor] PLANNED_REBOOT 速率异常, rejected (within %ds)\n",
                   SYSMGR_PLANNED_REBOOT_MIN_INTERVAL_SEC);
        }

        send_response(SYSMGR_RESPONSE_REBOOT_ACK, hdr->seq,
                      &ack, sizeof(ack));
        break;
    }

    case SYSMGR_GET_RCORE_STATUS: {
        iccmsg_t *ept = sys_monitor_get_ept();

        sysmgr_rcore_status_t st;
        memset(&st, 0, sizeof(st));
        st.boot_reason    = SYSMGR_BOOT_UNKNOWN;
        st.ssdk_ready     = 1;
        st.endpoint_count = (uint8_t)get_iccmsg_num();
        st.uptime_ms      = (uint32_t)(osKernelGetTickCount() * 1000 / osKernelGetTickFreq());

        /* Memory stats (FreeRTOS) */
#ifdef CONFIG_OS_FREERTOS
        {
            extern size_t xPortGetFreeHeapSize(void);
            extern size_t xPortGetMinimumEverFreeHeapSize(void);
            st.heap_free = (uint32_t)xPortGetFreeHeapSize();
            st.heap_min  = (uint32_t)xPortGetMinimumEverFreeHeapSize();
        }
#endif

        /* WDT time-left */
        st.wdt_timeleft_sec = sys_monitor_get_wdt_timeleft();

        /* iccmsg stats from our endpoint */
        if (ept) {
            st.iccmsg_tx_cnt  = ept->stats.tx_cnt;
            st.iccmsg_rx_cnt  = ept->stats.rx_cnt;
            st.iccmsg_err_cnt = ept->stats.ack_timeout_cnt
                               + ept->stats.reassembly_fail_cnt
                               + ept->stats.crc_fail_cnt;
        }

        /* Task supervision stats */
        st.monitored_task_count = sys_monitor_task_get_monitored_count();
        st.task_crash_total     = (uint8_t)sys_monitor_task_get_crash_total();

        send_response(SYSMGR_RESPONSE_RCORE_STATUS, hdr->seq,
                      &st, sizeof(st));
        break;
    }

    case SYSMGR_CHANNEL_STATUS: {
        if (hdr->payload_len >= sizeof(sysmgr_channel_status_t)) {
            const sysmgr_channel_status_t *cs =
                (const sysmgr_channel_status_t *)payload;
            sys_monitor_channel_update(cs->name, cs->connected ? true : false);
        }

        sysmgr_channel_ack_t ack;
        memset(&ack, 0, sizeof(ack));
        ack.ack = 1;
        send_response(SYSMGR_RESPONSE_CHANNEL_ACK, hdr->seq,
                      &ack, sizeof(ack));
        break;
    }

    case SYSMGR_GET_SLOT_STATUS: {
        sysmgr_slot_status_t st;
        memset(&st, 0, sizeof(st));

        /* 实际运行槽（自址判定，优先）——GPT attr 可能因 FDA 回滚与实际不符,
         * A 核 hook 据此定 target=1-active 恒指向非运行槽 (根因修复 2026-09-03) */
        int slot = sysmgr_running_slot();
#ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
        if (slot < 0)   /* 非 XIP 窗口: 回退 attr 读 */
            slot = sysmgr_get_active_slot();
        if (slot >= 0) {
            st.active_slot = (uint8_t)slot;
            st.retry_count = 0xFF;   /* retry 计数暂不可读，填 0xFF */
            /* NOR GPT 可读 => 本固件为 NOR XIP 启动 (A 核据此分流 hook 模式) */
            st.boot_media  = SYSMGR_BOOT_MEDIA_NOR_XIP;
        } else {
            st.active_slot = 0xFF;   /* 读取失败 */
            st.retry_count = 0xFF;
            st.boot_media  = SYSMGR_BOOT_MEDIA_UNKNOWN;
            printf("[sys_monitor] GET_SLOT_STATUS: GPT read failed\n");
        }
#else  /* CONFIG_SYSMON_BOOT_MEDIA_EMMC: R5 由 FDA 载 OCRAM 执行, 无 NOR GPT;
          * eMMC 槽/GPT 由 A 核 RAUC 管理. running_slot 自址在 OCRAM 恒 -1. */
        st.active_slot = 0xFF;
        st.retry_count = 0xFF;
        st.boot_media  = SYSMGR_BOOT_MEDIA_EMMC;
#endif
        send_response(SYSMGR_RESPONSE_SLOT_STATUS, hdr->seq,
                      &st, sizeof(st));
        break;
    }

    case SYSMGR_MARK_GOOD: {
        /* S3.3 (2026-09-05): A 核确认 R 核 boot 成功后重置运行槽 GPT retry/successful,
         * 防 ALWAYS_ROLL_BACK_CHECK 无 mark-good 时 4 次 boot 自动回滚新槽。 */
        sysmgr_mark_good_ack_t ack;
        memset(&ack, 0, sizeof(ack));
#ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
        int slot = sysmgr_running_slot();
        if (slot < 0)
            slot = sysmgr_get_active_slot();
        if (sys_monitor_ota_mark_good() == 0 && slot >= 0) {
            ack.ok = 1;
            ack.slot = (uint8_t)slot;
        } else {
            ack.ok = 0;
            ack.slot = 0xFF;
            printf("[sys_monitor] MARK_GOOD: GPT update failed\n");
        }
#else  /* emmc_only: R 核无 NOR GPT mark 职责 (A 核 S40rauc 操作 eMMC GPT);
          * 防御性 no-op 响应, 不触碰 NOR. */
        ack.ok = 1;
        ack.slot = 0xFF;
#endif
        send_response(SYSMGR_RESPONSE_MARK_GOOD, hdr->seq, &ack, sizeof(ack));
        break;
    }

    default:
        printf("[sys_monitor] Unknown msg_type 0x%02x\n", hdr->msg_type);
        break;
    }

    return 0;
}
