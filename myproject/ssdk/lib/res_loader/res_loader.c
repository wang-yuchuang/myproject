#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <ff.h>
#include <board.h>
#include <arch/cache.h>

#include "res_loader.h"
#include <lib/disk/disk.h>
#include <lib/partition/partition_parser.h>
#include <lib/partition/ab_partition_parser.h>
#if CONFIG_VERIFIED_BOOT
#include "partition_parser.h"
#include "verified_boot.h"
#endif

#define FPATH_LEN   128
#define RES_IMAGE_NAME "res"

static  FATFS fsd;

#define DISKD       CONFIG_DISK_NAME ":"

/* eg: emmc0:1: emmc0:2: */
static char diskd_active[16] = {0};
static const char GUID_MS_Basic[PARTITION_TYPE_GUID_SIZE] = {0xA2,0xA0,0xD0,0xEB,0xE5,0xB9,0x33,0x44,0x87,0xC0,0x68,0xB6,0xB7,0x26,0x99,0xC7};

#define ERROR(fmt, args...) ssdk_printf(SSDK_CRIT, "ERROR:"fmt, ##args)
#define INFO(fmt, args...) ssdk_printf(SSDK_INFO, fmt, ##args)

static int verified_res_image(void)
{
#if CONFIG_VERIFIED_BOOT
    int ret = -1;
    uint64_t res_offset, res_size;
    uint8_t *res_buf = NULL;
    uint8_t *vbmeta_buf = NULL;
    uint32_t vbmeta_sz = 8 * 1024;
    struct list_node verified_images_list;
    partition_device_t  *ptdev = NULL;
    struct disk_dev disk = {0};

    list_initialize(&verified_images_list);

    ret = disk_open(DISK_MMC_NAME(0), &disk);

    if ( ret != -DISK_ERROR_BUSY && ret != 0 ) {
        ERROR("setup storage failed while %s\n", __func__);
        return -1;
    }

    ptdev = ptdev_setup(&disk, 0);

    if (!ptdev) {
        ERROR("Fail to setup partition device!\n");
        goto out;
    }

    ptdev_read_table(ptdev);

    res_offset = ptdev_get_offset(ptdev, RES_IMAGE_NAME);
    res_size = ptdev_get_size(ptdev, RES_IMAGE_NAME);

    if (!res_size) {
        ERROR("%s %d cann't get res partition size!\n", __func__, __LINE__);
        goto out;
    }

    res_buf = memalign(disk_get_block_size(&disk), res_size);

    if (!res_buf) {
        ERROR("%s %d fail to allocate memory!\n", __func__, __LINE__);
        goto out;
    }

    if (disk_read(&disk, res_offset, res_buf, res_size)) {
        ERROR("%s %d fail to read disk\n", __func__, __LINE__);
        goto out;
    }

do_verify:
    ret = add_verified_image_list(&verified_images_list,
                                  res_buf, res_size, RES_IMAGE_NAME);
    ASSERT(ret);
#if 0
    ret = add_verified_image_list(&verified_images_list,
                                  vbmeta_buf, vbmeta_sz, VBMETA_PARTITION_NAME);
    ASSERT(ret);
#endif

    if (!verify_loaded_images(ptdev, &verified_images_list, NULL)) {
        ERROR("%s %d verify images fail\n", __func__, __LINE__);
        goto out;
    }

    ret = 0;
out:
    free_image_info_list(&verified_images_list);

    if (res_buf)
        free(res_buf);

    disk_close(&disk);
    return ret;
#endif
    return 0;
}

#define CL_BLOCK_SIZE 512
static int get_parition_slot(void)
{
    int ret = -1;
    int slot = 0;
    uint32_t partition_count = 0;
    struct partition_device *ptdev = NULL;
    struct disk_dev flash_dev = {0};

    const struct partition_entry *partition_entries = NULL;
    const char *suffix_slot[] = {"_a", "_b"};
    int fat_partition_num = 0;

    ret = disk_open(CONFIG_DISK_NAME, &flash_dev);
    if ( ret != -DISK_ERROR_BUSY && ret != 0) {
        ssdk_printf(SSDK_EMERG, "setup storage failed while %s\n", __func__);
        return ret;
    }
    disk_set_block_size(&flash_dev, CL_BLOCK_SIZE);

    ptdev = ptdev_setup(&flash_dev, 0);
    if(!ptdev) {
        ssdk_printf(SSDK_EMERG, "%s %d ptdev_setup error\n", __func__, __LINE__);
        goto end;
    }

    if (ptdev_read_table(ptdev)) {
        ssdk_printf(SSDK_EMERG, "%s %d read partition error!\n", __func__, __LINE__);
        goto end;
    }

    slot = ptdev_find_active_slot(ptdev);
    ssdk_printf(SSDK_CRIT, "%s: ptdev_find_active_slot slot=%d, SLOT_A=%d, SLOT_B=%d\n", __func__, slot, SLOT_A, SLOT_B);

    if((SLOT_A == slot) || (SLOT_B == slot)) {
        ssdk_printf(SSDK_EMERG, "run in slot%s\n",suffix_slot[slot]);
    }
    else {
        ssdk_printf(SSDK_EMERG, "can not find active slot");
        ret = -1;
        goto end;
    }

    partition_entries = ptdev->partition_entries;
    partition_count = ptdev->count;

    for (int i = 0; i < partition_count; i++) {
        if (!memcmp(partition_entries[i].type_guid, GUID_MS_Basic, PARTITION_TYPE_GUID_SIZE)) {
            fat_partition_num++;
            if(((SLOT_A == slot) && (!strcmp((const char *)partition_entries[i].name,"res_a")
                || !strcmp((const char *)partition_entries[i].name,"res")))
                || ((SLOT_B == slot) && !strcmp((const char *)partition_entries[i].name,"res_b"))){
                ret = fat_partition_num;
                break;
            }
            else {
                ret = 1;
            }
        }
    }

end:
    if(ptdev)
        ptdev_destroy(ptdev);

    disk_close(&flash_dev);

    return ret;
}

const char* get_diskd_active(void)
{
    return diskd_active;
}

int path_joint(char *fpath, const char *path)
{
    if (strlen(path) >= FPATH_LEN - 2) {
        return -1;
    }

    fpath[sprintf(fpath, "%s/%s", diskd_active, path)] = 0;

    return 0;
}

int res_fs_init(void)
{
    FRESULT r;
    uint64_t start;
    int boot_slot = 0;
    char slot[4] = {0};

    start = current_time_hires();

    if (verified_res_image()) {
        ERROR("fail to verify res image!\n");
        return -1;
    }

    start = current_time_hires() - start;
    ssdk_printf(SSDK_CRIT, "verified time:%llu\n", start);

    boot_slot = get_parition_slot();
    if (boot_slot <= 0) {
        ssdk_printf(SSDK_CRIT, "%s: get_parition_slot failed, use the default slot:res_a.\n", __func__);
        boot_slot = 1;
    }
    strcpy(diskd_active, DISKD);
    sprintf(slot, "%d:", boot_slot);
    strcat(diskd_active, slot);
    ssdk_printf(SSDK_CRIT, "%s: get_parition_slot boot_slot=%d, diskd_active=%s\n", __func__, boot_slot, diskd_active);

    r = f_mount(&fsd, diskd_active, 1);

    if (r) {
        ERROR("FS init fail. ret code:%d\n", r);
    }
    else {
        ssdk_printf(SSDK_CRIT, "FS init succ.\n");
    }

    return r;
}

int res_size(const char *path)
{
    FIL f;
    int fsz = 0;

    char *fpath = malloc(FPATH_LEN);

    if (!fpath) {
        ERROR("fpath Out of memory\n");
        return -1;
    }

    if (path_joint(fpath, path)) {
        ERROR("fpath is too long\n");
        free(fpath);
        return -1;
    }

    FRESULT rc = f_open(&f, fpath, FA_READ);

    if (rc) {
        ERROR("f_open fail %d\n", rc);
        free(fpath);
        return -1;
    }

    fsz = f_size(&f);

    RLDBG("res_size: %s -> 0x%x\n", fpath, fsz);

    free(fpath);

    f_close(&f);

    return fsz;
}

int res_load(const char *path, void *buf, uint32_t size, uint32_t offset)
{

    if (!buf) {
        ERROR("invalid buf para\n");
        return -1;
    }

    if ((addr_t)buf % 32 || size % 32) {
        ERROR("read buf & size must be 32bytes aligned.\n");
        return -1;
    }

    char *fpath = malloc(FPATH_LEN);

    if (!fpath) {
        ERROR("fpath Out of memory\n");
        return -1;
    }

    if (path_joint(fpath, path)) {
        ERROR("fpath is too long\n");
        free(fpath);
        return -1;
    }

    UINT br = -1;
    FIL f;
    FRESULT rc = f_open(&f, fpath, FA_READ);

    if (rc) {
        ERROR("open %s fail %d\n", fpath, rc);
        free(fpath);
        return -1;
    }

    if (offset) {
        rc = f_lseek(&f, offset);

        if (rc) {
            ERROR("seek %s : 0x%x fail %d\n", fpath, offset, rc);
            goto F_CLOSE;
        }
    }

    rc = f_read(&f, buf, size, &br);

    if (rc) {
        ERROR("read %s : 0x%x fail %d\n", fpath, offset, rc);
        br = -1;
        goto F_CLOSE;
    }

    arch_clean_cache_range((addr_t)buf, size);

    RLDBG("res_load: %s -> %p Wanted 0x%x Bytes @ 0x%x offset br 0x%x\n", fpath,
          buf, size, offset, br);

F_CLOSE:
    free(fpath);
    f_close(&f);
    return (int)br;
}

