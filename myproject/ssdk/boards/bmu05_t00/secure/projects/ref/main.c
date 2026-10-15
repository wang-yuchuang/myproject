/**
 * @file main.c
 * @brief bmu05_a00 secure main source file (unified template).
 *
 * 单工程模板入口 (ssdk-project-mode-unification D5): xip/xip_b/ref 三份 main.c 归一,
 * 差异由 CONFIG_XIP_MODE 等编译宏分支 (基线 diff 见 tasks.md 验证区 §0.2):
 *   - XIP 变体 (CONFIG_XIP_MODE=y): .text 在 OSPI2 NOR 原地执行, disk_init+board_norflash_init
 *   - emmc_only 变体: 固件由 FDA 载入 OCRAM, 不初始化 NOR/disk
 */

#include <stdio.h>
#include <stdlib.h>
#include <debug.h>

#ifdef CONFIG_CLI
#include <CLI.h>
#endif
#ifdef CONFIG_RPMSG_SERVICE
#include <rpmsg_service.h>
#include <rpmsg_service_cfg.h>
#endif /* end CONFIG_RPMSG_SERVICE */

#ifdef CONFIG_APP_REBOOT_SS
#include <app_reboot_ss.h>
#endif /* end CONFIG_APP_REBOOT_SS */

#ifdef CONFIG_UPDATE_MONITOR
#include <update_monitor.h>
#include <update_monitor_cfg.h>
#endif /* end CONFIG_UPDATE_MONITOR */

#ifdef CONFIG_REBOOT
#include "reboot_cfg.h"
#endif
#ifdef CONFIG_SAVE_DDR_INFO_IN_EMMC
#include "str.h"
#endif

#ifdef CONFIG_OSPI
#include <lib/disk/disk.h>
#include "ospi_cfg.h"
#endif
#include "image_cfg.h"

#ifdef CONFIG_APPLICATION_SYS_MONITOR
#include "sys_monitor.h"
#endif

extern void ExtHeapInit( uint32_t address ,uint32_t size);
int main(void)
{
#ifdef CONFIG_APP_REBOOT_SS
    app_reboot_ss_init(WDT5____ovflow_int);
#endif /* end CONFIG_APP_REBOOT_SS */

#ifdef CONFIG_REBOOT
    board_reboot_init();
#endif

#ifdef CONFIG_XIP_MODE
    printf("Hello <%s> secure SSDK (XIP on OSPI2 NOR)!\n", CONFIG_BOARD_NAME_STR);
#else
    printf("Hello <%s> secure SSDK!\n", CONFIG_BOARD_NAME_STR);
#endif

#ifdef CONFIG_RPMSG_SERVICE
    rpmsg_service_init(RPMSG_SERVICES_NUM, g_rpmsg_services);
#endif /* end CONFIG_RPMSG_SERVICE */

#ifdef CONFIG_SAVE_DDR_INFO_IN_EMMC
    str_slave_commu_init();
#endif

    ExtHeapInit(SAF_MEMBASE,SAF_MEMSIZE);

    /* OSPI/NOR 初始化: 必须先 ExtHeapInit 再 disk_init (osMutexNew 用 heap_ext 堆),
     * 否则 g_disk_mutex=NULL, disk_open 死等。
     * 仅 XIP 变体执行 (emmc_only: R5 由 FDA 载入 OCRAM, NOR 非启动介质;
     * CONFIG_OSPI 因 flash_test/sys_monitor 编译依赖而开启, 但不可初始化 disk——
     * 无 disk_init 前置的 register_disk 写 NULL list 崩溃, D1 修复)。 */
#if defined(CONFIG_OSPI) && defined(CONFIG_XIP_MODE)
    disk_init();
    board_norflash_init();
#endif

#ifdef CONFIG_APPLICATION_SYS_MONITOR
    /* 统一在 ExtHeapInit 之后启动 (sys_monitor_init 内部 osThreadNew/osEventFlagsNew
     * 依赖 heap; D5 定序——xip 原在 heap 前, 已核实 init 为后台线程启动, 延后无
     * 顺序依赖; sys_monitor_init_deferred 为 v3 兼容别名归一为 sys_monitor_init) */
    sys_monitor_init();
#endif

#ifdef CONFIG_UPDATE_MONITOR
    update_monitor_start(g_update_mon_cfg);
#endif /* end CONFIG_UPDATE_MONITOR */

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif

}
