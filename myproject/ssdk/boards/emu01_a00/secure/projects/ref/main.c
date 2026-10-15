/**
 * @file main.c
 * @brief emu01_a00 secure main source file (unified template).
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 * 单工程模板入口 (ssdk-standalone-boot-mode Stage-3): ref/xip 两份 main.c 归一,
 * 差异由 CONFIG_XIP_MODE 等编译宏分支:
 *   - XIP 变体 (CONFIG_XIP_MODE=y): .text 在 OSPI2 NOR 原地执行, 打印 XIP 诊断/Hello
 *   - emmc_only 变体: 固件由 FDA 载入 OCRAM, 不初始化 NOR
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
#ifdef CONFIG_APPLICATION_SYS_MONITOR
#include "sys_monitor.h"
#endif

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

extern void ExtHeapInit( uint32_t address ,uint32_t size);
int main(void)
{
#ifdef CONFIG_XIP_MODE
    /* XIP 诊断: 如果看到此消息，说明 CPU 已从 NOR 取指进入 main() */
    printf("\n*** XIP: ssystem main entered ***\n");
#endif

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
    sys_monitor_init();
#endif

#ifdef CONFIG_UPDATE_MONITOR
    update_monitor_start(g_update_mon_cfg);
#endif /* end CONFIG_UPDATE_MONITOR */

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif

}
