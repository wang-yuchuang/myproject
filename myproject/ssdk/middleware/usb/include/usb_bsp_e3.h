/**
 * @file usb_bsp_e3.h
 * @brief usb board support header file.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef USB_BSP_E3_H
#define USB_BSP_E3_H


#include <board.h>


#ifndef APB_USB0H_BASE
#define APB_USB0H_BASE          APB_USB_BASE
#define APB_USB0D_BASE          (APB_USB0H_BASE + 0x1000u)
#endif

#ifndef USB0_INTR_NUM
#define USB0_INTR_NUM           USB_INTR_NUM
#endif


void usb_irq_init(void);
void usb_bsp_wait_pll_lock(void);


#ifdef  CONFIG_USB_DEVICE

#include <Drivers/Renesas_USBHS/usbd_drv_renesas_usbhs.h>


extern  USBD_DRV_BSP_API  USBD_DrvBSP_TAISHAN_USB0;


static inline void USBD_BSP_DlyUs(CPU_INT32U us)
{
    udelay(us);
}

#endif

#ifdef  CONFIG_USB_HOST

#include <HCD/EHCI/usbh_hcd_ehci.h>
#include <HCD/OHCI/usbh_hcd_ohci.h>


#define  RUSB_OHCI_OFFSET       0x0u
#define  RUSB_EHCI_OFFSET       0x100u


extern  USBH_HC_BSP_API  USB0H_EHCI_BSP_TAISHAN;
extern  USBH_HC_BSP_API  USB0H_OHCI_BSP_TAISHAN;

#endif


#ifdef  CONFIG_USB_OTG
void USB0_BSP_OTG_ISR_Reg (CPU_FNCT_PTR isr_fnct, void *arg);
#endif


#endif
