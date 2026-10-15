/**
 * @file sdrv_mbox_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv mbox interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_SDRV_MBOX_H
#define INCLUDE_DRV_SDRV_MBOX_H

#include <types.h>
#include <compiler.h>

#include <driver/mbox.h>

#define SDRV_MBOX_RPROC_NUM         8
#define SDRV_MBOX_CHN_NUM           32

#define SDRV_MBOX_ANY_ADDR          -1

#define SDRV_MBOX_PROTO_DFT         0
#define SDRV_MBOX_PROTO_ROM         1
#define SDRV_MBOX_PROTO_RPMSG       2
#define SDRV_MBOX_PROTO_DSP         3

#define SDRV_MBOX_SHORT_BUF_LEN     8

#ifndef ASSEMBLY

__BEGIN_CDECLS

#define SDRV_MBOX_MTU               (512 - sizeof(struct sdrv_mbox_hdr))

/* sdrv mbox rproc id */
typedef enum {
    MBOX_RPROC_SAF   = 0,
    MBOX_RPROC_SEC   = 1,
    MBOX_RPROC_MP    = 2,
    MBOX_RPROC_AP1   = 3,
    MBOX_RPROC_AP2   = 4,
    MBOX_RPROC_VDSP  = 5,
} sdrv_mbox_rproc_t;

/* sdrv mbox address id */
typedef enum {
    MBOX_ADDR_INVALID           = 0x0,
    MBOX_ADDR_MB_ECHO           = 0x04,
    MBOX_ADDR_MB_TEST           = 0x05,
    MBOX_ADDR_ECHO_TEST         = 0x08,
    MBOX_ADDR_RPMSG_VIRTIO      = 0x10,
    MBOX_ADDR_RPMSG_TEST        = 0x20,
    MBOX_ADDR_RPMSG_IPCC        = 0x30,
    MBOX_ADDR_SDPE_RPC_BASE     = 0x60,
    MBOX_ADDR_MBOX_RAW          = 0x80,
    MBOX_ADDR_VDEV_BASE         = 0xb0,
    MBOX_ADDR_LEGACY            = 0xfe,
    MBOX_ADDR_MAX               = 0xff,
} sdrv_mbox_addr_t;

/* sdrv mbox message header */
typedef struct sdrv_mbox_hdr {
    uint16_t protocol: 4;
    uint16_t rproc   : 3;
    uint16_t priority: 1;
    uint16_t addr    : 8;
    uint16_t data_len;
    uint8_t data[0];
} __PACKED sdrv_mbox_hdr_t;

/* sdrv chan req priv data */
typedef struct sdrv_mbox_chan_req {
    uint32_t                rproc;
    int                     src_addr;       /* -1: any addr */
    int                     dest_addr;      /* -1: any addr */
} sdrv_mbox_chan_req_t;

/* sdrv mbox chan priv */
typedef struct sdrv_mbox_chan {
    uint8_t                 used;
    uint8_t                 msg_id;
    uint8_t                 rproc;
    uint8_t                 padding;
    uint16_t                src_addr;
    uint16_t                dest_addr;
} sdrv_mbox_chan_t;

/* sdrv mbox dev */
typedef struct sdrv_mbox_dev {
    mbox_dev_t              dev;
    mbox_chan_t             chan[SDRV_MBOX_CHN_NUM];
    sdrv_mbox_chan_t        priv[SDRV_MBOX_CHN_NUM];
    uint32_t                msg_bitmap;
    spin_lock_t             lock;
} sdrv_mbox_dev_t __ALIGNED(4);

/**
 * @brief sdrv mbox probe
 *
 * @param[in] sdrv_mbox sdrv mbox dev
 * @param[in] cfg mbox config
 * return mbox dev
 */
mbox_dev_t *sdrv_mbox_probe(sdrv_mbox_dev_t *sdrv_mbox,
                            const mbox_config_t *cfg);

__END_CDECLS

#endif

#endif
