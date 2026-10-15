/**
 * @file usb_bsp_e3.c
 * @brief usb board support source file.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <types.h>
#include <arch/barriers.h>
#include <driver/irq.h>
#include <reg.h>
#include "usb_bsp_e3_reg.h"


#define  USB_BSP_LOG_EN         0


struct usb_irq {
    void (*handler)(void *arg);
    void *arg;
};

enum usb_irq_id {
    USB_IRQ_OHCI = 0,
    USB_IRQ_EHCI,
    USB_IRQ_PERI,
    USB_IRQ_DMA0,
    USB_IRQ_DMA1,
    USB_IRQ_OTG,
    USB_IRQ_NUM
};


static struct usb_irq usb_irq_table[USB_IRQ_NUM];
static volatile uint32_t usb_int_sts_msk;


static int usb_handler(uint32_t irq, void *arg)
{
    uint32_t int_sts;

    int_sts = readl(OTG_NCR_STS7);
    int_sts &= usb_int_sts_msk;

    if (int_sts & (OTG_STS7_IXL_INT | OTG_STS7_DMA0_INT | OTG_STS7_DMA0_INT)) {
        if (usb_irq_table[USB_IRQ_PERI].handler)
            usb_irq_table[USB_IRQ_PERI].handler(usb_irq_table[USB_IRQ_PERI].arg);
    }

    if (int_sts & OTG_STS7_EHCI_INT) {
        if (usb_irq_table[USB_IRQ_EHCI].handler)
            usb_irq_table[USB_IRQ_EHCI].handler(usb_irq_table[USB_IRQ_EHCI].arg);
    }

    if (int_sts & OTG_STS7_OHCI_INT) {
        if (usb_irq_table[USB_IRQ_OHCI].handler)
            usb_irq_table[USB_IRQ_OHCI].handler(usb_irq_table[USB_IRQ_OHCI].arg);
    }

    if (int_sts & OTG_STS7_OB_INT) {
        if (usb_irq_table[USB_IRQ_OTG].handler)
            usb_irq_table[USB_IRQ_OTG].handler(usb_irq_table[USB_IRQ_OTG].arg);
    }

    if (int_sts & OTG_STS7_AHB_INT) {
        printf("unexpected usb AHB Bus Error %X\n", int_sts);
        CLRSETBITS_32(USBH_INT_STATUS,
                      INT_STATUS_ULPI_INT | INT_STATUS_WAKEON_INT,
                      INT_STATUS_AHB_INT);
    }

    if (int_sts & (OTG_STS7_WKON_INT |
                   OTG_STS7_DMAERR_INT)) {
        printf("unexpected usb int %X\n", int_sts);
    }

    return 0;
}

void usb_irq_init(void)
{
    usb_int_sts_msk = OTG_STS7_AHB_INT | OTG_STS7_WKON_INT | OTG_STS7_DMAERR_INT;
    SETBITS_32(OTG_NCR_CTL0, OTG_CTL0_AHB_INTEN | OTG_CTL0_WKON_INTEN | OTG_CTL0_DMAERR_INTEN);

    irq_attach(DEVICE_INTR(USB0), usb_handler, NULL);
#ifndef CONFIG_USB_OTG
    irq_enable(DEVICE_INTR(USB0));
#endif

    return;
}

void usb_bsp_wait_pll_lock(void)
{
#if USB_BSP_LOG_EN
    uint64_t start_us;

    start_us = current_time_hires();
#endif

    while (!(readl(PHY_NCR_STS1) & PHY_STS_PLLLOCK)) {
        /* Wait for PLL Lock */
    }

#if USB_BSP_LOG_EN
    printf("hw_usb_wait_pll_lock from %lld to %lld\n", start_us, current_time_hires());
#endif

    if (!(readl(PHY_NCR_STS1) & PHY_STS_PLLLOCK))
        printf("PHY_NCR_STS1 status %X\n", readl(PHY_NCR_STS1));
}

#ifdef  CONFIG_USB_DEVICE

/*
*********************************************************************************************************
*                                         USBD_BSP_InitUSB0()
*
* Description : USB device controller board-specific initialization.
*
* Argument(s) : p_drv       Pointer to device driver structure.
*
* Return(s)   : none.
*
* Caller(s)   : Device controller driver init function via 'p_drv_api->Init()'
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  USBD_BSP_InitUSB0 (USBD_DRV *p_drv)
{
    SETBITS_32(USBH_COMMCTRL, COMMCTRL_OTG_PERI);       /* 0 : Host, 1 : Peri */
    writew((USB_CFG_BUSWAIT & 0x003f), USBD_BUSWAIT);
    SETBITS_32(USBD_DCTRL, DCTRL_PR_ROUND_ROBIN | DCTRL_INT_LVL);

    usb_irq_table[USB_IRQ_PERI].handler = (CPU_FNCT_PTR)p_drv->API_Ptr->ISR_Handler;
    usb_irq_table[USB_IRQ_PERI].arg = p_drv;

    return;
}

/*
*********************************************************************************************************
*                                         USBD_BSP_ConnUSB0()
*
* Description : Enable USB0 interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Device controller driver start function via 'p_drv_api->Conn()'
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  USBD_BSP_ConnUSB0 (void)
{
    setbits_32(&usb_int_sts_msk, OTG_STS7_IXL_INT | OTG_STS7_DMA0_INT | OTG_STS7_DMA0_INT);
    SETBITS_32(OTG_NCR_CTL0, OTG_CTL0_IXL_INTEN | OTG_CTL0_DMA0_INTEN | OTG_CTL0_DMA1_INTEN);

    return;
}

/*
*********************************************************************************************************
*                                       USBD_BSP_DisconnUSB0()
*
* Description : Disable USB0 interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Device controller driver stop function via 'p_drv_api->Disconn()'
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  USBD_BSP_DisconnUSB0 (void)
{
    CLRBITS_32(OTG_NCR_CTL0, OTG_CTL0_IXL_INTEN | OTG_CTL0_DMA0_INTEN | OTG_CTL0_DMA1_INTEN);
    clrbits_32(&usb_int_sts_msk, OTG_STS7_IXL_INT | OTG_STS7_DMA0_INT | OTG_STS7_DMA0_INT);
}

USBD_DRV_BSP_API  USBD_DrvBSP_TAISHAN_USB0 = {
    USBD_BSP_InitUSB0,
    USBD_BSP_ConnUSB0,
    USBD_BSP_DisconnUSB0
};

#endif


#ifdef  CONFIG_USB_HOST

/* Enable write to USBH_UTMI_CTRL */
static inline void usbh_uphy_write_en(bool en)
{
    if (en)
        SETBITS_32(USBH_REGEN_CG_CTRL, REGEN_CG_CTRL_UPHY_WEN);
    else
        CLRBITS_32(USBH_REGEN_CG_CTRL, REGEN_CG_CTRL_UPHY_WEN);
}

static  void  USB0H_BSP_Init (void)
{
    static bool hci_inited = 0;

    if (hci_inited)
        return;
    hci_inited = 1;

#if USB_OC_INVERT
    SETBITS_32(OTG_NCR_CTL1, OTG_CTL1_EXT_OCINV);
#endif
    SETBITS_32(USBH_USBCTR, USBCTR_USBH_RST);
    udelay(2);

#ifdef CONFIG_USB_OTG
    SETBITS_32(USBH_ADP_CTRL, ADP_CTRL_ID_PULLUP);
#else
    SETBITS_32(USBH_VBCTRL, VBCTRL_VBOUT);
#endif

    CLRBITS_32(USBH_COMMCTRL, COMMCTRL_OTG_PERI);       /* 0 : Host, 1 : Peri */
    usb_bsp_wait_pll_lock();

    writel(INT_ENABLE_AHB_INTEN | INT_ENABLE_UCOM_INTEN | INT_ENABLE_WAKEON_INTEN |
            INT_ENABLE_USBH_INTAEN | INT_ENABLE_USBH_INTBEN,
            USBH_INT_ENABLE);
    writel(0, USBH_AHB_BUS_CTR);

    CLRSETBITS_32(USBH_REGEN_CG_CTRL,
                  REGEN_CG_CTRL_PERI_CLK_MSK | REGEN_CG_CTRL_HOST_CLK_MSK,
                  REGEN_CG_CTRL_NONUSE_CLK_MSK);

    usbh_uphy_write_en(1);
    DSB;
    CLRBITS_32(USBH_UTMI_CTRL, UTMI_CTRL_LS_TXDAT_INV_EN);
    DSB;
    usbh_uphy_write_en(0);

    writel(SPD_CTRL_SUSPENDM_ENABLE | SPD_CTRL_WKCNNT_ENABLE, USBH_SPD_CTRL);

    return;
}

static  void  USB0H_BSP_EHCI_Init ( USBH_HC_DRV   *p_hc_drv,
                                    USBH_ERR      *p_err)
{
    usb_irq_table[USB_IRQ_EHCI].arg = p_hc_drv;

    USB0H_BSP_Init();
    *p_err = USBH_ERR_NONE;
}

static  void  USB0H_BSP_OHCI_Init ( USBH_HC_DRV   *p_hc_drv,
                                    USBH_ERR      *p_err)
{
    usb_irq_table[USB_IRQ_OHCI].arg = p_hc_drv;

    USB0H_BSP_Init();
    *p_err = USBH_ERR_NONE;
}

static  void  USB0H_BSP_EHCI_ISR_Reg (  CPU_FNCT_PTR    isr_fnct,
                                        USBH_ERR        *p_err)
{
    usb_irq_table[USB_IRQ_EHCI].handler = isr_fnct;

    setbits_32(&usb_int_sts_msk, OTG_STS7_EHCI_INT);
    SETBITS_32(OTG_NCR_CTL0, OTG_CTL0_EHCI_INTEN);

    *p_err = USBH_ERR_NONE;
}

static  void  USB0H_BSP_OHCI_ISR_Reg (  CPU_FNCT_PTR    isr_fnct,
                                        USBH_ERR        *p_err)
{
    usb_irq_table[USB_IRQ_OHCI].handler = isr_fnct;

    setbits_32(&usb_int_sts_msk, OTG_STS7_OHCI_INT);
    SETBITS_32(OTG_NCR_CTL0, OTG_CTL0_OHCI_INTEN);

    *p_err = USBH_ERR_NONE;
}

static  void  USB0H_BSP_EHCI_ISR_Unreg (USBH_ERR    *p_err)
{
    CLRBITS_32(OTG_NCR_CTL0, OTG_CTL0_EHCI_INTEN);
    clrbits_32(&usb_int_sts_msk, OTG_STS7_EHCI_INT);

    *p_err = USBH_ERR_NONE;
}

static  void  USB0H_BSP_OHCI_ISR_Unreg (USBH_ERR    *p_err)
{
    CLRBITS_32(OTG_NCR_CTL0, OTG_CTL0_OHCI_INTEN);
    clrbits_32(&usb_int_sts_msk, OTG_STS7_OHCI_INT);

    *p_err = USBH_ERR_NONE;
}

USBH_HC_BSP_API  USB0H_EHCI_BSP_TAISHAN =
{
    USB0H_BSP_EHCI_Init,
    USB0H_BSP_EHCI_ISR_Reg,
    USB0H_BSP_EHCI_ISR_Unreg
};

USBH_HC_BSP_API  USB0H_OHCI_BSP_TAISHAN =
{
    USB0H_BSP_OHCI_Init,
    USB0H_BSP_OHCI_ISR_Reg,
    USB0H_BSP_OHCI_ISR_Unreg
};

#endif

#ifdef  CONFIG_USB_OTG
void USB0_BSP_OTG_ISR_Reg (CPU_FNCT_PTR isr_fnct, void *arg)
{
    usb_irq_table[USB_IRQ_OTG].handler = isr_fnct;
    usb_irq_table[USB_IRQ_OTG].arg = arg;

    setbits_32(&usb_int_sts_msk, OTG_STS7_OB_INT);
    SETBITS_32(OTG_NCR_CTL0, OTG_CTL0_OB_INTEN);
}
#endif
