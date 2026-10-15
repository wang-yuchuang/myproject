/**
 * @file sd_boot_img.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SD_BOOT_IMG_H
#define _SD_BOOT_IMG_H

#define SFS_TAG  (0x53465301)
#define SFS_TAG_OFFSET         (0x0)
#define SFS_INIT_ACT_OFFSET    (0x4)
#define SFS_XFER_CONFIG_OFFSET (0x40)
#define SFS_IP_SETTINGS_OFFSET (0x50)
#define SFS_FREQ_OFFSET        (0x60)
#define SFS_SW_RESET           (0x67)
#define SFS_TP_OFFSET          (0x68)
#define SFS_NIA_OFFSET         (0x70)
#define SFS_BIA_OFFSET         (0x74)
#define SFS_TIA_OFFSET         (0x78)
#define SFS_CRC32_OFFSET       (0x7C)

#define SFS_SIZE 128
#define SFS_INIT_ACT_SIZE      (0x3C)
#define SFS_XFER_CONFIG_SIZE   (0x10)
#define SFS_IP_SETTINGS_SIZE   (0x10)
#define SFS_TP_SIZE            (0x8)

#define BPT_TAG                (0x42505402)
#define BPT_TAG_OFFSET         (0x0)
#define BPT_SIZE_OFFSET        (0x6)
#define BPT_SEC_VER_OFFSET     (0x8)
#define BPT_MDA_OFFSET         (0xC)
#define BPT_IIB_OFFSET         (0x20)
#define BPT_RCP_OFFSET         (0x400)
#define BPT_SIG_OFFSET         (0x814)
#define BPT_KW_OFFSET          (0xA14)
#define BPT_CRC32_OFFSET       (0xFEC)
#define BPT_PSN_OFFSET         (0xFF0)
#define BPT_IPSN_OFFSET        (0xFF4)

#define BPT_SIZE             (0x1000)
#define BPT_RCP_SIZE         (0x414)
#define BPT_SIG_SIZE         (0x200)
#define BPT_KW_SIZE          (0x200)

#define IIB_TAG              (0xEAE2)
#define IIB_TAG_OFFSET       (0x0)
#define IIB_SIZE_OFFSET      (0x2)
#define IIB_DCC_OFFSET       (0x8)
#define IIB_DID_OFFSET       (0x10)
#define IIB_IMG_TYPE_OFFSET  (0x18)
#define IIB_TG_CORE_OFFSET   (0x19)
#define IIB_DCB_OFFSET       (0x1A)
#define IIB_IV_OFFSET        (0x1C)
#define IIB_DLP_OFFSET       (0x24)
#define IIB_IMG_SZ_OFFSET    (0x28)
#define IIB_LOAD_BASE_OFFSET (0x2C)
#define IIB_EP_OFFSET        (0x34)
#define IIB_HASH_OFFSET      (0x3C)

#define IIB_SIZE             (0x7C)
#define IIB_DCC_SIZE         (0x8)
#define IIB_DID_SIZE         (0x8)
#define IIB_IV_SIZE          (0x8)
#define IIB_HASH_SIZE        (0x40)

#define RCP_TAG              (0xEAF0)
#define RCP_TAG_OFFSET       (0x0)
#define RCP_SIZE_OFFSET      (0x2)
#define RCP_ID_OFFSET        (0x4)
#define RCP_PKT_OFFSET       (0x5)
#define RCP_PK_OFFSET        (0x10)

#define RCP_PK_SIZE          (0x404)
#define RCP_SIZE             (0x414)

#define RSA_PUBLIC_KEY_N_SIZE_OFFSET (0)
#define RSA_PUBLIC_KEY_N_OFFSET      (0x4)
#define RSA_PUBLIC_KEY_E_OFFSET      (0x204)
#define RSA_PUBLIC_KEY_N_SIZE        (0x200)
#define RSA_PUBLIC_KEY_E_SIZE        (0x200)

#define ECDSA_PUBLIC_KEY_PN_SIZE_OFFSET (0)
#define ECDSA_PUBLIC_KEY_QX_OFFSET      (0x4)
#define ECDSA_PUBLIC_KEY_QY_OFFSET      (0x4C)
#define RSA_PUBLIC_KEY_QX_SIZE          (0x44)
#define RSA_PUBLIC_KEY_QY_SIZE          (0x44)

#define BOOT_PARTITION_NAME0    "boot0"
#define BOOT_PARTITION_NAME1    "boot1"
#define BOOT_PARTITION_NAME2    "boot2"
#define EMMC_BOOT_PT_SIZE      (0x400000)

typedef enum boot_pack_num {
    BOOT0_NUM = 0,
    BOOT1_NUM,
    BOOT2_NUM,
    BOOT_MAX
} BOOT_PACKAGE_NUM_E;

struct sfs {
    uint32_t tag;
    uint8_t  init_act[SFS_INIT_ACT_SIZE];
    uint8_t  xfer_config[SFS_XFER_CONFIG_SIZE];
    uint8_t  ospi_settings[SFS_IP_SETTINGS_SIZE];
    uint8_t  freq;
    uint8_t  training_pattern[SFS_TP_SIZE];
    uint8_t  sw_reset_info;
    uint32_t normal_img_base;
    uint32_t backup_img_base;
    uint32_t third_img_base;
    uint32_t crc32;
};


struct rcp {
    uint16_t tag;
    uint16_t size;
    uint8_t id;
    uint8_t pk_type;
    uint8_t pk[RCP_PK_SIZE];
};

struct iib {
    uint16_t tag;
    uint16_t size;
    uint8_t dcc[IIB_DCC_SIZE];
    uint8_t did[IIB_DID_SIZE];
    uint8_t image_type;
    uint8_t target_core;
    uint8_t decryp_ctl;
    uint8_t initial_vec[IIB_IV_SIZE];
    uint32_t dev_logic_page;
    uint32_t image_size;
    uint32_t load_base;
    uint32_t entry_point;
    uint8_t hash[IIB_HASH_SIZE];
};

struct bpt {
    uint32_t tag;
    uint16_t size;
    uint32_t sec_version;
    uint8_t hash_alg;
    struct iib iib[8];
    struct rcp rcp;
    uint8_t signature[BPT_SIG_SIZE];
    uint8_t key_wraps[BPT_KW_SIZE];
    uint32_t crc32;
    uint32_t pac_serial_num;
    uint32_t inver_pac_serial_num;
};

extern int32_t get_sfs_info(struct sfs *sfs, uint8_t *buffer, uint32_t len);
extern int32_t get_bpt_info(struct bpt *bpt, uint8_t *buffer, uint32_t len);
extern uint32_t sfs_crc32(uint32_t crc, uint8_t *buffer, uint32_t len);

#endif
