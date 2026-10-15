/*
 * barriers.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARMV7R barriers interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_ARMV7R_BARRIERS_H
#define INCLUDE_ARCH_ARMV7R_BARRIERS_H

#include <compiler.h>

#define DSB __ASM volatile("dsb" ::: "memory")
#define DMB __ASM volatile("dmb" ::: "memory")
#define ISB __ASM volatile("isb" ::: "memory")

#endif
