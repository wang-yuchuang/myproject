#pragma once

#include <driver/kunlun/mbox/sdrv_mbox_drv.h>

#define ENABLE_VIRTIO_DEBUG 1

/*
 * 1 * virtio service + 32 * virtio device
 */
#define VIRTIO_DEVICE_NUM       33
/*
 * size of reg area: 0x100
 * size of cfg area: 0x100
 */
#define VIRTIO_DEVICE_REG_SIZE  0x100
#define VIRTIO_DEVICE_CFG_SIZE  0x100
/*
 * vq count: 2
 * max length: 128
 * alignment: 64 bytes
 */
#define VIRTIO_DEVICE_VQ_SIZE   0x1b00
/*
 * sum of all reg+cfg area
 * sum of all vq area
 */
#define VIRTIO_DEVICE_REG_TOTAL (VIRTIO_DEVICE_NUM*(VIRTIO_DEVICE_REG_SIZE+VIRTIO_DEVICE_CFG_SIZE))
#define VIRTIO_DEVICE_VQ_TOTAL  (VIRTIO_DEVICE_NUM*VIRTIO_DEVICE_VQ_SIZE)
/*
 * 33 * (0x200 + 0x1b00) + 16 * (0x200 + 0x200)
 * align to 4K
 * Note: 0x3fd00 ~ 0x40000 is unused
 */
#define VIRTIO_MEMSIZE          0x40000

#define VIRTIO_VQ_ALIGN         64

/*
 * virtio domain
 */
#define DOMAIN_SAF      0
#define DOMAIN_SEC      1
#define DOMAIN_MP       2
#define DOMAIN_AP1      3
#define DOMAIN_AP2      4
#define DOMAIN_MAX      5

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

/*
 * peer info
 */
typedef struct {
    int domain;
    u32 mbox_addr;
    paddr_t mem_base;
} peer_t;

/*
 * virtio config for frontend or backend
 */
typedef struct {
    int my_domain;
    int peer_num;
    peer_t peers[];
} cfg_t;

const cfg_t *get_backend_config(void);
const cfg_t *get_frontend_config(void);
int get_domain_from_rproc(sdrv_mbox_rproc_t rproc);
sdrv_mbox_rproc_t get_rproc_from_domain(int domain);
const char *get_domain_name(int domain);

#define VM_MT_REG       0x0
#define VM_MT_CFG       0x1
#define VM_MT_BOOT      0x2
#define VM_MT_IRQ       0x3
#define VM_VID_MASK     0x3f

/*
 * virtio message
 */
typedef struct {
    u16 mt:2;
    u16 ac:1;
    u16 ar:1;
    u16 mid:6;
    u16 vid:6;

    u8 offset;
    u8 length;
} msg_t;

/* mark if physical address is from ap domain */
#define VP_CFG_ADDR_AP_DOMAIN   0x10

/*
 * virtio device info
 */
typedef struct {
    u32 vid;
    u32 addr;
} vdi_t;

/*
 * virtio provider message type
 */
enum {
    VPM_TEXT = 0,
    VPM_TIMESTAMP,
    VPM_DEVICE,
    VPM_ARITHMETIC,
};

/*
 * RX buf len - sizeof(type) - sizeof(len) - 1
 */
#define VPM_TEXT_MAXLEN         247

/*
 * virtio provider message
 */
typedef struct {
    u32 type;
    union {
        struct {
            u32 len;
            char str[];
        } text;
        struct {
            u32 client_send;
            u32 server_recv;
            u32 server_send;
            u32 client_recv;
        } timestamp;
        struct {
            u32 num;
            vdi_t devs[];
        } device;
        struct {
            u32 operand1;
            u32 operand2;
            u32 _operator;
            u32 result;
        } arithmetic;
    };
} __attribute__((packed, aligned(4))) vpm_t;

typedef struct {
    void (*on_reg_changed)(u32 vid, u8 offset, void *priv);
    void (*on_cfg_changed)(u32 vid, u8 offset, u8 length, void *priv);
} vbo_t;

typedef struct {
    void (*on_backend_found)(paddr_t reg, void *priv);
    void (*on_irq)(u32 vid, void *priv);
} vfo_t;

#if __HAS_BE && __HAS_FE
void virtio_agent_update_reg(u32 vid, u8 offset);
void virtio_agent_update_cfg(u32 vid, u8 offset, u8 length);
void virtio_agent_send_irq(u32 vid);
void virtio_agent_register_backend(vbo_t *vbo, paddr_t reg, void *priv);
void virtio_agent_register_frontend(vfo_t *vfo, void *priv);
#else
static inline void virtio_agent_update_reg(u32 vid, u8 offset) {
}
static inline void virtio_agent_update_cfg(u32 vid, u8 offset, u8 length) {
}
static inline void virtio_agent_send_irq(u32 vid) {
}
static inline void virtio_agent_register_backend(vbo_t *vbo, paddr_t reg, void *priv) {
}
static inline void virtio_agent_register_frontend(vfo_t *vfo, void *priv) {
}
#endif

#define VIRTIO_MBOX_ADDR_INVALID        0

#define VIRTIO_MBOX_ADDR_SAF_SAF        VIRTIO_MBOX_ADDR_INVALID
#define VIRTIO_MBOX_ADDR_SAF_SEC        0xE0
#define VIRTIO_MBOX_ADDR_SAF_MP         0xE1
#define VIRTIO_MBOX_ADDR_SAF_AP1        0xE2
#define VIRTIO_MBOX_ADDR_SAF_AP2        0xE3

#define VIRTIO_MBOX_ADDR_SEC_SAF        0xE4
#define VIRTIO_MBOX_ADDR_SEC_SEC        VIRTIO_MBOX_ADDR_INVALID
#define VIRTIO_MBOX_ADDR_SEC_MP         0xE5
#define VIRTIO_MBOX_ADDR_SEC_AP1        0xE6
#define VIRTIO_MBOX_ADDR_SEC_AP2        0xE7

#define VIRTIO_MBOX_ADDR_MP_SAF         0xE8
#define VIRTIO_MBOX_ADDR_MP_SEC         0xE9
#define VIRTIO_MBOX_ADDR_MP_MP          VIRTIO_MBOX_ADDR_INVALID
#define VIRTIO_MBOX_ADDR_MP_AP1         0xEA
#define VIRTIO_MBOX_ADDR_MP_AP2         0xEB

void virtio_backend_init(void);
void virtio_frontend_init(void);

