/**
 * @file memmap.c
 * @brief memmap source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <types.h>
#include <driver/memmap.h>

static struct addr_map memmap_table[] = {
    MEMMAP_TABLE
};

/**
 * @brief Transform address from r core to a core.
 *
 * @param[in] pa r core address
 * @return a core address
 */
paddr_t p2ap(paddr_t pa)
{
    int i = 0;
    struct addr_map *addr;
    for (i = 0; i < (int)(sizeof(memmap_table) / sizeof(memmap_table[0])); i++) {
        addr = &memmap_table[i];
        if (pa >= addr->src && (pa - addr->src) < addr->size) {
            return (pa - addr->src + addr->dst);
        }
    }
    return pa;
}

uint64_t lp2ap(uint64_t pa)
{
    int i = 0;
    struct addr_map *addr;
    for (i = 0; i < (int)(sizeof(memmap_table) / sizeof(memmap_table[0])); i++) {
        addr = &memmap_table[i];
        if (pa >= addr->src && (pa - addr->src) < addr->size) {
            return (pa - addr->src + addr->dst);
        }
    }
    return pa;
}
/**
 * @brief Transform address from a core to r core.
 *
 * @param[in] pa a core address
 * @return r core address
 */
paddr_t ap2p(paddr_t va)
{
    int i = 0;
    struct addr_map *addr;
    for (i = 0; i < (int)(sizeof(memmap_table) / sizeof(memmap_table[0])); i++) {
        addr = &memmap_table[i];
        if (va >= addr->dst && (va - addr->dst) < addr->size) {
            return (va - addr->dst + addr->src);
        }
    }
    return va;
}

uint64_t lap2p(uint64_t va)
{
    int i = 0;
    struct addr_map *addr;
    for (i = 0; i < (int)(sizeof(memmap_table) / sizeof(memmap_table[0])); i++) {
        addr = &memmap_table[i];
        if (va >= addr->dst && (va - addr->dst) < addr->size) {
            return (va - addr->dst + addr->src);
        }
    }
    return va;
}

