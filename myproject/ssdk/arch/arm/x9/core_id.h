/*
 * core_id.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: X9 core id interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef _CORE_ID_H_
#define _CORE_ID_H_

#ifndef ASSEMBLY
#include "arch/register.h"

#ifdef CONFIG_MULTI_CORE
#define CORE_NUM    2
#else
#define CORE_NUM    1
#endif

static inline int get_core_id(void)
{
#ifdef CONFIG_MULTI_CORE
    uint32_t mpidr = arm_read_mpidr();
    uint8_t cluster_id = (mpidr >> 8) & 0xFF;

    return cluster_id;
#else
    return 0;
#endif
}

#endif

#endif
