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
*                                USB HID CLASS OPERATING SYSTEM LAYER
*                                          For CMSIS OS2
*
* Filename : usbd_hid_os.c
* Version  : V1.00.00
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  "../../../../Source/usbd_core.h"
#include  "../../usbd_hid_report.h"
#include  "../../usbd_hid_os.h"
#include  <cmsis_os2.h>
#include  <driver/irq.h>
#include  <debug.h>


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
#define HID_DATA_EVENT_POST     (1u<<0)
#define HID_DATA_EVENT_ABORT    (1u<<1)
#define HID_DATA_EVENT_ALL      (HID_DATA_EVENT_POST | HID_DATA_EVENT_ABORT)

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

                                                                /* ---------------- USB HID SEM OBJECTS --------------- */
static  osEventFlagsId_t    USBD_HID_OS_InputDataSem_Tbl[USBD_HID_CFG_MAX_NBR_DEV];
static volatile CPU_INT32U  USBD_HID_OS_InDataSemCnt_Tbl[USBD_HID_CFG_MAX_NBR_DEV];

static  osSemaphoreId_t     USBD_HID_OS_InputLockSem_Tbl[USBD_HID_CFG_MAX_NBR_DEV];

static  osSemaphoreId_t     USBD_HID_OS_TxSem_Tbl[USBD_HID_CFG_MAX_NBR_DEV];

static  osSemaphoreId_t     USBD_HID_OS_OutputLockSem_Tbl[USBD_HID_CFG_MAX_NBR_DEV];

static  osEventFlagsId_t    USBD_HID_OS_OutputDataSem_Tbl[USBD_HID_CFG_MAX_NBR_DEV];
static volatile CPU_INT32U  USBD_HID_OS_OutDataSemCnt_Tbl[USBD_HID_CFG_MAX_NBR_DEV];

static  osThreadId_t        USBD_HID_OS_TmrTaskID;


/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/

#define  hidd_os_err(...)       ssdk_printf(SSDK_CRIT, __VA_ARGS__)

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  USBD_HID_OS_TmrTask(void  *p_arg);


/*
*********************************************************************************************************
*                                         USBD_HID_OS_Init()
*
* Description : Initialize HID OS interface.
*
* Argument(s) : p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE               HID OS initialization successful.
*                               USBD_ERR_OS_SIGNAL_CREATE   HID OS objects NOT successfully initialized.
*                               USBD_ERR_OS_INIT_FAIL       HID OS task    NOT successfully initialized.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_Init (USBD_ERR  *p_err)
{
    CPU_INT08U      class_nbr;
    osThreadId_t    thread;
    osThreadAttr_t  attr = {
        .name = "USBD HID Tmr Tsk",
        .stack_size = 4096,
        .priority = osPriorityAboveNormal3,
    };

    for (class_nbr = 0; class_nbr < USBD_HID_CFG_MAX_NBR_DEV; class_nbr++) {
        USBD_HID_OS_TxSem_Tbl[class_nbr] = osSemaphoreNew(1, 1, NULL);
        if (!USBD_HID_OS_TxSem_Tbl[class_nbr]) {
            *p_err = USBD_ERR_OS_SIGNAL_CREATE;
            return;
        }

        USBD_HID_OS_OutputLockSem_Tbl[class_nbr] = osSemaphoreNew(1, 1, NULL);
        if (!USBD_HID_OS_OutputLockSem_Tbl[class_nbr]) {
            *p_err = USBD_ERR_OS_SIGNAL_CREATE;
            return;
        }

        USBD_HID_OS_OutputDataSem_Tbl[class_nbr] = osEventFlagsNew(NULL);
        if (!USBD_HID_OS_OutputDataSem_Tbl[class_nbr]) {
            *p_err = USBD_ERR_OS_SIGNAL_CREATE;
            return;
        }
        USBD_HID_OS_OutDataSemCnt_Tbl[class_nbr] = 0;

        USBD_HID_OS_InputLockSem_Tbl[class_nbr] = osSemaphoreNew(1, 1, NULL);
        if (!USBD_HID_OS_InputLockSem_Tbl[class_nbr]) {
            *p_err = USBD_ERR_OS_SIGNAL_CREATE;
            return;
        }

        USBD_HID_OS_InputDataSem_Tbl[class_nbr] = osEventFlagsNew(NULL);
        if (!USBD_HID_OS_InputDataSem_Tbl[class_nbr]) {
            *p_err = USBD_ERR_OS_SIGNAL_CREATE;
            return;
        }
        USBD_HID_OS_InDataSemCnt_Tbl[class_nbr] = 0;
    }

    thread = osThreadNew(USBD_HID_OS_TmrTask, NULL, &attr);
    if (!thread) {
        *p_err = USBD_ERR_OS_INIT_FAIL;
        return;
    }
    USBD_HID_OS_TmrTaskID = thread;

    *p_err = USBD_ERR_NONE;
}


/*
*********************************************************************************************************
*                                        USBD_HID_OS_TmrTask()
*
* Description : OS-dependent shell task to process periodic HID input reports.
*
* Argument(s) : p_arg       Pointer to task initialization argument (required by CMSIS OS2).
*
* Return(s)   : none.
*
* Note(s)     : (1) Assumes OSCfg_TickRate_Hz frequency is greater than or equal to 250 Hz.  Otherwise,
*                   timer task scheduling rate will NOT be correct.
*
*               (2) Timer task MUST delay without failure.
*
*                   (a) Failure to delay timer task will prevent some HID report task(s)/operation(s) from
*                       functioning correctly.  Thus, timer task is assumed to be successfully delayed
*                       since NO error handling could be performed to counteract failure.
*********************************************************************************************************
*/

static  void  USBD_HID_OS_TmrTask (void  *p_arg)
{
    while (DEF_ON) {

        osDelay(4);

        USBD_HID_Report_TmrTaskHandler();
    }
}


/*
*********************************************************************************************************
*                                       USBD_HID_OS_InputLock()
*
* Description : Lock class input report.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE       Class input report successfully locked.
*                               USBD_ERR_OS_ABORT   Class input report aborted.
*                               USBD_ERR_OS_FAIL    OS signal not acquired because another error.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_InputLock (CPU_INT08U   class_nbr,
                             USBD_ERR    *p_err)
{
    osStatus_t  ret;

    ret = osSemaphoreAcquire(USBD_HID_OS_InputLockSem_Tbl[class_nbr], osWaitForever);
    switch (ret) {
    case osOK:
        *p_err = USBD_ERR_NONE;
        break;

    default:
        *p_err = USBD_ERR_OS_FAIL;
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
        break;
    }
}


/*
*********************************************************************************************************
*                                      USBD_HID_OS_InputUnlock()
*
* Description : Unlock class input report.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_InputUnlock (CPU_INT08U  class_nbr)
{
    osStatus_t  ret;

    ret = osSemaphoreRelease(USBD_HID_OS_InputLockSem_Tbl[class_nbr]);
    if (ret < 0)
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
}


/*
*********************************************************************************************************
*                                    USBD_HID_OS_OutputDataPendAbort()
*
* Description : Abort class output report.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_OutputDataPendAbort (CPU_INT08U  class_nbr)
{
    int32_t ret;

    ret = osEventFlagsSet(USBD_HID_OS_OutputDataSem_Tbl[class_nbr], HID_DATA_EVENT_ABORT);
    if (ret < 0)
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
}


/*
*********************************************************************************************************
*                                     USBD_HID_OS_OutputDataPend()
*
* Description : Wait for output report data transfer to complete.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
*               timeout_ms  Signal wait timeout, in milliseconds.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE       Class output successfully locked.
*                               USBD_ERR_OS_ABORT   Class output aborted.
*                               USBD_ERR_OS_FAIL    OS signal not acquired because another error.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_OutputDataPend (CPU_INT08U   class_nbr,
                                  CPU_INT16U   timeout_ms,
                                  USBD_ERR    *p_err)
{
    int32_t     ret;
    uint32_t    tmo;
    uint32_t    tk_start, tk_current;
    CPU_INT32U  cnt;
    CPU_SR_ALLOC();

    tk_start = osKernelGetTickCount();
    tk_current = tk_start;
    do {
        tmo = timeout_ms ? (tk_start + timeout_ms - tk_current): osWaitForever;
        ret = osEventFlagsWait(USBD_HID_OS_OutputDataSem_Tbl[class_nbr], HID_DATA_EVENT_ALL, 0, tmo);
        if (ret < 0) {
            switch (ret) {
            case osErrorTimeout:
                *p_err = USBD_ERR_OS_TIMEOUT;
                return;

            default:
                *p_err = USBD_ERR_OS_FAIL;
                hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
                return;
            }
        } else if (ret & HID_DATA_EVENT_ABORT) {
            *p_err = USBD_ERR_OS_ABORT;
            return;
        }

        CPU_CRITICAL_ENTER();
        cnt = USBD_HID_OS_OutDataSemCnt_Tbl[class_nbr];
        if (cnt)
            USBD_HID_OS_OutDataSemCnt_Tbl[class_nbr] = cnt - 1;
        CPU_CRITICAL_EXIT();

        if (cnt > 1) {
            osEventFlagsSet(USBD_HID_OS_OutputDataSem_Tbl[class_nbr], HID_DATA_EVENT_POST);
        }

        if (cnt) {
            *p_err = USBD_ERR_NONE;
            return;
        }

        tk_current = osKernelGetTickCount();
    } while (!timeout_ms || (tk_current - tk_start <= timeout_ms));

    *p_err = USBD_ERR_OS_TIMEOUT;
    return;
}


/*
*********************************************************************************************************
*                                     USBD_HID_OS_OutputDataPost()
*
* Description : Signal that output report data is available.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_OutputDataPost (CPU_INT08U  class_nbr)
{
    int32_t ret;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    USBD_HID_OS_OutDataSemCnt_Tbl[class_nbr]++;
    CPU_CRITICAL_EXIT();

    ret = osEventFlagsSet(USBD_HID_OS_OutputDataSem_Tbl[class_nbr], HID_DATA_EVENT_POST);
    if (ret < 0)
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
}


/*
*********************************************************************************************************
*                                      USBD_HID_OS_OutputLock()
*
* Description : Lock class output report.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE       Class output successfully locked.
*                               USBD_ERR_OS_ABORT   Class output aborted.
*                               USBD_ERR_OS_FAIL    OS signal not acquired because another error.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_OutputLock (CPU_INT08U   class_nbr,
                              USBD_ERR    *p_err)
{
    osStatus_t  ret;

    ret = osSemaphoreAcquire(USBD_HID_OS_OutputLockSem_Tbl[class_nbr], osWaitForever);
    switch (ret) {
    case osOK:
        *p_err = USBD_ERR_NONE;
        break;

    default:
        *p_err = USBD_ERR_OS_FAIL;
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
        break;
    }
}


/*
*********************************************************************************************************
*                                     USBD_HID_OS_OutputUnlock()
*
* Description : Unlock class output report.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_OutputUnlock (CPU_INT08U  class_nbr)
{
    osStatus_t  ret;

    ret = osSemaphoreRelease(USBD_HID_OS_OutputLockSem_Tbl[class_nbr]);
    if (ret < 0)
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
}


/*
*********************************************************************************************************
*                                        USBD_HID_OS_TxLock()
*
* Description : Lock class transmit.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE       Class feature report successfully locked.
*                               USBD_ERR_OS_ABORT   Class feature report aborted.
*                               USBD_ERR_OS_FAIL    OS signal not acquired because another error.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_TxLock (CPU_INT08U   class_nbr,
                          USBD_ERR    *p_err)
{
    osStatus_t  ret;

    ret = osSemaphoreAcquire(USBD_HID_OS_TxSem_Tbl[class_nbr], osWaitForever);
    switch (ret) {
    case osOK:
        *p_err = USBD_ERR_NONE;
        break;

    default:
        *p_err = USBD_ERR_OS_FAIL;
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
        break;
    }
}


/*
*********************************************************************************************************
*                                       USBD_HID_OS_TxUnlock()
*
* Description : Unlock class transmit.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_TxUnlock (CPU_INT08U  class_nbr)
{
    osStatus_t  ret;

    ret = osSemaphoreRelease(USBD_HID_OS_TxSem_Tbl[class_nbr]);
    if (ret < 0)
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
}


/*
*********************************************************************************************************
*                                     USBD_HID_OS_InputDataPend()
*
* Description : Wait for input report data transfer to complete.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
*               timeout_ms  Signal wait timeout in milliseconds.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE           OS signal     successfully acquired.
*                               USBD_ERR_OS_TIMEOUT     OS signal NOT successfully acquired in the time
*                                                           specified by 'timeout_ms'.
*                               USBD_ERR_OS_ABORT       OS signal aborted.
*                               USBD_ERR_OS_FAIL        OS signal not acquired because another error.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_InputDataPend (CPU_INT08U   class_nbr,
                                 CPU_INT16U   timeout_ms,
                                 USBD_ERR    *p_err)
{
    int32_t     ret;
    uint32_t    tmo;
    uint32_t    tk_start, tk_current;
    CPU_INT32U  cnt;
    CPU_SR_ALLOC();

    tk_start = osKernelGetTickCount();
    tk_current = tk_start;
    do {
        tmo = timeout_ms ? (tk_start + timeout_ms - tk_current): osWaitForever;
        ret = osEventFlagsWait(USBD_HID_OS_InputDataSem_Tbl[class_nbr], HID_DATA_EVENT_ALL, 0, tmo);
        if (ret < 0) {
            switch (ret) {
            case osErrorTimeout:
                *p_err = USBD_ERR_OS_TIMEOUT;
                return;

            default:
                *p_err = USBD_ERR_OS_FAIL;
                hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
                return;
            }
        } else if (ret & HID_DATA_EVENT_ABORT) {
            *p_err = USBD_ERR_OS_ABORT;
            return;
        }

        CPU_CRITICAL_ENTER();
        cnt = USBD_HID_OS_InDataSemCnt_Tbl[class_nbr];
        if (cnt)
            USBD_HID_OS_InDataSemCnt_Tbl[class_nbr] = cnt - 1;
        CPU_CRITICAL_EXIT();

        if (cnt > 1) {
            osEventFlagsSet(USBD_HID_OS_InputDataSem_Tbl[class_nbr], HID_DATA_EVENT_POST);
        }

        if (cnt) {
            *p_err = USBD_ERR_NONE;
            return;
        }

        tk_current = osKernelGetTickCount();
    } while (!timeout_ms || (tk_current - tk_start <= timeout_ms));

    *p_err = USBD_ERR_OS_TIMEOUT;
    return;
}


/*
*********************************************************************************************************
*                                  USBD_HID_OS_InputDataPendAbort()
*
* Description : Abort any operation on input report.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_InputDataPendAbort (CPU_INT08U  class_nbr)
{
    int32_t ret;

    ret = osEventFlagsSet(USBD_HID_OS_InputDataSem_Tbl[class_nbr], HID_DATA_EVENT_ABORT);
    if (ret < 0)
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
}


/*
*********************************************************************************************************
*                                     USBD_HID_OS_InputDataPost()
*
* Description : Signal that input report data transfer has completed.
*
* Argument(s) : class_nbr   Class instance number.
*               ---------   Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_HID_OS_InputDataPost (CPU_INT08U  class_nbr)
{
    int32_t ret;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    USBD_HID_OS_InDataSemCnt_Tbl[class_nbr]++;
    CPU_CRITICAL_EXIT();

    ret = osEventFlagsSet(USBD_HID_OS_InputDataSem_Tbl[class_nbr], HID_DATA_EVENT_POST);
    if (ret < 0)
        hidd_os_err("Err %d @%d, %s\n", ret, __LINE__, __func__);
}


