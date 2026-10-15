#ifndef _OTA_CFG_H_
#define _OTA_CFG_H_

#define DL_DEBUG_ON 1
#if DL_DEBUG_ON
#define ERROR(format, args...) ssdk_printf(SSDK_CRIT, \
                               "[ERROR]OTA:%s %d "format"", __func__, __LINE__,  ##args);
#define DBG(format, args...) ssdk_printf(SSDK_CRIT, \
                               "[DEBUG]OTA:%s %d "format"", __func__, __LINE__,  ##args);
#else
#define ERROR(format, args...)
#define DBG(format, args...)
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define READ_SIZE 512
#define LOGIC_BLOCK_SIZE 512
#define MAX_DEV_INST_OF_DISK 3
#define MAX_GPT_NAME_SIZE 72
#define OSPI1_STORAGE_NAME    "flash0"
#define OSPI2_STORAGE_NAME    "flash1"
#define EMMC1_STORAGE_NAME    "emmc0"
#define EMMC2_STORAGE_NAME    "emmc1"
#define SD1_STORAGE_NAME      "sd0"
#define SD2_STORAGE_NAME      "sd1"
#define SD_BOOT_OFFSET         (0x5000)
#define OTA_ARGV_NUM           (10)
#define OTA_ARGV_LEN           (50)
#define OTA_STACK_SIZE         (16384)
#define NORFLASH_GPT_OFFSET    (0x2000)
#define PTB_SECTOR_INDEX       (2)

#define SP0_MEM_BASE 0x700000
#define SP1_MEM_BASE 0x780000
#define SX0_MEM_BASE 0x600000
#define SX1_MEM_BASE 0x680000
#define CORE_SIZE 524288

#define BOOT_INFO_ADDR           0x191FF80
#define BOOT_DEVICE_INFO_OFFSET  0x08
#define BOOT_PACKAGE_INFO_OFFSET 0x09
#define BOOT_INFO_TAG            0xE7002001
#define BOOT_DEVICE_EMMC_OR_SD1  0x01
#define BOOT_DEVICE_EMMC_OR_SD2  0x02
#define BOOT_DEVICE_NOR_FLASH    0x10
#define BOOT_DEVICE_USB          0x30
#define BOOT_DEVICE_UART         0x34
#define BOOT_NORMAL_PACKAGE      0x00
#define BOOT_BACKUP_PACKAGE      0x01
#define BOOT_THIRD_PACKAGE       0x02


#define MAX_BPT_NUM 3
#define MAX_FILE_NAME_LIMIT 20
#define INVALID_ADDR ~(0llu)
#define INVALID_VERSION ~(0u)
#define INVALID_BPT_NUMBER 0xFF
#define UPDATE_ONE_TIME 0

/*
struct iib_info {
    uint8_t image_type;
    uint8_t target_core;
    uint32_t dev_logic_page;
    uint32_t image_size;
    uint32_t load_base;
    uint32_t entry_point;
};
*/

struct bpt_info {
    uint8_t valid;
    uint8_t xip;
    uint8_t desk_inst;
    uint64_t bpt_addr;
    uint64_t max_size;
    uint8_t image_num;
    uint32_t sec_version;
    uint32_t crc32;
    uint32_t pac_serial_num;
    uint32_t inver_pac_serial_num;
/*
    struct iib iib[8];
*/
};

/* record downloaded state */
typedef struct dl_state {
    const char               download_name[MAX_GPT_NAME_SIZE + 1];
    const char               disk_name[MAX_GPT_NAME_SIZE + 1];
    const char               disk_boot1_name[MAX_GPT_NAME_SIZE + 1];
    const char               disk_boot2_name[MAX_GPT_NAME_SIZE + 1];
    struct disk_dev         *disk_inst[MAX_DEV_INST_OF_DISK];
    uint8_t                  disk_num;
    struct partition_device *ptdev;
    uint64_t                 ptb_offset;
    uint32_t                 block_size;
    uint32_t                 erase_size;
    uint64_t                 capacity;
    struct bpt_info          bpt_info[MAX_BPT_NUM];
} DISK_STATE_T;

extern uint8_t check_bpt_for_update(DISK_STATE_T *ds, struct bpt *bpt,
                                    uint8_t max_bpt_num, uint8_t min_bpt_num);
extern uint8_t sort_bpt_for_disk(DISK_STATE_T *ds, uint8_t *max_bpt_num,
                                 uint8_t *min_bpt_num);
extern uint8_t md5_read_check(DISK_STATE_T *ds, uint8_t inst,
                              uint64_t read_addr, uint32_t size);
#endif //_SYSTEM_CFG_H_