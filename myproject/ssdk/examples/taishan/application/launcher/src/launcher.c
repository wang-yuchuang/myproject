/**
 * @file launcher.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv launcher application.
 *
 * Revision History:
 * -----------------
 */

#include "launcher.h"

#ifdef CONFIG_APP_BOOTANIMATION
#include "animation.h"
#endif

#ifdef CONFIG_APP_CLUSTER
#include "lv_demo_cluster.h"
#endif

#ifdef CONFIG_APP_CONTROLPANEL
#include "lv_controlpanel.h"
#endif

#define IRAM_MEM_SIZE (0x1D0000)
char IRAM_AP_DATA[IRAM_MEM_SIZE] __SECTION(".apdata");

void launcher_entry() {
    printf("launcher_entry\n");

#ifdef CONFIG_APP_BOOTANIMATION
    printf("Hello animation!\n");
    animation_entry();
    osDelay(3000);// tmp use delay,Subsequent use of x9 property
#endif

#ifdef CONFIG_APP_CONTROLPANEL
    printf("Hello Control Panel!\n");
    lv_controlpanel();
#endif

#ifdef CONFIG_APP_CLUSTER
    printf("Hello Cluster!\n");
    lv_cluster();
#endif

}