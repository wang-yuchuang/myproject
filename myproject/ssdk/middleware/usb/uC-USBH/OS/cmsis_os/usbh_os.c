/*
*********************************************************************************************************
*                                             uC/USB-Host
*                                     The Embedded USB Host Stack
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
*                                   USB HOST OPERATING SYSTEM LAYER
*                                          For CMSIS OS2
*
* Filename : usbh_os.c
* Version  : V1.00.00
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdlib.h>
#include  <cmsis_os2.h>
#include  <usbh_cfg.h>
#include  "../../Source/usbh_os.h"
#include  <board.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  USBH_OS_MUTEX_REQUIRED             ((((USBH_CFG_MAX_NBR_EPS * USBH_CFG_MAX_NBR_IFS) + 1u) * USBH_CFG_MAX_NBR_DEVS) + \
                                             USBH_CFG_MAX_NBR_DEVS + USBH_CFG_MAX_NBR_HC +                                    \
                                             USBH_CDC_CFG_MAX_DEV + USBH_HID_CFG_MAX_DEV + USBH_MSC_CFG_MAX_DEV)
#define  USBH_OS_SEM_REQUIRED               (3u + (((USBH_CFG_MAX_NBR_EPS * USBH_CFG_MAX_NBR_IFS) + 1u) * USBH_CFG_MAX_NBR_DEVS))
#define  USBH_OS_TCB_REQUIRED                 4u
#define  USBH_OS_Q_REQUIRED                   1u


#define  USBH_SEM_EVENT_POST                (1u<<0)
#define  USBH_SEM_EVENT_ABORT               (1u<<1)
#define  USBH_SEM_EVENT_DEL                 (1u<<2)
#define  USBH_SEM_EVENT_ALL                 (USBH_SEM_EVENT_POST | USBH_SEM_EVENT_ABORT | USBH_SEM_EVENT_DEL)


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

typedef struct usbh_semaphore {
    osEventFlagsId_t    evt;
    volatile CPU_INT32U cnt;
} USBH_SIGNAL;


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


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#define  usbh_os_err(...)      ssdk_printf(SSDK_CRIT, __VA_ARGS__)


/*
*********************************************************************************************************
*                                         USBH_OS_LayerInit()
*
* Description : Initialize OS layer.
*
* Argument(s) : none.
*
* Return(s)   : USBH_ERR_NONE, if successful.
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_LayerInit (void)
{
    return (USBH_ERR_NONE);
}


/*
**************************************************************************************************************
*                                       USBH_OS_VirToBus()
*
* Description : Convert from virtual address to physical address if the operating system uses
*               virtual memory
*
* Arguments   : x       Virtual address to convert
*
* Returns     : The corresponding physical address
*
* Note(s)     : None.
**************************************************************************************************************
*/

void  *USBH_OS_VirToBus (void  *x)
{
    return (x);
}


/*
**************************************************************************************************************
*                                       USBH_OS_BusToVir()
*
* Description : Convert from physical address to virtual address if the operating system uses
*               virtual memory
*
* Arguments   : x       Physical address to convert
*
* Returns     : The corresponding virtual address
*
* Note(s)     : None.
**************************************************************************************************************
*/

void  *USBH_OS_BusToVir (void  *x)
{
    return (x);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           DELAY FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           USBH_OS_DlyMS()
*
* Description : Delay the current task by specified delay.
*
* Argument(s) : dly       Delay, in milliseconds.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBH_OS_DlyMS (CPU_INT32U  dly)
{
    if (dly < 3)
        udelay(dly * 1000);
    else
        osDelay(dly);
}


/*
*********************************************************************************************************
*                                           USBH_OS_DlyUS()
*
* Description : Delay the current task by specified delay.
*
* Argument(s) : dly       Delay, in microseconds.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBH_OS_DlyUS (CPU_INT32U  dly)
{
    udelay(dly);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           MUTEX FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        USBH_OS_MutexCreate()
*
* Description : Create a mutex.
*
* Argument(s) : p_mutex    Pointer to variable that will receive handle of the mutex.
*
* Return(s)   : USBH_ERR_NONE,              if successful.
*               USBH_ERR_ALLOC,
*               USBH_ERR_OS_SIGNAL_CREATE,
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_MutexCreate (USBH_HMUTEX  *p_mutex)
{
    osMutexId_t ret;

    ret = osMutexNew(NULL);
    if (!ret) {
        return USBH_ERR_OS_SIGNAL_CREATE;
    }

    *p_mutex = (USBH_HMUTEX)ret;
    return USBH_ERR_NONE;
}


/*
*********************************************************************************************************
*                                         USBH_OS_MutexLock()
*
* Description : Acquire a mutex.
*
* Argument(s) : mutex     Mutex handle.
*
* Return(s)   : USBH_ERR_NONE,          if successful.
*               USBH_ERR_OS_TIMEOUT,    if timeout.
*               USBH_ERR_OS_FAIL,       otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_MutexLock (USBH_HMUTEX  mutex)
{
    osStatus_t  ret;

    ret = osMutexAcquire((osMutexId_t)mutex, osWaitForever);
    switch (ret) {
    case osOK:
        return USBH_ERR_NONE;
        break;

    case osErrorTimeout:
        return USBH_ERR_OS_TIMEOUT;
        break;

    default:
        usbh_os_err("Get Mutex %X ret %d\n", mutex, ret);
        return USBH_ERR_OS_FAIL;
        break;
    }
}


/*
*********************************************************************************************************
*                                        USBH_OS_MutexUnlock()
*
* Description : Releases a mutex.
*
* Argument(s) : mutex     Mutex handle.
*
* Return(s)   : USBH_ERR_NONE,      if successful.
*               USBH_ERR_OS_FAIL,   otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_MutexUnlock (USBH_HMUTEX  mutex)
{
    osStatus_t  ret;

    ret = osMutexRelease((osMutexId_t)mutex);
    if (ret) {
        usbh_os_err("Mutex %X Release ret %d\n", mutex, ret);
        return USBH_ERR_OS_FAIL;
    }

    return USBH_ERR_NONE;
}


/*
*********************************************************************************************************
*                                        USBH_OS_MutexDestroy()
*
* Description : Destroys a mutex only when no task is pending on the mutex.
*
* Argument(s) : mutex     Mutex handle.
*
* Return(s)   : USBH_ERR_NONE,      if successful.
*               USBH_ERR_OS_FAIL,   if mutex delete failed.
*               USBH_ERR_FREE,      if mutex free failed.
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_MutexDestroy (USBH_HMUTEX  mutex)
{
    osStatus_t  ret;

    ret = osMutexDelete((osMutexId_t)mutex);
    if (ret) {
        usbh_os_err("Mutex %X Del ret %d\n", mutex, ret);
        return USBH_ERR_OS_FAIL;
    }

    return USBH_ERR_NONE;
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         SEMAPHORE FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         USBH_OS_SemCreate()
*
* Description : Create semaphore with given count.
*
* Argument(s) : p_sem      Pointer that will receive handle for managing semaphore.

*               cnt        Value with which the semaphore will be initialized.
*
* Return(s)   : USBH_ERR_NONE,              if successful.
*               USBH_ERR_OS_SIGNAL_CREATE,
*               USBH_ERR_ALLOC,
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_SemCreate (USBH_HSEM   *p_sem,
                             CPU_INT32U   cnt)
{
    osEventFlagsId_t    ret;
    USBH_SIGNAL         *hsig;

    hsig = osAllocAlign(_Alignof(*hsig), sizeof(*hsig));
    if (!hsig)
        return USBH_ERR_ALLOC;

    ret = osEventFlagsNew(NULL);
    if (!ret) {
        free(hsig);
        return USBH_ERR_OS_SIGNAL_CREATE;
    }

    hsig->evt = ret;
    hsig->cnt = cnt;
    if (cnt > 0) {
        osEventFlagsSet(hsig->evt, USBH_SEM_EVENT_POST);
    }

    *p_sem = (USBH_HSEM)hsig;

    return USBH_ERR_NONE;
}


/*
*********************************************************************************************************
*                                         USBH_OS_SemDestroy()
*
* Description : Destroy a semphore if no tasks are waiting on it.
*
* Argument(s) : sem       Semaphore handle.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_SemDestroy (USBH_HSEM  sem)
{
    USBH_SIGNAL *hsig;
    osStatus_t  ret;

    hsig = (USBH_SIGNAL *)sem;

    if (!hsig || !hsig->evt)
        return USBH_ERR_INVALID_ARG;

    osEventFlagsSet(hsig->evt, USBH_SEM_EVENT_DEL);
    ret = osEventFlagsDelete(hsig->evt);
    if (ret) {
        usbh_os_err("Signal %p Del ret %d\n", hsig->evt, ret);
        return USBH_ERR_OS_FAIL;
    }

    free(hsig);

    return USBH_ERR_NONE;
}


/*
*********************************************************************************************************
*                                          USBH_OS_SemWait()
*
* Description : Wait on a semaphore to become available.
*
* Argument(s) : sem       Semaphore handle.
*
*               timeout   Timeout.
*
* Return(s)   : USBH_ERR_NONE,          if successful.
*               USBH_ERR_OS_TIMEOUT,    if timeout error.
*               USBH_ERR_OS_ABORT,
*               USBH_ERR_OS_FAIL,
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_SemWait (USBH_HSEM   sem,
                           CPU_INT32U  timeout)
{
    USBH_SIGNAL *hsig;
    int32_t     ret;
    uint32_t    tmo;
    uint32_t    tk_start, tk_current;
    CPU_INT32U  cnt;
    CPU_SR_ALLOC();

    hsig = (USBH_SIGNAL *)sem;
    if (!hsig || !hsig->evt)
        return USBH_ERR_INVALID_ARG;

    tk_start = osKernelGetTickCount();
    tk_current = tk_start;
    do {
        tmo = timeout ? (tk_start + timeout - tk_current): osWaitForever;
        ret = osEventFlagsWait(hsig->evt, USBH_SEM_EVENT_ALL, 0, tmo);
        if (ret < 0) {
            switch (ret) {
            case osErrorTimeout:
                return USBH_ERR_OS_TIMEOUT;

            default:
                usbh_os_err("Get Signal %p ret %d\n", hsig->evt, ret);
                return USBH_ERR_OS_FAIL;
            }
        } else if (ret & USBH_SEM_EVENT_DEL) {
            usbh_os_err("Get Multi Signal %p ret Del\n", hsig->evt);
            return USBH_ERR_OS_FAIL;
        } else if (ret & USBH_SEM_EVENT_ABORT) {
            return USBH_ERR_OS_ABORT;
        }

        CPU_CRITICAL_ENTER();
        cnt = hsig->cnt;
        if (cnt)
            hsig->cnt = cnt - 1;
        CPU_CRITICAL_EXIT();

        if (cnt > 1) {
            osEventFlagsSet(hsig->evt, USBH_SEM_EVENT_POST);
        }

        if (cnt) {
            return USBH_ERR_NONE;
        }

        tk_current = osKernelGetTickCount();
    } while (!timeout || (tk_current - tk_start <= timeout));

    return USBH_ERR_OS_TIMEOUT;
}


/*
*********************************************************************************************************
*                                       USBH_OS_SemWaitAbort()
*
* Description : Resume all tasks waiting on specified semaphore.
*
* Argument(s) : sem       Semaphore handle.
*
* Return(s)   : USBH_ERR_NONE,          if successful.
*               USBH_ERR_OS_TIMEOUT,    if timeout error.
*               USBH_ERR_OS_ABORT,
*               USBH_ERR_OS_FAIL,
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_SemWaitAbort (USBH_HSEM  sem)
{
    int32_t     ret;
    USBH_SIGNAL *hsig;

    hsig = (USBH_SIGNAL *)sem;
    if (!hsig || !hsig->evt)
        return USBH_ERR_INVALID_ARG;

    ret = osEventFlagsSet(hsig->evt, USBH_SEM_EVENT_ABORT);
    if (ret < 0) {
       return USBH_ERR_OS_FAIL;
    }

    return USBH_ERR_NONE;
}


/*
*********************************************************************************************************
*                                          USBH_OS_SemPost()
*
* Description : Post a semaphore.
*
* Argument(s) : sem       Semaphore handle.
*
* Return(s)   : USBH_ERR_NONE,      if successful.
*               USBH_ERR_OS_FAIL,   otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_SemPost (USBH_HSEM  sem)
{
    int32_t     ret;
    USBH_SIGNAL *hsig;
    CPU_SR_ALLOC();

    hsig = (USBH_SIGNAL *)sem;
    if (!hsig || !hsig->evt)
        return USBH_ERR_INVALID_ARG;

    CPU_CRITICAL_ENTER();
    hsig->cnt++;
    CPU_CRITICAL_EXIT();

    ret = osEventFlagsSet(hsig->evt, USBH_SEM_EVENT_POST);
    if (ret < 0) {
       return USBH_ERR_OS_FAIL;
    }

    return USBH_ERR_NONE;
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          THREAD FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        USBH_OS_ThreadCreate()
*
* Description : Create a thread (or task).
*
* Argument(s) : p_name          Pointer to name to assign to thread.
*
*               prio            Priority of the thread to be created.
*
*               thread_fnct     Pointer to the function that will be executed in this thread.
*
*               p_data          Pointer to the data that is passed to the thread function.
*
*               p_stk           Pointer to the beginning of the stack used by the thread.
*
*               stk_size        Size of the stack.
*
*               p_thread        Pointer that will receive handle for managing thread.
*
* Return(s)   : USBH_ERR_NONE,               if successful.
*               USBH_ERR_ALLOC,
*               USBH_ERR_OS_TASK_CREATE,
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_TaskCreate (CPU_CHAR        *p_name,
                              CPU_INT32U       prio,
                              USBH_TASK_FNCT   task_fnct,
                              void            *p_data,
                              CPU_INT32U      *p_stk,
                              CPU_INT32U       stk_size,
                              USBH_HTASK      *p_task)
{

    osThreadId_t    thread;
    osThreadAttr_t  attr = {0};

    attr.name = p_name;
    attr.priority = prio;
    attr.stack_mem = p_stk;
    attr.stack_size = stk_size;

    thread = osThreadNew(task_fnct, p_data, &attr);
    if (!thread) {
        return USBH_ERR_OS_TASK_CREATE;
    }

    *p_task = (USBH_HTASK)thread;

    return USBH_ERR_NONE;
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            MESSAGE QUEUE
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       USBH_OS_MsgQueueCreate()
*
* Description : Create a message queue.
*
* Argument(s) : p_start         Pointer to the base address of the message queue storage area.
*
*               size            Number of elements in the storage area.
*
*               p_err           Pointer to variable that will receive the return error code from this function:
*
*                                   USBH_ERR_NONE               Message queue created.
*                                   USBH_ERR_ALLOC              Message queue creation failed.
*                                   USBH_ERR_OS_SIGNAL_CREATE
*
* Return(s)   : The handle of the message queue.
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_HQUEUE  USBH_OS_MsgQueueCreate (void        **p_start,
                                     CPU_INT16U    size,
                                     USBH_ERR     *p_err)
{
    osMessageQueueId_t  msg_q;

    msg_q = osMessageQueueNew(size, sizeof(void *), NULL);
    if (!msg_q) {
        *p_err = USBH_ERR_OS_SIGNAL_CREATE;
        return ((USBH_HQUEUE )0);
    }

    *p_err = USBH_ERR_NONE;
    return ((USBH_HQUEUE)msg_q);
}


/*
*********************************************************************************************************
*                                        USBH_OS_MsgQueuePut()
*
* Description : Post a message to a message queue.
*
* Argument(s) : msg_q     Message queue handle.
*
*               p_msg     Pointer to the message to send.
*
* Return(s)   : USBH_ERR_NONE,      if successful.
*               USBH_ERR_OS_FAIL,   otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

USBH_ERR  USBH_OS_MsgQueuePut (USBH_HQUEUE   msg_q,
                               void         *p_msg)
{
    void        *p;
    osStatus_t  ret;

    p = p_msg;
    ret = osMessageQueuePut((osMessageQueueId_t)msg_q, &p, 0, osWaitForever);
    if (ret != osOK) {
        usbh_os_err("MsgQ Put ret %d\n", ret);
        return USBH_ERR_OS_FAIL;
    }

    return (USBH_ERR_NONE);
}


/*
*********************************************************************************************************
*                                        USBH_OS_MsgQueueGet()
*
* Description : Get a message from a message queue and blocks forever until a message is posted in the
*               queue.
*
* Argument(s) : msg_q     Message queue handle.
*
*               timeout   Time to wait for a message, in milliseconds.
*
*               p_err     Pointer to variable that will receive the return error code from this function :
*
*                         USBH_ERR_NONE,           if successful.
*                         USBH_ERR_OS_TIMEOUT,
*                         USBH_ERR_OS_ABORT,
*                         USBH_ERR_OS_FAIL,
*
* Return(s)   : Pointer to message, if successful.
*               Pointer to null,    otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  *USBH_OS_MsgQueueGet (USBH_HQUEUE   msg_q,
                            CPU_INT32U    timeout,
                            USBH_ERR     *p_err)
{
    void        *p_msg;
    osStatus_t  ret;
    uint32_t    tmo;

    tmo = timeout ? timeout : osWaitForever;
    ret = osMessageQueueGet((osMessageQueueId_t)msg_q, &p_msg, NULL, tmo);
    switch (ret) {
    case osOK:
        *p_err = USBH_ERR_NONE;
        break;

    case osErrorTimeout:
        *p_err = USBH_ERR_OS_TIMEOUT;
        break;

    case osErrorResource:
    default:
        *p_err = USBH_ERR_OS_FAIL;
        usbh_os_err("MsgQ Get ret %d\n", ret);
        break;
    }

    return (*p_err == USBH_ERR_NONE ? p_msg : NULL);
}

