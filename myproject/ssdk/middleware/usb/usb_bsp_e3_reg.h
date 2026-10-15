/**
 * @file usb_bsp_e3_reg.h
 * @brief usb board support header file.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef USB_BSP_E3_REG_H
#define USB_BSP_E3_REG_H


#include <usb_bsp.h>


// USB components base address
#define  USB_HOST_BASE_ADDR     DEVICE_BASE(USB0H)
#define  USB_OTG_BASE_ADDR      DEVICE_BASE(USB0D)
#define  USB_OTGNC_BASE_ADDR    (USB_OTG_BASE_ADDR + 0x800)
#define  USB_PHYNC_BASE_ADDR    (USB_OTGNC_BASE_ADDR + 0x100)


//OTG NCR
#define OTG_NCR_CTL0            (USB_OTGNC_BASE_ADDR + 0x0)
#define OTG_CTL0_AHB_INTEN      (1<<0)
#define OTG_CTL0_OHCI_INTEN     (1<<1)
#define OTG_CTL0_EHCI_INTEN     (1<<2)
#define OTG_CTL0_WKON_INTEN     (1<<4)  //EHCI wakeup event
#define OTG_CTL0_OB_INTEN       (1<<5)  //OTG or battery-charging interrupt signal
#define OTG_CTL0_IXL_INTEN      (1<<7)
#define OTG_CTL0_DMA0_INTEN     (1<<8)
#define OTG_CTL0_DMA1_INTEN     (1<<9)
#define OTG_CTL0_DMAERR_INTEN   (1<<10)
#define OTG_CTL0_PINT_EN        ((1<<7) | (3<<8) | (1<<10))
/* BIND_INT is logical or of others. ULPI is not used. */
#define OTG_CTL0_HINT_EN        ((1<<0) | (1<<1) | (1<<2) | (1<<4) | (1<<5))

#define OTG_NCR_CTL1            (USB_OTGNC_BASE_ADDR + 0x4)
#define OTG_CTL1_RSVD0_POS      (18)
#define OTG_CTL1_RSVD0_MASK     (0x1FFF << OTG_CTL1_RSVD0_POS)
#define OTG_CTL1_EXT_OCINV      (1u << 16)

#define OTG_NCR_STS7            (USB_OTGNC_BASE_ADDR + 0x5C)
#define OTG_STS7_AHB_INT        (1<<0)
#define OTG_STS7_OHCI_INT       (1<<1)
#define OTG_STS7_EHCI_INT       (1<<2)
#define OTG_STS7_WKON_INT       (1<<4)
#define OTG_STS7_OB_INT         (1<<5)
#define OTG_STS7_IXL_INT        (1<<7)
#define OTG_STS7_DMA0_INT       (1<<8)
#define OTG_STS7_DMA1_INT       (1<<9)
#define OTG_STS7_DMAERR_INT     (1<<10)

#define PHY_NCR_STS1            (USB_PHYNC_BASE_ADDR + 0x44)
#define PHY_STS_PLLLOCK         (1u << 24)


#define USBD_BUSWAIT            (USB_OTG_BASE_ADDR + 0x02)

#define USBD_DCTRL              (USB_OTG_BASE_ADDR + 0x700)
#define DCTRL_PR_ROUND_ROBIN    (1u << 0)
#define DCTRL_INT_LVL           (1u << 1)


#define USBH_HCRHPORTSTATUS     (USB_HOST_BASE_ADDR + 0x54)
#define HCRHPORTSTATUS_PPS      (1u << 8)
#define HCRHPORTSTATUS_OCIC     (1u << 19)

#define USBH_PORTSC             (USB_HOST_BASE_ADDR + 0x164)
#define PORTSC_CSC              (1u << 1)
#define PORTSC_PEC              (1u << 3)
#define PORTSC_OCC              (1u << 5)
#define PORTSC_PP               (1u << 12)

#define USBH_USBCTR             (USB_HOST_BASE_ADDR + 0x20C)
#define USBCTR_USBH_RST         (1u << 0)

#define USBH_INT_ENABLE         (USB_HOST_BASE_ADDR + 0x200)
#define INT_ENABLE_AHB_INTEN    (1u << 0)
#define INT_ENABLE_USBH_INTAEN  (1u << 1)
#define INT_ENABLE_USBH_INTBEN  (1u << 2)
#define INT_ENABLE_UCOM_INTEN   (1u << 3)
#define INT_ENABLE_WAKEON_INTEN (1u << 4)
#define INT_ENABLE_ULPI_INTEN   (1u << 5)

#define USBH_INT_STATUS         (USB_HOST_BASE_ADDR + 0x204)
#define INT_STATUS_AHB_INT      (1u << 0)
#define INT_STATUS_USBH_INTA    (1u << 1)
#define INT_STATUS_USBH_INTB    (1u << 2)
#define INT_STATUS_UCOM_INT     (1u << 3)
#define INT_STATUS_WAKEON_INT   (1u << 4)
#define INT_STATUS_ULPI_INT     (1u << 5)

#define USBH_AHB_BUS_CTR        (USB_HOST_BASE_ADDR + 0x208)

#define USBH_REGEN_CG_CTRL      (USB_HOST_BASE_ADDR + 0x304)
#define REGEN_CG_CTRL_UPHY_WEN          (1u << 0)
#define REGEN_CG_CTRL_PERI_CLK_MSK      (1u << 28)
#define REGEN_CG_CTRL_HOST_CLK_MSK      (1u << 29)
#define REGEN_CG_CTRL_NONUSE_CLK_MSK    (1u << 31)

#define USBH_UTMI_CTRL          (USB_HOST_BASE_ADDR + 0x318)
#define UTMI_CTRL_LS_TXDAT_INV_EN       (1u << 1)

#define USBH_SPD_CTRL           (USB_HOST_BASE_ADDR + 0x308)
#define SPD_CTRL_GLB_SUSP_P1            (1u << 0)
#define SPD_CTRL_WKCNNT_ENABLE          (1u << 23)
#define SPD_CTRL_SLEEPM_ENABLE          (1u << 30)
#define SPD_CTRL_SUSPENDM_ENABLE        (1u << 31)

#define USBH_COMMCTRL           (USB_HOST_BASE_ADDR + 0x800)
#define COMMCTRL_OTG_PERI       (1u << 31)

#define USBH_OBINTSTA           (USB_HOST_BASE_ADDR + 0x804)
#define OBINTSTA_IDDIGCHG       (1u << 11)

#define USBH_OBINTEN            (USB_HOST_BASE_ADDR + 0x808)
#define OBINTEN_IDDIGCHG        (1u << 11)

#define USBH_VBCTRL             (USB_HOST_BASE_ADDR + 0x80C)
#define VBCTRL_VBOUT            (1u << 0)

#define USBH_ADP_CTRL           (USB_HOST_BASE_ADDR + 0x830)
#define ADP_CTRL_ID_PULLUP      (1u << 5)
#define ADP_CTRL_IDDIG          (1u << 19)


#endif
