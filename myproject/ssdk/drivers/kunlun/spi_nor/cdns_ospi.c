/**
 * @file cdns_ospi.c
 * @brief Cadence ospi host low level driver C code.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <string.h>
#include <arch/cache.h>

#include "cdns_ospi.h"

static uint32_t g_dummy = 0;

static inline uint8_t my_log2_uint(uint64_t val)
{
    uint8_t ret = 0;

    while ((val != 0ull) && (val % 2ull == 0ull)) {
        val /= 2ull;
        ret++;
    }

    return ret;
}

static inline void readsl(uint32_t *addr, uint32_t *data, uint32_t len)
{
    while (len-- != 0u) {
        *data++ = readl(addr++);
    }
}

static inline void writesl(uint32_t *addr, uint32_t const *data, uint32_t len)
{
    while (len-- != 0u) {
        writel(*data++, addr++);
    }
}

static void cospi_enable(struct cospi_pdata *cospi, bool enable)
{
    uint32_t reg;
    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);

    if (enable) {
        reg |= COSPI_REG_CONFIG_ENABLE_MASK;
    }
    else {
        reg &= ~COSPI_REG_CONFIG_ENABLE_MASK;
    }

    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

static uint32_t cospi_idle_status(struct cospi_pdata *cospi)
{
    uint32_t reg;
    reg = readl(cospi->apb_base + COSPI_REG_CONFIG);
    return reg & COSPI_REG_CONFIG_IDLE_MASK;
}

static int cospi_wait_idle(struct cospi_pdata *cospi)
{
    uint32_t idle_latency_cycles = 4;
    uint32_t idle_count = 0;
    uint32_t polling_cycles = 0;

    while (1) {
        if (cospi_idle_status(cospi) != 0u) {
            idle_count++;
        }
        else {
            idle_count = 0;
        }

        /* When IDLE bit asserted, need wait 4 cycyles of ref_clk. */
        if (idle_count >= idle_latency_cycles) {
            return 0;
        }
        else if (polling_cycles > COSPI_IDLE_TIMEOUT_CYCLES) {
            return -1;
        }

        polling_cycles++;
    }
}

static int cospi_wait_for_bit_times(addr_t reg, const uint32_t mask, bool clear,
                                    uint32_t times)
{
    uint32_t val;
    uint32_t count = 0;

    while (count < times) {
        val = readl(reg);

        if (clear) {
            val = ~val;
        }

        val &= mask;

        if (val == mask) {
            return 0;
        }

        count++;
    }

    return -1;
}

static uint32_t cospi_get_rd_sram_level(struct cospi_pdata *cospi)
{
    uint32_t reg = readl(cospi->apb_base + COSPI_REG_SDRAMLEVEL);
    reg >>= COSPI_REG_SDRAMLEVEL_RD_LSB;
    return reg & COSPI_REG_SDRAMLEVEL_RD_MASK;
}

static uint32_t cospi_get_wr_sram_level(struct cospi_pdata *cospi)
{
    uint32_t reg = readl(cospi->apb_base + COSPI_REG_SDRAMLEVEL);
    reg >>= COSPI_REG_SDRAMLEVEL_WR_LSB;
    return reg & COSPI_REG_SDRAMLEVEL_WR_MASK;
}

static void cospi_size_set(struct spi_nor *nor)
{
    uint32_t reg = 0;
    struct cospi_pdata *cospi = nor->host->priv_data;
    cospi->page_size = nor->info.page_size;
    cospi->block_power_index = my_log2_uint(cospi->page_size);

    // TODO: for support direct access ahb decode
#if 0
    reg = ospi_readl(cospi->apb_base + COSPI_REG_SIZE);

    /* Config spi flash device size */
    reg &= ~(0x3 << (COSPI_REG_SIZE_SIZE_LSB + 2 * nor->cs));
    reg |= ((u64)nor->size << (COSPI_REG_SIZE_SIZE_LSB + 2 * nor->cs));
#endif
    reg = ((uint32_t)cospi->block_power_index << COSPI_REG_SIZE_BLOCK_LSB);
    reg |= (cospi->page_size << COSPI_REG_SIZE_PAGE_LSB);

    ospi_writel(reg, cospi->apb_base + COSPI_REG_SIZE);
}

static void cospi_cs_set(struct spi_nor *nor)
{
    uint32_t reg;
    uint8_t cs = nor->cs;
    struct cospi_pdata *cospi = nor->host->priv_data;

    cospi_wait_idle(cospi);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);

    if (cospi->cs_decoded) {
        reg |= COSPI_REG_CONFIG_DECODE_MASK;
    }
    else {
        reg &= ~COSPI_REG_CONFIG_DECODE_MASK;
        cs = ~(1u << cs);
    }

    /* Set DTR protocol enable or not */
    reg &= ~COSPI_REG_CONFIG_DTR_ENABLE_MASK;
    reg |= ((uint32_t)nor->octal_dtr_en << COSPI_REG_CONFIG_DTR_ENABLE_LSB);

    reg &=
        ~(COSPI_REG_CONFIG_CHIPSELECT_MASK << COSPI_REG_CONFIG_CHIPSELECT_LSB);
    reg |= ((uint32_t)cs & COSPI_REG_CONFIG_CHIPSELECT_MASK)
           << COSPI_REG_CONFIG_CHIPSELECT_LSB;

    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

static void cospi_baudrate_set(struct cospi_pdata *cospi)
{
    uint32_t reg, clk_div;

    /* Recalculate the baudrate divisor based on OSPI specification. */
    clk_div = DIV_ROUND_UP(cospi->ref_clk_hz, 2u * cospi->sclk) - 1u;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg &= ~(COSPI_REG_CONFIG_BAUD_MASK << COSPI_REG_CONFIG_BAUD_LSB);
    reg |= (clk_div & COSPI_REG_CONFIG_BAUD_MASK) << COSPI_REG_CONFIG_BAUD_LSB;
    cospi_wait_idle(cospi);
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

static void cospi_delay_set(struct cospi_pdata *cospi)
{
    uint32_t reg, cssot, cseot, csdads, csda;

    /* the default value is 30ns */
    cssot = OSPI_NS_2_TICKS(cospi->ref_clk_hz, 30u);
    cseot = OSPI_NS_2_TICKS(cospi->ref_clk_hz, 30u);
    csdads = OSPI_NS_2_TICKS(cospi->ref_clk_hz, 30u);
    csda = OSPI_NS_2_TICKS(cospi->ref_clk_hz, 30u);

    reg = (cssot & COSPI_REG_DELAY_CSSOT_MASK) << COSPI_REG_DELAY_CSSOT_LSB;
    reg |= (cseot & COSPI_REG_DELAY_CSEOT_MASK) << COSPI_REG_DELAY_CSEOT_LSB;
    reg |= (csdads & COSPI_REG_DELAY_CSDADS_MASK) << COSPI_REG_DELAY_CSDADS_LSB;
    reg |= (csda & COSPI_REG_DELAY_CSDA_MASK) << COSPI_REG_DELAY_CSDA_LSB;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_DELAY);
}

static void cospi_capture_set(struct spi_nor *nor)
{
    uint32_t reg;
    struct cospi_pdata *cospi = nor->host->priv_data;

    if (nor->mst_slv_mode)
        cospi->dqs_en = nor->dqs_en;

    if (nor->octal_dtr_en) {
        cospi->master_delay = 2;
    }
    else {
        cospi->master_delay = 0;
    }

    cospi->capture_delay =
        cospi->master_delay + COSPI_READCAPTURE_DELAY_DEFAULT;

    if (nor->mst_slv_mode && nor->octal_dtr_en) {
        cospi->master_delay = 0; // 50mhz 8 line ddr
        cospi->capture_delay = 2;
    }

    if (nor->phy_en) {
        cospi->master_delay = 0;
        cospi->capture_delay = 0;
    }

    if (nor->info.name) {
        reg = ((nor->info.status_dummy + g_dummy) & COSPI_REG_FLASH_STATUS_DUMMY_MASK)
              << COSPI_REG_FLASH_STATUS_DUMMY_LSB;
        ospi_writel(reg, cospi->apb_base + COSPI_REG_FLASH_STATUS);
    }

    reg = ospi_readl(cospi->apb_base + COSPI_REG_RDCAP);

    if (cospi->rclk_loopback)
        reg &= ~(1u << COSPI_REG_RDCAP_BYPASS_LSB);
    else
        reg |= (1u << COSPI_REG_RDCAP_BYPASS_LSB);

    if (cospi->dqs_en)
        reg |= (1u << COSPI_REG_RDCAP_DQSEN_LSB);
    else
        reg &= ~(1u << COSPI_REG_RDCAP_DQSEN_LSB);

    reg &= ~(COSPI_REG_RDCAP_DELAY_MASK << COSPI_REG_RDCAP_DELAY_LSB);

    reg |= (((uint32_t)cospi->capture_delay & COSPI_REG_RDCAP_DELAY_MASK)
            << COSPI_REG_RDCAP_DELAY_LSB);

    reg &= ~(COSPI_REG_RDCAP_MASTER_DELAY_MASK
             << COSPI_REG_RDCAP_MASTER_DELAY_LSB);

    reg |= (((uint32_t)cospi->master_delay & COSPI_REG_RDCAP_MASTER_DELAY_MASK)
            << COSPI_REG_RDCAP_MASTER_DELAY_LSB);

    ospi_writel(reg, cospi->apb_base + COSPI_REG_RDCAP);
}

static void cospi_addr_bytes_set(struct cospi_pdata *cospi)
{
    uint32_t reg;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_SIZE);
    /* Config spi flash device addr bytes */
    reg &= ~(COSPI_REG_SIZE_ADDRESS_MASK << COSPI_REG_SIZE_ADDRESS_LSB);

    if (cospi->addr_bytes > 1u)
        reg |=
            (((uint32_t)cospi->addr_bytes - 1u) << COSPI_REG_SIZE_ADDRESS_LSB);

    ospi_writel(reg, cospi->apb_base + COSPI_REG_SIZE);
    return;
}

static void cospi_set_bytes_per_device_page(struct spi_nor *nor)
{
    struct cospi_pdata *cospi = nor->host->priv_data;
    cospi->page_size = nor->info.page_size;
    /* Config spi flash device page sizes */
    RMWREG32(cospi->apb_base + COSPI_REG_SIZE, COSPI_REG_SIZE_PAGE_LSB, 12,
             cospi->page_size);
    return;
}

/* all read ops, write ops, stig ops use the read inst reg's inst type */
static inline void cospi_inst_width_set(struct cospi_pdata *cospi,
                                        uint8_t inst_width)
{
    uint32_t reg;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_RD_INSTR);
    reg &= ~(COSPI_REG_RD_INSTR_TYPE_INST_MASK
             << COSPI_REG_RD_INSTR_TYPE_INST_LSB);
    reg |= (uint32_t)inst_width << COSPI_REG_RD_INSTR_TYPE_INST_LSB;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_RD_INSTR);
}


static void cospi_proto_mst_slv_setup(struct spi_nor *nor, enum spi_nor_ops ops,
                                      uint32_t opcode)
{
    struct cospi_pdata *cospi = nor->host->priv_data;

    switch (ops) {
        case SPI_NOR_OPS_READ:
            if (nor->cmd_bytes > 1) { //cmd 2bytes opcode
                if (SNOR_INST_LANS(nor->reg_proto) == 3) {
                    RMWREG32(cospi->apb_base + COSPI_REG_CONFIG,
                             COSPI_REG_CONFIG_DUAL_OPCODE_ENABLE, 1u, 1);//set 2bytes opcode
                    RMWREG32(cospi->apb_base + COSPI_REG_OPCODE_EXTENSION,
                             COSPI_REG_SUPPLEMENT_ANY_READ_OPCODE_LSB, 8u,
                             (opcode >> 8) & 0xff);//fill read opcode high bytes
                }
                else {
                    ;
                }
            }
            else {
                RMWREG32(cospi->apb_base + COSPI_REG_CONFIG,
                         COSPI_REG_CONFIG_DUAL_OPCODE_ENABLE, 1u, 0);//set 1byte opcode
            }

            break;

        case SPI_NOR_OPS_WRITE:
            if (nor->cmd_bytes > 1) {//cmd 2bytes opcode
                if (SNOR_INST_LANS(nor->reg_proto) == 3) {
                    RMWREG32(cospi->apb_base + COSPI_REG_CONFIG,
                             COSPI_REG_CONFIG_DUAL_OPCODE_ENABLE, 1u, 1);//set 2bytes opcode
                    RMWREG32(cospi->apb_base + COSPI_REG_OPCODE_EXTENSION,
                             COSPI_REG_SUPPLEMENT_ANY_WRITE_OPCODE_LSB, 8u,
                             (opcode >> 8) & 0xff);//fill write opcode high bytes
                }
                else {
                    ;
                }
            }
            else {
                RMWREG32(cospi->apb_base + COSPI_REG_CONFIG,
                         COSPI_REG_CONFIG_DUAL_OPCODE_ENABLE, 1u, 0);//set 1byte opcode
            }

            break;

        default:
            return;
    }
}

static void cospi_proto_setup(struct spi_nor *nor, enum spi_nor_ops ops)
{
    uint32_t reg = 0;
    addr_t address = 0;
    uint32_t protocal = 0;
    uint32_t dummy = 0;
    uint32_t inst_width;
    uint32_t addr_width;
    uint32_t data_width;
    uint32_t addr_bytes;
    uint32_t opcode;

    struct cospi_pdata *cospi = nor->host->priv_data;

    switch (ops) {
        case SPI_NOR_OPS_READ:
            protocal = nor->info.read_proto;
            address = COSPI_REG_RD_INSTR;
            dummy = nor->info.read_dummy + g_dummy;

            if (!nor->hyperbus_mode && !nor->octal_dtr_en && (protocal & SNOR_DTR_PROTO)) {
                reg |= (1u << COSPI_REG_RD_INSTR_DDR_EN);
            }

            break;

        case SPI_NOR_OPS_WRITE:
            protocal = nor->info.write_proto;
            address = COSPI_REG_WR_INSTR;
            /* not need set write dummy */
            dummy = 0;
            break;

        case SPI_NOR_OPS_ERASE:
            /* not need do anything, because erase use command write func */
            return;

        default:
            return;
    }

    opcode = (protocal >> SNOR_OPCODE_PROTO_LSB);
    inst_width =
        SNOR_INST_LANS_PROTO_MASK & (protocal >> SNOR_INST_LANS_PROTO_LSB);
    addr_width =
        SNOR_ADDR_LANS_PROTO_MASK & (protocal >> SNOR_ADDR_LANS_PROTO_LSB);
    data_width =
        SNOR_DATA_LANS_PROTO_MASK & (protocal >> SNOR_DATA_LANS_PROTO_LSB);
    addr_bytes = nor->addr_width;

    reg |= opcode & 0xff;

    reg |= (dummy & COSPI_REG_RDWR_INSTR_DUMMY_MASK)
           << COSPI_REG_RDWR_INSTR_DUMMY_LSB;
    reg |= addr_width << COSPI_REG_RDWR_INSTR_ADDR_LSB;
    reg |= data_width << COSPI_REG_RDWR_INSTR_DATA_LSB;

    if (ops == SPI_NOR_OPS_READ) {
        reg |= (uint32_t)inst_width << COSPI_REG_RD_INSTR_TYPE_INST_LSB;
    }

    ospi_writel(reg, cospi->apb_base + address);

    if (nor->mst_slv_mode) {
        cospi_proto_mst_slv_setup(nor, ops, opcode);
    }

    if (inst_width != cospi->inst_type) {
        cospi_inst_width_set(cospi, (uint8_t)inst_width);
        cospi->inst_type = (uint8_t)inst_width;
    }

    /* The addr_bytes may be changed by other flash operation */
    if (cospi->addr_bytes != addr_bytes) {
        cospi->addr_bytes = (uint8_t)addr_bytes;
        cospi_addr_bytes_set(cospi);
        cospi_set_bytes_per_device_page(nor);
    }
}

static int cospi_exec_flash_cmd(struct cospi_pdata *cospi, uint32_t reg)
{
    int ret;

    /* Write the CMDCTRL without start execution. */
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CMDCTRL);
    /* Start execute */
    reg |= COSPI_REG_CMDCTRL_EXECUTE_MASK;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CMDCTRL);

    /* Polling for completion. */
    ret = cospi_wait_for_bit_times(cospi->apb_base + COSPI_REG_CMDCTRL,
                                   COSPI_REG_CMDCTRL_INPROGRESS_MASK, true,
                                   10000u);

    if (ret != 0) {
        ssdk_printf(SSDK_ERR, "Flash command execution timed out.\n");
        ospi_readl(cospi->apb_base + COSPI_REG_CMDCTRL);
        return ret;
    }

    /* Polling OSPI idle status. */
    return cospi_wait_idle(cospi);
}

static int cospi_exec_flash_cmd_ext(struct cospi_pdata *cospi, uint32_t reg,
                                    uint32_t ext_reg, uint8_t *buf,
                                    uint32_t len)
{
    uint32_t i;
    uint32_t tmp_reg;
    int ret;

    /* Write the CMDCTRL without start execution. */
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CMDCTRL);

    /* Write the CMDCTRL_MEM without start request. */
    ospi_writel(ext_reg, cospi->apb_base + COSPI_REG_CMDCTRL_MEM);

    /* Start execute */
    reg |= COSPI_REG_CMDCTRL_MEM_EXECUTE_MASK;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CMDCTRL);

    /* Polling for completion. */
    ret = cospi_wait_for_bit_times(cospi->apb_base + COSPI_REG_CMDCTRL,
                                   COSPI_REG_CMDCTRL_INPROGRESS_MASK, true,
                                   10000u);

    if (ret != 0) {
        ssdk_printf(SSDK_ERR, "Flash command execution timed out.\n");
        return ret;
    }

    for (i = 0; i < len; i++) {
        tmp_reg = (i << COSPI_REG_CTDCTRL_MEM_ADDR_LSB);
        ospi_writel(tmp_reg, cospi->apb_base + COSPI_REG_CMDCTRL_MEM);

        tmp_reg |= COSPI_REG_CMDCTRL_MEM_EXECUTE_MASK;
        ospi_writel(tmp_reg, cospi->apb_base + COSPI_REG_CMDCTRL_MEM);
        /* Polling for completion. */
        ret = cospi_wait_for_bit_times(cospi->apb_base + COSPI_REG_CMDCTRL_MEM,
                                       COSPI_REG_CMDCTRL_MEM_INPROGRESS_MASK,
                                       true, 1000u);

        if (ret != 0) {
            ssdk_printf(SSDK_ERR, "Membank request timed out.\n");
            return ret;
        }

        *(buf + i) = readb(cospi->apb_base + COSPI_REG_CTDCTRL_MEM_DATA);
    }

    /* disable membank */
    ospi_writel((uint32_t)0u, cospi->apb_base + COSPI_REG_CMDCTRL);
    /* Polling OSPI idle status. */
    return cospi_wait_idle(cospi);
}

static uint32_t cospi_lld_set_rd_addr(struct spi_nor *nor,
                                      struct spi_nor_cmd *cmd,
                                      flash_addr_t addr,  uint32_t reg, bool is_write)
{
    struct cospi_pdata *cospi = nor->host->priv_data;
    uint32_t addr_tmp = (addr >> 8u) | ((cmd->opcode << 16u) & 0xff000000);
    uint8_t mode = 0;

    if (cmd->addr_bytes != 0)
        mode = (addr & 0xff);
    else
        mode = (cmd->opcode >> 8u) & 0xff;

    RMWREG32(cospi->apb_base + COSPI_REG_RD_INSTR, COSPI_REG_RD_INSTR_MODE_EN_LSB,
             1u, 0);//disabe read cmd mode bit

    reg |= 0u << COSPI_REG_CMDCTRL_MODE_BIT_ENABLE_LSB ; //disabe stig mode bit

    if (nor->cmd_bytes > 1) {
        if (SNOR_INST_LANS(nor->reg_proto) == 3) {
            RMWREG32(cospi->apb_base + COSPI_REG_CONFIG,
                     COSPI_REG_CONFIG_DUAL_OPCODE_ENABLE, 1u, 1);//set 2bytes opcode
            RMWREG32(cospi->apb_base + COSPI_REG_OPCODE_EXTENSION,
                     COSPI_REG_SUPPLEMENT_ANY_STIG_OPCODE_LSB, 8u,
                     (cmd->opcode >> 8) & 0xff);//stig opcode
        }
        else {
            if (is_write) {
                if (cmd->addr_bytes != 0) {
                    reg |= 1u << COSPI_REG_CMDCTRL_MODE_BIT_ENABLE_LSB
                           ;//the stig needs to send the mode bit after the address byte
                    RMWREG32(cospi->apb_base + COSPI_REG_MODE_BIT, COSPI_REG_MODE_BIT_LSB, 8u,
                             mode);//the 8-bit mode data sent to the device followed by the address byte
                }
                else {
                    writel(mode, cospi->apb_base + COSPI_REG_COMMAND_WRITE_DATA);
                    reg |= (0u & 0x7) << COSPI_REG_CMDCTRL_WR_BYTES_LSB;//0: write 1byte data
                    reg |=  1u << COSPI_REG_CMDCTRL_WR_EN_LSB;//write data enable
                }
            }
            else {
                reg |= 1u << COSPI_REG_CMDCTRL_MODE_BIT_ENABLE_LSB
                       ;//the stig needs to send the mode bit after the address byte
                RMWREG32(cospi->apb_base + COSPI_REG_MODE_BIT, COSPI_REG_MODE_BIT_LSB, 8u,
                         mode);//the 8-bit mode data sent to the device followed by the address byte
            }
        }
    }
    else {
        RMWREG32(cospi->apb_base + COSPI_REG_CONFIG,
                 COSPI_REG_CONFIG_DUAL_OPCODE_ENABLE, 1u, 0);//1byte opcode mode
    }

    if (SNOR_ADDR_LANS(nor->reg_proto) == 3) {
        writel(addr, cospi->apb_base + COSPI_REG_CMDADDRESS);//send stig cmd address
    }
    else {
        writel(addr_tmp, cospi->apb_base + COSPI_REG_CMDADDRESS);
    }

    return reg;
}


int cospi_lld_command_read(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                           flash_addr_t addr, uint8_t *buf, flash_size_t len)
{
    struct cospi_pdata *cospi = nor->host->priv_data;
    addr_t reg_base = cospi->apb_base;
    uint32_t reg, ext_reg;
    uint32_t read_len;
    uint32_t mem_bank_index = 0;
    uint32_t mem_bank_len = 16;
    uint32_t cmd_len = (uint32_t)len;
    int status;

    if (0 != cospi_wait_idle(cospi)) {
        ssdk_printf(SSDK_ERR, "Wait ospi idle time out!\n");
        return -1;
    }

    if (cmd_len == 0u || cmd_len > COSPI_STIG_DATA_LEN_MAX || buf == NULL) {
        ssdk_printf(SSDK_ERR, "Invalid input argument!\n");
        return -1;
    }

    if (cmd->inst_type != cospi->inst_type) {
        cospi_inst_width_set(cospi, cmd->inst_type);
        cospi->inst_type = cmd->inst_type;
    }

    reg = (uint32_t)cmd->opcode << COSPI_REG_CMDCTRL_OPCODE_LSB;

    if (cmd->addr_bytes != 0u) {
        reg |= (0x1u << COSPI_REG_CMDCTRL_ADDR_EN_LSB);
        reg |= (((uint32_t)cmd->addr_bytes - 1u) &
                COSPI_REG_CMDCTRL_ADD_BYTES_MASK)
               << COSPI_REG_CMDCTRL_ADD_BYTES_LSB;

        ospi_writel((uint32_t)addr, cospi->apb_base + COSPI_REG_CMDADDRESS);
    }

    if (nor->mst_slv_mode) {
        reg = cospi_lld_set_rd_addr(nor, cmd, addr, reg, false);
    }

    reg |= (0x1u << COSPI_REG_CMDCTRL_RD_EN_LSB);

    /* set dummy */
    reg |= ((cmd->dummy + g_dummy) & COSPI_REG_CMDCTRL_WR_DUMMY_MASK)
           << COSPI_REG_CMDCTRL_WR_DUMMY_LSB;

    if (cmd_len <= 8u) {
        /* 0 means 1 byte. */
        reg |= (((cmd_len - 1u) & COSPI_REG_CMDCTRL_RD_BYTES_MASK)
                << COSPI_REG_CMDCTRL_RD_BYTES_LSB);
        status = cospi_exec_flash_cmd(cospi, reg);

        if (status != 0)
            return status;

        reg = ospi_readl(reg_base + COSPI_REG_CMDREADDATALOWER);

        /* Put the read value into rx_buf */
        read_len = (cmd_len > 4u) ? 4u : cmd_len;
        memcpy(buf, (uint8_t *)&reg, read_len);
        buf += read_len;

        if (cmd_len > 4u) {
            reg = ospi_readl(reg_base + COSPI_REG_CMDREADDATAUPPER);

            read_len = cmd_len - 4u;
            memcpy(buf, (uint8_t *)&reg, read_len);
        }
    }
    else {
        reg |= COSPI_REG_CMDCTRL_MEMBANK_EN_MASK;

        while (cmd_len > mem_bank_len) {
            mem_bank_index++;
            mem_bank_len *= ((uint32_t)1u << mem_bank_index);
        }

        ext_reg = (mem_bank_index << COSPI_REG_CTDCTRL_MEM_LEN_LSB);

        status = cospi_exec_flash_cmd_ext(cospi, reg, ext_reg, buf, cmd_len);
        reg = ospi_readl(reg_base + COSPI_REG_CMDREADDATALOWER);
        reg = ospi_readl(reg_base + COSPI_REG_CMDREADDATAUPPER);

        if (status != 0)
            return status;
    }

    return 0;
}


int cospi_lld_command_write(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                            flash_addr_t addr, const uint8_t *buf,
                            flash_size_t len)
{
    struct cospi_pdata *cospi = nor->host->priv_data;
    uint32_t cmd_len = (uint32_t)len;
    uint32_t reg;
    uint32_t data;
    int ret;

    if (0 != cospi_wait_idle(cospi)) {
        ssdk_printf(SSDK_ERR, "Wait ospi idle time out!\n");
        return -1;
    }

    if (cmd_len > 4u || (cmd_len != 0u && buf == NULL)) {
        ssdk_printf(SSDK_ERR, "Invalid input argument!\n");
        return -1;
    }

    if (cmd->inst_type != cospi->inst_type) {
        cospi_inst_width_set(cospi, cmd->inst_type);
        cospi->inst_type = cmd->inst_type;
    }

    reg = (uint32_t)cmd->opcode << COSPI_REG_CMDCTRL_OPCODE_LSB;

    if (cmd->addr_bytes != 0u) {
        reg |= (0x1u << COSPI_REG_CMDCTRL_ADDR_EN_LSB);
        reg |= (((uint32_t)cmd->addr_bytes - 1u) &
                COSPI_REG_CMDCTRL_ADD_BYTES_MASK)
               << COSPI_REG_CMDCTRL_ADD_BYTES_LSB;

        ospi_writel((uint32_t)addr, cospi->apb_base + COSPI_REG_CMDADDRESS);
    }

    if (nor->mst_slv_mode) {
        reg = cospi_lld_set_rd_addr(nor, cmd, addr, reg, true);
    }

    if (cmd_len != 0u) {
        reg |= (0x1u << COSPI_REG_CMDCTRL_WR_EN_LSB);
        reg |= ((cmd_len - 1u) & COSPI_REG_CMDCTRL_WR_BYTES_MASK)
               << COSPI_REG_CMDCTRL_WR_BYTES_LSB;

        data = 0;
        memcpy((uint8_t *)&data, buf, cmd_len);
        ospi_writel(data, cospi->apb_base + COSPI_REG_CMDWRITEDATALOWER);
    }

    reg |= ((uint32_t)cmd->dummy & COSPI_REG_CMDCTRL_WR_DUMMY_MASK)
           << COSPI_REG_CMDCTRL_WR_DUMMY_LSB;

    ret = cospi_exec_flash_cmd(cospi, reg);

    return ret;
}

int cospi_lld_stig_erase(struct spi_nor *nor, flash_addr_t offset)
{
    int ret;
    uint32_t inst_type;
    uint32_t addr_bytes;

    inst_type = SNOR_INST_LANS(nor->reg_proto);
    addr_bytes = nor->addr_width;

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = 0x6u,
        .addr_bytes = 0,
        /* the write enable cmd inst_width need aline with erase cmd */
        .inst_type = (uint8_t)inst_type,
    };

    ret = cospi_lld_command_write(nor, &write_enable_cmd, 0, NULL, 0);

    if (ret != 0)
        return ret;

    struct spi_nor_cmd erase_cmd = {
        .opcode = (uint8_t)(nor->info.erase_proto >> SNOR_OPCODE_PROTO_LSB),
        .addr_bytes = (uint8_t)addr_bytes,
        /* the write enable cmd inst_width need aline with erase cmd */
        .inst_type = (uint8_t)inst_type,
    };

    return cospi_lld_command_write(nor, &erase_cmd, offset, NULL, 0);
}

int cospi_lld_stig_complex_erase(struct spi_nor *nor,
                                 struct spi_nor_erase_cmd *cmd)
{
    int ret;
    flash_addr_t erase_addr = cmd->addr;

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = 0x6u,
        .addr_bytes = 0,
        /* the write enable cmd inst_width need aline with erase cmd */
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = cospi_lld_command_write(nor, &write_enable_cmd, 0, NULL, 0);

    if (ret != 0)
        return ret;

    struct spi_nor_cmd erase_cmd = {
        .opcode = (uint8_t)(cmd->map->erase_proto >> SNOR_OPCODE_PROTO_LSB),
        .addr_bytes = (uint8_t)nor->addr_width,
        /* the write enable cmd inst_width need aline with erase cmd */
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    return cospi_lld_command_write(nor, &erase_cmd, erase_addr, NULL, 0);
}

void cospi_lld_dma_enable(struct cospi_pdata *cospi, bool enable)
{
    uint32_t reg;

    cospi_wait_idle(cospi);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg &= ~COSPI_REG_CONFIG_DMA_MASK;
    reg |= (uint32_t)enable << COSPI_REG_CONFIG_DMA_LSB;
    cospi_wait_idle(cospi);
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

void cospi_lld_dma_config(struct cospi_pdata *cospi, enum spi_nor_ops ops,
                          uint8_t *buf, size_t len)
{
    uint32_t reg;
    uint32_t transfer_width;
    uint32_t remaining_size;
    uint32_t burst_size;
    uint32_t water_mark;
    uint32_t burst_size_index = 8;
    uint32_t single_size_index = 8;
    const uint32_t burst_size_start = 256;
    unsigned int max_burst;

// transfer_width = 1u;
    if (len % 4u != 0u) {
        transfer_width = 1u;
    }
    else {
        transfer_width = 4u;
    }

    burst_size = burst_size_start;

    /* Caculate the burst type request bytes number */
    while ((burst_size > len) ||
            (burst_size >
             (uint32_t)cospi->fifo_width * (uint32_t)cospi->fifo_depth)) {
        burst_size /= 2u;
        burst_size_index -= 1u;
    }

    max_burst = burst_size / transfer_width;

    if (max_burst >= 4u) {
        max_burst = my_log2_uint(max_burst) - 1;
        remaining_size = len % burst_size;
    }
    else {
        max_burst = 0;

        if (transfer_width == 4) {
            burst_size = 4;
            burst_size_index = 2;
        }
        else {
            burst_size = 1;
            burst_size_index = 0;
        }

        remaining_size = len;
    }

    if ((transfer_width >= 4u) && (remaining_size >= 4u)) {
        single_size_index = 2u;
    }
    else {
        single_size_index = 0u;
    }

    cospi->dma_bus_width = (uint8_t)transfer_width;
    cospi->dma_burst_size = burst_size / transfer_width;
    cospi->dma_max_burst = max_burst;

    /* Set burst bytes size and single bytes size */
    reg = ((single_size_index & COSPI_REG_DMA_SINGLE_MASK)
           << COSPI_REG_DMA_SINGLE_LSB);

    reg |= ((burst_size_index & COSPI_REG_DMA_BURST_MASK)
            << COSPI_REG_DMA_BURST_LSB);

    ospi_writel(reg, cospi->apb_base + COSPI_REG_DMA);

    if (ops == SPI_NOR_OPS_READ) {
        /* Indirect read watermark set with burst size */
        water_mark = burst_size;
        ospi_writel(water_mark,
                    cospi->apb_base + COSPI_REG_INDIRECTRDWATERMARK);
    }
    else {
        /*
         * Indirect write watermark set,
         * the num add with burst size equal to SRAM tx fifo size.
         */
        water_mark = (uint32_t)cospi->fifo_depth * cospi->fifo_width - burst_size;
        ospi_writel(water_mark,
                    cospi->apb_base + COSPI_REG_INDIRECTWRWATERMARK);
    }

    /* Enable DMA peripheral interface */
    cospi_lld_dma_enable(cospi, true);
}

int cospi_lld_rx_complete(struct cospi_pdata *cospi)
{
    int ret = 0;
    uint32_t reg;
    uint32_t sram_full = 0;

    reg = readl(cospi->apb_base + COSPI_REG_INDIRECTRD);

    if (reg == 0u)
        return 0;

    sram_full = reg & COSPI_REG_INDIRECTRD_SRAM_FULL_MASK;

    ret = cospi_wait_for_bit_times(cospi->apb_base + COSPI_REG_INDIRECTRD,
                                   COSPI_REG_INDIRECTRD_DONE_MASK, false, 100u);

    if (ret != 0) {
        ssdk_printf(SSDK_ERR, "Indirect read not complete, ret %d.\n", ret);
        /* Cancel the indirect read */
        writel(COSPI_REG_INDIRECTRD_CANCEL_MASK,
               cospi->apb_base + COSPI_REG_INDIRECTRD);
    }

    /* Clear indirect completion status */
    writel(COSPI_REG_INDIRECTRD_DONE_MASK | sram_full,
           cospi->apb_base + COSPI_REG_INDIRECTRD);

    return ret;
}

int cospi_lld_tx_complete(struct cospi_pdata *cospi)
{
    int ret = 0;
    uint32_t reg;

    reg = readl(cospi->apb_base + COSPI_REG_INDIRECTWR);

    if (reg == 0u)
        return 0;

    ret =
        cospi_wait_for_bit_times(cospi->apb_base + COSPI_REG_INDIRECTWR,
                                 COSPI_REG_INDIRECTWR_DONE_MASK, false, 10000u);

    if (ret != 0) {
        ssdk_printf(SSDK_ERR, "Indirect write not complete, ret %d.\n", ret);
        /* Cancel the indirect write */
        writel(COSPI_REG_INDIRECTWR_CANCEL_MASK,
               cospi->apb_base + COSPI_REG_INDIRECTWR);
    }

    /* Clear indirect completion status */
    writel(COSPI_REG_INDIRECTWR_DONE_MASK,
           cospi->apb_base + COSPI_REG_INDIRECTWR);

    return ret;
}

void cospi_lld_indirect_trigger(const struct cospi_pdata *cospi, uint32_t addr,
                                uint32_t size, int flag)
{
    if (flag == INDIRECT_READ_FLAG) {
        /* Set indirect read start address */
        ospi_writel(addr, cospi->apb_base + COSPI_REG_INDIRECTRDSTARTADDR);
        /* Set indirect read bytes number */
        ospi_writel(size, cospi->apb_base + COSPI_REG_INDIRECTRDBYTES);
        /* Start the indirect read */
        writel(COSPI_REG_INDIRECTRD_START_MASK,
               cospi->apb_base + COSPI_REG_INDIRECTRD);
        udelay(1u);
    }
    else {
        /* Set indirect write start address */
        ospi_writel(addr, cospi->apb_base + COSPI_REG_INDIRECTWRSTARTADDR);
        /* Set indirect write bytes number */
        ospi_writel(size, cospi->apb_base + COSPI_REG_INDIRECTWRBYTES);
        /* Start the indirect write */
        writel(COSPI_REG_INDIRECTWR_START_MASK,
               cospi->apb_base + COSPI_REG_INDIRECTWR);
        /* There need delay before write data to sram */
        udelay(1u);
    }
}

uint32_t cospi_lld_read_sram(struct cospi_pdata *cospi, uint8_t *buf,
                             flash_size_t size)
{
    uint32_t bytes_need_read = 0;

    unsigned long ahb_trigger_address =
        cospi->ahb_base + cospi->trigger_address;

    if (0u == readl(cospi->apb_base + COSPI_REG_INDIRECTRD))
        return bytes_need_read;

    bytes_need_read =
        cospi_get_rd_sram_level(cospi) * (uint32_t)cospi->fifo_width;
    /* read can't exceed the indirect trigger range */
    bytes_need_read = MIN(bytes_need_read, cospi->trigger_range_size);

    if (bytes_need_read) {
        arch_invalidate_cache_range(ahb_trigger_address, bytes_need_read);
        readsl((uint32_t *)ahb_trigger_address, (void *)buf,
               DIV_ROUND_UP(bytes_need_read, 4u));
        return bytes_need_read;
    }

    return 0;
}

uint32_t cospi_lld_write_sram(struct cospi_pdata *cospi, const uint8_t *buf,
                              flash_size_t size)
{
    uint32_t sram_level;
    uint32_t write_bytes = 0;
    uint32_t sram_size = (uint32_t)cospi->fifo_depth * cospi->fifo_width;
    unsigned long ahb_trigger_address =
        cospi->ahb_base + cospi->trigger_address;

    if (0u == readl(cospi->apb_base + COSPI_REG_INDIRECTWR))
        return write_bytes;

#if !defined(CONFIG_MST_SLV_OCTAL_LINE_DTR_MODE) && !defined(CONFIG_MST_SLV_OCTAL_LINE_SDR_MODE)
    write_bytes = MIN(size, sram_size);
#else
    /* write can't exceed the page size */
    write_bytes = MIN((uint32_t)size, cospi->page_size);
    write_bytes = MIN(write_bytes, sram_size);
#endif


    sram_level = cospi_get_wr_sram_level(cospi);

    if (0u == sram_level) {
        writesl((uint32_t *)ahb_trigger_address, (void *)buf,
                DIV_ROUND_UP(write_bytes, 4u));
        return write_bytes;
    }

    return 0;
}

int cospi_lld_cancel(struct spi_nor *nor)
{
    uint32_t reg;
    struct cospi_pdata *cospi = nor->host->priv_data;

    if (nor->xfer_info_bottom.opt_result != FLASH_OPT_PENDING)
        return 0;

    switch (nor->xfer_info_bottom.opt_type) {
        case FLASH_OPT_READ:
            ospi_writel(COSPI_REG_INDIRECTRD_CANCEL_MASK,
                        cospi->apb_base + COSPI_REG_INDIRECTRD);
            /* Clear indirect completion status */
            reg = ospi_readl(cospi->apb_base + COSPI_REG_INDIRECTRD);

            if ((reg & COSPI_REG_INDIRECTRD_DONE_MASK) != 0u)
                ospi_writel(COSPI_REG_INDIRECTRD_DONE_MASK,
                            cospi->apb_base + COSPI_REG_INDIRECTRD);

            break;

        case FLASH_OPT_WRITE:
            ospi_writel(COSPI_REG_INDIRECTWR_CANCEL_MASK,
                        cospi->apb_base + COSPI_REG_INDIRECTWR);

            /* Clear indirect completion status */
            reg = ospi_readl(cospi->apb_base + COSPI_REG_INDIRECTWR);

            if ((reg & COSPI_REG_INDIRECTWR_DONE_MASK) != 0u)
                ospi_writel(COSPI_REG_INDIRECTWR_DONE_MASK,
                            cospi->apb_base + COSPI_REG_INDIRECTWR);

            break;

        default:
            break;
    }

    return 0;
}

void cospi_lld_enter_xip(struct cospi_pdata *cospi)
{
    uint32_t reg;

    /* Disable direct access */
    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg &= ~COSPI_REG_CONFIG_ENB_DIR_ACC_CTRL;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);

    /* Enable xip mode */
    cospi_wait_idle(cospi);
    reg |= COSPI_REG_CONFIG_ENTER_XIP;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);

    /* Enable direct access */
    reg |= COSPI_REG_CONFIG_ENB_DIR_ACC_CTRL;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

static void cospi_phy_en(struct spi_nor *nor, bool phy_en)
{
    struct cospi_pdata *cospi = nor->host->priv_data;
    uint32_t reg;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);

    if (phy_en) {
        nor->phy_en = true;
        cospi->phy_mode = COSPI_PHY_MASTER_DLL;
        cospi->capture_delay = 0;

        /*if (nor->octal_dtr_en) {
            cospi->dqs_en = true;
            if (cospi->ref_clk_hz / 2u >= 130000000u)
                g_dummy = 1u;
            else
                g_dummy = 0u;
        } else */{
            cospi->dqs_en = nor->dqs_en;

            if (cospi->ref_clk_hz / 2u >= 130000000u)
                g_dummy = 1u;
            else
                g_dummy = 0u;
        }

        reg |= COSPI_REG_CONFIG_ENABLE_PHY_MASK;
    }
    else {
        nor->phy_en = false;
        cospi->phy_mode = COSPI_PHY_NONE;

        /* If disable phy, use ref_clk tap mode. */
        cospi->dqs_en = false;
        cospi->rclk_loopback = false;

        if (nor->octal_dtr_en)
            cospi->master_delay = 2u;
        else
            cospi->master_delay = 0u;

        cospi->capture_delay =
            cospi->master_delay + COSPI_READCAPTURE_DELAY_DEFAULT;

        g_dummy = 0u;
        reg &= ~COSPI_REG_CONFIG_ENABLE_PHY_MASK;
    }

    if (0 != cospi_wait_idle(cospi)) {
        ssdk_printf(SSDK_ERR, "Wait ospi idle time out!\n");
        return;
    }

    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
    udelay(1u);

    /* update read dummy */
    cospi_proto_setup(nor, SPI_NOR_OPS_READ);
}

static int cospi_phy_master_dll_init(struct cospi_pdata *cospi)
{
    uint32_t reg;
    int ret;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYCONFIG);

    reg &= ~COSPI_REG_PHYCONFIG_RST;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);
    reg = 0x04u << COSPI_REG_PHY_MCTL_INIT_DELAY_LSB;

    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHY_MCTL);
    udelay(1u);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYCONFIG);
    reg |= COSPI_REG_PHYCONFIG_RST;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYDLLOBS);
    ssdk_printf(SSDK_INFO, "COSPI_REG_PHYDLLOBS = 0x%x \n", reg);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYDLLOBS);
    ssdk_printf(SSDK_INFO, "COSPI_REG_PHYDLLOBS = 0x%x \n", reg);

    ret = cospi_wait_for_bit_times(cospi->apb_base + COSPI_REG_PHYDLLOBS,
                                   COSPI_REG_PHYDLLOBS_LOOPBACK_LOCK, false,
                                   1000u);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYDLLOBS);
    ssdk_printf(SSDK_INFO, "COSPI_REG_PHYDLLOBS = 0x%x \n", reg);

    reg = (COSPI_REG_PHYCONFIG_RST & ~COSPI_REG_PHYCONFIG_RESYNC) |
          (0x20u << COSPI_REG_PHYCONFIG_TX_DELAY_LSB) |
          (0x20u << COSPI_REG_PHYCONFIG_RX_DELAY_LSB);

    /* bypass rx dll */
    reg |= COSPI_REG_PHYCONFIG_RX_BYPASS;
    udelay(1u);
    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);
    udelay(10u);

    reg |= COSPI_REG_PHYCONFIG_RESYNC;
    /* disable rx dll */
    reg &= ~COSPI_REG_PHYCONFIG_RX_BYPASS;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

    udelay(10u);
    return ret;
}

static int cospi_phy_config(struct cospi_pdata *cospi)
{
    int ret;
    uint32_t reg;

    switch (cospi->phy_mode) {
        case COSPI_PHY_NONE:
            ret = 0;
            break;

        case COSPI_PHY_BYPASS_DLL:
            reg = 0x04u << COSPI_REG_PHY_MCTL_INIT_DELAY_LSB |
                  COSPI_REG_PHY_MCTL_BYPASS;
            ospi_writel(reg, cospi->apb_base + COSPI_REG_PHY_MCTL);
            ret = 0;
            break;

        case COSPI_PHY_MASTER_DLL:
            ret = cospi_phy_master_dll_init(cospi);
            break;

        default:
            ret = -1;
            break;
    }

    if (ret != 0)
        return ret;

    return 0;
}

static int cospi_do_training(struct spi_nor *nor, flash_addr_t addr,
                             uint8_t *buf, const uint8_t *pattern,
                             flash_size_t size)
{
    int ret = 0;
    uint32_t tx_delay, rx_delay;
    uint32_t rx_delay_start = 0;
    bool train_ok = false;
    struct cospi_pdata *cospi = nor->host->priv_data;
    uint32_t reg, ref_clock_per;

    if (cospi->phy_mode == COSPI_PHY_NONE) {
        return 0;
    }

    if (cospi->phy_mode == COSPI_PHY_BYPASS_DLL) {
        /* Calculate each clock cycle time in picoseconds */
        ref_clock_per = 1000000u / (cospi->ref_clk_hz / 1000000u);

        if (nor->octal_dtr_en) {
            tx_delay = ref_clock_per / 4u / 164u;
        }
        else {
            tx_delay = ref_clock_per / 2u / 164u;
        }
    }
    else {
        if (nor->octal_dtr_en) {
            tx_delay = 0x1f;
        }
        else {
            tx_delay = 0x3f;
        }
    }

    for (rx_delay = 0u; rx_delay <= 0x7fu; rx_delay++) {
        reg = (COSPI_REG_PHYCONFIG_RST & ~COSPI_REG_PHYCONFIG_RESYNC) |
              (tx_delay << COSPI_REG_PHYCONFIG_TX_DELAY_LSB) |
              (rx_delay << COSPI_REG_PHYCONFIG_RX_DELAY_LSB);

        udelay(1u);
        ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);
        udelay(1u);

        reg |= COSPI_REG_PHYCONFIG_RESYNC;
        ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

        udelay(1u);

        spi_nor_read(nor, addr, buf, size);

        if (memcmp(buf, pattern, (size_t)size) == 0) {
            if (train_ok == false) {
                train_ok = true;
                rx_delay_start = rx_delay;
            }

            ssdk_printf(SSDK_INFO,
                        "ospi phy training succeful, rx_delay = %d \n",
                        rx_delay);
        }
        else {
            if (train_ok && (rx_delay > (rx_delay_start + 10u)))
                break;
            else
                train_ok = false;
        }
    }

    if (train_ok) {
        rx_delay = (rx_delay_start + rx_delay) / 2u;
        cospi->phy_training_succ = true;

        reg = (COSPI_REG_PHYCONFIG_RST & ~COSPI_REG_PHYCONFIG_RESYNC) |
              (tx_delay << COSPI_REG_PHYCONFIG_TX_DELAY_LSB) |
              (rx_delay << COSPI_REG_PHYCONFIG_RX_DELAY_LSB);
        ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

        udelay(1u);
        reg |= COSPI_REG_PHYCONFIG_RESYNC;
        ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

        ssdk_printf(SSDK_ERR, "ospi phy training pass!\n");
        ret = 0;
    }
    else {
        cospi_phy_en(nor, false);
        ssdk_printf(SSDK_ERR, "ospi phy training failed, exit phy mode!\n");
        ret = -1;
    }

    cospi_wait_idle(cospi);
    return ret;
}

int cospi_lld_phy_training(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                           const uint8_t *pattern, flash_size_t size)
{
    struct cospi_pdata *cospi = nor->host->priv_data;
    cospi_wait_idle(cospi);

    if (nor->info.read_proto & SNOR_DTR_PROTO) { //|| nor->ddr_training) {
        ospi_writel((uint32_t)0x4u, cospi->apb_base + 0x110u);//tx ignored
    }
    else {
        ospi_writel((uint32_t)0xcu, cospi->apb_base + 0x110u);
    }

    udelay(1u);

    /* glitch workaround: Enable loopback and disable dqs, bypass rx dll. */
    ospi_writel((uint32_t)0u, cospi->apb_base + COSPI_REG_RDCAP);
    ospi_writel(COSPI_REG_PHYCONFIG_RX_BYPASS,
                cospi->apb_base + COSPI_REG_PHYCONFIG);

    cospi_phy_en(nor, true);

    cospi_phy_config(cospi);

    /* glitch workaround: recover capture config. */
    cospi_capture_set(nor);

    if (nor->mst_slv_mode) {
        return 0;//mst-slv mode don't need to training
    }
    else {
        return cospi_do_training(nor, addr, buf, pattern, size);
    }
}

void cospi_lld_nor_setup(struct spi_nor *nor, enum spi_nor_ops ops)
{
    uint32_t reg = 0u;
    struct cospi_pdata *cospi = nor->host->priv_data;

    if (cospi->page_size != nor->info.page_size && !nor->mst_slv_mode) {
        cospi_size_set(nor);
    }

    /*
     * If countroller cs not select this chip,
     * config baudrate delay and capture set.
     */
    if (cospi->current_cs != nor->cs) {
        cospi_cs_set(nor);
        cospi->sclk = nor->baudrate;
        cospi_baudrate_set(cospi);
        cospi->current_cs = nor->cs;
    }

    cospi_capture_set(nor);
    cospi_proto_setup(nor, ops);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);

    if (nor->octal_dtr_en)
        reg |= COSPI_REG_CONFIG_DTR_ENABLE_MASK;
    else
        reg &= ~COSPI_REG_CONFIG_DTR_ENABLE_MASK;

    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);

    if (nor->hyperbus_mode) {
        // cospi_proto_setup(nor, SPI_NOR_OPS_READ);
        // cospi_proto_setup(nor, SPI_NOR_OPS_WRITE);
        cospi_hyperbus_enanle(nor, true);
    }

    return;
}

void cospi_lld_setup_xfer(struct spi_nor *nor, enum flash_opt opt,
                          flash_addr_t addr, uint8_t *buf, flash_size_t size)

{
    nor->xfer_info_bottom.opt_type = opt;
    nor->xfer_info_bottom.addr = addr;
    nor->xfer_info_bottom.buf = buf;
    nor->xfer_info_bottom.opt_result = FLASH_OPT_PENDING;
    nor->xfer_info_bottom.size = size;
}

uint32_t cospi_lld_irq_handler(struct cospi_pdata *cospi)
{
    uint32_t status;

    status = ospi_readl(cospi->apb_base + COSPI_REG_IRQSTATUS);
    writel(status, cospi->apb_base + COSPI_REG_IRQSTATUS);

#if 0
    // TODO: atf irq event
    status = ospi_readl(cospi->apb_base + 0xa04);

    status = readl(cospi->apb_base + 0xa38);
    writel(0, cospi->apb_base + 0xa38);

    status = readl(cospi->apb_base + 0xa44);
    writel(0, cospi->apb_base + 0xa44);
#endif

    return status;
}

void cospi_lld_reset_flash(struct cospi_pdata *cospi)
{
    uint32_t reg;

    /* de-assert the reset pin */
    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg |= COSPI_REG_CONFIG_RESET_SELECT_MASK;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
    reg |= COSPI_REG_CONFIG_RESET_PIN_MASK;
    udelay(1000u);
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
    reg &= ~COSPI_REG_CONFIG_RESET_PIN_MASK;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
    udelay(1000u);
}

/* File-local proto/opcode helpers (mirror spi_nor.c; not exported via spi_nor.h). */
#define PROTO(_opcode, _dq) ((uint32_t)(_opcode) << SNOR_OPCODE_PROTO_LSB | (_dq))
#define SPINOR_OP_READ_FAST 0x0b
#define SPINOR_OP_PP 0x02

/* XIP-safe init: fill software fields only, never touch controller CONFIG/
 * REMAP/IRQMASK/RESET registers (firmware is executing via this controller's
 * AHB direct window; disabling it even briefly kills instruction fetch).
 * Fields mirror cospi_lld_init so cospi_read/cospi_write indirect branches
 * compute correctly. Register writes for SRAMPARTITION/INDIRECTTRIGGER/
 * watermarks are skipped: Boot ROM already programmed them for XIP. */
void cospi_lld_soft_init(struct cospi_pdata *cospi)
{
    cospi->current_cs = 0xff;
    cospi->capture_delay = COSPI_READCAPTURE_DELAY_DEFAULT;
    cospi->sram_size_nbit = 8u;
    cospi->fifo_depth = 1u << (cospi->sram_size_nbit - 1u);
    cospi->fifo_width = 4u;
    cospi->trigger_range_size = 1u << (7u + 2u);
    cospi->trigger_address = 0x4000000u - cospi->trigger_range_size;
}

/* xip_proto_setup: called by spibus_init when host->xip_mode is set.
 * Must mirror the controller read protocol already programmed by Boot ROM
 * (XIP direct reads only work with the most conservative common denomitor:
 * 1-1-1 fast read). We assert that via RD_INSTR inst_type and fall back to
 * defaults from default_spi_nor_ids (1-1-1 / READ_FAST 0x0B / dummy 8). */
void cospi_xip_proto_setup(struct spi_nor *nor)
{
    struct cospi_pdata *cospi = nor->host->priv_data;
    uint32_t instr = ospi_readl(cospi->apb_base + COSPI_REG_RD_INSTR);
    uint32_t it = (instr >> COSPI_REG_RD_INSTR_TYPE_INST_LSB)
                  & COSPI_REG_RD_INSTR_TYPE_INST_MASK;

    if (it != 0u)
        ssdk_printf(SSDK_CRIT, "xip: RD_INSTR inst_type=%u (expect 0)\n", it);

    nor->reg_proto = SNOR_PROTO_1_1_1;
    nor->info.write_proto = PROTO(SPINOR_OP_PP, SNOR_PROTO_1_1_1);
    nor->info.read_proto = PROTO(SPINOR_OP_READ_FAST, SNOR_PROTO_1_1_1);
    nor->info.read_dummy = 8;
    nor->info.sector_size = SPINOR_SECTOR_4K_SIZE;
    nor->info.page_size = 256;
}

void *cospi_lld_init(struct cospi_pdata *cospi)
{
    uint32_t reg;

    g_dummy = 0;

    /* initialize the current cs for no one */
    cospi->current_cs = 0xff;
    /* the default capture delay is 1 */
    cospi->capture_delay = COSPI_READCAPTURE_DELAY_DEFAULT;

    cospi->sram_size_nbit = 8u;
    cospi->fifo_depth = 1u << (cospi->sram_size_nbit - 1u);
    cospi->fifo_width = 4u;

    if (0 != cospi_wait_idle(cospi)) {
        ssdk_printf(SSDK_ERR, "Wait ospi idle time out!\n");
        return NULL;
    }

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);

    /* Clear ospi misc register */
    ospi_writel((uint32_t)0u, cospi->apb_base + COSPI_PRE_MISC);

    /* Disable the controller */
    ospi_writel((uint32_t)0u, cospi->apb_base + COSPI_REG_CONFIG);

    /* Disable all interrupts */
    ospi_writel((uint32_t)0u, cospi->apb_base + COSPI_REG_IRQMASK);

    /* Not remap the address */
    ospi_writel((uint32_t)0u, cospi->apb_base + COSPI_REG_REMAP);

    /* Disable host write protection */
#ifndef CDNS_WRITE_PROTECTION_SUPPORT
    ospi_writel((uint32_t)0u, cospi->apb_base + COSPI_REG_WRITE_PROTECTION);
#endif

    /* Set indirect trigger address range width, equal fifo bytes*/
    ospi_writel((uint32_t)7u,
                cospi->apb_base + COSPI_REG_INDIRECTTRIGGER_RANGE);
    cospi->trigger_range_size = 1u << (7u + 2u);

    /* Load indirect trigger address, this address equal to AHB paddr */
    cospi->trigger_address = 0x4000000u - cospi->trigger_range_size;
    ospi_writel(cospi->trigger_address,
                cospi->apb_base + COSPI_REG_INDIRECTTRIGGER);

    /* Configure the read fifo depth equal sram / 2. */
    ospi_writel((uint32_t)cospi->fifo_depth,
                cospi->apb_base + COSPI_REG_SRAMPARTITION);

    /* Indirect read watermark set with read fifo_depth / 2 */
    ospi_writel((uint32_t)cospi->fifo_depth * cospi->fifo_width / 2u,
                cospi->apb_base + COSPI_REG_INDIRECTRDWATERMARK);

    /* Indirect write watermark set with write fifo_depth / 4 */
    ospi_writel((uint32_t)cospi->fifo_depth * cospi->fifo_width / 4u,
                cospi->apb_base + COSPI_REG_INDIRECTWRWATERMARK);

    cospi_delay_set(cospi);

    /* Enable Direct Access mode*/
    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg |= COSPI_REG_CONFIG_ENB_DIR_ACC_CTRL;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);

    ospi_writel(COSPI_IRQ_STATUS_MASK, cospi->apb_base + COSPI_REG_IRQSTATUS);
    ospi_writel(COSPI_IRQ_MASK_RD | COSPI_IRQ_MASK_WR,
                cospi->apb_base + COSPI_REG_IRQMASK);

#if 0
    // TODO: atf irq event
    writel(0, cospi->apb_base + 0xa08);
    writel(0, cospi->apb_base + 0xa10);
    writel(0, cospi->apb_base + 0xa34);
    writel(0, cospi->apb_base + 0xa40);
#endif

#ifdef CONFIG_CDNS_OSPI_MST_SLV
    //wel disable
    RMWREG32(cospi->apb_base + COSPI_REG_WR_INSTR, COSPI_REG_WR_DIS_WEL, 1, 1);
    //disable polling completion
    RMWREG32(cospi->apb_base + COSPI_WRITE_COMPLETION_CONTROL,
             COSPI_WRITE_DISABLE_POLLING, 1, 1);
    //polling opcode
    RMWREG32(cospi->apb_base + COSPI_WRITE_COMPLETION_CONTROL,
             COSPI_WRITE_POLLING_OPCODE, 8, 0xdd);
#endif

    /* Enable the controller */
    cospi_enable(cospi, true);
    return (void *)cospi;
}

void cospi_cache_flush(struct spi_nor *nor, flash_addr_t addr,
                       flash_size_t length)
{
    // struct cospi_pdata *cospi = nor->host->priv_data;
    if (nor->host->xip_mode) {
        return;  // xip mode can not flush cache
    }

    // arch_invalidate_cache_range((addr_t)cospi->ahb_base + addr, ROUNDUP(length, 32));
    // cospi_prefetch_flush(cospi, 0);
    // cospi_prefetch_flush(cospi, 1);
}


/**
 * @brief cospi hyper command read
 * @param[in] nor spi_nor ptr
 * @param[in] addr flash register addr
 * @param[out] buf  2byte data
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
int cospi_hyper_read16(struct spi_nor *nor, flash_addr_t addr,
                       uint16_t *buf)
{
    int ret = 0;

    struct spi_nor_cmd read_cmd = {
        .opcode = 0,
        .dummy = nor->info.read_dummy + g_dummy,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = cospi_lld_command_read(nor, &read_cmd, addr, (uint8_t *)buf, 2u);

    if (ret)
        return -1;

    return 0;
}

/**
 * @brief cospi hyper command write
 * @param[in] nor spi_nor ptr
 * @param[in] addr flash register addr
 * @param[in] buf  2byte data
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
int cospi_hyper_write16(struct spi_nor *nor, flash_addr_t addr,
                        uint16_t *buf)
{
    int ret = 0;

    struct spi_nor_cmd write_cmd = {
        .opcode = 0,
        .dummy = 0,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    if (nor->xfer_info.opt_type == FLASH_OPT_ERASE) {
        addr *= 2u;
    }

    ret = cospi_lld_command_write(nor, &write_cmd, addr, (uint8_t *)buf, 1u);

    if (ret)
        return -1;

    return 0;
}

/**
 * @brief cospi hyper setup before transaction
 * @param[in] nor spi_nor ptr
 * @param[in] is_read read or write mode
 * @param[in] is_direct direct or indirect mode
 * @param[in] num hyperbus_ca trans number
 * @param[in] ca_array  hyperbus_ca ptr
 */
void cospi_hyper_set_pre_transaction(struct spi_nor *nor,
                                     bool is_read,
                                     bool is_direct, uint32_t num, struct hyperbus_ca **ca_array)
{
    uint32_t addr;
    uint16_t data;
    addr_t ptc_addr = 0;
    addr_t pta_base_addr;
    addr_t ptd_base_addr;
    struct hyperbus_ca *ca_ptr = *ca_array;
    struct cospi_pdata *cospi = nor->host->priv_data;

    cospi_pre_tran_config_t config_value = {
        .config.number = num,
    };

    if (is_direct) {
        if (is_read) {
            ptc_addr = cospi->apb_base + COSPI_REG_HYPERFLASH_READ_CONFIG;
            pta_base_addr = cospi->apb_base + COSPI_REG_HYPERFLASH_READ_TRAN1_ADDRESS;
            ptd_base_addr = cospi->apb_base + COSPI_REG_HYPERFLASH_READ_TRAN1_DATA;
        }
        else {
            ptc_addr = cospi->apb_base + COSPI_REG_HYPERFLASH_WRITE_CONFIG;
            pta_base_addr = cospi->apb_base + COSPI_REG_HYPERFLASH_WRITE_TRAN1_ADDRESS;
            ptd_base_addr = cospi->apb_base + COSPI_REG_HYPERFLASH_WRITE_TRAN1_DATA;
        }
    }
    else {
        ptc_addr = cospi->apb_base + COSPI_REG_HYPERFLASH_STIG_CONFIG;
        pta_base_addr = cospi->apb_base + COSPI_REG_HYPERFLASH_STIG_TRAN1_ADDRESS;
        ptd_base_addr = cospi->apb_base + COSPI_REG_HYPERFLASH_STIG_TRAN1_DATA;
    }

    for (uint32_t i = 0u; i < num; i++) {
        addr = ca_ptr->addr;
        data = ca_ptr->data;
        ospi_writel(addr, pta_base_addr);
        ospi_writel(data, ptd_base_addr);

        pta_base_addr += COSPI_REG_HYPERFLASH_PRE_TRAN_DEPTH;
        ptd_base_addr += COSPI_REG_HYPERFLASH_PRE_TRAN_DEPTH;
        ca_ptr += 1;
    }

    /* set pre-tansaction num */
    ospi_writel(config_value.data, ptc_addr);
}

void cospi_hyperram_enable(struct spi_nor *nor, uint8_t cs,
                           uint8_t read_dummy, uint8_t write_dummy)
{
    // uint32_t dummy;
    // struct cospi_pdata *cospi = nor->host->priv_data;

    // RMWREG32(cospi->apb_base + cospi_DEV_SIZE, 16u + cs, 1u, 0x1u);

    // /* Config direct read pre-transaction to 0 */
    // ospi_writel(0, cospi->apb_base + cospi_DIRECT_RD_PTC);

    // /* Config direct write pre-transaction to 0 */
    // ospi_writel(0, cospi->apb_base + cospi_DIRECT_WR_PTC);

    // dummy = ((read_dummy + 1) & cospi_DIRECT_D_CYC_RD_MASK) <<
    //         cospi_DIRECT_D_CYC_RD_LSB;
    // dummy |= ((write_dummy + 1) & cospi_DIRECT_D_CYC_WR_MASK) <<
    //          cospi_DIRECT_D_CYC_WR_LSB;
    // ospi_writel(dummy, cospi->apb_base + cospi_DIRECT_D_CYC);

    // cospi_prefetch_disable(cospi);
}

void cospi_hyperbus_enanle(struct spi_nor *nor, bool enable)
{
    struct cospi_pdata *cospi = nor->host->priv_data;

    cospi_hyperflash_config_t hyper_config;
    hyper_config.data = ospi_readl(cospi->apb_base + COSPI_REG_HYPERFLASH_CONFIG);
    hyper_config.config.enable = 1;
    hyper_config.config.burst_type = 1;
    ospi_writel(hyper_config.data, cospi->apb_base + COSPI_REG_HYPERFLASH_CONFIG);


    // cospi_main_config_t main_config;
    // main_config.data = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    // main_config.config.dtr_en = 0;
    // main_config.config.swap_en = 1;
    // ospi_writel(main_config.data, cospi->apb_base + COSPI_REG_CONFIG);

    ospi_writel(0u, cospi->apb_base + COSPI_REG_HYPERFLASH_STIG_CONFIG);
    ospi_writel(0u, cospi->apb_base + COSPI_REG_HYPERFLASH_WRITE_CONFIG);
    ospi_writel(0u, cospi->apb_base + COSPI_REG_HYPERFLASH_READ_CONFIG);


    // ospi_writel(0x01100081, cospi->apb_base + 0x00);
    // ospi_writel(0x0f0333ee, cospi->apb_base + 0x04);
    // ospi_writel(0x00033002, cospi->apb_base + 0x08);
    // ospi_writel(0x02012705, cospi->apb_base + 0x38);
    // ospi_writel(0x00110000, cospi->apb_base + 0xb0);
    // ospi_writel(0x00000007, cospi->apb_base + 0x10);
    // ospi_writel(0x00101003, cospi->apb_base + 0x14);
    // ospi_writel(0x0c0c0c0c, cospi->apb_base + 0x0c);
}
