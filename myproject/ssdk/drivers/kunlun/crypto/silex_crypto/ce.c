/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/
#include <stdlib.h>
#include <sram_conf.h>
#include <ce.h>
#include <ce_reg.h>
#include <malloc.h>
#include <sx_trng.h>
#include <spinlock.h>
#include <ce_mem.h>
#include <driver/irq.h>
#include <arch/cache.h>

#define CE_MEM_NODE_ITEM_NUM_MAX 6
#define CE_MEM_NODE_RSA_MAX 512
#define CE_MEM_NODE_RSA_MAX_EXT 516
#define CE_MEM_NODE_DIG_MAX 64
#define CE_MEM_NODE_HASH_PAD_MAX 256

uint8_t __attribute__((aligned(CONFIG_ARCH_CACHE_LINE))) msg_padding[ROUNDUP(CE_MEM_NODE_RSA_MAX, CONFIG_ARCH_CACHE_LINE)];
uint8_t __attribute__((aligned(CONFIG_ARCH_CACHE_LINE))) rev_cpy_buf[ROUNDUP(CE_MEM_NODE_RSA_MAX, CONFIG_ARCH_CACHE_LINE)];
uint8_t __attribute__((aligned(CONFIG_ARCH_CACHE_LINE))) msg_padding_ext[ROUNDUP(CE_MEM_NODE_RSA_MAX_EXT, CONFIG_ARCH_CACHE_LINE)];
uint8_t __attribute__((aligned(CONFIG_ARCH_CACHE_LINE))) dbMask[ROUNDUP(CE_MEM_NODE_RSA_MAX, CONFIG_ARCH_CACHE_LINE)];
uint8_t __attribute__((aligned(CONFIG_ARCH_CACHE_LINE))) hash[ROUNDUP(CE_MEM_NODE_DIG_MAX, CONFIG_ARCH_CACHE_LINE)];
uint8_t __attribute__((aligned(CONFIG_ARCH_CACHE_LINE))) hash_padding[ROUNDUP(CE_MEM_NODE_HASH_PAD_MAX, CONFIG_ARCH_CACHE_LINE)];

struct ce_heap {
    size_t item_num;
    size_t used_num;
    size_t used_num_max;
    spin_lock_t lock;
    struct mem_node mem_node_list[CE_MEM_NODE_ITEM_NUM_MAX];
};

// ce_heap static vars
static struct ce_heap inheap = {
    .item_num = CE_MEM_NODE_ITEM_NUM_MAX,
    .used_num = 0,
    .used_num_max = 0,
    .lock = 0,
    .mem_node_list[0].size = CE_MEM_NODE_DIG_MAX,
    .mem_node_list[0].is_used = 0,
    .mem_node_list[0].ptr = hash,
    .mem_node_list[1].size = CE_MEM_NODE_HASH_PAD_MAX,
    .mem_node_list[1].is_used = 0,
    .mem_node_list[1].ptr = hash_padding,
    .mem_node_list[2].size = CE_MEM_NODE_RSA_MAX,
    .mem_node_list[2].is_used = 0,
    .mem_node_list[2].ptr = msg_padding,
    .mem_node_list[3].size = CE_MEM_NODE_RSA_MAX,
    .mem_node_list[3].is_used = 0,
    .mem_node_list[3].ptr = dbMask,
    .mem_node_list[4].size = CE_MEM_NODE_RSA_MAX,
    .mem_node_list[4].is_used = 0,
    .mem_node_list[4].ptr = rev_cpy_buf,
    .mem_node_list[5].size = CE_MEM_NODE_RSA_MAX_EXT,
    .mem_node_list[5].is_used = 0,
    .mem_node_list[5].ptr = msg_padding_ext,
};

osEventFlagsId_t g_ce_signal[VCE_COUNT];
uint8_t g_ce_int_arg[VCE_COUNT];
osEventFlagsId_t g_trng_signal;
bool g_ce_inited = false;
volatile uint32_t g_int_flag = 0;

struct mem_node *ce_malloc(size_t size)
{
    uint8_t i = 0;
    irq_state_t states;
    struct mem_node *return_node = NULL;

    states = spin_lock_irqsave(&(inheap.lock));

    for (i = 0; i < inheap.item_num; i++) {
        if ((inheap.mem_node_list[i].size >= size) && (inheap.mem_node_list[i].is_used == 0)) {
            inheap.mem_node_list[i].is_used = 1;
            inheap.used_num++;

            if (inheap.used_num > inheap.used_num_max) {
                inheap.used_num_max = inheap.used_num;
            }

            return_node = &(inheap.mem_node_list[i]);
            break;
        }
    }

    spin_unlock_irqrestore(&(inheap.lock), states);
    return return_node;
}

void ce_free(struct mem_node *mem_node)
{
    irq_state_t states;

    states = spin_lock_irqsave(&(inheap.lock));

    if ((mem_node != NULL) && (mem_node->is_used == 1)) {
        mem_node->is_used = 0;
        inheap.used_num--;
    }

    spin_unlock_irqrestore(&(inheap.lock), states);
}

addr_t addr_switch_to_ce(uint32_t vce_id, ce_addr_type_t addr_type_s, addr_t addr)
{
    addr_t addr_switch = addr;

    if (EXT_MEM == addr_type_s) {
        addr_switch = p2ap(addr);
    } else if (SRAM_PUB == addr_type_s || SRAM_SEC == addr_type_s) {
        addr_switch = get_sram_base(vce_id);
        addr_switch = ~addr_switch & addr;
    }

    return addr_switch;
}

static int ce_irq_handle(uint32_t irq, void *arg)
{
    uint32_t vce_id = *(uint32_t *)arg;

    //ssdk_printf(SSDK_DEBUG, "ce_irq_handle irq: 0x%x\n", readl(_ioaddr(REG_INTSTAT_CE_(vce_id))));
    writel(0x1f, _ioaddr(REG_INTCLR_CE_(vce_id)));
    //ssdk_printf(SSDK_DEBUG, "ce_irq_handle irq after clear: 0x%x\n", readl(_ioaddr(REG_INTSTAT_CE_(vce_id))));

    osEventFlagsSet(g_ce_signal[vce_id], SILEX_EVENT_COMPLETE);
    return 0;
}

static int trng_irq_handle(uint32_t irq, void *arg)
{
    osEventFlagsSet(&g_trng_signal, SILEX_EVENT_COMPLETE);
    ssdk_printf(SSDK_DEBUG, "trng_irq_handle\n");
    writel(0x0, REG_AP_APB_CRYPTOENGINE_TRNG_FIFOLEVEL);
    return 0;
}

void init_vce_key_interface(void)
{
#if (CONFIG_SILEX_CRYPTO_CPUID == 0) /* safety */
    addr_t src_saf_base = 0xfc200000;
    writel(0xffff, _ioaddr(src_saf_base + (0x210 << 10)));
#else /* (CONFIG_SILEX_CRYPTO_CPUID == 0) */
    addr_t src_sec_base = 0xf8280000;
    for (int i = 0; i < 8; i++) {
        writel(0xffff, _ioaddr(src_sec_base + i * 0x1000));
    }
#endif /* (CONFIG_SILEX_CRYPTO_CPUID == 0) */
}

int32_t ce_init(uint32_t vce_id)
{
    uint32_t int_base;

    if (vce_id >= VCE_COUNT) {
        return -1;
    }

    int_base = ZONE_VCE0_INT + vce_id;
    g_ce_int_arg[vce_id] = vce_id;

    g_ce_signal[vce_id] = osEventFlagsNew(NULL);
    irq_attach(int_base, &ce_irq_handle, (void *)&g_ce_int_arg[vce_id]);

    writel(0x1f, _ioaddr(REG_INTCLR_CE_(vce_id)));
    writel(0x1f, _ioaddr(REG_INTEN_CE_(vce_id)));

    //enable interrupt
    irq_enable(int_base);

    return 0;
}

int32_t ce_globle_init(void)
{
    if (g_ce_inited) {
        return 0;
    }

    ssdk_printf(SSDK_DEBUG, "ce_globle_init enter\n");
    spin_lock_init(&(inheap.lock));

    g_trng_signal = osEventFlagsNew(NULL);
    irq_attach(ZONE_TRNG_INT, &trng_irq_handle, NULL);
    irq_enable(ZONE_TRNG_INT);

    sram_config();
    init_vce_key_interface();

    trng_init(0);
    g_ce_inited = true;

    return 0;
}

void clean_cache_block(ce_block_t *data, uint32_t ce_id)
{
    if ((data == NULL) || (data->len == 0)) {
        return;
    }

    switch (data->addr_type) {
        case EXT_MEM:
            arch_clean_cache_range((addr_t)data->addr, data->len);

            break;

        case SRAM_PUB:
            arch_clean_cache_range(_vaddr((addr_t)data->addr + sram_addr(data->addr_type, ce_id)), data->len);
            break;

        default:
            return;
    }
}

void invalidate_cache_block(ce_block_t *data, uint32_t ce_id)
{
    if ((data == NULL) || (data->len == 0)) {
        return;
    }

    switch (data->addr_type) {
        case EXT_MEM:
            arch_invalidate_cache_range((addr_t)data->addr, data->len);

            break;

        case SRAM_PUB:
            arch_invalidate_cache_range(_vaddr((addr_t)data->addr + sram_addr(data->addr_type, ce_id)), data->len);
            break;

        default:
            return;
    }
}

void clean_cache(addr_t start, uint32_t size)
{
    arch_clean_cache_range(start, size);
}

void flush_cache(addr_t start, uint32_t size)
{
    arch_invalidate_cache_range(start, size);
}
