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
* Filename : usbd_os.c
* Version  : V1.00.00
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdio.h>
#include  <cmsis_os2.h>
#include  "../../Source/usbd_core.h"
#include  "../../Source/usbd_internal.h"
#include  <driver/irq.h>
#include  <board.h>

/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if (USBD_CFG_DBG_TRACE_EN == DEF_ENABLED)
#error  "USBD_CFG_DBG_TRACE is not supported yet."
#endif

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define EP_SIG_EVENT_POST       (1u<<0)
#define EP_SIG_EVENT_ABORT      (1u<<1)
#define EP_SIG_EVENT_DEL        (1u<<2)
#define EP_SIG_EVENT_ALL        (EP_SIG_EVENT_POST | EP_SIG_EVENT_ABORT | EP_SIG_EVENT_DEL)


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

static osThreadId_t         USBD_OS_CoreTaskID;
static osMessageQueueId_t   USBD_OS_CoreMsgID;

static  osEventFlagsId_t    USBD_OS_EP_SemTbl[USBD_CFG_MAX_NBR_DEV][USBD_CFG_MAX_NBR_EP_OPEN];
static volatile CPU_INT32U  USBD_OS_EP_SemCntTbl[USBD_CFG_MAX_NBR_DEV][USBD_CFG_MAX_NBR_EP_OPEN];
static  osMutexId_t         USBD_OS_EP_MutexTbl[USBD_CFG_MAX_NBR_DEV][USBD_CFG_MAX_NBR_EP_OPEN];


/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/

#define  usbd_os_err(...)      ssdk_printf(SSDK_CRIT, __VA_ARGS__)


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  USBD_OS_CoreTask (void  *p_arg);


/*
*********************************************************************************************************
*                                           USBD_OS_Init()
*
* Description : Initialize OS interface.
*
* Argument(s) : p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE           OS initialization successful.
*                               USBD_ERR_OS_INIT_FAIL   OS objects NOT successfully initialized.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_OS_Init (USBD_ERR  *p_err)
{
    osMessageQueueId_t  msg_q;
    osThreadId_t        thread;
    osThreadAttr_t      attr = {
        .name = "USBD_Core",
        .stack_size = 4096,
        .priority = osPriorityHigh3,
    };

    msg_q = osMessageQueueNew(USBD_CORE_EVENT_NBR_TOTAL, sizeof(void *), NULL);
    if (!msg_q) {
        *p_err = USBD_ERR_OS_INIT_FAIL;
        return;
    }
    USBD_OS_CoreMsgID = msg_q;

    thread = osThreadNew(USBD_OS_CoreTask, NULL, &attr);
    if (!thread) {
        *p_err = USBD_ERR_OS_INIT_FAIL;
        osMessageQueueDelete(msg_q);
        return;
    }

   *p_err = USBD_ERR_NONE;
    USBD_OS_CoreTaskID = thread;

    return;
}


/*
*********************************************************************************************************
*                                         USBD_OS_CoreTask()
*
* Description : OS-dependent shell task to process USB core events.
*
* Argument(s) : p_arg       Pointer to task initialization argument.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  USBD_OS_CoreTask (void  *p_arg)
{
    (void)p_arg;

    while (DEF_ON) {
        USBD_CoreTaskHandler();
    }
}


/*
*********************************************************************************************************
*                                       USBD_OS_SignalCreate()
*
* Description : Create an OS signal.
*
* Argument(s) : dev_nbr     Device number.
*               -------     Argument validated by the caller(s).
*
*               ep_ix       Endpoint index.
*               -----       Argument validated by the caller(s).
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE               OS signal     successfully created.
*                               USBD_ERR_OS_SIGNAL_CREATE   OS signal NOT successfully created.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_OS_EP_SignalCreate (CPU_INT08U   dev_nbr,
                               CPU_INT08U   ep_ix,
                               USBD_ERR    *p_err)
{
    osEventFlagsId_t    ret;

    ret = osEventFlagsNew(NULL);
    if (!ret) {
        *p_err = USBD_ERR_OS_SIGNAL_CREATE;
    } else {
        *p_err = USBD_ERR_NONE;
        USBD_OS_EP_SemTbl[dev_nbr][ep_ix] = ret;
        USBD_OS_EP_SemCntTbl[dev_nbr][ep_ix] = 0;
    }
}


/*
*********************************************************************************************************
*                                         USBD_OS_SignalDel()
*
* Description : Delete an OS signal.
*
* Argument(s) : dev_nbr     Device number.
*               -------     Argument validated by the caller(s).
*
*               ep_ix       Endpoint index.
*               -----       Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_OS_EP_SignalDel (CPU_INT08U  dev_nbr,
                            CPU_INT08U  ep_ix)
{
    osStatus_t  ret;

    osEventFlagsSet(USBD_OS_EP_SemTbl[dev_nbr][ep_ix], EP_SIG_EVENT_DEL);
    ret = osEventFlagsDelete(USBD_OS_EP_SemTbl[dev_nbr][ep_ix]);
    if (ret)
        usbd_os_err("Signal %d-%d Del ret %d\n", dev_nbr, ep_ix, ret);
    else
        USBD_OS_EP_SemTbl[dev_nbr][ep_ix] = NULL;
}


/*
*********************************************************************************************************
*                                        USBD_OS_SignalPend()
*
* Description : Wait for a signal to become available.
*
* Argument(s) : dev_nbr     Device number.
*               -------     Argument validated by the caller(s).
*
*               ep_ix       Endpoint index.
*               -----       Argument validated by the caller(s).
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

void  USBD_OS_EP_SignalPend (CPU_INT08U   dev_nbr,
                             CPU_INT08U   ep_ix,
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
        ret = osEventFlagsWait(USBD_OS_EP_SemTbl[dev_nbr][ep_ix], EP_SIG_EVENT_ALL, 0, tmo);
        if (ret < 0) {
            switch (ret) {
            case osErrorTimeout:
                *p_err = USBD_ERR_OS_TIMEOUT;
                return;

            default:
                *p_err = USBD_ERR_OS_FAIL;
                usbd_os_err("Get Signal %d-%d ret %d\n", dev_nbr, ep_ix, ret);
                return;
            }
        } else if (ret & EP_SIG_EVENT_DEL) {
            *p_err = USBD_ERR_OS_FAIL;
            usbd_os_err("Get Multi Signal %d-%d ret Del\n", dev_nbr, ep_ix);
            return;
        } else if (ret & EP_SIG_EVENT_ABORT) {
            *p_err = USBD_ERR_OS_ABORT;
            return;
        }

        CPU_CRITICAL_ENTER();
        cnt = USBD_OS_EP_SemCntTbl[dev_nbr][ep_ix];
        if (cnt)
            USBD_OS_EP_SemCntTbl[dev_nbr][ep_ix] = cnt - 1;
        CPU_CRITICAL_EXIT();

        if (cnt > 1) {
            osEventFlagsSet(USBD_OS_EP_SemTbl[dev_nbr][ep_ix], EP_SIG_EVENT_POST);
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
*                                        USBD_OS_SignalAbort()
*
* Description : Abort any wait operation on signal.
*
* Argument(s) : dev_nbr     Device number.
*               -------     Argument validated by the caller(s).
*
*               ep_ix       Endpoint index.
*               -----       Argument validated by the caller(s).
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE       OS signal     successfully aborted.
*                               USBD_ERR_OS_FAIL    OS signal NOT successfully aborted.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_OS_EP_SignalAbort (CPU_INT08U   dev_nbr,
                              CPU_INT08U   ep_ix,
                              USBD_ERR    *p_err)
{
    int32_t ret;

    ret = osEventFlagsSet(USBD_OS_EP_SemTbl[dev_nbr][ep_ix], EP_SIG_EVENT_ABORT);
    if (ret < 0) {
       *p_err = USBD_ERR_OS_FAIL;
    } else {
       *p_err = USBD_ERR_NONE;
    }
}


/*
*********************************************************************************************************
*                                        USBD_OS_SignalPost()
*
* Description : Make a signal available.
*
* Argument(s) : dev_nbr     Device number.
*               -------     Argument validated by the caller(s).
*
*               ep_ix       Endpoint index.
*               -----       Argument validated by the caller(s).
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE       OS signal     successfully readied.
*                               USBD_ERR_OS_FAIL    OS signal NOT successfully readied.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_OS_EP_SignalPost (CPU_INT08U   dev_nbr,
                             CPU_INT08U   ep_ix,
                             USBD_ERR    *p_err)
{
    int32_t ret;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    USBD_OS_EP_SemCntTbl[dev_nbr][ep_ix]++;
    CPU_CRITICAL_EXIT();

    ret = osEventFlagsSet(USBD_OS_EP_SemTbl[dev_nbr][ep_ix], EP_SIG_EVENT_POST);
    if (ret < 0) {
       *p_err = USBD_ERR_OS_FAIL;
    } else {
       *p_err = USBD_ERR_NONE;
    }
}


/*
*********************************************************************************************************
*                                       USBD_OS_EP_LockCreate()
*
* Description : Create an OS resource to use as an endpoint lock.
*
* Argument(s) : dev_nbr     Device number.
*               -------     Argument validated by the caller(s).
*
*               ep_ix       Endpoint index.
*               -----       Argument validated by the caller(s).
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE               OS lock     successfully created.
*                               USBD_ERR_OS_SIGNAL_CREATE   OS lock NOT successfully created.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void   USBD_OS_EP_LockCreate (CPU_INT08U   dev_nbr,
                              CPU_INT08U   ep_ix,
                              USBD_ERR    *p_err)
{
    osMutexId_t ret;

    ret = osMutexNew(NULL);
    if (!ret) {
       *p_err = USBD_ERR_OS_SIGNAL_CREATE;
    } else {
       *p_err = USBD_ERR_NONE;
       USBD_OS_EP_MutexTbl[dev_nbr][ep_ix] = ret;
    }
}


/*
*********************************************************************************************************
*                                         USBD_OS_EP_LockDel()
*
* Description : Delete the OS resource used as an endpoint lock.
*
* Argument(s) : dev_nbr     Device number.
*               -------     Argument validated by the caller(s).
*
*               ep_ix       Endpoint index.
*               -----       Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void   USBD_OS_EP_LockDel (CPU_INT08U  dev_nbr,
                           CPU_INT08U  ep_ix)
{
    osStatus_t  ret;

    ret = osMutexDelete(USBD_OS_EP_MutexTbl[dev_nbr][ep_ix]);
    if (ret)
        usbd_os_err("Mutex %d-%d Del ret %d\n", dev_nbr, ep_ix, ret);
}


/*
*********************************************************************************************************
*                                       USBD_OS_EP_LockAcquire()
*
* Description : Wait for an endpoint to become available and acquire its lock.
*
* Argument(s) : dev_nbr     Device number.
*               -------     Argument validated by the caller(s).
*
*               ep_ix       Endpoint index.
*               -----       Argument validated by the caller(s).
*
*               timeout_ms  Lock wait timeout in milliseconds.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE           OS lock     successfully acquired.
*                               USBD_ERR_OS_TIMEOUT     OS lock NOT successfully acquired in the time
*                                                           specified by 'timeout_ms'.
*                               USBD_ERR_OS_ABORT       OS lock aborted.
*                               USBD_ERR_OS_FAIL        OS lock not acquired because another error.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void   USBD_OS_EP_LockAcquire (CPU_INT08U   dev_nbr,
                               CPU_INT08U   ep_ix,
                               CPU_INT16U   timeout_ms,
                               USBD_ERR    *p_err)
{
    osStatus_t  ret;
    uint32_t    tmo;

    tmo = timeout_ms ? timeout_ms : osWaitForever;
    ret = osMutexAcquire(USBD_OS_EP_MutexTbl[dev_nbr][ep_ix], tmo);
    switch (ret) {
    case osOK:
        *p_err = USBD_ERR_NONE;
        break;

    case osErrorTimeout:
        *p_err = USBD_ERR_OS_TIMEOUT;
        break;

    default:
        *p_err = USBD_ERR_OS_FAIL;
        usbd_os_err("Get Mutex %d-%d ret %d\n", dev_nbr, ep_ix, ret);
        break;
    }
}


/*
*********************************************************************************************************
*                                      USBD_OS_EP_LockRelease()
*
* Description : Release an endpoint lock.
*
* Argument(s) : dev_nbr     Device number.
*               -------     Argument validated by the caller(s).
*
*               ep_ix       Endpoint index.
*               -----       Argument validated by the caller(s).
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void   USBD_OS_EP_LockRelease (CPU_INT08U  dev_nbr,
                               CPU_INT08U  ep_ix)
{
    osStatus_t  ret;

    ret = osMutexRelease(USBD_OS_EP_MutexTbl[dev_nbr][ep_ix]);
    if (ret)
        usbd_os_err("Mutex %d-%d Release ret %d\n", dev_nbr, ep_ix, ret);
}


/*
*********************************************************************************************************
*                                        USBD_OS_DlyMs()
*
* Description : Delay a task for a certain time.
*
* Argument(s) : ms          Delay in milliseconds.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  USBD_OS_DlyMs (CPU_INT32U  ms)
{
    if (ms < 3)
        udelay(ms * 1000);
    else
        osDelay(ms);
}


/*
*********************************************************************************************************
*                                       USBD_OS_CoreEventGet()
*
* Description : Wait until a core event is ready.
*
* Argument(s) : timeout_ms  Timeout in milliseconds.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               USBD_ERR_NONE           Core event successfully obtained.
*                               USBD_ERR_OS_TIMEOUT     Core event NOT ready and a timeout occurred.
*                               USBD_ERR_OS_ABORT       Core event was aborted.
*                               USBD_ERR_OS_FAIL        Core event NOT ready because of another error.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  *USBD_OS_CoreEventGet (CPU_INT32U   timeout_ms,
                             USBD_ERR    *p_err)
{
    void        *p_msg;
    osStatus_t  ret;
    uint32_t    tmo;

    tmo = timeout_ms ? timeout_ms : osWaitForever;
    ret = osMessageQueueGet(USBD_OS_CoreMsgID, &p_msg, NULL, tmo);
    switch (ret) {
    case osOK:
        *p_err = USBD_ERR_NONE;
        break;

    case osErrorTimeout:
        *p_err = USBD_ERR_OS_TIMEOUT;
        break;

    case osErrorResource:
    default:
        *p_err = USBD_ERR_OS_FAIL;
        usbd_os_err("MsgQ Get ret %d\n", ret);
        break;
    }

    return (*p_err == USBD_ERR_NONE ? p_msg : NULL);
}


/*
*********************************************************************************************************
*                                       USBD_OS_CoreEventPut()
*
* Description : Queues core event.
*
* Argument(s) : p_event     Pointer to core event.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_OS_CoreEventPut (void  *p_event)
{
    void        *p_msg;
    osStatus_t  ret;

    p_msg = p_event;
    ret = osMessageQueuePut(USBD_OS_CoreMsgID, &p_msg, 0, 0);
    if (ret != osOK)
        usbd_os_err("MsgQ Put ret %d\n", ret);
}


