/*
* driver/taishan/dispss/lvds_reg.h
*
* Copyright (c) 2018-2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: lvds register defs file.
*
* Revision History:
* -----------------
* 011, 11/12/2021 BILLY create this file
*/
#ifndef LVDS_REG_H__
#define LVDS_REG_H__

#define LVDS_BASE                   0xF34C0000
/* LVDS registers (RMW mode) definition */

#define LVDS_CH0_CTRL               (0x1000)
#define CH0_EN_SHIFT                31
#define CH0_EN_MASK                 0x1 << CH0_EN_SHIFT
#define CH0_MUX_SHIFT               23
#define CH0_MUX_MASK                0x3 << CH0_MUX_SHIFT
#define CH0_DUALODD_SHIFT           22
#define CH0_DUALODD_MASK            0x1 << CH0_DUALODD_SHIFT
#define CH0_LANE_UPDATE_SHIFT       7
#define CH0_LANE_UPDATE_MASK        0x7FFF << CH0_LANE_UPDATE_SHIFT
#define CH0_FRAME_MASK_SHIFT        5
#define CH0_FRAME_MASK_MASK         0x3 << CH0_FRAME_MASK_SHIFT
#define CH0_VSYNC_POL_SHIFT         4
#define CH0_VSYNC_POL_MASK          0x1 << CH0_VSYNC_POL_SHIFT
#define CH0_FORMAT_SHIFT            3
#define CH0_FORMAT_MASK             0x1 << CH0_FORMAT_SHIFT
#define CH0_BPP_SHIFT               1
#define CH0_BPP_MASK                0x3 << CH0_BPP_SHIFT
#define CH0_DUALMODE_SHIFT          0
#define CH0_DUALMODE_MASK           0x1 << CH0_DUALMODE_SHIFT

/* SOFT RESET */
#define LVDS_SOFT_RESET             (0x1008)
#define CH0_SOFT_RESET_SHIFT        0
#define CH0_SOFT_RESET_MASK         0x1 << CH0_SOFT_RESET_SHIFT

#define LVDS_TEST_CFG               (0x100c)
#define LVDS_TEST_CLK_SHIFT         8
#define LVDS_TEST_CLK_MASK          0x7F << LVDS_TEST_CLK_SHIFT
#define LVDS_TEST_EN_SHIFT          7
#define LVDS_TEST_EN_MASK           0x1 << LVDS_TEST_EN_SHIFT
#define LVDS_TEST_DATA_SHIFT        0
#define LVDS_TEST_DATA_MASK         0x7F << LVDS_TEST_DATA_MASK

#define LVDS_CH0_PAD_SET_(i)        (0x1010 + 0x4 * i)
#define CH0_RXD_N_SHIFT             26
#define CH0_RXD_N_MASK              0x1 << CH0_RXD_N_SHIFT
#define CH0_RXD_P_SHIFT             25
#define CH0_RXD_P_MASK              0x1 << CH0_RXD_P_SHIFT
#define CH0_RXDA_SHIFT              24
#define CH0_RXDA_MASK               0x1 << CH0_RXDA_SHIFT
#define CH0_DTEST_OUT_SHIFT         23
#define CH0_DTEST_OUT_MASK          0x1 << CH0_DTEST_OUT_SHIFT
#define CH0_ATEST_SEL_SHIFT         16
#define CH0_ATEST_SEL_MASK          0x7 << CH0_ATEST_SEL_SHIFT
#define CH0_DTEST_SEL_SHIFT         14
#define CH0_DTEST_SEL_MASK          0x3 << CH0_DTEST_SEL_SHIFT
#define CH0_TXVCOM_SHIFT            8
#define CH0_TXVCOM_MASK             0x1 << CH0_TXVCOM_SHIFT  ///
#define CH0_TXEN_SHIFT              7
#define CH0_TXEN_MASK               0x1 << CH0_TXEN_SHIFT
#define CH0_RXEN_SHIFT              6
#define CH0_RXEN_MASK               0x1 << CH0_RXEN_SHIFT
#define CH0_TXSWING_SHIFT           3
#define CH0_TXSWING_MASK            0x1 << CH0_TXSWING_SHIFT
#define CH0_SUBLVDS_SHIFT           2
#define CH0_SUBLVDS_MASK            0x1 << CH0_SUBLVDS_SHIFT
#define CH0_RTERM_EN_SHIFT          1
#define CH0_RTERM_EN_MASK           0x1 << CH0_RTERM_EN_SHIFT


#define LVDS_CH0_PAD_COM_SET_(i)    (0x1100)
#define CH0_TEST_TXD_P_SHIFT        13
#define CH0_TEST_TXD_P_MASK         0x1 << CH0_TEST_TXD_P_SHIFT
#define CH0_TEST_TXD_N_SHIFT        12
#define CH0_TEST_TXD_N_MASK         0x1 << CH0_TEST_TXD_N_SHIFT
#define CH0_TEST_SCHMITT_EN_SHIFT   11
#define CH0_TEST_SCHMITT_EN_MASK    0x1 << CH0_TEST_SCHMITT_EN_SHIFT
#define CH0_TEST_RXEN_SHIFT         10
#define CH0_TEST_RXEN_MASK          0x1 << CH0_TEST_RXEN_SHIFT
#define CH0_TEST_RXCM_EN_SHIFT      9
#define CH0_TEST_RXCM_EN_MASK       0x1 << CH0_TEST_RXCM_EN_SHIFT
#define CH0_TEST_PULLDN_SHIFT       8
#define CH0_TEST_PULLDN_MASK        0x1 << CH0_TEST_PULLDN_SHIFT
#define CH0_TEST_OEN_P_SHIFT        7
#define CH0_TEST_OEN_P_MASK         0x1 << CH0_TEST_OEN_P_SHIFT
#define CH0_TEST_OEN_N_SHIFT        6
#define CH0_TEST_OEN_N_MASK         0x1 << CH0_TEST_OEN_N_SHIFT
#define CH0_TEST_IEN_P_SHIFT        5
#define CH0_TEST_IEN_P_MASK         0x1 << CH0_TEST_IEN_P_SHIFT
#define CH0_TEST_IEN_N_SHIFT        4
#define CH0_TEST_IEN_N_MASK         0x1 << CH0_TEST_IEN_N_SHIFT
#define CH0_RTERM_SHIFT             1
#define CH0_RTERM_MASK              0xF << CH0_RTERM_SHIFT
#define CH0_TRIM_SEL_SHIFT          0
#define CH0_TRIM_SEL_MASK           0x1 << CH0_TRIM_SEL_SHIFT

#define LVDS_CH0_RX_CTRL            (0x1200)
#define CH0_CMP_MASK_SHFIT          24
#define CH0_CMP_MASK_MASK           0xFFFF << CH0_CMP_MASK_SHFIT
#define CH0_DATA_CLK_PT_SL_SHIFT    4
#define CH0_DATA_CLK_PT_SL_MASK     0x7F << CH0_DATA_CLK_PT_SL_SHIFT
#define CH0_COM_EN_SHIFT            3
#define CH0_COM_EN_MASK             0x1 << CH0_COM_EN_SHIFT
#define CH0_DATA_CLK_DELAY_SHIFT    1
#define CH0_DATA_CLK_DELAY_MASK     0x3 << CH0_DATA_CLK_DELAY_SHIFT
#define CH0_ERR_CLK_SHIFT           0
#define CH0_ERR_CLK_MASK            0x1 << CH0_ERR_CLK_SHIFT

#define DC1_MUX_CTRL                (0x2000)
#define PARAL_OUT_BPP_SHIFT         16
#define PARAL_OUT_BPP_MASK          0x3 << PARAL_OUT_BPP_SHIFT
#define PARAL_TO_CSI_EN_SHIFT       10
#define PARAL_TO_CSI_EN_MASK        0x1 << PARAL_TO_CSI_EN_SHIFT
#define PARAL_TO_LVDS_EN_SHIFT      9
#define PARAL_TO_LVDS_EN_MASK       0x1 << PARAL_TO_LVDS_EN_SHIFT
#define PARAL_OUT_EN_SHIFT          8
#define PARAL_OUT_EN_MASK           0x1 << PARAL_OUT_EN_SHIFT
#define PARAL_OUT_CLK_POL_SHIFT     6
#define PARAL_OUT_CLK_POL_MASK      0x1 << PARAL_OUT_CLK_POL_SHIFT
#define PARAL_FB_CLK_POL_SHIFT      5
#define PARAL_FB_CLK_POL_MASK       0x1 << PARAL_FB_CLK_POL_SHIFT
#define CRC_SRC_SHIFT               4
#define CRC_SRC_MASK                0x1 << CRC_SRC_SHIFT
#define DSP_CLK_SHIFT               0
#define DSP_CLK_MASK                0x1 << DSP_CLK_SHIFT
#endif //LVDS_REG_H__
