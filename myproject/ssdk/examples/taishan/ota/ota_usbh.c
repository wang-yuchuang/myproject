/**
 * @file ota_usbh.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ota from usb host application.
 *
 * Revision History:
 * -----------------
 */

#include  <cmsis_os2.h>
#include  <Source/usbh_core.h>
#include  <usb_bsp.h>
#include  <ff.h>
#include  <lib/disk/disk.h>
#include  <lib/disk/disk_usb.h>
#include  <lib/sd_boot_img/sd_boot_img.h>
#include  "ota_usbh.h"
#include  "ota.h"

osEventFlagsId_t udisk_ready;
static  struct disk_info    udisk_inf = {DISK_USB_NAME(0)};
static  void  App_USBH_MSC_ClassNotify(void *p_msc_dev, CPU_INT08U is_conn,
                                       void *p_ctx);


static USBH_KERNEL_TASK_INFO  AsyncTaskInfo
= {    /* ---------------- INFO ON ASYNC TASK ---------------- */
    osPriorityHigh,                         /* Async task priority. */
    NULL,                                   /* Ptr to async task stack. */
    4096u                                   /* Size of async task stack. */
};

static USBH_KERNEL_TASK_INFO  HubTaskInfo
= {      /* ----------------- INFO ON HUB TASK ----------------- */
    osPriorityHigh1,                        /* Hub task priority. */
    NULL,                                   /* Ptr to hub task stack. */
    4096u                                   /* Size of hub task stack. */
};

static USBH_HC_CFG  USB0H_EHCI_Cfg_TAISHAN = {
    (CPU_ADDR)DEVICE_BASE(USB0H) + RUSB_EHCI_OFFSET,        /* Base addr of host controller hw registers. */
    (CPU_ADDR)0x00000000u,                                  /* Base addr of host controller dedicated mem. */
    0u,                                           /* Size      of host controller dedicated mem. */
    DEF_ENABLED,                                  /* Does HC can access sys mem? */
    0x1000,                                       /* Data buf max len. */
    12u,                                          /* Max nbr opened bulk EP. */
    12u,                                          /* Max nbr opened intr EP. */
    4u                                            /* Max nbr opened isoc EP. */
};

USBH_HC_CFG  USB0H_OHCI_Cfg_TAISHAN = {
    (CPU_ADDR)DEVICE_BASE(USB0H) + RUSB_OHCI_OFFSET,        /* Base addr of host controller hw registers.           */
    (CPU_ADDR)0x00000000u,                                  /* Base addr of host controller dedicated mem.          */
    0u,                                           /* Size      of host controller dedicated mem.          */
    DEF_ENABLED,                                  /* Does HC can access sys mem?                          */
    0x1000,                                       /* Data buf max len.                                    */
    12u,                                          /* Max nbr opened bulk EP.                              */
    12u,                                          /* Max nbr opened intr EP.                              */
    4u                                            /* Max nbr opened isoc EP.                              */
};


/**
 * @brief Handle device state change notification for mass storage class devices.
 *
 * @param p_class_dev  Pointer to class device.
 * @param is_conn Indicate connection status of MSC device.
 *                       USBH_CLASS_DEV_STATE_CONN       Device is connected.
 *                       USBH_CLASS_DEV_STATE_DISCONN    Device is disconnected.
 * @param p_ctx Pointer to context.
 */
static  void  OTA_USBH_MSC_ClassNotify (void        *p_class_dev,
                                        CPU_INT08U   is_conn,
                                        void        *p_ctx)
{
    USBH_MSC_DEV  *p_msc_dev;
    USBH_ERR       usb_err;


    (void)p_ctx;

    p_msc_dev = (USBH_MSC_DEV *)p_class_dev;

    switch (is_conn) {
        /* ------------ MASS STORAGE DEVICE CONN'D ------------ */
        case USBH_CLASS_DEV_STATE_CONN:
            usb_err = USBH_MSC_RefAdd(p_msc_dev);

            if (usb_err != USBH_ERR_NONE) {
                ERROR("Cannot add new MSC reference w/err: %d\n", usb_err);
                return;
            }

            udisk_inf.private_data = p_msc_dev;

            if (register_usb_disk(&udisk_inf) < 0) {
                ERROR("Could not register disk err\n");
                return;
            }

            osEventFlagsSet(udisk_ready, 1);


            DBG("MSC Demo: Device with unit #%p connected\n", p_msc_dev);
            break;

        /* ----------- MASS STORAGE DEVICE REMOVED ------------ */
        case USBH_CLASS_DEV_STATE_DISCONN:
            USBH_MSC_RefRel(p_msc_dev);

            if (unregister_usb_disk(&udisk_inf) < 0) {
                ERROR("unregister disk err\n");
                return;
            }

            udisk_inf.private_data = NULL;
            osEventFlagsSet(udisk_ready, 0);

            DBG("MSC Demo: Device Removed\n");
            break;

        default:
            break;
    }
}

/**
 * @brief Initialize USB Host Stack and additional demos.
 *
 * @return DEF_OK:if successfull DEF_FAIL:otherwise.
 */
CPU_BOOLEAN  OTA_USBH_Init (void)
{
    USBH_ERR    err;
    CPU_INT08U  ohci_hc_nbr;
    CPU_INT08U  ehci_hc_nbr;

    DBG("\n");
    DBG("=============================\n");
    DBG("=  USB HOST INITIALIZATION  =\n");
    DBG("=============================\n");

    err = USBH_Init(&AsyncTaskInfo, &HubTaskInfo);

    if (err != USBH_ERR_NONE) {
        ERROR("...could not initialize USB HOST Stack  w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    DBG("... Initiliazing HOST Mass Storage class ...\n");
    err = USBH_ClassDrvReg(&USBH_MSC_ClassDrv, OTA_USBH_MSC_ClassNotify, (void *)0);

    if (err != USBH_ERR_NONE) {
        ERROR("...could not initialize HOST Mass Storage Class w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    ehci_hc_nbr = USBH_HC_Add (&USB0H_EHCI_Cfg_TAISHAN, &EHCI_DrvAPI, &EHCI_RH_API,
                               &USB0H_EHCI_BSP_TAISHAN, &err);

    if (err != USBH_ERR_NONE) {
        ERROR("...could not add EHCI controller w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    ohci_hc_nbr = USBH_HC_Add (&USB0H_OHCI_Cfg_TAISHAN, &OHCI_DrvAPI, &OHCI_RH_API,
                               &USB0H_OHCI_BSP_TAISHAN, &err);

    if (err != USBH_ERR_NONE) {
        ERROR("...could not add OHCI controller w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    /* Note: Start OHCI before EHCI! */
    err = USBH_HC_Start(ohci_hc_nbr);

    if (err != USBH_ERR_NONE) {
        ERROR("...could not start OHCI controller w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    err = USBH_HC_Start(ehci_hc_nbr);

    if (err != USBH_ERR_NONE) {
        ERROR("...could not start EHCI controller w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    udisk_ready = osEventFlagsNew(NULL);
    osEventFlagsSet(udisk_ready, 0);


    return (DEF_OK);
}
