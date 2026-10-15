/**
 * @file sys_monitor_ota.h
 * @brief System Monitor — OTA 升级模块（R 核侧接收/写 NOR/切槽）
 *
 * 协议: sysmgr OTA_BEGIN/DATA/COMMIT/STATUS (rauc-xip-ota S1b)
 * 数据流: A 核 ota-client 分块推送 → R 核 flash_test RAM 闭包 STIG 写 NOR
 *         非活动槽 → COMMIT 回读 CRC 校验 → NOR GPT 原子切槽。
 *
 * 复用已验证原语 (不重复验证):
 *   - STIG 擦/写: flash_test_erase()/flash_test_write()  (examples/.../flash_test)
 *   - GPT 读/XIP direct / 切槽位语义: gpt_slot_switch.c (ab-slot 13.5c/l)
 *
 * 关联变更: openspec/changes/rauc-xip-ota/
 */
#ifndef SYS_MONITOR_OTA_H
#define SYS_MONITOR_OTA_H

#include <stdint.h>

#include "sysmgr_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 分发 OTA 命令（sys_monitor_msg.c rx 分发调用；在 rx_task 上下文执行）。
 *  @return 0 = 已处理（调用方不再处理）, -1 = 非 OTA 命令 */
int sys_monitor_ota_handle(const sysmgr_hdr_t *hdr, const uint8_t *payload);

/** 初始化 OTA 模块（sys_monitor init 流程调用：复位状态 + 创建 worker task） */
void sys_monitor_ota_init(void);

/** B1b (2026-09-03): 绑定独立 OTA endpoint（"ota"@502）——OTA 响应经此 ept 发出
 * 而非 sysmgr@500（避免 ota-client 与 app_sysmgr 心跳同通道排他互斥）。
 * @param ept  iccmsg 实例指针（void* 避免头文件依赖 iccmsg.h） */
void sys_monitor_ota_set_ept(void *ept);

/* S3.3 (2026-09-05): mark-good——重置运行槽 GPT retry/successful (防 ALWAYS_ROLL_BACK_CHECK
 * 4 次 boot 自动回滚)。由 SYSMGR_MARK_GOOD 命令触发 (sys_monitor_msg.c rx_cb)。返回 0=成功 */
int sys_monitor_ota_mark_good(void);

#ifdef __cplusplus
}
#endif

#endif /* SYS_MONITOR_OTA_H */
