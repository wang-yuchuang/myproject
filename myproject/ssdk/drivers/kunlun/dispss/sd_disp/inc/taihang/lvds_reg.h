/*
* lvds_reg.h
*
* Copyright (c) 2018-2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: dc register defs file.
*
* Revision History:
* -----------------
* 011, 10/17/2019 BI create this file
*/
#ifndef __LVDS_REG_H__
#define __LVDS_REG_H__

#include <regs_base.h>

#define LVDS_BASE                   APB_LVDS_COMMON_BASE
/* LVDS registers (RMW mode) definition */

#define REG(x)                      (x)
/* SOFT RESET */
#define LVDS_SOFT_RESET             REG(0x0)
#define CFG_WRITE_N_ZERO_SHIFT      7
#define CFG_WRITE_N_ZERO_MASK       0x1 << CFG_WRITE_N_ZERO_SHIFT
#define TRIM_MODE_SHIFT             6
#define TRIM_MODE_MASK              TRIM_MODE_SHIFT
#define RTE_CTL_SHIFT               5
#define RTE_CTL_MASK                0x1 << RTE_CTL_SHIFT
#define CH3_SOFT_RESET_SHIFT        3
#define CH3_SOFT_RESET_MASK         0x1 << CH3_SOFT_RESET_SHIFT
#define CH2_SOFT_RESET_SHIFT        2
#define CH2_SOFT_RESET_MASK         0x1 << CH2_SOFT_RESET_SHIFT
#define CH1_SOFT_RESET_SHIFT        1
#define CH1_SOFT_RESET_MASK         0x1 << CH1_SOFT_RESET_SHIFT
#define CH0_SOFT_RESET_SHIFT        0
#define CH0_SOFT_RESET_MASK         0x1 << CH0_SOFT_RESET_SHIFT

/*MUX SEL*/
#define LVDS_MUX_SEL                REG(0x4)
#define MUX8_SEL_SHIFT              19
#define MUX8_SEL_MASK               0x3 << MUX8_SEL_SHIFT
#define MUX7_SEL_SHIFT              17
#define MUX7_SEL_MASK               0x3 << MUX7_SEL_SHIFT
#define MUX6_SEL_SHIFT              15
#define MUX6_SEL_MASK               0x3 << MUX6_SEL_SHIFT
#define MUX5_SEL_SHIFT              13
#define MUX5_SEL_MASK               0x3 << MUX5_SEL_SHIFT

/*LVDS BIAS SET*/
#define LVDS_BIAS_SET               REG(0xc)
#define BIAS_TRIM_SHIFT             2
#define BIAS_TRIM_MASK              0x3 << BIAS_TRIM_SHIFT
#define BIAS_SELVDD_SHIFT           1
#define BIAS_SELVDD_MASK            0x1 << BIAS_SELVDD_SHIFT
#define BIAS_EN_SHIFT               0
#define BIAS_EN_MASK                0x1 << BIAS_EN_SHIFT

/*TEST CFG*/
#define LVDS_TEST_CFG_0             REG(0x100)
#define LVDS_TEST_CFG_1             REG(0x104)
#define TEST_CFG_CLK_SHIFT          8
#define TEST_CFG_CLK_MASK           0x7F << TEST_CFG_CLK_SHIFT
#define TEST_CFG_EN_SHIFT           7
#define TEST_CFG_EN_MASK            0x1 << TEST_CFG_EN_SHIFT
#define TEST_CFG_DATA_SHIFT         0
#define TEST_CFG_DATA_MASK          0x7F << TEST_CFG_DATA_SHIFT

/* LVDS CHN */
#define LVDS_CHN_JMP 0x10000
#define LVDS_CHN_COUNT 4

#define LVDS_CHN_CTRL_(i)           (REG(0x10000) + LVDS_CHN_JMP * i)
#define CHN_EN_SHIFT                31
#define CHN_EN_MASK                 0x1 << CHN_EN_SHIFT
#define CHN_MUX_SHIFT               23
#define CHN_MUX_MASK                0x3 << CHN_MUX_SHIFT
#define CHN_DUALODD_SHIFT           22
#define CHN_DUALODD_MASK            0x1 << CHN_DUALODD_SHIFT
#define CHN_DATA_UPDATE_SHIFT       17
#define CHN_DATA_UPDATE_MASK        0x3 << CHN_DATA_UPDATE_SHIFT
#define CHN_CLK_LANE_DATE_SHIFT     10
#define CHN_CLK_LANE_DATE_MASK      0x7F << CHN_CLK_LANE_DATE_SHIFT
#define CHN_LANE_UPDATE_SHIFT       7
#define CHN_LANE_UPDATE_MASK        0x7 << CHN_LANE_UPDATE_SHIFT
#define CHN_FRAME_MASK_SHIFT        5
#define CHN_FRAME_MASK_MASK         0x3 << CHN_FRAME_MASK_SHIFT
#define CHN_VSYNC_POL_SHIFT         4
#define CHN_VSYNC_POL_MASK          0x1 << CHN_VSYNC_POL_SHIFT
#define CHN_FORMAT_SHIFT            3
#define CHN_FORMAT_MASK             0x1 << CHN_FORMAT_SHIFT
#define CHN_BPP_SHIFT               1
#define CHN_BPP_MASK                0x3 << CHN_BPP_SHIFT
#define CHN_DUALMODE_SHIFT          0
#define CHN_DUALMODE_MASK           0x1 << CHN_DUALMODE_SHIFT

#define LVDS_CHN_CLOCK_(i)          (REG(0x10004) + LVDS_CHN_JMP * i)
#define CHN_DIV_NUM_DBG_SHIFT       11
#define CHN_DIV_NUM_DBG_MASK        0xF << CHN_DIV_NUM_DBG_SHIFT
#define CHN_DIV_NUM_LVDS_SHIFT      7
#define CHN_DIV_NUM_LVDS_MASK       0xF << CHN_DIV_NUM_LVDS_SHIFT
#define CHN_GATING_AKC_SHIFT        6
#define CHN_GATING_AKC_MASK         0x1 << CHN_GATING_AKC_SHIFT
#define CHN_GATING_EN_SHIFT         1
#define CHN_GATING_EN_MASK          0x1F << CHN_GATING_EN_SHIFT
#define CHN_SRC_SEL_SHIFT           0
#define CHN_SRC_SEL_MASK            0x1 << CHN_SRC_SEL_SHIFT

#define LVDS_CHN_PAD_COM_SET_(i)    (REG(0x10008) + LVDS_CHN_JMP * i)
#define CHN_LVDS_TRIM_SHIFT         14
#define CHN_LVDS_TRIM_MASK          0x7 << CHN_LVDS_TRIM_SHIFT
#define CHN_TEST_TXD_P_SHIFT        13
#define CHN_TEST_TXD_P_MASK         0x1 << CHN_TEST_TXD_P_SHIFT
#define CHN_TEST_TXD_N_SHIFT        12
#define CHN_TEST_TXD_N_MASK         0x1 << CHN_TEST_TXD_N_SHIFT
#define CHN_TEST_SCHMITT_EN_SHIFT   11
#define CHN_TEST_SCHMITT_EN_MASK    0x1 << CHN_TEST_SCHMITT_EN_SHIFT
#define CHN_TEST_RXEN_SHIFT         10
#define CHN_TEST_RXEN_MASK          0x1 << CHN_TEST_RXEN_SHIFT
#define CHN_TEST_RXCM_EN_SHIFT      9
#define CHN_TEST_RXCM_EN_MASK       0x1 << CHN_TEST_RXCM_EN_SHIFT
#define CHN_TEST_PULLDN_SHIFT       8
#define CHN_TEST_PULLDN_MASK        0x1 << CHN_TEST_PULLDN_SHIFT
#define CHN_TEST_OEN_P_SHIFT        7
#define CHN_TEST_OEN_P_MASK         0x1 << CHN_TEST_OEN_P_SHIFT
#define CHN_TEST_OEN_N_SHIFT        6
#define CHN_TEST_OEN_N_MASK         0x1 << CHN_TEST_OEN_N_SHIFT
#define CHN_TEST_IEN_P_SHIFT        5
#define CHN_TEST_IEN_P_MASK         0x1 << CHN_TEST_IEN_P_SHIFT
#define CHN_TEST_IEN_N_SHIFT        4
#define CHN_TEST_IEN_N_MASK         0x1 << CHN_TEST_IEN_N_SHIFT

#define LVDS_CHN_PAD_SET_(i)        (REG(0x10010) + LVDS_CHN_JMP * i)
#define CHN_RXDA_SHIFT              11
#define CHN_RXDA_MASK               0x1 << CHN_RXDA_SHIFT
#define CHN_RXD_P_SHIFT             10
#define CHN_RXD_P_MASK              0x1 << CHN_RXD_P_SHIFT
#define CHN_RXD_N_SHIFT             9
#define CHN_RXD_N_MASK              0x1 << CHN_RXD_N_SHIFT
#define CHN_VBIAS_SEL_SHIFT         8
#define CHN_VBIAS_SEL_MASK          0x1 << CHN_VBIAS_SEL_SHIFT
#define CHN_TXEN_SHIFT              7
#define CHN_TXEN_MASK               0x1 << CHN_TXEN_SHIFT
#define CHN_TXDRV_SHIFT             3
#define CHN_TXDRV_MASK              0xF << CHN_TXDRV_SHIFT
#define CHN_TXCM_SHIFT              2
#define CHN_TXCM_MASK               0x1 << CHN_TXCM_SHIFT
#define CHN_RTERM_EN_SHIFT          1
#define CHN_RTERM_EN_MASK           0x1 << CHN_RTERM_EN_SHIFT
#define CHN_BIAS_EN_SHIFT           0
#define CHN_BIAS_EN_MASK            0x1 << CHN_BIAS_EN_SHIFT

#endif //__LVDS_REG_H__
