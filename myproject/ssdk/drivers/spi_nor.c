/**
 * @file spi_nor.c
 * @brief spi norfash driver C code.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#if defined(CONFIG_OSPI) || defined(CONFIG_XSPI)

#include <stdio.h>
#include <compiler.h>
#include <string.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <param.h>
#include <board.h>

#include "driver/spi_nor.h"

#define ARRAY_NUMS(array) (sizeof(array) / sizeof(array[0]))

#define PROTO(_opcode, _dq) \
        ((uint32_t)(_opcode) << SNOR_OPCODE_PROTO_LSB | (_dq))
#define ID_PROTO(dummy, _dq) \
        ((uint32_t)(dummy) << SNOR_READID_DUMMY_LSB | (_dq))

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

#define SPINOR_ID_CAPACITY_OFFSET 2


static uint8_t training_pattern[32] __ALIGNED(CONFIG_ARCH_CACHE_LINE) = {
    0x44, 0x1c, 0x39, 0x05, 0xd3, 0x7a, 0x3c, 0x04,
    0x16, 0x42, 0x0c, 0x8b, 0x7d, 0x12, 0x89, 0xa2,
    0xb8, 0xb1, 0xf7, 0xe8, 0xb7, 0x49, 0xca, 0x1c,
    0xaa, 0x9b, 0xf2, 0x7e, 0x01, 0x97, 0x60, 0x8c
};
static uint8_t training_buf[32] __ALIGNED(CONFIG_ARCH_CACHE_LINE) = {0};

static int spi_nor_general_set_4byte_addr_mode(struct spi_nor *nor,
        bool enable);
static int miron_default_init(struct spi_nor *nor);
static int miron_octal_dtr_enable(struct spi_nor *nor, bool enable);
static int issi_quad_enable(struct spi_nor *nor, bool enable);
static int issi_enter_quad(struct spi_nor *nor, bool enable);
static int issi_default_init(struct spi_nor *nor);
static int cypress_default_init(struct spi_nor *nor);
static int cypress_set_4byte_addr_mode(struct spi_nor *nor, bool enable);
static int cypress_octal_dtr_enable(struct spi_nor *nor, bool enable);
static int giga_enter_quad(struct spi_nor *nor, bool enable);
static int giga_default_init(struct spi_nor *nor);
static int giga_quad_enable(struct spi_nor *nor, bool enable);
static inline int spibus_write_enable_locked(struct spi_nor *nor,
        bool enable);

static int spibus_wait_idle(struct spi_nor *nor);
static int spibus_write_enable(struct spi_nor *nor, bool enable);
static int spibus_init(struct spi_nor *nor, struct spi_nor_host *host,
                 const struct spi_nor_config *config);
static void spibus_deinit(struct spi_nor *nor);
static int spibus_read(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                 flash_size_t size);
static int spibus_write(struct spi_nor *nor, flash_addr_t addr, const uint8_t *buf,
                  flash_size_t size);
static int spibus_erase(struct spi_nor *nor, flash_addr_t addr, flash_size_t size);
static int spibus_cancel(struct spi_nor *nor);
static void spibus_main_function(struct spi_nor *nor);

flash_ops_t spi_nor_ops = {
    .fls_wait_idle= spibus_wait_idle,
    .fls_write_enable= spibus_write_enable,
    .fls_init= spibus_init,
    .fls_deinit= spibus_deinit,
    .fls_read= spibus_read,
    .fls_write= spibus_write,
    .fls_erase= spibus_erase,
    .fls_cancel= spibus_cancel,
    .fls_main_function= spibus_main_function,
};

static struct flash_info default_spi_nor_ids = {
    .name = "default",
    .flash_id = {0x00, 0x00},
    .read_proto = PROTO(SPINOR_OP_READ_FAST, SNOR_PROTO_1_1_1),
    .write_proto = PROTO(SPINOR_OP_PP, SNOR_PROTO_1_1_1),
    .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_1),
    .erase_proto_list = {
        PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_1),  // erase 4Kb, 0: nonsupport
        0,  // erase 32Kb, 0: nonsupport
        0,  // erase 64Kb, 0: nonsupport
        0,  // erase 128Kb, 0: nonsupport
        0,  // erase 256Kb, 0: nonsupport
    },
    .sector_size = SPINOR_SECTOR_4K_SIZE,
    .read_dummy = 8,
    .status_dummy = 0,
    .page_size = 256,
};

static struct flash_info spi_nor_ids[] = {
    /* miron */
    {
        .name = "mt35xu",
        .flash_id = {0x2c, 0x5b},
        .read_proto = PROTO(SPINOR_OP_READ_1_8_8, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(SPINOR_OP_PP_1_8_8, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_8_8_8_DTR),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_8_8_8_DTR),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_8_8_8_DTR),  // erase 32Kb, 0: nonsupport
            0,  // erase 64Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_8_8_8_DTR),  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 20,
        .status_dummy = 8,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .octal_dtr_enable = miron_octal_dtr_enable,
        .default_init = miron_default_init
    },
    /* miron */
    {
        .name = "mt35xl",
        .flash_id = {0x2c, 0x5a},
        .read_proto = PROTO(SPINOR_OP_READ_1_8_8, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(SPINOR_OP_PP_1_8_8, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_8_8_8_DTR),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_8_8_8_DTR),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_8_8_8_DTR),  // erase 32Kb, 0: nonsupport
            0,  // erase 64Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_8_8_8_DTR),  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 20,
        .status_dummy = 8,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .octal_dtr_enable = miron_octal_dtr_enable,
        .default_init = miron_default_init
    },
    /* issi */
    {
        .name = "is25wp",
        .flash_id = {0x9d, 0x70},
        .read_proto = PROTO(SPINOR_OP_READ_1_4_4, SNOR_PROTO_4_4_4),
        .write_proto = PROTO(SPINOR_OP_PP, SNOR_PROTO_4_4_4),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_4_4_4),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_4_4_4),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_4_4_4),  // erase 32Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_4_4_4),  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 14,
        .status_dummy = 0,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .quad_enable = issi_quad_enable,
        .enter_quad = issi_enter_quad,
        .default_init = issi_default_init
    },
    {
        .name = "is25lp",
        .flash_id = {0x9d, 0x60},
        .read_proto = PROTO(SPINOR_OP_READ_1_4_4, SNOR_PROTO_4_4_4),
        .write_proto = PROTO(SPINOR_OP_PP, SNOR_PROTO_4_4_4),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_4_4_4),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_4_4_4),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_4_4_4),  // erase 32Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_4_4_4),  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 14,
        .status_dummy = 0,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .quad_enable = issi_quad_enable,
        .enter_quad = issi_enter_quad,
        .default_init = issi_default_init
    },
    /* giga */
    {
        .name = "gd25lb",
        .flash_id = {0xc8, 0x67},
        .read_proto = PROTO(SPINOR_OP_READ_1_1_4, SNOR_PROTO_4_4_4),
        .write_proto = PROTO(SPINOR_OP_PP_1_4_4_GIGA, SNOR_PROTO_4_4_4),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_4_4_4),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_4_4_4),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_4_4_4),  // erase 32Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_4_4_4),  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 10,
        .status_dummy = 0,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .enter_quad = giga_enter_quad,
        .default_init = giga_default_init
    },
        /* giga */
    {
        .name = "gd25q",
        .flash_id = {0xc8, 0x40},
        .read_proto = PROTO(SPINOR_OP_READ_1_1_4, SNOR_PROTO_1_1_4),
        .write_proto = PROTO(SPINOR_OP_PP_1_1_4, SNOR_PROTO_1_1_4),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_4),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_4),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_1_1_4),  // erase 32Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_1_1_4),  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 8,
        .status_dummy = 0,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .quad_enable = giga_quad_enable,
    },
    {
        .name = "gd25wq",
        .flash_id = {0xc8, 0x65},
        .read_proto = PROTO(SPINOR_OP_READ_1_1_4, SNOR_PROTO_1_1_4),
        .write_proto = PROTO(SPINOR_OP_PP_1_1_4, SNOR_PROTO_1_1_4),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_4),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_4),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_1_1_4),  // erase 32Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_1_1_4),  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 8,
        .status_dummy = 0,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .quad_enable = giga_quad_enable,
    },
    {
        .name = "gd25f",
        .flash_id = {0xc8, 0x43},
        .read_proto = PROTO(SPINOR_OP_READ_1_1_4, SNOR_PROTO_1_1_4),
        .write_proto = PROTO(SPINOR_OP_PP_1_1_4, SNOR_PROTO_1_1_4),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_4),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_4),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_1_1_4),  // erase 32Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_1_1_4),  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 8,
        .status_dummy = 0,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .quad_enable = giga_quad_enable,
    },
    {
        .name = "gd25lx",
        .flash_id = {0xc8, 0x68},
        .read_proto = PROTO(SPINOR_OP_READ_1_8_8, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(SPINOR_OP_PP_1_8_8, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_8_8_8_DTR),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_8_8_8_DTR),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_8_8_8_DTR),  // erase 32Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_8_8_8_DTR),  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 16,
        .status_dummy = 8,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .default_init = miron_default_init,
        .octal_dtr_enable = miron_octal_dtr_enable,
    },
    {
        .name = "gd25x",
        .flash_id = {0xc8, 0x48},
        .read_proto = PROTO(SPINOR_OP_READ_1_8_8, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(SPINOR_OP_PP_1_8_8, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_8_8_8_DTR),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_8_8_8_DTR),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_8_8_8_DTR),  // erase 32Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_8_8_8_DTR),  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 14,
        .status_dummy = 8,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .default_init = miron_default_init,
        .octal_dtr_enable = miron_octal_dtr_enable,
    },
    {
        .name = "gd25b",
        .flash_id = {0xc8, 0x47},
        .read_proto = PROTO(SPINOR_OP_READ_1_1_4, SNOR_PROTO_4_4_4),
        .write_proto = PROTO(SPINOR_OP_PP_1_4_4_GIGA, SNOR_PROTO_4_4_4),
        .erase_proto = PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_4_4_4),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_8_8_8_DTR),  // erase 4Kb, 0: nonsupport
            PROTO(SPINOR_OP_BE_32K, SNOR_PROTO_8_8_8_DTR),  // erase 32Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE, SNOR_PROTO_8_8_8_DTR),  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            0,  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 10,
        .status_dummy = 0,
        .page_size = 256,
        .set_4byte_addr_mode = spi_nor_general_set_4byte_addr_mode,
        .enter_quad = giga_enter_quad,
        .default_init = giga_default_init
    },
    {
        .name = "S28hl",
        .flash_id = {0x34, 0x5a},
        .read_proto = PROTO(SPINOR_OP_READ_1_4_4_DTR_4B, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(SPINOR_OP_PP_4B, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(SPINOR_OP_SE_4B, SNOR_PROTO_8_8_8_DTR),
        .erase_proto_list = {
            PROTO(SPINOR_OP_BE_4K_4B, SNOR_PROTO_8_8_8_DTR),  // erase 4Kb, 0: nonsupport
            0,  // erase 32Kb, 0: nonsupport
            0,  // erase 64Kb, 0: nonsupport
            0,  // erase 128Kb, 0: nonsupport
            PROTO(SPINOR_OP_SE_4B, SNOR_PROTO_8_8_8_DTR),  // erase 256Kb, 0: nonsupport
        },
        .sector_size = SPINOR_SECTOR_256K_SIZE,
        .read_dummy = 23,
        .status_dummy = 8,
        .page_size = 256,
        .set_4byte_addr_mode = cypress_set_4byte_addr_mode,
        .default_init = cypress_default_init,
        .octal_dtr_enable = cypress_octal_dtr_enable,
    },
};

/**
 * @brief spi nor read register
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] cmd spinor cmd ptr
 * @param[in] addr flash read address
 * @param[in] buf read to buf addr
 * @param[in] length read size
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
int spi_nor_reg_read(struct spi_nor *flash_handle,
                                 struct spi_nor_cmd *cmd, flash_addr_t addr,
                                 uint8_t *buf, flash_size_t length)
{
    int ret;

    flash_handle->host->ops->prepare(flash_handle, SPI_NOR_OPS_LOCK);
    ret = flash_handle->host->ops->reg_read(flash_handle, cmd, addr, buf, length);
    flash_handle->host->ops->unprepare(flash_handle, SPI_NOR_OPS_LOCK);

    return ret;
}

/**
 * @brief spi nor write register
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] cmd spinor cmd ptr
 * @param[in] addr flash read address
 * @param[in] buf write from buf addr
 * @param[in] length write size
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
int spi_nor_reg_write(struct spi_nor *flash_handle,
                                  struct spi_nor_cmd *cmd, flash_addr_t addr,
                                  uint8_t *buf, flash_size_t length)
{
    int ret;

    flash_handle->host->ops->prepare(flash_handle, SPI_NOR_OPS_LOCK);
    ret = flash_handle->host->ops->reg_write(flash_handle, cmd, addr, buf, length);
    flash_handle->host->ops->unprepare(flash_handle, SPI_NOR_OPS_LOCK);

    return ret;
}

/**
 * @brief spi nor get flash current status
 * @param[in] nor spi norflash ptr.
 * @param[out] status get spinor current status
 * @return spinor current status
 */
static int spi_nor_get_status(struct spi_nor *nor, uint8_t *status)
{
    uint8_t reg[2] = {0};

    int ret;
    struct spi_nor_cmd read_cmd = {
        .opcode = SPINOR_OP_RDSR,
        .addr_bytes = 0,
        .dummy = nor->info.status_dummy,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = nor->host->ops->reg_read(nor, &read_cmd, 0, reg, 2);
    if (ret) {
        return ret;
    }

    *status = reg[0];
    return 0;
}


static int spibus_write_enable(struct spi_nor *nor, bool enable)
{
    int ret;

    struct spi_nor_cmd wr_en_cmd = {
        .opcode = enable ? SPINOR_OP_WREN : SPINOR_OP_WRDI,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    ret = nor->host->ops->reg_write(nor, &wr_en_cmd, 0, 0, 0);
    return SPI_NOR_ERR_STATUS(SPI_NOR_COMMON, ret);
}

/**
 * @brief spinor write enable,use it before write flash
 * @param[in] nor spi norflash ptr.
 * @param[out] enable write enable not not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static inline int spibus_write_enable_locked(struct spi_nor *nor,
        bool enable)
{
    int ret;

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    ret = spibus_write_enable(nor, enable);
    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    return ret;
}

/**
 * @brief wait for the flash to be idle
 * @param[in] nor spi norflash ptr.
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static inline int spibus_wait_idle_locked(struct spi_nor *nor)
{
    int ret;

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    ret = spibus_wait_idle(nor);
    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    return ret;
}

/* wait flash entry idle status in 3 seconds */
static int spibus_wait_idle(struct spi_nor *nor)
{
    int ret = SPI_NOR_OK_STATUS;
    uint8_t flash_status;
    uint32_t tick_count = 0;
    uint32_t parallel_count = 0;

    while (1) {
        ret = spi_nor_get_status(nor, &flash_status);

        if (ret) {
            if ((nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE)
                && (parallel_count < 10000)) {
                parallel_count++;
                continue;
            } else {
                ret = SPI_NOR_UNREACHABLE;
                ssdk_printf(SSDK_CRIT,
                            "spi_nor get flash status failed, ret: %d!\n", ret);
                break;
            }
        }

        ssdk_printf(SSDK_INFO, "flash_status = 0x%x \n", flash_status);
        if (!(flash_status & BIT(0)))
            break;

        if (tick_count > 3000) {
            ret = SPI_NOR_TIMEOUT;
            ssdk_printf(SSDK_CRIT, "wait flash idle timeout, ret = %d!\n", ret);
            break;
        }

        udelay(1000u);
        tick_count++;
    }

    return SPI_NOR_ERR_STATUS(SPI_NOR_COMMON, ret);
}

/**
 * @brief spi nor flash soft reset
 * @param[in] nor spi norflash ptr.
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int spi_nor_soft_reset(struct spi_nor *nor)
{
    int ret = 0;

    struct spi_nor_cmd rst_en_cmd = {
        .opcode = SPINOR_OP_RSTEN,
        .dummy = 0,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    struct spi_nor_cmd rst_mem_cmd = {
        .opcode = SPINOR_OP_RSTMEM,
        .dummy = 0,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = spi_nor_reg_write(nor, &rst_en_cmd, 0, 0, 0);

    if (ret) {
        return ret;
    }

    ret = spi_nor_reg_write(nor, &rst_mem_cmd, 0, 0, 0);

    return ret;
}


/**
 * @brief spi nor read device info
 * 1.use SNOR_PROTO_1_1_1 or
 * SNOR_PROTO_2_2_2 or SNOR_PROTO_4_4_4 or SNOR_PROTO_8_8_8_DTR mode
 * try to read  device id
 * 2.read flash capacity
 * @param[in] nor spi norflash ptr.
 * @retval 0: success
 * @retval !0: failed
 */
static int spi_nor_id_read(struct spi_nor *nor)
{
    int ret = 0;
    uint8_t id[8];
    struct flash_info *info;
    uint32_t reg_lans_list[] = {ID_PROTO(0, SNOR_PROTO_1_1_1), ID_PROTO(0, SNOR_PROTO_2_2_2),
                                ID_PROTO(0, SNOR_PROTO_4_4_4), ID_PROTO(8, SNOR_PROTO_8_8_8_DTR)
                               };

    struct spi_nor_cmd read_cmd = {
        .opcode = SPINOR_OP_RDID,
        .dummy = reg_lans_list[0] >> SNOR_READID_DUMMY_LSB,
                                  .addr_bytes = 0,
                                  .inst_type = SNOR_INST_LANS(reg_lans_list[0]),
    };

    for (uint32_t i = 0; i < ARRAY_SIZE(reg_lans_list); i++) {
        read_cmd.dummy = (reg_lans_list[i] >> SNOR_READID_DUMMY_LSB) & 0xff;
        read_cmd.inst_type = SNOR_INST_LANS(reg_lans_list[i]);
        nor->reg_proto = reg_lans_list[i] & SNOR_PROTO_MASK;

        if (reg_lans_list[i] & SNOR_DTR_PROTO) {
            nor->octal_dtr_en = 1;
            nor->addr_width = 4;
        }

        ssdk_printf(SSDK_NOTICE, "octal_dtr_en: %d, dummy = %d, inst_type = %d, \
proto = %x\n", nor->octal_dtr_en, read_cmd.dummy, read_cmd.inst_type,
                    nor->reg_proto);

        ret = spi_nor_reg_read(nor, &read_cmd, 0, id, 4);

        if (ret) return -1;

        ssdk_printf(SSDK_CRIT, "norflash id0: %x, id1: %x, id2: %x\r\n", id[0], id[1],
                    id[2]);

        for (uint32_t i = 0; i < ARRAY_NUMS(spi_nor_ids); i++) {
            info = &spi_nor_ids[i];

            if (!memcmp(info->flash_id, id, 2)) {
                memcpy(&(nor->info), info, sizeof(struct flash_info));
                nor->info.size = 1 << id[SPINOR_ID_CAPACITY_OFFSET];
                return 0;
            }
        }
    }

    return -1;
}

/**
 * @brief spi nor flash octal dtr enable,if device support octal dtr mode and enable it
 * @param[in] nor spi norflash ptr.
 * @param[in] enable spi norflash octal dtr enable or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int spi_nor_octal_dtr_enable(struct spi_nor *nor, bool enable)
{
    int ret;

    if (!nor->info.octal_dtr_enable) {
        return 0;
    }

    if (!(SNOR_PROTO_DTR(nor->info.read_proto) == SNOR_PROTO_8_8_8_DTR &&
            SNOR_PROTO_DTR(nor->info.write_proto) == SNOR_PROTO_8_8_8_DTR)) {
        return 0;
    }

    ret = nor->info.octal_dtr_enable(nor, enable);

    if (ret) {
        return ret;
    }

    if (enable) {
        nor->octal_dtr_en = 1;
        nor->dqs_en = 1;
        nor->reg_proto = SNOR_PROTO_8_8_8_DTR;
        nor->addr_width = 4;
    }
    else {
        nor->octal_dtr_en = 0;
        nor->dqs_en = 0;
        nor->reg_proto = SNOR_PROTO_1_1_1;
        nor->addr_width = 3;
    }

    return ret;
}


/**
 * @brief spi nor flash quad enable,according to read/write data lans and enable it or not
 * @param[in] nor spi norflash ptr.
 * @param[in] enable spi norflash quad enable or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int spi_nor_quad_enable(struct spi_nor *nor, bool enable)
{
    int ret;

    if (!nor->info.quad_enable) {
        return 0;
    }

    if (!(SNOR_DATA_LANS(nor->info.read_proto) == 2 ||
            SNOR_DATA_LANS(nor->info.write_proto) == 2)) {
        return 0;
    }

    ret = nor->info.quad_enable(nor, enable);

    return ret;
}

/**
 * @brief spi nor flash qpi enable,according to read/write data lans and enable it or not
 * @param[in] nor spi norflash ptr.
 * @param[in] enable spi norflash qpi enable or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int spi_nor_enter_quad(struct spi_nor *nor, bool enable)
{
    int ret;

    if (!nor->info.enter_quad) {
        return 0;
    }

    if (SNOR_PROTO(nor->info.read_proto) != SNOR_PROTO_4_4_4 ||
            SNOR_PROTO(nor->info.write_proto) != SNOR_PROTO_4_4_4) {
        return 0;
    }

    ret = nor->info.enter_quad(nor, enable);

    if (ret) {
        return ret;
    }

    nor->reg_proto = enable ? SNOR_PROTO_4_4_4 : SNOR_PROTO_1_1_1;

    return ret;
}


/**
 * @brief spi nor flash enable qpi 4byte mode ,according to read flash capacity and enable it or not, if more than 16MB, set 4 byte mode
 * @param[in] nor spi norflash ptr.
 * @param[in] enable spi norflash qpi enable or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int spi_nor_set_4byte(struct spi_nor *nor, bool enable)
{
    int ret;

    if (!nor->info.set_4byte_addr_mode ||
            nor->info.size <= 0x1000000) {
        return 0;
    }

    ret = nor->info.set_4byte_addr_mode(nor, enable);

    if (ret) {
        return ret;
    }

    nor->addr_width = enable ? 4 : 3;

    return ret;
}

/**
 * @brief entry function for Autofill information.
 * 1. auto fill flash info
 * @param[in] spi nor ptr.
 * @return void
 */
static void spibus_auto_fill(struct spi_nor *nor)
{
    uint32_t i;
    // auto fill flash info
    memset(nor->info.erase_map, 0, sizeof(nor->info.erase_map));
    for (i = 0; i < SPI_NOR_SECTOR_TYPE_MAX; i++) {
        if (!(nor->info.erase_proto_list[i])) {
            continue;  // skip nonsupport proto
        }
        nor->info.erase_map[i].erase_proto = nor->info.erase_proto_list[i];
        nor->info.erase_map[i].erase_size = SECTOR_TYPE_TO_SIZE(i);
        nor->info.erase_map[i].post = SECTOR_TYPE_TO_POST(i);
    }
}

/**
 * @brief entry function for init spi norflash.
 * 1. read flash id info
 * 2. soft reset or not
 * 3. according to different flash info filled and use callback func
 * 4. spi norflash read training
 * @param[in] spi nor ptr.
 * @param[in] spi nor_host config ptr.
 * @param[in] spi nor config config.
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int spibus_init(struct spi_nor *nor, struct spi_nor_host *host,
                      const struct spi_nor_config *config)
{
    int ret = SPI_NOR_OK_STATUS;
    uint32_t training_len = 32;
    flash_addr_t training_addr;

    if (config->cs >= SWITCH_DEVICE_MAX_NUM) {
        ssdk_printf(SSDK_ERR, "flash cs%d out of scope\r\n", config->cs);
        return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, SPI_NOR_FAIL);
    }

    memset(nor, 0, sizeof(struct spi_nor));

    for (uint8_t i = 0; i < config->cs; i++) {
        if (host->nor_tab[i]) {
            nor->offset_address += host->nor_tab[i]->info.size;
        } else {
            ssdk_printf(SSDK_ERR, "flash cs%d need init firstly\r\n", i);
            return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, SPI_NOR_FAIL);
        }
    }

    nor->ops = &spi_nor_ops;
    nor->id = config->id;
    nor->cs = config->cs;
    nor->baudrate = config->baudrate;
    nor->xfer_mode = config->xfer_mode;
    nor->dev_mode = config->dev_mode;
    nor->host = host;
    nor->sw_rst = config->sw_rst;
    nor->hyperbus_mode = false;

    nor->reg_proto = SNOR_PROTO_1_1_1;
    nor->addr_width = 3;
    nor->octal_dtr_en = 0;
    nor->dqs_en = 0;

    memcpy(&(nor->info), &default_spi_nor_ids, sizeof(struct flash_info));

    if (host->xip_mode) {
		if (nor->host->ops->xip_proto_setup) {
			nor->host->ops->xip_proto_setup(nor);
		} else {
		    ssdk_printf(SSDK_CRIT, "Not support xip mode!");
        	return -1;
		}
    } else {
        ret = spi_nor_id_read(nor);

        if (ret) {
            return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, SPI_NOR_FAIL);
        }
    }

    if (config->force_size) {
        nor->info.size = config->force_size;
    }

    spibus_auto_fill(nor);

    if (nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE) {
        nor->info.sector_size *= 2u;
        nor->info.size *= 2u;
    }

    ssdk_printf(SSDK_NOTICE, "flash size: %llx\r\n", nor->info.size);

    if (!host->xip_mode) {
        if (nor->sw_rst) {
            spi_nor_soft_reset(nor);
            /* 1ms delay */
            udelay(1000u);
            nor->octal_dtr_en = 0;
            nor->reg_proto = SNOR_PROTO_1_1_1;
            nor->addr_width = 3;
            nor->dqs_en = 0;
        }

#ifdef CONFIG_SPI_NOR_VALIDATION

        if (config->flash_init) {
            ret = config->flash_init(nor);
        } else {
            return SDRV_STATUS_INVALID_PARAM;
        }

#else
        /* set dummy ... */
        if (nor->info.default_init) {
            nor->info.default_init(nor);
        }

        /* if more than 16MB, set 4 byte mode */
        ret |= spi_nor_set_4byte(nor, true);

        ret |= spi_nor_octal_dtr_enable(nor, true);

        ret |= spi_nor_quad_enable(nor, true);

        ret |= spi_nor_enter_quad(nor, true);

#endif
        if (ret) {
            return ret;
        }

        if (nor->host->ops->training) {
            training_addr = nor->info.sector_size;
            spibus_read(nor, training_addr, training_buf, 32);

            if (0u != memcmp(training_pattern, training_buf, training_len)) {
                if (spibus_erase(nor, training_addr, nor->info.sector_size)) {
                    return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, SPI_NOR_UNREACHABLE);
                }

                if (spibus_write(nor, training_addr, training_pattern, training_len)) {
                    return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, SPI_NOR_UNREACHABLE);
                }
            }

            if (nor->host->clk != NULL) {
                clk_set_rate(nor->host->clk, nor->host->ref_clk_hz);
                ssdk_printf(SSDK_CRIT, "spinor host clock rate is %u!\r\n",
                            clk_get_rate(nor->host->clk));
            }

            ret = nor->host->ops->training(nor, training_addr, training_buf,
                                        training_pattern, training_len);
            if (ret) {
                ret = SPI_NOR_INIT_TRAIN_E;
                ssdk_printf(SSDK_CRIT, "spinor training failed!!");
            }
        }

        /* Don't use async mode for training */
        nor->async_mode = config->async_mode;

    }

    // bind nor to host
    if (!ret) {
        host->total_size += nor->info.size;
        host->nor_tab[nor->cs] = nor;
    }

    return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, ret);
}



/**
 * @brief record the information, and record tansfer not complete
 * @param[in] nor spi norflash ptr
 * @param[in] opt record flash_opt
 * @param[in] addr record flash address
 * @param[in] buf record buf addr
 * @param[in] size record size
 */
static void inline spi_nor_setup_xfer(struct spi_nor *nor,
        enum flash_opt opt,
        flash_addr_t addr, uint8_t *buf,
        flash_size_t size)
{
    nor->xfer_info.opt_type = opt;
    nor->xfer_info.addr = addr;
    nor->xfer_info.buf = buf;
    nor->xfer_info.size = size;
    nor->xfer_info.opt_result = FLASH_OPT_PENDING;
    ssdk_printf(SSDK_INFO, "spinor setup xfer: type = %d, addr = 0x%llx, size = %lld\n", opt, addr, size);
}

/**
 * @brief record the information, and record tansfer complete
 * @param[in] nor spi norflash ptr
 */
static void inline spi_nor_xfer_comp(struct spi_nor *nor)
{
    nor->xfer_info.size = 0;
    nor->xfer_info.opt_result = FLASH_OPT_COMPLETE;
    nor->host->ops->unprepare(nor, (enum spi_nor_ops)nor->xfer_info.opt_type);
    nor->xfer_info.opt_type = FLASH_OPT_NONE;
}

/**
 * @brief record the information, and record tansfer failed
 * @param[in] nor spi norflash ptr
 */
static void inline spi_nor_xfer_error(struct spi_nor *nor)
{
    nor->xfer_info.size = 0;
    nor->xfer_info.opt_result = FLASH_OPT_FAILED;
    nor->host->ops->unprepare(nor, (enum spi_nor_ops)nor->xfer_info.opt_type);
    nor->xfer_info.opt_type = FLASH_OPT_NONE;
    ssdk_printf(SSDK_CRIT, "spi_nor xfer failed\n");
}


static int spibus_read(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                      flash_size_t size)
{
    int ret;

    if (!IS_ALIGNED(addr, 4) || !IS_ALIGNED(buf, 4)) {
        return SPI_NOR_ERR_STATUS(SPI_NOR_READ, SPI_NOR_ADDRESS_INVALID);
    }

    nor->host->ops->prepare(nor, SPI_NOR_OPS_READ);
    spi_nor_setup_xfer(nor, FLASH_OPT_READ, addr, buf, size);
    ret = nor->host->ops->read(nor, addr, buf, size);

    if (ret) {
        spi_nor_xfer_error(nor);
        ret = SPI_NOR_FAIL;
    }
    else if (!nor->async_mode) {
        spi_nor_xfer_comp(nor);
    }

    return SPI_NOR_ERR_STATUS(SPI_NOR_READ, ret);
}

static int spibus_write(struct spi_nor *nor, flash_addr_t addr,
                       const uint8_t *buf,
                       flash_size_t size)
{
    int ret;

    if (!IS_ALIGNED(addr, 4) || !IS_ALIGNED(buf, 4)) {
        return SPI_NOR_ERR_STATUS(SPI_NOR_WRITE, SPI_NOR_ADDRESS_INVALID);
    } else if (!IS_ALIGNED(size, 4)) {
        return SPI_NOR_ERR_STATUS(SPI_NOR_WRITE, SPI_NOR_LENGTH_INVALID);
    }

    nor->host->ops->prepare(nor, SPI_NOR_OPS_WRITE);
    spi_nor_setup_xfer(nor, FLASH_OPT_WRITE, addr, (uint8_t *)buf, size);
    ret = nor->host->ops->write(nor, addr, buf, size);

    if (ret) {
        spi_nor_xfer_error(nor);
        ret = SPI_NOR_FAIL;
    }
    else if (!nor->async_mode) {
        spi_nor_xfer_comp(nor);
    }

    return SPI_NOR_ERR_STATUS(SPI_NOR_WRITE, ret);
}

#ifdef CONFIG_NORFLASH_TESTER
static uint32_t erase_count[SPI_NOR_SECTOR_TYPE_MAX];
#endif

/* the arguments erase length and dst address must 4K alined */
static int spibus_erase(struct spi_nor *nor, flash_addr_t addr,
                       flash_size_t size)
{
    int ret = SPI_NOR_OK_STATUS;

    if (!IS_ALIGNED(addr, nor->info.sector_size)) {
        return SPI_NOR_ERR_STATUS(SPI_NOR_ERASE, SPI_NOR_ADDRESS_INVALID);
    } else if (!IS_ALIGNED(size, nor->info.sector_size)) {
        return SPI_NOR_ERR_STATUS(SPI_NOR_ERASE, SPI_NOR_LENGTH_INVALID);
    }

#ifdef CONFIG_NORFLASH_TESTER
    memset(erase_count, 0, sizeof(erase_count));
#endif

    nor->host->ops->prepare(nor, SPI_NOR_OPS_ERASE);
    spi_nor_setup_xfer(nor, FLASH_OPT_ERASE, addr, NULL, size);

    if (!nor->async_mode) {
        while (nor->xfer_info.size) {
            int i;
            uint32_t count = 0;
            uint32_t prev_index = SPI_NOR_SECTOR_TYPE_MAX;

            for (i = SPI_NOR_SECTOR_TYPE_MAX - 1; i >= 0; i--) {
                if (!(nor->info.erase_proto_list[i])) {
                    continue;  // skip nonsupport proto
                }else if (nor->xfer_info.size < nor->info.erase_map[i].erase_size) {
                    continue;  // skip oversize proto
                } else if (!IS_ALIGNED(nor->xfer_info.addr, nor->info.erase_map[i].erase_size)) {
                    prev_index = i;
                    continue;  // skip unaligned proto
                } else if (prev_index != SPI_NOR_SECTOR_TYPE_MAX) {
                    count = (MIN(ROUNDUP(nor->xfer_info.addr, nor->info.erase_map[prev_index].erase_size)
                            - nor->xfer_info.addr, nor->xfer_info.size)) >> nor->info.erase_map[i].post;
                } else {
                    count = nor->xfer_info.size >> nor->info.erase_map[i].post;
                }

                if (count) {
                    break;
                }
            }

#ifdef CONFIG_NORFLASH_TESTER
            erase_count[i] += count;
            ssdk_printf(SSDK_EMERG, "[SYNC] 4k: %u, 32k: %u, 64k: %u, 128k: %u, 256k: %u\r\n",
                        erase_count[SPI_NOR_SECTOR_4KB],
                        erase_count[SPI_NOR_SECTOR_32KB],
                        erase_count[SPI_NOR_SECTOR_64KB],
                        erase_count[SPI_NOR_SECTOR_128KB],
                        erase_count[SPI_NOR_SECTOR_256KB]);
#endif
            while (count) {
                struct spi_nor_erase_cmd erase_cmd = {
                    .addr = nor->xfer_info.addr,
                    .map = &nor->info.erase_map[i],
                };
                ret = nor->host->ops->complex_erase(nor, &erase_cmd);
                if (ret) {
                    ret = SPI_NOR_FAIL;
                    break;
                }

                /* wait for flash idle */
                ret = spibus_wait_idle(nor);

                if (ret) {
                    ret = SPI_NOR_TIMEOUT;
                    break;
                }

                nor->xfer_info.addr += nor->info.erase_map[i].erase_size;
                nor->xfer_info.size -= nor->info.erase_map[i].erase_size;
                count--;
            }

            if (count || (i < 0)) {
                if (!ret) {
                    ret = SPI_NOR_FAIL;
                }
                break; // Something went wrong, exit
            }
        }

        if (ret) {
            spi_nor_xfer_error(nor);
        }
        else {
            spi_nor_xfer_comp(nor);
        }
    }

    return SPI_NOR_ERR_STATUS(SPI_NOR_ERASE, ret);
}

static int spibus_cancel(struct spi_nor *nor)
{
    int ret = SPI_NOR_OK_STATUS;
    if (nor->host->ops->cancel) {
        ret = nor->host->ops->cancel(nor);
        if (ret) {
            ret = SPI_NOR_FAIL;
        }
    } else {
        ret = SPI_NOR_UNSUPPORT;
    }
    return SPI_NOR_ERR_STATUS(SPI_NOR_CANCEL, ret);
}

/**
 * @brief spi nor erase flash use polling mode
 * @param[in] nor spi norflash ptr
 */
static void spibus_erase_polling(struct spi_nor *nor)
{
    int ret = 0;
    uint8_t flash_status;
    uint32_t sector_size = nor->info.sector_size;

    if (nor->xfer_info.size != 0) {
        ret = spi_nor_get_status(nor, &flash_status);

        if (ret) {
            spi_nor_xfer_error(nor);
        }
        else if (!(flash_status & BIT(0))) {
            uint32_t count = 0;
            int i;
            for (i = SPI_NOR_SECTOR_TYPE_MAX - 1; i >= 0; i--) {
                if (!(nor->info.erase_proto_list[i]) ||
                    !IS_ALIGNED(nor->xfer_info.addr, nor->info.erase_map[i].erase_size)) {
                    continue;  // skip nonsupport&unaligned proto
                }
                count = nor->xfer_info.size >> nor->info.erase_map[i].post;
                if (count) {
                    sector_size = nor->info.erase_map[i].erase_size;
                    break;
                }
            }

            if (count) {
                struct spi_nor_erase_cmd erase_cmd = {
                    .addr = nor->xfer_info.addr,
                    .map = &nor->info.erase_map[i],
                };
                ret = nor->host->ops->complex_erase(nor, &erase_cmd);
                if (ret) {
                    spi_nor_xfer_error(nor);
                } else {

#ifdef CONFIG_NORFLASH_TESTER
                    erase_count[i]++;
                    ssdk_printf(SSDK_EMERG, "[SYNC] 4k: %u, 32k: %u, 64k: %u, 128k: %u, 256k: %u\r\n",
                                erase_count[SPI_NOR_SECTOR_4KB],
                                erase_count[SPI_NOR_SECTOR_32KB],
                                erase_count[SPI_NOR_SECTOR_64KB],
                                erase_count[SPI_NOR_SECTOR_128KB],
                                erase_count[SPI_NOR_SECTOR_256KB]);
#endif

                    nor->xfer_info.size -= sector_size;
                    nor->xfer_info.addr += sector_size;
                }
            } else {
                spi_nor_xfer_error(nor);
            }
        }
    }

    if (nor->xfer_info.size == 0) {
        ret = spi_nor_get_status(nor, &flash_status);

        if (ret) {
            spi_nor_xfer_error(nor);
        }
        else if (!(flash_status & BIT(0))) {
            spi_nor_xfer_comp(nor);
        }
    }
}

static void spibus_main_function(struct spi_nor *nor)
{
    if (!nor->async_mode)
        return;

    if (nor->xfer_info.opt_result == FLASH_OPT_PENDING) {
        if (nor->xfer_info.opt_type != FLASH_OPT_ERASE) {
            /* spi nor host polling handler */
            nor->host->ops->drv_main_function(nor);

            if (nor->xfer_info.opt_result != FLASH_OPT_PENDING) {
                nor->host->ops->unprepare(nor, (enum spi_nor_ops)nor->xfer_info.opt_type);
                nor->xfer_info.opt_type = FLASH_OPT_NONE;
            }
        }
        else {
            spibus_erase_polling(nor);
        }
    }
    else {
        if (nor->xfer_info.opt_type) {
            nor->host->ops->unprepare(nor, (enum spi_nor_ops)nor->xfer_info.opt_type);
            nor->xfer_info.opt_type = FLASH_OPT_NONE;
        }
    }
}

static void spibus_deinit(struct spi_nor *nor)
{
    if (nor->sw_rst) {
        spi_nor_soft_reset(nor);
        udelay(1000u);
    }

}

/**
 * @brief spi nor set 4byte addr mode
 * @param[in] nor spi norflash ptr
 * @param[in] enable spi norflash 4byte mode enable or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int spi_nor_general_set_4byte_addr_mode(struct spi_nor *nor,
        bool enable)
{
    int ret;

    struct spi_nor_cmd byte_cmd = {
        .opcode = enable ? SPINOR_OP_EN4B : SPINOR_OP_EX4B,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    ret = spi_nor_reg_write(nor, &byte_cmd, 0, 0, 0);

    return ret;
}

/**
 * @brief set read dummy for miron spi norflash
 * @param[in] nor spi norflash ptr
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int miron_default_init(struct spi_nor *nor)
{
    int ret;
    uint8_t data[2];

    struct spi_nor_cmd byte_cmd = {
        .opcode = 0x81,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    spibus_write_enable_locked(nor, true);

    data[0] = nor->info.read_dummy;
    ret = spi_nor_reg_write(nor, &byte_cmd, 1, data,
                                 SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret) {
        return ret;
    }

    struct spi_nor_cmd read_cmd = {
        .opcode = 0x85,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 8,
    };

    ret = spi_nor_reg_read(nor, &read_cmd, 1, data,
                                SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret || data[0] != nor->info.read_dummy) {
        ssdk_printf(SSDK_EMERG, "dummy: %d, %d\r\n", nor->info.read_dummy, data[0]);
        return -1;
    }

    return ret;
}

/**
 * @brief  set read dummy for issi spi norflash
 * @param[in] nor spi norflash ptr
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int issi_default_init(struct spi_nor *nor)
{
    int ret;
    uint8_t data[2];

    struct spi_nor_cmd byte_cmd = {
        .opcode = 0xc0,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    spibus_write_enable_locked(nor, true);

    data[0] = nor->info.read_dummy << 3;

#ifdef CONFIG_IS25LP064A_DUMMY_FIXUP
    if (CONFIG_IS25LP064A_DUMMY_FIXUP & (1ul << nor->id)) {
        /* Fix up for IS25LP064A. Refer to descriptions in part.h */
        nor->info.read_dummy = 10;
        data[0] = 0x3 << 3;
    }
#endif

    ret = spi_nor_reg_write(nor, &byte_cmd, 1, data,
                                 SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    return ret;
}


/**
 * @brief spi nor set octal dtr mode for miron spi norflash
 * @param[in] nor spi norflash ptr
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int miron_octal_dtr_enable(struct spi_nor *nor, bool enable)
{
    int ret;
    uint8_t data[3];

    struct spi_nor_cmd byte_cmd = {
        .opcode = 0x81,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    spibus_write_enable_locked(nor, true);

    data[0] = enable ? 0xe7 : 0xff;
    ret = spi_nor_reg_write(nor, &byte_cmd, 0, data,
                                 SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (enable) {
        nor->octal_dtr_en = 1;
        nor->dqs_en = 1;

        struct spi_nor_cmd read_cmd = {
            .opcode = 0x85,
            .addr_bytes = 4,
            .inst_type = 3,
            .dummy = 8,
        };

        ret = spi_nor_reg_read(nor, &read_cmd, 0, data, 2);

        if (ret || data[0] != 0xe7) {
            ssdk_printf(SSDK_EMERG, "I/O mode: %x, %x, ret = %d\r\n", data[0], data[1], ret);
            return -1;
        }
    }

    return ret;
}

/**
 * @brief set quad  mode for issi spi norflash
 * @param[in] nor spi norflash ptr
 * @param[in] enable quad  mode use or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int issi_quad_enable(struct spi_nor *nor, bool enable)
{
    int ret = 0;
    uint8_t data[2] = {0};

    struct spi_nor_cmd r_cmd = {
        .opcode = SPINOR_OP_RDSR,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    ret = spi_nor_reg_read(nor, &r_cmd, 0, data,
                                SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret) return ret;

    if (!(data[0] & (1u << 6))) {
        struct spi_nor_cmd w_cmd = {
            .opcode = SPINOR_OP_WRSR,
            .addr_bytes = 0,
            .inst_type = SNOR_INST_LANS(nor->reg_proto),
            .dummy = 0,
        };


        spibus_write_enable_locked(nor, true);

        data[0] |= (1u << 6);
        ret = spi_nor_reg_write(nor, &w_cmd, 0, data,
                                     SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

        if (ret) {
            return ret;
        }

        ret = spibus_wait_idle_locked(nor);
    }

    return ret;
}

/**
 * @brief set qpi  mode for issi spi norflash
 * @param[in] nor spi norflash ptr
 * @param[in] enable qpi  mode use or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int issi_enter_quad(struct spi_nor *nor, bool enable)
{
    int ret = 0;

    struct spi_nor_cmd q_cmd = {
        .opcode = enable ? 0x35 : 0xf5,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    ret = spi_nor_reg_write(nor, &q_cmd, 0, 0, 0);

    return ret;
}


/**
 * @brief set qpi  mode for giga spi norflash
 * @param[in] nor spi norflash ptr
 * @param[in] enable qpi  mode use or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int giga_enter_quad(struct spi_nor *nor, bool enable)
{
    int ret = 0;

    struct spi_nor_cmd cmd = {
        .opcode = enable ? 0x38 : 0xff,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    ret = spi_nor_reg_write(nor, &cmd, 0, 0, 0);

    //if clock frequency is higher than 104MHz, set 8 dummy clocks
    switch (nor->info.flash_id[1]) {
        case 0x66: /* gd25lt */
        case 0x67: /* gd25lb */
            if (nor->host->ref_clk_hz >= 208000000u) {
                nor->info.status_dummy = 8;
            }

            break;

        default:
            break;
    }

    return ret;
}


/**
 * @brief set read dummy for giga spi norflash
 * @param[in] nor spi norflash ptr
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int giga_default_init(struct spi_nor *nor)
{
    int ret;
    uint8_t data[2];

    struct spi_nor_cmd byte_cmd = {
        .opcode = 0x81,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    spibus_write_enable_locked(nor, true);

    data[0] = nor->info.read_dummy;
    ret = spi_nor_reg_write(nor, &byte_cmd, 1, data,
                                 SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret) {
        return ret;
    }

    struct spi_nor_cmd read_cmd = {
        .opcode = 0x85,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 8,
    };

    ret = spi_nor_reg_read(nor, &read_cmd, 1, data,
                                SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret || data[0] != nor->info.read_dummy) {
        ssdk_printf(SSDK_EMERG, "dummy: %d, %d\r\n", nor->info.read_dummy, data[0]);
        return -1;
    }

    return ret;
}

/**
 * @brief set quad  mode for giga spi norflash
 * @param[in] nor spi norflash ptr
 * @param[in] enable quad  mode use or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int giga_quad_enable(struct spi_nor *nor, bool enable)
{
    int ret = 0;
    uint8_t data[2] = {0};
    uint8_t rdata[2] = {0};

    struct spi_nor_cmd r_cmd = {
        .opcode = 0x35,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    ret = spi_nor_reg_read(nor, &r_cmd, 0, data,
                                SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);


    if (ret) {
        return ret;
    }

    if (!(data[0] & (1u << 1))) {
        struct spi_nor_cmd w_cmd = {
            .opcode = 0x31,
            .addr_bytes = 0,
            .inst_type = SNOR_INST_LANS(nor->reg_proto),
            .dummy = 0,
        };

        spibus_write_enable_locked(nor, true);
        data[0] |= (1u << 1);
        ret = spi_nor_reg_write(nor, &w_cmd, 0, data,
                                     SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);
    }

    ret = spi_nor_reg_read(nor, &r_cmd, 0, rdata,
                                SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret || rdata[0] != 0x02) {
        ssdk_printf(SSDK_CRIT, "I/O mode: %x, %x, ret = %d\r\n", data[0], data[1], ret);
        return -1;
    }

    return ret;
}

/**
 * @brief set 256KB sector size for cypress spi norflash
 * @param[in] nor spi norflash ptr
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int cypress_default_init(struct spi_nor *nor)
{
    int ret = 0;
    uint8_t data[2] = {0};

    /* set erase 256kb */
    struct spi_nor_cmd e_r_cmd = {
        .opcode = 0x65,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 8,
    };

    ret = spi_nor_reg_read(nor, &e_r_cmd, 0x800004, data,
                                SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret) {
        return ret;
    }

    if (!(data[0] & BIT(3))) {
        struct spi_nor_cmd e_w_cmd = {
            .opcode = 0x71,
            .addr_bytes = nor->addr_width,
            .inst_type = SNOR_INST_LANS(nor->reg_proto),
            .dummy = 0,
        };
        spibus_write_enable_locked(nor, true);

        data[0] |= (1 << 3);//256KB
        ret = spi_nor_reg_write(nor, &e_w_cmd, 0x04, data,
                                     SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

        if (ret) {
            return ret;
        }

        ret = spibus_wait_idle_locked(nor);

        if (ret) {
            return ret;
        }

        ret = spi_nor_reg_read(nor, &e_r_cmd, 0x800004, data,
                                    SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

        if (!(data[0] & BIT(3))) {
            ssdk_printf(SSDK_CRIT, "erase 256kb failed, read data = 0x%x\r\n", data[0]);
            return -1;
        }
    }

    return ret;
}


/**
 * @brief set 4byte addr mode size for cypress spi norflash
 * @param[in] nor spi norflash ptr
 * @param[in] enable 4byte addr mode use or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int cypress_set_4byte_addr_mode(struct spi_nor *nor,
        bool enable)
{
    int ret;

    struct spi_nor_cmd byte_cmd = {
        .opcode = enable ? SPINOR_OP_EN4B : 0xb8,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    ret = spi_nor_reg_write(nor, &byte_cmd, 0, 0, 0);

    return ret;
}

/**
 * @brief set octal dtr mode for cypress spi norflash
 * @param[in] nor spi norflash ptr
 * @param[in] enable octal dtr mode use or not
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int cypress_octal_dtr_enable(struct spi_nor *nor, bool enable)
{
    int ret = 0;
    uint8_t data[3] = {0};

    struct spi_nor_cmd r_cmd = {
        .opcode = 0x65,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 8,
    };

    ret = spi_nor_reg_read(nor, &r_cmd, 0x800006, data,
                                SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret) {
        return ret;
    }

    struct spi_nor_cmd w_cmd = {
        .opcode = 0x71,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    spibus_write_enable_locked(nor, true);

    data[2] = enable ? data[0] |= 0x03 : 0;

    ret = spi_nor_reg_write(nor, &w_cmd, 0x800006, data,
                                 SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (enable) {
        nor->octal_dtr_en = 1;
        nor->dqs_en = 1;

        struct spi_nor_cmd read_cmd = {
            .opcode = 0x65,
            .addr_bytes = 4,
            .inst_type = 3,
            .dummy = 8,
        };

        ret = spi_nor_reg_read(nor, &read_cmd, 0x800006, data, 2);

        if (ret || data[0] != data[2]) {
            ssdk_printf(SSDK_EMERG, "I/O mode: %x, %x, %x, ret = %d\r\n", data[0], data[1],
                        data[2], ret);
            return -1;
        }
    }

    return ret;
}
#endif
