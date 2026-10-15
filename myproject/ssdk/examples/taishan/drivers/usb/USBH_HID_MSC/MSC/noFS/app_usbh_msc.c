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
#include  "app_usbh_msc.h"


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_USBH_MSC_BUF_SIZE                         512u
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
static  CPU_INT08U   App_USBH_MSC_BufRx[APP_USBH_MSC_BUF_SIZE];
_Alignas(CONFIG_ARCH_CACHE_LINE)
static  CPU_INT08U   App_USBH_MSC_BufTx[APP_USBH_MSC_BUF_SIZE];

static  USBH_HQUEUE  App_USBH_MSC_DevQ;


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

static inline
char digit_to_char(unsigned char d)
{
    if (d >= 10)
        return d - 10 + 'A';
    else
        return d + '0';
}

static void printf_buf8(const unsigned char *buf8, unsigned int len)
{
    int i;
    char buf_ascii[3*16+8];
    char *pbf;

    if (len > 16)
        len = 16;
    pbf = buf_ascii;
    for (i = 0; i < len; i++) {
        *pbf++ = digit_to_char(buf8[i] >> 4);
        *pbf++ = digit_to_char(buf8[i] & 0xFu);
        *pbf++ = ' ';
    }
    *pbf++ = '\n';
    *pbf++ = 0;

    printf(buf_ascii);
}

static void msc_rw_test(USBH_MSC_DEV *p_msc_dev, CPU_INT32U blk_addr, CPU_INT08U *wbuf, CPU_INT08U *rbuf)
{
    CPU_INT32U  actual_len;
    USBH_ERR    err_usbh;

    actual_len = USBH_MSC_Wr(p_msc_dev, 0, blk_addr, 1, 512, wbuf, &err_usbh);
    if ((err_usbh != USBH_ERR_NONE) || (actual_len != 512)) {
        printf("@%s.%d MSC Write Fail %d len=%d.\n",
                         __func__, __LINE__, err_usbh, actual_len);
        return;
    }

    actual_len = USBH_MSC_Rd(p_msc_dev, 0, blk_addr, 1, 512, rbuf, &err_usbh);
    if ((err_usbh != USBH_ERR_NONE) || (actual_len != 512)) {
        printf("@%s.%d MSC Read Fail %d len=%d.\n",
                         __func__, __LINE__, err_usbh, actual_len);
        return;
    }

    if (!memcmp((void *)wbuf, (void *)rbuf, 512u)) {
        printf("RW Test Passed\n");
    } else {
        printf("RW Test Failed!\n");
    }

    return;
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
    USBH_MSC_DEV  *p_msc_dev;
    USBH_ERR      err_usbh;
    CPU_INT08U    lun_nbr;
    _Bool       unit_ready;
    CPU_INT32U      blk_nr, blk_sz;
    USBH_MSC_INQUIRY_INFO   msc_inqr;
    CPU_INT32U      actual_len, ix, j;


    while (1) {
        p_msc_dev = USBH_OS_MsgQueueGet(App_USBH_MSC_DevQ,
                                        0u,
                                        &err_usbh);
        if (err_usbh != USBH_ERR_NONE) {
            printf("Could not find volume w/err %d.\n",
                             err_usbh);
            continue;
        }

        printf("USB Disk %d(%p) connected...\n", p_msc_dev->DevPtr->DevAddr, p_msc_dev);

        lun_nbr = USBH_MSC_MaxLUN_Get(p_msc_dev, &err_usbh);
        if (err_usbh != USBH_ERR_NONE) {
            printf("@%s.%d Get Max LUN Fail %d.\n",
                             __func__, __LINE__, err_usbh);
            continue;
        }
        printf("USB Disk %d Max LUN is %d\n", p_msc_dev->DevPtr->DevAddr, lun_nbr);

        unit_ready = USBH_MSC_UnitRdyTest(p_msc_dev, 0, &err_usbh);
        if (err_usbh != USBH_ERR_NONE) {
            printf("@%s.%d Unit Ready Test Fail %d.\n",
                             __func__, __LINE__, err_usbh);
            continue;
        }
        printf("USB Disk %d Unit Ready? %c\n", p_msc_dev->DevPtr->DevAddr, unit_ready ? 'Y' : 'N');

        err_usbh = USBH_MSC_CapacityRd(p_msc_dev, 0, &blk_nr, &blk_sz);
        if (err_usbh != USBH_ERR_NONE) {
            printf("@%s.%d Get Capacity Fail %d.\n",
                             __func__, __LINE__, err_usbh);
            continue;
        }
        printf("USB Disk %d Capacity is %d X %dBytes\n", p_msc_dev->DevPtr->DevAddr, blk_nr, blk_sz);

        err_usbh = USBH_MSC_StdInquiry(p_msc_dev, &msc_inqr, 0);
        if (err_usbh != USBH_ERR_NONE) {
            printf("@%s.%d StdInquiry Fail %d.\n",
                             __func__, __LINE__, err_usbh);
            continue;
        }
        printf("StdInquiry Type:%d Removable:%d Revision:%d \n", msc_inqr.DevType,
                            msc_inqr.IsRemovable, msc_inqr.ProductRevisionLevel);
        printf("VID\t");
        printf_buf8(msc_inqr.Vendor_ID, sizeof(msc_inqr.Vendor_ID));
        printf("PID\t");
        printf_buf8(msc_inqr.Product_ID, sizeof(msc_inqr.Product_ID));

        err_usbh = USBH_MSC_Init(p_msc_dev, 0);
        if (err_usbh != USBH_ERR_NONE) {
            printf("@%s.%d MSC Init Fail %d.\n",
                             __func__, __LINE__, err_usbh);
            continue;
        }

        printf("Block 0 Data:\n");
        actual_len = USBH_MSC_Rd(p_msc_dev, 0, 0, 1, blk_sz,
                                                App_USBH_MSC_BufRx, &err_usbh);
        if (err_usbh != USBH_ERR_NONE) {
            printf("@%s.%d MSC Read Fail %d.\n",
                             __func__, __LINE__, err_usbh);
            continue;
        }
        hexdump(App_USBH_MSC_BufRx, actual_len);

        printf("Block 2048 Data:\n");
        actual_len = USBH_MSC_Rd(p_msc_dev, 0, 1024 * 2, 1, blk_sz,
                                                App_USBH_MSC_BufRx, &err_usbh);
        if (err_usbh != USBH_ERR_NONE) {
            printf("@%s.%d MSC Read Fail %d.\n",
                             __func__, __LINE__, err_usbh);
            continue;
        }
        hexdump(App_USBH_MSC_BufRx, actual_len);

        actual_len = USBH_MSC_Rd(p_msc_dev, 0, APP_USBH_MSC_TEST_BLK_ADDR, 1,
                                        blk_sz, App_USBH_MSC_BufRx, &err_usbh);
        if (err_usbh != USBH_ERR_NONE) {
            printf("@%s.%d MSC Read Fail %d.\n",
                             __func__, __LINE__, err_usbh);
            continue;
        }

        for (j = 0; j < 2; j++) {
            for (ix = 0; ix < blk_sz; ix++)
                App_USBH_MSC_BufRx[ix] = ~App_USBH_MSC_BufRx[ix];
            msc_rw_test(p_msc_dev, APP_USBH_MSC_TEST_BLK_ADDR,
                                        App_USBH_MSC_BufRx, App_USBH_MSC_BufTx);
        }

        printf("USB Disk %d Test Done... ", p_msc_dev->DevPtr->DevAddr);
    }
}

