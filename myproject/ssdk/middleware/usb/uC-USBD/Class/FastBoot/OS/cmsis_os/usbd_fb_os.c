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
*                                   USB DEVICE OPERATING SYSTEM LAYER
*                                          For CMSIS OS2
*
* Filename : usbd_fb_os.c
* Version  : V1.00.00
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  "../../usbd_fb.h"
#include  "../../usbd_fb_os.h"
#include  <cmsis_os2.h>


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


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

static  osSemaphoreId_t   USBD_FB_OS_EnumSignal;


/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/

#define  fbd_os_err(...)        ssdk_printf(SSDK_CRIT, __VA_ARGS__)

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  USBD_FB_OS_Task        (void  *p_arg);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           USBD_FB_OS_Init()
*
* Description : Initialize FastBoot OS interface.
*
* Argument(s) : p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE       OS initialization successful.
*                               USBD_ERR_OS_FAIL    OS objects NOT successfully initialized.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  USBD_FB_OS_Init (USBD_ERR  *p_err)
{
    osThreadId_t    thread;
    osThreadAttr_t  attr = {
        .name = "USBD FB Tsk",
        .stack_size = 4096,
        .priority = osPriorityAboveNormal3,
    };

    USBD_FB_OS_EnumSignal = osSemaphoreNew(10000u, 0, NULL);
    if (!USBD_FB_OS_EnumSignal) {
        *p_err = USBD_ERR_OS_SIGNAL_CREATE;
        return;
    }

    thread = osThreadNew(USBD_FB_OS_Task, NULL, &attr);
    if (!thread) {
        *p_err = USBD_ERR_OS_INIT_FAIL;
        return;
    }

    *p_err = USBD_ERR_NONE;
}


/*
*********************************************************************************************************
*                                         USBD_FB_OS_Task()
*
* Description : OS-dependent shell task to process FastBoot task
*
* Argument(s) : p_arg       Pointer to task initialization argument (required by CMSIS OS2).
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  USBD_FB_OS_Task (void  *p_arg)
{
    CPU_INT08U  class_nbr;


    class_nbr = (CPU_INT08U)(CPU_ADDR)p_arg;

    USBD_FB_TaskHandler(class_nbr);
}


/*
*********************************************************************************************************
*                                          USBD_FB_OS_EnumSignalPost()
*
* Description : Post a semaphore for FastBoot enumeration process.
*
* Argument(s) : p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE       OS signal     successfully posted.
*                               USBD_ERR_OS_FAIL    OS signal NOT successfully posted.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  USBD_FB_OS_EnumSignalPost (USBD_ERR  *p_err)
{
    osStatus_t  ret;

    ret = osSemaphoreRelease(USBD_FB_OS_EnumSignal);
    if (ret < 0) {
        fbd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
        *p_err = USBD_ERR_OS_FAIL;
    } else {
        *p_err = USBD_ERR_NONE;
    }
}


/*
*********************************************************************************************************
*                                       USBD_FB_OS_EnumSignalPend()
*
* Description : Wait on a semaphore to become available for FastBoot enumeration process.
*
* Argument(s) : timeout     Timeout in milliseconds.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*                               USBD_ERR_NONE          The call was successful and your task owns the resource
*                                                       or, the event you are waiting for occurred.
*                               USBD_ERR_OS_TIMEOUT    The semaphore was not received within the specified timeout.
*                               USBD_ERR_OS_FAIL       otherwise.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  USBD_FB_OS_EnumSignalPend (CPU_INT32U  timeout,
                                  USBD_ERR   *p_err)
{
    osStatus_t  ret;
    uint32_t tmo;

    tmo = timeout ? timeout : osWaitForever;

    ret = osSemaphoreAcquire(USBD_FB_OS_EnumSignal, tmo);
    switch (ret) {
    case osOK:
        *p_err = USBD_ERR_NONE;
        break;

    case osErrorTimeout:
        *p_err = USBD_ERR_OS_TIMEOUT;
        break;

    default:
        *p_err = USBD_ERR_OS_FAIL;
        fbd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
        break;
    }
}


