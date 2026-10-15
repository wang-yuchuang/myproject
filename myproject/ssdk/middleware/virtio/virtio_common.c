#include <debug.h>
#include <image_cfg.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <spinlock.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sdrv-virtio.h>
#include <virtio_init.h>
#include "virtio_common.h"


#define RMPSG_SIZE                      (0x100000u)
#define HSM_SIZE                        (0x20000u)

/* local */
#define VIRTIO_MEM_BASE_SAF_SAF         ((u32)0)
#define VIRTIO_MEM_BASE_SEC_SEC         ((u32)0)
#define VIRTIO_MEM_BASE_MP_MP           ((u32)0)

/* saf<->sec */
#ifndef SAF_SEC_MEMBASE
#define VIRTIO_MEM_BASE_SAF_SEC         (0u)
#define VIRTIO_MEM_BASE_SEC_SAF         (0u)
#else
#define VIRTIO_MEM_BASE_SAF_SEC         ((u32)(SAF_SEC_MEMBASE+RMPSG_SIZE+HSM_SIZE))
#define VIRTIO_MEM_BASE_SEC_SAF         ((u32)(SAF_SEC_MEMBASE+RMPSG_SIZE+HSM_SIZE+VIRTIO_MEMSIZE))
#endif

/* saf<->mp */
#ifndef SAF_MP_MEMBASE
#define VIRTIO_MEM_BASE_SAF_MP          (0u)
#define VIRTIO_MEM_BASE_MP_SAF          (0u)
#else
#define VIRTIO_MEM_BASE_SAF_MP          ((u32)(SAF_MP_MEMBASE+RMPSG_SIZE+HSM_SIZE))
#define VIRTIO_MEM_BASE_MP_SAF          ((u32)(SAF_MP_MEMBASE+RMPSG_SIZE+HSM_SIZE+VIRTIO_MEMSIZE))
#endif

/* saf<->ap1 */
#ifndef SAF_AP1_MEMBASE
#define VIRTIO_MEM_BASE_SAF_AP1         (0u)
#else
#define VIRTIO_MEM_BASE_SAF_AP1         ((u32)(SAF_AP1_MEMBASE+RMPSG_SIZE+HSM_SIZE))
#endif

/* saf<->ap2 */
#ifndef SAF_AP2_MEMBASE
#define VIRTIO_MEM_BASE_SAF_AP2         (0u)
#else
#define VIRTIO_MEM_BASE_SAF_AP2         ((u32)(SAF_AP2_MEMBASE+RMPSG_SIZE+HSM_SIZE))
#endif

/* sec<->mp */
#ifndef SEC_MP_MEMBASE
#define VIRTIO_MEM_BASE_SEC_MP          (0u)
#define VIRTIO_MEM_BASE_MP_SEC          (0u)
#else
#define VIRTIO_MEM_BASE_SEC_MP          ((u32)(SEC_MP_MEMBASE+RMPSG_SIZE+HSM_SIZE))
#define VIRTIO_MEM_BASE_MP_SEC          ((u32)(SEC_MP_MEMBASE+RMPSG_SIZE+HSM_SIZE+VIRTIO_MEMSIZE))
#endif

/* sec<-> ap1 */
#ifndef SEC_AP1_MEMBASE
#define VIRTIO_MEM_BASE_SEC_AP1         (0u)
#else
#define VIRTIO_MEM_BASE_SEC_AP1         ((u32)(SEC_AP1_MEMBASE+RMPSG_SIZE+HSM_SIZE))
#endif

/* sec<->ap2 */
#ifndef SEC_AP2_MEMBASE
#define VIRTIO_MEM_BASE_SEC_AP2         (0u)
#else
#define VIRTIO_MEM_BASE_SEC_AP2         ((u32)(SEC_AP2_MEMBASE+RMPSG_SIZE+HSM_SIZE))
#endif

/* mp<->ap1 */
#ifndef MP_AP1_MEMBASE
#define VIRTIO_MEM_BASE_MP_AP1          (0u)
#else
#define VIRTIO_MEM_BASE_MP_AP1          ((u32)(MP_AP1_MEMBASE+RMPSG_SIZE+HSM_SIZE))
#endif

/* mp<->ap2 */
#ifndef MP_AP2_MEMBASE
#define VIRTIO_MEM_BASE_MP_AP2          (0u)
#else
#define VIRTIO_MEM_BASE_MP_AP2          ((u32)(MP_AP2_MEMBASE+RMPSG_SIZE+HSM_SIZE))
#endif

#define DECLARE_BACKEND_CFG_FULL(_domain) \
    cfg_t __be_cfg_##_domain __attribute__((unused)) = {\
        .my_domain = DOMAIN_##_domain,\
        .peer_num = 5,\
        .peers[0].domain = DOMAIN_SAF,\
        .peers[0].mbox_addr = VIRTIO_MBOX_ADDR_##_domain##_SAF,\
        .peers[0].mem_base = VIRTIO_MEM_BASE_##_domain##_SAF,\
        .peers[1].domain = DOMAIN_SEC,\
        .peers[1].mbox_addr = VIRTIO_MBOX_ADDR_##_domain##_SEC,\
        .peers[1].mem_base = VIRTIO_MEM_BASE_##_domain##_SEC,\
        .peers[2].domain = DOMAIN_MP,\
        .peers[2].mbox_addr = VIRTIO_MBOX_ADDR_##_domain##_MP,\
        .peers[2].mem_base = VIRTIO_MEM_BASE_##_domain##_MP,\
        .peers[3].domain = DOMAIN_AP1,\
        .peers[3].mbox_addr = VIRTIO_MBOX_ADDR_##_domain##_AP1,\
        .peers[3].mem_base = VIRTIO_MEM_BASE_##_domain##_AP1,\
        .peers[4].domain = DOMAIN_AP2,\
        .peers[4].mbox_addr = VIRTIO_MBOX_ADDR_##_domain##_AP2,\
        .peers[4].mem_base = VIRTIO_MEM_BASE_##_domain##_AP2,\
    }

#define BACKEND_CFG_PTR(_domain) &__be_cfg_##_domain

#define DECLARE_FRONTEND_CFG_FULL(_domain) \
    cfg_t __fe_cfg_##_domain __attribute__((unused)) = {\
        .my_domain = DOMAIN_##_domain,\
        .peer_num = 3,\
        .peers[0].domain = DOMAIN_SAF,\
        .peers[0].mbox_addr = VIRTIO_MBOX_ADDR_SAF_##_domain,\
        .peers[0].mem_base = VIRTIO_MEM_BASE_SAF_##_domain,\
        .peers[1].domain = DOMAIN_SEC,\
        .peers[1].mbox_addr = VIRTIO_MBOX_ADDR_SEC_##_domain,\
        .peers[1].mem_base = VIRTIO_MEM_BASE_SEC_##_domain,\
        .peers[2].domain = DOMAIN_MP,\
        .peers[2].mbox_addr = VIRTIO_MBOX_ADDR_MP_##_domain,\
        .peers[2].mem_base = VIRTIO_MEM_BASE_MP_##_domain,\
    }

#define FRONTEND_CFG_PTR(_domain) &__fe_cfg_##_domain


static const cfg_t __dummy __attribute__((unused)) = {
    .my_domain = __MY_DOMAIN,
    .peer_num = 0,
};

#if __HAS_BE
static const DECLARE_BACKEND_CFG_FULL(SAF);
static const DECLARE_BACKEND_CFG_FULL(SEC);
static const DECLARE_BACKEND_CFG_FULL(MP);

const cfg_t *get_backend_config(void)
{
#if (__MY_DOMAIN == DOMAIN_SAF)
    return BACKEND_CFG_PTR(SAF);
#elif (__MY_DOMAIN == DOMAIN_SEC)
    return BACKEND_CFG_PTR(SEC);
#elif (__MY_DOMAIN == DOMAIN_MP)
    return BACKEND_CFG_PTR(MP);
#else
    /* does not support other domain now */
    return &__dummy;
#endif
}
#endif

#if __HAS_FE
static const DECLARE_FRONTEND_CFG_FULL(SAF);
static const DECLARE_FRONTEND_CFG_FULL(SEC);
static const DECLARE_FRONTEND_CFG_FULL(MP);
static const DECLARE_FRONTEND_CFG_FULL(AP1);
static const DECLARE_FRONTEND_CFG_FULL(AP2);

const cfg_t *get_frontend_config(void)
{
#if (__MY_DOMAIN == DOMAIN_SAF)
    return FRONTEND_CFG_PTR(SAF);
#elif (__MY_DOMAIN == DOMAIN_SEC)
    return FRONTEND_CFG_PTR(SEC);
#elif (__MY_DOMAIN == DOMAIN_MP)
    return FRONTEND_CFG_PTR(MP);
#elif (__MY_DOMAIN == DOMAIN_AP1)
    return FRONTEND_CFG_PTR(AP1);
#elif (__MY_DOMAIN == DOMAIN_AP2)
    return FRONTEND_CFG_PTR(AP2);
#else
    /* does not support other domain now */
    return &__dummy;
#endif
}
#endif

static const int __rproc_to_domain[] = {
    [MBOX_RPROC_SAF] = DOMAIN_SAF,
    [MBOX_RPROC_SEC] = DOMAIN_SEC,
    [MBOX_RPROC_MP] = DOMAIN_MP,
    [MBOX_RPROC_AP1] = DOMAIN_AP1,
    [MBOX_RPROC_AP2] = DOMAIN_AP2,
};

int get_domain_from_rproc(sdrv_mbox_rproc_t rproc)
{
    if (rproc > MBOX_RPROC_AP2)
        return DOMAIN_MAX;

    return __rproc_to_domain[rproc];
}

static const u8 __domain_to_rproc[] = {
    [DOMAIN_SAF] = MBOX_RPROC_SAF,
    [DOMAIN_SEC] = MBOX_RPROC_SEC,
    [DOMAIN_MP] = MBOX_RPROC_MP,
    [DOMAIN_AP1] = MBOX_RPROC_AP1,
    [DOMAIN_AP2] = MBOX_RPROC_AP2,
};

sdrv_mbox_rproc_t get_rproc_from_domain(int domain)
{
    if (domain < 0 || domain >= DOMAIN_MAX)
        return MBOX_RPROC_AP2+1;/* MBOX_RPROC_AP2+1 is MBOX_RPROC_VDSP */

    return __domain_to_rproc[domain];
}

static const char *__domain_names[DOMAIN_MAX] = {
    "saf",
    "sec",
    "mp",
    "ap1",
    "ap2",
};

const char *get_domain_name(int domain) {
    if (domain < 0 || domain >= DOMAIN_MAX)
        return "(null)";

    return __domain_names[domain];
}

#if __HAS_BE && __HAS_FE
typedef struct {
    vbo_t *vbo;
    paddr_t reg;
    void *be;

    vfo_t *vfo;
    void *fe;
} agent_t;

static agent_t __agent = {
    .vbo = NULL,
    .reg = 0,
    .be = NULL,
    .vfo = NULL,
    .fe = NULL,
};
static spin_lock_t __lock = SPIN_LOCK_INITIAL_VALUE;

void virtio_agent_update_reg(u32 vid, u8 offset)
{
    __agent.vbo->on_reg_changed(vid, offset, __agent.be);
}

void virtio_agent_update_cfg(u32 vid, u8 offset, u8 length)
{
    __agent.vbo->on_cfg_changed(vid, offset, length, __agent.be);
}

void virtio_agent_send_irq(u32 vid)
{
    __agent.vfo->on_irq(vid, __agent.fe);
}

void virtio_agent_register_backend(vbo_t *vbo, paddr_t reg, void *priv)
{
    if (!vbo || !vbo->on_reg_changed || !vbo->on_cfg_changed) {
        ssdk_printf(SSDK_CRIT, "invalid backend ops\n");
        return;
    }

    int probe = 0;
    irq_state_t state;
    state = spin_lock_irqsave(&__lock);
    __agent.vbo = vbo;
    __agent.reg = reg;
    __agent.be = priv;

    if (__agent.vfo)
        probe = 1;
    spin_unlock_irqrestore(&__lock, state);

    if (probe)
        __agent.vfo->on_backend_found(__agent.reg, __agent.fe);
}

void virtio_agent_register_frontend(vfo_t *vfo, void *priv)
{
    if (!vfo || !vfo->on_backend_found || !vfo->on_irq) {
        ssdk_printf(SSDK_CRIT, "invalid frontend ops\n");
        return;
    }

    int probe = 0;
    irq_state_t state;
    state = spin_lock_irqsave(&__lock);
    __agent.vfo = vfo;
    __agent.fe = priv;

    if (__agent.vbo)
        probe = 1;
    spin_unlock_irqrestore(&__lock, state);

    if (probe)
        __agent.vfo->on_backend_found(__agent.reg, __agent.fe);
}
#endif

void virtio_init(void)
{
#if __HAS_BE
    virtio_backend_init();
#endif
#if __HAS_FE
    virtio_frontend_init();
#endif
}

