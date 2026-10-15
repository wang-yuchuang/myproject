/**
 * dw_mac_lld.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dw eth mac lld driver
 *
 * Revision History:
 * -----------------
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "debug.h"
#include "reg.h"
#include "regs_base.h"
#include "board/board.h"
#include "driver/taishan/eth/phy.h"
#include "dw_mac_lld.h"

static uint8_t deafult_mac_addr[6] = {0x00, 0x1f, 0x29, 0x00, 0x12, 0x15};

#define mdelay(x)   udelay(x * 1000)

void dwc_write_reg(uint32_t val, uint32_t regaddr)
{
    writel(val, regaddr);
}

/**
 * @brief generate mac addr
 */
static void generate_default_mac_from_uuid(void)
{
    uint32_t mac;
    mac = readl(APB_EFUSEC_BASE + 0x1020);
    mac ^= readl(APB_EFUSEC_BASE + 0x1024);

    deafult_mac_addr[3] = (uint8_t)(mac >> 0) & 0xFF;
    deafult_mac_addr[4] = (uint8_t)(mac >> 8) & 0xFF;
    deafult_mac_addr[5] = (uint8_t)(mac >> 16) & 0xFF;

}

static void dwmac_set_mac_addr(uint32_t regbase, uint8_t Addr[6],
                               uint32_t high, uint32_t low)
{
    unsigned long data;

    data = (Addr[5] << 8) | Addr[4];
    /* For MAC Addr registers se have to set the Address Enable (AE)
     * bit that has no effect on the High Reg 0 where the bit 31 (MO)
     * is RO.
     */
    data |= (0 << MAC_HI_DCS_SHIFT);
    dwc_write_reg(data | MAC_HI_REG_AE, regbase + high);
    data = (Addr[3] << 24) | (Addr[2] << 16) | (Addr[1] << 8) | Addr[0];
    dwc_write_reg(data, regbase + low);
}

static void dwmac_clr_mac_addr(uint32_t regbase, uint32_t high, uint32_t low)
{
    unsigned long data = 0xFFFF;
    data &= ~MAC_HI_REG_AE;
    dwc_write_reg(data, regbase + high);

    data = 0xFFFFFFFF;
    dwc_write_reg(data, regbase + low);
}

static void dwmac_get_mac_addr(uint32_t regbase, uint8_t *Addr,
                               uint32_t high, uint32_t low)
{
    uint32_t hi_addr, lo_addr;

    /* Read the MAC address from the hardware */
    hi_addr = readl(regbase + high);
    lo_addr = readl(regbase + low);

    /* Extract the MAC address from the high and low words */
    Addr[0] = lo_addr & 0xff;
    Addr[1] = (lo_addr >> 8) & 0xff;
    Addr[2] = (lo_addr >> 16) & 0xff;
    Addr[3] = (lo_addr >> 24) & 0xff;
    Addr[4] = hi_addr & 0xff;
    Addr[5] = (hi_addr >> 8) & 0xff;
}

/**
 * @description:
 * @param {*}
 * @return {*}
 */
static inline int32_t poll_reg_bits(uint32_t regbase, uint32_t BitMask, uint32_t BitVel,
                                    uint32_t PollPeriod, uint32_t Timeout, bool Neq)
{
    uint32_t limit = Timeout;

    do {
        if (Neq) {
            if ((readl(regbase) & BitMask) == BitVel) {
                udelay(PollPeriod);
            }
            else {
                return 0;
            }
        }
        else {
            if ((readl(regbase) & BitMask) != BitVel) {
                udelay(PollPeriod);
            }
            else {
                return 0;
            }
        }

        limit--;
    }
    while (limit);

    return -1;
}

/* =====================> MAC BLOCK <==================== */
/**
 * @name:
 * @Desc:
 * @param {void } *regbase
 * @param {uint32} MTU
 * @param {uint32} Speed
 * @return {*}
 */
void dwmac_core_init(uint32_t regbase, uint32_t MTU, uint32_t Speed,
                     uint8_t *macaddr)
{

    uint32_t value = readl(regbase + MAC_CONFIG);

    value |= MAC_CORE_INIT;
    value &= ~MAC_CONFIG_TE;
    value &= ~MAC_CONFIG_RE;

    if (MTU > 1500)
        value |= MAC_CONFIG_2K;

    if (MTU > 2000)
        value |= MAC_CONFIG_JE;

    // default is 1000Mbps
    value &= ~(MAC_CONFIG_FES | MAC_CONFIG_PS);

    switch (Speed) {
        case PHY_SPEED_1000:
            break;

        case PHY_SPEED_100:
            value |= MAC_CONFIG_FES | MAC_CONFIG_PS;
            break;

        case PHY_SPEED_10:
            value |= MAC_CONFIG_PS;
            break;
    }

    dwc_write_reg(value, regbase + MAC_CONFIG);

    /* Enable GMAC interrupts */
    value = MAC_INT_DEFAULT_ENABLE;

    value |= MAC_PCS_IRQ_DEFAULT;

    dwc_write_reg(value, regbase + MAC_INT_EN);

    /* Mask all MMC interrupts. */
    writel(~0, regbase + MMC_RX_INTERRUPT_MASK);
    writel(~0, regbase + MMC_TX_INTERRUPT_MASK);
    writel(~0, regbase + MMC_IPC_RX_INTERRUPT_MASK);

#ifdef CONFIG_ARCH_CHIP_E3
    /* enable bus parity */
    value = readl(regbase + MTL_DPP_CTRL);
    value |= MTL_DPP_CTRL_DATA_PARITY | MTL_DPP_CTRL_SLAVE_PARITY;
    writel(value, regbase + MTL_DPP_CTRL);
#endif

    /* Set user mac addr */
    if (macaddr == NULL) {
        dwmac_set_umac_addr(regbase, deafult_mac_addr, 0);
    }
    else {
        dwmac_set_umac_addr(regbase, macaddr, 0);
    }

    /* Receive all */
    value = 0;
    value |= MAC_PACKET_FILTER_PA;
    dwc_write_reg(value, regbase + MAC_PACKET_FILTER);

    /* Because fillter will set the mac match dorp so init all mac register */
    for (int i = 1; i < 128; i++) {
        dwmac_clr_umac_addr(regbase, i);
    }
}

/**
 * @name:
 * @Desc:
 * @param {void} *Addr
 * @param {uint8} Mode
 * @param {uint32} Queue
 * @return {*}
 */
void dwmac_mac_rx_queue_mcbc_routing(uint32_t regbase, uint8_t channel)
{
    uint32_t value;
    value = readl(regbase + MAC_RXQ_CTRL1);
    value |= MAC_RXQCTRL_MCBCQEN;
    value &= ~MAC_RXQCTRL_MCBCQ_MASK;
    value |= (channel << MAC_RXQCTRL_MCBCQ_SHIFT & \
              MAC_RXQCTRL_MCBCQ_MASK);
    dwc_write_reg(value, regbase + MAC_RXQ_CTRL1);
}

/**
 * @description:
 * @param {*}
 * @return {*}
 */
void dwmac_clr_umac_addr(uint32_t regbase, uint32_t Reg_n)
{
    dwmac_clr_mac_addr(regbase, MAC_ADDR_HIGH(Reg_n),
                       MAC_ADDR_LOW(Reg_n));
}

/**
 * @name:
 * @Desc:
 * @param {*}
 * @return {*}
 */
void dwmac_set_umac_addr(uint32_t regbase,
                         const uint8_t *Addr, uint32_t Reg_n)
{
    dwmac_set_mac_addr(regbase, (uint8_t *)Addr, MAC_ADDR_HIGH(Reg_n),
                       MAC_ADDR_LOW(Reg_n));
}

/**
 * @name:
 * @Desc:
 * @param {*}
 * @return {*}
 */
void dwmac_get_umac_addr(uint32_t regbase,
                         unsigned char *Addr, uint32_t Reg_n)
{
    dwmac_get_mac_addr(regbase, Addr, MAC_ADDR_HIGH(Reg_n),
                       MAC_ADDR_LOW(Reg_n));
}

/**
 * @description:
 * @param {uint8*} src
 * @param {uint8*} dst
 * @return {*}
 */
static uint32_t dwmac_mac_comp(const uint8_t *src, const uint8_t *dst)
{
    for (int i = 0; i < 6; i++) {
        if (src[i] != dst[i]) {
            return -1;
        }
    }

    return 0;
}

/**
 * @description:
 * @param {uint32} regbase
 * @param {uint32} high
 * @return {*}
 */
static uint32_t dwmac_mac_busy(uint32_t regbase, uint32_t i)
{
    uint32_t hi_addr;
    /* Read the MAC address from the hardware */
    hi_addr = readl(regbase + MAC_ADDR_HIGH(i));

    if (hi_addr & MAC_HI_REG_AE) {
        return 1;
    }
    else {
        return 0;
    }
}

/**
 * @name:
 * @Desc:
 * @param {void } *regbase
 * @param {uint32} Mode
 * @param {uint8*} Addr
 * @return {*}
 */
int32_t dwmac_set_filter(uint32_t regbase, uint32_t Mode, const uint8_t *Addr)
{
    uint32_t value = 0;
    uint32_t ret = 0;
    uint8_t mac_buf[6];
    uint8_t use_index = 0xFF, free_index = 0xFF;
    value = readl(regbase + MAC_PACKET_FILTER);
    value &= ~MAC_PACKET_FILTER_PM;
    value |= MAC_PACKET_FILTER_DAIF | MAC_PACKET_FILTER_HPF;

    for (int i = 1; i < 128; i++) {
        if (dwmac_mac_busy(regbase, i)) {
            dwmac_get_umac_addr(regbase, mac_buf, i);

            if ((use_index == 0xFF) && !dwmac_mac_comp(Addr, mac_buf)) {
                use_index = i;
            }
        }
        else if ( free_index == 0xFF) {
            free_index = i;
        }
    }

    /* Have the same mac in table ,not add */
    if (use_index != 0xFF) {
        free_index = 0xFF;
    }

    if (Mode) {
        /* del */
        if (use_index == 0xFF) {
            ret = -1;
        }
        else {
            dwmac_clr_umac_addr(regbase, use_index);
        }
    }
    else {
        /* add */
        if (free_index == 0xFF) {
            ret = -1;
        }
        else {
            dwmac_set_umac_addr(regbase, Addr, free_index);
        }
    }

    dwc_write_reg(value, regbase + MAC_PACKET_FILTER);
    return ret;
}

/* =====================>MAC BLOCK END<==================== */
/* =====================> MTL BLOCK <==================== */
/**
 * @name:
 * @Desc:
 * @param {void} *regbase
 * @param {uint32} alg
 * @return {*}
 */
void dwmac_prog_mtl_rx_algorithms(uint32_t regbase, uint32_t Algorithm)
{
    uint32_t value = readl(regbase + MTL_OPERATION_MODE);

    switch (Algorithm) {
        case MTL_RX_ALGORITHM_SP:
            value &= ~MTL_OPERATION_RAA;
            break;

        case MTL_RX_ALGORITHM_WSP:
            value |= MTL_OPERATION_RAA_WSP;
            break;

        default:
            break;
    }

    dwc_write_reg(value, regbase + MTL_OPERATION_MODE);
}

/**
 * @name:
 * @Desc:
 * @param {void} *regbase
 * @param {uint32} alg
 * @return {*}
 */
void dwmac_prog_mtl_tx_algorithms(uint32_t regbase, uint32_t Algorithm)
{
    uint32_t value = readl(regbase + MTL_OPERATION_MODE);

    value &= ~MTL_OPERATION_SCHALG_MASK;

    switch (Algorithm) {
        case MTL_TX_ALGORITHM_WRR:
            value |= MTL_OPERATION_SCHALG_WRR;
            break;

        case MTL_TX_ALGORITHM_WFQ:
            value |= MTL_OPERATION_SCHALG_WFQ;
            break;

        case MTL_TX_ALGORITHM_DWRR:
            value |= MTL_OPERATION_SCHALG_DWRR;
            break;

        case MTL_TX_ALGORITHM_SP:
            value |= MTL_OPERATION_SCHALG_SP;
            break;

        default:
            break;
    }

    /* flush MTL */
    value |= 0x01 << 9;
    dwc_write_reg(value, regbase + MTL_OPERATION_MODE);
}

/**
 * @name:
 * @Desc:
 * @param {void} *Addr
 * @param {uint32} Queue
 * @param {uint32} channel
 * @return {*}
 */
void dwmac_map_mtl_dma(uint32_t Addr, uint32_t Queue, uint32_t channel)
{
    uint32_t regbase = Addr;
    uint32_t value;

    if (Queue < 4)
        value = readl(regbase + MTL_RXQ_DMA_MAP0);
    else
        value = readl(regbase + MTL_RXQ_DMA_MAP1);

    if (Queue == 0 || Queue == 4) {
        value &= ~MTL_RXQ_DMA_Q04MDMACH_MASK;
        value |= MTL_RXQ_DMA_Q04MDMACH(channel);
    }
    else {
        value &= ~MTL_RXQ_DMA_QXMDMACH_MASK(Queue);
        value |= MTL_RXQ_DMA_QXMDMACH(channel, Queue);
    }

    if (Queue < 4)
        dwc_write_reg(value, regbase + MTL_RXQ_DMA_MAP0);
    else
        dwc_write_reg(value, regbase + MTL_RXQ_DMA_MAP1);
}

/**
 * @name:
 * @Desc:
 * @param {void} *Addr
 * @param {uint8} Mode
 * @param {uint32} Queue
 * @return {*}
 */
void dwmac_rx_queue_enable(uint32_t Addr, uint8_t Mode, uint32_t Queue)
{
    uint32_t regbase = Addr;
    uint32_t value = readl(regbase + MAC_RXQ_CTRL0);

    value &= MAC_RX_QUEUE_CLEAR(Queue);

    if (Mode == MTL_QUEUE_MODE_AVB)
        value |= MAC_RX_AV_QUEUE_ENABLE(Queue);
    else if (Mode == MTL_QUEUE_MODE_DCB)
        value |= MAC_RX_DCB_QUEUE_ENABLE(Queue);

    dwc_write_reg(value, regbase + MAC_RXQ_CTRL0);
}

/**
 * @name:
 * @Desc:
 * @param {*}
 * @return {*}
 */
void dwmac_dma_rx_chan_op_mode(uint32_t regbase, uint32_t channel, int Mode,
                               int FifoSize, uint8_t QMode)
{
    unsigned int rqs = FifoSize / 256 - 1;
    uint32_t mtl_rx_op, mtl_rx_int;

    mtl_rx_op = readl(regbase + MTL_CHAN_RX_OP_MODE(channel));
    /* Forward Undersized Good Packets */
    mtl_rx_op |= MTL_OP_MODE_FUP;

    if (Mode == SF_MODE) {
        mtl_rx_op |= MTL_OP_MODE_RSF;
    }
    else {
        mtl_rx_op &= ~MTL_OP_MODE_RSF;
        mtl_rx_op &= MTL_OP_MODE_RTC_MASK;

        if (Mode <= 32)
            mtl_rx_op |= MTL_OP_MODE_RTC_32;
        else if (Mode <= 64)
            mtl_rx_op |= MTL_OP_MODE_RTC_64;
        else if (Mode <= 96)
            mtl_rx_op |= MTL_OP_MODE_RTC_96;
        else
            mtl_rx_op |= MTL_OP_MODE_RTC_128;
    }

    mtl_rx_op &= ~MTL_OP_MODE_RQS_MASK;
    mtl_rx_op |= rqs << MTL_OP_MODE_RQS_SHIFT;

    /* Enable flow control only if each channel gets 4 KiB or more FIFO and
     * only if channel is not an AVB channel.
     */
    if ((FifoSize >= 4096) && (QMode != MTL_QUEUE_MODE_AVB)) {
        unsigned int rfd, rfa;

        mtl_rx_op |= MTL_OP_MODE_EHFC;

        /* Set Threshold for Activating Flow Control to min 2 frames,
         * i.e. 1500 * 2 = 3000 bytes.
         *
         * Set Threshold for Deactivating Flow Control to min 1 frame,
         * i.e. 1500 bytes.
         */
        switch (FifoSize) {
            case 4096:
                /* This violates the above formula because of FIFO size
                 * limit therefore overflow may occur in spite of this.
                 */
                rfd = 0x03; /* Full-2.5K */
                rfa = 0x01; /* Full-1.5K */
                break;

            case 8192:
                rfd = 0x06; /* Full-4K */
                rfa = 0x0a; /* Full-6K */
                break;

            case 16384:
                rfd = 0x06; /* Full-4K */
                rfa = 0x12; /* Full-10K */
                break;

            default:
                rfd = 0x06; /* Full-4K */
                rfa = 0x1e; /* Full-16K */
                break;
        }

        mtl_rx_op &= ~MTL_OP_MODE_RFD_MASK;
        mtl_rx_op |= rfd << MTL_OP_MODE_RFD_SHIFT;

        mtl_rx_op &= ~MTL_OP_MODE_RFA_MASK;
        mtl_rx_op |= rfa << MTL_OP_MODE_RFA_SHIFT;
    }

    dwc_write_reg(mtl_rx_op, regbase + MTL_CHAN_RX_OP_MODE(channel));

    /* Enable MTL RX overflow */
    mtl_rx_int = readl(regbase + MTL_CHAN_INT_CTRL(channel));

    dwc_write_reg(mtl_rx_int | MTL_RX_OVERFLOW_INT_EN,
                  regbase + MTL_CHAN_INT_CTRL(channel));
}

/*
    Mode transmit store and forward enable or not
    QMode transmit Queue enable or not
    fifo transmit Queue size
*/
void dwmac_dma_tx_chan_op_mode(uint32_t regbase, uint32_t channel, int Mode,
                               int FifoSize, uint8_t QMode)
{
    uint32_t mtl_tx_op = readl(regbase + MTL_CHAN_TX_OP_MODE(channel));
    unsigned int tqs = FifoSize / 256 - 1;

    if (Mode == SF_MODE) {
        /* Transmit COE type 2 cannot be done in cut-through Mode. */
        mtl_tx_op |= MTL_OP_MODE_TSF;
    }
    else {
        mtl_tx_op &= ~MTL_OP_MODE_TSF;
        mtl_tx_op &= MTL_OP_MODE_TTC_MASK;

        /* Set the transmit threshold */
        if (Mode <= 32)
            mtl_tx_op |= MTL_OP_MODE_TTC_32;
        else if (Mode <= 64)
            mtl_tx_op |= MTL_OP_MODE_TTC_64;
        else if (Mode <= 96)
            mtl_tx_op |= MTL_OP_MODE_TTC_96;
        else if (Mode <= 128)
            mtl_tx_op |= MTL_OP_MODE_TTC_128;
        else if (Mode <= 192)
            mtl_tx_op |= MTL_OP_MODE_TTC_192;
        else if (Mode <= 256)
            mtl_tx_op |= MTL_OP_MODE_TTC_256;
        else if (Mode <= 384)
            mtl_tx_op |= MTL_OP_MODE_TTC_384;
        else
            mtl_tx_op |= MTL_OP_MODE_TTC_512;
    }

    /* For an IP with DWC_EQOS_NUM_TXQ == 1, the fields TXQEN and TQS are RO
     * with reset values: TXQEN on, TQS == DWC_EQOS_TXFIFO_SIZE.
     * For an IP with DWC_EQOS_NUM_TXQ > 1, the fields TXQEN and TQS are R/W
     * with reset values: TXQEN off, TQS 256 bytes.
     *
     * TXQEN must be written for multi-channel operation and TQS must
     * reflect the available fifo size per Queue (total fifo size / number
     * of enabled queues).
     */
    mtl_tx_op &= ~MTL_OP_MODE_TXQEN_MASK;

    if (QMode != MTL_QUEUE_MODE_AVB)
        mtl_tx_op |= MTL_OP_MODE_TXQEN;
    else
        mtl_tx_op |= MTL_OP_MODE_TXQEN_AV;

    mtl_tx_op &= ~MTL_OP_MODE_TQS_MASK;
    mtl_tx_op |= tqs << MTL_OP_MODE_TQS_SHIFT;

    dwc_write_reg(mtl_tx_op, regbase + MTL_CHAN_TX_OP_MODE(channel));
}

/* =====================>MTL BLOCK END<==================== */
/* =====================> DMA BLOCK <==================== */
/**
 * @name:
 * @Desc:
 * @param {void } *regbase
 * @return {*}
 */
int32_t dwmac_dma_reset(uint32_t regbase)
{
    int32_t limit;
    uint32_t value = readl(regbase + DMA_BUS_MODE);

    /* DMA SW reset */
    value |= DMA_BUS_MODE_SFT_RESET;
    dwc_write_reg(value, regbase + DMA_BUS_MODE);
    limit = 10;

    while (limit--) {
        if (!(readl(regbase + DMA_BUS_MODE) & DMA_BUS_MODE_SFT_RESET))
            break;

        mdelay(10);
    }

    if (limit <= 0) {
        return -1;
    }

    return 0;
}

void dwmac_set_rx_list_ptr(uint32_t regbase, uint32_t list_ptr,
                           uint32_t channel)
{
    writel(list_ptr, regbase + DMA_CHAN_RX_BASE_ADDR(channel));
}

void dwmac_set_tx_list_ptr(uint32_t regbase, uint32_t list_ptr,
                           uint32_t channel)
{
    writel(list_ptr, regbase + DMA_CHAN_TX_BASE_ADDR(channel));
}

void dwmac_set_rx_tail_ptr(uint32_t regbase, uint32_t tail_ptr,
                           uint32_t channel)
{
    dwc_write_reg(tail_ptr, regbase + DMA_CHAN_RX_END_ADDR(channel));
}

void dwmac_set_tx_tail_ptr(uint32_t regbase, uint32_t tail_ptr,
                           uint32_t channel)
{
    dwc_write_reg(tail_ptr, regbase + DMA_CHAN_TX_END_ADDR(channel));
}

void dwmac_rx_trigger(uint32_t regbase, uint32_t channel)
{
    uint32_t tail = readl(regbase + DMA_CHAN_RX_END_ADDR(channel));
    dwc_write_reg(tail, regbase + DMA_CHAN_RX_END_ADDR(channel));
}

void dwmac_dma_start_tx(uint32_t regbase, uint32_t channel)
{
    uint32_t value = readl(regbase + DMA_CHAN_TX_CONTROL(channel));

    value |= DMA_CONTROL_ST;
    dwc_write_reg(value, regbase + DMA_CHAN_TX_CONTROL(channel));

    value = readl(regbase + MAC_CONFIG);
    value |= MAC_CONFIG_TE;
    dwc_write_reg(value, regbase + MAC_CONFIG);
}

void dwmac_dma_stop_tx(uint32_t regbase, uint32_t channel)
{
    uint32_t value = readl(regbase + DMA_CHAN_TX_CONTROL(channel));

    value &= ~DMA_CONTROL_ST;
    dwc_write_reg(value, regbase + DMA_CHAN_TX_CONTROL(channel));

    value = readl(regbase + MAC_CONFIG);
    value &= ~MAC_CONFIG_TE;
    dwc_write_reg(value, regbase + MAC_CONFIG);
}

void dwmac_dma_start_rx(uint32_t regbase, uint32_t channel)
{
    uint32_t value = readl(regbase + DMA_CHAN_RX_CONTROL(channel));

    value |= DMA_CONTROL_SR;

    dwc_write_reg(value, regbase + DMA_CHAN_RX_CONTROL(channel));

    value = readl(regbase + MAC_CONFIG);
    value |= MAC_CONFIG_RE;
    dwc_write_reg(value, regbase + MAC_CONFIG);
}

void dwmac_dma_stop_rx(uint32_t regbase, uint32_t channel)
{
    uint32_t value = readl(regbase + DMA_CHAN_RX_CONTROL(channel));

    value &= ~DMA_CONTROL_SR;
    dwc_write_reg(value, regbase + DMA_CHAN_RX_CONTROL(channel));

    value = readl(regbase + MAC_CONFIG);
    value &= ~MAC_CONFIG_RE;
    dwc_write_reg(value, regbase + MAC_CONFIG);
}

void dwmac_set_tx_ring_len(uint32_t regbase, uint32_t len, uint32_t channel)
{
    dwc_write_reg(len, regbase + DMA_CHAN_TX_RING_LEN(channel));
}

void dwmac_set_rx_ring_len(uint32_t regbase, uint32_t len, uint32_t channel)
{
    dwc_write_reg(len, regbase + DMA_CHAN_RX_RING_LEN(channel));
}

void dwmac_enable_dma_rx_int(uint32_t regbase, uint32_t channel, bool enable)
{
    uint32_t val = readl(regbase + DMA_CHAN_INTR_ENA(channel));

    val |= DMA_CHAN_INTR_DEFAULT_MASK;

    if (enable)
        val |= DMA_CHAN_INTR_ENA_RIE;
    else
        val &= ~DMA_CHAN_INTR_ENA_RIE;

    dwc_write_reg(val, regbase + DMA_CHAN_INTR_ENA(channel));
}

void dwmac_enable_dma_tx_int(uint32_t regbase, uint32_t channel, bool enable)
{
    uint32_t val = readl(regbase + DMA_CHAN_INTR_ENA(channel));

    val |= DMA_CHAN_INTR_DEFAULT_MASK;

    if (enable)
        val |= DMA_CHAN_INTR_ENA_TIE;
    else
        val &= ~DMA_CHAN_INTR_ENA_TIE;

    dwc_write_reg(val, regbase + DMA_CHAN_INTR_ENA(channel));
}

uint32_t dwmac_get_dma_int_status(uint32_t regbase)
{
    return readl(regbase + DMA_STATUS);
}

uint32_t dwmac_get_mac_int_status(uint32_t regbase)
{
    return readl(regbase + MAC_INT_STATUS);
}

uint32_t dwmac_get_mac_phyif_control_status(uint32_t regbase)
{
    return readl(regbase + MAC_PHYIF_CONTROL_STATUS);
}

uint32_t dwmac_get_mac_an_status(uint32_t regbase)
{
    return readl(regbase + MAC_AN_STATUS);
}

uint32_t dwmac_get_mac_pmt_control_status(uint32_t regbase)
{
    return readl(regbase + MAC_PMT_CONTROL_STATUS);
}

uint32_t dwmac_get_mac_lpi_control_status(uint32_t regbase)
{
    return readl(regbase + MAC_LPI_CONTROL_STATUS);
}

uint32_t dwmac_get_mtl_int_status(uint32_t regbase)
{
    return readl(regbase + MTL_INT_STATUS);
}

uint32_t dwmac_get_mtl_q_int_status(uint32_t regbase, uint32_t channel)
{
    return readl(regbase + MTL_CHAN_INT_CTRL(channel));
}

void dwmac_clr_mtl_q_int_status(uint32_t regbase, uint32_t channel, uint32_t val)
{
    writel(val, regbase + MTL_CHAN_INT_CTRL(channel));
}

uint32_t dwmac_get_dma_chn_status(uint32_t regbase, uint32_t channel)
{
    return readl(regbase + DMA_CHAN_STATUS(channel));
}

void dwmac_clear_dma_chn_int_status(uint32_t regbase, uint32_t channel)
{
    dwc_write_reg(0x3fffc7, regbase + DMA_CHAN_STATUS(channel));
}

void dwmac_enable_dma_irq(uint32_t regbase, uint32_t channel, bool rx, bool tx)
{
    uint32_t irq_enable_mask = DMA_CHAN_INTR_DEFAULT_MASK;
    /* clearn irq */
    dwc_write_reg(0x3fffc7, regbase + DMA_CHAN_STATUS(channel));

    /* enable irq */
    if (!rx) {
        irq_enable_mask &= ~DMA_CHAN_INTR_ENA_RIE;
    }
    else {
        irq_enable_mask |= DMA_CHAN_INTR_ENA_RIE;
    }

    if (!tx) {
        irq_enable_mask &= ~DMA_CHAN_INTR_ENA_TIE;
    }
    else {
        irq_enable_mask |= DMA_CHAN_INTR_ENA_TIE;
    }

    dwc_write_reg(irq_enable_mask, regbase + DMA_CHAN_INTR_ENA(channel));
}

void dwmac_disable_dma_irq(uint32_t regbase, uint32_t channel)
{
    dwc_write_reg(0, regbase + DMA_CHAN_INTR_ENA(channel));
}

void dwmac_dma_init_rx_chan(uint32_t regbase, uint32_t base_ptr, uint32_t rbsz,
                            uint32_t channel)
{
    uint32_t value;

    value = readl(regbase + DMA_CHAN_RX_CONTROL(channel));
    value = value | (rbsz << DMA_RBSZ_SHIFT);
    /* Set max rx burst length to 16. Taishan only supports
     * burst length less than or equal to 16.
     */
    value &= ~GENMASK(21, 16);
    value |= DMA_CONTROL_RXPBL(16);
    dwc_write_reg(value, regbase + DMA_CHAN_RX_CONTROL(channel));

    dwc_write_reg(base_ptr, regbase + DMA_CHAN_RX_BASE_ADDR(channel));
}

void dwmac_dma_init_tx_chan(uint32_t regbase, uint32_t base_ptr,
                            uint32_t channel)
{
    uint32_t value;

    value = readl(regbase + DMA_CHAN_TX_CONTROL(channel));

    /* Enable OSP to get best performance */
    value |= DMA_CONTROL_OSP;
    /* Set max tx burst length to 16. Taishan only supports
     * burst length less than or equal to 16.
     */
    value &= ~GENMASK(21, 16);
    value |= DMA_CONTROL_TXPBL(16);

    dwc_write_reg(value, regbase + DMA_CHAN_TX_CONTROL(channel));
    dwc_write_reg(base_ptr, regbase + DMA_CHAN_TX_BASE_ADDR(channel));
}

void dwmac_dma_init_channel(uint32_t regbase, uint32_t Flags, \
                            uint32_t channel, uint32_t skip)
{
    uint32_t value;

    /* common channel control register config */
    value = readl(regbase + DMA_CHAN_CONTROL(channel));

    if (Flags & DMA_BUS_MODE_PBL)
        value = value | DMA_BUS_MODE_PBL;
    else
        value = value & (~DMA_BUS_MODE_PBL);

    value |= ((skip << DMA_CONTROL_DSL_SHIFT) & DMA_CONTROL_DSL_MASK);

    dwc_write_reg(value, regbase + DMA_CHAN_CONTROL(channel));

}

void dwmac_dma_bus_init(uint32_t regbase, uint32_t Flags)
{
    uint32_t value = readl(regbase + DMA_SYS_BUS_MODE);

    /* Set the Fixed burst Mode */
    if (Flags & DMA_SYS_BUS_FB)
        value |= DMA_SYS_BUS_FB;

    /* Mixed Burst has no effect when fb is set */
    if (Flags & DMA_SYS_BUS_MB)
        value |= DMA_SYS_BUS_MB;

    if (Flags & DMA_SYS_BUS_AAL)
        value |= DMA_SYS_BUS_AAL;

    if (Flags & DMA_SYS_BUS_EAME)
        value |= DMA_SYS_BUS_EAME;

    if (Flags & DMA_AXI_BLEN256)
        value |= DMA_AXI_BLEN256;
    else if (Flags & DMA_AXI_BLEN128)
        value |= DMA_AXI_BLEN128;
    else if (Flags & DMA_AXI_BLEN64)
        value |= DMA_AXI_BLEN64;
    else if (Flags & DMA_AXI_BLEN32)
        value |= DMA_AXI_BLEN32;
    else if (Flags & DMA_AXI_BLEN16)
        value |= DMA_AXI_BLEN16;
    else if (Flags & DMA_AXI_BLEN8)
        value |= DMA_AXI_BLEN8;
    else if (Flags & DMA_AXI_BLEN4)
        value |= DMA_AXI_BLEN4;

    dwc_write_reg(value, regbase + DMA_SYS_BUS_MODE);
}

/**
 * @description:
 * @param {uint32} regbase
 * @param {uint32} channel
 * @return {*}
 */
uint32_t dwmac_dma_rx_state_get(uint32_t regbase, uint32_t channel)
{
    uint32_t state = 0;

    switch (channel) {
        case 0:
        case 1:
        case 2:
            state = (readl(regbase + DMA_DEBUG_STATUS_0) >> (channel * 8 + 8)) & 0xF;
            break;

        case 3:
        case 4:
        case 5:
            state = (readl(regbase + DMA_DEBUG_STATUS_2) >> ((channel - 3) * 8)) & 0xF;
            break;

        case 6:
        case 7:
            state = (readl(regbase + DMA_DEBUG_STATUS_2) >> ((channel - 6) * 8)) & 0xF;
            break;

        default:
            break;
    }

    return state;
}

void dwmac_rx_watchdog(uint32_t regbase, uint32_t riwt, uint32_t channel)
{
    dwc_write_reg(riwt, regbase + DMA_CHAN_RX_WATCHDOG(channel));
}

void dwmac_set_bfsize(uint32_t regbase, int BuffSize, uint32_t channel)
{
    uint32_t value = readl(regbase + DMA_CHAN_RX_CONTROL(channel));

    value &= ~DMA_RBSZ_MASK;
    value |= (BuffSize << DMA_RBSZ_SHIFT) & DMA_RBSZ_MASK;

    dwc_write_reg(value, regbase + DMA_CHAN_RX_CONTROL(channel));
}

/* =====================>DMA BLOCK END<==================== */
#define MII_BUSY 0x00000001
#define MII_WRITE 0x00000002
#define MII_DATA_MASK 0xFFFF

/* GMAC4 defines */
#define MII_MAC_GOC_SHIFT       2
#define MII_MAC_REG_ADDR_SHIFT  16
#define MII_MAC_WRITE           (1 << MII_MAC_GOC_SHIFT)
#define MII_MAC_READ            (3 << MII_MAC_GOC_SHIFT)
#define MII_MAC_C45E            0x02

#define MII_ADDR_OFFSET         21
#define MII_ADDR_MASK           (0x1F<<MII_ADDR_OFFSET)
#define MII_REG_OFFSET          16
#define MII_REG_MASK            (0x1F<<MII_REG_OFFSET)
#define MII_CLK_CSR_OFFSET      8
#define MII_CLK_CSR_MASK        (0xf<<8)

/* Or MII_ADDR_C45 into regnum for read/write on mii_bus to enable the 21 bit
   IEEE 802.3ae clause 45 addressing Mode used by 10GIGE phy chips. */
#define MII_ADDR_C45 (1<<30)
#define MII_DEVADDR_C45_SHIFT   16
#define MII_REGADDR_C45_MASK    0xFFFF
#define CLK_CSR     0x5

static inline int32_t poll_mdio_busy(uint32_t regbase, uint32_t timeout)
{

    uint32_t limit = timeout;

    do {
        if (readl(regbase) & MII_BUSY) {
            udelay(1000);
        }
        else {
            return 0;
        }

        limit--;
    }
    while (limit);

    return -1;
}

/**
 * @description: this function support to Access Clause 45 Phy
 * @param {uint8} phyaddr
 * @param {uint8} device
 * @param {uint16} phyreg
 * @return {*}
 */
int32_t mac_mdio_read(uint32_t regbase, uint8_t phyaddr, uint8_t device,
                      uint16_t phyreg)
{
    uint32_t mii_address = (uint32_t)regbase + MAC_MDIO_ADDR;
    uint32_t mii_data = (uint32_t)regbase + MAC_MDIO_DATA;
    uint32_t value = MII_BUSY;
    int32_t data = 0;

    value |= (phyaddr << MII_ADDR_OFFSET) & MII_ADDR_MASK;
    value |= (CLK_CSR << MII_CLK_CSR_OFFSET)& MII_CLK_CSR_MASK;
    value |= MII_MAC_READ;

    if (PHY_CLAUSE_22 == device) {
        value |= (phyreg << MII_REG_OFFSET) & MII_REG_MASK;
    }
    else {
        value |= MII_MAC_C45E;
        value |= (device << MII_REG_OFFSET) & MII_REG_MASK;
        data |= (phyreg & MII_REGADDR_C45_MASK) << MII_DEVADDR_C45_SHIFT;
    }

    if (poll_mdio_busy(mii_address, 10000)) {
        ssdk_printf(SSDK_INFO, "PHY awalys busyed.\n");
        return -1;
    }

    dwc_write_reg(data, mii_data);
    dwc_write_reg(value, mii_address);

    if (poll_mdio_busy(mii_address, 10000))
        return -2;

    /* Read the data from the MII data register */
    data = (int32_t)(readl(mii_data) & MII_DATA_MASK);

    return data;
}

/**
 * @description:this function support to Access Clause 45 Phy when device < 32
 * @param {uint8} phyaddr
 * @param {uint8} phyregspace
 * @param {uint32} phyreg
 * @param {uint16} phydata
 * @return {*}
 */
int32_t mac_mdio_write(uint32_t regbase, uint8_t phyaddr, uint8_t device,
                       uint16_t phyreg, uint16_t phydata)
{
    uint32_t mii_address = (uint32_t)regbase + MAC_MDIO_ADDR;
    uint32_t mii_data = (uint32_t)regbase + MAC_MDIO_DATA;

    uint32_t value = MII_BUSY;
    uint32_t data = phydata & 0xFFFF;

    value |= (phyaddr << MII_ADDR_OFFSET) & MII_ADDR_MASK;
    value |= (CLK_CSR << MII_CLK_CSR_OFFSET) & MII_CLK_CSR_MASK;
    value |= MII_MAC_WRITE;

    if (PHY_CLAUSE_22 == device) {
        value |= (phyreg << MII_REG_OFFSET) & MII_REG_MASK;
    }
    else {
        value |= MII_MAC_C45E;
        value |= (device << MII_REG_OFFSET) & MII_REG_MASK;
        data |= (phyreg & MII_REGADDR_C45_MASK) << MII_DEVADDR_C45_SHIFT;
    }

    /* Wait until any existing MII operation is complete */
    if (poll_mdio_busy(mii_address, 10000)) {
        return -1;
    }

    /* Set the MII address register to write */
    dwc_write_reg(data, mii_data);
    dwc_write_reg(value, mii_address);

    /* Wait until any existing MII operation is complete */
    return poll_mdio_busy(mii_address, 10000);
}

/**
 * Description:
 * @regbase: points to the reg Addr
 */
bool dwmac_access_test(uint32_t regbase)
{
    return (readl(regbase + MAC_VERSION) & 0xFF) == 0x51;
}

uint8_t *dwmac_get_default_mac_addr(void)
{
    return deafult_mac_addr;
}