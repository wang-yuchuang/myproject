/**
 * dw_mac_lld.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dw eth mac lld header file
 *
 * Revision History:
 * -----------------
 */

#ifndef _DW_MAC_LLD_H_
#define _DW_MAC_LLD_H_

#ifndef BIT
#define BIT(n) (1 << (n))
#endif

#ifndef GENMASK
#define GENMASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32 - 1 - (h))))
#endif

/*  MAC reg */
#define MAC_CONFIG              0x00000000
#define MAC_PACKET_FILTER       0x00000008
#define MAC_HASH_TAB_0_31       0x00000010
#define MAC_HASH_TAB_32_63      0x00000014
#define MAC_HASH_TAB_64_95      0x00000018
#define MAC_HASH_TAB_96_127     0x0000001c
#define MAC_RX_FLOW_CTRL        0x00000090
#define MAC_QX_TX_FLOW_CTRL(x)  (0x70 + x * 4)
#define MAC_TXQ_PRTY_MAP0       0x98
#define MAC_TXQ_PRTY_MAP1       0x9C
#define MAC_RXQ_CTRL0           0x000000a0
#define MAC_RXQ_CTRL1           0x000000a4
#define MAC_RXQ_CTRL2           0x000000a8
#define MAC_RXQ_CTRL3           0x000000ac
#define MAC_INT_STATUS          0x000000b0
#define MAC_INT_EN              0x000000b4
#define MAC_RX_TX_STATUS        0x000000b8
#define MAC_PMT_CONTROL_STATUS  0x000000c0
#define MAC_LPI_CONTROL_STATUS  0x000000d0
#define MAC_1US_TIC_COUNTER     0x000000dc
#define MAC_PCS_BASE            0x000000e0
#define MAC_AN_STATUS           0x000000e4
#define MAC_PHYIF_CONTROL_STATUS    0x000000f8
#define MAC_PMT                 0x000000c0
#define MAC_VERSION             0x00000110
#define MAC_DEBUG               0x00000114
#define MAC_HW_FEATURE0         0x0000011c
#define MAC_HW_FEATURE1         0x00000120
#define MAC_HW_FEATURE2         0x00000124
#define MAC_HW_FEATURE3         0x00000128
#define MAC_MDIO_ADDR           0x00000200
#define MAC_MDIO_DATA           0x00000204
#define MAC_ADDR_HIGH(reg)      (0x300 + reg * 8)
#define MAC_ADDR_LOW(reg)       (0x304 + reg * 8)
/* Rx Stats */
#define MAC_PACKETS_RX_CONUT_64             0x000007ac
#define MAC_PACKETS_RX_CONUT_65_127         0x000007b0
#define MAC_PACKETS_RX_CONUT_128_255        0x000007b4
#define MAC_PACKETS_RX_CONUT_256_511        0x000007b8
#define MAC_PACKETS_RX_CONUT_512_1023       0x000007bc
#define MAC_PACKETS_RX_CONUT_1024_MAX       0x000007c0
#define MAC_PACKETS_RX_CONUT_UNICAST        0x000007c4
#define MAC_PACKETS_RX_CONUT_MULTICAST      0x00000790
#define MAC_PACKETS_RX_CONUT_BROADCAST      0x0000078c
#define MAC_PACKETS_RX_CONUT_ALL_BYTES      0x00000784
#define MAC_PACKETS_RX_CONUT_ALL_PKT        0x00000780
#define MAC_PACKETS_RX_CONUT_BAD_PKT        0x000007e0
#define MAC_PACKETS_RX_CONUT_CRC_ERR        0x00000794
#define MAC_PACKETS_RX_CONUT_FRAG_ERR       0x0000079C
#define MAC_PACKETS_RX_CONUT_ALIGN_ERR      0x00000798
#define MAC_PACKETS_RX_CONUT_JABBER_ERR     0x000007a0
#define MAC_PACKETS_RX_CONUT_UNDER_PKT      0x000007a4
#define MAC_PACKETS_RX_CONUT_OVER_PKT       0x000007a8
#define MAC_PACKETS_RX_CONUT_DROP_PKT       0x000007d4
#define MAC_PACKETS_RX_CONUT_MIIDROP_PKT    0x000007e0
/* Tx Stats */
#define MAC_PACKETS_TX_CONUT_ALL_BYTES      0x00000714
#define MAC_PACKETS_TX_CONUT_UNICAST        0x0000073C
#define MAC_PACKETS_TX_CONUT_BROADCAST      0x00000744
#define MAC_PACKETS_TX_CONUT_MULTICAST      0x00000740
#define MAC_PACKETS_TX_CONUT_SINGL_COLL     0x0000074C
#define MAC_PACKETS_TX_CONUT_MULT_COLL      0x00000750
#define MAC_PACKETS_TX_CONUT_DFRD_COLL      0x00000754
#define MAC_PACKETS_TX_CONUT_LATE_COLL      0x00000758
#define MAC_PACKETS_TX_CONUT_OVER_PKT       0x00000778
#define MAC_PACKETS_TX_CONUT_ALL_PKT        0x00000718
#define MAC_PACKETS_TX_CONUT_GOOD_PKT       0x00000768

#define MAC_PACKETS_TX_CONUT_DROP_PKT_COLL  0x0000075c
#define MAC_PACKETS_TX_CONUT_DROP_PKT_CARR  0x00000760
#define MAC_PACKETS_TX_CONUT_ERR_PKT        0x0000076c

#define MMC_RX_INTERRUPT_MASK               0x70C
#define MMC_TX_INTERRUPT_MASK               0x710
#define MMC_IPC_RX_INTERRUPT_MASK           0x800

/* RX Queues Routing */
#define MAC_RXQCTRL_AVCPQ_MASK      GENMASK(2, 0)
#define MAC_RXQCTRL_AVCPQ_SHIFT     0
#define MAC_RXQCTRL_PTPQ_MASK       GENMASK(6, 4)
#define MAC_RXQCTRL_PTPQ_SHIFT      4
#define MAC_RXQCTRL_DCBCPQ_MASK     GENMASK(10, 8)
#define MAC_RXQCTRL_DCBCPQ_SHIFT    8
#define MAC_RXQCTRL_UPQ_MASK        GENMASK(14, 12)
#define MAC_RXQCTRL_UPQ_SHIFT       12
#define MAC_RXQCTRL_MCBCQ_MASK      GENMASK(18, 16)
#define MAC_RXQCTRL_MCBCQ_SHIFT     16
#define MAC_RXQCTRL_MCBCQEN         BIT(20)
#define MAC_RXQCTRL_MCBCQEN_SHIFT   20
#define MAC_RXQCTRL_TACPQE          BIT(21)
#define MAC_RXQCTRL_TACPQE_SHIFT    21

/* MAC Packet Filtering */
#define MAC_PACKET_FILTER_PR        BIT(0)
#define MAC_PACKET_FILTER_HUC       BIT(1)
#define MAC_PACKET_FILTER_HMC       BIT(2)
#define MAC_PACKET_FILTER_DAIF      BIT(3)
#define MAC_PACKET_FILTER_PM        BIT(4)
#define MAC_PACKET_FILTER_DBF       BIT(5)
#define MAC_PACKET_FILTER_MASK      GENMASK(7, 6)
#define MAC_PACKET_FILTER_SHIFT     (6)
#define MAC_PACKET_FILTER_HPF       BIT(10)
#define MAC_PACKET_FILTER_PA        BIT(31)

#define MAC_MAX_PERFECT_ADDRESSES   128

/* MAC RX Queue Enable */
#define MAC_RX_QUEUE_CLEAR(queue)       ~(GENMASK(1, 0) << ((queue) * 2))
#define MAC_RX_AV_QUEUE_ENABLE(queue)   BIT((queue) * 2)
#define MAC_RX_DCB_QUEUE_ENABLE(queue)  BIT(((queue) * 2) + 1)

/* MAC Flow Control RX */
#define MAC_RX_FLOW_CTRL_RFE        BIT(0)

/* RX Queues Priorities */
#define MAC_RXQCTRL_PSRQX_MASK(x)   GENMASK(7 + ((x) * 8), 0 + ((x) * 8))
#define MAC_RXQCTRL_PSRQX_SHIFT(x)  ((x) * 8)

/* TX Queues Priorities */
#define MAC_TXQCTRL_PSTQX_MASK(x)   GENMASK(7 + ((x) * 8), 0 + ((x) * 8))
#define MAC_TXQCTRL_PSTQX_SHIFT(x)  ((x) * 8)

/* MAC Flow Control TX */
#define MAC_TX_FLOW_CTRL_TFE        BIT(1)
#define MAC_TX_FLOW_CTRL_PT_SHIFT   16

/*  MAC Interrupt bitmap*/
#define MAC_INT_RGSMIIS         BIT(0)
#define MAC_INT_PCS_LINK        BIT(1)
#define MAC_INT_PCS_ANE         BIT(2)
#define MAC_INT_PCS_PHYIS       BIT(3)
#define MAC_INT_PMT_EN          BIT(4)
#define MAC_INT_LPI_EN          BIT(5)

#define MAC_PCS_IRQ_DEFAULT (MAC_INT_RGSMIIS | MAC_INT_PCS_LINK | MAC_INT_PCS_ANE)

#define MAC_INT_DEFAULT_ENABLE (MAC_INT_PMT_EN | MAC_INT_LPI_EN)

/* MAC config */
#define MAC_CONFIG_IPC          BIT(27)
#define MAC_CONFIG_2K           BIT(22)
#define MAC_CONFIG_ACS          BIT(20)
#define MAC_CONFIG_WD           BIT(19)
#define MAC_CONFIG_BE           BIT(18)
#define MAC_CONFIG_JD           BIT(17)
#define MAC_CONFIG_JE           BIT(16)
#define MAC_CONFIG_PS           BIT(15)
#define MAC_CONFIG_FES          BIT(14)
#define MAC_CONFIG_DM           BIT(13)
#define MAC_CONFIG_DCRS         BIT(9)
#define MAC_CONFIG_TE           BIT(1)
#define MAC_CONFIG_RE           BIT(0)

/* MAC HW ADDR regs */
#define MAC_HI_DCS              GENMASK(18, 16)
#define MAC_HI_DCS_SHIFT        16
#define MAC_HI_REG_AE           BIT(31)

/*  MTL registers */
#define MTL_OPERATION_MODE          0x00000c00
#define MTL_OPERATION_SCHALG_MASK   GENMASK(6, 5)
#define MTL_OPERATION_SCHALG_WRR    (0x0 << 5)
#define MTL_OPERATION_SCHALG_WFQ    (0x1 << 5)
#define MTL_OPERATION_SCHALG_DWRR   (0x2 << 5)
#define MTL_OPERATION_SCHALG_SP     (0x3 << 5)
#define MTL_OPERATION_RAA           BIT(2)
#define MTL_OPERATION_RAA_SP        (0x0 << 2)
#define MTL_OPERATION_RAA_WSP       (0x1 << 2)

#define MTL_TX_OPERATION_TSF        BIT(2)
#define MTL_TX_OPERATION_TQ         GENMASK(3, 2)
#define MTL_TX_OPERATION_TQ_EN      (0x2 << 2)
#define MTL_TX_OPERATION_TQ_DIS     (0x0 << 2)

#define MTL_TX_CONFIG_FLAGS_TQ      BIT(3)

#define MTL_INT_STATUS              0x00000c20
#define MTL_INT_QX(x)               BIT(x)

#define MTL_RXQ_DMA_MAP0            0x00000c30 /* queue 0 to 3 */
#define MTL_RXQ_DMA_MAP1            0x00000c34 /* queue 4 to 7 */
#define MTL_RXQ_DMA_Q04MDMACH_MASK  GENMASK(3, 0)
#define MTL_RXQ_DMA_Q04MDMACH(x)    ((x) << 0)
#define MTL_RXQ_DMA_QXMDMACH_MASK(x)        GENMASK(11 + (8 * ((x) - 1)), 8 * (x))
#define MTL_RXQ_DMA_QXMDMACH(channel, q)    ((channel) << (8 * (q)))

#define MTL_DPP_CTRL                0x00000ce0
#define MTL_DPP_CTRL_DATA_PARITY    BIT(0)
#define MTL_DPP_CTRL_SLAVE_PARITY   BIT(2)

#define MTL_CHAN_BASE_ADDR          0x00000d00
#define MTL_CHAN_BASE_OFFSET        0x40
#define MTL_CHANX_BASE_ADDR(x)      (MTL_CHAN_BASE_ADDR + \
                    (x * MTL_CHAN_BASE_OFFSET))

#define MTL_CHAN_TX_OP_MODE(x)      MTL_CHANX_BASE_ADDR(x)
#define MTL_CHAN_TX_DEBUG(x)        (MTL_CHANX_BASE_ADDR(x) + 0x8)
#define MTL_CHAN_INT_CTRL(x)        (MTL_CHANX_BASE_ADDR(x) + 0x2c)
#define MTL_CHAN_RX_OP_MODE(x)      (MTL_CHANX_BASE_ADDR(x) + 0x30)
#define MTL_CHAN_RX_DEBUG(x)        (MTL_CHANX_BASE_ADDR(x) + 0x38)

#define MTL_OP_MODE_RSF             BIT(5)
#define MTL_OP_MODE_TXQEN_MASK      GENMASK(3, 2)
#define MTL_OP_MODE_TXQEN_AV        BIT(2)
#define MTL_OP_MODE_TXQEN           BIT(3)
#define MTL_OP_MODE_TSF             BIT(1)

#define MTL_OP_MODE_TQS_MASK        GENMASK(24, 16)
#define MTL_OP_MODE_TQS_SHIFT       16

#define MTL_OP_MODE_TTC_MASK        0x70
#define MTL_OP_MODE_TTC_SHIFT       4

#define MTL_OP_MODE_TTC_32          0
#define MTL_OP_MODE_TTC_64          (1 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_96          (2 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_128         (3 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_192         (4 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_256         (5 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_384         (6 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_512         (7 << MTL_OP_MODE_TTC_SHIFT)

#define MTL_OP_MODE_RQS_MASK        GENMASK(29, 20)
#define MTL_OP_MODE_RQS_SHIFT       20

#define MTL_OP_MODE_RFD_MASK        GENMASK(19, 14)
#define MTL_OP_MODE_RFD_SHIFT       14

#define MTL_OP_MODE_RFA_MASK        GENMASK(13, 8)
#define MTL_OP_MODE_RFA_SHIFT       8
#define MTL_OP_MODE_FEP             BIT(4)
#define MTL_OP_MODE_FUP             BIT(3)
#define MTL_OP_MODE_EHFC            BIT(7)

#define MTL_OP_MODE_RTC_MASK        0x18
#define MTL_OP_MODE_RTC_SHIFT       3

#define MTL_OP_MODE_RTC_32          (1 << MTL_OP_MODE_RTC_SHIFT)
#define MTL_OP_MODE_RTC_64          0
#define MTL_OP_MODE_RTC_96          (2 << MTL_OP_MODE_RTC_SHIFT)
#define MTL_OP_MODE_RTC_128         (3 << MTL_OP_MODE_RTC_SHIFT)

/* MTL ETS Control register */
#define MTL_ETS_CTRL_BASE_ADDR      0x00000d10
#define MTL_ETS_CTRL_BASE_OFFSET    0x40
#define MTL_ETSX_CTRL_BASE_ADDR(x)  (MTL_ETS_CTRL_BASE_ADDR + \
                    ((x) * MTL_ETS_CTRL_BASE_OFFSET))

#define MTL_ETS_CTRL_CC             BIT(3)
#define MTL_ETS_CTRL_AVALG          BIT(2)

/* MTL Queue Quantum Weight */
#define MTL_TXQ_WEIGHT_BASE_ADDR    0x00000d18
#define MTL_TXQ_WEIGHT_BASE_OFFSET  0x40
#define MTL_TXQX_WEIGHT_BASE_ADDR(x)    (MTL_TXQ_WEIGHT_BASE_ADDR + \
                    ((x) * MTL_TXQ_WEIGHT_BASE_OFFSET))
#define MTL_TXQ_WEIGHT_ISCQW_MASK   GENMASK(20, 0)

/* MTL sendSlopeCredit register */
#define MTL_SEND_SLP_CRED_BASE_ADDR 0x00000d1c
#define MTL_SEND_SLP_CRED_OFFSET    0x40
#define MTL_SEND_SLP_CREDX_BASE_ADDR(x) (MTL_SEND_SLP_CRED_BASE_ADDR + \
                    ((x) * MTL_SEND_SLP_CRED_OFFSET))

#define MTL_SEND_SLP_CRED_SSC_MASK  GENMASK(13, 0)

/* MTL hiCredit register */
#define MTL_HIGH_CRED_BASE_ADDR     0x00000d20
#define MTL_HIGH_CRED_OFFSET        0x40
#define MTL_HIGH_CREDX_BASE_ADDR(x) (MTL_HIGH_CRED_BASE_ADDR + \
                    ((x) * MTL_HIGH_CRED_OFFSET))

#define MTL_HIGH_CRED_HC_MASK       GENMASK(28, 0)

/* MTL loCredit register */
#define MTL_LOW_CRED_BASE_ADDR      0x00000d24
#define MTL_LOW_CRED_OFFSET         0x40
#define MTL_LOW_CREDX_BASE_ADDR(x)  (MTL_LOW_CRED_BASE_ADDR + \
                    ((x) * MTL_LOW_CRED_OFFSET))

#define MTL_HIGH_CRED_LC_MASK       GENMASK(28, 0)

/*  MTL interrupt */
#define MTL_RX_OVERFLOW_INT_EN      BIT(24)
#define MTL_RX_OVERFLOW_INT         BIT(16)
#define MTL_TX_UNDEROVERFLOW_INT    BIT(0)

/* Default operating mode of the MAC */
#define MAC_CORE_INIT (MAC_CONFIG_JD | MAC_CONFIG_PS | \
            MAC_CONFIG_BE | MAC_CONFIG_DCRS | MAC_CONFIG_DM)

/* To dump the core regs excluding  the Address Registers */
#define MAC_REG_NUM 0x300

/* MTL algorithms identifiers */
#define MTL_TX_ALGORITHM_WRR    0x0
#define MTL_TX_ALGORITHM_WFQ    0x1
#define MTL_TX_ALGORITHM_DWRR   0x2
#define MTL_TX_ALGORITHM_SP     0x3
#define MTL_RX_ALGORITHM_SP     0x4
#define MTL_RX_ALGORITHM_WSP    0x5

/* RX/TX Queue Mode */
#define MTL_QUEUE_MODE_AVB      0x1
#define MTL_QUEUE_MODE_DCB      0x2

#define SF_MODE  1

#define MAC_FITER_PASS_ALL                  0
#define MAC_FITER_PASS_MULTICAST            1
#define MAC_FITER_PASS_HASH_MULTICAST       2
#define MAC_FITER_PASS_UNICAST              3

#define ERDES4_MSG_TYPE_MASK        GENMASK(11, 8)

/* Extended RDES4 message type definitions */
#define RDES_EXT_NO_PTP             0x0
#define RDES_EXT_SYNC               0x1
#define RDES_EXT_FOLLOW_UP          0x2
#define RDES_EXT_DELAY_REQ          0x3
#define RDES_EXT_DELAY_RESP         0x4
#define RDES_EXT_PDELAY_REQ         0x5
#define RDES_EXT_PDELAY_RESP        0x6
#define RDES_EXT_PDELAY_FOLLOW_UP   0x7
#define RDES_PTP_ANNOUNCE           0x8
#define RDES_PTP_MANAGEMENT         0x9
#define RDES_PTP_SIGNALING          0xa
#define RDES_PTP_PKT_RESERVED_TYPE  0xf

/* Rx IPC status */
enum rx_frame_status {
    RX_GOOD = 0x0,
    RX_BAD = 0x1,
    RX_DMA_PROCESS = 0x8,
    csum_none = 0x2,
    llc_snap = 0x4,
    rx_not_ls = 0x10,
};

enum tx_frame_status {
    TX_GOOD = 0x0,
    TX_ERROR = 0x1,
    TX_DMA_PROCESS = 0x2,
    TX_NOT_LS = 0x3,
};

/* DMA config */
#define ETH_MAX_DMA_CHANNEL     1

#define DMA_BUS_MODE            0x00001000
#define DMA_STATUS              0x00001008
#define DMA_SYS_BUS_MODE        0x00001004
#define DMA_DEBUG_STATUS_0      0x0000100c
#define DMA_DEBUG_STATUS_1      0x00001010
#define DMA_DEBUG_STATUS_2      0x00001014
#define DMA_AXI_BUS_MODE        0x00001028

enum dma_rx_status {
    DMX_RX_STOP = 0,
    DMX_RX_RUN_FRTD,
    DMX_RX_RSVD,
    DMX_RX_WRP,
    DMX_RX_SUSPND,
    DMX_RX_RUN_CRD,
    DMX_RX_TSTMP,
    DMX_RX_TRP
};

/* DMA Bus Mode bitmap */
#define DMA_BUS_MODE_SFT_RESET      BIT(0)

/* DMA SYS Bus Mode bitmap */
#define DMA_BUS_MODE_SPH            BIT(24)
#define DMA_BUS_MODE_PBL            BIT(16)
#define DMA_BUS_MODE_PBL_SHIFT      16
#define DMA_BUS_MODE_RPBL_SHIFT     16
#define DMA_BUS_MODE_MB             BIT(14)
#define DMA_BUS_MODE_FB             BIT(0)

/* DMA Interrupt top status */
#define DMA_STATUS_MAC          BIT(17)
#define DMA_STATUS_MTL          BIT(16)
#define DMA_STATUS_CHAN7        BIT(7)
#define DMA_STATUS_CHAN6        BIT(6)
#define DMA_STATUS_CHAN5        BIT(5)
#define DMA_STATUS_CHAN4        BIT(4)
#define DMA_STATUS_CHAN3        BIT(3)
#define DMA_STATUS_CHAN2        BIT(2)
#define DMA_STATUS_CHAN1        BIT(1)
#define DMA_STATUS_CHAN0        BIT(0)
#define DMA_STATUS_CHAN0_5      GENMASK(4,0)

/* DMA debug status bitmap */
#define DMA_DEBUG_STATUS_TS_MASK    0xf
#define DMA_DEBUG_STATUS_RS_MASK    0xf

/* DMA AXI bitmap */
#define DMA_AXI_EN_LPI          BIT(31)
#define DMA_AXI_LPI_XIT_FRM     BIT(30)
#define DMA_AXI_WR_OSR_LMT      GENMASK(27, 24)
#define DMA_AXI_WR_OSR_LMT_SHIFT    24
#define DMA_AXI_RD_OSR_LMT      GENMASK(19, 16)
#define DMA_AXI_RD_OSR_LMT_SHIFT    16

#define DMA_AXI_OSR_MAX         0xf
#define DMA_AXI_MAX_OSR_LIMIT ((DMA_AXI_OSR_MAX << DMA_AXI_WR_OSR_LMT_SHIFT) | \
                (DMA_AXI_OSR_MAX << DMA_AXI_RD_OSR_LMT_SHIFT))

#define DMA_SYS_BUS_MB          BIT(14)
#define DMA_AXI_1KBBE           BIT(13)
#define DMA_SYS_BUS_AAL         BIT(12)
#define DMA_SYS_BUS_EAME        BIT(11)
#define DMA_AXI_BLEN256         BIT(7)
#define DMA_AXI_BLEN128         BIT(6)
#define DMA_AXI_BLEN64          BIT(5)
#define DMA_AXI_BLEN32          BIT(4)
#define DMA_AXI_BLEN16          BIT(3)
#define DMA_AXI_BLEN8           BIT(2)
#define DMA_AXI_BLEN4           BIT(1)
#define DMA_SYS_BUS_FB          BIT(0)

#define DMA_BURST_LEN_DEFAULT       (DMA_AXI_BLEN256 | DMA_AXI_BLEN128 | \
                    DMA_AXI_BLEN64 | DMA_AXI_BLEN32 | \
                    DMA_AXI_BLEN16 | DMA_AXI_BLEN8 | \
                    DMA_AXI_BLEN4)

#define DMA_AXI_BURST_LEN_MASK      0x000000FE

/* Following DMA defines are chanels oriented */
#define DMA_CHAN_BASE_ADDR          0x00001100
#define DMA_CHAN_BASE_OFFSET        0x80
#define DMA_CHANX_BASE_ADDR(x)      (DMA_CHAN_BASE_ADDR + (x * DMA_CHAN_BASE_OFFSET))
#define DMA_CHAN_REG_NUMBER         17

#define DMA_CHAN_CONTROL(x)         DMA_CHANX_BASE_ADDR(x)
#define DMA_CHAN_TX_CONTROL(x)      (DMA_CHANX_BASE_ADDR(x) + 0x4)
#define DMA_CHAN_RX_CONTROL(x)      (DMA_CHANX_BASE_ADDR(x) + 0x8)
#define DMA_CHAN_TX_BASE_ADDR_HI(x) (DMA_CHANX_BASE_ADDR(x) + 0x10)
#define DMA_CHAN_TX_BASE_ADDR(x)    (DMA_CHANX_BASE_ADDR(x) + 0x14)
#define DMA_CHAN_RX_BASE_ADDR_HI(x) (DMA_CHANX_BASE_ADDR(x) + 0x18)
#define DMA_CHAN_RX_BASE_ADDR(x)    (DMA_CHANX_BASE_ADDR(x) + 0x1c)
#define DMA_CHAN_TX_END_ADDR(x)     (DMA_CHANX_BASE_ADDR(x) + 0x20)
#define DMA_CHAN_RX_END_ADDR(x)     (DMA_CHANX_BASE_ADDR(x) + 0x28)
#define DMA_CHAN_TX_RING_LEN(x)     (DMA_CHANX_BASE_ADDR(x) + 0x2c)
#define DMA_CHAN_RX_RING_LEN(x)     (DMA_CHANX_BASE_ADDR(x) + 0x30)
#define DMA_CHAN_INTR_ENA(x)        (DMA_CHANX_BASE_ADDR(x) + 0x34)
#define DMA_CHAN_RX_WATCHDOG(x)     (DMA_CHANX_BASE_ADDR(x) + 0x38)
#define DMA_CHAN_SLOT_CTRL_STATUS(x)    (DMA_CHANX_BASE_ADDR(x) + 0x3c)
#define DMA_CHAN_CUR_TX_DESC(x)     (DMA_CHANX_BASE_ADDR(x) + 0x44)
#define DMA_CHAN_CUR_RX_DESC(x)     (DMA_CHANX_BASE_ADDR(x) + 0x4c)
#define DMA_CHAN_CUR_TX_BUF_ADDR(x) (DMA_CHANX_BASE_ADDR(x) + 0x54)
#define DMA_CHAN_CUR_RX_BUF_ADDR(x) (DMA_CHANX_BASE_ADDR(x) + 0x5c)
#define DMA_CHAN_STATUS(x)          (DMA_CHANX_BASE_ADDR(x) + 0x60)

/* DMA Control X */
#define DMA_CONTROL_MSS_MASK        GENMASK(13, 0)

/* DMA Tx channel X Control register defines */
#define DMA_CONTROL_DSL_MASK        GENMASK(20, 18)
#define DMA_CONTROL_DSL_SHIFT       18
#define DMA_CONTROL_TSE         BIT(12)
#define DMA_CONTROL_OSP         BIT(4)
#define DMA_CONTROL_ST          BIT(0)
#define DMA_CONTROL_TXPBL(x)    (((x) << 16) & GENMASK(21, 16))

/* DMA Rx channel X Control register defines */
#define DMA_CONTROL_SR          BIT(0)
#define DMA_RBSZ_MASK           GENMASK(14, 1)
#define DMA_RBSZ_SHIFT          1
#define DMA_CONTROL_RXPBL(x)    (((x) << 16) & GENMASK(21, 16))

/* Interrupt status per channel */
#define DMA_CHAN_STATUS_REB     GENMASK(21, 19)
#define DMA_CHAN_STATUS_REB_SHIFT   19
#define DMA_CHAN_STATUS_TEB     GENMASK(18, 16)
#define DMA_CHAN_STATUS_TEB_SHIFT   16
#define DMA_CHAN_STATUS_NIS     BIT(15)
#define DMA_CHAN_STATUS_AIS     BIT(14)
#define DMA_CHAN_STATUS_CDE     BIT(13)
#define DMA_CHAN_STATUS_FBE     BIT(12)
#define DMA_CHAN_STATUS_ERI     BIT(11)
#define DMA_CHAN_STATUS_ETI     BIT(10)
#define DMA_CHAN_STATUS_RWT     BIT(9)
#define DMA_CHAN_STATUS_RPS     BIT(8)
#define DMA_CHAN_STATUS_RBU     BIT(7)
#define DMA_CHAN_STATUS_RI      BIT(6)
#define DMA_CHAN_STATUS_TBU     BIT(2)
#define DMA_CHAN_STATUS_TPS     BIT(1)
#define DMA_CHAN_STATUS_TI      BIT(0)

/* Interrupt enable bits per channel */
#define DMA_CHAN_INTR_ENA_NIE       BIT(15)
#define DMA_CHAN_INTR_ENA_AIE       BIT(14)
#define DMA_CHAN_INTR_ENA_CDE       BIT(13)
#define DMA_CHAN_INTR_ENA_FBE       BIT(12)
#define DMA_CHAN_INTR_ENA_ERE       BIT(11)
#define DMA_CHAN_INTR_ENA_ETE       BIT(10)
#define DMA_CHAN_INTR_ENA_RWE       BIT(9)
#define DMA_CHAN_INTR_ENA_RSE       BIT(8)
#define DMA_CHAN_INTR_ENA_RBUE      BIT(7)
#define DMA_CHAN_INTR_ENA_RIE       BIT(6)
#define DMA_CHAN_INTR_ENA_TBUE      BIT(2)
#define DMA_CHAN_INTR_ENA_TSE       BIT(1)
#define DMA_CHAN_INTR_ENA_TIE       BIT(0)

#define DMA_CHAN_INTR_NORMAL        (DMA_CHAN_INTR_ENA_NIE | \
                     DMA_CHAN_INTR_ENA_RIE | \
                     DMA_CHAN_INTR_ENA_TIE )

#define DMA_CHAN_INTR_ABNORMAL      (DMA_CHAN_INTR_ENA_AIE | \
                     DMA_CHAN_INTR_ENA_FBE)
/* DMA default interrupt mask for 4.00 */
#define DMA_CHAN_INTR_DEFAULT_MASK  (DMA_CHAN_INTR_NORMAL | \
                     DMA_CHAN_INTR_ABNORMAL)

#define PHY_CLAUSE_22   0xFF

/*
    mode  transmit store and forward enable or not
    qmode transmit queue enable or not
    fifo  transmit queue size
*/
#if defined(DEBUG_ENABLE)
void dwmac_dump_mac(uint32_t   addr);
void dwmac_dump_mtl(uint32_t  regbase, uint32_t queue);
#endif
void dwmac_prog_mtl_rx_algorithms(uint32_t  regbase, uint32_t rx_alg);
void dwmac_prog_mtl_tx_algorithms(uint32_t  regbase, uint32_t tx_alg);
void dwmac_map_mtl_dma(uint32_t  addr, uint32_t queue, uint32_t channel);
int32_t dwmac_set_filter(uint32_t  regbase, uint32_t mode,
                         const uint8_t *mac_addr);
void dwmac_flow_ctrl(uint32_t  regbase, uint32_t duplex, uint32_t fc,
                     uint32_t pause_time, uint32_t tx_cnt);
void dwmac_core_init(uint32_t  regbase, uint32_t mtu, uint32_t speed,
                     uint8_t *macaddr);
void dwmac_clr_umac_addr(uint32_t regbase, uint32_t Reg_n);
void dwmac_set_umac_addr(uint32_t  regbase, const uint8_t *addr,
                         uint32_t reg_n);
void dwmac_get_umac_addr(uint32_t  regbase, uint8_t *addr, uint32_t reg_n);
void dwmac_rx_queue_enable(uint32_t  addr, uint8_t mode, uint32_t queue);
int dwmac_irq_mtl_status(uint32_t  regbase, uint32_t channel);
int32_t dwmac_dma_reset(uint32_t  regbase);
bool dwmac_access_test(uint32_t  regbase);
void dwmac_enable_dma_irq(uint32_t  regbase, uint32_t channel, bool rx,
                          bool tx);
void dwmac_disable_dma_irq(uint32_t  regbase, uint32_t channel);
void dwmac_dump_dma_regs(uint32_t  regbase, int channel);
void dwmac_dma_start_tx(uint32_t  regbase, uint32_t channel);
void dwmac_dma_stop_tx(uint32_t  regbase, uint32_t channel);
void dwmac_dma_start_rx(uint32_t  regbase, uint32_t channel);
void dwmac_dma_stop_rx(uint32_t  regbase, uint32_t channel);
void dwmac_dma_rx_chan_op_mode(uint32_t  regbase, uint32_t channel, int mode,
                               int fifosz, uint8_t qmode);
/*
    mode  transmit store and forward enable or not
    qmode transmit queue enable or not
    fifo  transmit queue size
*/
void dwmac_dma_tx_chan_op_mode(uint32_t  regbase, uint32_t channel, int mode,
                               int fifosz, uint8_t qmode);

void dwmac_set_rx_ring_len(uint32_t  regbase, uint32_t len, uint32_t channel);
void dwmac_set_tx_ring_len(uint32_t  regbase, uint32_t len, uint32_t channel);
void dwmac_set_rx_list_ptr(uint32_t regbase, uint32_t  list_ptr,
                           uint32_t channel);
void dwmac_set_tx_list_ptr(uint32_t regbase, uint32_t  list_ptr,
                           uint32_t channel);
void dwmac_set_rx_tail_ptr(uint32_t  regbase, uint32_t   tail_ptr,
                           uint32_t channel);
void dwmac_set_tx_tail_ptr(uint32_t  regbase, uint32_t   tail_ptr,
                           uint32_t channel);
void dwmac_dma_init_rx_chan(uint32_t  regbase, uint32_t   base_ptr,
                            uint32_t rbsz, uint32_t channel);
void dwmac_dma_init_tx_chan(uint32_t  regbase, uint32_t   base_ptr,
                            uint32_t channel);
void dwmac_dma_bus_init(uint32_t  regbase, uint32_t flags);
uint32_t dwmac_dma_rx_state_get(uint32_t regbase, uint32_t channel);
void dwmac_dma_init_channel(uint32_t  regbase, uint32_t flags, \
                            uint32_t channel, uint32_t skip);

void dwmac_set_bfsize(uint32_t  regbase, int bfsize, uint32_t channel);
int32_t mac_mdio_write(uint32_t  regbase, uint8_t phyaddr, uint8_t device,
                       uint16_t phyreg, uint16_t phydata);
int32_t mac_mdio_read(uint32_t  regbase, uint8_t phyaddr, uint8_t device,
                      uint16_t phyreg);
void dwmac_rx_trigger(uint32_t  regbase, uint32_t channel);
void dwmac_mac_rx_queue_mcbc_routing(uint32_t  regbase, uint8_t channel);
uint8_t *dwmac_get_default_mac_addr(void);
void dwmac_enable_dma_rx_int(uint32_t regbase, uint32_t channel, bool enable);
void dwmac_enable_dma_tx_int(uint32_t regbase, uint32_t channel, bool enable);
uint32_t dwmac_get_dma_int_status(uint32_t regbase);
uint32_t dwmac_get_mac_int_status(uint32_t regbase);
uint32_t dwmac_get_mac_phyif_control_status(uint32_t regbase);
uint32_t dwmac_get_mac_an_status(uint32_t regbase);
uint32_t dwmac_get_mac_pmt_control_status(uint32_t regbase);
uint32_t dwmac_get_mac_lpi_control_status(uint32_t regbase);
uint32_t dwmac_get_mtl_int_status(uint32_t regbase);
uint32_t dwmac_get_mtl_q_int_status(uint32_t regbase, uint32_t channel);
void dwmac_clr_mtl_q_int_status(uint32_t regbase, uint32_t channel, uint32_t val);
uint32_t dwmac_get_dma_chn_status(uint32_t regbase, uint32_t channel);
void dwmac_clear_dma_chn_int_status(uint32_t regbase, uint32_t channel);
#endif
