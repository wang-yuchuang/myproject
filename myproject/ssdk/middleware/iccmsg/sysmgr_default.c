/**
 * @file sysmgr_default.c
 * @brief sysmgr 弱定义默认实现（不阻塞，向后兼容）
 */
#include <types.h>
#include <compiler.h>
#include "sysmgr.h"

__WEAK bool sysmgr_is_ap_ready(void)
{
    return 1;  /* true */
}

__WEAK int sysmgr_wait_ap_ready(uint32_t timeout_ms)
{
    (void)timeout_ms;
    return 0;
}

__WEAK int sysmgr_wait_ept_ready(uint32_t timeout_ms)
{
    (void)timeout_ms;
    return 0;
}

__WEAK void sysmgr_signal_ap_ready(void)
{
    /* 默认空操作 — 未启用 sys_monitor 时不做任何事 */
}
