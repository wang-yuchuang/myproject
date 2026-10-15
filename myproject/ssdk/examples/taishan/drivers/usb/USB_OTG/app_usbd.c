/**
 * @file app_usbd.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: usb device HID application of OTG demo.
 *
 * Revision History:
 * -----------------
 */


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/


#include  "app_usbd.h"
#include  <Source/usbd_core.h>
#include  <board.h>
#include  <usb_bsp.h>

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

/**
*********************************************************************************************************
 *                                USB DEVICE ENDPOINT INFORMATION TABLE
 *
 *  Note(s) :
 *          (1) It is possible to optimize the performances by correctly allocating the buffer size in
 *              the endpoint table. The Taishan USB controller has an 8 KBytes USB dedicated memory accessible
 *              through the FIFO channels. Depending on the buffer length you define here, you will enable
 *              the double buffering and continuous mode features of the controller. Buffer size values
 *              MUST ALWAYS be a multiple of 64 bytes.
 *
 *              (a) If the buffer size for a pipe is > (2 * maximum packet size), double buffering will
 *                  be enabled by the driver (isochronous and bulk pipes only).
 *
 *                  (1) If the size of a single buffer (i.e. buffer size / 2) is > (2 * maximum packet
 *                      size), continuous mode will be enabled by the driver (bulk pipes only).
 *
 *                      (a) The maximum buffer value that can be assigned to a pipe is 4096 bytes. This
 *                          will enable double buffering with 2 buffers of 2048 bytes.
 *
 *          (2) The pipe that you define here as interrupt should NOT be defined as bulk or
 *              isochronous as well.
 *
 *          (3) Always set the control and interrupt pipes buffer size to 64.
 *
 *          (4) Always set the pipes buffer size to the maximum packet size if DMA is not used.
 *
 *          (5) You do not need to define pipes that you won't use. (For instance, if you build a MSC
 *              device, you only need to define the 2 control endpoints + 2 bulk endpoints).
*********************************************************************************************************
**/
USBD_DRV_EP_INFO  USBD_DrvEP_InfoTbl_TAISHAN[] = {
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


static const USBD_DEV_CFG  USBD_DevCfg_Template = {
    0xFFFE,                             /* Vendor  ID.                                          */
    0x1233,                             /* Product ID (HID).                                    */
    0x0100,                             /* Device release number.                               */
   "SEMIDRIVE SEMICONDUCTOR",           /* Manufacturer  string.                                */
   "SEMIDRIVE TAISHAN",                 /* Product       string.                                */
   "1234567890ABCDEF",                  /* Serial number string.                                */
    USBD_LANG_ID_ENGLISH_US             /* String language ID.                                  */
};

static const USBD_DRV_CFG  USBD_DrvCfg_Template = {
    DEVICE_BASE(USB0D),                 /* Base addr   of device controller hw registers.       */
    0x00000000,                         /* Base addr   of device controller dedicated mem.      */
             0u,                        /* Size        of device controller dedicated mem.      */
#if (USBD_CFG_HS_EN == DEF_ENABLED)
    USBD_DEV_SPD_HIGH,                  /* Speed       of device controller.                    */
#else
    USBD_DEV_SPD_FULL,
#endif

    USBD_DrvEP_InfoTbl_TAISHAN          /* EP Info tbl of device controller.                    */
};


/*
*********************************************************************************************************
*                                   USB DEVICE CALLBACKS FUNCTIONS
*********************************************************************************************************
*/

static  void  App_USBD_EventReset  (CPU_INT08U  dev_nbr);
static  void  App_USBD_EventSuspend(CPU_INT08U  dev_nbr);
static  void  App_USBD_EventResume (CPU_INT08U  dev_nbr);
static  void  App_USBD_EventCfgSet (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val);
static  void  App_USBD_EventCfgClr (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val);
static  void  App_USBD_EventConn   (CPU_INT08U  dev_nbr);
static  void  App_USBD_EventDisconn(CPU_INT08U  dev_nbr);

static  USBD_BUS_FNCTS  App_USBD_BusFncts = {
    App_USBD_EventReset,
    App_USBD_EventSuspend,
    App_USBD_EventResume,
    App_USBD_EventCfgSet,
    App_USBD_EventCfgClr,
    App_USBD_EventConn,
    App_USBD_EventDisconn
};


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           App_USBD_Init()
*
* Description : Initialize USB device stack.
*
* Argument(s) : none.
*
* Return(s)   : 1,  if the stack was created successfully.
*               0,  if the stack could not be created.
*
* Note(s)     : none.
*********************************************************************************************************
*/
int App_USBD_Init (void)
{
    CPU_INT08U   dev_nbr;
    CPU_INT08U   cfg_hs_nbr;
    CPU_INT08U   cfg_fs_nbr;
    CPU_BOOLEAN  ok;
    USBD_ERR     err;


    printf("\n");
    printf("===================================================================\n");
    printf("=                    USB DEVICE INITIALIZATION                    =\n");
    printf("===================================================================\n");
    printf("Initializing USB Device...\n");


    USBD_Init(&err);
    if (err!= USBD_ERR_NONE) {
        printf("... init failed w/err = %d\n\n", err);
        return -1;
    }

    printf("    Adding controller driver ... \n");
                                        /* Add USB device instance.                             */
    dev_nbr = USBD_DevAdd(&USBD_DevCfg_Template,
                          &App_USBD_BusFncts,
                          &USBD_DrvAPI_RenesasRZ_DMA,
                          &USBD_DrvCfg_Template,
                          &USBD_DrvBSP_TAISHAN_USB0,
                          &err);
    if (err != USBD_ERR_NONE) {
        printf("    ... could not add controller driver w/err =  %d\n\n", err);
        return -1;
    }
                                        /* Device is not self-powered.                          */
    USBD_DevSelfPwrSet(dev_nbr, DEF_NO, &err);

    cfg_hs_nbr = USBD_CFG_NBR_NONE;
    cfg_fs_nbr = USBD_CFG_NBR_NONE;

#if (USBD_CFG_HS_EN == DEF_ENABLED)
    printf("    Adding HS configuration ... \n");
                                        /* Add HS configuration.                                */
    cfg_hs_nbr = USBD_CfgAdd (  dev_nbr,
                                USBD_DEV_ATTRIB_SELF_POWERED,
                                100u,
                                USBD_DEV_SPD_HIGH,
                                "HS configuration",
                                &err);
    if (err != USBD_ERR_NONE) {
        printf("    ... could not add HS configuration w/err =  %d\n\n", err);
                                        /* Continue if dev is not high-speed.                   */
        if (err != USBD_ERR_DEV_INVALID_SPD) {
            return -1;
        }
    }
#endif

    printf("    Adding FS configuration ... \n");
                                        /* Add FS configuration.                                */
    cfg_fs_nbr = USBD_CfgAdd (  dev_nbr,
                                USBD_DEV_ATTRIB_SELF_POWERED,
                                100u,
                                USBD_DEV_SPD_FULL,
                                "FS configuration",
                                &err);
    if (err != USBD_ERR_NONE) {
        printf("    ... could not add FS configuration w/err =  %d\n\n", err);
        return -1;
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
            printf("    ... could not associate other speed configuration w/err =  %d\n\n", err);
            return -1;
        }
    }
#endif

    printf("\n");
    printf("        ===========================================================\n");
    printf("        Initializing HID class ... \n\n");

                                        /* Initialize HID class.                                */
    ok = App_USBD_HID_Init( dev_nbr,
                            cfg_hs_nbr,
                            cfg_fs_nbr);
    if (ok != DEF_OK) {
        printf("    ... could not initialize HID class w/err =  %d\n\n", err);
        return -1;
    }

    printf("        .... USB classes initialization done.\n");

    return dev_nbr;
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            USB CALLBACKS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        App_USBD_EventReset()
*
* Description : Bus reset event callback function.
*
* Argument(s) : dev_nbr    Device number.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  App_USBD_EventReset (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventSuspend()
*
* Description : Bus suspend event callback function.
*
* Argument(s) : dev_nbr    Device number.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  App_USBD_EventSuspend (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventResume()
*
* Description : Bus Resume event callback function.
*
* Argument(s) : dev_nbr    Device number.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  App_USBD_EventResume (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventCfgSet()
*
* Description : Set configuration callback event callback function.
*
* Argument(s) : dev_nbr    Device number.
*
*               cfg_nbr    Active device configuration number selected by the host.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  App_USBD_EventCfgSet (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val)
{
    (void)dev_nbr;
    (void)cfg_val;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventCfgClr()
*
* Description : Clear configuration event callback function.
*
* Argument(s) : dev_nbr    Device number.
*
*               cfg_nbr    Current device configuration number that will be closed.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  App_USBD_EventCfgClr (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val)
{
    (void)dev_nbr;
    (void)cfg_val;
}


/*
*********************************************************************************************************
*                                        App_USBD_EventConn()
*
* Description : Device connection event callback function.
*
* Argument(s) : dev_nbr    Device number.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  App_USBD_EventConn (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventDisconn()
*
* Description : Device connection event callback function.
*
* Argument(s) : dev_nbr    USB device number.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  App_USBD_EventDisconn (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}

