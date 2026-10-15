/**
 * @file sdrv_uart.h
 * @brief sdrv UART driver head.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef SDRV_UART_H
#define SDRV_UART_H

#include <types.h>

#define SDRV_UART_PCR_CHARLEN_8_BIT         0x0
#define SDRV_UART_PCR_CHARLEN_7_BIT         0x1
#define SDRV_UART_PCR_CHARLEN_6_BIT         0x2
#define SDRV_UART_PCR_CHARLEN_5_BIT         0x3
#define SDRV_UART_PCR_CHARLEN_4_BIT         0x4
#define SDRV_UART_PCR_CHARLEN_9_BIT         0x5

#define SDRV_UART_PCR_STOPBIT_1_BIT         0x0
#define SDRV_UART_PCR_STOPBIT_0_5_BIT       0x1
#define SDRV_UART_PCR_STOPBIT_2_BIT         0x2
#define SDRV_UART_PCR_STOPBIT_1_5_BIT       0x3

#define SDRV_UART_PCR_PARITY_NONE           0x0
#define SDRV_UART_PCR_PARITY_EVEN           0x1
#define SDRV_UART_PCR_PARITY_ODD            0x2
#define SDRV_UART_PCR_PARITY_ALL_1          0x3
#define SDRV_UART_PCR_PARITY_ALL_0          0x4

#define SDRV_UART_CLASSIC_CHECKSUM          0x0
#define SDRV_UART_ENHANCED_CHECKSUM         0x1

#define SDRV_UART_TXBREAK_BIT(bit)          (bit - 8) + 0x1
#define SDRV_UART_RXBREAK_BIT(bit)          (bit - 9) + 0x1

#define SDRV_UART_INTR_TX_FWE               1
#define SDRV_UART_INTR_RX_FWF               (1 << 1)
#define SDRV_UART_INTR_RX_FOVF              (1 << 5)
#define SDRV_UART_INTR_RXBREAK              (1 << 13)
#define SDRV_UART_INTR_TX_CMP               (1 << 17)
#define SDRV_UART_INTR_ABRPASS              (1 << 18)
#define SDRV_UART_INTR_ABRFAIL              (1 << 19)
#define SDRV_UART_INTR_APBCMDDONE           (1 << 21)
#define SDRV_UART_INTR_APBCMDABORT          (1 << 22)
#define SDRV_UART_INTR_PIDPASS              (1 << 23)
#define SDRV_UART_INTR_PIDERR               (1 << 24)
#define SDRV_UART_INTR_RXCHKSUMPASS         (1 << 25)
#define SDRV_UART_INTR_RXCHKSUMERR          (1 << 26)

typedef union {
    struct {
        uint32_t MODEN:             1;
        uint32_t MODRST:            1;
        uint32_t CGC:               2;
        uint32_t OPMOD:             4;
        uint32_t QCHCTL:            1;
        uint32_t SELFTESTMODEEN:    1;
        uint32_t RESERVED:          22;
    };
    uint32_t v;
} MCR0_Type;

typedef union {
    struct {
        uint32_t PRESALE:           5;
        uint32_t TXEN:              1;
        uint32_t RXEN:              1;
        uint32_t BITORDER:          1;
        uint32_t DATABIT:           3;
        uint32_t STOPBIT:           2;
        uint32_t PARITYBIT:         3;
        uint32_t ADDRBIT:           1;
        uint32_t ABREN:             1;
        uint32_t ABRCTL0:           4;
        uint32_t ABRCTL1:           3;
        uint32_t FCM:               3;
        uint32_t TRANSFERMODE:      1;
        uint32_t RXMUTECTL:         3;
    };
    uint32_t v;
} PCR0_Type;

typedef union {
    struct {
        uint32_t BAUDRATE:          24;
        uint32_t SAMPLERATE:        2;
        uint32_t RXBREAKCTL:        3;
        uint32_t RXIDLECTL:         3;
    };
    uint32_t v;
} PCR1_Type;

typedef union {
    struct {
        uint32_t CHAR1:             8;
        uint32_t CHAR2:             8;
        uint32_t ADDRCHAR:          8;
        uint32_t ADDRMSK:           8;
    };
    uint32_t v;
} PCR2_Type;

typedef union {
    struct {
        uint32_t DE2RE:             8;
        uint32_t RE2DE:             8;
        uint32_t DEAT:              8;
        uint32_t DEDT:              8;
    };
    uint32_t v;
} PCR3_Type;

typedef union {
    struct {
        uint32_t TXADDR:            1;
        uint32_t TXBREAK:           1;
        uint32_t TXIDLE:            1;
        uint32_t RESERVED1:         5;
        uint32_t TXBREAKCTL:        4;
        uint32_t TXIDLECTL:         4;
        uint32_t CHARMATCHCTL:      3;
        uint32_t BCLKOE:            1;
        uint32_t RXTOCTL:           3;
        uint32_t RESERVED2:         9;
    };
    uint32_t v;
} PCR4_Type;

typedef union {
    struct {
        uint32_t TXCHARINT:         8;
        uint32_t RESERVED1:         8;
        uint32_t SIRTXPUL:          8;
        uint32_t SIRRXPUL:          8;
    };
    uint32_t v;
} PCR5_Type;

typedef union {
    struct {
        uint32_t LINBREAKDEL:       3;
        uint32_t CHKSUMTYP:         1;
        uint32_t RESERVED:          28;
    };
    uint32_t v;
} PCR6_Type;

typedef union {
    struct {
        uint32_t TXSEL:             4;
        uint32_t TXMODE:            3;
        uint32_t TXPOL:             1;
        uint32_t RXSEL:             4;
        uint32_t RXMODE:            2;
        uint32_t RXEDGESEL:         2;
        uint32_t RXPOL:             1;
        uint32_t RXSYNCEN:          1;
        uint32_t RXFILTCTL:         3;
        uint32_t RESERVED:          11;
    };
    uint32_t v;
} PCR8_Type;

typedef union {
    struct {
        uint32_t TXSEL:             4;
        uint32_t TXMODE:            3;
        uint32_t TXPOL:             1;
        uint32_t RXSEL:             4;
        uint32_t RXMODE:            2;
        uint32_t RXEDGESEL:         2;
        uint32_t RXPOL:             1;
        uint32_t RXSYNCEN:          1;
        uint32_t RXFILTCTL:         3;
        uint32_t RESERVED:          11;
    };
    uint32_t v;
} PCR11_Type;

typedef union {
    struct {
        uint32_t WMLVL:             8;
        uint32_t RESERVED1:         8;
        uint32_t CLR:               1;
        uint32_t RESERVED2:         15;
    };
    uint32_t v;
} FCR_Type;

typedef union {
    struct {
        uint32_t FILLLVL:           8;
        uint32_t EMPTYLVL:          8;
        uint32_t FIFOSZ:            8;
        uint32_t EMPTY:             1;
        uint32_t FULL:              1;
        uint32_t RESERVED:          6;
    };
    uint32_t v;
} FSR_Type;

typedef union {
    struct {
        uint32_t TXFWE:             1;
        uint32_t RXFWF:             1;
        uint32_t TXFUDF:            1;
        uint32_t RXFUDF:            1;
        uint32_t TXFOVF:            1;
        uint32_t RXFOVF:            1;
        uint32_t TXFABT:            1;
        uint32_t RXFABT:            1;
        uint32_t PARITYERR:         1;
        uint32_t FRAMEERR:          1;
        uint32_t BAUDRATEERR:       1;
        uint32_t NOISEERR:          1;
        uint32_t RXTO:              1;
        uint32_t RXBREAK:           1;
        uint32_t RXIDLE:            1;
        uint32_t RXADDR:            1;
        uint32_t CHARMATCH:         1;
        uint32_t TC:                1;
        uint32_t ABRPASS:           1;
        uint32_t ABRFAIL:           1;
        uint32_t STARTERR:          1;
        uint32_t APBCMDDONE:        1;
        uint32_t APBCMDABORT:       1;
        uint32_t RXPIDPASS:         1;
        uint32_t RXPIDERR:          1;
        uint32_t RXCHKSUMPASS:      1;
        uint32_t RXCHKSUMERR:       1;
        uint32_t RESERVED:          1;
        uint32_t IO0EDGE:           1;
        uint32_t IO1EDGE:           1;
        uint32_t IO2EDGE:           1;
        uint32_t IO3EDGE:           1;
    };
    uint32_t v;
} INTR0_Type;

typedef union {
    struct {
        uint32_t FC_STATE:          4;
        uint32_t LIN_STATE:         4;
        uint32_t IORX_STATE:        4;
        uint32_t IOTX_STATE:        4;
        uint32_t RXBREAK_STATE:     2;
        uint32_t ABR_STATE:         3;
        uint32_t MUTEMODE:          1;
        uint32_t PCMD_STATE:        2;
        uint32_t LIN_CHECKSUM:      8;
    };
    uint32_t v;
} PSR0_Type;

typedef union {
    struct {
        uint32_t AUTOBAUDRATE:      24;
        uint32_t ABRCHAR:           4;
    };
    uint32_t v;
} PSR1_Type;

typedef union {
    struct {
        uint32_t DOORBELL:          16;
        uint32_t RESERVED:          16;
    };
    uint32_t v;
} CMDCSR0_Type;

typedef union {
    struct {
        uint32_t TXBYTENUM:         8;
        uint32_t RXBYTENUM:         8;
        uint32_t PID:               8;
        uint32_t RESERVED:          8;
    };
    uint32_t v;
} CMDCSR1_Type;

typedef union {
    struct {
        uint32_t TXDMAE:            1;
        uint32_t RXDMAE:            1;
        uint32_t TXDMARST:          1;
        uint32_t RXDMARST:          1;
        uint32_t RESERVED:          28;
    };
    uint32_t v;
} DMACR_Type;

typedef union {
    struct {
        uint32_t TXDMA_STATE:       2;
        uint32_t RXDMA_STATE:       2;
        uint32_t TXDMAREQ:          1;
        uint32_t RXDMAREQ:          1;
        uint32_t RESERVED:          26;
    };
    uint32_t v;
} DMASR_Type;

typedef struct sdrv_uart {
    volatile MCR0_Type      MCR0;               /* 0x0 */
    volatile uint32_t       PRDATAINJ;          /* 0x4 */
    volatile uint32_t       MCR2;               /* 0x8 */
    volatile uint32_t       MCR3;               /* 0xC */
    volatile uint32_t       MCR4;               /* 0x10 */
    volatile uint32_t       RESERVED1;          /* 0x14 */
    volatile uint32_t       MSR0;               /* 0x18 */
    volatile uint32_t       MSR1;               /* 0x1C */
    volatile INTR0_Type     INTR0;              /* 0x20 */
    volatile uint32_t       RESERVED2[2];       /* 0x24 ~ 0x28 */
    volatile uint32_t       INTR3;              /* 0x2C */
    volatile INTR0_Type     INTEN0;             /* 0x30 */
    volatile uint32_t       RESERVED3[2];       /* 0x34 ~ 0x38 */
    volatile uint32_t       INTEN3;             /* 0x3C */
    volatile CMDCSR0_Type   CMDCSR0;            /* 0x40 */
    volatile CMDCSR1_Type   CMDCSR1;            /* 0x44 */
    volatile uint32_t       RESERVED4[2];       /* 0x48 ~ 0x4C */
    volatile FCR_Type       FCR0;               /* 0x50 */
    volatile FCR_Type       FCR1;               /* 0x54 */
    volatile uint32_t       RESERVED5[2];       /* 0x58 ~ 0x5C */
    volatile FSR_Type       FSR0;               /* 0x60 */
    volatile FSR_Type       FSR1;               /* 0x64 */
    volatile uint32_t       RESERVED6[2];       /* 0x68 ~ 0x6C */
    volatile DMACR_Type     DMACR;              /* 0x70 */
    volatile DMASR_Type     DMASR;              /* 0x74 */
    volatile uint32_t       RESERVED7[2];       /* 0x78 ~ 0x7C */
    volatile PCR0_Type      PCR0;               /* 0x80 */
    volatile PCR1_Type      PCR1;               /* 0x84 */
    volatile PCR2_Type      PCR2;               /* 0x88 */
    volatile PCR3_Type      PCR3;               /* 0x8C */
    volatile PCR4_Type      PCR4;               /* 0x90 */
    volatile PCR5_Type      PCR5;               /* 0x94 */
    volatile PCR6_Type      PCR6;               /* 0x98 */
    volatile uint32_t       PCR7;               /* 0x9C */
    volatile PCR8_Type      PCR8;               /* 0xA0 */
    volatile uint32_t       PCR9;               /* 0xA4 */
    volatile uint32_t       PCR10;              /* 0xA8 */
    volatile PCR11_Type     PCR11;              /* 0xAC */
    volatile uint32_t       PCR12;              /* 0xB0 */
    volatile uint32_t       PCR13;              /* 0xB4 */
    volatile uint32_t       PCR14;              /* 0xB8 */
    volatile uint32_t       PCR15;              /* 0xBC */
    volatile uint32_t       RESERVED8[8];       /* 0xC0 ~ 0xDC */
    volatile uint32_t       INT_STA;            /* 0xE0 */
    volatile uint32_t       INT_STA_EN;         /* 0xE4 */
    volatile uint32_t       INT_SIG_EN;         /* 0xE8 */
    volatile uint32_t       RESERVED9[5];       /* 0xEC ~ 0xFC */
    volatile PSR0_Type      PSR0;               /* 0x100 */
    volatile PSR1_Type      PSR1;               /* 0x104 */
    volatile uint32_t       PSR2;               /* 0x108 */
    volatile uint32_t       PSR3;               /* 0x10C */
    volatile uint32_t       PSR4;               /* 0x110 */
    volatile uint32_t       PSR5;               /* 0x114 */
    volatile uint32_t       PSR6;               /* 0x118 */
    volatile uint32_t       PSR7;               /* 0x11C */
    volatile uint32_t       RESERVED10[56];      /* 0x120 ~ 0x1FC */
    volatile uint32_t       TXDR;               /* 0x200 */
    volatile uint32_t       RESERVED11[63];     /* 0x204 ~ 0x2FC */
    volatile uint32_t       RXDR;               /* 0x300 */
} sdrv_uart_t;

/**
 * @brief UART op mode.
 */
typedef enum sdrv_uart_op_mode {
    SDRV_UART_SERIAL_MODE,
    SDRV_UART_LIN_MASTER_MODE,
    SDRV_UART_LIN_SLAVE_MODE,
    SDRV_UART_IRDA_MODE,
    SDRV_UART_FIFO_TEST_MODE = 0xf
} sdrv_uart_op_mode_t;

/**
 * @brief Number of bits in one character.
 */
typedef enum sdrv_uart_dls {
    SDRV_UART_CHAR_5_BITS,
    SDRV_UART_CHAR_6_BITS,
    SDRV_UART_CHAR_7_BITS,
    SDRV_UART_CHAR_8_BITS,
    SDRV_UART_CHAR_9_BITS
} sdrv_uart_dls_t;

/**
 * @brief Number of stop bits.
 */
typedef enum sdrv_uart_stop {
    SDRV_UART_STOP_1_BIT,
    SDRV_UART_STOP_1_5_BIT,
    SDRV_UART_STOP_2_BIT,
    SDRV_UART_STOP_0_5_BIT
} sdrv_uart_stop_t;

/**
 * @brief UART parity type.
 */
typedef enum sdrv_uart_parity {
    SDRV_UART_NO_PARITY,
    SDRV_UART_ODD_PARITY,
    SDRV_UART_EVEN_PARITY
} sdrv_uart_parity_t;

/**
 * @brief UART flow control mode.
 */
typedef enum sdrv_uart_fcm {
    SDRV_UART_NO_FC,
    SDRV_UART_SW_FC,
    SDRV_UART_AUTO_FC,
    SDRV_UART_RS485_FC,
    SDRV_UART_XON_XOFF_FC
} sdrv_uart_fcm_t;

typedef struct sdrv_uart_rs485_fcm {
    uint8_t                 deat_cycle;
    uint8_t                 dedt_cycle;
    uint8_t                 de2re_cycle;
    uint8_t                 re2de_cycle;
} sdrv_uart_rs485_fcm_t;

typedef struct sdrv_uart_fcm_config {
    sdrv_uart_fcm_t         fcm_type;
    sdrv_uart_rs485_fcm_t   rs485_fcm;
} sdrv_uart_fcm_config_t;

/**
 * @brief UART abr config.
 */
typedef enum sdrv_uart_abr_max {
    SDRV_UART_ABR_2TIMES,
    SDRV_UART_ABR_4TIMES,
    SDRV_UART_ABR_8TIMES,
    SDRV_UART_ABR_16TIMES,
    SDRV_UART_ABR_32TIMES,
    SDRV_UART_ABR_64TIMES,
    SDRV_UART_ABR_128TIMES,
    SDRV_UART_ABR_256TIMES
} sdrv_uart_abr_max_t;

typedef struct sdrv_uart_abr_config {
    uint8_t                 abr_en;
    uint8_t                 pattern;
    sdrv_uart_abr_max_t     max_time;
} sdrv_uart_abr_config_t;

/**
 * @brief UART transfer mode.
 */
typedef enum sdrv_uart_transfer_mode {
    SDRV_UART_FULL_DUPLEX,
    SDRV_UART_HALF_DUPLEX
} sdrv_uart_transfer_mode_t;

/**
 * @brief UART configurations.
 */
typedef struct sdrv_uart_config {
    uint32_t                    baud;
    uint32_t                    clk_freq;
    sdrv_uart_op_mode_t         op_mode;
    sdrv_uart_dls_t             data_bits;
    sdrv_uart_stop_t            stop_bits;
    sdrv_uart_parity_t          parity;
    sdrv_uart_transfer_mode_t   transfer_mode;
    sdrv_uart_fcm_config_t      fcm;
    sdrv_uart_abr_config_t      abr;
    bool                        dma_en;
    uint8_t                     txwmlvl;    // byte: txwmlvl + 1
    uint8_t                     rxmwlvl;    // byte: rxwmlvl + 1
    uint8_t                     breakbit;   // only for lin
    uint8_t                     chksum;     // only for lin
    uint8_t                     tx_pul;     // only for sir
    uint8_t                     rx_pul;     // only for sir
} sdrv_uart_config_t;

uint32_t sdrv_uart_lld_handle_irq(paddr_t base);
void sdrv_uart_lld_clear_irq(paddr_t base, uint32_t state);
void sdrv_uart_lld_enable_tx(paddr_t base, bool enable);
void sdrv_uart_lld_enable_rx(paddr_t base, bool enable);
void sdrv_uart_lld_enable_txint(paddr_t base, bool enable);
void sdrv_uart_lld_enable_rxint(paddr_t base, bool enable);
void sdrv_uart_lld_enable_rxfovf(paddr_t base, bool enable);
void sdrv_uart_lld_start_abr(paddr_t base);
void sdrv_uart_lld_stop_abr(paddr_t base);
size_t sdrv_uart_lld_transmit_async(paddr_t base, const char *data,
                                    size_t size);
size_t sdrv_uart_lld_receive_async(paddr_t base, char *data,
                                   size_t size);
void sdrv_uart_lld_putc(paddr_t base, char c);
uint8_t sdrv_uart_lld_getc(paddr_t base);
void sdrv_uart_lld_transmit(paddr_t base, const char *data, size_t size);
void sdrv_uart_lld_receive(paddr_t base, char *data, size_t size);
void sdrv_uart_lld_set_baudrate(paddr_t base, uint32_t baud, uint32_t clk_freq);
uint32_t sdrv_uart_lld_get_baudrate(paddr_t base);
void sdrv_uart_lld_init(paddr_t base, sdrv_uart_config_t *cfg);
void sdrv_uart_lld_deinit(paddr_t base);

void sdrv_uart_lld_lin_transmit(paddr_t base, uint8_t cs,
                                const char *data, uint8_t size);
void sdrv_uart_lld_lin_receive(paddr_t base,
                               uint8_t cs, uint8_t size);
void sdrv_uart_lld_lin_abort(paddr_t base);
void sdrv_uart_lld_set_lin_pid(paddr_t base, uint8_t pid);
uint8_t sdrv_uart_lld_get_lin_pid(paddr_t base);
uint8_t sdrv_uart_lld_get_lin_chksum(paddr_t base);
void sdrv_uart_lld_sir_transmit(paddr_t base, const char *data, uint8_t size);
void sdrv_uart_lld_sir_receive(paddr_t base, uint8_t size);

#endif /* SDRV_UART_H */
