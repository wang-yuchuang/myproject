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
* Filename : usbd_fb.h
* Version  : V1.00.00
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  USBD_FB_MODULE_PRESENT
#define  USBD_FB_MODULE_PRESENT


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  "../../Source/usbd_core.h"


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                                DEFINES
*********************************************************************************************************
*/

#define MAX_RSP_SIZE            64u


/*
**********************************************************************************************************
*                                             DATA TYPES
**********************************************************************************************************
*/

typedef void (*USBD_FB_CMD_HDL)(unsigned char *fb_cls, const char *arg, void *data, unsigned int sz);

typedef struct usbd_fb_cmd {
    const char      *cmd;
    USBD_FB_CMD_HDL handler;
} USBD_FB_CMD;

typedef struct usbd_fb_var {
    const char  *name;
    const char  *value;
} USBD_FB_VAR;

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MACROS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/


void         USBD_FB_Init       (   USBD_ERR        *p_err);

CPU_INT08U   USBD_FB_Add        (   USBD_ERR        *p_err);

CPU_BOOLEAN  USBD_FB_CfgAdd     (   CPU_INT08U      class_nbr,
                                    CPU_INT08U      dev_nbr,
                                    CPU_INT08U      cfg_nbr,
                                    USBD_ERR        *p_err);

CPU_BOOLEAN  USBD_FB_SetDL      (   CPU_INT08U      class_nbr,
                                    void            *download_addr,
                                    CPU_INT32U      download_max,
                                    USBD_ERR        *p_err);

CPU_BOOLEAN  USBD_FB_SetCmdTbl  (   CPU_INT08U      class_nbr,
                            const   USBD_FB_CMD     *cmd_tbl,
                                    CPU_INT16U      cmd_len,
                                    USBD_ERR        *p_err);

CPU_BOOLEAN  USBD_FB_SetVarTbl  (   CPU_INT08U      class_nbr,
                            const   USBD_FB_VAR     *var_tbl,
                                    CPU_INT16U      var_len,
                                    USBD_ERR        *p_err);

CPU_BOOLEAN  USBD_FB_IsConn     (   CPU_INT08U      class_nbr);

void         USBD_FB_TaskHandler(   CPU_INT08U      class_nbr);

void USBD_FB_Ack                (   unsigned char   *fb_cls,
                                    const char      *code,
                                    const char      *reason);

void USBD_FB_Info               (   unsigned char   *fb_cls,
                                    const char      *reason);

void USBD_FB_Fail               (   unsigned char   *fb_cls,
                                    const char      *reason);

void USBD_FB_Okay               (   unsigned char   *fb_cls,
                                    const char      *info);

void USBD_FB_CmdGetVar          (   unsigned char   *fb_cls,
                                    const char      *arg,
                                    void            *data,
                                    unsigned        sz);

void USBD_FB_CmdFlash           (   unsigned char   *fb_cls,
                                    const char      *arg,
                                    void            *data,
                                    unsigned        sz);

void USBD_FB_CmdDownload        (   unsigned char   *fb_cls,
                                    const char      *arg,
                                    void            *data,
                                    unsigned        sz);


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  USBD_FB_CFG_MAX_NBR_DEV
#error  "USBD_FB_CFG_MAX_NBR_DEV not #define'd in 'usbd_cfg.h' [MUST be >= 1]"
#endif

#if     (USBD_FB_CFG_MAX_NBR_DEV < 1u)
#error  "USBD_FB_CFG_MAX_NBR_DEV illegally #define'd in 'usbd_cfg.h' [MUST be >= 1]"
#endif

#ifndef  USBD_FB_CFG_MAX_NBR_CFG
#error  "USBD_FB_CFG_MAX_NBR_CFG not #define'd in 'usbd_cfg.h' [MUST be >= 1]"
#endif

#if     (USBD_FB_CFG_MAX_NBR_CFG < 1u)
#error  "USBD_FB_CFG_MAX_NBR_CFG illegally #define'd in 'usbd_cfg.h' [MUST be >= 1]"
#endif


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif
