/**
 * @file memmap.h
 * @brief memmap header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */


#ifndef MEMMAP_H
#define MEMMAP_H

#include <types.h>

#ifdef CONFIG_MEMMAP
#include <memmap.h>
#endif

#ifndef struct_addr_map
#define struct_addr_map
struct addr_map {
    paddr_t src;
    size_t size;
    paddr_t dst;
};
#endif

/**
 * @brief Transform address from r core to a core.
 *
 * @param[in] pa r core address
 * @return a core address
 */
#ifdef CONFIG_MEMMAP
paddr_t p2ap(paddr_t pa);
uint64_t lp2ap(uint64_t pa);
#else
#define p2ap(pa) (pa)
#endif

/**
 * @brief Transform address from a core to r core.
 *
 * @param[in] pa a core address
 * @return r core address
 */
#ifdef CONFIG_MEMMAP
paddr_t ap2p(paddr_t va);
uint64_t lap2p(uint64_t va);
#else
#define ap2p(va) (va)
#endif

#endif /* MEMMAP_H */
