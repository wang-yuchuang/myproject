/**
 * @file fastboot_common.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: fastboot common package.
 *
 * Revision History:
 * -----------------
 */


#include  <stdbool.h>
#include  <Source/usbd_core.h>
#include  <board.h>
#include  <usb_bsp.h>
#include  "fastboot_common.h"


#if (USBD_CFG_MS_OS_DESC_EN == DEF_ENABLED)
#define  SD_VENDOR_ID                   0x3273
#define  SD_PRODUCT_ID                  0x0003
#else
#define  SD_VENDOR_ID                   0x18d1
#define  SD_PRODUCT_ID                  0x4d00
#endif
#define  SD_VERSION_ID                  0x0100

#define  SD_FB_CFG_MAX_NBR_CMD          20u
#define  SD_FB_CFG_MAX_NBR_VAR          20u


#define  fbc_err(x...)                  ssdk_printf(SSDK_NOTICE, "[FBC]" x)
#define  fbc_log(x...)                  ssdk_printf(SSDK_DEBUG,  "[FBC]" x)


static USBD_DRV_EP_INFO  USBD_DrvEP_InfoTbl_TAISHAN[] = {
    {USBD_EP_INFO_TYPE_CTRL                          | USBD_EP_INFO_DIR_OUT,                        0u,   64u},
    {USBD_EP_INFO_TYPE_CTRL                          | USBD_EP_INFO_DIR_IN,                         0u,   64u},
    {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN,  1u,  512u},
    {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN,  2u,  512u},
    {USBD_EP_INFO_TYPE_BULK                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN,  3u,  512u},
    {USBD_EP_INFO_TYPE_BULK                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN,  4u,  512u},
    {USBD_EP_INFO_TYPE_BULK                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN,  5u,  512u},
    {USBD_EP_INFO_TYPE_INTR                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN,  6u,   64u},
    {USBD_EP_INFO_TYPE_INTR                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN,  7u,   64u},
    {USBD_EP_INFO_TYPE_INTR                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN,  8u,   64u},
    {USBD_EP_INFO_TYPE_BULK                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN,  9u,  512u},
    {USBD_EP_INFO_TYPE_INTR                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN, 10u,   64u},
    {USBD_EP_INFO_TYPE_BULK                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN, 11u,  512u},
    {USBD_EP_INFO_TYPE_BULK                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN, 12u,  512u},
    {USBD_EP_INFO_TYPE_BULK                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN, 13u,  512u},
    {USBD_EP_INFO_TYPE_BULK                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN, 14u,  512u},
    {USBD_EP_INFO_TYPE_BULK                          | USBD_EP_INFO_DIR_OUT | USBD_EP_INFO_DIR_IN, 15u,  512u},
    {DEF_BIT_NONE                                                                                ,  0u,    0u}
};

static char usbd_fb_serial[] = "1234567890ABCDEF";

static const USBD_DEV_CFG  FB_USBD_DevCfg = {
    SD_VENDOR_ID,                       /* Vendor  ID.                                          */
    SD_PRODUCT_ID,                      /* Product ID.                                          */
    SD_VERSION_ID,                      /* Device release number.                               */
    "Semidrive",                        /* Manufacturer  string.                                */
    "Taishan",                          /* Product       string.                                */
    usbd_fb_serial,                     /* Serial number string.                                */
    USBD_LANG_ID_ENGLISH_US             /* String language ID.                                  */
};

static const USBD_DRV_CFG  FB_USBD_DrvCfg = {
    DEVICE_BASE(USB0D),                 /* Base addr   of device controller hw registers.       */
    0u,                                 /* Base addr   of device controller dedicated mem.      */
    0u,                                 /* Size        of device controller dedicated mem.      */
#if (USBD_CFG_HS_EN == DEF_ENABLED)
    USBD_DEV_SPD_HIGH,                  /* Speed       of device controller.                    */
#else
    USBD_DEV_SPD_FULL,
#endif

    USBD_DrvEP_InfoTbl_TAISHAN          /* EP Info tbl of device controller.                    */
};

                                        /* Donnot care about bus events.                        */
static  USBD_BUS_FNCTS  FB_USBD_BusFncts = {};

static fastboot_dev_t fb_dev = {USBD_CLASS_NBR_NONE, USBD_DEV_NBR_NONE};

static USBD_FB_CMD fb_cmd_tbl[SD_FB_CFG_MAX_NBR_CMD] = {
    {"getvar:",     USBD_FB_CmdGetVar},
    {"download:",   USBD_FB_CmdDownload},
};
static CPU_INT16U fb_cmd_len = 2;

static USBD_FB_VAR fb_var_tbl[SD_FB_CFG_MAX_NBR_VAR] = {
    {"version", "0.5"},
    {"name",    "fastboot"},
};
static CPU_INT16U fb_var_len = 2;


bool App_USBD_FB_Init ( fastboot_dev_t  *fbd,
                        CPU_INT08U      cfg_hs,
                        CPU_INT08U      cfg_fs)
{
    USBD_ERR    err;
    CPU_INT08U  fb_nbr;
    bool        valid;

    fbc_log("        Initializing FB class ... \n");

    USBD_FB_Init(&err);
    if (err != USBD_ERR_NONE) {
        fbc_err("        ... could not initialize FB class w/err = %d\n\n", err);
        return 0;
    }

    fb_nbr = USBD_FB_Add(&err);

    if (cfg_hs != USBD_CFG_NBR_NONE) {
        valid = USBD_FB_CfgAdd (fb_nbr,
                                fbd->dev_nbr,
                                cfg_hs,
                                &err);
        if (!valid) {
            fbc_err("        ... could not add fb instance #%d to HS configuration w/err = %d\n\n", fb_nbr, err);
            return 0;
        }
    }

    if (cfg_fs != USBD_CFG_NBR_NONE) {
        valid = USBD_FB_CfgAdd (fb_nbr,
                                fbd->dev_nbr,
                                cfg_fs,
                                &err);
        if (!valid) {
            fbc_err("        ... could not add fb instance #%d to FS configuration w/err = %d\n\n", fb_nbr, err);
            return 0;
        }
    }

    fbd->cls_nbr = fb_nbr;

#if (USBD_CFG_MS_OS_DESC_EN == DEF_ENABLED)
    USBD_DevSetMS_VendorCode(fbd->dev_nbr, 1u, &err);
    if (err != USBD_ERR_NONE) {
        fbc_err("        ... could not set MS vendor code w/err = %d\n\n", err);
    }
#endif

    return 1;
}

static bool fastboot_common_usb_init(fastboot_dev_t *fbd)
{
    CPU_INT08U  dev_nbr;
    CPU_INT08U  cfg_hs_nbr;
    CPU_INT08U  cfg_fs_nbr;
    bool        ok;
    USBD_ERR    err;

    fbc_log("Initializing USB Device...\n");

    USBD_Init(&err);
    if (err != USBD_ERR_NONE) {
        fbc_err("... init failed w/err = %d\n\n", err);
        return 0;
    }

    fbc_log("    Adding controller driver ... \n");
                                        /* Add USB device instance.                             */
    dev_nbr = USBD_DevAdd(&FB_USBD_DevCfg,
                          &FB_USBD_BusFncts,
                          &USBD_DrvAPI_RenesasRZ_DMA,
                          &FB_USBD_DrvCfg,
                          &USBD_DrvBSP_TAISHAN_USB0,
                          &err);
    if (err != USBD_ERR_NONE) {
        fbc_err("    ... could not add controller driver w/err =  %d\n\n", err);
        return 0;
    }
                                        /* Device is not self-powered.                          */
    USBD_DevSelfPwrSet(dev_nbr, DEF_NO, &err);

    cfg_hs_nbr = USBD_CFG_NBR_NONE;
    cfg_fs_nbr = USBD_CFG_NBR_NONE;

#if (USBD_CFG_HS_EN == DEF_ENABLED)
    fbc_log("    Adding HS configuration ... \n");
                                        /* Add HS configuration.                                */
    cfg_hs_nbr = USBD_CfgAdd(   dev_nbr,
                                USBD_DEV_ATTRIB_SELF_POWERED,
                                100u,
                                USBD_DEV_SPD_HIGH,
                                "HS fastboot config",
                                &err);
    if (err != USBD_ERR_NONE) {
        fbc_err("    ... could not add HS configuration w/err =  %d\n\n", err);
                                        /* Continue if dev is not high-speed.                   */
        if (err != USBD_ERR_DEV_INVALID_SPD) {
            return 0;
        }
    }
#endif

    fbc_log("    Adding FS configuration ... \n");
                                        /* Add FS configuration.                                */
    cfg_fs_nbr = USBD_CfgAdd(   dev_nbr,
                                USBD_DEV_ATTRIB_SELF_POWERED,
                                100u,
                                USBD_DEV_SPD_FULL,
                                "FS fastboot config",
                                &err);
    if (err != USBD_ERR_NONE) {
        fbc_err("    ... could not add FS configuration w/err =  %d\n\n", err);
        return 0;
    }

#if (USBD_CFG_HS_EN == DEF_ENABLED)
    if ((cfg_fs_nbr != USBD_CFG_NBR_NONE) &&
        (cfg_hs_nbr != USBD_CFG_NBR_NONE)) {
                                        /* Associate both config for other spd desc.            */
        USBD_CfgOtherSpeed( dev_nbr,
                            cfg_hs_nbr,
                            cfg_fs_nbr,
                            &err);
        if (err != USBD_ERR_NONE) {
            fbc_err("    ... could not associate other speed configuration w/err =  %d\n\n", err);
            return 0;
        }
    }
#endif

    fbc_log("        Initializing FB classes ... \n\n");

    fbd->dev_nbr = dev_nbr;
                                        /* Initialize FB class.                                 */
    ok = App_USBD_FB_Init(  fbd,
                            cfg_hs_nbr,
                            cfg_fs_nbr);
    if (!ok) {
        fbc_err("    ... could not initialize FB class w/err =  %d\n\n", err);
        return 0;
    }

    return 1;
}

void fastboot_register_cmd(const char *prefix, USBD_FB_CMD_HDL handle)
{
    if (!prefix || !*prefix || !handle|| (fb_cmd_len >= SD_FB_CFG_MAX_NBR_CMD)) {
        fbc_err("Add command fail cmd len=%d\n", fb_cmd_len);
        return;
    }

    fb_cmd_tbl[fb_cmd_len].cmd = prefix;
    fb_cmd_tbl[fb_cmd_len].handler = handle;
    fb_cmd_len++;
}

void fastboot_register_var(const char *name, const char *value)
{
    if (!name || !*name || !value || !*value || (fb_var_len >= SD_FB_CFG_MAX_NBR_VAR)) {
        fbc_err("Add variable fail var len=%d\n", fb_var_len);
        return;
    }

    fb_var_tbl[fb_var_len].name = name;
    fb_var_tbl[fb_var_len].value = value;
    fb_var_len++;
}

fastboot_t *fastboot_common_init(void *dl_addr, uint32_t dl_sz)
{
    fastboot_dev_t  *fbd;
    bool            ok;
    USBD_ERR        err;

    fbd = &fb_dev;

    ok = fastboot_common_usb_init(fbd);
    if (!ok)
        return NULL;

    USBD_FB_SetDL(fbd->cls_nbr, dl_addr, dl_sz, &err);
    USBD_FB_SetCmdTbl(fbd->cls_nbr, fb_cmd_tbl, fb_cmd_len, &err);
    USBD_FB_SetVarTbl(fbd->cls_nbr, fb_var_tbl, fb_var_len, &err);

    USBD_DevStart(fbd->dev_nbr, &err);
    if (err != USBD_ERR_NONE) {
        fbc_err("    ... could not start device stack w/err =  %d\n\n", err);
        return NULL;
    }

    fbc_log("        .... USB classes initialization done.\n");

    return &fbd->cls_nbr;
}

void fastboot_common_okay(fastboot_t *fb, const char *reason)
{
    if (fb) {
        USBD_FB_Okay(fb, reason);
    }
}


void fastboot_common_fail(fastboot_t *fb, const char *reason)
{
    if (fb) {
        USBD_FB_Fail(fb, reason);
    }
}

void fastboot_common_info(fastboot_t *fb, const char *reason)
{
    if (fb) {
        USBD_FB_Info(fb, reason);
    }
}

void fastboot_common_stop(fastboot_t *fb)
{
    USBD_ERR err;

    if (fb && (*fb == fb_dev.cls_nbr)) {
        USBD_DevStop(fb_dev.dev_nbr, &err);
    }
}


