/**
 * @file usb_otg_e3.c
 * @brief usb board support source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <types.h>
#include <debug.h>
#include <driver/irq.h>
#include <reg.h>
#include "usb_bsp_e3_reg.h"
#include  <Source/usbd_core.h>
#include  <usb_otg.h>


#define  otg_err(...)                   ssdk_printf(SSDK_ERR, __VA_ARGS__)
#define  otg_log(...)                   ssdk_printf(SSDK_INFO, __VA_ARGS__)


static osSemaphoreId_t ob_int_sem;


static void otg_isr(void *arg)
{
    writel(0, USBH_OBINTEN);
    osSemaphoreRelease(ob_int_sem);
}

static void otg_hdl_func(void *arg)
{
    CPU_INT08U  otg_nbr_dev;
    uint32_t    iddig;
    int         cnt;
    USBD_ERR    derr;
    osStatus_t  status;

    otg_nbr_dev = (CPU_INT08U)(int)arg;
    while (1) {

        status = osSemaphoreAcquire(ob_int_sem, osWaitForever);
         if (status != osOK) {
            continue;
        }

        osDelay(2);

        cnt = 0;
        iddig = readl(USBH_ADP_CTRL) & ADP_CTRL_IDDIG;
        do {
            udelay(10);
            if (iddig == (readl(USBH_ADP_CTRL) & ADP_CTRL_IDDIG)) {
                cnt++;
            } else {
                cnt = 0;
                iddig = readl(USBH_ADP_CTRL) & ADP_CTRL_IDDIG;
            }
        } while (cnt < 4);

        writel(OBINTSTA_IDDIGCHG, USBH_OBINTSTA);
        if (readl(USBH_ADP_CTRL) & ADP_CTRL_IDDIG) {
            if (!(readl(USBH_COMMCTRL) & COMMCTRL_OTG_PERI)) {
                /* Host to Device */
                otg_log("OTG->D\n");
                CLRBITS_32(USBH_VBCTRL, VBCTRL_VBOUT);

                osDelay(120);
                SETBITS_32(USBH_COMMCTRL, COMMCTRL_OTG_PERI);
                USBD_DevStart(otg_nbr_dev, &derr);
                if (derr != USBD_ERR_NONE) {
                    otg_err("    ... could not start device stack w/err =  %d\n\n", derr);
                }
            }
        } else {
            if (readl(USBH_COMMCTRL) & COMMCTRL_OTG_PERI) {
                /* Device to Host */
                otg_log("OTG->H\n");
                USBD_DevStop(otg_nbr_dev, &derr);
                if (derr != USBD_ERR_NONE) {
                    otg_err("    ... could not stop device stack w/err =  %d\n\n", derr);
                }

                osDelay(10);

                CLRBITS_32(USBH_COMMCTRL, COMMCTRL_OTG_PERI);
                if (readl(USBH_HCRHPORTSTATUS) & HCRHPORTSTATUS_OCIC) {
                    writel(HCRHPORTSTATUS_OCIC, USBH_HCRHPORTSTATUS);
                }
                if (!(readl(USBH_HCRHPORTSTATUS) & HCRHPORTSTATUS_PPS)) {
                    writel(HCRHPORTSTATUS_PPS, USBH_HCRHPORTSTATUS);
                }
                if (readl(USBH_PORTSC) & PORTSC_OCC) {
                    CLRSETBITS_32(USBH_PORTSC, PORTSC_CSC | PORTSC_PEC, PORTSC_OCC);
                }
                if (!(readl(USBH_PORTSC) & PORTSC_PP)) {
                    SETBITS_32(USBH_PORTSC, PORTSC_PP);
                }
                SETBITS_32(USBH_VBCTRL, VBCTRL_VBOUT);
            }
        }

        writel(OBINTEN_IDDIGCHG, USBH_OBINTEN);
    }
}

void USB_OTG_Start(CPU_INT08U otg_ctl_nbr, CPU_INT08U otg_nbr_dev)
{
    USBD_ERR        err;
    osThreadId_t    thread;
    osThreadAttr_t  attr = {
        .name = "USB OTG HDL",
        .stack_size = 2048,
        .priority = osPriorityNormal,
    };

    writel(OBINTSTA_IDDIGCHG, USBH_OBINTSTA);
    if (readl(USBH_ADP_CTRL) & ADP_CTRL_IDDIG) {
        /* Device */
        CLRBITS_32(USBH_VBCTRL, VBCTRL_VBOUT);
        SETBITS_32(USBH_COMMCTRL, COMMCTRL_OTG_PERI);

        USBD_DevStart(otg_nbr_dev, &err);
        if (err != USBD_ERR_NONE) {
            otg_err("    ... could not start device stack w/err =  %d\n\n", err);
        }
    } else {
        /* Host */
        CLRBITS_32(USBH_COMMCTRL, COMMCTRL_OTG_PERI);
        SETBITS_32(USBH_VBCTRL, VBCTRL_VBOUT);
    }

    ob_int_sem = osSemaphoreNew(1, 0, NULL);
    if (!ob_int_sem) {
        otg_err("create otg sem fail\n");
    } else {
        thread = osThreadNew(otg_hdl_func, (void *)(int)otg_nbr_dev, &attr);
        if (!thread) {
            otg_err("create otg thread fail\n");
        } else {
            USB0_BSP_OTG_ISR_Reg(otg_isr, NULL);

            writel(OBINTEN_IDDIGCHG, USBH_OBINTEN);
        }
    }

    irq_enable(DEVICE_INTR(USB0));
}
