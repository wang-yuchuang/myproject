/**
 * @file sys_monitor.c
 * @brief System Monitor — 完整版（握手门控 + iccmsg "sysmgr" + monitor_task）
 *
 * 合并来源:
 *  - middleware/sysmgr v3: BOOT_READY 握手门控（g_ap_ready 事件 + sysmgr_*_ap_ready API）
 *  - examples/kunlun/application/sys_monitor: iccmsg_add("sysmgr") + monitor_task + 协议分发
 *
 * 行为要点:
 *  - WDT1 已启用（SYSMGR_WDT_ENABLE=1, timeout=10s, int_rst）: monitor_task 无条件喂狗
 *  - 握手门控: 等 A 核 BOOT_READY（echo_cb → sysmgr_signal_ap_ready）后才 iccmsg_add("sysmgr")
 *  - monitor_task: AP 心跳（PING）+ grace 模式 + R 核任务监管
 *  - g_ap_connected（心跳）与 g_ap_ready（握手）是两个独立机制
 *
 * 关联变更: openspec/changes/add-system-mgmt-service/
 * 移植记录: openspec/changes/fix-virtio-rpmsg-ns-deadlock (Phase 2 启用)
 */
#include "sys_monitor.h"
#include "sys_monitor_channel.h"
#include "sys_monitor_task.h"
#include "sys_monitor_ota.h"

#include <cmsis_os2.h>
#include <stdio.h>
#include <string.h>

#include "iccmsg.h"
#include "driver/watchdog.h"
#include "driver/kunlun/watchdog/sdrv_watchdog_drv.h"
#include "regs_base.h"
#include "irq_num.h"
#include <rpmsg.h>
#include <rpmsg_channel.h>
#include <rpmsg_service.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>

#ifdef CONFIG_REBOOT
#include "reboot_cfg.h"   /* boot_ap1() */
#endif

/* 配置参数（继承 v3；WDT1 已启用） */
/* WDT_TIMEOUT/HEARTBEAT_TIMEOUT/GRACE_PERIOD/MONITOR_TASK_PERIOD 复用 sysmgr_protocol.h 共享默认值，避免重复定义 */
#define SYSMGR_WDT_ENABLE           1     /* WDT1 已启用: monitor_task 无条件喂狗 (曾因握手等待期无人喂狗而禁用) */
#define SYSMGR_BOOT_TIMEOUT_SEC     120   /* BOOT_READY 等待超时 */

/* ---- 全局状态 ---- */

/** AP core connected flag（心跳机制，PING 设置，超时清除） */
bool g_ap_connected = false;

/* 握手状态（v3 保留） */
static bool                  g_ap_ready       = false;
static osEventFlagsId_t      g_ap_ready_event = NULL;
static osEventFlagsId_t      g_ept_ready_event = NULL;

static watchdog_device_t        g_wdt;
static sdrv_watchdog_device_t   g_sdrv_wdt = {
    .base    = APB_WDT1_BASE,
    .irq     = WDT1_OVFLOW_INTR_NUM,
    .int_rst = 1,
};

#if SYSMGR_WDT_ENABLE
#define wdt_init()   do { \
    if (sdrv_watchdog_init(&g_wdt, &g_sdrv_wdt) == 0) { \
        watchdog_set_timeout(&g_wdt, SYSMGR_WDT_TIMEOUT); \
        watchdog_start(&g_wdt); \
        watchdog_ping(&g_wdt); \
        printf("[sys_monitor] WDT1 started, timeout=%us\n", SYSMGR_WDT_TIMEOUT); \
    } else { \
        printf("[sys_monitor] ERROR: WDT1 init failed\n"); \
    } \
} while(0)
#define wdt_ping()   watchdog_ping(&g_wdt)

#else
#define wdt_init()   printf("[sys_monitor] WDT disabled\n")
#define wdt_ping()   ((void)0)
#endif

/* [ssdk-standalone-boot-mode WDT fix] 供 sys_monitor_ota.c 在长时 NOR STIG 操作循环中喂狗。
 * 背景: ota_flash_write_part 64KB/批×~9s 总写入期间关中断, WDT1(10s) 无法由 monitor_task
 * 正常喟狗 → B→A OTA 竞争失败(实测两例), 防回滚回退但 OTA 本身不可靠。
 * 注: 函数定义在 #if 块外 (始终导出), 内部 #if 控制实际行为 —
 *     避免 SYSMGR_WDT_ENABLE=0 时链接失败 (fresh clone 独立构建发现的 bug)。 */
void sys_monitor_wdt_ping(void)
{
#if SYSMGR_WDT_ENABLE
    wdt_ping();
#endif
}

static iccmsg_t *g_sysmgr_ept      = NULL;
#ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
static iccmsg_t *g_ota_ept         = NULL;   /* B1b: 独立 OTA ept ("ota"@502) */
#endif
static uint32_t  g_last_ping_tick  = 0;
static bool      g_grace_mode      = false;
static uint32_t  g_grace_start_tick = 0;

/* 前向声明 */
static void monitor_task(void *arg);
static void sys_monitor_init_task(void *arg);
static void sys_monitor_rx_task(void *arg);
#ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
static void sys_monitor_ota_rx_task(void *arg);   /* B1b */
#endif

/* === sysmgr 公共 API（v3 保留，rpmsg_echo.c 与 R 核应用依赖） === */

bool sysmgr_is_ap_ready(void) { return g_ap_ready; }

int sysmgr_wait_ap_ready(uint32_t timeout_ms)
{
    if (g_ap_ready) return 0;
    if (g_ap_ready_event == NULL) return 0;
    uint32_t ret = osEventFlagsWait(g_ap_ready_event, 0x1,
                                    osFlagsWaitAny, timeout_ms);
    return (ret & 0x1) ? 0 : -1;
}

int sysmgr_wait_ept_ready(uint32_t timeout_ms)
{
    if (g_ept_ready_event == NULL) return 0;
    uint32_t ret = osEventFlagsWait(g_ept_ready_event, 0x1,
                                    osFlagsWaitAny, timeout_ms);
    return (ret & 0x1) ? 0 : -1;
}

/* 由 echo_cb（BOOT_READY ×3）调用，设置 A 核就绪事件 */
void sysmgr_signal_ap_ready(void)
{
    g_ap_ready = true;
    if (g_ap_ready_event) {
        osEventFlagsSet(g_ap_ready_event, 0x1);
    }
}

/* === 主入口 === */

void sys_monitor_init(void)
{
    wdt_init();
    g_ap_ready_event = osEventFlagsNew(NULL);
    g_ept_ready_event = osEventFlagsNew(NULL);

    osThreadAttr_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.name       = "sysmon_init";
    attr.priority   = 25;   /* 修复: osPriorityAboveNormal(=32) 会被钳位到 31(系统最高)，改用数字优先级 25 */
    attr.stack_size = 4096;

    osThreadNew((void (*)(void *))sys_monitor_init_task, NULL, &attr);
    printf("[sys_monitor] deferred init started (background)\n");
}

/* 兼容旧入口（main.c 旧调用名） */
void sys_monitor_init_deferred(void)
{
    sys_monitor_init();
}

/* 初始化任务: 等 IPCC link → 等 BOOT_READY（握手门控）→ iccmsg_add("sysmgr") → monitor_task */
static void sys_monitor_init_task(void *arg)
{
    (void)arg;

    printf("[sys_monitor] Waiting for IPCC link...\n");
    for (int i = 0; i < 300; i++) {
        wdt_ping();
        struct rpmsg_instance *ins =
            rpmsg_get_instance(RPMSG_TYPE_IPCC, MBOX_RPROC_AP1);
        if (ins && rpmsg_is_link_up(ins)) {
            printf("[sys_monitor] IPCC link up after %dms\n", i * 100);
            break;
        }
        osDelay(100);
    }

    /* 握手门控: 等 A 核 BOOT_READY（echo_cb → sysmgr_signal_ap_ready） */
    printf("[sys_monitor] Waiting for A-core BOOT_READY (timeout=%us)...\n",
           SYSMGR_BOOT_TIMEOUT_SEC);
    uint32_t ev_ret      = 0;
    uint32_t slice_ticks = osKernelGetTickFreq();   /* 1s/片 */
    for (uint32_t waited = 0; waited < SYSMGR_BOOT_TIMEOUT_SEC; waited++) {
        wdt_ping();   /* 握手等待期间必须喂狗: 单次阻塞 120s 会使 WDT1(10s) 溢复位 */
        ev_ret = osEventFlagsWait(g_ap_ready_event, 0x1,
                                  osFlagsWaitAny, slice_ticks);
        if (ev_ret & 0x1) break;
    }
    if (ev_ret & 0x1) {
        printf("[sys_monitor] A-core ready (BOOT_READY received)\n");
    } else {
        printf("[sys_monitor] BOOT_READY timeout after %us\n", SYSMGR_BOOT_TIMEOUT_SEC);
    }

    /* Phase 2: 创建 iccmsg "sysmgr" endpoint（IPCC + VirtIO 双通道） */
    iccmsg_init();

    frame_conf_t frm;
    memset(&frm, 0, sizeof(frm));
    frm.checksum_type = CRC32;

    g_sysmgr_ept = iccmsg_add("sysmgr", frm, NULL, NULL);  /* cb=NULL: 框架 ipcc_rx_cb 不自动调用(iccmsg.c:604)，由 rx_task 消费 rx_queue */
    if (g_sysmgr_ept == NULL) {
        printf("[sys_monitor] ERROR: iccmsg_add(\"sysmgr\") failed\n");
        /* Continue anyway */
    } else {
        printf("[sys_monitor] iccmsg endpoint \"sysmgr\" created\n");
        #ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
        /* OTA 模块必须先于 rx_task 初始化（OTA_BEGIN 可能紧随握手到达，
         * 需 g_ota_ev/worker 已就绪，否则 osEventFlagsSet(NULL) 崩溃） */
        sys_monitor_ota_init();
#endif
        /* 启动 rx 消费任务（协议分发入口） */
        osThreadAttr_t rattr;
        memset(&rattr, 0, sizeof(rattr));
        rattr.name       = "sysmgr_rx";
        rattr.priority   = 25;   /* 修复: 数字优先级 25 (避免 osPriorityAboveNormal 钳位到 31) */
        rattr.stack_size = 6144;   /* 修复 2026-09-03: 2048 溢出——OTA finalize 后积压的
                                    * 心跳 PING+STATUS 轮询在 rx 任务爆发处理(printf/send_response)
                                    * 导致 StackOverFlow (img0 写完后崩, hook exit 1) */
        osThreadNew(sys_monitor_rx_task, NULL, &rattr);
    }

    #ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
    /* B1b (2026-09-03): 独立 OTA endpoint "ota"（ept 502, 偶数递增）。
     * ota-client 走此通道 ↔ app_sysmgr 心跳仍占 sysmgr@500 —— 避免双进程同通道
     * 触发 rpmsg_chrdev 排他绑定互斥（后者 unbind+rebind 会销毁前者 eptdev）。 */
    g_ota_ept = iccmsg_add("ota", frm, NULL, NULL);
    if (g_ota_ept == NULL) {
        printf("[sys_monitor] ERROR: iccmsg_add(\"ota\") failed\n");
    } else {
        printf("[sys_monitor] iccmsg endpoint \"ota\" created\n");
        sys_monitor_ota_set_ept(g_ota_ept);   /* OTA 响应经本 ept 回发 */
        osThreadAttr_t oattr;
        memset(&oattr, 0, sizeof(oattr));
        oattr.name       = "ota_rx";
        oattr.priority   = 25;
        oattr.stack_size = 6144;   /* 与 sysmgr_rx 同级（OTA handler 含 printf） */
        osThreadNew(sys_monitor_ota_rx_task, NULL, &oattr);
    }
#else
    (void)frm;
#endif

    /* sysmgr/ota 已固定占用 500/502, 通知业务侧(如 BMS)现在才可分配其余 ept */
    if (g_ept_ready_event) {
        osEventFlagsSet(g_ept_ready_event, 0x1);
    }

    /* 通道状态表 + 任务监管表 */
    sys_monitor_channel_init();
    sys_monitor_task_init();

    /* 启动 monitor_task */
    osThreadAttr_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.name       = "sys_monitor";
    attr.priority   = 25;   /* 修复: 数字优先级 25 */
    attr.stack_size = 4096;
    osThreadNew(monitor_task, NULL, &attr);

    printf("[sys_monitor] initialized\n");
    osThreadExit();   /* 修复: 任务函数不能 return (configTASK_RETURN_ADDRESS=NULL → 跳0崩溃 → prefetch abort → 全核硬锁),
                       * 必须 osThreadExit 结束任务 (仿 board.c:141 惯例) */
    /* NOTREACHED */
    for (;;) { osDelay(1000); }   /* 保险: 万一 osThreadExit 不返回 */
}


/* monitor_task: WDT feed + AP 心跳 + grace 模式 + R 核任务监管 */
static void monitor_task(void *arg)
{
    (void)arg;

    for (;;) {
        uint32_t now          = osKernelGetTickCount();
        uint32_t tick_per_sec = osKernelGetTickFreq();

        /* 1. WDT feed（monitor loop 每轮无条件喂狗） */
        wdt_ping();

        /* 2. AP heartbeat check（仅已连接且非 grace 时检查超时） */
        if (g_ap_connected && !g_grace_mode) {
            uint32_t elapsed = (now - g_last_ping_tick) / tick_per_sec;
            if (elapsed >= SYSMGR_HEARTBEAT_TIMEOUT) {
                printf("[sys_monitor] AP heartbeat timeout (%us), kicking AP1\n",
                       elapsed);
                g_ap_connected = false;
                sys_monitor_channel_reset_all();
#ifdef CONFIG_REBOOT
                boot_ap1();
#endif
                osDelay(SYSMGR_MONITOR_TASK_PERIOD * 1000);
                continue;
            }
        }

        /* 3. Grace mode timeout check */
        if (g_grace_mode) {
            uint32_t grace_elapsed = (now - g_grace_start_tick) / tick_per_sec;
            if (grace_elapsed >= SYSMGR_GRACE_PERIOD) {
                printf("[sys_monitor] Grace period expired, kicking AP1\n");
                g_grace_mode = false;
                sys_monitor_channel_reset_all();
#ifdef CONFIG_REBOOT
                boot_ap1();
#endif
                osDelay(SYSMGR_MONITOR_TASK_PERIOD * 1000);
                continue;
            }
        }

        /* 4. R-core task heartbeat check */
        sys_monitor_task_check(now, tick_per_sec);

        osDelay(SYSMGR_MONITOR_TASK_PERIOD * 1000);
    }
}

/* ---- 心跳/grace API（完整版） ---- */

void sys_monitor_on_ping(void)
{
    g_last_ping_tick = osKernelGetTickCount();
    if (!g_ap_connected) {
        g_ap_connected = true;
        printf("[sys_monitor] AP connected (first PING received)\n");
        /* If we were in grace mode, exit it */
        if (g_grace_mode) {
            g_grace_mode = false;
            printf("[sys_monitor] Exiting grace mode (AP reconnected)\n");
        }
    }
}

void sys_monitor_enter_grace(void)
{
    g_grace_mode       = true;
    g_grace_start_tick = osKernelGetTickCount();
    printf("[sys_monitor] Entering grace mode (%us)\n", SYSMGR_GRACE_PERIOD);
}

void sys_monitor_exit_grace(void)
{
    if (g_grace_mode) {
        g_grace_mode = false;
        printf("[sys_monitor] Exiting grace mode (AP reconnected)\n");
    }
}

iccmsg_t *sys_monitor_get_ept(void)
{
    return g_sysmgr_ept;
}

/* rx_task: 消费 iccmsg rx_queue → 协议分发（iccmsg 框架 ipcc_rx_cb 不自动调用，oracle 审核 P1 修复） */
static void sys_monitor_rx_task(void *arg)
{
    (void)arg;
    for (;;) {
        if (g_sysmgr_ept == NULL) {
            osDelay(100);
            continue;
        }
        iccmsg_rx_msg_t *rx_msg = NULL;
        if (osMessageQueueGet(g_sysmgr_ept->rx_queue, &rx_msg, NULL, osWaitForever) == osOK && rx_msg) {
            sys_monitor_rx_cb(NULL, rx_msg->data, rx_msg->len);
            ExtFree(rx_msg);
        }
    }
}

#ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
/* ota_rx_task (B1b): 消费独立 OTA ept rx_queue → 仅分发 OTA 命令。
 * 数据格式同 sysmgr: sysmgr_hdr_t + payload。非 OTA 命令在此 ept 丢弃。 */
static void sys_monitor_ota_rx_task(void *arg)
{
    (void)arg;
    for (;;) {
        if (g_ota_ept == NULL) {
            osDelay(100);
            continue;
        }
        iccmsg_rx_msg_t *rx_msg = NULL;
        if (osMessageQueueGet(g_ota_ept->rx_queue, &rx_msg, NULL, osWaitForever) == osOK && rx_msg) {
            if (rx_msg->data && rx_msg->len >= sizeof(sysmgr_hdr_t)) {
                const sysmgr_hdr_t *hdr = (const sysmgr_hdr_t *)rx_msg->data;
                const uint8_t *payload = (const uint8_t *)rx_msg->data + sizeof(sysmgr_hdr_t);
                sys_monitor_ota_handle(hdr, payload);   /* OTA 命令分发 (0=已处理/-1=忽略) */
            }
            ExtFree(rx_msg);
        }
    }
}
#endif
uint32_t sys_monitor_get_wdt_timeleft(void)
{
    uint32_t timeleft = 0;
#if SYSMGR_WDT_ENABLE
    watchdog_get_timeleft(&g_wdt, &timeleft);
#endif
    return timeleft;
}
