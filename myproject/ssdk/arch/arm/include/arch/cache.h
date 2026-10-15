/*
 * cache.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM cache interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_CACHE_H
#define INCLUDE_ARCH_CACHE_H

#define ICACHE                  2               //The old value is 1
#define DCACHE                  1               //The old value is 2
#define UCACHE                  (ICACHE|DCACHE)

#define __CACHE_ALIGN           __ALIGNED(CONFIG_ARCH_CACHE_LINE)

#ifndef ASSEMBLY

#include <types.h>
#include <compiler.h>

__BEGIN_CDECLS

/*
 * enable caches.
 *
 * @flags   cache type.
 */
#ifdef CONFIG_ARCH_WITH_CACHE
void arch_enable_cache(uint8_t flags);
#else
#define arch_enable_cache(flags)
#endif

/*
 * disable caches.
 *
 * @flags   cache type.
 */
#ifdef CONFIG_ARCH_WITH_CACHE
void arch_disable_cache(uint8_t flags);
#else
#define arch_disable_cache(flags)
#endif

/*
 * clean dcache.
 *
 * @start   start address.
 * @len     clean data length.
 */
#ifdef CONFIG_ARCH_WITH_CACHE
void arch_clean_cache_range(addr_t start, size_t len);
#else
#define arch_clean_cache_range(start, len)
#endif

/*
 * clean and invalidate dcache.
 *
 * @start   start address.
 * @len     clean data length.
 */
#ifdef CONFIG_ARCH_WITH_CACHE
void arch_clean_invalidate_cache_range(addr_t start, size_t len);
#else
#define arch_clean_invalidate_cache_range(start, len)
#endif

/*
 * clean and invalidate dcache all
 *
 */
#ifdef CONFIG_ARCH_WITH_CACHE
void arch_clean_invalidate_dcache_all(void);
#else
#define arch_clean_invalidate_dcache_all()
#endif

/*
 * invalidate dcache.
 *
 * @start   start address.
 * @len     clean data length.
 */
#ifdef CONFIG_ARCH_WITH_CACHE
void arch_invalidate_cache_range(addr_t start, size_t len);
#else
#define arch_invalidate_cache_range(start, len)
#endif

/*
 * sync dcache.
 *
 * @start   start address.
 * @len     clean data length.
 */
#ifdef CONFIG_ARCH_WITH_CACHE
void arch_sync_cache_range(addr_t start, size_t len);
#else
#define arch_sync_cache_range(start, len)
#endif

__END_CDECLS

#endif

#endif
