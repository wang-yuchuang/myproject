/**
 * @file app_usbd_msc.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: usb device MSC test & demo application.
 *
 * Revision History:
 * -----------------
 */


#include  "app_usbd.h"
#include  <Class/MSC/usbd_msc.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         App_USBD_MSC_Init()
*
* Description : Initialize USB device mass storage class.
*
* Argument(s) : p_dev     Pointer to USB device.
*
*               cfg_hs    Index of high-speed configuration to which this interface will be added to.
*
*               cfg_fs    Index of high-speed configuration to which this interface will be added to.
*
* Return(s)   : 1,        if the Mass storage interface was added.
*               0,        if the Mass storage interface could not be added.
*
* Note(s)     : none.
*********************************************************************************************************
*/

bool  App_USBD_MSC_Init(unsigned char   dev_nbr,
                        unsigned char   cfg_hs,
                        unsigned char   cfg_fs)
{
    USBD_ERR    err;
    CPU_INT08U  msc_nbr;
    bool        valid;


    printf("        Initializing MSC class ... \n");

    USBD_MSC_Init(&err);
    if (err != USBD_ERR_NONE) {
        printf("        ... could not initialize MSC class w/err = %d\n\n", err);
        return 0;
    }

    msc_nbr = USBD_MSC_Add(&err);

    if (cfg_hs != USBD_CFG_NBR_NONE) {
        valid = USBD_MSC_CfgAdd (msc_nbr,
                                 dev_nbr,
                                 cfg_hs,
                                 &err);

        if (!valid) {
            printf("        ... could not add msc instance #%d to HS configuration w/err = %d\n\n", msc_nbr, err);
            return 0;
        }
    }

    if (cfg_fs != USBD_CFG_NBR_NONE) {
        valid = USBD_MSC_CfgAdd(msc_nbr,
                                dev_nbr,
                                cfg_fs,
                                &err);

        if (!valid) {
            printf("        ... could not add msc instance #%d to FS configuration w/err = %d\n\n", msc_nbr, err);
            return 0;
        }
    }
                                        /* Add Logical Unit to MSC interface.                   */
    USBD_MSC_LunAdd((void *)"ram:0:",
                             msc_nbr,
                    (void *)"Micrium",
                    (void *)"MSC FS Storage",
                             0x00,
                             DEF_FALSE,
                            &err);
    if (err != USBD_ERR_NONE) {
        printf("        ... could not add LU to MSC class w/err = %d\n\n", err);
        return 0;
    }

    return 1;
}

