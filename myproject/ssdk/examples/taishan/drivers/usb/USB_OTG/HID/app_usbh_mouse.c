/**
 * @file app_usbh_mouse.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: usb host Mouse test & demo application.
 *
 * Revision History:
 * -----------------
 */


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/


#include  "app_usbh_mouse.h"


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  MOUSE_BUTTON_1                                 0x01u
#define  MOUSE_BUTTON_2                                 0x02u
#define  MOUSE_BUTTON_3                                 0x04u


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

static  CPU_INT08U  App_USBH_MouseStatePrev = 0u;
static  CPU_INT32S  App_USBH_MousePosX      = 0u;
static  CPU_INT32S  App_USBH_MousePosY      = 0u;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  App_USBH_MouseDisplayData (CPU_INT08S  *p_buf);


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
*********************************************************************************************************
*                                      App_USBH_MouseCallBack()
*
* Description : Callback function for mouse events.
*
* Argument(s) : p_hid_dev       Pointer to the HID class device.
*
*               p_buf           Pointer to the data sent by the keyboard.
*
*               buf_len         Buffer length.
*
*               err             Status of the callback.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  App_USBH_MouseCallBack (USBH_HID_DEV  *p_hid_dev,
                              void          *p_buf,
                              CPU_INT08U     buf_len,
                              USBH_ERR       err)
{
    (void)p_hid_dev;

    if ((err     == USBH_ERR_NONE) &&
        (buf_len >= 3u)) {
        App_USBH_MouseDisplayData(p_buf);
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     App_USBH_MouseDisplayData()
*
* Description : Print buttons pressed & pointer location.
*
* Argument(s) : pbuf            Buffer pointer.
*
* Return(s)   : None.
*
* Note(s)     : This function parses the report buffer of a simple/basic mouse. It might require to be
*               modified when used with a complex mouse
*********************************************************************************************************
*/

static  void  App_USBH_MouseDisplayData (CPU_INT08S  *p_buf)
{
    CPU_INT08U   state;
    CPU_BOOLEAN  pressed_prev;
    CPU_BOOLEAN  pressed;


    state        = (CPU_INT08U)p_buf[0];
    pressed      = (state                   & MOUSE_BUTTON_1) == MOUSE_BUTTON_1 ? DEF_TRUE : DEF_FALSE;
    pressed_prev = (App_USBH_MouseStatePrev & MOUSE_BUTTON_1) == MOUSE_BUTTON_1 ? DEF_TRUE : DEF_FALSE;

    if ((pressed      == DEF_TRUE) &&
        (pressed_prev == DEF_FALSE)) {
        printf("Left   button pressed\n");
    }
    if ((pressed      == DEF_FALSE) &&
        (pressed_prev == DEF_TRUE)) {
        printf("Left   button released\n");
    }

    pressed      = (state                   & MOUSE_BUTTON_3) == MOUSE_BUTTON_3 ? DEF_TRUE : DEF_FALSE;
    pressed_prev = (App_USBH_MouseStatePrev & MOUSE_BUTTON_3) == MOUSE_BUTTON_3 ? DEF_TRUE : DEF_FALSE;

    if ((pressed      == DEF_TRUE) &&
        (pressed_prev == DEF_FALSE)) {
        printf("Middle button pressed\n");
    }
    if ((pressed      == DEF_FALSE) &&
        (pressed_prev == DEF_TRUE)) {
        printf("Middle button released\n");
    }

    pressed      = (state                   & MOUSE_BUTTON_2) == MOUSE_BUTTON_2 ? DEF_TRUE : DEF_FALSE;
    pressed_prev = (App_USBH_MouseStatePrev & MOUSE_BUTTON_2) == MOUSE_BUTTON_2 ? DEF_TRUE : DEF_FALSE;

    if ((pressed      == DEF_TRUE) &&
        (pressed_prev == DEF_FALSE)) {
        printf("Right  button pressed\n");
    }
    if ((pressed      == DEF_FALSE) &&
        (pressed_prev == DEF_TRUE)) {
        printf("Right  button released\n");
    }

    App_USBH_MouseStatePrev = state;

    if ((p_buf[1u] != 0u) ||
        (p_buf[2u] != 0u)) {
        App_USBH_MousePosX += p_buf[1u];
        App_USBH_MousePosY += p_buf[2u];

        printf("Pointer at (x, y) = (%d, %d)\n",
                        App_USBH_MousePosX,
                        App_USBH_MousePosY);
    }
}

/*
*********************************************************************************************************
*                                                  END
*********************************************************************************************************
*/
