/**
 * @file app_usbh_msc.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: USB Host MSC test & demo application.
 *
 * Revision History:
 * -----------------
 */


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <debug.h>
#include  <cmsis_os2.h>
#include  <ff.h>
#include  <lib/disk/disk.h>
#include  <lib/disk/disk_usb.h>
#include  "app_usbh_msc.h"


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_USBH_MSC_BUF_SIZE                         (16*2*512u)
#define  APP_USBH_MSC_TEST_BLK_ADDR                    (1024u * 2 - 1)


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

_Alignas(CONFIG_ARCH_CACHE_LINE)
static  CPU_INT08U          App_USBH_MSC_BufRx[APP_USBH_MSC_BUF_SIZE];

static  USBH_HQUEUE         App_USBH_MSC_DevQ;

static  struct disk_info    udisk_inf = {DISK_USB_NAME(0)};
static  FATFS               udisk_fs;
static  FIL                 f_src, f_dst;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  App_USBH_MSC_ClassNotify(void        *p_msc_dev,
                                       CPU_INT08U   is_conn,
                                       void        *p_ctx);

static  void  App_USBH_MSC_RWTask     (void        *p_ctx);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL FUNCTION
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         App_USBH_MSC_Init()
*
* Description : Register MSC notify callback, create demo objects & start demo tasks.
*
* Argument(s) : None.
*
* Return(s)   : USBH_ERR_NONE,          if the demo is initialized.
*               Specific error code,    otherwise.
*
* Note(s)     : None.
*********************************************************************************************************
*/

USBH_ERR  App_USBH_MSC_Init (void)
{
    USBH_ERR    err;
    USBH_HTASK  htask;


    App_USBH_MSC_DevQ = USBH_OS_MsgQueueCreate( NULL,
                                                USBH_MSC_CFG_MAX_DEV,
                                                &err);
    if (err != USBH_ERR_NONE) {
        return (err);
    }

    err = USBH_OS_TaskCreate(           "MSC Demo File Task",   /* Create the file I/O task.                            */
                                         osPriorityNormal,
                                         App_USBH_MSC_RWTask,
                             (void    *) 0,
                             (CPU_STK *) NULL,
                                         4096u,
                                        &htask);
    if (err != USBH_ERR_NONE) {
        return (err);
    }

    err = USBH_ClassDrvReg(       &USBH_MSC_ClassDrv,           /* Register MSC driver.                                 */
                                   App_USBH_MSC_ClassNotify,
                           (void *)0);
    return(err);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTION
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                     App_USBH_MSC_ClassNotify()
*
* Description : Handle device state change notification for mass storage class devices.
*
* Argument(s) : p_class_dev     Pointer to class device.
*
*               is_conn         Indicate connection status of MSC device.
*                       USBH_CLASS_DEV_STATE_CONN       Device is connected.
*                       USBH_CLASS_DEV_STATE_DISCONN    Device is disconnected.
*
*               p_ctx           Pointer to context.
*
* Return(s)   : USBH_ERR_NONE,      if no error
*               Specific error,     otherwise
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  App_USBH_MSC_ClassNotify (void        *p_class_dev,
                                        CPU_INT08U   is_conn,
                                        void        *p_ctx)
{
    USBH_MSC_DEV  *p_msc_dev;
    USBH_ERR       usb_err;


    (void)p_ctx;

    p_msc_dev = (USBH_MSC_DEV *)p_class_dev;
    switch (is_conn) {
        case USBH_CLASS_DEV_STATE_CONN:                         /* ------------ MASS STORAGE DEVICE CONN'D ------------ */
             usb_err = USBH_MSC_RefAdd(p_msc_dev);
             if (usb_err != USBH_ERR_NONE) {
                 printf("Cannot add new MSC reference w/err: %d\n", usb_err);
                 return;
             }

             (void)USBH_OS_MsgQueuePut(        App_USBH_MSC_DevQ,
                                       (void *)p_msc_dev);

             printf("MSC Demo: Device with unit #%p connected\n", p_msc_dev);
             break;

        case USBH_CLASS_DEV_STATE_DISCONN:                      /* ----------- MASS STORAGE DEVICE REMOVED ------------ */
             USBH_MSC_RefRel(p_msc_dev);

             printf("MSC Demo: Device Removed\n");
             break;

        default:
             break;
    }
}

static int msc_copy_test(void)
{
    char    fil_name[32];
    FRESULT rc_s = 0;
    FRESULT rc_d = 0;
    UINT    br;

    snprintf(fil_name, sizeof(fil_name), "%s:1:test.exe", udisk_inf.disk_name);
    rc_s |= f_open(&f_src, fil_name, FA_READ);
    if (rc_s) {
        printf("ERR: f_open %s ret %d\n", fil_name, rc_s);
        return (rc_s == FR_NO_FILE) ? 0 : -1;
    }
    rc_s |= f_lseek(&f_src, 0);
    if (rc_s) {
        printf("ERR:f_lseek ret %d\n", rc_s);
        goto close_src;
    }

    snprintf(fil_name, sizeof(fil_name), "%s:1:test.exe.bak", udisk_inf.disk_name);
    rc_d |= f_open(&f_dst, fil_name, FA_CREATE_ALWAYS | FA_WRITE);
    if (rc_d) {
        printf("ERR: f_open %s ret %d\n", fil_name, rc_d);
        goto close_src;
    }

    do {
        rc_s |= f_read(&f_src, App_USBH_MSC_BufRx, sizeof(App_USBH_MSC_BufRx), &br);
        if (rc_s) {
            printf("ERR: f_read ret %d\n", rc_s);
            break;
        }
        printf("read %d bytes\n", br);

        rc_d |= f_write(&f_dst, App_USBH_MSC_BufRx, br, &br);
        if (rc_d) {
            printf("ERR: f_write ret %d\n", rc_d);
            break;
        }
        printf("write %d bytes\n", br);

    } while(br == sizeof(App_USBH_MSC_BufRx));


    rc_d |= f_close(&f_dst);
    if (rc_d) {
        printf("ERR:f_close ret %d\n", rc_d);
    }

close_src:
    rc_s |= f_close(&f_src);
    if (rc_s) {
        printf("ERR:f_close ret %d\n", rc_s);
    }

    return (rc_s || rc_d) ? -1 : 0;
}

/*
*********************************************************************************************************
*                                       App_USBH_MSC_RWTask()
*
* Description : Open, Write, Read and compare a block to a connected mass storage device.
*
* Argument(s) : p_ctx       Pointer to the context.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  App_USBH_MSC_RWTask (void  *p_ctx)
{
    USBH_MSC_DEV   *p_msc_dev;
    USBH_ERR        err_usbh;
    int             ret;
    FRESULT         rc;
    char            mnt_name[16];

    while (1) {
        p_msc_dev = USBH_OS_MsgQueueGet(App_USBH_MSC_DevQ,
                                        0u,
                                        &err_usbh);
        if (err_usbh != USBH_ERR_NONE) {
            printf("Could not find volume w/err %d.\n",
                             err_usbh);
            continue;
        }

        udisk_inf.private_data = p_msc_dev;
        ret = register_usb_disk(&udisk_inf);
        if (ret < 0) {
            printf("Could not register disk err %d.\n", ret);
            continue;
        }

        snprintf(mnt_name, sizeof(mnt_name), "%s:1", udisk_inf.disk_name);
        rc = f_mount(&udisk_fs, mnt_name, 1);
        if (rc) {
            printf("ERROR: f_mount ret %d\n", rc);
            goto unregister;
        }

        msc_copy_test();

        printf("USBH MSC copy test of dev%d is done.\n", p_msc_dev->DevPtr->DevAddr);

        rc = f_unmount(mnt_name);
        if (rc) {
            printf("ERR: unmount ret %d\n", rc);
        }

unregister:
        ret = unregister_usb_disk(&udisk_inf);
        if (ret < 0) {
            printf("unregister disk err %d.\n", ret);
        }
        udisk_inf.private_data = NULL;
    }
}

