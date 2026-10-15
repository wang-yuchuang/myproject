/*
*********************************************************************************************************
*                                            uC/USB-Device
*                                    The Embedded USB Device Stack
*
*                               Copyright (c) 2021 Semidrive Semiconductor
*
*                                         All rights reserved.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        USB DEVICE FASTBOOT CLASS
*
* Filename : usbd_fb.c
* Version  : V1.00.00
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <errno.h>
#include  <stdlib.h>
#include  <arch/cache.h>
#include  "usbd_fb.h"
#include  "usbd_fb_os.h"
#include  <debug.h>


/*
*********************************************************************************************************
*                                     EXTERNAL C LANGUAGE LINKAGE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  USBD_FB_MSG_BUF_LEN                512u
#define  USBD_FB_VAR_INFO_LEN               128u

/* Since Win10 sends fastboot data 1MB by 1MB, let usb controller receive it correspondingly. */
#define  USBD_FB_RX_LIMIT                   (1024 * 1024u)


#define  USBD_FB_COM_NBR_MAX                (USBD_FB_CFG_MAX_NBR_DEV * \
                                             USBD_FB_CFG_MAX_NBR_CFG)


#define  USBD_FB_SUBCLASS_CODE              0x42u
#define  USBD_FB_PROTOCOL_CODE              3u


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
*                                        FORWARD DECLARATIONS
*********************************************************************************************************
*/

typedef struct usbd_fb_ctrl  USBD_FB_CTRL;


/*
*********************************************************************************************************
*                                           FASTBOOT STATES
*********************************************************************************************************
*/

typedef  enum  usbd_fb_state {
    USBD_FB_STATE_NONE = 0,
    USBD_FB_STATE_INIT,
    USBD_FB_STATE_CFG
} USBD_FB_STATE;

typedef  enum  usbd_fb_comm_state {
    USBD_FB_COMM_STATE_NONE = 0,
    USBD_FB_COMM_STATE_COMMAND,
    USBD_FB_COMM_STATE_COMPLETE,
    USBD_FB_COMM_STATE_ERROR,
} USBD_FB_COMM_STATE;


/*
**********************************************************************************************************
*                                   FASTBOOT EP REQUIREMENTS DATA TYPE
**********************************************************************************************************
*/

typedef struct usbd_fb_comm {                                   /* ---------- FB COMMUNICATION INFORMATION ----------- */
    USBD_FB_CTRL        *CtrlPtr;                               /* Ptr to ctrl information.                             */
    CPU_INT08U          DataBulkInEpAddr;
    CPU_INT08U          DataBulkOutEpAddr;
    USBD_FB_COMM_STATE  CommState;                              /* comm state of the FB device.                   */
} USBD_FB_COMM;


struct usbd_fb_ctrl {                                           /* ------------- FB CONTROL INFORMATION -------------- */
    CPU_INT08U          DevNbr;                                 /* FB dev nbr.                                         */
    USBD_FB_STATE       State;                                  /* FB dev state.                                       */
    USBD_FB_COMM        *CommPtr;                               /* FB comm info ptr.                                   */

    CPU_INT16U          CmdLen;
    CPU_INT16U          VarLen;
    const USBD_FB_CMD   *CmdArray;
    const USBD_FB_VAR   *VarArray;

    void                *DLAddr;
    CPU_INT32U          DLMax;
    CPU_INT32U          DLSize;

    char                *MSG_BufPtr;
};


/*
**********************************************************************************************************
*                                              LOCAL TABLES
**********************************************************************************************************
*/


/*
**********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
**********************************************************************************************************
*/
                                                                /* FB instance array.                                  */
static  USBD_FB_CTRL    USBD_FBCtrlTbl[USBD_FB_CFG_MAX_NBR_DEV];
static  CPU_INT08U      USBD_FBCtrlNbrNext;
                                                                /* FB comm array.                                      */
static  USBD_FB_COMM    USBD_FBCommTbl[USBD_FB_COM_NBR_MAX];
static  CPU_INT16U      USBD_FBCommNbrNext;

#if (USBD_CFG_MS_OS_DESC_EN == DEF_ENABLED)
static const  CPU_INT08U USBD_FBMsExtPropName[] = {
    'L', 0, 'a', 0, 'b', 0, 'e', 0, 'l', 0, 0, 0
};
static const  CPU_INT08U USBD_FBMsExtPropStr[] = {
    'F', 0, 'a', 0, 's', 0, 't', 0, 'B', 0, 'o', 0, 'o', 0, 't', 0,
    ' ', 0, 'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0,  0 , 0
};
static const USBD_MS_OS_EXT_PROPERTY USBD_FBMsExtProp[] = {
    {
        USBD_MS_OS_PROPERTY_TYPE_REG_SZ,
        USBD_FBMsExtPropName, sizeof(USBD_FBMsExtPropName),
        USBD_FBMsExtPropStr, sizeof(USBD_FBMsExtPropStr)
    }
};
#endif


/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/

#define  fbd_err(...)                   ssdk_printf(SSDK_CRIT,   "[FB]" __VA_ARGS__)
#define  fbd_log(...)                   ssdk_printf(SSDK_NOTICE, "[FB]" __VA_ARGS__)

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void    USBD_FB_Conn        (   CPU_INT08U          dev_nbr,
                                        CPU_INT08U          cfg_nbr,
                                        void               *p_if_class_arg);

static  void    USBD_FB_Disconn     (   CPU_INT08U          dev_nbr,
                                        CPU_INT08U          cfg_nbr,
                                        void               *p_if_class_arg);

#if (USBD_CFG_MS_OS_DESC_EN == DEF_ENABLED)
static  CPU_INT08U   USBD_FB_MS_GetCompatID      (  CPU_INT08U                 dev_nbr,
                                                    CPU_INT08U                *p_sub_compat_id_ix);

static  CPU_INT08U   USBD_FB_MS_GetExtPropertyTbl(  CPU_INT08U                 dev_nbr,
                                                    USBD_MS_OS_EXT_PROPERTY  **pp_ext_property_tbl);
#endif


/*
*********************************************************************************************************
*                                              FastBoot CLASS DRIVER
*********************************************************************************************************
*/

USBD_CLASS_DRV USBD_FB_Drv = {
    USBD_FB_Conn,
    USBD_FB_Disconn,
    DEF_NULL,
#if (USBD_CFG_MS_OS_DESC_EN == DEF_ENABLED)
    .MS_GetCompatID = USBD_FB_MS_GetCompatID,
    .MS_GetExtPropertyTbl = USBD_FB_MS_GetExtPropertyTbl,
#endif
};


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
**********************************************************************************************************
*                                               USBD_FB_Init()
*
* Description : Initialize internal structures and variables used by the Fast Boot Class
*               Bulk Only Transport.
*
* Argument(s) : p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE    FB class successfully initialized.
*
* Return(s)   : None.
*
* Note(s)     : None.
**********************************************************************************************************
*/

void  USBD_FB_Init (USBD_ERR  *p_err)
{
    CPU_INT08U      ix;
    USBD_FB_CTRL    *p_ctrl;
    USBD_FB_COMM    *p_comm;
    LIB_ERR         err_lib;


#if (USBD_CFG_ERR_ARG_CHK_EXT_EN == DEF_ENABLED)                /* ---------------- VALIDATE ARGUMENTS ---------------- */
    if (p_err == (USBD_ERR *)0) {                               /* Validate error ptr.                                  */
        CPU_SW_EXCEPTION(;);
    }
#endif

    for (ix = 0u; ix < USBD_FB_CFG_MAX_NBR_DEV; ix++) {        /* Init FB class struct.                               */
        p_ctrl              = &USBD_FBCtrlTbl[ix];
        p_ctrl->State       = USBD_FB_STATE_NONE;
        p_ctrl->CommPtr     = (USBD_FB_COMM *)0;

        p_ctrl->CmdLen      = 0;
        p_ctrl->VarLen      = 0;

        p_ctrl->DLAddr      = NULL;
        p_ctrl->DLMax       = 0;
        p_ctrl->DLSize      = 0;

        p_ctrl->MSG_BufPtr  = (char *)Mem_HeapAlloc(    USBD_FB_MSG_BUF_LEN,
                                                        CONFIG_ARCH_CACHE_LINE,
                                                        NULL,
                                                        &err_lib);
        if (err_lib != LIB_MEM_ERR_NONE) {
            *p_err = USBD_ERR_ALLOC;
            return;
        }
    }

    for (ix = 0u; ix < USBD_FB_COM_NBR_MAX; ix++) {            /* Init FB class EP tbl.                              */
        p_comm                      = &USBD_FBCommTbl[ix];
        p_comm->DataBulkInEpAddr    = USBD_EP_ADDR_NONE;
        p_comm->DataBulkOutEpAddr   = USBD_EP_ADDR_NONE;
        p_comm->CtrlPtr             = (USBD_FB_CTRL *)0;
    }

    USBD_FBCtrlNbrNext = 0u;
    USBD_FBCommNbrNext = 0u;

    USBD_FB_OS_Init(p_err);                                    /* Init FB OS layer.                                   */
    if (*p_err != USBD_ERR_NONE) {
        return;
    }
}


/*
*********************************************************************************************************
*                                          USBD_FB_Add()
*
* Description : Add a new instance of the Fast Boot Class.
*
* Argument(s) : p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE    FB class instance successfully added.
*
*                               USBD_ERR_ALLOC   No more class instance structure available.
*
* Return(s)   : Class instance number, if NO error(s).
*
*               USBD_CLASS_NBR_NONE,   otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT08U  USBD_FB_Add (USBD_ERR  *p_err)
{
    CPU_INT08U  fb_nbr;
    CPU_SR_ALLOC();


#if (USBD_CFG_ERR_ARG_CHK_EXT_EN == DEF_ENABLED)                /* ---------------- VALIDATE ARGUMENTS ---------------- */
    if (p_err == (USBD_ERR *)0) {                               /* Validate error ptr.                                  */
        CPU_SW_EXCEPTION(0);
    }
#endif

    CPU_CRITICAL_ENTER();
    fb_nbr = USBD_FBCtrlNbrNext;                              /* Alloc new FB class.                                */

    if (fb_nbr >= USBD_FB_CFG_MAX_NBR_DEV) {
        CPU_CRITICAL_EXIT();
       *p_err = USBD_ERR_FB_INSTANCE_ALLOC;
        return (USBD_CLASS_NBR_NONE);
    }

    USBD_FBCtrlNbrNext++;
    CPU_CRITICAL_EXIT();

    *p_err = USBD_ERR_NONE;
    return (fb_nbr);
}


/*
*********************************************************************************************************
*                                           USBD_FB_CfgAdd()
*
* Description : Add an existing FB instance to the specified configuration and device.
*
* Argument(s) : class_nbr   FB instance number.
*
*               dev_nbr     Device number.
*
*               cfg_nbr     Configuration index to add existing FB interface to.
*
*               p_err       Pointer to variable that will receive the return error code from this function:

*                               USBD_ERR_NONE                   FB class instance successfully added.
*                               USBD_ERR_INVALID_ARG            Invalid argument(s) passed to 'class_nbr'/
*                                                                   'dev_nbr'/'intr_en'/'interval'.
*                               USBD_ERR_ALLOC                  No more class communication structure available.

*                                                               ---------- RETURNED BY USBD_IF_Add() : ----------
*                               USBD_ERR_INVALID_ARG            Invalid argument(s) passed to 'class_desc'/
*                                                                   'class_desc_size'.
*                               USBD_ERR_NULL_PTR               Argument 'p_class_drv'/'p_class_drv->Conn'/
*                                                                   'p_class_drv->Disconn' passed a NULL
*                                                               pointer.
*                               USBD_ERR_DEV_INVALID_NBR        Invalid device number.
*                               USBD_ERR_DEV_INVALID_STATE      Invalid device state.
*                               USBD_ERR_CFG_INVALID_NBR        Invalid configuration number.
*                               USBD_ERR_IF_ALLOC               Interfaces                   NOT available.
*                               USBD_ERR_IF_ALT_ALLOC           Interface alternate settings NOT available.
*
*                                                               ---------- RETURNED BY USBD_BulkAdd() : ----------
*                               USBD_ERR_INVALID_ARG            Invalid argument(s) passed to 'max_pkt_len'.
*
*                               USBD_ERR_DEV_INVALID_NBR        Invalid device        number.
*                               USBD_ERR_CFG_INVALID_NBR        Invalid configuration number.
*                               USBD_ERR_IF_INVALID_NBR         Invalid interface     number.
*                               USBD_ERR_EP_NONE_AVAIL          Physical endpoint NOT available.
*                               USBD_ERR_EP_ALLOC               Endpoints NOT available.
*
* Return(s)   : DEF_YES, if FB instance added to USB device configuration successfully.
*
*               DEF_NO,  otherwise
*
* Note(s)     : (1) USBD_FB_CfgAdd() basically adds an Interface descriptor and its associated Endpoint
*                   descriptor(s) to the Configuration descriptor. One call to USBD_FB_CfgAdd() builds
*                   the Configuration descriptor corresponding to a FB device with the following format:
*
*                   Configuration Descriptor
*                   |-- Interface Descriptor (FB)
*                   |-- Endpoint Descriptor (Bulk OUT)
*                   |-- Endpoint Descriptor (Bulk IN)
*
*                   If USBD_FB_CfgAdd() is called several times from the application, it allows to create
*                   multiple instances and multiple configurations.
*********************************************************************************************************
*/

CPU_BOOLEAN  USBD_FB_CfgAdd(CPU_INT08U   class_nbr,
                            CPU_INT08U   dev_nbr,
                            CPU_INT08U   cfg_nbr,
                            USBD_ERR    *p_err)
{
    USBD_FB_CTRL    *p_ctrl;
    USBD_FB_COMM    *p_comm;
    USBD_DEV_CFG    *p_dev_cfg;
    CPU_INT08U      if_nbr;
    CPU_INT08U      ep_addr;
    CPU_INT16U      comm_nbr;
    CPU_SR_ALLOC();


#if (USBD_CFG_ERR_ARG_CHK_EXT_EN == DEF_ENABLED)                /* ---------------- VALIDATE ARGUMENTS ---------------- */
    if (p_err == (USBD_ERR *)0) {                               /* Validate error ptr.                                  */
        CPU_SW_EXCEPTION(DEF_NO);
    }
#endif

    if (class_nbr >= USBD_FBCtrlNbrNext) {
       *p_err = USBD_ERR_CLASS_INVALID_NBR;
        return (DEF_NO);
    }

    p_dev_cfg = USBD_DevCfgGet(dev_nbr, p_err);
    if (*p_err != USBD_ERR_NONE) {
        return (DEF_NO);
    }

    if (p_dev_cfg->SerialNbrStrPtr == (CPU_CHAR *)0) {
       *p_err = USBD_ERR_INVALID_ARG;
        return (DEF_NO);
    }

    p_ctrl = &USBD_FBCtrlTbl[class_nbr];

    CPU_CRITICAL_ENTER();
    p_ctrl->DevNbr = dev_nbr;

    comm_nbr = USBD_FBCommNbrNext;
    if (comm_nbr >= USBD_FB_COM_NBR_MAX) {
        USBD_FBCtrlNbrNext--;
        CPU_CRITICAL_EXIT();
        *p_err = USBD_ERR_FB_INSTANCE_ALLOC;
        return (DEF_NO);
    }

    USBD_FBCommNbrNext++;
    CPU_CRITICAL_EXIT();

    p_comm = &USBD_FBCommTbl[comm_nbr];

    if_nbr = USBD_IF_Add (        dev_nbr,                      /* Add FB IF desc to cfg desc.                         */
                                  cfg_nbr,
                                 &USBD_FB_Drv,
                          (void *)p_comm,
                          (void *)0,
                                  USBD_CLASS_CODE_VENDOR_SPECIFIC,
                                  USBD_FB_SUBCLASS_CODE,
                                  USBD_FB_PROTOCOL_CODE,
                                  "USB Fast Boot Interface",
                                  p_err);
    if (*p_err != USBD_ERR_NONE) {
        return (DEF_NO);
    }

    ep_addr = USBD_BulkAdd (      dev_nbr,                     /* Add bulk-IN EP desc.                                 */
                                  cfg_nbr,
                                  if_nbr,
                                  0u,
                                  DEF_YES,
                                  0u,
                                  p_err);
    if (*p_err != USBD_ERR_NONE) {
        return (DEF_NO);
    }

    p_comm->DataBulkInEpAddr = ep_addr;                         /* Store bulk-IN EP address.                            */

    ep_addr = USBD_BulkAdd (      dev_nbr,                      /* Add bulk-OUT EP desc.                                */
                                  cfg_nbr,
                                  if_nbr,
                                  0u,
                                  DEF_NO,
                                  0u,
                                  p_err);
    if (*p_err != USBD_ERR_NONE) {
        return (DEF_NO);
    }

    p_comm->DataBulkOutEpAddr = ep_addr;                        /* Store bulk-OUT EP address.                           */

    CPU_CRITICAL_ENTER();
    p_ctrl->State   =  USBD_FB_STATE_INIT;                     /* Set class instance to init state.                    */
    p_ctrl->DevNbr  =  dev_nbr;
    p_ctrl->CommPtr = (USBD_FB_COMM *)0;
    CPU_CRITICAL_EXIT();

    p_comm->CtrlPtr = p_ctrl;
    *p_err          = USBD_ERR_NONE;

    return (DEF_YES);
}

CPU_BOOLEAN  USBD_FB_SetDL( CPU_INT08U  class_nbr,
                            void        *download_addr,
                            CPU_INT32U  download_max,
                            USBD_ERR    *p_err)
{
    USBD_FB_CTRL  *p_ctrl;

#if (USBD_CFG_ERR_ARG_CHK_EXT_EN == DEF_ENABLED)
    if (!p_err) {
        CPU_SW_EXCEPTION(DEF_NO);
    }
#endif

    if (!download_addr || !download_max) {
        *p_err = USBD_ERR_INVALID_ARG;
        return (DEF_NO);
    }

    if (class_nbr >= USBD_FBCtrlNbrNext) {
        *p_err = USBD_ERR_CLASS_INVALID_NBR;
        return (DEF_NO);
    }

    p_ctrl = &USBD_FBCtrlTbl[class_nbr];

    p_ctrl->DLAddr = download_addr;
    p_ctrl->DLMax = download_max;

    *p_err = USBD_ERR_NONE;
    return (DEF_YES);
}

CPU_BOOLEAN  USBD_FB_SetCmdTbl( CPU_INT08U          class_nbr,
                                const USBD_FB_CMD   *cmd_tbl,
                                CPU_INT16U          cmd_len,
                                USBD_ERR            *p_err)
{
    CPU_INT16U      ix;
    USBD_FB_CTRL    *p_ctrl;
    CPU_SR_ALLOC();

#if (USBD_CFG_ERR_ARG_CHK_EXT_EN == DEF_ENABLED)
    if (!p_err) {
        CPU_SW_EXCEPTION(DEF_NO);
    }
#endif

    if (!cmd_tbl || !cmd_len) {
        *p_err = USBD_ERR_INVALID_ARG;
        return (DEF_NO);
    }

    for (ix = 0; ix < cmd_len; ix++) {
        if (!cmd_tbl[ix].cmd || !*cmd_tbl[ix].cmd || !cmd_tbl[ix].handler) {
            *p_err = USBD_ERR_INVALID_ARG;
            return (DEF_NO);
        }
    }

    if (class_nbr >= USBD_FBCtrlNbrNext) {
        *p_err = USBD_ERR_CLASS_INVALID_NBR;
        return (DEF_NO);
    }

    p_ctrl = &USBD_FBCtrlTbl[class_nbr];

    CPU_CRITICAL_ENTER();
    p_ctrl->CmdLen = cmd_len;
    p_ctrl->CmdArray = cmd_tbl;
    CPU_CRITICAL_EXIT();

    *p_err = USBD_ERR_NONE;
    return (DEF_YES);
}

CPU_BOOLEAN  USBD_FB_SetVarTbl( CPU_INT08U          class_nbr,
                                const USBD_FB_VAR   *var_tbl,
                                CPU_INT16U          var_len,
                                USBD_ERR            *p_err)
{
    CPU_INT16U      ix;
    USBD_FB_CTRL    *p_ctrl;
    CPU_SR_ALLOC();

#if (USBD_CFG_ERR_ARG_CHK_EXT_EN == DEF_ENABLED)
    if (!p_err) {
        CPU_SW_EXCEPTION(DEF_NO);
    }
#endif

    if (!var_tbl || !var_len) {
        *p_err = USBD_ERR_INVALID_ARG;
        return (DEF_NO);
    }

    for (ix = 0; ix < var_len; ix++) {
        if (!var_tbl[ix].name ||
            !*var_tbl[ix].name ||
            !var_tbl[ix].value ||
            !*var_tbl[ix].value) {
            *p_err = USBD_ERR_INVALID_ARG;
            return (DEF_NO);
        }
    }


    if (class_nbr >= USBD_FBCtrlNbrNext) {
        *p_err = USBD_ERR_CLASS_INVALID_NBR;
        return (DEF_NO);
    }

    p_ctrl = &USBD_FBCtrlTbl[class_nbr];

    CPU_CRITICAL_ENTER();
    p_ctrl->VarLen = var_len;
    p_ctrl->VarArray = var_tbl;
    CPU_CRITICAL_EXIT();

    *p_err = USBD_ERR_NONE;
    return (DEF_YES);
}


/*
*********************************************************************************************************
*                                          USBD_FB_IsConn()
*
* Description : Get the FB connection state of the device.
*
* Argument(s) : class_nbr         FB instance number.
*
* Return(s)   : DEF_YES, if FB class is connected.
*
*               DEF_NO,  otherwise.
*
* Note(s)     : None.
*********************************************************************************************************
*/

CPU_BOOLEAN  USBD_FB_IsConn (CPU_INT08U  class_nbr)
{
    USBD_FB_CTRL    *p_ctrl;
    USBD_DEV_STATE  state;
    USBD_ERR        err;


#if (USBD_CFG_ERR_ARG_CHK_EXT_EN == DEF_ENABLED)
    if (class_nbr >= USBD_FBCtrlNbrNext) {
        return (DEF_NO);
    }
#endif

    p_ctrl = &USBD_FBCtrlTbl[class_nbr];

    if (p_ctrl->CommPtr == (USBD_FB_COMM *)0) {
        return (DEF_NO);
    }

    state = USBD_DevStateGet(p_ctrl->DevNbr, &err);             /* Get dev state.                                       */

    if ((err           == USBD_ERR_NONE            ) &&         /* Return true if dev state is cfg & FB state is cfg.  */
        (state         == USBD_DEV_STATE_CONFIGURED) &&
        (p_ctrl->State == USBD_FB_STATE_CFG       )) {
        return (DEF_YES);
    } else {
        return (DEF_NO);
    }
}


/*
**********************************************************************************************************
*                                            USBD_FB_TaskHandler()
*
* Description : This function is used to handle FB transfers.
*
* Argument(s) : class_nbr   FB instance number.
*
* Return(s)   : none.
*
* Note(s)     : none.
**********************************************************************************************************
*/

void  USBD_FB_TaskHandler (CPU_INT08U  class_nbr)
{
    USBD_ERR        err;
    USBD_ERR        os_err;
    USBD_FB_CTRL    *p_ctrl;
    USBD_FB_COMM    *p_comm;
    CPU_BOOLEAN     conn;
    CPU_INT32U      xfer_len;
    CPU_DATA        ix;
    unsigned char   fb_cls_nbr;
    USBD_FB_CMD_HDL cmd_hdl;

    fb_cls_nbr = class_nbr;
    p_ctrl = &USBD_FBCtrlTbl[class_nbr];

    while (DEF_TRUE) {

        conn = USBD_FB_IsConn(class_nbr);
        if (conn != DEF_YES) {
            if (p_ctrl->CommPtr)
                p_ctrl->CommPtr->CommState = USBD_FB_COMM_STATE_NONE;
            fbd_log("fastboot: offline!\n");

            /* Wait till FB state and dev state is connected.      */
            USBD_FB_OS_EnumSignalPend((CPU_INT16U)0,
                                                  &os_err);
        }

        p_comm = p_ctrl->CommPtr;

        if (p_comm != (USBD_FB_COMM *)0) {
            xfer_len = USBD_BulkRx (p_ctrl->DevNbr,
                                    p_comm->DataBulkOutEpAddr,
                                    p_ctrl->MSG_BufPtr,
                                    USBD_FB_MSG_BUF_LEN,
                                    0,
                                    &err);
            if ((err != USBD_ERR_NONE) || (xfer_len == 0)) {
                fbd_err("get fastboot command err=%d\n", err);
                continue;
            }

            if (xfer_len == USBD_FB_MSG_BUF_LEN)
                p_ctrl->MSG_BufPtr[xfer_len - 1] = 0;
            else
                p_ctrl->MSG_BufPtr[xfer_len] = 0;

            fbd_log("fastboot cmd: %s\n", p_ctrl->MSG_BufPtr);

            p_comm->CommState = USBD_FB_COMM_STATE_COMMAND;
            cmd_hdl = NULL;
            for (ix = 0; ix < p_ctrl->CmdLen; ix++) {
                if (!memcmp(p_ctrl->MSG_BufPtr,
                    p_ctrl->CmdArray[ix].cmd,
                    strlen(p_ctrl->CmdArray[ix].cmd))) {
                    cmd_hdl = p_ctrl->CmdArray[ix].handler;
                    break;
                }
            }

            if (cmd_hdl) {
                cmd_hdl(&fb_cls_nbr,
                        p_ctrl->MSG_BufPtr + strlen(p_ctrl->CmdArray[ix].cmd),
                        p_ctrl->DLAddr,
                        p_ctrl->DLSize);

                if (p_comm->CommState == USBD_FB_COMM_STATE_COMMAND) {
                    USBD_FB_Fail(&fb_cls_nbr, "unknown reason");
                }
            } else {
                USBD_FB_Fail(&fb_cls_nbr, "unknown command");
            }
        }
    }
}


/*
**********************************************************************************************************
**********************************************************************************************************
*                                            LOCAL FUNCTIONS
**********************************************************************************************************
**********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         USBD_FB_Conn()
*
* Description : Notify class that configuration is active.
*
* Argument(s) : dev_nbr         Device number.
*
*               cfg_nbr         Configuration index to add the interface to.
*
*               p_if_class_arg  Pointer to class argument.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  USBD_FB_Conn (CPU_INT08U  dev_nbr,
                            CPU_INT08U  cfg_nbr,
                            void        *p_if_class_arg)
{
    USBD_FB_COMM    *p_comm;
    USBD_ERR        os_err;
    CPU_SR_ALLOC();


    (void)dev_nbr;
    (void)cfg_nbr;
    p_comm  = (USBD_FB_COMM *)p_if_class_arg;

    CPU_CRITICAL_ENTER();
    p_comm->CtrlPtr->CommPtr = p_comm;
    p_comm->CtrlPtr->State   = USBD_FB_STATE_CFG;              /* Set initial MSC state to cfg.                        */
    CPU_CRITICAL_EXIT();

    USBD_FB_OS_EnumSignalPost(&os_err);
    fbd_log("FB: Conn");
}


/*
*********************************************************************************************************
*                                       USBD_FB_Disconn()
*
* Description : Notify class that configuration is not active.
*
* Argument(s) : dev_nbr         Device number.
*
*               cfg_nbr         Configuration index to add the interface.
*
*               p_if_class_arg  Pointer to class argument.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  USBD_FB_Disconn ( CPU_INT08U  dev_nbr,
                                CPU_INT08U  cfg_nbr,
                                void        *p_if_class_arg)
{
    USBD_FB_COMM    *p_comm;
    CPU_SR_ALLOC();


    (void)dev_nbr;
    (void)cfg_nbr;
    p_comm  = (USBD_FB_COMM *)p_if_class_arg;

    CPU_CRITICAL_ENTER();
    p_comm->CtrlPtr->CommPtr = (USBD_FB_COMM *)0;
    p_comm->CtrlPtr->State   =  USBD_FB_STATE_INIT;            /* Set FB state to init.                               */
    p_comm->CommState        =  USBD_FB_COMM_STATE_NONE;       /* Set FB comm state to none.                          */
    CPU_CRITICAL_EXIT();

    fbd_log("FB: Disconn");
}

#if (USBD_CFG_MS_OS_DESC_EN == DEF_ENABLED)
/*
*********************************************************************************************************
*                                    USBD_FB_MS_GetCompatID()
*
* Description : Returns Microsoft descriptor compatible ID.
*
* Argument(s) : dev_nbr                 Device number.
*
*               p_sub_compat_id_ix      Pointer to variable that will receive subcompatible ID.
*
* Return(s)   : Compatible ID.
*
*               DEF_FAIL, otherwise.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static  CPU_INT08U  USBD_FB_MS_GetCompatID (CPU_INT08U   dev_nbr,
                                            CPU_INT08U  *p_sub_compat_id_ix)
{
     (void)dev_nbr;

    *p_sub_compat_id_ix = USBD_MS_OS_SUBCOMPAT_ID_NULL;

     return (USBD_MS_OS_COMPAT_ID_WINUSB);
}

/*
*********************************************************************************************************
*                                USBD_FB_MS_GetExtPropertyTbl()
*
* Description : Returns Microsoft descriptor extended properties table.
*
* Argument(s) : dev_nbr                 Device number.
*
*               pp_ext_property_tbl     Pointer to variable that will receive the Microsoft extended
*                                       properties table.
*
* Return(s)   : Number of Microsoft extended properties in table.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static  CPU_INT08U  USBD_FB_MS_GetExtPropertyTbl(CPU_INT08U                 dev_nbr,
                                                 USBD_MS_OS_EXT_PROPERTY  **pp_ext_property_tbl)
{
    *pp_ext_property_tbl =  USBD_FBMsExtProp;

    return sizeof(USBD_FBMsExtProp) / sizeof(USBD_FBMsExtProp[0]);
}
#endif


void USBD_FB_Ack(   unsigned char   *fb_cls,
                    const char      *code,
                    const char      *reason)
{
    USBD_FB_CTRL    *p_ctrl;
    USBD_FB_COMM    *p_comm;
    int             str_len;
    USBD_ERR        err;

    if (!fb_cls || (*fb_cls >=  USBD_FBCtrlNbrNext)) {
        return;
    }

    p_ctrl = &USBD_FBCtrlTbl[*fb_cls];
    p_comm = p_ctrl->CommPtr;

    if (!p_comm || (p_comm->CommState != USBD_FB_COMM_STATE_COMMAND)) {
        return;
    }

    if (reason == 0) {
        reason = "";
    }

    str_len = snprintf((char *)p_ctrl->MSG_BufPtr, MAX_RSP_SIZE, "%s%s", code, reason);
    if (str_len >= MAX_RSP_SIZE)
        str_len = MAX_RSP_SIZE - 1;
    p_comm->CommState = USBD_FB_COMM_STATE_COMPLETE;

    (void)USBD_BulkTx(p_ctrl->DevNbr,
                      p_comm->DataBulkInEpAddr,
                      p_ctrl->MSG_BufPtr,
                      str_len,
                      0,
                      DEF_NO,
                      &err);
    if (err != USBD_ERR_NONE)
        p_comm->CommState = USBD_FB_COMM_STATE_ERROR;
}

void USBD_FB_Info(unsigned char *fb_cls, const char *reason)
{
    USBD_FB_CTRL    *p_ctrl;
    USBD_FB_COMM    *p_comm;
    int             str_len;
    USBD_ERR        err;

    if (!fb_cls || (*fb_cls >=  USBD_FBCtrlNbrNext)) {
        return;
    }

    p_ctrl = &USBD_FBCtrlTbl[*fb_cls];
    p_comm = p_ctrl->CommPtr;

    if (!reason || !p_comm || (p_comm->CommState != USBD_FB_COMM_STATE_COMMAND)) {
        return;
    }

    str_len = snprintf((char *)p_ctrl->MSG_BufPtr, MAX_RSP_SIZE, "INFO%s", reason);
    if (str_len >= MAX_RSP_SIZE)
        str_len = MAX_RSP_SIZE - 1;

    (void)USBD_BulkTx(p_ctrl->DevNbr,
                      p_comm->DataBulkInEpAddr,
                      p_ctrl->MSG_BufPtr,
                      str_len,
                      0,
                      DEF_NO,
                      &err);
    if (err != USBD_ERR_NONE)
        p_comm->CommState = USBD_FB_COMM_STATE_ERROR;
}

void USBD_FB_Fail(unsigned char *fb_cls, const char *reason)
{
    USBD_FB_Ack(fb_cls, "FAIL", reason);
}

void USBD_FB_Okay(unsigned char *fb_cls, const char *info)
{
    USBD_FB_Ack(fb_cls, "OKAY", info);
}

static void USBD_FB_GetAllVar(unsigned char *fb_cls, USBD_FB_CTRL *p_ctrl)
{
    CPU_DATA    ix;
    char        getvar_all[USBD_FB_VAR_INFO_LEN];

    for (ix = 0; ix < p_ctrl->VarLen; ix++) {
        snprintf(   getvar_all,
                    USBD_FB_VAR_INFO_LEN,
                    "%s:%s",
                    p_ctrl->VarArray[ix].name,
                    p_ctrl->VarArray[ix].value);
        USBD_FB_Info(fb_cls, getvar_all);
    }

    USBD_FB_Okay(fb_cls, "");
}

void USBD_FB_CmdGetVar( unsigned char   *fb_cls,
                        const char      *arg,
                        void            *data,
                        unsigned        sz)
{
    USBD_FB_CTRL    *p_ctrl;
    CPU_DATA        ix;

    if (!fb_cls || (*fb_cls >=  USBD_FBCtrlNbrNext)) {
        return;
    }

    p_ctrl = &USBD_FBCtrlTbl[*fb_cls];

    if (!strcmp("all", arg)) {
        USBD_FB_GetAllVar(fb_cls, p_ctrl);
        return;
    }

    for (ix = 0; ix < p_ctrl->VarLen; ix++) {
        if (!strcmp(p_ctrl->VarArray[ix].name, arg)) {
            USBD_FB_Okay(fb_cls, p_ctrl->VarArray[ix].value);
            return;
        }
    }

    USBD_FB_Okay(fb_cls, "");
}

void USBD_FB_CmdFlash(  unsigned char   *fb_cls,
                        const char      *arg,
                        void            *data,
                        unsigned        sz)
{
    fbd_log("Flash%s\n", arg);
    USBD_FB_Okay(fb_cls, "");
}

static CPU_INT32U USBD_FB_ReadLarge(CPU_INT08U  dev_nbr,
                                    CPU_INT08U  ep_addr,
                                    void        *p_buf,
                                    CPU_INT32U  buf_len,
                                    USBD_ERR    *p_err)
{
    CPU_INT32U  usbd_rd_limit;
    CPU_INT32U  rd_len;
    CPU_INT32U  ret_len;
    CPU_INT32U  already_rd;

    usbd_rd_limit = USBD_FB_RX_LIMIT;

    already_rd = 0;
    while (buf_len > 0) {
        rd_len = (buf_len > usbd_rd_limit) ? usbd_rd_limit : buf_len;
        ret_len = USBD_BulkRx ( dev_nbr,
                                ep_addr,
                                p_buf,
                                rd_len,
                                0,
                                p_err);
        if ((*p_err != USBD_ERR_NONE) || (ret_len != rd_len)) {
            fbd_err("read large data err=%d\n", *p_err);
            return already_rd;
        }

        p_buf = (uint8_t *)p_buf + rd_len;
        buf_len -= rd_len;
        already_rd += rd_len;
    }

    *p_err = USBD_ERR_NONE;

    return already_rd;
}

void USBD_FB_CmdDownload(   unsigned char   *fb_cls,
                            const char      *arg,
                            void            *data,
                            unsigned        sz)
{
    char            *tmp;
    unsigned long   len;
    int             str_len;
    CPU_INT32U      rd_len;
    USBD_FB_CTRL    *p_ctrl;
    USBD_FB_COMM    *p_comm;
    USBD_ERR        err;

    if (!fb_cls || (*fb_cls >=  USBD_FBCtrlNbrNext)) {
        return;
    }

    p_ctrl = &USBD_FBCtrlTbl[*fb_cls];
    p_comm = p_ctrl->CommPtr;

    if (!p_comm || (p_comm->CommState != USBD_FB_COMM_STATE_COMMAND)) {
        return;
    }

    p_ctrl->DLSize = 0;

    errno = 0;
    len = strtoul(arg, &tmp, 16);
    if (*tmp || ((len == ULONG_MAX) && errno) || !len) {
        USBD_FB_Fail(fb_cls, "data len incorrect");
        return;
    }

    if (len > p_ctrl->DLMax) {
        USBD_FB_Fail(fb_cls, "data too large");
        return;
    }

    str_len = snprintf((char *)p_ctrl->MSG_BufPtr, MAX_RSP_SIZE, "DATA%08x", (CPU_INT32U)len);

    (void)USBD_BulkTx(p_ctrl->DevNbr,
                      p_comm->DataBulkInEpAddr,
                      p_ctrl->MSG_BufPtr,
                      str_len,
                      0,
                      DEF_NO,
                      &err);
    if (err != USBD_ERR_NONE){
        p_comm->CommState = USBD_FB_COMM_STATE_ERROR;
        fbd_err("download tx data err=%d\n", err);
        return;
    }

    rd_len = USBD_FB_ReadLarge( p_ctrl->DevNbr,
                                p_comm->DataBulkOutEpAddr,
                                p_ctrl->DLAddr,
                                len,
                                &err);
    if ((err != USBD_ERR_NONE) || (rd_len != len)) {
        fbd_err("read download data err=%d\n", err);
        p_comm->CommState = USBD_FB_COMM_STATE_ERROR;
        return;
    }

    p_ctrl->DLSize = len;
    USBD_FB_Okay(fb_cls, "");
}


