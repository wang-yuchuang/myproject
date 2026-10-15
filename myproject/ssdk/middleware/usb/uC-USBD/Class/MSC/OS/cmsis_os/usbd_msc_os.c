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
* Filename : usbd_msc_os.c
* Version  : V1.00.00
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  "../../usbd_msc.h"
#include  "../../usbd_msc_os.h"
#if (USBD_MSC_CFG_FS_REFRESH_TASK_EN == DEF_ENABLED)
#include  "../../Storage/uC-FS/V4/usbd_storage.h"
#endif
#include  <cmsis_os2.h>


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if (USBD_MSC_CFG_FS_REFRESH_TASK_EN == DEF_ENABLED)
#error  "USBD_MSC_CFG_FS_REFRESH_TASK_EN  is not supported yet."
#endif


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

static  osSemaphoreId_t   USBD_MSC_OS_TASK_SemTbl[USBD_MSC_CFG_MAX_NBR_DEV];

static  osSemaphoreId_t   USBD_MSC_OS_EnumSignal;


/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/

#define  mscd_os_err(...)       ssdk_printf(SSDK_CRIT, __VA_ARGS__)

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  USBD_MSC_OS_Task        (void  *p_arg);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           USBD_MSC_OS_Init()
*
* Description : Initialize MSC OS interface.
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

void  USBD_MSC_OS_Init (USBD_ERR  *p_err)
{
    CPU_INT08U      class_nbr;
    osThreadId_t    thread;
    osThreadAttr_t  attr = {
        .name = "USBD MSC Tsk",
        .stack_size = 4096,
        .priority = osPriorityAboveNormal3,
    };

                                                                /* Create sem for signal used for MSC comm.             */
    for (class_nbr = 0u; class_nbr < USBD_MSC_CFG_MAX_NBR_DEV; class_nbr++) {
        USBD_MSC_OS_TASK_SemTbl[class_nbr] = osSemaphoreNew(10000u, 0, NULL);
        if (!USBD_MSC_OS_TASK_SemTbl[class_nbr]) {
            *p_err = USBD_ERR_OS_SIGNAL_CREATE;
            return;
        }
    }
                                                                /* Create sem for signal used for MSC enum.             */
    USBD_MSC_OS_EnumSignal = osSemaphoreNew(10000u, 0, NULL);
    if (!USBD_MSC_OS_EnumSignal) {
        *p_err = USBD_ERR_OS_SIGNAL_CREATE;
        return;
    }

    thread = osThreadNew(USBD_MSC_OS_Task, NULL, &attr);
    if (!thread) {
        *p_err = USBD_ERR_OS_INIT_FAIL;
        return;
    }

    *p_err = USBD_ERR_NONE;
}


/*
*********************************************************************************************************
*                                         USBD_MSC_OS_Task()
*
* Description : OS-dependent shell task to process MSC task
*
* Argument(s) : p_arg       Pointer to task initialization argument (required by CMSIS OS2).
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  USBD_MSC_OS_Task (void  *p_arg)
{
    CPU_INT08U  class_nbr;


    class_nbr = (CPU_INT08U)(CPU_ADDR)p_arg;

    USBD_MSC_TaskHandler(class_nbr);
}


/*
*********************************************************************************************************
*                                          USBD_MSC_OS_CommSignalPost()
*
* Description : Post a semaphore used for MSC communication.
*
* Argument(s) : class_nbr   MSC instance class number
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE       OS signal     successfully posted.
*                               USBD_ERR_OS_FAIL    OS signal NOT successfully posted.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  USBD_MSC_OS_CommSignalPost (CPU_INT08U   class_nbr,
                                  USBD_ERR    *p_err)
{
    osStatus_t  ret;

    ret = osSemaphoreRelease(USBD_MSC_OS_TASK_SemTbl[class_nbr]);
    if (ret < 0) {
        mscd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
        *p_err = USBD_ERR_OS_FAIL;
    } else {
        *p_err = USBD_ERR_NONE;
    }
}


/*
*********************************************************************************************************
*                                          USBD_MSC_OS_CommSignalPend()
*
* Description : Wait on a semaphore to become available for MSC communication.
*
* Argument(s) : class_nbr   MSC instance class number
*
*               timeout     Timeout in milliseconds.
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

void  USBD_MSC_OS_CommSignalPend (CPU_INT08U   class_nbr,
                                  CPU_INT32U   timeout,
                                  USBD_ERR    *p_err)
{
    osStatus_t  ret;
    uint32_t    tmo;

    tmo = timeout ? timeout : osWaitForever;

    ret = osSemaphoreAcquire(USBD_MSC_OS_TASK_SemTbl[class_nbr], tmo);
    switch (ret) {
    case osOK:
        *p_err = USBD_ERR_NONE;
        break;

    case osErrorTimeout:
        *p_err = USBD_ERR_OS_TIMEOUT;
        break;

    default:
        *p_err = USBD_ERR_OS_FAIL;
        mscd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
        break;
    }
}


/*
*********************************************************************************************************
*                                         USBD_MSC_OS_CommSignalDel()
*
* Description : Delete a semaphore if no tasks are waiting on it for MSC communication.
*
* Argument(s) : class_nbr   MSC instance class number
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*                               USBD_ERR_NONE          The call was successful and the semaphore was destroyed
*                               USBD_ERR_OS_FAIL       otherwise.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  USBD_MSC_OS_CommSignalDel (CPU_INT08U   class_nbr,
                                 USBD_ERR    *p_err)
{
    osStatus_t  ret;

    ret = osSemaphoreDelete(USBD_MSC_OS_TASK_SemTbl[class_nbr]);
    if (ret < 0) {
        *p_err = USBD_ERR_OS_FAIL;
        mscd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
    } else {
        *p_err = USBD_ERR_NONE;
    }
}


/*
*********************************************************************************************************
*                                          USBD_MSC_OS_EnumSignalPost()
*
* Description : Post a semaphore for MSC enumeration process.
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

void  USBD_MSC_OS_EnumSignalPost (USBD_ERR  *p_err)
{
    osStatus_t  ret;

    ret = osSemaphoreRelease(USBD_MSC_OS_EnumSignal);
    if (ret < 0) {
        mscd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
        *p_err = USBD_ERR_OS_FAIL;
    } else {
        *p_err = USBD_ERR_NONE;
    }
}


/*
*********************************************************************************************************
*                                       USBD_MSC_OS_EnumSignalPend()
*
* Description : Wait on a semaphore to become available for MSC enumeration process.
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

void  USBD_MSC_OS_EnumSignalPend (CPU_INT32U  timeout,
                                  USBD_ERR   *p_err)
{
    osStatus_t  ret;
    uint32_t tmo;

    tmo = timeout ? timeout : osWaitForever;

    ret = osSemaphoreAcquire(USBD_MSC_OS_EnumSignal, tmo);
    switch (ret) {
    case osOK:
        *p_err = USBD_ERR_NONE;
        break;

    case osErrorTimeout:
        *p_err = USBD_ERR_OS_TIMEOUT;
        break;

    default:
        *p_err = USBD_ERR_OS_FAIL;
        mscd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
        break;
    }
}


