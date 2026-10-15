/*
 * dloader.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _DLOADER_H_
#define _DLOADER_H_

#include <lib/disk/disk.h>


#define DL_CMD_TB_LEN 17
#define DL_TYPE_TB_LEN 8

#define MAX_DEV_INST_OF_DISK 3
#define MAX_DISK_NAME_SIZE 16

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef MD5_LEN
#define MD5_LEN 16
#endif

#ifndef MAX_GPT_NAME_SIZE
#define MAX_GPT_NAME_SIZE 72
#endif

#define DL_DEBUG_ON 1
#if DL_DEBUG_ON
#define ERROR(format, args...) ssdk_printf(SSDK_CRIT, \
                               "[ERROR]DLOADER:%s %d "format"", __func__, __LINE__,  ##args);
#define DBG(format, args...) ssdk_printf(SSDK_CRIT, \
                               "[DEBUG]DLOADER:%s %d "format"", __func__, __LINE__,  ##args);
#else
#define ERROR(format, args...)
#define DBG(format, args...)
#endif

#define SPARSE_HEADER_MAGIC 0xed26ff3a
#define CHUNK_TYPE_RAW      0xCAC1
#define CHUNK_TYPE_FILL     0xCAC2
#define CHUNK_TYPE_DONT_CARE    0xCAC3
#define CHUNK_TYPE_CRC      0xCAC4

typedef enum dl_err_code {
    ERR_NONE    =   0,
    ERR_UNKNOWN = 0x1,
    ERR_PRI_PTB_NOT_MATCH,
    ERR_SUB_PTB_NOT_MATCH,
    ERR_PT_NOT_FOUND,
    ERR_IMAGE_TOO_LARGE,
    ERR_IMAGE_FORMAT_ERR,
    ERR_PRI_PTB_NOT_FLASH,
    ERR_SUB_PTB_NOT_FLASH,
    ERR_PT_READ_FAIL,
    ERR_PT_FLASH_FAIL,
    ERR_PT_ERASE_FAIL,
    ERR_PT_OVERLAP,
    ERR_PT_FULL_NAME_FORMAT,
    ERR_INVALID_BLOCK_SIZE,
    ERR_SPARSE_IMAGE_SIZE_TOO_LOW,
    ERR_SPARSE_IMAGE_HEADER,
    ERR_SPARSE_IMAGE_BUFFERED,
    ERR_SPARSE_IMAGE_CHUNK_HEADER,
    ERR_SPARSE_IMAGE_CHUNK_TOO_LARGE,
    ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH,
    ERR_SPARSE_IMAGE_CHUNK_UNKNOWN,
    ERR_SPARSE_IMAGE_MALLOC,
    ERR_SPARSE_IMAGE_DO_NOT_SUPPORT,
    ERR_HASH_FAIL,
    ERR_EFUSE_INDEX,
    ERR_EFUSE_BURN,
    ERR_EFUSE_READ,
    ERR_SWITCH_PART,
    ERR_CMD_ERROR,
    ERR_PT_BASE_ERROR,
    ERR_PT_SIZE_ERROR,
    ERR_PTNAME_NOT_EXIT,
    ERR_DISK_NOT_EXIT,
    ERR_DISK_OPEN_ERROR,
    ERR_PARTITION_READ_ERROR,
    HYPER_FLASH_FUSE_READ_ERROR,
    HYPER_FLASH_FUSE_WRITE_ERROR,
    HYPER_FLASH_FUSE_VAL_ERROR,
    ERR_MAX,
} DL_ERR_CODE_E;

typedef enum partition_type_enum {
    TYPE_PT_UNKNOWN = 0,
    TYPE_PRI_PTB,
    TYPE_PRI_PT,
    TYPE_SUB_PTB,
    TYPE_SUB_PT,
    TYPE_SUB_PT_WHOLE,
    TYPE_ALL_PT,/* only for erase all partitions in the gpt */
    TYPE_NOT_IN_GPT = 0x100,
    TYPE_BOOT_PACK0,
    TYPE_BOOT_PACK1,
    TYPE_BOOT_PACK2,
    TYPE_SAFETY_SFS_PT,
    TYPE_SAFETY_RFD_PT,
    TYPE_ADD_DIRECT,
} PARTITION_TYPE_E;

typedef enum dev_inst_enum {
    FLASH_DEV_INST = 0,
    EMMC_USER_SPACE_DEV_INST = 0,
    EMMC_BOOT1_DEV_INST = 1,
    EMMC_BOOT2_DEV_INST = 2,
    EMMC_DEV_INST_MAX,
} DEV_INST_E;

typedef enum disk_type_enum {
    MMC,
    NORFLASH,
    RAMDISK,
    USBDISK,
    ALLDISK,
    STORAGE_TYPE_NM,
} DISK_TYPE_E;

struct ptb_dl_name {
    struct list_node node;
    char *name;
};

/* record downloaded state */
typedef struct dl_state {
    const char               download_name[MAX_GPT_NAME_SIZE + 1];
    const char               disk_name[MAX_GPT_NAME_SIZE + 1];
    const char               disk_boot1_name[MAX_GPT_NAME_SIZE + 1];
    const char               disk_boot2_name[MAX_GPT_NAME_SIZE + 1];
    DISK_TYPE_E              disk_type;
    struct disk_dev         *disk_inst[MAX_DEV_INST_OF_DISK];
    uint32_t                 block_size;
    uint32_t                 erase_size;
    uint64_t                 capacity;
    struct partition_device  *ptdev;
    uint64_t                 ptb_offset;
    PARTITION_TYPE_E         partiton_type;
    char                     ptname[MAX_GPT_NAME_SIZE + 1];
    char                     sub_ptbname[MAX_GPT_NAME_SIZE + 1];
    const uint64_t           boot_offset;
} DL_STATE_T;

typedef struct command_table {
    PARTITION_TYPE_E partiton_type;
    DISK_TYPE_E disk_type;
    DL_ERR_CODE_E (*flash_callback)(DL_STATE_T *ds, void *data, unsigned sz,
                                    uint8_t sparse_data);
    DL_ERR_CODE_E (*erase_cmd_callback)(DL_STATE_T *ds);
    char *download_cmd_name;
    char *erase_cmd_name;
} COMMAND_TABLE_T;

typedef struct download_type_table {
    char  *ptname;
    uint8_t is_sub_pt;
    char  *type_name;
    PARTITION_TYPE_E partiton_type;
} DOWNLOAD_TYPE_TABLE_T;

typedef struct sparse_header {
    uint32_t  magic;      /* 0xed26ff3a */
    uint16_t  major_version;  /* (0x1) - reject images with higher major versions */
    uint16_t  minor_version;  /* (0x0) - allow images with higer minor versions */
    uint16_t  file_hdr_sz;    /* 28 bytes for first revision of the file format */
    uint16_t  chunk_hdr_sz;   /* 12 bytes for first revision of the file format */
    uint32_t  blk_sz;     /* block size in bytes, must be a multiple of 4 (4096) */
    uint32_t  total_blks; /* total blocks in the non-sparse output image */
    uint32_t  total_chunks;   /* total chunks in the sparse input image */
    uint32_t  image_checksum; /* CRC32 checksum of the original data, counting "don't care" */
    /* as 0. Standard 802.3 polynomial, use a Public Domain */
    /* table implementation */
} sparse_header_t;


typedef struct chunk_header {
    uint16_t  chunk_type; /* 0xCAC1 -> raw; 0xCAC2 -> fill; 0xCAC3 -> don't care */
    uint16_t  reserved1;
    uint32_t  chunk_sz;   /* in blocks in output image */
    uint32_t  total_sz;   /* in bytes of chunk input file including chunk header and data */
} chunk_header_t;

extern void *dl_scratch_base;
extern uint32_t dl_scratch_sz;
extern void *dl_data_base;
extern uint32_t dl_data_sz;
extern DL_STATE_T *parse_partition_name(const char *full_ptname, DL_ERR_CODE_E *err);
extern int dloader_init(void);
extern void read_sfs(DL_STATE_T *ds);
extern uint8_t md5_received[MD5_LEN];
extern DL_ERR_CODE_E dl_cmd_flash(const char *arg, void *data, unsigned sz);
extern DL_ERR_CODE_E dl_cmd_erase(const char *arg, void *data, unsigned sz);
int dl_disk_read(struct disk_dev *dev, disk_addr_t addr, uint8_t *dst,
                 disk_size_t size);
int dl_disk_write(struct disk_dev *dev, disk_addr_t addr, uint8_t *src,
                  disk_size_t size);
int dl_disk_erase(struct disk_dev *dev, disk_addr_t addr, disk_size_t size);

#endif
