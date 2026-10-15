/**
 * @file ulink_cfg.c
 * @unilink resource configurations.
 *
 * @copyright Copyright (c) 2023  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifdef CONFIG_UNILINK
#include "ulink.h"

static ulink_channel_desc_t ulink_channel_local_desc_mp = {
    (SERVER_BASE + 0x00000000),
    (SERVER_BASE + 0x00600000),
    0x200000,
    ulink_irq_mbox,
    0,
    0xb7,
};

static ulink_channel_desc_t ulink_channel_local_desc_ap1 = {
    (SERVER_BASE + 0x00200000),
    (SERVER_BASE + 0x00c00000),
    0x200000,
    ulink_irq_mbox,
    0,
    0xb0,
};

static ulink_channel_desc_t ulink_channel_local_desc_ap2 = {
    (SERVER_BASE + 0x00400000),
    (SERVER_BASE + 0x01200000),
    0x200000,
    ulink_irq_mbox,
    0,
    0xc0,
};

ulink_channel_desc_t *ulink_channel_server_desc[ALL_DP_CPU_MAX] = {
    NULL,
    NULL, // sec
    &ulink_channel_local_desc_mp,
    &ulink_channel_local_desc_ap1,
    &ulink_channel_local_desc_ap2,
    NULL, // DSP
    NULL,
    NULL, // sec
    NULL,
    NULL,
    NULL,
    NULL, // DSP
};

ulink_mini_client_t ulink_mini_client[] = {
    {DP_CPU_MAX,0,0,0}
};

#endif
