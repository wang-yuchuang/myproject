/**
 * @file update_monitor.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef UPDATE_MONITOR_H_
#define UPDATE_MONITOR_H_

#include <rpmsg_channel.h>
#include <lib/disk/disk.h>
#include <lib/disk/disk_mapped.h>
#include <lib/partition/partition_parser.h>
#include <rpc/rpmsg_common.h>

/* domain */

/* debug */
#define UPDATE_MONITOR_PRINT_MSG    0
#define UPDATE_MONITOR_DEBUG        SSDK_INFO
//#define UPDATE_MONITOR_DEBUG      SSDK_CRIT

#define  ERROR(format, ...)         {ssdk_printf(SSDK_CRIT,"[update monitor %d %s]:",  __LINE__,__FUNCTION__);           ssdk_printf(SSDK_CRIT,            format, ##__VA_ARGS__);}
#define  DBG(format, ...)           {ssdk_printf(UPDATE_MONITOR_DEBUG,"[update monitor %d %s]:",  __LINE__,__FUNCTION__);ssdk_printf(UPDATE_MONITOR_DEBUG, format, ##__VA_ARGS__);}


/* rpmsg define */
#define UPDATE_MONITOR                "update-monitor"
#define USE_IPCC                      1

/* update protocol */
#define OTA_CMD_UNDEFINED             0xFF
#define UPDATE_MONITOR_NAME_LENGTH    50
//#define MAX_IPCC_BUFF_SIZE            IPCC_MB_MTU
#define OTA_START_MAGIC               0x7E7E
#define OTA_END_MAGIC                 0x7B7B
#define MAX_DATA_LENGTH               448
#define MSG_HEAD_SIZE                 (sizeof(update_msg_head_t))
#define MAX_SEND_LENGTH               (MAX_DATA_LENGTH + MSG_HEAD_SIZE)
#define MAX_RECV_LENGTH               (MAX_DATA_LENGTH + MSG_HEAD_SIZE)

/* error code */
#define DEFAULT_MOD_NO                0

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef _ioaddr
#define _ioaddr(x)  (x)
#endif

#if !CONFIG_HAS_AP1 && !CONFIG_HAS_AP2
#define CONFIG_HAS_AP1 1
#endif

#if CONFIG_SAFETY
#if CONFIG_HAS_AP1
#define SHARED_MEMORY_BASE  (_ioaddr(SAF_AP1_MEMBASE + SHARED_MEMORY_BASE_OFFSET))
#elif CONFIG_HAS_AP2
#define SHARED_MEMORY_BASE  (_ioaddr(SAF_AP2_MEMBASE + SHARED_MEMORY_BASE_OFFSET))
#else
#undef USE_SHARED_MEMORY
#endif
#elif CONFIG_SECURE
#if CONFIG_HAS_AP1
#define SHARED_MEMORY_BASE  (_ioaddr(SEC_AP1_MEMBASE + SHARED_MEMORY_BASE_OFFSET))
#elif CONFIG_HAS_AP2
#define SHARED_MEMORY_BASE  (_ioaddr(SEC_AP2_MEMBASE + SHARED_MEMORY_BASE_OFFSET))
#else
#undef USE_SHARED_MEMORY
#endif
#else
#error ssdk update monitor could not build - need CONFIG_SAFETY or CONFIG_SECURE
#endif //END of CONFIG_SAFETY/CONFIG_SECURE

#define SHARED_MEMORY_READ_DATA_OFFSET  0x0
#define SHARED_MEMORY_WRITE_DATA_OFFSET (SHARED_MEMORY_READ_DATA_OFFSET + MAX_SHARED_MEMORY_READ_DATA_LENGTH)
#define SHARED_MEMORY_READ_CRC_OFFSET   (SHARED_MEMORY_WRITE_DATA_OFFSET + MAX_SHARED_MEMORY_WRITE_DATA_LENGTH)
#define SHARED_MEMORY_WRITE_CRC_OFFSET  (SHARED_MEMORY_READ_CRC_OFFSET + MAX_SHARED_MEMORY_CRC_LENGTH)
#define GET_ALIGN_ADDR(addr, erase_size) (((addr) / (erase_size)) * (erase_size))
#define GET_REL_ADDR(addr, sect_size)    ((addr) % (sect_size))

#define UPDATE_MONITOR_PTDEV_NAME_LEN 72
#define ALIGNED_BUFFER_SIZE 512
#define  DO_NOT_UPDATE  false
#define  DO_UPDATE      true
#define ONE_BOOT_INFO_LEN                128
#define BOOT_INFO_PREFIX                 "BOOT_MODE="
#define LEVEL2_PTDEV_PREFIX              "-SUB_PTDEV="
#define CAPACITY_DON_CARE_VALUE          1048576
#define SECTOR_DON_CARE_VALUE            4096
#define BLOCK_DON_CARE_VALUE             512
#define SCAN_FOR_LEVEL2_ERROR_DATA       OTA_CMD_MAX + 1
#define REGISTER_BOOT_INFO_ERROR_DATA    OTA_CMD_MAX + 2
#define UPDATE_MONITOR_MAPPED_RES_INDEX(x)  (0xFFFFFFFF - x)

typedef enum update_monitor_mode {
    MODE_SKIP_UPDATE,
    MODE_NEED_UPDATE,
}update_monitor_mode_e;

typedef enum update_cmd {
    OTA_CMD_START,
    OTA_CMD_START_OK,
    OTA_CMD_START_FAIL,
    OTA_CMD_SEEK,
    OTA_CMD_SEEK_OK,
    OTA_CMD_SEEK_FAIL,
    OTA_CMD_READ,
    OTA_CMD_READ_OK,
    OTA_CMD_READ_FAIL,
    OTA_CMD_WRITE,
    OTA_CMD_WRITE_OK,
    OTA_CMD_WRITE_FAIL,
    OTA_CMD_WRITE_INFO,
    OTA_CMD_WRITE_INFO_OK,
    OTA_CMD_WRITE_INFO_FAIL,
    OTA_CMD_GET_CAPACITY,
    OTA_CMD_GET_CAPACITY_OK,
    OTA_CMD_GET_CAPACITY_FAIL,
    OTA_CMD_GET_BLOCK,
    OTA_CMD_GET_BLOCK_OK,
    OTA_CMD_GET_BLOCK_FAIL,
    OTA_CMD_GET_ERASESIZE,
    OTA_CMD_GET_ERASESIZE_OK,
    OTA_CMD_GET_ERASESIZE_FAIL,
    OTA_CMD_COPY,
    OTA_CMD_COPY_OK,
    OTA_CMD_COPY_FAIL,
    OTA_CMD_HANDOVER,
    OTA_CMD_HANDOVER_OK,
    OTA_CMD_HANDOVER_FAIL,
    OTA_CMD_CLOSE,
    OTA_CMD_CLOSE_OK,
    OTA_CMD_CLOSE_FAIL,
    OTA_CMD_PRIORITY,
    OTA_CMD_PRIORITY_OK,
    OTA_CMD_PRIORITY_FAIL,
    OTA_CMD_STOP_WDT,
    OTA_CMD_STOP_WDT_OK,
    OTA_CMD_STOP_WDT_FAIL,
    OTA_CMD_SETUP_PTDEV,
    OTA_CMD_SETUP_PTDEV_OK,
    OTA_CMD_SETUP_PTDEV_FAIL,
    OTA_CMD_DESTROY_PTDEV,
    OTA_CMD_DESTROY_PTDEV_OK,
    OTA_CMD_DESTROY_PTDEV_FAIL,
    OTA_CMD_GET_PARTITION_INDEX,
    OTA_CMD_GET_PARTITION_INDEX_OK,
    OTA_CMD_GET_PARTITION_INDEX_FAIL,
    OTA_CMD_GET_PARTITION_SIZE,
    OTA_CMD_GET_PARTITION_SIZE_OK,
    OTA_CMD_GET_PARTITION_SIZE_FAIL,
    OTA_CMD_GET_PARTITION_OFFSET,
    OTA_CMD_GET_PARTITION_OFFSET_OK,
    OTA_CMD_GET_PARTITION_OFFSET_FAIL,
    OTA_CMD_GET_NUMBER_SLOTS,
    OTA_CMD_GET_NUMBER_SLOTS_OK,
    OTA_CMD_GET_NUMBER_SLOTS_FAIL,
    OTA_CMD_GET_CURRENT_SLOTS,
    OTA_CMD_GET_CURRENT_SLOTS_OK,
    OTA_CMD_GET_CURRENT_SLOTS_FAIL,
    OTA_CMD_GET_SLOT_ATTR,
    OTA_CMD_GET_SLOT_ATTR_OK,
    OTA_CMD_GET_SLOT_ATTR_FAIL,
    OTA_CMD_SET_SLOT_ATTR,
    OTA_CMD_SET_SLOT_ATTR_OK,
    OTA_CMD_SET_SLOT_ATTR_FAIL,
    OTA_CMD_UPDATE_SLOT_ATTR,
    OTA_CMD_UPDATE_SLOT_ATTR_OK,
    OTA_CMD_UPDATE_SLOT_ATTR_FAIL,
    OTA_CMD_GET_NUMBER_PARTITION,
    OTA_CMD_GET_NUMBER_PARTITION_OK,
    OTA_CMD_GET_NUMBER_PARTITION_FAIL,
    OTA_CMD_GET_PARTITION_INFO,
    OTA_CMD_GET_PARTITION_INFO_OK,
    OTA_CMD_GET_PARTITION_INFO_FAIL,
    OTA_CMD_GET_BOOT_INFO,
    OTA_CMD_GET_BOOT_INFO_OK,
    OTA_CMD_GET_BOOT_INFO_FAIL,
    OTA_CMD_SEEK_MAPPED,
    OTA_CMD_SEEK_MAPPED_OK,
    OTA_CMD_SEEK_MAPPED_FAIL,
    OTA_CMD_GET_CAPACITY_MAPPED,
    OTA_CMD_GET_CAPACITY_MAPPED_OK,
    OTA_CMD_GET_CAPACITY_MAPPED_FAIL,
    OTA_CMD_COPY_MAPPED,
    OTA_CMD_COPY_MAPPED_OK,
    OTA_CMD_COPY_MAPPED_FAIL,
    OTA_CMD_SETUP_MAPPED_STORAGE,
    OTA_CMD_SETUP_MAPPED_STORAGE_OK,
    OTA_CMD_SETUP_MAPPED_STORAGE_FAIL,
    OTA_CMD_MAX,
} update_cmd_enum;

typedef enum update_monitor_state {
    STATE_UN_INIT,
    STATE_INIT,
} update_state_e;

struct __attribute__((packed)) update_msg_head {
    uint16_t flag1;
    uint16_t len;
    uint16_t cmd;
    uint32_t resv;
    uint32_t crc;
    uint16_t flag2;
};

typedef struct update_msg_head update_msg_head_t;

typedef struct msg_reply {
    update_cmd_enum reply_cmd;
    uint16_t     reply_data_len;
} msg_reply_t;

typedef struct ptdev_index {
    char ptdev_name[UPDATE_MONITOR_PTDEV_NAME_LEN];
    partition_device_t *ptdev;
    bool is_sub_ptdev;
    char upper_level_ptdev_name[UPDATE_MONITOR_PTDEV_NAME_LEN];
    char upper_level_partition_name[MAX_GPT_NAME_SIZE];
    struct disk_dev* mapped_storage;
    struct disk_mapped_info* mapped_info;
    uint32_t sub_partition_ptn;
    uint64_t sub_partition_pt_size;
} ptdev_index_t;

/* update_monitor */
typedef struct update_monitor_struct {
    char                    name[UPDATE_MONITOR_NAME_LENGTH];
    struct rpmsg_instance  *rpmsg_dev;
    rpmsg_channel_t         rpmsg_chan;
    osThreadId_t            msg_process_thread;
    osEventFlagsId_t        new_msg_event;
    struct disk_dev         *storage;
    const char*             disk_name;
    uint64_t                seek_addr;
    uint64_t                write_len_total;
    uint8_t                *write_buf;
    uint8_t                *copy_buf;
    uint8_t                 write_buf_flag;
    uint64_t                capacity;
    uint32_t                erase_size;
    uint32_t                block_size;
    update_state_e          state;
    update_msg_head_t      *recv_head;
    update_msg_head_t      *send_head;
    uint8_t                 recv_msg[MAX_RECV_LENGTH];
    uint8_t                 send_msg[MAX_RECV_LENGTH];
    update_monitor_mode_e   mode;
    uint8_t                 load_storage;
    char                    boot_info[MAX_DATA_LENGTH];
    msg_reply_t             reply;
} update_monitor_dev_t;

typedef struct update_monitor_cfg {
    uint8_t     rpmsg_type;
    uint8_t     local_proc;
    uint8_t     remote_proc;
    uint8_t     disk_type;
} update_monitor_cfg_t;

typedef struct update_msg_cmd {
    update_cmd_enum cmd;
    int (*func)(update_monitor_dev_t *dev);
    const char *cmd_name;
    bool is_set_errcode;
} update_msg_cmd_t;

extern int update_monitor_start(update_monitor_cfg_t *cfg);

#endif /* UPDATE_MONITOR_H_ */


