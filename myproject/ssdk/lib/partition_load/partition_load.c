/*
* SEMIDRIVE Copyright Statement
* Copyright (c) SEMIDRIVE. All rights reserved
*
* This software and all rights therein are owned by SEMIDRIVE, and are
* protected by copyright law and other relevant laws, regulations and
* protection. Without SEMIDRIVE's prior written consent and/or related rights,
* please do not use this software or any potion thereof in any form or by any
* means. You may not reproduce, modify or distribute this software except in
* compliance with the License. Unless required by applicable law or agreed to
* in writing, software distributed under the License is distributed on
* an AS IS basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
*
* You should have received a copy of the License along with this program.
* If not, see <http://www.semidrive.com/licenses/>.
*/

#include <debug.h>
#include <malloc.h>
#include <stdlib.h>
#include "libavb.h"
#include "partition_load.h"
#include "verified_boot.h"

#define BOOT_DEVICE_USER_DATA 0

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))
#endif

#define ERROR(format, args...) printf(\
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args);

#define INFO(format, args...) printf(\
                               "INFO:%s %d "format"\n", __func__, __LINE__,  ##args);

int partition_load(partition_device_t *ptdev, const char *part_name,
                   void *buf, uint64_t buf_size, uint64_t *loaded_size_p)
{
    int ret = -1;
    uint32_t bk_sz;
    AvbFooter footer;
    uint8_t *block_buf = NULL;
    struct disk_dev *disk = NULL;
    uint64_t ptn = ptdev_get_offset(ptdev, part_name);
    uint64_t part_size = ptdev_get_size(ptdev, part_name);

    ERROR("ptn %llx, size %llu\n", ptn, part_size);

    disk = ptdev->diskdev;

    if (!ptn || !part_size || !disk) {
        ERROR("partition %s no found.\n", part_name);
        goto out;
    }

    bk_sz = disk_get_block_size(disk);
    block_buf = memalign(bk_sz, bk_sz);

    if (block_buf
            && !disk_read(disk, ptn + part_size - bk_sz, block_buf, bk_sz)) {
        if (avb_footer_validate_and_byteswap((AvbFooter *)(block_buf + bk_sz - sizeof(
                AvbFooter)), &footer))
            part_size = footer.original_image_size;
    }

    if (buf_size < part_size) {
        ERROR("partition %s size is too large.\n", part_name);
        goto out;
    }

    ret = disk_read(disk, ptn, buf, ROUNDUP(part_size, bk_sz));

    if (ret) {
        ERROR("failed to load %s partition.\n", part_name);
        ret = -1;
        goto out;
    }

    if (loaded_size_p != NULL) {
        *loaded_size_p = ROUNDUP(part_size, bk_sz);
    }

out:

    if (block_buf)
        free(block_buf);

    return ret;
}

static const char *get_vbmeta_partition_info(partition_device_t *ptdev,
        uint64_t *ptn_out, uint64_t *pt_size_out)
{
    /* The top vbmeta partition name is "vbmete-top",
     * the vbmeta partition name in ap1 sub partition is "vbmeta",
     * the vbmeta partition name in ap2 sub partition is "vbmeta_ap2",
     * the vbmeta partition name in safety is "vbmeta_safety".
     * If don't support virtio emmc, the vbmeta partition name is "vbmeta".
     */
    const char *vbmeta_name[] = {"vbmeta", "vbmeta_top", "vbmeta_ap1", "vbmeta_ap2", "vbmeta_safety", "vbmeta_mpc", "vbmeta-ssdk"};
    uint64_t ptn, pt_size;

    if (!ptn_out || !pt_size_out) {
        return NULL;
    }

    for (uint32_t i = 0; i < ARRAY_SIZE(vbmeta_name); i++) {
        ptn      = ptdev_get_offset(ptdev, vbmeta_name[i]);
        pt_size  = ptdev_get_size(ptdev, vbmeta_name[i]);

        if (ptn != 0 && pt_size != 0) {
            *ptn_out = ptn;
            *pt_size_out = pt_size;
            return vbmeta_name[i];
        }
    }

    return NULL;
}

uint8_t *load_vbmeta_partition(partition_device_t *ptdev,
                               uint8_t *load_addr_in, size_t load_buf_sz_in,
                               size_t *load_sz_out)
{
    struct disk_dev *disk = NULL;
    const char *ptname            = NULL;
    uint64_t ptn        = 0;
    uint64_t pt_size    = 0;
    uint8_t *load_addr            = NULL;
    AvbFooter footer = {0};
    uint32_t bk_sz;

    disk = ptdev->diskdev;

    if (!ptdev || !disk) {
        ERROR("%s %d\n", __func__, __LINE__);
        goto out;
    }

    if (load_addr_in && !load_buf_sz_in) {
        ERROR("The size of the buffer to hold vbmeta is 0.\n");
        goto out;
    }

    bk_sz = disk_get_block_size(disk);
    ptname = get_vbmeta_partition_info(ptdev, &ptn, &pt_size);

    if (!ptname) {
        ERROR("vbmeta partition not found!.\n");
        goto out;
    }

    if (avb_get_footer_from_partition(ptdev, ptname, &footer)) {
        pt_size = footer.original_image_size;
    }

    load_addr = load_addr_in;

    if (!load_addr) {
        load_addr = memalign(bk_sz, ROUNDUP(pt_size, bk_sz));

        if (!load_addr) {
            ERROR("Fail to allocate memory!\n");
            goto out;
        }
    }
    else if (load_buf_sz_in < ROUNDUP(pt_size, bk_sz)) {
        ERROR("Buffer too small to load image!\n");
        load_addr = 0;
        goto out;
    }

    if (disk_read(disk, ptn, load_addr, ROUNDUP(pt_size, bk_sz))) {
        ERROR("load paritition error.name:%s \n", ptname);
        goto out;
    }

    if (load_sz_out)
        *load_sz_out = pt_size;

out:
    return load_addr;
}

