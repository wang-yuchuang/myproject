
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#ifdef CONFIG_SBC

#include <board.h>
#include <cmsis_os2.h>
#include <debug.h>

#include "../examples/taishan/drivers/sbc/sbc_sys.h"

#define WD_TRIGGER_PERIOD 700
void board_sbc_init(void)
{
    SBC_Init();
    /* create Trigger watchdog thread */
    if (SBC_Create_WD_Trigger_Task(WD_TRIGGER_PERIOD) == NULL) {
        printf("Create SBC Wtd Trigger Thread Failed\r\n");
    }
}
#endif
