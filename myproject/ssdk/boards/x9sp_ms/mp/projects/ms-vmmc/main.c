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

#if defined CONFIG_DISK
#include <lib/disk/disk.h>
#endif /* end CONFIG_DISK */

#if defined(CONFIG_KUNLUN_MMC) && defined(CONFIG_DISK_MMC)
#include "mmc_cfg.h"
#endif

#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
#include "cam_service.h"
#endif /* end CONFIG_KUNLUN_CAMERA_SERVICE */

#ifdef CONFIG_RES_LOADER
#include <res_loader.h>
#endif /* end CONFIG_RES_LOADER */

#if defined(CONFIG_AUDIO_MANAGER) || defined(CONFIG_AUDIO_HUB)
#include <audio_cfg.h>
#endif

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

#ifdef CONFIG_ERRCODE
#include <sd_errcode.h>
#endif /* end CONFIG_ERRCODE */

#ifdef CONFIG_APP_LAUNCHER
#include <launcher.h>
#endif /* end CONFIG_APP_LAUNCHER */

#ifdef CONFIG_CAN_PROXY
#include <can_proxy.h>
#endif

#if defined(CONFIG_DISK) && defined(CONFIG_DISK_RAM)
#include <ram_cfg.h>
#endif /* CONFIG_DISK_RAM */

#ifdef CONFIG_SDRV_VIRT_CONSOLE
#include <sdshell.h>
#endif

#ifdef CONFIG_VCAN_CLIENT
#include <can_cfg.h>
#endif

#ifdef CONFIG_SDRV_THERMAL_SERVICE
#include <thermal_service.h>
#endif

#ifdef CONFIG_VPU_SRV
#include "vpu_virtio_svc.h"
#endif

#ifdef CONFIG_VIRTIO
#include "virtio_init.h"
#endif

#ifdef CONFIG_VPU_CLIENT
#include "vpu_proto_client.h"
#endif

#ifdef CONFIG_SDRV_SHM
#include <sdrv_shm_init.h>
#endif

#ifdef CONFIG_PFM_ON
#include <sd_ddr_pfmon.h>
#endif

#ifdef CONFIG_DISPLAY_SERVICE
#include <disp_service_defs.h>
#endif

#ifdef CONFIG_AVMCAST_SERVICE
#include <avmcast.h>
#endif

#ifdef CONFIG_VI2C
#include <vi2c.h>
#endif

#ifdef CONFIG_SLIMFS
#include <sdrv_slimfs.h>
#endif /* end CONFIG_SLIMFS */

#ifdef CONFIG_TIME_SYNC
#include <time_sync.h>
#endif

#if CONFIG_SDSHELL_BACKEND
#include <sdshell_be.h>
#endif

#if defined(CONFIG_MARK_SUCCESS) && defined(CONFIG_PROPERTY)
#include <ota_mark_success.h>
#endif

int main(void)
{
    printf("Hello MP SSDK!\n");

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif

#ifdef CONFIG_SDRV_THERMAL_SERVICE
    thermal_service_init(SYS_SHUTDOWN);
#endif

#ifdef CONFIG_RPMSG_SERVICE
    rpmsg_service_init(RPMSG_SERVICES_NUM, g_rpmsg_services);
#endif /* end CONFIG_RPMSG_SERVICE */

#ifdef CONFIG_VI2C
    virtual_i2c_init();
#endif

#if defined CONFIG_DISK
    disk_init();
#endif /* end CONFIG_DISK */

#if defined(CONFIG_DISK) && defined(CONFIG_DISK_RAM)
    board_ram_init();
#endif

#ifdef CONFIG_PROPERTY
    start_property_service(NULL, 0);
#endif

#ifdef CONFIG_VIRTIO
    virtio_init();
#endif

#ifdef CONFIG_SDRV_SHM
    shm_init();
#endif

#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
    cam_service_init();
#endif

#ifdef CONFIG_VCAN_CLIENT
    board_vcan_init();
#endif

#ifdef CONFIG_CLUSTER_BRIDGE
    clusterapp_bridge_init();
#endif

#ifdef CONFIG_INPUT_SERVICE
    touch_service_start(g_ts_report_cfg_tbl, TS_REPORT_CFG_TBL_SIZE);
#endif /* end CONFIG_INPUT_SERVICE */

#ifdef CONFIG_UNILINK
    tcpip_init(NULL,NULL);
    ulink_service_init();
#endif

#ifdef CONFIG_SDRV_VIRT_CONSOLE
    sdshell_console_init();
#endif

#if CONFIG_SDSHELL_BACKEND
    board_sdshell_be_init();
#endif

#ifdef CONFIG_BACKLIGHT_SERVICE
    backlight_service_init();
#endif

#ifdef CONFIG_DISPLAY_SERVICE
    display_service_init();
#endif

#if defined(CONFIG_DISK) && defined(CONFIG_DISK_MMC)
    board_mmc_init();
#endif

#ifdef CONFIG_RES_LOADER
    res_fs_init();
#endif /* end CONFIG_RES_LOADER */

#if defined(CONFIG_AUDIO_MANAGER) || defined(CONFIG_AUDIO_HUB)
    board_audio_init();
#endif

#ifdef CONFIG_VPU_SRV
    vpu_virtio_svc_entry();
#endif

#ifdef CONFIG_VPU_CLIENT
    vpu_virtio_local_client_init();
#endif

#ifdef CONFIG_APP_LAUNCHER
    launcher_app();
#endif

#ifdef CONFIG_SLIMFS
    slimfs_init_main();
#endif /* end CONFIG_SLIMFS */

#ifdef CONFIG_ERRCODE
    system_errcode_init();
#endif

#ifdef CONFIG_AVMCAST_SERVICE
    avm_cast_service_init();
#endif

#ifdef CONFIG_PFM_ON
    start_pfm_service();
#endif

#ifdef CONFIG_TIME_SYNC
    time_sync_service_init();
#endif

#ifdef CONFIG_PVT_APPLICATION
    pvt_app_init();
#endif /* end CONFIG_PVT_APPLICATION */

#if defined(CONFIG_MARK_SUCCESS) && defined(CONFIG_PROPERTY)
    mark_system_success();
#endif
}
