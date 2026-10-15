/**
 * @file main.c
 * @brief main source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
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

#ifdef CONFIG_UPDATE_MONITOR
#include <update_monitor.h>
#include <update_monitor_cfg.h>
#endif /* end CONFIG_UPDATE_MONITOR */

#ifdef CONFIG_APP_REBOOT_SS
#include <app_reboot_ss.h>
#endif /* end CONFIG_APP_REBOOT_SS */

#ifdef CONFIG_BACKLIGHT_SERVICE
#include <backlight.h>
// 0: Cluster
// 1: Infotainment
// 2: Entertainment
// 3: ControlPanel
// 4: HUD
#define DISPLAY_MAX_NUM 5
#endif

#ifdef CONFIG_DISPLAY_SERVICE
#include <disp_service_defs.h>
#endif

#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
#include "cam_service.h"
#endif /* end CONFIG_KUNLUN_CAMERA_SERVICE */

#ifdef CONFIG_RES_LOADER
#include <res_loader.h>
#endif /* end CONFIG_RES_LOADER */

#ifdef CONFIG_INPUT_SERVICE
#include <input_cfg.h>
#include <input_service.h>
#endif  /* end CONFIG_INPUT_SERVICE */

#ifdef CONFIG_UNILINK
#include <ulink_service.h>
#include <lwip/tcpip.h>
#endif /* end CONFIG_UNILINK */

#ifdef CONFIG_PVT_APPLICATION
#include <pvt_app.h>
#endif /* end CONFIG_PVT_APPLICATION */

#ifdef CONFIG_CLUSTER_BRIDGE
#include <cluster_config.h>
#endif /* end CONFIG_CLUSTER_BRIDGE */

#ifdef CONFIG_PROPERTY
#include <sd_property.h>
#endif /* end CONFIG_PROPERTY */

#ifdef CONFIG_APP_LAUNCHER
#include <launcher.h>
#endif /* end CONFIG_APP_LAUNCHER */

#ifdef CONFIG_CAN_PROXY
#include <can_proxy.h>
#endif

#ifdef CONFIG_SDRV_VIRT_CONSOLE
#include <sdshell.h>
#endif

int main(void)
{
    printf("Hello SSDK!\n");

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif
}