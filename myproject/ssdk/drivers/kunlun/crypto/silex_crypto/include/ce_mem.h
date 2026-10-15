/**
 * @file disp_config.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __CE_MEM_H
#define __CE_MEM_H

#ifndef _ioaddr
#define _ioaddr(x)  (x)
#endif

#ifndef lk_time_t
typedef uint32_t lk_time_t;
#endif

#define addrmap_def \
    {/*DDR */   \
        0x30000000,\
        0xc0000000,\
        0x40000000,\
    },\
    {/*APB_MUX */\
        0xf0000000,\
        0x10000000,\
        0x30000000,\
    },

#ifndef struct_addr_map
#define struct_addr_map
struct addr_map {
    paddr_t src;
    size_t size;
    paddr_t dst;
};
#endif

static struct addr_map addrmap_tab[] = {
    addrmap_def
};

static inline paddr_t p2ap(paddr_t pa)
{
    int i = 0;
    struct addr_map *addr;
    for (i=0; i< (int)(sizeof(addrmap_tab)/sizeof(addrmap_tab[0])); i++) {
        addr = &addrmap_tab[i];
        if (pa >= addr->src && (pa - addr->src) < addr->size) {
            return ((pa - addr->src) + addr->dst);
        }
    }
    return pa;
}

static inline paddr_t ap2p(paddr_t va)
{
    int i = 0;
    struct addr_map *addr;
    for (i = 0; i < (int)(sizeof(addrmap_tab) / sizeof(addrmap_tab[0])); i++) {
        addr = &addrmap_tab[i];
        if (va >= addr->dst && (va - addr->dst) < addr->size) {
            return (va - addr->dst + addr->src);
        }
    }
    return va;
}
#endif
