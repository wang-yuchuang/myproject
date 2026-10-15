/**
 * @file app_usbd_hid.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: usb device HID test & demo application.
 *
 * Revision History:
 * -----------------
 */


#include  "app_usbd.h"
#include  <Class/HID/usbd_hid.h>
#include  <cmsis_os2.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

                                        /* Should always be 4 for mouse report.                 */
#define  APP_USBD_HID_REPORT_LEN                         4u


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

static  CPU_INT08U  App_USBD_HID_Protocol;
static  CPU_INT08U  App_USBD_HID_ReportBuf[APP_USBD_HID_REPORT_LEN];


static  CPU_INT32S  App_USBD_HID_X;
static  CPU_INT32S  App_USBD_HID_Y;

static  CPU_INT08U  App_USBD_HID_ReportDesc[] = {               /* HID Mouse report                                     */
    USBD_HID_GLOBAL_USAGE_PAGE           + 1, USBD_HID_USAGE_PAGE_GENERIC_DESKTOP_CONTROLS,
    USBD_HID_LOCAL_USAGE                 + 1, USBD_HID_CA_MOUSE,
    USBD_HID_MAIN_COLLECTION             + 1, USBD_HID_COLLECTION_APPLICATION,
        USBD_HID_LOCAL_USAGE             + 1, USBD_HID_CP_POINTER,
        USBD_HID_MAIN_COLLECTION         + 1, USBD_HID_COLLECTION_PHYSICAL,
            USBD_HID_GLOBAL_USAGE_PAGE   + 1, USBD_HID_USAGE_PAGE_BUTTON,
            USBD_HID_LOCAL_USAGE_MIN     + 1, 0x01,
            USBD_HID_LOCAL_USAGE_MAX     + 1, 0x03,
            USBD_HID_GLOBAL_LOG_MIN      + 1, 0x00,
            USBD_HID_GLOBAL_LOG_MAX      + 1, 0x01,
            USBD_HID_GLOBAL_REPORT_COUNT + 1, 0x03,
            USBD_HID_GLOBAL_REPORT_SIZE  + 1, 0x01,
            USBD_HID_MAIN_INPUT          + 1, USBD_HID_MAIN_DATA | USBD_HID_MAIN_VARIABLE | USBD_HID_MAIN_ABSOLUTE,
            USBD_HID_GLOBAL_REPORT_COUNT + 1, 0x01,
            USBD_HID_GLOBAL_REPORT_SIZE  + 1, 0x0D,
            USBD_HID_MAIN_INPUT          + 1, USBD_HID_MAIN_CONSTANT,
            USBD_HID_GLOBAL_USAGE_PAGE   + 1, USBD_HID_USAGE_PAGE_GENERIC_DESKTOP_CONTROLS,
            USBD_HID_LOCAL_USAGE         + 1, USBD_HID_DV_X,
            USBD_HID_LOCAL_USAGE         + 1, USBD_HID_DV_Y,
            USBD_HID_GLOBAL_LOG_MIN      + 1, 0x81,
            USBD_HID_GLOBAL_LOG_MAX      + 1, 0x7F,
            USBD_HID_GLOBAL_REPORT_SIZE  + 1, 0x08,
            USBD_HID_GLOBAL_REPORT_COUNT + 1, 0x02,
            USBD_HID_MAIN_INPUT          + 1, USBD_HID_MAIN_DATA | USBD_HID_MAIN_VARIABLE | USBD_HID_MAIN_RELATIVE,
        USBD_HID_MAIN_ENDCOLLECTION,
    USBD_HID_MAIN_ENDCOLLECTION
};


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void         App_USBD_HID_MouseTask       (void        *p_arg);

static  void         App_USBD_HID_MouseSetReport  (CPU_INT08U  *p_buf);

static  CPU_BOOLEAN  App_USBD_HID_FeatureReportGet(CPU_INT08U   class_nbr,
                                                   CPU_INT08U   report_id,
                                                   CPU_INT08U  *p_report_buf,
                                                   CPU_INT16U   report_len);

static  CPU_BOOLEAN  App_USBD_HID_FeatureReportSet(CPU_INT08U   class_nbr,
                                                   CPU_INT08U   report_id,
                                                   CPU_INT08U  *p_report_buf,
                                                   CPU_INT16U   report_len);

static  CPU_INT08U   App_USBD_HID_ProtocolGet     (CPU_INT08U   class_nbr,
                                                   USBD_ERR    *p_err);

static  void         App_USBD_HID_ProtocolSet     (CPU_INT08U   class_nbr,
                                                   CPU_INT08U   protocol,
                                                   USBD_ERR    *p_err);

static  void         App_USBD_HID_ReportSet       (CPU_INT08U   class_nbr,
                                                   CPU_INT08U   report_id,
                                                   CPU_INT08U  *p_report_buf,
                                                   CPU_INT16U   report_len);


static  USBD_HID_CALLBACK  App_USBD_HID_Callback = {
    App_USBD_HID_FeatureReportGet,
    App_USBD_HID_FeatureReportSet,
    App_USBD_HID_ProtocolGet,
    App_USBD_HID_ProtocolSet,
    App_USBD_HID_ReportSet
};


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         App_USBD_HID_Init()
*
* Description : Add a HID interface to the USB device stack.
*
* Argument(s) : dev_nbr    Device number.
*
*               cfg_hs     Index of high-speed configuration to which this interface will be added to.
*
*               cfg_fs     Index of full-speed configuration to which this interface will be added to.
*
* Return(s)   : 1,         if HID interface successfully added.
*
*               0,         otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

bool  App_USBD_HID_Init(unsigned char   dev_nbr,
                        unsigned char   cfg_hs,
                        unsigned char   cfg_fs)
{
    USBD_ERR        err;
    USBD_ERR        err_hs;
    USBD_ERR        err_fs;
    CPU_INT08U      class_nbr;
    osThreadId_t    thread;
    osThreadAttr_t  attr = {
        .name = "USBD HID Mouse",
        .stack_size = 4096,
        .priority = osPriorityNormal3,
    };

    err_hs = USBD_ERR_NONE;
    err_fs = USBD_ERR_NONE;

    App_USBD_HID_X = 101;
    App_USBD_HID_Y = 101;

    printf("        Initializing HID class ... \n");
                                        /* Init HID class.                                      */
    USBD_HID_Init(&err);
    if (err != USBD_ERR_NONE) {
        printf("        ... could not initialize HID class w/err = %d\n\n", err);
        return 0;
    }
                                                                /* Create a HID class instance.                         */
    class_nbr = USBD_HID_Add(   USBD_HID_SUBCLASS_BOOT,
                                USBD_HID_PROTOCOL_MOUSE,
                                USBD_HID_COUNTRY_CODE_NOT_SUPPORTED,
                                &App_USBD_HID_ReportDesc[0],
                                sizeof(App_USBD_HID_ReportDesc),
                                (CPU_INT08U *)0,
                                0u,
                                2u,
                                2u,
                                DEF_YES,
                                &App_USBD_HID_Callback,
                                &err);
    if (err != USBD_ERR_NONE) {
        printf("        ... could not instantiate a HID class w/err = %d\n\n", err);
        return 0;
    }

    if (cfg_hs != USBD_CFG_NBR_NONE) {
                                        /* Add HID class to HS dflt cfg.                        */
        USBD_HID_CfgAdd(class_nbr, dev_nbr, cfg_hs, &err_hs);
        if (err_hs != USBD_ERR_NONE) {
            printf("        ... could not add HID class instance #%d to HS configuration w/err = %d\n\n", class_nbr, err_hs);
        }
    }

    if (cfg_fs != USBD_CFG_NBR_NONE) {
                                        /* Add HID class to FS dflt cfg.                        */
        USBD_HID_CfgAdd(class_nbr, dev_nbr, cfg_fs, &err_fs);
        if (err_fs != USBD_ERR_NONE) {
            printf("        ... could not add HID class instance #%d to FS configuration w/err = %d\n\n", class_nbr, err_fs);
        }
    }

    if ((err_hs != USBD_ERR_NONE) &&    /* If HS and FS cfg fail, stop class init.              */
        (err_fs != USBD_ERR_NONE)) {
        return 0;
    }

    thread = osThreadNew(App_USBD_HID_MouseTask, (void *)(CPU_ADDR)class_nbr, &attr);
    if (!thread) {
        printf("        ... could not add HID mouse task \n");
        return 0;
    }

    return 1;
}


/*
*********************************************************************************************************
*                                      App_USBD_HID_MouseTask()
*
* Description : Perform HID writes to host. The HID writes simulate the movement of a mouse.
*
* Argument(s) : p_arg    Argument passed to 'App_USBD_HID_MouseTask()'.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_HID_MouseTask (void  *p_arg)
{
    CPU_INT08U  class_nbr;
    bool        conn;
    USBD_ERR    err;

    class_nbr = (CPU_INT08U)(CPU_ADDR)p_arg;
    printf("%s cls no.=%d\n", __func__, class_nbr);

    while (1) {
        conn = USBD_HID_IsConn(class_nbr);
        while (!conn) {

            osDelay(250u);

            conn = USBD_HID_IsConn(class_nbr);
        }

        App_USBD_HID_MouseSetReport(&App_USBD_HID_ReportBuf[0u]);

        USBD_HID_Wr(class_nbr,
                    &App_USBD_HID_ReportBuf[0],
                    APP_USBD_HID_REPORT_LEN,
                    0u,
                    &err);
        if (err != USBD_ERR_NONE)
            printf("%s write hid ret %d\n", __func__, err);

        osDelay(100u);
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          USB HID CALLBACKS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                   App_USBD_HID_FeatureReportGet()
*
* Description : Get HID feature report corresponding to report ID.
*
* Argument(s) : class_nbr       Class instance number.
*
*               report_id       Report ID.
*
*               p_report_buf    Pointer to feature report buffer.
*
*               report_len      Length of report.
*
* Return(s)   : DEF_OK,         if NO error(s) occurred and report ID is supported.
*
*               DEF_FAIL,       otherwise.
*
* Note(s)     : (1) Report ID must not be written into the feature report buffer.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  App_USBD_HID_FeatureReportGet (CPU_INT08U   class_nbr,
                                                    CPU_INT08U   report_id,
                                                    CPU_INT08U  *p_report_buf,
                                                    CPU_INT16U   report_len)
{
    (void)class_nbr;
    (void)report_id;

    Mem_Clr(p_report_buf, report_len);

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                   App_USBD_HID_FeatureReportSet()
*
* Description : Set HID feature report corresponding to report ID.
*
* Argument(s) : class_nbr       Class instance number.
*
*               report_id       Report ID.
*
*               p_report_buf    Pointer to feature report buffer.
*
*               report_len      Length of report.
*
* Return(s)   : DEF_OK,         if NO error(s) occurred and report ID is supported.
*
*               DEF_FAIL,       otherwise.
*
* Note(s)     : (1) Report ID is not present in the feature report buffer.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  App_USBD_HID_FeatureReportSet (CPU_INT08U   class_nbr,
                                                    CPU_INT08U   report_id,
                                                    CPU_INT08U  *p_report_buf,
                                                    CPU_INT16U   report_len)
{
    (void)class_nbr;
    (void)report_id;
    (void)p_report_buf;
    (void)report_len;

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                     App_USBD_HID_ProtocolGet()
*
* Description : Retrieve active protocol: BOOT or REPORT protocol.
*
* Argument(s) : class_nbr    Class instance number.
*
*               p_err        Pointer to variable that will receive the return error code from this function :
*
*                                USBD_ERR_NONE    Protocol successfully retrieved.
*
* Return(s)   : Protocol,    if NO error(s).
*
*               0,           otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT08U  App_USBD_HID_ProtocolGet (CPU_INT08U   class_nbr,
                                              USBD_ERR    *p_err)
{
    (void)class_nbr;

   *p_err = USBD_ERR_NONE;

    return (App_USBD_HID_Protocol);
}


/*
*********************************************************************************************************
*                                     App_USBD_HID_ProtocolSet()
*
* Description : Store active protocol: BOOT or REPORT protocol.
*
* Argument(s) : class_nbr    Class instance number.
*
*               protocol     Protocol.
*
*               p_err        Pointer to variable that will receive the return error code from this function :
*
*                                USBD_ERR_NONE    Protocol successfully set.
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_HID_ProtocolSet (CPU_INT08U   class_nbr,
                                        CPU_INT08U   protocol,
                                        USBD_ERR    *p_err)
{
    (void)class_nbr;

    App_USBD_HID_Protocol = protocol;

   *p_err = USBD_ERR_NONE;
}


/*
*********************************************************************************************************
*                                      App_USBD_HID_ReportSet()
*
* Description : Notification of a new set report received on control endpoint.
*
* Argument(s) : class_nbr       Class instance number.
*
*               report_id       Report ID.
*
*               p_report_buf    Pointer to report buffer.
*
*               report_len      Length of report.
*
* Return(s)   : None.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_HID_ReportSet (CPU_INT08U   class_nbr,
                                      CPU_INT08U   report_id,
                                      CPU_INT08U  *p_report_buf,
                                      CPU_INT16U   report_len)
{
    (void)class_nbr;
    (void)report_id;
    (void)p_report_buf;
    (void)report_len;
}


/*
*********************************************************************************************************
*                                    App_USBD_HID_MouseSetReport()
*
* Description : Populate buffer with mouse report.
*
* Argument(s) : p_buf    Pointer to buffer.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void  App_USBD_HID_MouseSetReport (CPU_INT08U  *p_buf)
{
    CPU_SR_ALLOC();

    p_buf[0] = 0;
    p_buf[1] = 0;

    CPU_CRITICAL_ENTER();
    p_buf[2] = (App_USBD_HID_X > 0) ? 50 : -50;
    p_buf[3] = (App_USBD_HID_Y > 0) ? 50 : -50;

    App_USBD_HID_X = !App_USBD_HID_X;
    App_USBD_HID_Y = !App_USBD_HID_Y;
    CPU_CRITICAL_EXIT();
}

