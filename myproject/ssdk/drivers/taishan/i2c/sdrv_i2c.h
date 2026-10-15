/**
 * @file sdrv_i2c.h
 * @low level sdrv i2c head file.
 *
 * @Copyright (c) 2020  Semidrive Semiconductor.
 * @All rights reserved.
 *
 **/

#ifndef SDRV_I2C_H_
#define SDRV_I2C_H_

/* adapter opmode */
#define SDRV_I2C_SLAVE  0x0
#define SDRV_SM_SLAVE   0x1
#define SDRV_PM_SLAVE   0x2
#define SDRV_I2C_MASTER 0x8

/* adapter speedmode */
#define SDRV_I2C_NSPEED 0x0
#define SDRV_I2C_HSPEED 0x3
#define SDRV_I2C_USPEED 0x4

/* fifo watermark level */
#define SDRV_I2C_RX_WML 0x1f
#define SDRV_I2C_TX_WML 0x1f

/* register address */
#define I2C_MCR0    0x00
#define I2C_MCR1    0x04
#define I2C_MCR2    0x08
#define I2C_MCR3    0x0C
#define I2C_MCR4    0x10

#define I2C_INTR0   0x20
#define I2C_INTR1   0x24
#define I2C_INTR2   0x28
#define I2C_INTR3   0x2C
#define I2C_INTEN0  0x30
#define I2C_INTEN1  0x34
#define I2C_INTEN2  0x38
#define I2C_INTEN3  0x3C

#define I2C_CMDCSR0 0x40
#define I2C_CMDCSR1 0x44
#define I2C_CMDCSR2 0x48
#define I2C_CMDCSR3 0x4C

#define I2C_FCR     0x50
#define I2C_FSR     0x60
#define I2C_DMACR   0x70
#define I2C_DMASR   0x74

#define I2C_PCR0    0x80
#define I2C_PCR1    0x84
#define I2C_PCR2    0x88
#define I2C_PCR3    0x8C
#define I2C_PCR4    0x90
#define I2C_PCR5    0x94
#define I2C_PCR6    0x98
#define I2C_PCR7    0x9C
#define I2C_PCR8    0xA0
#define I2C_PCR9    0xA4
#define I2C_PCR10   0xA8
#define I2C_PCR11   0xAC
#define I2C_PCR12   0xB0
#define I2C_PCR13   0xB4
#define I2C_PCR14   0xB8
#define I2C_PCR15   0xBC
#define I2C_PSR0    0x100

#define I2C_TXFIFO  0x200
#define I2C_RXFIFO  0x300

#define I2C_RED_PARITY_ERR_INT_STAT    0xE0
#define I2C_RED_PARITY_ERR_INT_STAT_EN 0xE4
#define I2C_RED_PARITY_ERR_INT_SIG_EN  0xE8

/* int0 bitmask */
#define SDRV_I2C_INT0_TXFWE             (1 << 0)
#define SDRV_I2C_INT0_RXFWF             (1 << 1)
#define SDRV_I2C_INT0_TXFUDF            (1 << 2)
#define SDRV_I2C_INT0_RXFUDF            (1 << 3)
#define SDRV_I2C_INT0_TXFOVF            (1 << 4)
#define SDRV_I2C_INT0_RXFOVF            (1 << 5)
#define SDRV_I2C_INT0_TXFABT            (1 << 6)
#define SDRV_I2C_INT0_RXFABT            (1 << 7)
#define SDRV_I2C_INT0_SCLSTUCKLOW       (1 << 8)
#define SDRV_I2C_INT0_SDASTUCKLOW       (1 << 9)
#define SDRV_I2C_INT0_BUSCLRPASS        (1 << 10)
#define SDRV_I2C_INT0_BUSCLRERR         (1 << 11)
#define SDRV_I2C_INT0_APBCMDDONE        (1 << 12)
#define SDRV_I2C_INT0_APBCMDABORT       (1 << 13)
#define SDRV_I2C_INT0_SLVWRTRANSDONE    (1 << 14)
#define SDRV_I2C_INT0_SLVRDTRANSDONE    (1 << 15)
#define SDRV_I2C_INT0_SLVWRTRANSABORT   (1 << 16)
#define SDRV_I2C_INT0_SLVRDTRANSABORT   (1 << 17)
#define SDRV_I2C_INT0_SCLHLDTX          (1 << 18)
#define SDRV_I2C_INT0_SCLHLDRX          (1 << 19)
#define SDRV_I2C_INT0_BUSCLRDET         (1 << 20)

#define SDRV_I2C_INT0_DEFAULT_MASK \
	(SDRV_I2C_INT0_TXFWE  |\
	 SDRV_I2C_INT0_RXFWF  |\
	 SDRV_I2C_INT0_TXFUDF |\
	 SDRV_I2C_INT0_RXFUDF |\
	 SDRV_I2C_INT0_TXFOVF |\
	 SDRV_I2C_INT0_RXFOVF |\
	 SDRV_I2C_INT0_TXFABT |\
	 SDRV_I2C_INT0_RXFABT |\
	 SDRV_I2C_INT0_SCLSTUCKLOW |\
	 SDRV_I2C_INT0_SDASTUCKLOW |\
	 SDRV_I2C_INT0_APBCMDDONE  |\
	 SDRV_I2C_INT0_APBCMDABORT)

#define SDRV_I2C_INT0_SLV_DEFAULT_MASK \
	(SDRV_I2C_INT0_TXFUDF |\
	 SDRV_I2C_INT0_RXFUDF |\
	 SDRV_I2C_INT0_TXFOVF |\
	 SDRV_I2C_INT0_RXFOVF |\
	 SDRV_I2C_INT0_TXFABT |\
	 SDRV_I2C_INT0_RXFABT |\
	 SDRV_I2C_INT0_SCLSTUCKLOW |\
	 SDRV_I2C_INT0_SDASTUCKLOW |\
	 SDRV_I2C_INT0_APBCMDABORT |\
	 SDRV_I2C_INT0_SLVWRTRANSDONE |\
	 SDRV_I2C_INT0_SLVRDTRANSDONE |\
	 SDRV_I2C_INT0_SLVWRTRANSABORT|\
	 SDRV_I2C_INT0_SLVRDTRANSABORT)

#define SDRV_I2C_INT0_ERR_STAT \
	(SDRV_I2C_INT0_TXFUDF |\
	 SDRV_I2C_INT0_RXFUDF |\
	 SDRV_I2C_INT0_TXFOVF |\
	 SDRV_I2C_INT0_RXFOVF |\
	 SDRV_I2C_INT0_TXFABT |\
	 SDRV_I2C_INT0_RXFABT |\
	 SDRV_I2C_INT0_SCLSTUCKLOW |\
	 SDRV_I2C_INT0_SDASTUCKLOW |\
	 SDRV_I2C_INT0_APBCMDABORT |\
     SDRV_I2C_INT0_SLVWRTRANSABORT |\
     SDRV_I2C_INT0_SLVRDTRANSABORT)

/* int2 bitmask */
#define SDRV_I2C_INT2_STARTDET         (1 << 0)
#define SDRV_I2C_INT2_RESTARTDET       (1 << 1)
#define SDRV_I2C_INT2_STOPDET          (1 << 2)
#define SDRV_I2C_INT2_SLVRDREQDET      (1 << 18)
#define SDRV_I2C_INT2_SLVWRREQDET      (1 << 19)
#define SDRV_I2C_INT2_CTLBYTEDET       (1 << 28)

#define SDRV_I2C_INT2_SLV_MASK \
	(SDRV_I2C_INT2_SLVRDREQDET  |\
	 SDRV_I2C_INT2_SLVWRREQDET  |\
	 SDRV_I2C_INT2_CTLBYTEDET)

/* int3 bitmask */
#define SDRV_I2C_INT3_DMAEOBAERR     (1 << 0)
#define SDRV_I2C_INT3_DMAEOBCERR     (1 << 1)
#define SDRV_I2C_INT3_DMABWUNCERR    (1 << 2)
#define SDRV_I2C_INT3_DMABWCORERR    (1 << 3)
#define SDRV_I2C_INT3_DMABWFATALERR  (1 << 4)
#define SDRV_I2C_INT3_PCTL0UNCERR    (1 << 16)
#define SDRV_I2C_INT3_PCTL1UNCERR    (1 << 17)
#define SDRV_I2C_INT3_PUSERUNCERR    (1 << 18)
#define SDRV_I2C_INT3_PADDRUNCERR    (1 << 19)
#define SDRV_I2C_INT3_PWDATACORERR   (1 << 20)
#define SDRV_I2C_INT3_PWDATAUNCERR   (1 << 21)
#define SDRV_I2C_INT3_PWDATAFATALERR (1 << 22)
#define SDRV_I2C_INT3_SELFTESTMODERR (1 << 25)
#define SDRV_I2C_INT3_REGPAREJENERR  (1 << 26)

#define SDRV_I2C_INT3_MASK \
	(SDRV_I2C_INT3_DMAEOBAERR    |\
	 SDRV_I2C_INT3_DMAEOBCERR    |\
	 SDRV_I2C_INT3_DMABWUNCERR   |\
	 SDRV_I2C_INT3_DMABWFATALERR |\
	 SDRV_I2C_INT3_PCTL0UNCERR   |\
	 SDRV_I2C_INT3_PCTL1UNCERR   |\
	 SDRV_I2C_INT3_PUSERUNCERR   |\
	 SDRV_I2C_INT3_PADDRUNCERR   |\
	 SDRV_I2C_INT3_PWDATAUNCERR  |\
	 SDRV_I2C_INT3_PWDATAFATALERR)

#define SDRV_I2C_CMD0_SLV_MASK (0x0)
#define SDRV_I2C_CMD1_SLV_MASK (0x0)
#define SDRV_I2C_CMD2_SLV_MASK (0x1000)
#define SDRV_I2C_CMD3_SLV_MASK (0x0)

uint8_t sdrv_i2c_lld_recv_data(paddr_t base);
void sdrv_i2c_lld_send_data(paddr_t base, uint8_t data);
uint32_t sdrv_i2c_lld_get_psr0_stat(paddr_t base);
uint32_t sdrv_i2c_lld_get_wspace(paddr_t base);
uint32_t sdrv_i2c_lld_get_rspace(paddr_t base);
void sdrv_i2c_lld_set_watermark(paddr_t base,
                                uint32_t tx_lvl, uint32_t rx_lvl);
void sdrv_i2c_lld_clear_fifo(paddr_t base);
void sdrv_i2c_lld_set_cmdcsr0(paddr_t base, uint32_t cmdcsr0);
void sdrv_i2c_lld_set_cmdcsr1(paddr_t base, uint32_t cmdcsr1);
void sdrv_i2c_lld_set_cmdcsr2(paddr_t base, uint32_t cmdcsr2);
void sdrv_i2c_lld_set_cmdcsr3(paddr_t base, uint32_t cmdcsr3);
uint32_t sdrv_i2c_lld_get_cmdcsr0(paddr_t base);
uint32_t sdrv_i2c_lld_get_cmdcsr1(paddr_t base);
uint32_t sdrv_i2c_lld_get_cmdcsr2(paddr_t base);
uint32_t sdrv_i2c_lld_get_cmdcsr3(paddr_t base);
void sdrv_i2c_lld_clear_int(paddr_t base);
uint32_t sdrv_i2c_lld_get_int0_stat(paddr_t base);
uint32_t sdrv_i2c_lld_get_int1_stat(paddr_t base);
uint32_t sdrv_i2c_lld_get_int2_stat(paddr_t base);
uint32_t sdrv_i2c_lld_get_int3_stat(paddr_t base);
void sdrv_i2c_lld_unmask_int0(paddr_t base, uint32_t int_flag);
void sdrv_i2c_lld_unmask_int1(paddr_t base, uint32_t int_flag);
void sdrv_i2c_lld_unmask_int2(paddr_t base, uint32_t int_flag);
void sdrv_i2c_lld_unmask_int3(paddr_t base, uint32_t int_flag);
void sdrv_i2c_lld_set_speed(paddr_t base, uint32_t speed_mode);
void sdrv_i2c_lld_set_slvaddr(paddr_t base, uint8_t addr);
void sdrv_i2c_lld_set_timing(paddr_t base,
                             uint32_t pcr2, uint32_t pcr3, uint32_t pcr4, uint32_t pcr5);
void sdrv_i2c_lld_set_IO0(paddr_t base,  uint32_t val);
void sdrv_i2c_lld_set_IO1(paddr_t base,  uint32_t val);
void sdrv_i2c_lld_set_IO2(paddr_t base,  uint32_t val);
void sdrv_i2c_lld_set_IO3(paddr_t base,  uint32_t val);
void sdrv_i2c_lld_set_opmode(paddr_t base, uint32_t opmode);
void sdrv_i2c_lld_disable(paddr_t base);
void sdrv_i2c_lld_enable(paddr_t base);
int sdrv_i2c_lld_reset(paddr_t base, bool init);
void sdrv_i2c_lld_dump_reg(paddr_t base, uint32_t *reg_val);
void sdrv_i2c_lld_write_reg(paddr_t base, uint32_t reg, uint32_t val);
void sdrv_i2c_lld_read_reg(paddr_t base, uint32_t reg, uint32_t *val);

#endif
