/**
 * @file app_usbd_cdc.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: usb device 5 VCOM test & demo application.
 *
 * Revision History:
 * -----------------
 */


#include  "app_usbd.h"
#include  <Class/CDC/usbd_cdc.h>
#include  <Class/CDC/ACM/usbd_acm_serial.h>
#include  <cmsis_os2.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  ASCII_CHAR_END_OF_TEXT                         0x03

#define  APP_USBD_CDC_COM_CNT                           5u
#define  APP_USBD_CDC_BUF_LEN                           2048u

#define  APP_USBD_CDC_CURSOR_START                     "\033[H"
#define  APP_USBD_CDC_CURSOR_START_SIZE                   3u

#define  APP_USBD_CDC_SCREEN_CLR                "\033[2J\033[H"
#define  APP_USBD_CDC_SCREEN_CLR_SIZE                     7u

#define  APP_USBD_CDC_SCREEN_SIZE                        80u

#define  APP_USBD_CDC_TX_TIMEOUT_mS                       0u
#define  APP_USBD_CDC_RX_TIMEOUT_mS                       0u

#define  APP_USBD_CDC_MSG                       "===== USB CDC ACM Serial Emulation Demo ======"\
                                                "\r\n"\
                                                "\r\n"\
                                                "1. Echo 1 demo.\r\n"\
                                                "2. Echo N demo.\r\n"\
                                                "Option: "
#define  APP_USBD_CDC_MSG_SIZE                           92u

#define  APP_USBD_CDC_MSG1                      "Echo 1 demo... \r\n\r\n>> "
#define  APP_USBD_CDC_MSG1_SIZE                          22u

#define  APP_USBD_CDC_MSG2                      "Echo N demo. You can send up to 2048 characters at once... \r\n\r\n>> "
#define  APP_USBD_CDC_MSG2_SIZE                          66u

#define  APP_USBD_CDC_NEW_LINE                  "\n\r>> "
#define  APP_USBD_CDC_NEW_LINE_SIZE                       5u


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
*                                   ACM SERIAL DEMO STATE DATA TYPE
*********************************************************************************************************
*/

typedef  enum  app_usbd_cdc_state {
    APP_USBD_CDC_STATE_MENU = 0u,
    APP_USBD_CDC_STATE_ECHO_1,
    APP_USBD_CDC_STATE_ECHO_N
} APP_USBD_CDC_STATE;


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

_Alignas(CONFIG_ARCH_CACHE_LINE)
static  CPU_INT08U  App_USBD_CDC_SerialBuf[APP_USBD_CDC_COM_CNT][APP_USBD_CDC_BUF_LEN];

static CPU_INT08U          App_USBD_CDC_Msg[APP_USBD_CDC_MSG_SIZE] = {APP_USBD_CDC_MSG};
static CPU_INT08U          App_USBD_CDC_Msg1[APP_USBD_CDC_MSG1_SIZE] = {APP_USBD_CDC_MSG1};
static CPU_INT08U          App_USBD_CDC_Msg2[APP_USBD_CDC_MSG2_SIZE] = {APP_USBD_CDC_MSG2};
static CPU_INT08U          App_USBD_CDC_CursorStart[APP_USBD_CDC_CURSOR_START_SIZE] = {APP_USBD_CDC_CURSOR_START};
static CPU_INT08U          App_USBD_CDC_ScreenClr[APP_USBD_CDC_SCREEN_CLR_SIZE] = {APP_USBD_CDC_SCREEN_CLR};
static CPU_INT08U          App_USBD_CDC_NewLine[APP_USBD_CDC_NEW_LINE_SIZE] = {APP_USBD_CDC_NEW_LINE};


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void         App_USBD_CDC_SerialTask      (void                         *p_arg);

static  void         App_USBD_CDC_SerialLineCtrl  (CPU_INT08U                    subclass_nbr,
                                                   CPU_INT08U                    events,
                                                   CPU_INT08U                    events_chngd,
                                                   void                         *p_arg);

static  CPU_BOOLEAN  App_USBD_CDC_SerialLineCoding(CPU_INT08U                    subclass_nbr,
                                                   USBD_ACM_SERIAL_LINE_CODING  *p_line_coding,
                                                   void                         *p_arg);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         App_USBD_CDC_Init()
*
* Description : Initialize CDC class.
*
* Argument(s) : dev_nbr    Device number.
*
*               cfg_hs     High speed configuration number.
*
*               cfg_fs     Full speed configuration number.
*
* Return(s)   : DEF_OK,    if CDC class initialized successfully.
*
*               DEF_FAIL,  otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

bool  App_USBD_CDC_Init(unsigned char   dev_nbr,
                        unsigned char   cfg_hs,
                        unsigned char   cfg_fs)
{
    int             i;
    USBD_ERR        err;
    USBD_ERR        err_hs;
    USBD_ERR        err_fs;
    CPU_INT08U      subclass_nbr;
    osThreadId_t    thread;
    osThreadAttr_t  attr = {
        .name = "USBD CDC ACM Test",
        .stack_size = 4096,
        .priority = osPriorityNormal,
    };


    printf("        Initializing CDC class ... \n");

    USBD_CDC_Init(&err);
    if (err != USBD_ERR_NONE) {
        printf("        ... could not initialize CDC class w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    printf("            Initializing ACM serial subclass ... \n");

    USBD_ACM_SerialInit(&err);
    if (err != USBD_ERR_NONE) {
        printf("            ... could not initialize ACM serial subclass w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    for (i = 0; i < APP_USBD_CDC_COM_CNT; i++) {
        subclass_nbr = USBD_ACM_SerialAdd(64u,
                                         (USBD_ACM_SERIAL_CALL_MGMT_DATA_CCI_DCI | USBD_ACM_SERIAL_CALL_MGMT_DEV),
                                         &err);
        if (err != USBD_ERR_NONE) {
            printf("            ... could not create ACM serial subclass instance w/err = %d\n\n", err);
            return (DEF_FAIL);
        }

                                                                    /* Register line coding and ctrl line change callbacks. */
        USBD_ACM_SerialLineCodingReg(        subclass_nbr,
                                             App_USBD_CDC_SerialLineCoding,
                                     (void *)0,
                                            &err);

        USBD_ACM_SerialLineCtrlReg(        subclass_nbr,
                                           App_USBD_CDC_SerialLineCtrl,
                                   (void *)0,
                                          &err);


        if (cfg_hs != USBD_CFG_NBR_NONE) {
            USBD_ACM_SerialCfgAdd(subclass_nbr, dev_nbr, cfg_hs, &err_hs);
            if (err_hs != USBD_ERR_NONE) {
                printf("        ... could not add ACM serial subclass instance #%d to HS configuration w/err = %d\n\n", subclass_nbr, err_hs);
            }
        }

        if (cfg_fs != USBD_CFG_NBR_NONE) {
            USBD_ACM_SerialCfgAdd(subclass_nbr, dev_nbr, cfg_fs, &err_fs);
            if (err_fs != USBD_ERR_NONE) {
                printf("        ... could not add ACM serial subclass instance #%d to FS configuration w/err = %d\n\n", subclass_nbr, err_fs);
            }
        }

        if ((err_hs != USBD_ERR_NONE) &&                            /* If HS and FS cfg fail, stop class init.              */
            (err_fs != USBD_ERR_NONE)) {
            return (DEF_FAIL);
        }

        thread = osThreadNew(App_USBD_CDC_SerialTask, (void *)(CPU_ADDR)subclass_nbr, &attr);
        if (!thread) {
            printf("        ... could not add CDC ACM serial test task\n\n");
            return (DEF_FAIL);
        }
    }

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                      App_USBD_CDC_SerialTask()
*
* Description : USB CDC ACM serial emulation demo task.
*
* Argument(s) : p_arg    Task argument pointer.
*
* Return(s)   : none.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  App_USBD_CDC_SerialTask (void  *p_arg)
{
    CPU_BOOLEAN         conn;
    APP_USBD_CDC_STATE  state;
    CPU_INT08U          subclass_nbr;
    CPU_INT08U          line_ctr;
    CPU_INT08U          line_state;
    CPU_INT32U          xfer_len;
    USBD_ERR            err;


    subclass_nbr = (CPU_INT08U)(CPU_ADDR)p_arg;
    state        =  APP_USBD_CDC_STATE_MENU;
    line_ctr     =  0u;

    while (DEF_TRUE) {
                                                                /* Wait until device is in cfg'd state.                 */
        conn       = USBD_ACM_SerialIsConn(subclass_nbr);
        line_state = USBD_ACM_SerialLineCtrlGet(subclass_nbr, &err);

        while ((conn != DEF_YES                                     ) ||
               (DEF_BIT_IS_CLR(line_state, USBD_ACM_SERIAL_CTRL_DTR)) ||
               (err  != USBD_ERR_NONE                               )) {

            osDelay(250u);

            conn       = USBD_ACM_SerialIsConn(subclass_nbr);
            line_state = USBD_ACM_SerialLineCtrlGet(subclass_nbr, &err);
        }

        switch (state) {
            case APP_USBD_CDC_STATE_MENU:
                 (void)USBD_ACM_SerialTx( subclass_nbr,
                                          App_USBD_CDC_CursorStart,
                                          APP_USBD_CDC_CURSOR_START_SIZE,
                                          APP_USBD_CDC_TX_TIMEOUT_mS,
                                         &err);

                 (void)USBD_ACM_SerialTx( subclass_nbr,
                                          App_USBD_CDC_Msg,
                                          APP_USBD_CDC_MSG_SIZE,
                                          APP_USBD_CDC_TX_TIMEOUT_mS,
                                         &err);


                 (void)USBD_ACM_SerialRx( subclass_nbr,         /* Wait for character.                                  */
                                          App_USBD_CDC_SerialBuf[subclass_nbr],
                                          1u,
                                          APP_USBD_CDC_RX_TIMEOUT_mS,
                                         &err);

                 if (err != USBD_ERR_NONE) {
                     break;
                 }

                 (void)USBD_ACM_SerialTx( subclass_nbr,         /* Echo back character.                                 */
                                          App_USBD_CDC_SerialBuf[subclass_nbr],
                                          1u,
                                          APP_USBD_CDC_TX_TIMEOUT_mS,
                                         &err);

                 switch (App_USBD_CDC_SerialBuf[subclass_nbr][0]) {                                  /* Select demo options.                                 */
                     case '1':
                          (void)USBD_ACM_SerialTx( subclass_nbr,
                                                   App_USBD_CDC_ScreenClr,
                                                   APP_USBD_CDC_SCREEN_CLR_SIZE,
                                                   APP_USBD_CDC_TX_TIMEOUT_mS,
                                                  &err);

                          (void)USBD_ACM_SerialTx( subclass_nbr,
                                                   App_USBD_CDC_Msg1,
                                                   APP_USBD_CDC_MSG1_SIZE,
                                                   APP_USBD_CDC_TX_TIMEOUT_mS,
                                                  &err);

                          state    = APP_USBD_CDC_STATE_ECHO_1;
                          line_ctr = 0u;
                          break;


                     case '2':
                          (void)USBD_ACM_SerialTx( subclass_nbr,
                                                   App_USBD_CDC_ScreenClr,
                                                   APP_USBD_CDC_SCREEN_CLR_SIZE,
                                                   APP_USBD_CDC_TX_TIMEOUT_mS,
                                                  &err);

                          (void)USBD_ACM_SerialTx( subclass_nbr,
                                                   App_USBD_CDC_Msg2,
                                                   APP_USBD_CDC_MSG2_SIZE,
                                                   APP_USBD_CDC_TX_TIMEOUT_mS,
                                                  &err);

                           state    = APP_USBD_CDC_STATE_ECHO_N;
                           line_ctr = 0u;
                           break;


                     default:
                          break;

                 }
                 break;


            case APP_USBD_CDC_STATE_ECHO_1:                     /* 'Echo 1' state.                                      */
                 (void)USBD_ACM_SerialRx( subclass_nbr,         /*  Wait for character.                                 */
                                          App_USBD_CDC_SerialBuf[subclass_nbr],
                                          1u,
                                          0u,
                                         &err);

                 if (err != USBD_ERR_NONE) {
                     break;
                 }

                 if (App_USBD_CDC_SerialBuf[subclass_nbr][0] == ASCII_CHAR_END_OF_TEXT) {            /* If 'Ctrl-c' character is received.                   */
                     state = APP_USBD_CDC_STATE_MENU;           /* ... return to 'menu' state.                          */
                    (void)USBD_ACM_SerialTx( subclass_nbr,
                                             App_USBD_CDC_ScreenClr,
                                             APP_USBD_CDC_SCREEN_CLR_SIZE,
                                             APP_USBD_CDC_TX_TIMEOUT_mS,
                                            &err);

                 } else {
                     (void)USBD_ACM_SerialTx( subclass_nbr,     /* Echo back character.                                 */
                                              App_USBD_CDC_SerialBuf[subclass_nbr],
                                              1u,
                                              APP_USBD_CDC_TX_TIMEOUT_mS,
                                             &err);
                      line_ctr++;

                      if (line_ctr == APP_USBD_CDC_SCREEN_SIZE - 3u) {
                                                                /* Move to next line.                                   */
                          (void)USBD_ACM_SerialTx( subclass_nbr,
                                                   App_USBD_CDC_NewLine,
                                                   APP_USBD_CDC_NEW_LINE_SIZE,
                                                   APP_USBD_CDC_TX_TIMEOUT_mS,
                                                  &err);
                           line_ctr = 0u;
                      }
                 }
                 break;


            case APP_USBD_CDC_STATE_ECHO_N:                     /* 'Echo N' state.                                      */
                 xfer_len = USBD_ACM_SerialRx( subclass_nbr,    /*  Wait for N characters.                              */
                                              &App_USBD_CDC_SerialBuf[subclass_nbr][0],
                                               APP_USBD_CDC_BUF_LEN,
                                               0u,
                                              &err);

                 if (err != USBD_ERR_NONE) {
                     break;
                 }

                 if ((xfer_len == 1) &&                         /* If 'Ctrl-c' character is received.                   */
                     (App_USBD_CDC_SerialBuf[subclass_nbr][0] == ASCII_CHAR_END_OF_TEXT)) {

                     state = APP_USBD_CDC_STATE_MENU;           /* ... return to 'menu' state.                          */
                    (void)USBD_ACM_SerialTx( subclass_nbr,
                                             App_USBD_CDC_ScreenClr,
                                             APP_USBD_CDC_SCREEN_CLR_SIZE,
                                             APP_USBD_CDC_TX_TIMEOUT_mS,
                                            &err);

                 } else {
                     (void)USBD_ACM_SerialTx( subclass_nbr,     /* Echo back characters.                                */
                                             &App_USBD_CDC_SerialBuf[subclass_nbr][0],
                                              xfer_len,
                                              APP_USBD_CDC_TX_TIMEOUT_mS,
                                             &err);
                      line_ctr += xfer_len;

                      if (line_ctr == APP_USBD_CDC_SCREEN_SIZE - 3u) {
                                                                /* Move to next line.                                   */
                          (void)USBD_ACM_SerialTx( subclass_nbr,
                                                   App_USBD_CDC_NewLine,
                                                   APP_USBD_CDC_NEW_LINE_SIZE,
                                                   APP_USBD_CDC_TX_TIMEOUT_mS,
                                                  &err);
                           line_ctr = 0u;
                      }
                 }
                 break;


            default:
                 break;
        }
    }
}


/*
*********************************************************************************************************
*                                    App_USBD_CDC_SerialLineCtrl()
*
* Description : Serial control line change notification callback.
*
* Argument(s) : nbr             CDC ACM serial emulation subclass instance number.
*
*               events          Current line state. The line state is a OR'ed of the following flags :
*
*                                   USBD_ACM_SERIAL_CTRL_BREAK
*                                   USBD_ACM_SERIAL_CTRL_RTS
*                                   USBD_ACM_SERIAL_CTRL_DTR
*
*               events_chngd    Line state flags that have changed.
*
*               p_arg           Callback argument.
*
* Return(s)   : none.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  App_USBD_CDC_SerialLineCtrl (CPU_INT08U   nbr,
                                           CPU_INT08U   events,
                                           CPU_INT08U   events_chngd,
                                           void        *p_arg)
{
    (void)nbr;
    (void)events;
    (void)events_chngd;
    (void)p_arg;
}


/*
*********************************************************************************************************
*                                   App_USBD_CDC_SerialLineCoding()
*
* Description : Serial line coding line change notification callback.
*
* Argument(s) : nbr              CDC ACM serial emulation subclass instance number.
*
*               p_line_coding    Pointer to line coding structure.
*
*               p_arg            Callback argument.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  App_USBD_CDC_SerialLineCoding (CPU_INT08U                    nbr,
                                                    USBD_ACM_SERIAL_LINE_CODING  *p_line_coding,
                                                    void                         *p_arg)
{
    (void)nbr;
    (void)p_line_coding;
    (void)p_arg;

    return (DEF_OK);
}

