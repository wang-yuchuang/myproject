
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_REBOOT

#include <types.h>
#include <param.h>
#include <string.h>
#include <arch/cache.h>
#include <reboot.h>
#include <regs_base.h>
#include <scr_hw.h>
#include <rstgen_cfg.h>
#include <scr_cfg.h>
#include <mbox_cfg.h>
#include <bits.h>
#include <reg.h>
#include <board.h>
#include <mem_image.h>

#include <CLI.h>
#include <clock_cfg.h>
#include <driver/clk.h>
#include "image_cfg.h"
#include "reboot_cfg.h"



static int32_t boot_secure()
{
    printf("d9_ref board secure boot is not supported!\n");
    return 0;
}

int32_t boot_ap1()
{
    printf("d9_ref board ap1 boot is not supported!\n");
    return 0;
}

int32_t boot_ap2()
{
    printf("d9_ref board ap2 boot is not supported!\n");
    return 0;
}

void board_reboot_init(void)
{

}

#endif
