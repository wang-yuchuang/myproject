#include "uds_upgrade_manager.h"
#include "icc_upgrade_handler.h"
#include "can_uds_interface.h"
#include "can_uds_arch.h"
#include "can_uds_network_layer.h"
#include "DEVCAN.h"
#include <cmsis_os2.h>
#include <string.h>
#include <stdio.h>
#include <debug.h>

extern void uds_upgrade_start(void);
extern void uds_upgrade_reset(void);
extern void reset_uds_send_buf(void);
extern void uds_proc_tester(void);
extern void uds_set_upgrade_active(uint8_t active);
extern uint8_t uds_get_upgrade_step(void);
extern void ExtFree(void *pv);

#define CAN_TX_BASE_ID            0x0B110000  // 升级时本端(tester)发送 CAN 帧的基 ID，结合 dev_type/dev_addr 拼出各设备唯一 ID
#define CAN_RX_BASE_ID            0x0C110000  // 升级时接收目标设备应答 CAN 帧的基 ID

#define MAX_RETRY                 3            // 单个设备升级失败后的最大重试次数
#define PROGRESS_INTERVAL_MS      100          // 向主控上层上报升级进度的最小间隔(ms)，防止频繁刷爆 ICC 通道
#define COMM_TIMEOUT_MS           2000         // 判定目标设备"通信超时"的阈值(ms)，超过该时长未收到应答则标记超时
#define SILENCE_SEND_COUNT        3            // 静默/恢复帧重复发送次数，多次发送提高 CAN 总线接收可靠性

/* 升级总协调器的顶层状态机： */
enum {
    MANAGER_STATE_IDLE = 0,      /* 空闲：等待 icc_upgrade_handler 收完文件进入 STATE_UPGRADING 后启动升级 */
    MANAGER_STATE_PREPARE_DEV,   /* 设备准备：切换 CAN 通道/收发 ID、发静默帧、启动 UDS 流程，随后转入运行态 */
    MANAGER_STATE_UDS_RUNNING,   /* UDS 运行：驱动 UDS 流程、判定成功/失败、重试、周期上报进度 */
    MANAGER_STATE_CLEANUP,       /* 清理：解除静默、释放文件缓冲、复位上下文后回到 IDLE */
};

/* 升级总协调器上下文，整个升级过程只有一份(g_mgr)： */
struct uds_manager_ctx {
    uint8_t state;           /* 顶层状态机当前状态(MANAGER_STATE_xxx) */
    uint8_t dev_idx;         /* 当前正在升级的设备在 g_upgrade_dev_addr[] 中的下标 */
    uint8_t retry_count;     /* 当前设备已重试次数 */
    uint8_t upgrade_active;  /* 升级激活标志，供外部 uds_upgrade_manager_is_active() 查询 */
    uint8_t uds_was_running; /* UDS 子状态机是否曾离开过 IDLE(用于判定是否已完成一轮升级流程) */
    uint8_t uds_prev_step;   /* 上一轮 UDS 子状态机步骤号(检测从非 IDLE 回到 IDLE 表示流程结束) */
};

static struct uds_manager_ctx g_mgr = {0};
static uint32_t g_last_progress_tick = 0;

extern uint8_t g_uds_fail_sid;
extern uint8_t g_uds_fail_nrc;
extern uint32_t g_uds_can_tx_fail_count;
extern unsigned long g_uds_last_recv_tick;
extern uint8_t uds_get_upgrade_percent(void);
extern uint8_t uds_get_upgrade_sid(void);
extern void uds_set_upgrade_percent(uint8_t percent);

/*
 * 切换 CAN 通道与收发 ID，将 UDS 定向到当前要升级的设备。
 * 依据设备类型选择物理 CAN 口(BSU 走 CAN1，IMIU4/IMEU3 走 CAN2)，
 * 再按 (dev_type<<8)|dev_addr 拼出该设备的收发 ID(扩展帧)，
 * 最后清空发送缓冲，避免上一台设备的残留帧干扰本次升级。
 */
static void manager_switch_can(uint8_t dev_type, uint8_t dev_addr)
{
    if (dev_type == 1) {
        g_upgrade_can_port = eCAN1;   // BSU 设备挂接在 CAN1
    } else {
        g_upgrade_can_port = eCAN2;   // IMIU4/IMEU3 设备挂接在 CAN2
    }

    uint16_t id_offset = ((uint16_t)dev_type << 8) | dev_addr;  // 用(设备类型,地址)组合出 ID 偏移，保证每台设备 ID 唯一
    g_uds_config.tx_id    = CAN_TX_BASE_ID | id_offset;          // 本端发送(诊断请求)帧 ID
    g_uds_config.rx_phy_id = CAN_RX_BASE_ID | id_offset;         // 目标设备物理应答帧 ID
    g_uds_config.rx_fuc_id = CAN_RX_BASE_ID | id_offset;         // 目标设备功能应答帧 ID

    g_uds_config.tx_id_type = 1;       // 标记为扩展帧(29bit)
    g_uds_config.rx_phy_id_type = 1;
    g_uds_config.rx_fuc_id_type = 1;

    reset_uds_send_buf();              // 清空 UDS 发送缓冲，防止上一设备残留帧

    ssdk_printf(SSDK_INFO, "[MGR] Switch CAN: type=%d addr=%d port=CAN%d tx=0x%08lx rx=0x%08lx\r\n",
                dev_type, dev_addr,
                (g_upgrade_can_port == eCAN1) ? 1 : 2,
                (unsigned long)g_uds_config.tx_id,
                (unsigned long)g_uds_config.rx_phy_id);
}

/*
 * 发送一帧"静默/唤醒"控制帧：向指定类型下的所有设备(addr=0xFF)广播一个控制字节。
 * 进入升级前发 enable=1 让目标设备静默(停止周期性报文)，退出时发 enable=0 恢复。
 */
static void manager_send_silence(uint8_t dev_type, uint8_t enable, e_CANNUM port)
{
    can_frame_t frame;

    memset(&frame, 0, sizeof(can_frame_t));
    frame.id = CAN_TX_BASE_ID | ((uint32_t)dev_type << 8) | 0xFFu;  // 0xFF 表示对该类型下所有设备广播
    frame.ide = CAN_EXTID;      // 扩展帧
    frame.rtr = CAN_DTR;        // 数据帧(非远程请求帧)
    frame.fd = 0;               // 经典 CAN 帧(非 CAN-FD)
    frame.len = 8;              // 8 字节数据
    frame.data[0] = enable;     // 数据域第 0 字节 = 静默使能标志
    BSPCANSendQin(port, &frame);
}

/*
 * 按设备类型批量下发静默/唤醒控制帧：
 * BSU 走 CAN1；IMIU4/IMEU3 走 CAN2 且两种类型都发(共享同一总线)。
 * 每种帧连续发送 SILENCE_SEND_COUNT 次以提高接收可靠性。
 */
static void manager_silence_devices(uint8_t enable)
{
    uint8_t i;

    if (g_upgrade_dev_type == 1) {
        for (i = 0; i < SILENCE_SEND_COUNT; i++) {
            manager_send_silence(1, enable, eCAN1);
        }
    } else {
        for (i = 0; i < SILENCE_SEND_COUNT; i++) {
            manager_send_silence(5, enable, eCAN2);
            manager_send_silence(6, enable, eCAN2);
        }
    }
}

/*
 * 通过 ICC 通道向主控上层上报单个设备的升级进度：
 * 进度百分比、当前服务(步骤)、通信状态(0正常/1超时)、重试次数、设备类型与地址。
 */
static void manager_send_progress(uint8_t percent, uint8_t current_srv,
                                   uint8_t dev_state, uint8_t retry,
                                   uint8_t dev_type, uint8_t dev_addr)
{
    uint8_t payload[6];
    payload[0] = percent;
    payload[1] = current_srv;
    payload[2] = dev_state;
    payload[3] = retry;
    payload[4] = dev_type;
    payload[5] = dev_addr;
    icc_upgrade_send_msg(ICC_CMD_UPGRADE_PROGRESS, payload, 6);
}

/*
 * 通过 ICC 通道上报单个设备的最终升级结果(成功/失败)，
 * 附带重试次数、失败服务码、失败错误码、设备类型与地址，并把进度清零。
 */
static void manager_send_result(uint8_t status, uint8_t retry,
                                 uint8_t fail_srv, uint8_t fail_code,
                                 uint8_t dev_type, uint8_t dev_addr)
{
    uint8_t payload[6];
    payload[0] = status;
    payload[1] = retry;
    payload[2] = fail_srv;
    payload[3] = fail_code;
    payload[4] = dev_type;
    payload[5] = dev_addr;
    icc_upgrade_send_msg(ICC_CMD_UPGRADE_RESULT, payload, 6);
    ssdk_printf(SSDK_INFO, "[MGR] RESULT status=%d retry=%d fail_srv=0x%02x fail_code=0x%02x dev=%d:%d\r\n",
                status, retry, fail_srv, fail_code, dev_type, dev_addr);
    uds_set_upgrade_percent(0);
}

/*
 * 切换到下一台待升级设备(多设备轮询)：复位 UDS 子状态机，设备下标+1，清空重试计数。
 * 若所有设备已处理完毕则进入 CLEANUP；否则回到 PREPARE_DEV 准备下一台设备。
 */
static void manager_next_device(void)
{
    uds_upgrade_reset();
    g_mgr.dev_idx++;
    g_mgr.retry_count = 0;
    g_mgr.uds_was_running = 0;
    g_mgr.uds_prev_step = UDS_UPGRADE_IDLE;

    if (g_mgr.dev_idx >= g_upgrade_dev_num) {
        ssdk_printf(SSDK_INFO, "[MGR] All %d devices done, cleanup\r\n",
                    g_upgrade_dev_num);
        g_mgr.state = MANAGER_STATE_CLEANUP;   // 全部设备处理完毕，进入清理
    } else {
        ssdk_printf(SSDK_INFO, "[MGR] Next device: idx=%d/%d\r\n",
                    g_mgr.dev_idx, g_upgrade_dev_num);
        g_mgr.state = MANAGER_STATE_PREPARE_DEV;   // 仍有设备待升级，准备下一台
    }
}

/*
 * 升级总协调器主循环(由 RTOS 任务周期性调用)。
 * 按顶层状态机推进：IDLE 等待启动 → PREPARE_DEV 准备单台设备 → UDS_RUNNING
 * 驱动 UDS 流程并处理成功/失败/重试/进度上报 → CLEANUP 收尾回到 IDLE。
 */
void uds_upgrade_manager_proc(void)
{
    uint32_t now_tick = (uint32_t)osKernelGetTickCount();
    uint8_t current_step;
    uint8_t dev_type, dev_addr;

    // IDLE：等待 icc_upgrade_handler 收完文件并把整体状态置为 STATE_UPGRADING，随后启动升级
    if (g_mgr.state == MANAGER_STATE_IDLE) {
        if (g_upgrade_state == STATE_UPGRADING && !g_mgr.upgrade_active) {
            ssdk_printf(SSDK_INFO, "[MGR] Upgrade start: type=%s(%d) devs=%d\r\n",
                        (g_upgrade_dev_type == 1) ? "BSU" :
                        (g_upgrade_dev_type == 5) ? "IMIU4" :
                        (g_upgrade_dev_type == 6) ? "IMEU3" : "?",
                        g_upgrade_dev_type, g_upgrade_dev_num);
            g_mgr.state = MANAGER_STATE_PREPARE_DEV;
            g_mgr.dev_idx = 0;             // 从第 0 台设备开始
            g_mgr.retry_count = 0;
            g_mgr.upgrade_active = 1;
            g_mgr.uds_was_running = 0;
            g_mgr.uds_prev_step = UDS_UPGRADE_IDLE;
            uds_set_upgrade_active(1);     // 通知 UDS 协议栈进入升级激活状态
        }
        return;
    }

    // CLEANUP：升级全部结束(成功或失败)，解除静默、释放文件缓冲、整体复位回到 IDLE
    if (g_mgr.state == MANAGER_STATE_CLEANUP) {
        ssdk_printf(SSDK_INFO, "[MGR] CLEANUP: free buffer, back to IDLE\r\n");
        manager_silence_devices(0);        // 恢复目标设备的周期性报文
        icc_upgrade_cleanup();             // 释放 ICC 侧的文件缓冲与状态
        uds_set_upgrade_active(0);
        memset(&g_mgr, 0, sizeof(g_mgr));  // 复位协调器上下文，回到 IDLE
        return;
    }

    dev_type = g_upgrade_dev_type;
    dev_addr = g_upgrade_dev_addr[g_mgr.dev_idx];   // 取当前设备的类型与地址

    switch (g_mgr.state) {
    case MANAGER_STATE_PREPARE_DEV:
        // 准备单台设备：清零错误标志与计时基准，切换 CAN，静默目标设备，启动 UDS 流程
        ssdk_printf(SSDK_INFO, "[MGR] PREPARE dev[%d] addr=%d retry=%d\r\n",
                    g_mgr.dev_idx, dev_addr, g_mgr.retry_count);
        g_uds_fail_sid = 0;
        g_uds_fail_nrc = 0;
        g_uds_can_tx_fail_count = 0;
        g_uds_last_recv_tick = now_tick;   // 以当前时刻作为"最近一次收到应答"的计时基准
        manager_switch_can(dev_type, dev_addr);
        manager_silence_devices(1);        // 让目标设备静默，避免总线周期报文干扰升级
        uds_upgrade_start();               // 启动 UDS 升级流程
        g_last_progress_tick = now_tick;   // 初始化进度上报计时
        g_mgr.uds_was_running = 0;
        g_mgr.uds_prev_step = UDS_UPGRADE_IDLE;
        g_mgr.state = MANAGER_STATE_UDS_RUNNING;
        break;

    case MANAGER_STATE_UDS_RUNNING:
        uds_proc_tester();   // 驱动 UDS 协议栈 tester 端状态机，推进当前设备升级流程

        current_step = uds_get_upgrade_step();   // 读取 UDS 子状态机当前步骤

        if (g_mgr.uds_prev_step != 0) {
            g_mgr.uds_was_running = 1;   // 一旦离开 IDLE 就标记"流程已启动"
        }

        // 成功判定：流程曾启动(uds_was_running)且当前已回到 IDLE，说明升级流程走完
        if (g_mgr.uds_was_running && current_step == UDS_UPGRADE_IDLE) {
            ssdk_printf(SSDK_INFO, "[MGR] dev[%d] addr=%d SUCCESS retry=%d\r\n",
                        g_mgr.dev_idx, dev_addr, g_mgr.retry_count);
            manager_send_result(ICC_UPGRADE_RESULT_OK,
                                g_mgr.retry_count, 0, 0,
                                dev_type, dev_addr);
            manager_next_device();   // 切换到下一台设备
            break;
        }

        // 失败处理：UDS 子状态机进入 ERROR，按错误源分类后决定重试或放弃
        if (current_step == UDS_UPGRADE_ERROR) {
            uint8_t fail_code;
            g_mgr.retry_count++;
            // 错误分类：
            //  1) SID 超时且 CAN 发送失败计数 >0 → 判定为 CAN 发送失败；
            //  2) 否则 nrc==0(无 NRC 应答)     → 判定为 CAN 接收超时(目标无响应)；
            //  3) 否则                          → 直接上报目标设备返回的 NRC。
            if (g_uds_fail_nrc == UDS_FAIL_SID_TIMEOUT && g_uds_can_tx_fail_count > 0) {
                fail_code = UDS_FAIL_CAN_TX;
            } else if (g_uds_fail_nrc == 0) {
                fail_code = UDS_FAIL_CAN_RX_TIMEOUT;
            } else {
                fail_code = g_uds_fail_nrc;
            }
            ssdk_printf(SSDK_WARNING, "[MGR] dev[%d] addr=%d ERROR retry=%d/%d SID=0x%02x code=0x%02x\r\n",
                        g_mgr.dev_idx, dev_addr, g_mgr.retry_count, MAX_RETRY, g_uds_fail_sid, fail_code);
            manager_send_result(ICC_UPGRADE_RESULT_FAIL,
                                g_mgr.retry_count - 1, g_uds_fail_sid, fail_code,
                                dev_type, dev_addr);
            if (g_mgr.retry_count <= MAX_RETRY) {
                // 未超重试上限：复位 UDS 子状态机，回到 PREPARE_DEV 重新尝试该设备
                uds_upgrade_reset();
                g_mgr.uds_was_running = 0;
                g_mgr.uds_prev_step = UDS_UPGRADE_IDLE;
                g_mgr.state = MANAGER_STATE_PREPARE_DEV;
            } else {
                // 超过重试上限：判定该设备升级失败，切换到下一台设备
                ssdk_printf(SSDK_ERR, "[MGR] dev[%d] addr=%d FAILED after %d retries\r\n",
                            g_mgr.dev_idx, dev_addr, MAX_RETRY);
                manager_next_device();
            }
            break;
        }

        g_mgr.uds_prev_step = current_step;   // 记录当前步骤，用于下一轮检测状态变化

        // 周期进度上报：距上次上报超过 PROGRESS_INTERVAL_MS 才发送，避免刷爆 ICC 通道
        if ((now_tick - g_last_progress_tick) >= PROGRESS_INTERVAL_MS) {
            uint8_t comm_state = 0x00;   // 0=通信正常，1=通信超时
            g_last_progress_tick = now_tick;
            if ((int32_t)(now_tick - g_uds_last_recv_tick) > COMM_TIMEOUT_MS) {
                comm_state = 0x01;       // 超过 COMM_TIMEOUT_MS 未收到目标应答，标记通信超时
            }
            manager_send_progress(uds_get_upgrade_percent(),
                                  uds_get_upgrade_sid(),
                                  comm_state,
                                  g_mgr.retry_count, dev_type, dev_addr);
        }
        break;

    default:
        break;
    }
}

/* 供外部查询：升级是否处于激活状态 */
int uds_upgrade_manager_is_active(void)
{
    return g_mgr.upgrade_active;
}

/* 供外部查询：当前升级整体状态(IDLE/收文件/升级中等) */
upgrade_state_t uds_upgrade_manager_get_state(void)
{
    return g_upgrade_state;
}
