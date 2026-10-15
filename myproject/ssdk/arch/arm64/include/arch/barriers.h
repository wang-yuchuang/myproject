/*
 * barriers.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM barriers interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_BARRIERS_H
#define INCLUDE_ARCH_BARRIERS_H
#include <compiler.h>

#define DSB __ASM volatile("dsb sy" ::: "memory")
#define ISB __ASM volatile("isb" ::: "memory")

#endif
