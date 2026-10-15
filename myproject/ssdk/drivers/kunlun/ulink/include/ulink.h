#ifndef __ULINK_HEAD__
#define __ULINK_HEAD__

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <lib/list.h>
#include <cmsis_os2.h>
#include "ulink_cfg.h"

#ifndef _def_domain_cpu_id_t
#define _def_domain_cpu_id_t
typedef enum {
    DP_CR5_SAF,
    DP_CR5_SEC,
    DP_CR5_MPC,
    DP_CA_AP1,
    DP_CA_AP2,
    DP_DSP_V,
    DP_CPU_MAX,
    R_DP_CR5_SAF = DP_CPU_MAX,
    R_DP_CR5_SEC,
    R_DP_CR5_MPC,
    R_DP_CA_AP1,
    R_DP_CA_AP2,
    R_DP_DSP_V,
    ALL_DP_CPU_MAX,
} domain_cpu_id_t;
#endif

typedef enum {
    MAGIC_READY = 0x5a5a5a5a,
    MAGIC_CLEAN = 0xa5a5a5a5,
} channel_magic;

typedef struct {
    uint32_t magic;
    uint32_t addr_main;
    uint32_t addr_sub;
    uint32_t size;
    uint8_t payload[];
} __attribute__((__packed__)) ulink_channel_block_t;

typedef struct ulink_channel {
    uint32_t rproc;
    uint32_t addr_sub;
    void (*callback)(struct ulink_channel *ch, uint8_t *data, uint32_t size);;
    void *user_data;
} ulink_channel_t;


typedef struct {
    uint32_t block_num;
    uint32_t tx;
    uint32_t rx;
} ulink_channel_head_t;

typedef enum {
    ulink_irq_gpio,
    ulink_irq_mbox,
} ulink_irq_type;

typedef struct {
    uint32_t tx_mem;
    uint32_t rx_mem;
    uint32_t mem_size;
    ulink_irq_type irq_type;
    uint32_t tx_irq;
    uint32_t rx_irq;
    ulink_channel_head_t *tx_head;
    ulink_channel_head_t *rx_head;
    struct mbox_chan *chan;
    osMutexId_t tx_lock;
    osMutexId_t rx_lock;
    osMutexId_t ch_lock;

    osEventFlagsId_t event;
    struct list_node channels;
} ulink_channel_desc_t;

typedef struct {
    uint32_t rproc;
    uint32_t addr;
    uint32_t irq_to_mbox;
    uint32_t mbox_to_irq;
    struct mbox_chan *chan;
} ulink_mini_client_t;

typedef void (*ulink_rx_callback)(ulink_channel_t *ch, uint8_t *data, uint32_t size);

void ulink_irq_init(void);
void ulink_trigger_irq(int8_t irq_num);
void ulink_request_irq(int8_t irq_num, void(*func)(void*), void *data);
void ulink_irq_set_route(uint32_t irq_to_mbox, uint32_t mbox_to_irq,
                            uint32_t rproc, uint32_t addr,ulink_mini_client_t*client);

void ulink_channel_init(void);
ulink_channel_t *ulink_request_channel(uint32_t rproc, uint32_t sub_addr,  ulink_rx_callback f);
void ulink_release_channel(ulink_channel_t *ch);
int ulink_channel_send(ulink_channel_t *ch, uint8_t *data, uint32_t size);
uint32_t ulink_channel_rev(ulink_channel_t *ch, uint8_t **data);

int ulink_net_init(void);



#endif //__ULINK_HEAD__
