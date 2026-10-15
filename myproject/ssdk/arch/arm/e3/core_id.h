/*
 * core_id.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: E3 core id interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef _CORE_ID_H_
#define _CORE_ID_H_

#ifndef ASSEMBLY
#include "arch/register.h"

#ifdef CONFIG_MULTI_CORE
#define CORE_NUM    5
#else
#define CORE_NUM    1
#endif

static inline int get_core_id(void)
{
#ifdef CONFIG_MULTI_CORE
    uint32_t mpidr = arm_read_mpidr();
    uint8_t cluster_id = (mpidr >> 8) & 0xFF;
    uint8_t core_id = mpidr & 0xFF;

    /* SF only lockstep 0x0000
     * SP0 0x0100, SP1 0x0101
     * SX0 0x0200, SX1 0x0201
     */
    return cluster_id ?
           (((cluster_id << 1) - 1) + core_id) :
           core_id;
#else
    return 0;
#endif
}

#endif

#endif
