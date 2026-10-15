/*
 * csi_reg.h
 *
 * Semidrive platform csi controller regs header file
 *
 * Copyright (C) 2021, Semidrive  Communications Inc.
 *
 * This file is licensed under a dual GPLv2 or X11 license.
 */

#ifndef __CSI_REG_H__
#define __CSI_REG_H__

/* all registers in csi ip */

#define CSI_REG_ENABLE    0x00
#define CSI_REG_INT_STAT0 0x04
#define CSI_REG_INT_STAT1 0x08
#define CSI_REG_INT_MASK0 0x0c
#define CSI_REG_INT_MASK1 0x10
#define CSI_REG_REG_LOAD  0x14

/* n: 0 ~ 3, indicates image channel. */
#define CSI_REG_IMG_RGBY_BADDR_H(n)   (0x100 + n * 0x80)
#define CSI_REG_IMG_RGBY_BADDR_L(n)   (0x104 + n * 0x80)
#define CSI_REG_IMG_U_BADDR_H(n)      (0x108 + n * 0x80)
#define CSI_REG_IMG_U_BADDR_L(n)      (0x10c + n * 0x80)
#define CSI_REG_IMG_V_BADDR_H(n)      (0x110 + n * 0x80)
#define CSI_REG_IMG_V_BADDR_L(n)      (0x114 + n * 0x80)
#define CSI_REG_IMG_RGBY_STRIDE(n)    (0x118 + n * 0x80)
#define CSI_REG_IMG_U_STRIDE(n)       (0x11c + n * 0x80)
#define CSI_REG_IMG_V_STRIDE(n)       (0x120 + n * 0x80)
#define CSI_REG_IMG_SIZE(n)           (0x124 + n * 0x80)
#define CSI_REG_IMG_IPI_CTRL(n)       (0x128 + n * 0x80)
#define CSI_REG_IMG_IF_PIXEL_MASK0(n) (0x12c + n * 0x80)
#define CSI_REG_IMG_IF_PIXEL_MASK1(n) (0x130 + n * 0x80)
#define CSI_REG_IMG_CHN_CTRL(n)     (0x134 + n * 0x80)
#define CSI_REG_IMG_CHN_SPLIT0(n)   (0x138 + n * 0x80)
#define CSI_REG_IMG_CHN_SPLIT1(n)   (0x13c + n * 0x80)
#define CSI_REG_IMG_CHN_SPLIT2(n)   (0x140 + n * 0x80)
#define CSI_REG_IMG_CHN_CROP0(n)    (0x144 + n * 0x80)
#define CSI_REG_IMG_CHN_CROP1(n)    (0x148 + n * 0x80)
#define CSI_REG_IMG_CHN_CROP2(n)    (0x14c + n * 0x80)
#define CSI_REG_IMG_CHN_PACK0(n)    (0x150 + n * 0x80)
#define CSI_REG_IMG_CHN_PACK1(n)    (0x154 + n * 0x80)
#define CSI_REG_IMG_CHN_PACK2(n)    (0x158 + n * 0x80)
#define CSI_REG_IMG_CHN_VCROP0(n)   (0x15c + n * 0x80)
#define CSI_REG_IMG_R4R_LINE_CTRL1(n)   (0x160 + n * 0x80)
#define CSI_REG_IMG_R4R_LINE_CTRL2(n)   (0x164 + n * 0x80)

/* n: 0 ~ 7, indicates wdma output channel, max 2 channels for each image */
#define CSI_REG_WDMA_CHN_DFIFO(n)     (0x300 + n * 0x20)
#define CSI_REG_WDMA_CHN_CFIFO(n)     (0x304 + n * 0x20)
#define CSI_REG_WDMA_CHN_AXI_CTRL0(n) (0x308 + n * 0x20)
#define CSI_REG_WDMA_CHN_AXI_CTRL1(n) (0x30c + n * 0x20)
#define CSI_REG_WDMA_CHN_AXI_CTRL2(n) (0x310 + n * 0x20)
#define CSI_REG_WDMA_CHN_STATE(n)     (0x314 + n * 0x20)

/* n: 0 ~ 3, indicates image channel.
 * Actually 1~3 will never be used because only image0
 * can get input from outside of SOC
 */
#define CSI_REG_PARA_BT_CTRL0(n) (0x500 + n * 0x40)
#define CSI_REG_PARA_BT_CTRL1(n) (0x504 + n * 0x40)
#define CSI_REG_PARA_BT_CTRL2(n) (0x508 + n * 0x40)
#define CSI_REG_PIXEL_MAP(n)     (0x50C + n * 0x40)

/* n: 0 ~ 3, indicates image channel. */
#define CSI_REG_CSC_CTRL(n)   (0x800 + n * 0x80)
#define CSI_REG_CSC_COEF1(n)  (0x804 + n * 0x80)
#define CSI_REG_CSC_COEF2(n)  (0x808 + n * 0x80)
#define CSI_REG_CSC_COEF3(n)  (0x80c + n * 0x80)
#define CSI_REG_CSC_COEF4(n)  (0x810 + n * 0x80)
#define CSI_REG_CSC_COEF5(n)  (0x814 + n * 0x80)
#define CSI_REG_CSC_COEF6(n)  (0x818 + n * 0x80)
#define CSI_REG_CSC_COEF7(n)  (0x81c + n * 0x80)
#define CSI_REG_CSC_COEF8(n)  (0x820 + n * 0x80)
#define CSI_REG_UV_UP_CTRL(n) (0x830 + n * 0x80)

enum csi_int0_status {
	CSI_INT0_STORE_DONE0 = (1 << 0),
	CSI_INT0_STORE_DONE1 = (1 << 1),
	CSI_INT0_STORE_DONE2 = (1 << 2),
	CSI_INT0_STORE_DONE3 = (1 << 3),
	CSI_INT0_SHADOW_SET0 = (1 << 4),
	CSI_INT0_SHADOW_SET1 = (1 << 5),
	CSI_INT0_SHADOW_SET2 = (1 << 6),
	CSI_INT0_SHADOW_SET3 = (1 << 7),
	CSI_INT0_BT_ERR   = (1 << 8),
	CSI_INT0_BT_FATAL = (1 << 9),
	CSI_INT0_BT_FIELD_CHANGE = (1 << 10),
	CSI_INT0_BT_ERR1   = (1 << 11),
	CSI_INT0_BT_FATAL1 = (1 << 12),
	CSI_INT0_BT_FIELD_CHANGE1 = (1 << 13),
	CSI_INT0_R4R_IMG0_DONE0 = (1 << 16),
	CSI_INT0_R4R_IMG0_DONE1 = (1 << 17),
	CSI_INT0_R4R_IMG0_DONE2 = (1 << 18),
	CSI_INT0_R4R_IMG0_DONE3 = (1 << 19),
	CSI_INT0_R4R_IMG1_DONE0 = (1 << 20),
	CSI_INT0_R4R_IMG1_DONE1 = (1 << 21),
	CSI_INT0_R4R_IMG1_DONE2 = (1 << 22),
	CSI_INT0_R4R_IMG1_DONE3 = (1 << 23),
	CSI_INT0_R4R_IMG2_DONE0 = (1 << 24),
	CSI_INT0_R4R_IMG2_DONE1 = (1 << 25),
	CSI_INT0_R4R_IMG2_DONE2 = (1 << 26),
	CSI_INT0_R4R_IMG2_DONE3 = (1 << 27),
	CSI_INT0_R4R_IMG3_DONE0 = (1 << 28),
	CSI_INT0_R4R_IMG3_DONE1 = (1 << 29),
	CSI_INT0_R4R_IMG3_DONE2 = (1 << 30),
	CSI_INT0_R4R_IMG3_DONE3 = (1 << 31),
};

enum csi_int1_status {
	CSI_INT1_CROP_ERR0 = (1 << 0),
	CSI_INT1_CROP_ERR1 = (1 << 1),
	CSI_INT1_CROP_ERR2 = (1 << 2),
	CSI_INT1_CROP_ERR3 = (1 << 3),
	CSI_INT1_PIXEL_ERR0 = (1 << 4),
	CSI_INT1_PIXEL_ERR1 = (1 << 5),
	CSI_INT1_PIXEL_ERR2 = (1 << 6),
	CSI_INT1_PIXEL_ERR3 = (1 << 7),
	CSI_INT1_OVERFLOW0  = (1 << 8),
	CSI_INT1_OVERFLOW1  = (1 << 9),
	CSI_INT1_OVERFLOW2  = (1 << 10),
	CSI_INT1_OVERFLOW3  = (1 << 11),
	CSI_INT1_IMG0_BUSERR0  = (1 << 12),
	CSI_INT1_IMG0_BUSERR1  = (1 << 13),
	CSI_INT1_IMG1_BUSERR0  = (1 << 14),
	CSI_INT1_IMG1_BUSERR1  = (1 << 15),
	CSI_INT1_IMG2_BUSERR0  = (1 << 16),
	CSI_INT1_IMG2_BUSERR1  = (1 << 17),
	CSI_INT1_IMG3_BUSERR0  = (1 << 18),
	CSI_INT1_IMG3_BUSERR1  = (1 << 19),
};

#define INT0_ERR_MASK(n) \
	((CSI_INT0_BT_ERR << (n * 3)) | (CSI_INT0_BT_FATAL << (n * 3)))

#define INT0_MASK(n)    (CSI_INT0_SHADOW_SET0 << n)
#define INT0_BT_MASK(n) (INT0_ERR_MASK(n) | (CSI_INT0_SHADOW_SET0 << n))

#define INT0_ERR_MASK_ALL \
	(INT0_ERR_MASK(0) | INT0_ERR_MASK(1) | \
	 INT0_ERR_MASK(2) | INT0_ERR_MASK(3))

#define INT1_ERR_MASK(n) \
	(((CSI_INT1_CROP_ERR0 | CSI_INT1_PIXEL_ERR0 | CSI_INT1_OVERFLOW0) << n) | \
	((CSI_INT1_IMG0_BUSERR0 | CSI_INT1_IMG0_BUSERR1) << (n * 2)))

#define INT1_ERR_MASK_ALL \
	(INT1_ERR_MASK(0) | INT1_ERR_MASK(1) | \
	 INT1_ERR_MASK(2) | INT1_ERR_MASK(3))

#endif /* __CSI_REG_H__ */