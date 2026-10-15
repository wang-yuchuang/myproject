/**
 * @file hyperbus.c
 * @brief hyperbus driver C code.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifdef CONFIG_HYPERBUS_MODE

#include <stdio.h>
#include <compiler.h>
#include <string.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <param.h>
#include <board.h>

#include "driver/spi_nor.h"

#define PROTO(_opcode, _dq) \
        ((_opcode) << SNOR_OPCODE_PROTO_LSB | (_dq))
#define ID_PROTO(dummy, _dq) \
        ((dummy) << SNOR_READID_DUMMY_LSB | (_dq))

#define SPINOR_ID_CAPACITY_OFFSET 2

uint8_t hyperbus_training_pattern[32] __ALIGNED(CONFIG_ARCH_CACHE_LINE) = {
    0x44, 0x1c, 0x39, 0x05, 0xd3, 0x7a, 0x3c, 0x04,
    0x16, 0x42, 0x0c, 0x8b, 0x7d, 0x12, 0x89, 0xa2,
    0xb8, 0xb1, 0xf7, 0xe8, 0xb7, 0x49, 0xca, 0x1c,
    0xaa, 0x9b, 0xf2, 0x7e, 0x01, 0x97, 0x60, 0x8c
};
uint8_t hyperbus_training_buf[32] __ALIGNED(CONFIG_ARCH_CACHE_LINE) = {0};

/* fill hyperflash cmd according to spec */
struct hyperflash_cmd hyper_read_status = {
    .name = "read status",
    .is_read = true,
    .num = 1,
    .ca = {{0x555, 0x70}},
};

struct hyperflash_cmd hyper_read_vol_reg1 = {
    .name = "read vol reg1",
    .is_read = true,
    .num = 3,
    .ca = {{0x555, 0xAA}, {0x2AA, 0x55}, {0x555, 0xC7}},
};

struct hyperflash_cmd hyper_write_vol_reg1 = {
    .name = "write vol reg1",
    .is_read = true,
    .num = 3,
    .ca = {{0x555, 0xAA}, {0x2AA, 0x55}, {0x555, 0x38}},
};

struct hyperflash_cmd hyper_read_vol_reg2 = {
    .name = "read vol reg2",
    .is_read = true,
    .num = 3,
    .ca = {{0x555, 0xAA}, {0x2AA, 0x55}, {0x555, 0xC9}},
};

struct hyperflash_cmd hyper_write_vol_reg2 = {
    .name = "write vol reg2",
    .is_read = false,
    .num = 3,
    .ca = {{0x555, 0xAA}, {0x2AA, 0x55}, {0x555, 0x3A}},
};

struct hyperflash_cmd hyper_read_ecc_reg = {
    .name = "read ecc reg",
    .is_read = true,
    .num = 3,
    .ca = {{0x555, 0xAA}, {0x2AA, 0x55}, {0x555, 0x75}},
};

struct hyperflash_cmd hyper_read_id = {
    .name = "read device id",
    .is_read = true,
    .num = 4,
    .ca = {{0x555, 0xAA}, {0x2AA, 0x55}, {0x555, 0x90}, {0x555, 0x98}},
};

struct hyperflash_cmd hyper_sector_erase = {
    .name = "sector_erase",
    .is_read = false,
    .num = 5,
    .ca = {{0x555, 0xAA}, {0x2AA, 0x55}, {0x555, 0x80}, {0x555, 0xAA}, {0x2AA, 0x55}},
};

struct hyperflash_cmd hyper_word_program = {
    .name = "word_program",
    .is_read = false,
    .num = 3,
    .ca = {{0x555, 0xAA}, {0x2AA, 0x55}, {0x555, 0xA0}},
};

static int s26h_default_init(struct spi_nor *nor);

static int hyperbus_wait_idle(struct spi_nor *nor);
static int hyperbus_write_enable(struct spi_nor *nor, bool enable);
static int hyperbus_init(struct spi_nor *nor, struct spi_nor_host *host,
                 const struct spi_nor_config *config);
static void hyperbus_deinit(struct spi_nor *nor);
static int hyperbus_read(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                 flash_size_t size);
static int hyperbus_write(struct spi_nor *nor, flash_addr_t addr, const uint8_t *buf,
                  flash_size_t size);
static int hyperbus_erase(struct spi_nor *nor, flash_addr_t addr, flash_size_t size);
static int hyperbus_cancel(struct spi_nor *nor);
static void hyperbus_main_function(struct spi_nor *nor);

flash_ops_t hyperbus_ops = {
    .fls_wait_idle= hyperbus_wait_idle,
    .fls_write_enable= hyperbus_write_enable,
    .fls_init= hyperbus_init,
    .fls_deinit= hyperbus_deinit,
    .fls_read= hyperbus_read,
    .fls_write= hyperbus_write,
    .fls_erase= hyperbus_erase,
    .fls_cancel= hyperbus_cancel,
    .fls_main_function= hyperbus_main_function,
};

/* fill flash info, according to different device type */
static struct flash_info hyperbus_ids[] = {
    {
        .name = "default",
        /* The 0x00, 0xff, 0x00, 0xff, is a dummy device ID */
        .flash_id = {0x00, 0xff, 0x00, 0xff},
        .read_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .read_dummy = 16,
        .write_dummy = 0,
        .page_size = 256,
    },
    {
        .name = "hyperram",
        /* The 0x00, 0xff, 0x00, 0xff, is a dummy device ID */
        .flash_id = {0x00, 0xff, 0x00, 0xff},
        .read_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .read_dummy = 5,
        .write_dummy = 5,
    },
    /* cypress */
    {
        .name = "s26hs",
        .flash_id = {0x34, 0x00, 0x7B, 0x00},
        .read_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .sector_size = SPINOR_SECTOR_256K_SIZE,
        .read_dummy = 16,
        .write_dummy = 0,
        .page_size = 256,
        .default_init = s26h_default_init,
    },
    /* cypress */
    {
        .name = "s26hs",
        .flash_id = {0x34, 0x00, 0x7A, 0x00},
        .read_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .sector_size = SPINOR_SECTOR_256K_SIZE,
        .read_dummy = 16,
        .write_dummy = 0,
        .page_size = 256,
        .default_init = s26h_default_init,
    },
    {
        .name = "s26hl",
        .flash_id = {0x34, 0x00, 0x6a, 0x00},
        .read_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .sector_size = SPINOR_SECTOR_256K_SIZE,
        .read_dummy = 16,
        .write_dummy = 0,
        .page_size = 256,
        .default_init = s26h_default_init,
    },
    {
        .name = "s26ks",
        .flash_id = {0x01, 0x00, 0x7E, 0x00},
        .read_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .write_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .erase_proto = PROTO(0u, SNOR_PROTO_8_8_8_DTR),
        .sector_size = SPINOR_SECTOR_256K_SIZE,
        .read_dummy = 16,
        .write_dummy = 0,
        .page_size = 256,
    },
};


static int hyperbus_write_enable(struct spi_nor *nor, bool enable)
{
    struct hyperbus_ca *ca = hyper_word_program.ca;

    nor->host->hyper_ops->set_pre_transaction(nor, false, false, 3u, &ca);

    return SPI_NOR_OK_STATUS;
}

/**
 * @brief get hyperflash current status
 * @param[out] status get hyperflash current status
 * return hyperflash current status
 */
static int spi_nor_get_status(struct spi_nor *nor, uint16_t *status)
{
    int ret;
    uint16_t reg = 0u;
    struct hyperbus_ca *ca = hyper_read_status.ca;

    nor->host->hyper_ops->set_pre_transaction(nor, true, false, 1u, &ca);
    ret = nor->host->hyper_ops->read16(nor, 0u, &reg);
    *status = reg;

    return ret;
}

/**
 * @brief get hyperflash  current ecc status
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
__UNUSED static int spi_nor_get_ecc_status(struct spi_nor *nor)
{
    int ret;
    uint16_t reg = 0u;
    uint16_t aso_exit_data = 0xF0u;
    struct hyperbus_ca *ca = hyper_read_ecc_reg.ca;

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    nor->host->hyper_ops->set_pre_transaction(nor, true, false, 3u, &ca);
    ret = nor->host->hyper_ops->read16(nor, 0u, &reg);
    nor->host->hyper_ops->write16(nor, 0, &aso_exit_data);
    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    return ret;
}

static int hyperbus_wait_idle(struct spi_nor *nor)
{
    int ret = SPI_NOR_OK_STATUS;
    uint16_t flash_status;
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
                            "spi_nor get flash status failed, ret: %d!\r\n", ret);
                break;
            }

        }

        ssdk_printf(SSDK_INFO, "flash_status = 0x%x \r\n", flash_status);

        if ((flash_status & BIT(7)))
            break;

        if (tick_count > 10000) {
            ret = SPI_NOR_TIMEOUT;
            ssdk_printf(SSDK_CRIT, "wait flash idle timeout, ret = %d!\r\n", ret);
            break;
        }

        udelay(1000u);
        tick_count++;
    }

    return SPI_NOR_ERR_STATUS(SPI_NOR_COMMON, ret);
}

/**
 * @brief spi nor read device info
 * 1.use SNOR_PROTO_1_1_1 try to read  device id
 * 2.read flash capacity
 * @param[in] nor spi norflash ptr.
 * @retval flash_info: success
 * @retval NULL: failed
 */
static int spi_nor_id_read(struct spi_nor *nor)
{
    int ret = 0;
    uint8_t id[8];
    struct flash_info *info;

    struct spi_nor_cmd read_cmd = {
        .opcode = 0x9Fu,
        .dummy = 0,
    };

    read_cmd.dummy = (ID_PROTO(0,
                               SNOR_PROTO_1_1_1) >> SNOR_READID_DUMMY_LSB) & 0xff;
    read_cmd.inst_type = SNOR_INST_LANS(ID_PROTO(0, SNOR_PROTO_1_1_1));

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    ret = nor->host->ops->reg_read(nor, &read_cmd, 0, id, 6);
    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    if (ret) return -2;

    ssdk_printf(SSDK_CRIT, "id0: %x, id1: %x, id2: %x\r\n", id[0], id[2],
                id[4]);

    for (uint32_t i = 0; i < ARRAY_SIZE(hyperbus_ids); i++) {
        info = &hyperbus_ids[i];

        if (!memcmp(info->flash_id, id, 4)) {
            memcpy(&(nor->info), info, sizeof(struct flash_info));
            nor->info.size = 1 << id[SPINOR_ID_CAPACITY_OFFSET];
            return 0;
        }
    }

    return -1;
}

/**
 * @brief hyperflash read device info, try read id by SFDP interface
 * 1.read device id
 * 2.read flash capacity
 * @retval 0: success
 * @retval !0: failed
 */
static int hyperflash_id_read(struct spi_nor *nor)
{
    int ret = 0;
    /* Hyper bus device ID is uint16 */
    uint16_t id[4] = {0};
    struct flash_info *info;
    uint16_t aso_exit_data = 0xF0u;
    struct hyperbus_ca *ca = hyper_read_id.ca;
    memcpy(&(nor->info), &hyperbus_ids[0], sizeof(struct flash_info));

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    nor->host->hyper_ops->set_pre_transaction(nor, true, false, 3u, &ca);
    ret = nor->host->hyper_ops->read16(nor, 0x800, id);
    ret = nor->host->hyper_ops->read16(nor, 0x801, id + 1);
    ret = nor->host->hyper_ops->read16(nor, 0x802, id + 2);
    nor->host->hyper_ops->write16(nor, 0, &aso_exit_data);

    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    if (ret)
        return -2;

    ssdk_printf(SSDK_CRIT, "id0: %x, id1: %x, id2: %x\r\n", id[0], id[1],
                id[2]);

    for (uint32_t i = 1; i < ARRAY_SIZE(hyperbus_ids); i++) {
        info = &hyperbus_ids[i];

        if (!memcmp(info->flash_id, id, 4u)) {
            memcpy(&(nor->info), info, sizeof(struct flash_info));
            nor->info.size = 1 << id[SPINOR_ID_CAPACITY_OFFSET];
            return 0;
        }
    }

    return -1;
}


/**
 * @brief hyperflash read device info, try read id by cfi interface
 * 1.read device id
 * 2.read flash capacity
 * @retval 0: success
 * @retval !0: failed
 */
static int hyperflash_cfi_id_read(struct spi_nor *nor)
{
    int ret = 0;
    /* Hyper bus device ID is uint16 */
    uint16_t id[4] = {0};
    struct flash_info *info;
    uint16_t aso_exit_data = 0xF0u;
    struct hyperbus_ca *ca = hyper_read_id.ca;
    memcpy(&(nor->info), &hyperbus_ids[0], sizeof(struct flash_info));

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    nor->host->hyper_ops->set_pre_transaction(nor, true, false, 3u, &ca);
    ret = nor->host->hyper_ops->read16(nor, 0x0, id);
    ret = nor->host->hyper_ops->read16(nor, 0x1, id + 1);
    ret = nor->host->hyper_ops->read16(nor, 0x27, id + 2);
    nor->host->hyper_ops->write16(nor, 0, &aso_exit_data);

    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    if (ret)
        return -2;

    ssdk_printf(SSDK_CRIT, "id0: %x, id1: %x, id2: %x\r\n", id[0], id[1],
                id[2]);

    for (uint32_t i = 1; i < ARRAY_SIZE(hyperbus_ids); i++) {
        info = &hyperbus_ids[i];

        if (!memcmp(info->flash_id, id, 4u)) {
            memcpy(&(nor->info), info, sizeof(struct flash_info));
            nor->info.size = 1 << id[SPINOR_ID_CAPACITY_OFFSET];
            return 0;
        }
    }

    return -1;
}

/**
 * @brief  hyperbus interface enable, enable hyperbus mode
 * @return int
 * @retval 0 success
 * @retval other false.
 */
static int hyperbus_interface_enable(struct spi_nor *nor)
{
    int ret = 0;
    uint8_t data[2] = {0};

    struct spi_nor_cmd r_cmd = {
        .opcode = 0x65,
        .addr_bytes = 3,
        .inst_type = 0,
        .dummy = 8,
    };

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    ret = nor->host->ops->reg_read(nor, &r_cmd, 0x4, data, 1);
    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    if (ret) {
        return ret;
    }

    ssdk_printf(SSDK_CRIT, "legacy x1 CFR4N register value 0x%x\r\n", data[0]);

    struct spi_nor_cmd wr_en_cmd = {
        .opcode = 0x06,
        .addr_bytes = 0,
        .inst_type = 0,
        .dummy = 0,
    };

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    ret = nor->host->ops->reg_write(nor, &wr_en_cmd, 0, 0, 0);
    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    if (ret) {
        return ret;
    }

    struct spi_nor_cmd w_cmd = {
        .opcode = 0x71,
        .addr_bytes = 3,
        .inst_type = 0,
        .dummy = 0,
    };

    data[0] |= BIT(1);

    ssdk_printf(SSDK_CRIT, "enable hyperbus mode\r\n");

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);

    ret = nor->host->ops->reg_write(nor, &w_cmd, 0x4, data, 1);

    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    return ret;
}

static int hyperbus_flash_init(struct spi_nor *nor)
{
    int ret = 0;

    ssdk_printf(SSDK_CRIT, "hyperbus: try read id by legacy x1 interface\r\n");
    ret = spi_nor_id_read(nor);

    if (!ret) {
        hyperbus_interface_enable(nor);
    }

    nor->hyperbus_mode = true;
    nor->reg_proto = SNOR_PROTO_8_8_8_DTR;
    nor->addr_width = 4u;
    nor->octal_dtr_en = 1u;
    nor->dqs_en = true;

    ssdk_printf(SSDK_CRIT, "hyperbus: try read id by cfi interface\r\n");
    ret = hyperflash_cfi_id_read(nor);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "hyperbus: try read id by SFDP interface\r\n");
        ret = hyperflash_id_read(nor);
    }

    ssdk_printf(SSDK_CRIT, "flash type is hyperflash\r\n");
    nor->mem_type = SPI_HYPERFLASH;

    return ret;
}

static int hyperbus_ram_init(struct spi_nor *nor)
{
    nor->hyperbus_mode = true;
    nor->reg_proto = SNOR_PROTO_8_8_8_DTR;
    nor->addr_width = 4u;
    nor->octal_dtr_en = 1u;
    nor->dqs_en = true;

    memcpy(&(nor->info), &hyperbus_ids[1], sizeof(struct flash_info));
    nor->mem_type = SPI_HYPERRAM;
    ssdk_printf(SSDK_CRIT, "flash type is hyperam\r\n");
    /* For hyperram driver not get size info, set 128M */
    nor->info.size = 0x8000000;
    nor->host->hyper_ops->hyperram_en(nor, nor->cs, 5u, 5u);

    return 0;
}

/**
 * @brief entry function for init spi norflash.
 * 1. read the flash id and check whether use hyperbus
 * 2. recognition hyperam, hyperbus mcp and hyperflash device
 * 3. according to different flash info filled and use callback func
 * 4. hyperbus read training
 * @param[in] spi nor ptr.
 * @param[in] spi nor_host config ptr.
 * @param[in] spi nor config config.
 * @return int
 * @retval 0 true
 * @retval other false.
 */
static int hyperbus_init(struct spi_nor *nor, struct spi_nor_host *host,
                      const struct spi_nor_config *config)
{
    int ret = SPI_NOR_OK_STATUS;
    uint32_t training_len = 32;
    flash_addr_t training_addr;

    memset(nor, 0, sizeof(struct spi_nor));

    for (uint8_t i = 0; i < config->cs; i++) {
        if (host->nor_tab[i]) {
            nor->offset_address += host->nor_tab[i]->info.size;
        } else {
            ssdk_printf(SSDK_ERR, "flash cs%d need init firstly\r\n", i);
            return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, SPI_NOR_FAIL);
        }
    }

    nor->ops = &hyperbus_ops;
    nor->id = config->id;
    nor->cs = config->cs;
    nor->baudrate = config->baudrate;
    nor->xfer_mode = config->xfer_mode;
    nor->dev_mode = config->dev_mode;

    nor->host = host;
    nor->sw_rst = config->sw_rst;

    switch (config->mem_type) {
    case SPI_HYPERRAM:
        ret = hyperbus_ram_init(nor);
        break;
    case SPI_HYPERBUS_MCP:
        if (nor->cs == 0u) {
            ret = hyperbus_flash_init(nor);
        } else {
            ret = hyperbus_ram_init(nor);
        }
        break;
    case SPI_AUTO_DETECT:
    default:
        ret = hyperbus_flash_init(nor);
        if (ret == -1) {
            ret = hyperbus_ram_init(nor);
        }
        break;
    }
    if (ret) {
        return ret;
    }

    if (nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE) {
        nor->info.sector_size *= 2u;
        if (nor->mem_type != SPI_HYPERRAM) {
            nor->info.size *= 2u;
        }
    }

    ssdk_printf(SSDK_NOTICE, "flash size: %llx\r\n", nor->info.size);

    /* set dummy ... */
    if (nor->info.default_init) {
        nor->info.default_init(nor);
    }

    if (nor->host->ops->training) {
        training_addr = nor->info.sector_size;
        hyperbus_read(nor, training_addr, hyperbus_training_buf, 32);

        if (0u != memcmp(hyperbus_training_pattern, hyperbus_training_buf, training_len)) {
            if (nor->mem_type != SPI_HYPERRAM) {
                if (hyperbus_erase(nor, training_addr, nor->info.sector_size)) {
                    return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, SPI_NOR_UNREACHABLE);
                }
            }

            if (hyperbus_write(nor, training_addr, hyperbus_training_pattern, training_len)) {
                return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, SPI_NOR_UNREACHABLE);
            }
        }

        if (nor->host->clk != NULL) {
            clk_set_rate(nor->host->clk, nor->host->ref_clk_hz);
            ssdk_printf(SSDK_CRIT, "spinor host clock rate is %u!\r\n",
                        clk_get_rate(nor->host->clk));
        }

        nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
        ret = nor->host->ops->training(nor, training_addr, hyperbus_training_buf,
                                       hyperbus_training_pattern, training_len);
        nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);
        if (ret) {
            ret = SPI_NOR_INIT_TRAIN_E;
            ssdk_printf(SSDK_CRIT, "spinor training failed!!");
        }
    }

    /* Don't use async mode for training */
    nor->async_mode = config->async_mode;

    // bind nor to host
    if (!ret) {
        host->total_size += nor->info.size;
        host->nor_tab[nor->cs] = nor;
    }

    return SPI_NOR_ERR_STATUS(SPI_NOR_INIT, ret);
}

/**
 * @brief record the information, and record tansfer not complete
 * @param[in] opt record flash_opt
 * @param[in] addr record hyperflash address
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
    ssdk_printf(SSDK_INFO,
                "hyperbus setup xfer: type = %d, addr = 0x%llx, size = %lld\r\n", opt, addr,
                size);
}

/* record the information, and record tansfer complete */
static void inline spi_nor_xfer_comp(struct spi_nor *nor)
{
    nor->xfer_info.size = 0;
    nor->xfer_info.opt_result = FLASH_OPT_COMPLETE;
    nor->host->ops->unprepare(nor, (enum spi_nor_ops)nor->xfer_info.opt_type);
    nor->xfer_info.opt_type = FLASH_OPT_NONE;
}

/* record the information, and record tansfer failed */
static void inline spi_nor_xfer_error(struct spi_nor *nor)
{
    nor->xfer_info.size = 0;
    nor->xfer_info.opt_result = FLASH_OPT_FAILED;
    nor->host->ops->unprepare(nor, (enum spi_nor_ops)nor->xfer_info.opt_type);
    nor->xfer_info.opt_type = FLASH_OPT_NONE;
    ssdk_printf(SSDK_CRIT, "spi_nor xfer failed\r\n");
}

static int hyperbus_read(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                      flash_size_t size)
{
    int ret;

    if (!IS_ALIGNED(addr, 4) || !IS_ALIGNED(buf, 4)) {
        return SPI_NOR_ERR_STATUS(SPI_NOR_READ, SPI_NOR_ADDRESS_INVALID);
    }

    nor->host->ops->prepare(nor, SPI_NOR_OPS_READ);
    spi_nor_setup_xfer(nor, FLASH_OPT_READ, addr, buf, size);
    ret = nor->host->hyper_ops->read(nor, addr, buf, size);

    if (ret) {
        spi_nor_xfer_error(nor);
        ret = SPI_NOR_FAIL;
    }
    else if (!nor->async_mode) {
        spi_nor_xfer_comp(nor);
    }

    return SPI_NOR_ERR_STATUS(SPI_NOR_READ, ret);
}

static int hyperbus_write(struct spi_nor *nor, flash_addr_t addr,
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
    ret = nor->host->hyper_ops->write(nor, addr, buf, size);

    if (ret) {
        spi_nor_xfer_error(nor);
        ret = SPI_NOR_FAIL;
    }
    else if (!nor->async_mode) {
        spi_nor_xfer_comp(nor);
    }

    return SPI_NOR_ERR_STATUS(SPI_NOR_WRITE, ret);
}

/* the arguments erase length and dst address must 4K alined */
static int hyperbus_erase(struct spi_nor *nor, flash_addr_t addr,
                       flash_size_t size)
{
    int ret = SPI_NOR_OK_STATUS;
    struct hyperbus_ca *ca;
    uint16_t erase_cmd_data = 0x30u;
    struct hyperbus_host_ops *hyper_ops = nor->host->hyper_ops;

    if (!IS_ALIGNED(addr, nor->info.sector_size)) {
        return SPI_NOR_ERR_STATUS(SPI_NOR_ERASE, SPI_NOR_ADDRESS_INVALID);
    } else if (!IS_ALIGNED(size, nor->info.sector_size)) {
        return SPI_NOR_ERR_STATUS(SPI_NOR_ERASE, SPI_NOR_LENGTH_INVALID);
    }

    nor->host->ops->prepare(nor, SPI_NOR_OPS_ERASE);
    nor->host->ops->cache_flush(nor, addr, size);
    spi_nor_setup_xfer(nor, FLASH_OPT_ERASE, addr, NULL, size);
    nor->xfer_info.size = size;

    if (nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE) {
        nor->xfer_info.addr = nor->xfer_info.addr / 2u;
    }

    if (!nor->async_mode) {
        while (nor->xfer_info.size) {
            ca = hyper_sector_erase.ca;

            hyper_ops->set_pre_transaction(nor, false, false, 5u, &ca);
            /* erase addr need convert to word addr */
            ret = hyper_ops->write16(nor, nor->xfer_info.addr / 2u,
                                     &erase_cmd_data);

            if (ret) {
                ret = SPI_NOR_FAIL;
                break;
            }

            /* wait for flash idle */
            ret = hyperbus_wait_idle(nor);

            if (ret) {
                ret = SPI_NOR_TIMEOUT;
                break;
            }

            nor->xfer_info.addr += nor->info.sector_size;
            nor->xfer_info.size -= nor->info.sector_size;
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

static int hyperbus_cancel(struct spi_nor *nor)
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
static void hyperbus_erase_polling(struct spi_nor *nor)
{
    int ret = 0;
    uint16_t flash_status;
    uint32_t sector_size = nor->info.sector_size;

    if (nor->xfer_info.size != 0) {
        ret = spi_nor_get_status(nor, &flash_status);

        if (ret) {
            spi_nor_xfer_error(nor);
        }
        else if (!(flash_status & BIT(0))) {
            ret = nor->host->ops->erase(nor, nor->xfer_info.addr);

            if (ret) {
                spi_nor_xfer_error(nor);
            }
            else {
                nor->xfer_info.size -= sector_size;
                nor->xfer_info.addr += sector_size;
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

static void hyperbus_main_function(struct spi_nor *nor)
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
            hyperbus_erase_polling(nor);
        }
    }
    else {
        if (nor->xfer_info.opt_type) {
            nor->host->ops->unprepare(nor, (enum spi_nor_ops)nor->xfer_info.opt_type);
            nor->xfer_info.opt_type = FLASH_OPT_NONE;
        }
    }
}

static int s26h_default_init(struct spi_nor *nor)
{
    int ret = 0;
    uint16_t reg = 0u;
    struct hyperbus_ca *ca = hyper_read_vol_reg2.ca;

    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    nor->host->hyper_ops->set_pre_transaction(nor, true, false, 3u, &ca);
    ret = nor->host->hyper_ops->read16(nor, 0u, &reg);
    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

#if CONFIG_HYPERBUS_DIFFERENTIAL_CLK_EN
    /* Enable differential clock mode */
    reg &= ~BIT(0);
#endif
#if CONFIG_HYPERBUS_INCREMENTAL_PROGRAM_EN
    /*
     * If 2bit ECC detect enable, program one page two times without erase
     * will result in a program error.
    */
    reg |= BIT(5);
#endif
    ca = hyper_write_vol_reg2.ca;
    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    nor->host->hyper_ops->set_pre_transaction(nor, false, false, 3u, &ca);
    ret = nor->host->hyper_ops->write16(nor, 0u, &reg);
    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);

    ca = hyper_read_vol_reg2.ca;
    nor->host->ops->prepare(nor, SPI_NOR_OPS_LOCK);
    nor->host->hyper_ops->set_pre_transaction(nor, true, false, 3u, &ca);
    ret = nor->host->hyper_ops->read16(nor, 0u, &reg);
    nor->host->ops->unprepare(nor, SPI_NOR_OPS_LOCK);
    ssdk_printf(SSDK_INFO, "cypress vol regster2 0x%x\r\n", reg);

    return ret;
}

static void hyperbus_deinit(struct spi_nor *nor)
{
    return;
}

#endif
