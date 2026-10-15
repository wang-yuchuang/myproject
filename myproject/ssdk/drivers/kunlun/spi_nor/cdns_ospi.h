/**
 * @file cdns_ospi.h
 * @brief Cadence ospi host low level driver header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef CDNS_OSPI_H_
#define CDNS_OSPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <debug.h>
#include <driver/spi_nor.h>
#include <reg.h>
#include <types.h>

#include "driver/kunlun/spi_nor/cdns_ospi_drv.h"

//#define WITH_OSPI_DEBUG
#ifdef WITH_OSPI_DEBUG
#define ospi_readl(reg) \
        readl(reg);         \
        ssdk_printf(SSDK_EMERG, "r(0x%x, r(0x%08x)\n", reg, readl(reg));
#define ospi_writel(val, reg)                                          \
        writel(val, reg);                                                  \
        ssdk_printf(SSDK_EMERG, "w(0x%x, 0x%08x), r(0x%08x)\n", reg, val, \
                    readl(reg));
#else
#define ospi_writel(val, reg) writel(val, reg)
#define ospi_readl(reg)       readl(reg)
#endif

#undef BIT
#define BIT(nr) ((uint32_t)1u << (nr))

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#endif

#ifndef MIN
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef udelay
#define udelay(x)                                      \
        {                                                  \
            volatile uint64_t count = 20ull * (uint64_t)x; \
            while (count != 0ull)                          \
                count--;                                   \
        }
#endif

#define OSPI_NS_2_TICKS(hz, ns) (((hz) / 1000U * (ns) + 1000000u) / 1000000u)

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d)-1u) / (d))
#endif

/* Operation timeout value */
#define COSPI_IDLE_TIMEOUT_CYCLES (10000u)

/* Instruction type */
#define COSPI_INST_TYPE_SINGLE 0
#define COSPI_INST_TYPE_DUAL   1
#define COSPI_INST_TYPE_QUAD   2
#define COSPI_INST_TYPE_OCTAL  3

#define COSPI_DUMMY_CLKS_PER_BYTE 8
#define COSPI_DUMMY_BYTES_MAX     4
#define COSPI_DUMMY_CLKS_MAX      31

#define COSPI_STIG_DATA_LEN_MAX (512u)

/* Register map */
#define COSPI_REG_CONFIG                   (0x0u)
#define COSPI_REG_CONFIG_ENABLE_MASK       BIT(0u)
#define COSPI_REG_CONFIG_ENABLE_PHY_MASK   BIT(3u)
#define COSPI_REG_CONFIG_RESET_PIN_MASK    BIT(5)
#define COSPI_REG_CONFIG_RESET_SELECT_MASK BIT(6)
#define COSPI_REG_CONFIG_ENB_DIR_ACC_CTRL  BIT(7)
#define COSPI_REG_CONFIG_DECODE_MASK       BIT(9)
#define COSPI_REG_CONFIG_CHIPSELECT_LSB    (10u)
#define COSPI_REG_CONFIG_DMA_MASK          BIT(15)
#define COSPI_REG_CONFIG_DMA_LSB           (15u)
#define COSPI_REG_CONFIG_ENTER_XIP         BIT(17)
#define COSPI_REG_CONFIG_BAUD_LSB          (19u)
#define COSPI_REG_CONFIG_DTR_ENABLE_MASK   BIT(24)
#define COSPI_REG_CONFIG_DTR_ENABLE_LSB    (24u)
#define COSPI_REG_CONFIG_DUAL_OPCODE_ENABLE    (30u)
#define COSPI_REG_CONFIG_IDLE_MASK         BIT(31)
#define COSPI_REG_CONFIG_CHIPSELECT_MASK   (0xFu)
#define COSPI_REG_CONFIG_BAUD_MASK         (0xFu)
typedef union {
    uint32_t data;
    struct {
        uint8_t octal_spi_en: 1;
        uint8_t clock_pol: 1;
        uint8_t clock_pha: 1;
        uint8_t phy_mode_en: 1;
        uint8_t hold_pin: 1;
        uint8_t reset_pin: 1;
        uint8_t reset_cfg: 1;
        uint8_t dir_acc_en: 1;

        uint8_t legacy_mode_en: 1;
        uint8_t peripheral_decode: 1;
        uint8_t peripheral_cs_lines: 4;
        uint8_t wr_prot_pin: 1;
        uint8_t dma_en: 1;

        uint8_t ahb_addr_remap_en: 1;
        uint8_t xip_mode_en: 1;
        uint8_t xip_mode_imm_en: 1;
        uint8_t baud_rate_div: 4;
        uint8_t ahb_decoder_en: 1;

        uint8_t dtr_en: 1;
        uint8_t pipe_phy_mode_en: 1;
        uint8_t res1: 2;
        uint8_t swap_en: 1;
        uint8_t crc_en: 1;
        uint8_t dual_byte_en: 1;
        uint8_t idle_statues: 1;
    } config;
} cospi_main_config_t;

#define COSPI_REG_RD_INSTR                (0x04u)
#define COSPI_REG_RD_INSTR_TYPE_INST_LSB  (8u)
#define COSPI_REG_RD_INSTR_TYPE_INST_MASK (0x3u)
#define COSPI_REG_RD_INSTR_DDR_EN         (10u)
#define COSPI_REG_RD_INSTR_MODE_EN_LSB    (20u)

#define COSPI_REG_WR_INSTR        (0x08u)
#define COSPI_REG_WR_DIS_WEL 8u
#define COSPI_REG_WR_DIS_WEL_MASK BIT(8)

/* same in read inst reg and write inst reg */
#define COSPI_REG_RDWR_INSTR_OPCODE_LSB (0u)
#define COSPI_REG_RDWR_INSTR_ADDR_LSB   (12u)
#define COSPI_REG_RDWR_INSTR_ADDR_MASK  (0x3u)
#define COSPI_REG_RDWR_INSTR_DATA_LSB   (16u)
#define COSPI_REG_RDWR_INSTR_DATA_MASK  (0x3u)
#define COSPI_REG_RDWR_INSTR_DUMMY_LSB  (24u)
#define COSPI_REG_RDWR_INSTR_DUMMY_MASK (0x1Fu)

#define COSPI_REG_DELAY             (0x0Cu)
#define COSPI_REG_DELAY_CSSOT_LSB   (0u)
#define COSPI_REG_DELAY_CSEOT_LSB   (8u)
#define COSPI_REG_DELAY_CSDADS_LSB  (16u)
#define COSPI_REG_DELAY_CSDA_LSB    (24u)
#define COSPI_REG_DELAY_CSSOT_MASK  (0xFFu)
#define COSPI_REG_DELAY_CSEOT_MASK  (0xFFu)
#define COSPI_REG_DELAY_CSDADS_MASK (0xFFu)
#define COSPI_REG_DELAY_CSDA_MASK   (0xFFu)

#define COSPI_REG_RDCAP                   (0x10u)
#define COSPI_REG_RDCAP_BYPASS_LSB        (0u)
#define COSPI_REG_RDCAP_SAMPLE_EDGE_SEL   (5u)
#define COSPI_REG_RDCAP_DQSEN_LSB         (8u)
#define COSPI_REG_RDCAP_DELAY_LSB         (1u)
#define COSPI_REG_RDCAP_DELAY_MASK        (0xFu)
#define COSPI_REG_RDCAP_MASTER_DELAY_LSB  (16u)
#define COSPI_REG_RDCAP_MASTER_DELAY_MASK (0xFu)
#define COSPI_READCAPTURE_DELAY_DEFAULT   (1u)

#define COSPI_REG_SIZE              (0x14u)
#define COSPI_REG_SIZE_ADDRESS_LSB  (0u)
#define COSPI_REG_SIZE_PAGE_LSB     (4u)
#define COSPI_REG_SIZE_BLOCK_LSB    (16u)
#define COSPI_REG_SIZE_ADDRESS_MASK (0xFu)
#define COSPI_REG_SIZE_PAGE_MASK    (0xFFFu)
#define COSPI_REG_SIZE_BLOCK_MASK   (0x3Fu)
#define COSPI_REG_SIZE_SIZE_LSB     (0x21u)

#define COSPI_REG_SRAMPARTITION   (0x18u)
#define COSPI_REG_INDIRECTTRIGGER (0x1Cu)

#define COSPI_REG_DMA             (0x20u)
#define COSPI_REG_DMA_SINGLE_LSB  (0u)
#define COSPI_REG_DMA_BURST_LSB   (8u)
#define COSPI_REG_DMA_SINGLE_MASK (0xFFu)
#define COSPI_REG_DMA_BURST_MASK  (0xFFu)

#define COSPI_REG_REMAP    (0x24u)
#define COSPI_REG_MODE_BIT (0x28u)
#define COSPI_REG_MODE_BIT_LSB (0u)

#define COSPI_REG_SDRAMLEVEL         (0x2Cu)
#define COSPI_REG_SDRAMLEVEL_RD_LSB  (0u)
#define COSPI_REG_SDRAMLEVEL_WR_LSB  (16u)
#define COSPI_REG_SDRAMLEVEL_RD_MASK (0xFFFFu)
#define COSPI_REG_SDRAMLEVEL_WR_MASK (0xFFFFu)


#define COSPI_WRITE_COMPLETION_CONTROL (0x38u)
#define COSPI_WRITE_DISABLE_POLLING (14u)
#define COSPI_WRITE_POLLING_OPCODE (0u)

#define COSPI_REG_IRQSTATUS                    (0x40u)
#define COSPI_REG_IRQSTATUS_INDIRECT_DONE_MASK BIT(2)
#define COSPI_REG_IRQMASK                      (0x44u)

#define COSPI_REG_WRITE_PROTECTION (0x58u)

#define COSPI_REG_INDIRECTRD                (0x60u)
#define COSPI_REG_INDIRECTRD_START_MASK     BIT(0)
#define COSPI_REG_INDIRECTRD_CANCEL_MASK    BIT(1)
#define COSPI_REG_INDIRECTRD_SRAM_FULL_MASK BIT(3)
#define COSPI_REG_INDIRECTRD_DONE_MASK      BIT(5)
#define COSPI_REG_INDIRECTRD_Q_DONE_MASK    BIT(7)

#define COSPI_REG_INDIRECTRDWATERMARK (0x64u)
#define COSPI_REG_INDIRECTRDSTARTADDR (0x68u)
#define COSPI_REG_INDIRECTRDBYTES     (0x6Cu)

#define COSPI_REG_CMDCTRL_MEM                 (0x8Cu)
#define COSPI_REG_CMDCTRL_MEM_EXECUTE_MASK    BIT(0)
#define COSPI_REG_CMDCTRL_MEM_INPROGRESS_MASK BIT(1)
#define COSPI_REG_CTDCTRL_MEM_LEN_LSB         (16u)
#define COSPI_REG_CTDCTRL_MEM_LEN_MASK        (0x7u)
#define COSPI_REG_CTDCTRL_MEM_ADDR_LSB        (20u)
#define COSPI_REG_CTDCTRL_MEM_ADDR_MASK       (0x1Fu)

#define COSPI_REG_CTDCTRL_MEM_DATA (0x8Du)

#define COSPI_REG_CMDCTRL                 (0x90u)
#define COSPI_REG_CMDCTRL_EXECUTE_MASK    BIT(0)
#define COSPI_REG_CMDCTRL_INPROGRESS_MASK BIT(1)
#define COSPI_REG_CMDCTRL_MEMBANK_EN_MASK BIT(2)
#define COSPI_REG_CMDCTRL_WR_DUMMY_LSB    (7u)
#define COSPI_REG_CMDCTRL_WR_DUMMY_MASK   (0x1Fu)
#define COSPI_REG_CMDCTRL_WR_BYTES_LSB    (12u)
#define COSPI_REG_CMDCTRL_WR_EN_LSB       (15u)
#define COSPI_REG_CMDCTRL_ADD_BYTES_LSB   (16u)
#define COSPI_REG_CMDCTRL_MODE_BIT_ENABLE_LSB (18u)
#define COSPI_REG_CMDCTRL_ADDR_EN_LSB     (19u)
#define COSPI_REG_CMDCTRL_RD_BYTES_LSB    (20u)
#define COSPI_REG_CMDCTRL_RD_EN_LSB       (23u)
#define COSPI_REG_CMDCTRL_OPCODE_LSB      (24u)
#define COSPI_REG_CMDCTRL_WR_BYTES_MASK   (0x7u)
#define COSPI_REG_CMDCTRL_ADD_BYTES_MASK  (0x3u)
#define COSPI_REG_CMDCTRL_RD_BYTES_MASK   (0x7u)

#define COSPI_REG_INDIRECTWR             (0x70u)
#define COSPI_REG_INDIRECTWR_START_MASK  BIT(0)
#define COSPI_REG_INDIRECTWR_CANCEL_MASK BIT(1)
#define COSPI_REG_INDIRECTWR_IN_MASK     BIT(2)
#define COSPI_REG_INDIRECTWR_DONE_MASK   BIT(5)

#define COSPI_REG_INDIRECTTRIGGER_RANGE (0x80u)

#define COSPI_REG_INDIRECTWRWATERMARK (0x74u)
#define COSPI_REG_INDIRECTWRSTARTADDR (0x78u)
#define COSPI_REG_INDIRECTWRBYTES     (0x7Cu)

#define COSPI_REG_CMDADDRESS        (0x94u)
#define COSPI_REG_CMDREADDATALOWER  (0xA0u)
#define COSPI_REG_CMDREADDATAUPPER  (0xA4u)
#define COSPI_REG_CMDWRITEDATALOWER (0xA8u)
#define COSPI_REG_CMDWRITEDATAUPPER (0xACu)

/* Interrupt status bits */
#define COSPI_REG_IRQ_MODE_ERR         BIT(0)
#define COSPI_REG_IRQ_UNDERFLOW        BIT(1)
#define COSPI_REG_IRQ_IND_COMP         BIT(2)
#define COSPI_REG_IRQ_IND_RD_REJECT    BIT(3)
#define COSPI_REG_IRQ_WR_PROTECTED_ERR BIT(4)
#define COSPI_REG_IRQ_ILLEGAL_AHB_ERR  BIT(5)
#define COSPI_REG_IRQ_WATERMARK        BIT(6)
#define COSPI_REG_IRQ_IND_SRAM_FULL    BIT(12)
#define COSPI_REG_IRQ_ECC_ERR          BIT(19)


#define COSPI_REG_COMMAND_WRITE_DATA (0xA8u)


/* auto polling status bits */
#define COSPI_REG_FLASH_STATUS             (0xB0u)
#define COSPI_REG_FLASH_STATUS_STATUS_LSB  (0u)
#define COSPI_REG_FLASH_STATUS_STATUS_MASK (0xFFu)
#define COSPI_REG_FLASH_STATUS_STATUS_LSB  (0u)
#define COSPI_REG_FLASH_STATUS_VALID       BIT(8)
#define COSPI_REG_FLASH_STATUS_DUMMY_LSB   (16u)
#define COSPI_REG_FLASH_STATUS_DUMMY_MASK  (0x1Fu)

/* Internal phy register */
#define COSPI_REG_PHYCONFIG               (0xB4u)
#define COSPI_REG_PHYCONFIG_RX_DELAY_LSB  (0u)
#define COSPI_REG_PHYCONFIG_TX_DELAY_LSB  (16u)
#define COSPI_REG_PHYCONFIG_RX_BYPASS     BIT(29)
#define COSPI_REG_PHYCONFIG_RX_DELAY_MASK (0x7Fu)
#define COSPI_REG_PHYCONFIG_TX_DELAY_MASK (0x7Fu)
#define COSPI_REG_PHYCONFIG_RST           BIT(30)
#define COSPI_REG_PHYCONFIG_RESYNC        BIT(31)

#define COSPI_REG_PHY_MCTL                 (0xB8u)
#define COSPI_REG_PHY_MCTL_INIT_DELAY_LSB  (0u)
#define COSPI_REG_PHY_MCTL_INIT_DELAY_MASK (0x7Fu)
#define COSPI_REG_PHY_MCTL_BYPASS          BIT(23)

#define COSPI_REG_PHYDLLOBS               (0xBCu)
#define COSPI_REG_PHYDLLOBS_LOCK          BIT(0)
#define COSPI_REG_PHYDLLOBS_LOOPBACK_LOCK BIT(15)

#define COSPI_REG_OPCODE_EXTENSION         (0xE0u)
#define COSPI_REG_SUPPLEMENT_ANY_READ_OPCODE_LSB  (24u)
#define COSPI_REG_SUPPLEMENT_ANY_WRITE_OPCODE_LSB  (16u)
#define COSPI_REG_SUPPLEMENT_ANY_POLLING_OPCODE_LSB  (8u)
#define COSPI_REG_SUPPLEMENT_ANY_STIG_OPCODE_LSB  (0u)

#define COSPI_REG_HYPERFLASH_CONFIG             (0x100u)
#define COSPI_REG_HYPERFLASH_ENABLE             BIT(0)
#define COSPI_REG_HYPERFLASH_LAT_CODE_LSB       (16u)
#define COSPI_REG_HYPERFLASH_LAT_CODE_MASK      (0xFu)
#define COSPI_REG_BURST_TYPE_HYPERFLASH_STIG    BIT(29)
#define COSPI_REG_TARGET_HYPERFLASH_STIG        BIT(30)
typedef union {
    uint32_t data;
    struct {
        uint8_t enable: 1;
        uint16_t res1: 15;
        uint8_t latency_code: 4;
        uint8_t res2: 4;
        uint8_t res3: 4;
        uint8_t res4: 1;
#define COSPI_WRAPPED_BURST                          (0u)
#define COSPI_LINEAR_BURST                           (1u)
        uint8_t burst_type: 1;
#define COSPI_TARGET_MEMORY_SPACE                    (0u)
#define COSPI_TARGET_REGISTER_SPACE                  (1u)
        uint8_t target_space: 1;
        uint8_t res5: 1;
    } config;
} cospi_hyperflash_config_t;

#define COSPI_REG_HYPERFLASH_STIG_CONFIG         (0x200u)
#define COSPI_REG_HYPERFLASH_WRITE_CONFIG        (0x300u)
#define COSPI_REG_HYPERFLASH_READ_CONFIG         (0x400u)
#define COSPI_REG_HYPERFLASH_POLLING_CONFIG      (0x500u)
#define COSPI_PRE_TRAN_MAX_NUMBER                    (7u)
#define COSPI_REG_HYPERFLASH_STIG_TRAN1_ADDRESS     (0x210u)
#define COSPI_REG_HYPERFLASH_STIG_TRAN1_DATA        (0x214u)
#define COSPI_REG_HYPERFLASH_WRITE_TRAN1_ADDRESS    (0x310u)
#define COSPI_REG_HYPERFLASH_WRITE_TRAN1_DATA       (0x314u)
#define COSPI_REG_HYPERFLASH_READ_TRAN1_ADDRESS     (0x410u)
#define COSPI_REG_HYPERFLASH_READ_TRAN1_DATA        (0x414u)
#define COSPI_REG_HYPERFLASH_PRE_TRAN_DEPTH         (0x10)
typedef union {
    uint32_t data;
    struct {
#define COSPI_PRE_TRAN_DATA_2B                       (0u)
#define COSPI_PRE_TRAN_DATA_4B                       (1u)
        uint8_t tran1_data_size: 1; // 0:2B 1:4B
        uint8_t res1: 1;
        uint8_t tran2_data_size: 1;
        uint8_t res2: 1;
        uint8_t tran3_data_size: 1;
        uint8_t res3: 1;
        uint8_t tran4_data_size: 1;
        uint8_t res4: 1;
        uint8_t tran5_data_size: 1;
        uint8_t res5: 1;
        uint8_t tran6_data_size: 1;
        uint8_t res6: 1;
        uint8_t tran7_data_size: 1;
        uint8_t res7: 3;
        uint8_t res8;
        uint8_t number: 3;
        uint8_t res9: 5;
    } config;
} cospi_pre_tran_config_t;

#define COSPI_PRE_MISC                              (0x110u)

#define COSPI_IRQ_MASK_RD                                    \
    (COSPI_REG_IRQ_WATERMARK | COSPI_REG_IRQ_IND_SRAM_FULL | \
     COSPI_REG_IRQ_IND_COMP)

#define COSPI_IRQ_MASK_WR \
    (COSPI_REG_IRQ_IND_COMP | COSPI_REG_IRQ_WATERMARK | COSPI_REG_IRQ_UNDERFLOW)

#define COSPI_IRQ_STATUS_MASK (0x1FFFFu | COSPI_REG_IRQ_ECC_ERR)

#define INDIRECT_READ_FLAG  (0)
#define INDIRECT_WRITE_FLAG (1)

void *cospi_lld_init(struct cospi_pdata *cospi);
void cospi_lld_soft_init(struct cospi_pdata *cospi);
void cospi_xip_proto_setup(struct spi_nor *nor);

int cospi_lld_command_read(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                           flash_addr_t addr, uint8_t *buf, flash_size_t len);

int cospi_lld_command_write(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                            flash_addr_t addr, const uint8_t *buf,
                            flash_size_t len);

int cospi_lld_stig_erase(struct spi_nor *nor, flash_addr_t offset);

int cospi_lld_stig_complex_erase(struct spi_nor *nor,
                                 struct spi_nor_erase_cmd *cmd);

void cospi_lld_dma_enable(struct cospi_pdata *cospi, bool enable);

void cospi_lld_indirect_trigger(const struct cospi_pdata *cospi, uint32_t addr,
                                uint32_t size, int flag);

int cospi_lld_cancel(struct spi_nor *nor);

int cospi_lld_phy_training(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                           const uint8_t *pattern, flash_size_t size);

uint32_t cospi_lld_irq_handler(struct cospi_pdata *cospi);

void cospi_lld_nor_setup(struct spi_nor *nor, enum spi_nor_ops ops);

void cospi_lld_setup_xfer(struct spi_nor *nor, enum flash_opt opt,
                          flash_addr_t addr, uint8_t *buf, flash_size_t size);

void cospi_lld_dma_config(struct cospi_pdata *cospi, enum spi_nor_ops ops,
                          uint8_t *buf, size_t len);

void cospi_lld_reset_flash(struct cospi_pdata *cospi);

void cospi_lld_enter_xip(struct cospi_pdata *cospi);

uint32_t cospi_lld_read_sram(struct cospi_pdata *cospi, uint8_t *buf,
                             flash_size_t size);

uint32_t cospi_lld_write_sram(struct cospi_pdata *cospi, const uint8_t *buf,
                              flash_size_t size);

int cospi_lld_rx_complete(struct cospi_pdata *cospi);

int cospi_lld_tx_complete(struct cospi_pdata *cospi);

void cospi_cache_flush(struct spi_nor *nor, flash_addr_t addr,
                       flash_size_t length);

int cospi_hyper_read16(struct spi_nor *nor, flash_addr_t addr, uint16_t *buf);

int cospi_hyper_write16(struct spi_nor *nor, flash_addr_t addr, uint16_t *buf);

void cospi_hyper_set_pre_transaction(struct spi_nor *nor,
                                     bool is_read, bool is_direct, uint32_t num, struct hyperbus_ca **ca_array);

void cospi_hyperram_enable(struct spi_nor *nor, uint8_t cs,
                           uint8_t read_dummy, uint8_t write_dummy);
void cospi_hyperbus_enanle(struct spi_nor *nor, bool en);

#ifdef __cplusplus
}
#endif

#endif /* CDNS_OSPI_H_ */
