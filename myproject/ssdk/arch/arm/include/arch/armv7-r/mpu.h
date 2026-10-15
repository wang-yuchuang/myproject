/*
 * mpu.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cortex V7R MPU driver.
 *
 * Revision History:
 * -----------------
 */
#ifndef INCLUDE_ARCH_ARMV7R_MPU_H
#define INCLUDE_ARCH_ARMV7R_MPU_H

/*
 * Memory types supported by the MPU driver.
 */
typedef enum mpu_region_type {
    /* Strong ordered
     *  - non-bufferable, non-cachable, shareable, RW, XN
     */
    MPU_REGION_STRONGORDERED,

    /* Device memory
     *  - bufferable, non-cachable, shareable, RW, XN
     */
    MPU_REGION_DEVICE,

    /* Normal memory
     *  - bufferable, outer and inner write-back & write allocate,
     *    shareble, RW, non XN
     */
    MPU_REGION_NORMAL,

    /* Normal non-cacheable memory
     *  - bufferable, outer and inner non-cacheable, shareble, RW,
     *    non XN
     */
    MPU_REGION_NORMAL_NONCACHEABLE,

    /* Normal ready-only memory
     *  - bufferable, outer and inner wb, no wa, no shareble, RO,
     *    non XN
     */
    MPU_REGION_NORMAL_RO,

    /* Not accessable.
     */
    MPU_REGION_NO_ACCESS,

    MPU_REGION_MAX,
} mpu_region_type_e;

#ifndef ASSEMBLY

#include <compiler.h>
#include <stdbool.h>
#include <types.h>

typedef struct mpu_config {
    addr_t              addr;
    uint64_t            size;
    mpu_region_type_e   type;
} mpu_config_t;

__BEGIN_CDECLS

#ifdef CONFIG_ARCH_WITH_MPU
void mpu_clear_region(void);
#else
#define mpu_clear_region()
#endif

#ifdef CONFIG_ARCH_WITH_MPU
void mpu_add_region(int region, uint32_t base, uint64_t size,
                    mpu_region_type_e type);
#else
#define mpu_add_region(region, base, size, type)
#endif

uint32_t mpu_region_index(void);

#ifdef CONFIG_ARCH_WITH_MPU
void mpu_enable(bool enable);
#else
#define mpu_enable(enable)
#endif

#ifdef CONFIG_ARCH_WITH_MPU
bool mpu_is_belong_uncache_region(uint32_t base, uint32_t size);
#else
bool mpu_is_belong_uncache_region(base, size)
#endif

__END_CDECLS

#endif /* ASSEMBLY */

#endif
