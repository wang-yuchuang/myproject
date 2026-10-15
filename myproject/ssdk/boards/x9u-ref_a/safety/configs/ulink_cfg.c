/**
 * @file ulink_cfg.c
 * @unilink resource configurations.
 *
 * @copyright Copyright (c) 2023  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifdef CONFIG_UNILINK
#include "ulink.h"

static ulink_channel_desc_t ulink_channel_local_desc_ap1 = {
    (SERVER_BASE + 0x4000000),
    (SERVER_BASE + 0x4400000),
    0x200000,
    ulink_irq_mbox,
    0,
    0xb0,
};
static ulink_channel_desc_t ulink_channel_local_desc_ap2 = {
    (SERVER_BASE + 0x4200000),
    (SERVER_BASE + 0x4a00000),
    0x200000,
    ulink_irq_mbox,
    0,
    0xc0,
};
static ulink_channel_desc_t ulink_channel_server_desc_saf = {
    (CLIENT_BASE + 0x400000),
    (SERVER_BASE + 0x400000),
    0x400000,
    ulink_irq_gpio,
    0x00,
    0x00,
};
static ulink_channel_desc_t ulink_channel_server_desc_ap1 = {
    (CLIENT_BASE + 0x800000),
    (SERVER_BASE + 0x1400000),
    0x400000,
    ulink_irq_gpio,
    0x03,
    0x30,
};
static ulink_channel_desc_t ulink_channel_server_desc_ap2 = {
    (CLIENT_BASE + 0xc00000),
    (SERVER_BASE + 0x2400000),
    0x400000,
    ulink_irq_gpio,
    0x04,
    0x40,
};

ulink_channel_desc_t *ulink_channel_server_desc[ALL_DP_CPU_MAX] = {
    NULL,
    NULL, // sec
    NULL, // mp
    &ulink_channel_local_desc_ap1,
    &ulink_channel_local_desc_ap2,
    NULL, // DSP
    &ulink_channel_server_desc_saf,
    NULL, // sec
    NULL,
    &ulink_channel_server_desc_ap1,
    &ulink_channel_server_desc_ap2,
    NULL, // DSP
};

ulink_mini_client_t ulink_mini_client[] = {
    {DP_CA_AP1, 0xb8, 0x03, 0x30}, {DP_CA_AP2, 0xc8, 0x04, 0x40},
    {DP_CA_AP1, 0xbc, 0x43, 0x34}, {DP_CA_AP2, 0xcb, 0x34, 0x43},
    {DP_CA_AP1, 0xbb, 0x33, 0x33}, {DP_CA_AP2, 0xcc, 0x44, 0x44},
    {DP_CPU_MAX,0,0,0}
};

#endif
