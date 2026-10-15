/**
 * @file sysmgr.h
 * @brief sysmgr 公共 API — iccmsg gate 接口
 *
 * 关联变更: openspec/changes/add-system-mgmt-service/ (v3 final)
 */
#ifndef SYSMGR_H
#define SYSMGR_H

#include <types.h>
#include <compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 设置 A 核为就绪状态（由 echo_cb 或 PING 处理器调用）。弱定义默认空操作。 */
__WEAK void sysmgr_signal_ap_ready(void);

/** @brief 查询 A 核是否就绪。弱定义默认返回 true。 */
__WEAK bool sysmgr_is_ap_ready(void);

/**
 * @brief 阻塞等待 A 核就绪
 * @param timeout_ms 超时时间（毫秒）
 * @return 0 = 就绪, -ETIMEDOUT = 超时
 */
__WEAK int  sysmgr_wait_ap_ready(uint32_t timeout_ms);

/**
 * @brief 阻塞等待 sysmgr/ota 端点创建完成
 * @param timeout_ms 超时时间（毫秒）
 * @return 0 = 已就绪, -ETIMEDOUT = 超时
 *
 * 业务侧（如 BMS）在分配自己的 iccmsg 端点前应等待本事件，
 * 以确保 sysmgr@500 / ota@502 已先占号，其余通道从 504 起稳定分配。
 */
__WEAK int  sysmgr_wait_ept_ready(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYSMGR_H */
