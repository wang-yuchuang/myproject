/**
 * @file xspi_validation.h
 * @brief SSDK XSPI validation configuration header file.
 *
 * @copyright Copyright (c) 2022 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __SPI_NOR_VALIDATION_H__
#define __SPI_NOR_VALIDATION_H__

#include <stdbool.h>
#include <types.h>
#include <driver/spi_nor.h>
#include "driver/taihang/spi_nor/taihang_xspi_drv.h"
#include "driver/kunlun/spi_nor/cdns_ospi_drv.h"

#define PROTO(_opcode, _dq) \
        ((uint32_t)(_opcode) << SNOR_OPCODE_PROTO_LSB | (_dq))

/* Flash opcodes. */
#define SPINOR_OP_WREN 0x06       /* Write enable */
#define SPINOR_OP_WRDI 0x04       /* Write disable */
#define SPINOR_OP_RDSR 0x05       /* Read status register */
#define SPINOR_OP_WRSR 0x01       /* Write status register 1 byte */
#define SPINOR_OP_RDSR2 0x3f      /* Read status register 2 */
#define SPINOR_OP_WRSR2 0x3e      /* Write status register 2 */
#define SPINOR_OP_READ 0x03       /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST 0x0b  /* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2 0x3b /* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2 0xbb /* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4 0x6b /* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4 0xeb /* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8 0x8b /* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8 0xcb /* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_PP 0x02         /* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4 0x32   /* Quad page program */
#define SPINOR_OP_PP_1_4_4 0x38   /* Quad page program */
#define SPINOR_OP_PP_1_1_8 0x82   /* Octal page program */
#define SPINOR_OP_PP_1_8_8 0xc2   /* Octal page program */
#define SPINOR_OP_BE_4K 0x20      /* Erase 4KiB block */
#define SPINOR_OP_BE_4K_PMC 0xd7  /* Erase 4KiB block on PMC chips */
#define SPINOR_OP_BE_32K 0x52     /* Erase 32KiB block */
#define SPINOR_OP_CHIP_ERASE 0xc7 /* Erase whole flash chip */
#define SPINOR_OP_SE 0xd8         /* Sector erase (usually 64KiB) */
#define SPINOR_OP_RDID 0x9f       /* Read JEDEC ID */
#define SPINOR_OP_RDSFDP 0x5a     /* Read SFDP */
#define SPINOR_OP_RDCR 0x35       /* Read configuration register */
#define SPINOR_OP_RDFSR 0x70      /* Read flag status register */
#define SPINOR_OP_CLFSR 0x50      /* Clear flag status register */
#define SPINOR_OP_RDEAR 0xc8      /* Read Extended Address Register */
#define SPINOR_OP_WREAR 0xc5      /* Write Extended Address Register */
#define SPINOR_OP_RSTEN 0x66      /* Reset Enable register */
#define SPINOR_OP_RSTMEM 0x99     /* Reset Memory */
#define SPINOR_OP_EN4B 0xb7       /* Enter 4-byte mode */
#define SPINOR_OP_EX4B 0xe9       /* Exit 4-byte mode */


/* 4-byte address opcodes - used on Spansion and some Macronix flashes. */
#define SPINOR_OP_READ_4B 0x13       /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST_4B 0x0c  /* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2_4B 0x3c /* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2_4B 0xbc /* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4_4B 0x6c /* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4_4B 0xec /* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8_4B 0x7c /* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8_4B 0xcc /* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_PP_4B 0x12         /* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4_4B 0x34   /* Quad page program */
#define SPINOR_OP_PP_1_4_4_4B 0x3e   /* Quad page program */
#define SPINOR_OP_PP_1_1_8_4B 0x84   /* Octal page program */
#define SPINOR_OP_PP_1_8_8_4B 0x8e   /* Octal page program */
#define SPINOR_OP_BE_4K_4B 0x21      /* Erase 4KiB block */
#define SPINOR_OP_BE_32K_4B 0x5c     /* Erase 32KiB block */
#define SPINOR_OP_SE_4B 0xdc         /* Sector erase (usually 64KiB) */

/* issi giga */
#define SPINOR_OP_READ_4S_4D_4D 0xed

/* giga */
#define SPINOR_OP_PP_1_4_4_GIGA 0xc2

/* Double Transfer Rate opcodes - defined in JEDEC JESD216B. */
#define SPINOR_OP_READ_1_1_1_DTR 0x0d
#define SPINOR_OP_READ_1_2_2_DTR 0xbd
#define SPINOR_OP_READ_1_4_4_DTR 0xed
#define SPINOR_OP_READ_1_1_8_DTR 0x9d

#define SPINOR_OP_READ_1_1_1_DTR_4B 0x0e
#define SPINOR_OP_READ_1_2_2_DTR_4B 0xbe
#define SPINOR_OP_READ_1_4_4_DTR_4B 0xee
#define SPINOR_OP_READ_1_8_8_DTR_4B 0xfd

typedef struct {
    const char *name;                     /**< flash name */
    uint8_t flash_id[SPI_NOR_MAX_ID_LEN]; /**< get flash id */

    uint32_t read_proto;  /**< falsh read transmission proto */
    uint32_t write_proto; /**< falsh write transmission proto */
    uint32_t erase_proto; /**< default falsh erase transmission proto */
    uint32_t erase_proto_list[SPI_NOR_SECTOR_TYPE_MAX]; /**< support falsh erase transmission protos */

    uint8_t read_dummy;   /**< falsh read dummy, it depends on the frequency */
    uint8_t write_dummy;  /**< falsh write dummy, it depends on the frequency */
    uint8_t status_dummy; /**< falsh get status dummy */

    int (*init)(struct spi_nor*);   /**< flash init ops */
} validation_flash_info_t;

typedef enum {
    XSPI_MASTER_1S_1S_1S_3B = 0,
    XSPI_MASTER_1S_1S_2S,
    XSPI_MASTER_1S_2S_2S,
    XSPI_MASTER_1S_2D_2D,
    XSPI_MASTER_1S_1S_4S,
    XSPI_MASTER_1S_4S_4S,
    XSPI_MASTER_1S_4D_4D,
    XSPI_MASTER_4S_4D_4D,
    XSPI_MASTER_1S_1S_8S,
    XSPI_MASTER_1S_8S_8S,
    XSPI_MASTER_1S_8D_8D,
    XSPI_MASTER_8D_8D_8D_DQS,
    XSPI_MASTER_8D_8D_8D,
    XSPI_MASTER_8D_8D_8D_NOCACHE,
    XSPI_MASTER_8D_8D_8D_PARALLEL,
    XSPI_MASTER_8D_8D_8D_LOCKSTEP_LOOP,
    XSPI_MASTER_8D_8D_8D_LOCKSTEP_SIGNAL,
    XSPI_MASTER_HYPERFLASH,
    XSPI_MASTER_HYPERRAM,
    XSPI_MASTER_MODE3,
    XSPI_MASTER_CRC,
    XSPI_MASTER_WP,
    XSPI_MASTER_AUTO_POLLING,
    XSPI_MASTER_IO_TIMING,
    XSPI_MASTER_DIFF_CLOCK,
    XSPI_MASTER_PARA_FIFO_SIZE,
    XSPI_MASTER_INDIRECT_AXI_MODE,
    XSPI_MASTER_HYPERRAM_LOCKSTEP,
    XSPI_MASTER_HYPERRAM_PARALLEL,
    XSPI_MASTER_HYPERFLASH_LOCKSTEP,
    XSPI_MASTER_HYPERFLASH_PARALLEL,
    XSPI_MASTER_PARALLEL_CRC,
    XSPI_MASTER_LOCKSTEP_READ_ERROR,
    XSPI_MASTER_SLV_XIP_MODE,
    XSPI_MASTER_CASE_MAX_NUM,
} validation_case_type_t;

typedef struct {
    validation_case_type_t type;
    const char *name;
    bool dsp_en;
    uint32_t proto;
    uint8_t addr_width;
    enum spi_nor_device_mode dev_mode;
    bool no_cache;
    bool crc_en;
    bool slv_xip_mode;
    uint8_t port_id;

    validation_flash_info_t flash_info;

    struct spi_nor_host spi_nor_host;
    struct spi_nor flash;
    struct flash_info *info;
    struct xspi_pdata xspi_data;
    struct cospi_pdata ospi_data;
} validation_context_t;

typedef enum {
    XSPI1_PORTA = 0,
    XSPI1_PORTB,
    OSPI1_PORT,
    OSPI2_PORT,
    CONTROLLER_ID_MAX,
} validation_controller_id_t;

#endif // __SPI_NOR_VALIDATION_H__
