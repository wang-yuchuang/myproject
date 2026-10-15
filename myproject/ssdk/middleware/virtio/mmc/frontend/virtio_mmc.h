/**
 * @file virtio_mmc.h
 * @brief virtio mmc device header.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#pragma once

#include <stdint.h>
#include <compiler.h>

#define VIRTIO_MMC_CDB_DEFAULT_SIZE (16)
#define VIRTIO_MMC_VIRTQUEUE_SIZE   (16)

/* Feature Bits */
#define VIRTIO_MMC_F_CAPABILITY     (1ull<<0)
#define VIRTIO_MMC_F_ERR_RECOVERY   (1ull<<1)
#define VIRTIO_MMC_F_CQE            (1ull<<2)
#define VIRTIO_MMC_F_RO             (1ull<<3)
#define VIRTIO_MMC_F_BLK_SIZE       (1ull<<4)
#define VIRTIO_MMC_F_CACHE          (1ull<<5)
#define VIRTIO_MMC_F_BARRIER        (1ull<<6)
#define VIRTIO_MMC_F_RETUNING       (1ull<<7)
#define VIRTIO_MMC_F_HOTPLUG        (1ull<<8)
#define VIRTIO_MMC_F_VERSION_1      (1ull<<32)

/* mmc cmd descriptor type */
#define VIRTIO_MMC_CDB_SDHCI (0)
#define VIRTIO_MMC_CDB_CQHCI (1)

/* request patition type */
#define VIRTIO_MMC_PART_DEFAULT     (0)
#define VIRTIO_MMC_PART_BOOT1       (1)
#define VIRTIO_MMC_PART_BOOT2       (2)
#define VIRTIO_MMC_PART_RPMB        (3)
#define VIRTIO_MMC_PART_GP1         (4)
#define VIRTIO_MMC_PART_GP2         (5)
#define VIRTIO_MMC_PART_GP3         (6)
#define VIRTIO_MMC_PART_GP4         (7)

/* mmc cmd descriptor buf mode */
#define VIRTIO_MMC_SDHCI_BUF_NORMAL             (0)
#define VIRTIO_MMC_SDHCI_BUF_ADDR               (1)
#define VIRTIO_MMC_SDHCI_BUF_AMDA2_32BITS_DESC  (2)
#define VIRTIO_MMC_SDHCI_BUF_AMDA2_64BITS_DESC  (3)
#define VIRTIO_MMC_SDHCI_BUF_AMDA3_DESC	        (4)

/* Response status codes of sdhci host */
#define VIRTIO_MMC_SDHCI_S_OK                   (0)
#define VIRTIO_MMC_SDHCI_S_CMD_TOUT_ERR         (1<<0)
#define VIRTIO_MMC_SDHCI_S_CMD_CRC_ERR          (1<<1)
#define VIRTIO_MMC_SDHCI_S_CMD_END_BIT_ERR      (1<<2)
#define VIRTIO_MMC_SDHCI_S_CMD_IDX_ERR          (1<<3)
#define VIRTIO_MMC_SDHCI_S_DATA_TOUT_ERR        (1<<4)
#define VIRTIO_MMC_SDHCI_S_DATA_CRC_ERR         (1<<5)
#define VIRTIO_MMC_SDHCI_S_DATA_END_BIT_ERR     (1<<6)
#define VIRTIO_MMC_SDHCI_S_CUR_LMT_ERR          (1<<7)
#define VIRTIO_MMC_SDHCI_S_AUTO_CMD_ERR         (1<<8)
#define VIRTIO_MMC_SDHCI_S_AMDA_ERR             (1<<9)
#define VIRTIO_MMC_SDHCI_S_TUNING_ERR           (1<<10)
#define VIRTIO_MMC_SDHCI_S_RESP_ERR             (1<<11)
#define VIRTIO_MMC_SDHCI_S_BOOT_ACK_ERR         (1<<12)
#define VIRTIO_MMC_SDHCI_S_UNSUPP               (1<<31)

/* Events.  */
#define VIRTIO_MMC_T_EVENTS_MISSED            (0x80000000)
#define VIRTIO_MMC_T_NO_EVENT                 (0)
#define VIRTIO_MMC_T_TRANSPORT_RESET          (1)
// #define VIRTIO_MMC_T_ASYNC_NOTIFY             2
// #define VIRTIO_MMC_T_PARAM_CHANGE             3

/* Reasons of transport reset event */
#define VIRTIO_MMC_EVT_RESET_HARD             (0)
#define VIRTIO_MMC_EVT_RESET_RESCAN           (1)
#define VIRTIO_MMC_EVT_RESET_REMOVED          (2)

struct virtio_mmc_config {
    uint8_t dev_id;
    uint8_t dev_type;
    uint8_t part_id;
    uint8_t reserve01;
    uint32_t sector_size;
    uint32_t sector;
    uint32_t num_sectors;
} __PACKED;

struct virtio_mmc_sdhci_cmd_desc {
    uint32_t sector;
    uint32_t num_sectors;
    uint16_t cmd_index;     /* Command index */
    uint16_t cmd_timeout;   /* Command timeout in ms */
    uint32_t argument;      /* Command argument */
    uint8_t resp_type;      /* Response type of the command */
    uint8_t cmd_retry;      /* Retry the command, if card is busy */
    uint8_t cmd23_support;  /* If card supports cmd23 */
    uint8_t part_type;      /* request patition type */
    uint8_t buf_mode;
} __PACKED;

/* mmc command request, followed by data-out */
struct virtio_mmc_cmd_req {
    uint64_t id;        /* Command identifier */
    uint16_t priority;   /* command priority field */
    uint16_t cdb_type;   /* mmc cmd descriptor type */
    uint8_t cdb[VIRTIO_MMC_CDB_DEFAULT_SIZE];   /* mmc cmd descriptor data */
} __PACKED;

/* Response, followed by sense data and data-in */
struct virtio_mmc_cmd_resp {
    uint32_t resp[4];   /* 128 bit response value */
    uint32_t status;    /* Command completion status */
} __PACKED;

struct virtio_mmc_event {
    uint32_t event;
    uint32_t reason;
} __PACKED;
