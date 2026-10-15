/**
 * @file reboot.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <types.h>
#include <debug.h>
#include <reg.h>
#include <regs_base.h>
#include <irq_num.h>
#include <driver/scr.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <driver/mbox.h>

#include <arch/cache.h>

#include "kunlun/reset/sdrv_rstgen.h"
#include "mem_image.h"
#include "peer_load.h"
#include "kunlun/reset/sdrv_rstgen.h"
#include "reboot.h"
#include "mbox_cfg.h"
#include "scr_cfg.h"
#include "sd_errcode.h"
#ifdef CONFIG_BOARD_D9350_REF
#include "lib/sd_boot_pin/sd_boot_pin.h"
#endif

#if CONFIG_VERIFIED_BOOT
#include "image_cfg.h"
#include "verified_boot.h"
#endif

#ifndef NO_ERROR
#define NO_ERROR 0
#endif

#ifndef ERR_NOT_VALID
#define ERR_NOT_VALID           (-7)
#endif

# define v2p(va)        (paddr_t)(va)
# define p2v(pa)        (vaddr_t)(pa)

#define _vaddr(pa)      p2v(pa)
#define _paddr(va)      v2p(va)

#define _ioaddr(pa)     _vaddr(pa)

#define DEFAULT_MOD_NO  0

#define SDRV_REG_STATUS 6

/* The firewall early config done */
#define SDRV_REG_STATUS_FIREWALL_EARLY_DONE (1<<2)
/* The sdpe load done */
#define SDRV_REG_STATUS_SDPE_LD_DONE (1<<4)

#define SDRV_GENERAL_REG(n) (APB_RSTGEN_SEC_BASE + SOC_RSTGEN_REG_MAP(RSTGEN_GENERAL_RST_OFF + n * 4))

static uint32_t send_peer_load_msg(uint32_t para, mbox_dev_t *mbox)
{
    struct peer_boot_message msg_ori = MK_PEER_LOAD_MSG(para);
    uint32_t len = sizeof(struct peer_boot_message);
    uint8_t *msg = (uint8_t *)&msg_ori;
    mbox_chan_t *reboot_chan;
    sdrv_mbox_chan_req_t chan_req;

    chan_req.rproc = 1;

    reboot_chan = mbox_request_channel(mbox, &chan_req);

    if (!reboot_chan) {
        ssdk_printf(SSDK_ERR, "mbox_test request send channel fail\n");
        return RESET_ERR_BAD_HANDLE;
    }

    mbox_send_nobuf(reboot_chan, msg, len, 1000);

    mbox_release_channel(reboot_chan);

    return NO_ERROR;
}

static inline uint32_t sdrv_general_reg_get(uint32_t reg, uint32_t mask)
{
    return readl(SDRV_GENERAL_REG(reg)) & mask;
}

static inline void sdrv_general_reg_set(uint32_t reg, uint32_t value,
                                        uint32_t mask)
{
    uint32_t tmp_value;
    tmp_value = readl(SDRV_GENERAL_REG(reg)) & ~mask;
    tmp_value |= value & mask;
    writel(tmp_value, SDRV_GENERAL_REG(reg));
}

static uint32_t verified_image(void *image_addr, uint64_t image_size,
                               const char *pt_name)
{
#if CONFIG_VERIFIED_BOOT

    uint32_t ret;
    uint8_t *vbmeta_buf = (uint8_t *)_ioaddr(VBMETA_MEMBASE);
    uint32_t vbmeta_sz = VBMETA_MEMSIZE;
    struct list_node verified_images_list;
    list_initialize(&verified_images_list);

    ret = add_verified_image_list(&verified_images_list,
                                  image_addr, image_size, pt_name);
    ASSERT(ret);

#ifdef CONFIG_BOARD_D9350_REF
    if (boot_get_pin() == BOOT_PIN_0 && !strcmp(pt_name,"cluster_preloader")) {
            vbmeta_buf = (uint8_t *)_ioaddr(VBMETA_MEMBASE + VBMETA_MEMSIZE/2);
            vbmeta_sz = VBMETA_MEMSIZE/2;
    }
#endif

    ret = add_verified_image_list(&verified_images_list,
                                  vbmeta_buf, vbmeta_sz, VBMETA_PARTITION_NAME);
    ASSERT(ret);

    if (!verify_loaded_images(NULL, &verified_images_list, NULL)) {
        ssdk_printf(SSDK_ERR, "%s %d verify images fail\n", __func__, __LINE__);
        free_image_info_list(&verified_images_list);
        return ERR_NOT_VALID;
    }

    free_image_info_list(&verified_images_list);
#endif
    return NO_ERROR;
}

void reboot_ctrl(struct reboot_cfg *reboot_array)
{
    uint32_t ret;
    mem_image_entry_t info;

    ASSERT(reboot_array);

    if (reboot_array->id >= CPU_ID_MAX || reboot_array->id <= CPU_ID_MIN) {
        ssdk_printf(SSDK_ERR, "%s cpu id:%d error\n", __func__, reboot_array->id );
        ERRCODE_PRINT(MOD_ID_RESET, DEFAULT_MOD_NO, RESET_ERR_INVALID_ARGS,
                      reboot_array->id);
        return;
    }

    if (reboot_array->id == CPU_ID_MP) {
        /* Wait for ssystem disabling firewall. */
        while (1) {
            if (sdrv_general_reg_get(SDRV_REG_STATUS,
                                     SDRV_REG_STATUS_FIREWALL_EARLY_DONE)) {
                break;
            }
        }
    }

    mem_image_init(reboot_array->seeker_base, reboot_array->seeker_size);

    ret = mem_image_seek(reboot_array->seeker_base, reboot_array->pt_name, &info);

    if (ret) {
        ssdk_printf(SSDK_ERR, "don't find preloader for reboot!\n");
        ERRCODE_PRINT(MOD_ID_RESET, DEFAULT_MOD_NO, RESET_ERR_SEEK_IMG_FAILED,
                      reboot_array->id);
        return;
    }

    if (reboot_array->id == CPU_ID_SEC) {
        if (reboot_array->mbox != NULL) {
            ret = send_peer_load_msg(info.base, reboot_array->mbox);

            if (ret)
                ERRCODE_PRINT(MOD_ID_RESET, DEFAULT_MOD_NO, ret, reboot_array->id);

            return;
        }
        else {
            // do nothing, kick secure core with common method
        }
    }

    if (!reboot_array->imgcpy_skip) {
        memcpy((void *)reboot_array->target_addr, (void *)(addr_t)info.base, info.sz);
        arch_clean_cache_range((uintptr_t)reboot_array->target_addr, info.sz);

        if (NO_ERROR != verified_image((void *)reboot_array->target_addr, info.sz,
                                    reboot_array->pt_name)) {
            ERRCODE_PRINT(MOD_ID_RESET, DEFAULT_MOD_NO, RESET_ERR_VERIFY_IMG_FAILED,
                        reboot_array->id);
            return;
        }
    }

#ifndef CONFIG_KUNLUN_BOOT_MP

    if (reboot_array->id == CPU_ID_MP) {
        sdrv_general_reg_set(SDRV_REG_STATUS, SDRV_REG_STATUS_SDPE_LD_DONE,
                             SDRV_REG_STATUS_SDPE_LD_DONE);
        return;
    }

#endif

    if (reboot_array->scr_entry_signal.num) {
        for (int i = 0; i < reboot_array->scr_entry_signal.num; i++) {
            scr_set(reboot_array->scr_entry_signal.scr_dev, \
                    &reboot_array->scr_entry_signal.scr_sig[i], \
                    reboot_array->scr_entry_signal.scr_value[i]);
        }
    }

    if (reboot_array->rst_cpu) {
        reset_ctl_reset(reboot_array->rst_cpu);
    }
}
