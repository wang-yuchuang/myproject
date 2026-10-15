
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <param.h>
#include <board.h>
#include <arch/barriers.h>
#include <driver/spi_nor.h>
#include <irq_num.h>
#include <regs_base.h>
#include <rstgen_cfg.h>
#include <clock_cfg.h>
#include <driver/clk.h>
#include <driver/kunlun/reset/sdrv_reset_drv.h>
#include <scr_cfg.h>
#include <driver/scr.h>
#include <scr_hw.h>
#include <driver/dma.h>

#include "CLI.h"
#include "validation.h"

extern reset_sig_t rstsig_xspi1_mst_core0;
extern reset_sig_t rstsig_xspi1_mst_core1;

static int validation_flash_init(void*);
static int validation_mt35xu_init(struct spi_nor *nor);
static int validation_gd25lb_init(struct spi_nor *nor);
static const char* get_validation_case_name(validation_case_type_t type);
static int validation_setup(validation_case_type_t type);

static validation_context_t v_context;

static struct cospi_config default_ospi_config[] = {
#ifdef OSPI1_BASE
    [0] = {
        .id = 1,
        .irq = OSPI1_O_INTERRUPT_NUM,
        .apb_base = APB_OSPI1_BASE,
        .ahb_base = OSPI1_BASE,
    },
#else
    [0] = { 0 },
#endif
#ifdef OSPI2_BASE
    [1] = {
        .id = 2,
        .irq = OSPI2_O_INTERRUPT_NUM,
        .apb_base = APB_OSPI2_BASE,
        .ahb_base = OSPI2_BASE,
    },
#else
    [1] = { 0 },
#endif
};

static struct xspi_config default_xspi_config[] = {
#ifdef XSPI1A_BASE
    [0] = {
        .id = 1,
        .irq = XSPI1_IRQ_0_NUM,
        .apb_base = APB_XSPI1A_BASE,
        .direct_base = XSPI1A_BASE,
    },
#else
    [0] = { 0 },
#endif
#ifdef XSPI1B_BASE
    [1] = {
        .id = 2,
        .irq = XSPI1_IRQ_1_NUM,
        .apb_base = APB_XSPI1B_BASE,
        .direct_base = XSPI1B_BASE,
    },
#else
    [1] = { 0 },
#endif
};

static struct spi_nor_config default_config = {
    .id = 0,
    .cs = 0,
    .baudrate = 20000000,
    /* SPI_NOR_XFER_POLLING_MODE | SPI_NOR_XFER_INTERRUPT_MODE | SPI_NOR_XFER_DMA_MODE  */
    .xfer_mode = SPI_NOR_XFER_POLLING_MODE,
    .dev_mode = 0,
    .sw_rst = false, /* config soft reset spi norflash */
    .async_mode = false,
    .hyperbus_mode = false,
    .flash_init = validation_flash_init,
};

static validation_flash_info_t spibus_flash_table[] = {
    /* miron */
    {
        .name = "mt35xu",
        .flash_id = {0x2c, 0x5b},
        .init = validation_mt35xu_init,
    },
    /* giga */
    {
        .name = "gd25lb",
        .flash_id = {0xc8, 0x67},
        .init = validation_gd25lb_init,
    },
};

static void xspi_lockstep_enable(bool flag, uint8_t id)
{
    scr_signal_t signal = SCR_SAFETY__xspi_src_cfg_lockstep_mode;
    struct reset_sig *xspi_porta = NULL, *xspi_portb = NULL;

    switch (id) {
    case 1:
        xspi_porta = &rstsig_xspi1_mst_core0;
        if (flag) {
            xspi_portb = &rstsig_xspi1_mst_core1;
        }
        break;
    case 2:
        xspi_portb = &rstsig_xspi1_mst_core1;
        break;
    default:
        ssdk_printf(SSDK_CRIT, "xspi %d unsupport lockstep mode", id);
        return;
    }

    if (xspi_portb != NULL) {
        reset_ctl_assert(xspi_portb);
    }
    if (xspi_porta != NULL) {
        reset_ctl_assert(xspi_porta);
    }
    ISB;
    udelay(10);
    scr_set(&g_scr_safety, &signal, !flag);
    udelay(10);
    ISB;
    if (xspi_porta != NULL) {
        reset_ctl_deassert(xspi_porta);
    }
    if (xspi_portb != NULL) {
        reset_ctl_deassert(xspi_portb);
    }
}

static void xspi_parallel_enable(bool flag, uint8_t id)
{
    scr_signal_t signal = SCR_SAFETY__xsp_src_cfg_parallel_mode;
    reset_sig_t *xspi_porta = NULL, * xspi_portb = NULL;

    switch (id) {
    case 1:
        xspi_porta = &rstsig_xspi1_mst_core0;
        if (flag) {
            xspi_portb = &rstsig_xspi1_mst_core1;
        }
        break;
    case 2:
        xspi_portb = &rstsig_xspi1_mst_core1;
        break;
    default:
        ssdk_printf(SSDK_CRIT, "xspi %d unsupport parallel mode", id);
        return;
    }

    if (xspi_portb != NULL) {
        reset_ctl_assert(xspi_portb);
    }
    if (xspi_porta != NULL) {
        reset_ctl_assert(xspi_porta);
    }
    ISB;
    udelay(10);
    scr_set(&g_scr_safety, &signal, flag);
    udelay(10);
    ISB;
    if (xspi_porta != NULL) {
        reset_ctl_deassert(xspi_porta);
    }
    if (xspi_portb != NULL) {
        reset_ctl_deassert(xspi_portb);
    }
}

static int validation_xspi_host_init(struct xspi_config* host_config) {
    int ret = -1;
    // Dynamic parameter assignment
    host_config->crc_en = v_context.crc_en;
    host_config->no_cache = v_context.no_cache;

#ifdef CONFIG_XSPI
    clk_node_t *clk;
    if (host_config->id == 1) {
        clk = CLK_NODE(g_ckgen_ip_xspi1a);
        reset_ctl_reset(&rstsig_xspi1_mst_core0);
    } else if (host_config->id == 2) {
        clk = CLK_NODE(g_ckgen_ip_xspi1b);
        reset_ctl_reset(&rstsig_xspi1_mst_core1);
    }
    // xspi_lockstep_enable(default_config.dev_mode == SPI_NOR_DEV_LOCKSTEP_MODE,
    //     host_config->id);
    // xspi_parallel_enable(default_config.dev_mode == SPI_NOR_DEV_PARALLEL_MODE,
    //     host_config->id);
    host_config->ref_clk = clk_get_rate(clk);
    ret = xspi_host_init(&v_context.spi_nor_host, &v_context.xspi_data, host_config);
    v_context.spi_nor_host.clk = clk;
    // clk_set_rate(clk, config.baudrate);
    ssdk_printf(SSDK_CRIT, "ospi clock rate is %u!\n", clk_get_rate(clk));
#endif
    return ret;
}

static int validation_ospi_host_init(struct cospi_config* host_config) {
    int ret = -1;
#ifdef CONFIG_OSPI
    clk_node_t *clk;
    switch (host_config->id) {
    case 1:
        clk = CLK_NODE(g_ckgen_ip_ospi1);
        reset_ctl_reset(&rstsig_ospi1);
        break;
    case 2:
        clk = CLK_NODE(g_ckgen_ip_ospi2);
        reset_ctl_reset(&rstsig_ospi2);
        break;
    default:
        return -1;
    }

    host_config->ref_clk = clk_get_rate(clk);
    ret = cospi_host_init(&v_context.spi_nor_host, &v_context.ospi_data, host_config);
    v_context.spi_nor_host.clk = clk;

    // clk_set_rate(clk, config.baudrate);
    ssdk_printf(SSDK_CRIT, "ospi clock rate is %u!\n", clk_get_rate(clk));
#endif
    return ret;
}

static int validation_spi_nor_soft_reset(struct spi_nor *nor) {
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

static dma_chan_t *chan = NULL;
extern dma_dev_t dma1;

static void validation_dma_transfer_done(uint32_t status, uint32_t param,
            void *context) {
    // struct spi_nor *nor = context;
    // struct cospi_pdata *cospi = nor->host->priv_data;
    ssdk_printf(SSDK_ERR, "DMA transfer complete?(%d)\n", status);
}

static void validation_ospi_dma_config(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                            bool is_read_flag, uint32_t len, uint32_t burst_width, uint32_t max_burst) {
    struct dma_dev_cfg dma_cfg = { 0 };
    struct cospi_pdata *cospi = nor->host->priv_data;

    ssdk_printf(SSDK_INFO, "burst is %d, width is %d\n", max_burst, burst_width);

    chan = dma_req_chan(&dma1);
    if (chan == NULL) {
        ssdk_printf(SSDK_ERR, "Channel request error\n");
        return;
    }

    if (is_read_flag) {
        dma_cfg.direction = DMA_DEV2MEM;
        dma_cfg.src_addr = (addr_t)cospi->ahb_base + cospi->trigger_address;
        dma_cfg.dst_addr = (addr_t)buf;
    } else {
        dma_cfg.direction = DMA_MEM2DEV;
        dma_cfg.src_addr = (addr_t)buf;
        dma_cfg.dst_addr = (addr_t)cospi->ahb_base + cospi->trigger_address;
    }
    dma_cfg.src_addr_width = burst_width;
    dma_cfg.dst_addr_width = burst_width;
    dma_cfg.src_maxburst = max_burst;
    dma_cfg.dst_maxburst = max_burst;
    chan->context = nor;
    chan->irq_cb = validation_dma_transfer_done;
    dma_config_chan(chan, &dma_cfg);
    if (false == dma_prep_dev(chan, (void *)buf, len, DMA_INTERRUPT)) {
        ssdk_printf(SSDK_ERR, "Dma prep error\n");
        return;
    }
    dma_start(chan);
}

static void validation_xspi_dma_config(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                            bool is_read_flag, uint32_t len, uint32_t burst_width, uint32_t max_burst) {
    struct dma_dev_cfg dma_cfg = { 0 };
    struct xspi_pdata *xspi = nor->host->priv_data;
    ssdk_printf(SSDK_INFO, "burst is %d, width is %d\n", max_burst, burst_width);

    chan = dma_req_chan(&dma1);
    if (chan == NULL) {
        ssdk_printf(SSDK_ERR, "Channel request error\n");
        return;
    }

    if (is_read_flag) {
        dma_cfg.direction = DMA_DEV2MEM;
        dma_cfg.src_addr = (addr_t)xspi->apb_base + 0x380u;
        dma_cfg.dst_addr = (addr_t)buf;
    } else {
        dma_cfg.direction = DMA_MEM2DEV;
        dma_cfg.src_addr = (addr_t)buf;
        dma_cfg.dst_addr = (addr_t)xspi->apb_base + 0x280u;
    }
    dma_cfg.src_addr_width = burst_width;
    dma_cfg.dst_addr_width = burst_width;
    dma_cfg.src_maxburst = max_burst;
    dma_cfg.dst_maxburst = max_burst;
    chan->context = nor;
    chan->irq_cb = validation_dma_transfer_done;
    dma_config_chan(chan, &dma_cfg);
    if (false == dma_prep_dev(chan, (void *)buf, len, DMA_INTERRUPT)) {
        ssdk_printf(SSDK_ERR, "Dma prep error\n");
        return;
    }
    dma_start(chan);
}

static void validation_dma_stop(struct spi_nor *nor) {
    if (chan) {
        uint32_t time_out = 0;
        while ((time_out++ < 4000u) && (DMA_COMPLETED != dma_get_chan_status(chan))) {
            udelay(10);
        }
        dma_stop(chan);
        dma_free_chan(chan);
    }
}


static int validation_spi_nor_write_enable(struct spi_nor *nor, bool enable) {
    int ret;

    struct spi_nor_cmd wr_en_cmd = {
        .opcode = enable ? SPINOR_OP_WREN : SPINOR_OP_WRDI,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    ret = spi_nor_reg_write(nor, &wr_en_cmd, 0, 0, 0);

    return ret;
}

static int validation_default_init(struct spi_nor *nor) {
    int ret;
    uint8_t data[3];

    switch (v_context.proto) {
    case SNOR_PROTO_1_1_1:
        nor->info.read_dummy = 8;
        nor->info.read_proto = PROTO(SPINOR_OP_READ_FAST, v_context.proto);
        nor->info.write_proto = PROTO(SPINOR_OP_PP, v_context.proto);
        v_context.addr_width = 3;
        break;
    case SNOR_PROTO_1_1_4:
        nor->info.read_dummy = 8;
        nor->info.read_proto = PROTO(SPINOR_OP_READ_1_1_4, v_context.proto);
        nor->info.write_proto = PROTO(SPINOR_OP_PP_1_1_4, v_context.proto);
        v_context.addr_width = 3;
        break;
    case SNOR_PROTO_1_4_4:
        nor->info.read_dummy = 6;
        nor->info.read_proto = PROTO(SPINOR_OP_READ_1_4_4, v_context.proto);
        nor->info.write_proto = PROTO(SPINOR_OP_PP_1_4_4_GIGA, v_context.proto);
        v_context.addr_width = 3;
        break;
    case SNOR_PROTO_1_4_4_DTR:
        nor->info.read_dummy = 6;
        nor->info.read_proto = PROTO(SPINOR_OP_READ_1_4_4_DTR, v_context.proto);
        nor->info.write_proto = PROTO(SPINOR_OP_PP_1_4_4_GIGA, SNOR_PROTO_1_4_4);
        v_context.addr_width = 3;
        break;
    case SNOR_PROTO_4_4_4_DTR: // 4s-4d-4d
        nor->info.read_dummy = 6;
        nor->info.read_proto = PROTO(SPINOR_OP_READ_1_4_4_DTR, v_context.proto);
        nor->info.write_proto = PROTO(SPINOR_OP_PP_1_4_4_GIGA, v_context.proto);
        v_context.addr_width = 3;
        break;
    case SNOR_PROTO_1_1_8:
        nor->info.read_dummy = 8;
        nor->info.read_proto = PROTO(SPINOR_OP_READ_1_1_8, v_context.proto);
        nor->info.write_proto = PROTO(SPINOR_OP_PP_1_1_8, v_context.proto);
        v_context.addr_width = 3;
        break;
    case SNOR_PROTO_1_8_8:
        nor->info.read_dummy = 20;
        nor->info.read_proto = PROTO(SPINOR_OP_READ_1_8_8, v_context.proto);
        nor->info.write_proto = PROTO(SPINOR_OP_PP_1_8_8, v_context.proto);
        v_context.addr_width = 3;
        break;
    case SNOR_PROTO_1_8_8_DTR:
        nor->info.read_dummy = 20;
        nor->info.read_proto = PROTO(SPINOR_OP_READ_1_8_8_DTR_4B, v_context.proto);
        nor->info.write_proto = PROTO(SPINOR_OP_PP_1_8_8, SNOR_PROTO_1_8_8);
        v_context.addr_width = 4;
        break;
    case SNOR_PROTO_8_8_8_DTR:
        nor->info.read_dummy = 20;
        nor->info.read_proto = PROTO(SPINOR_OP_READ_1_8_8, v_context.proto);
        nor->info.write_proto = PROTO(SPINOR_OP_PP_1_8_8, v_context.proto);
        v_context.addr_width = 4;
        break;
    default:
        return -1;
    }

    validation_spi_nor_write_enable(nor, true);

    struct spi_nor_cmd byte_cmd = {
        .opcode = 0x81,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

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
        ssdk_printf(SSDK_EMERG, "dummy set error: %d, %d\r\n", nor->info.read_dummy, data[0]);
    }

    return ret;
}

static int validation_spi_nor_4B_addr_enable(struct spi_nor *nor, bool enable) {
    int ret;
    uint8_t data[2] = { 0 };

    struct spi_nor_cmd read_cmd = {
        .opcode = SPINOR_OP_RDFSR,
        .addr_bytes = 0,
        .dummy = nor->info.status_dummy,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = spi_nor_reg_read(nor, &read_cmd, 0, data, SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);
    if (ret) {
        ssdk_printf(SSDK_EMERG, "read flag status error: %d\r\n", ret);
        return ret;
    } else if ((data[0] & BIT(0)) == !!enable) {
        ssdk_printf(SSDK_EMERG, "Enter %d-byte address mode already\r\n", enable ? 4 : 3);
    } else {
        struct spi_nor_cmd byte_cmd = {
            .opcode = enable ? SPINOR_OP_EN4B : SPINOR_OP_EX4B,
            .addr_bytes = 0,
            .inst_type = SNOR_INST_LANS(nor->reg_proto),
            .dummy = SNOR_INST_LANS(nor->reg_proto) == 3 ? 8 : 0,
        };

        validation_spi_nor_write_enable(nor, true);

        ret = spi_nor_reg_write(nor, &byte_cmd, 0, 0, 0);
    }
    if (!ret) {
        nor->addr_width = enable ? 4 : 3;
    }
    ssdk_printf(SSDK_EMERG, "%d-byte address mode enter %s\r\n",
                enable ? 4 : 3, ret ? "error" : "success");
    return ret;
}

static int validation_miron_octal_dtr_enable(struct spi_nor *nor, bool octal_ddr_en, bool dqs_en) {
    int ret = 0;
    uint8_t data[3];
    uint8_t io_mode_cmd;
    struct spi_nor_cmd w_cmd = {
        .opcode = 0x81,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };
    struct spi_nor_cmd r_cmd = {
        .opcode = 0x85,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 8,
    };


    if (octal_ddr_en) {
        io_mode_cmd = (dqs_en) ? 0xE7 : 0xC7;
    } else {
        io_mode_cmd = (dqs_en) ? 0xFF : 0xDF;
    }

    ret |= spi_nor_reg_read(nor, &r_cmd, 0, data, SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);
    if (!ret && data[0] == io_mode_cmd) {
        ssdk_printf(SSDK_EMERG, "I/O mode set already: %x, ret = %d\r\n", data[0], ret);
        return ret;
    }

    validation_spi_nor_write_enable(nor, true);

    data[0] = io_mode_cmd;
    ret |= spi_nor_reg_write(nor, &w_cmd, 0, data,
                                 SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    nor->octal_dtr_en = octal_ddr_en;
    nor->dqs_en = dqs_en;
    if (octal_ddr_en) {
        nor->reg_proto = SNOR_PROTO_8_8_8_DTR;
        nor->addr_width = 4;
    } else {
        nor->reg_proto = SNOR_PROTO_1_1_1;
    }

    r_cmd.addr_bytes = nor->addr_width,
    r_cmd.inst_type = SNOR_INST_LANS(nor->reg_proto),
    ret |= spi_nor_reg_read(nor, &r_cmd, 0, data, SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);
    if (ret || data[0] != io_mode_cmd) {
        ssdk_printf(SSDK_EMERG, "I/O mode set error: %x(mode) != %x(data), ret = %d\r\n", io_mode_cmd, data[0], ret);
    } else {
        ssdk_printf(SSDK_EMERG, "I/O mode set success %x(data), ret = %d\r\n", data[0], ret);
    }

    return ret;
}

static int validation_miron_xip_enter(struct spi_nor *nor) {
    int ret = 0;
    uint8_t data[2];

    struct spi_nor_cmd byte_cmd = {
        .opcode = 0x81,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };
    validation_spi_nor_write_enable(nor, true);
    data[0] = 0xFE;
    ret = spi_nor_reg_write(nor, &byte_cmd, 0x06, data,
                                 SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret) {
        ssdk_printf(SSDK_EMERG, "write reg6 fail %d.\r\n", ret);
        return ret;
    }
    udelay(1000);
    struct spi_nor_cmd read_cmd = {
        .opcode = 0x85,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 8,
    };

    ret = spi_nor_reg_read(nor, &read_cmd, 0x06, data,
                                SNOR_INST_LANS(nor->reg_proto) == 3 ? 2 : 1);

    if (ret || data[0] != 0xFE) {
        ssdk_printf(SSDK_EMERG, "read back reg6 [%d] is not FE.\r\n", data[0]);
        return -1;
    } else {
        ssdk_printf(SSDK_EMERG, "XIP mode set success %x(data), ret = %d\r\n", data[0], ret);
    }
    return ret;
}

static int validation_giga_quad_dtr_enable(struct spi_nor *nor, bool quad_ddr_en, bool dqs_en) {
    int ret = 0;

    struct spi_nor_cmd cmd = {
        .opcode = quad_ddr_en ? 0x38 : 0xff,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };

    ret = spi_nor_reg_write(nor, &cmd, 0, 0, 0);
    nor->dqs_en = dqs_en;
    if (quad_ddr_en) {
        nor->reg_proto = SNOR_PROTO_4_4_4;
    } else {
        nor->reg_proto = SNOR_PROTO_1_1_1;
    }

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

static int validation_mt35xu_init(struct spi_nor *nor) {
    int ret = 0;

    if ((v_context.proto != SNOR_PROTO_1_1_1)
            && (v_context.proto != SNOR_PROTO_1_1_8)
            && (v_context.proto != SNOR_PROTO_1_8_8)
            && (v_context.proto != SNOR_PROTO_1_8_8_DTR)
            && (v_context.proto != SNOR_PROTO_8_8_8_DTR)) {
        ssdk_printf(SSDK_CRIT, "%s do not support this proto: %x\r\n",
            nor->info.name, v_context.proto);
        return -1;
    }

    //ret |= validation_spi_nor_soft_reset(nor);

    if (v_context.proto != SNOR_PROTO_8_8_8_DTR) { // is octal?
        ret |= validation_spi_nor_4B_addr_enable(nor, v_context.proto & SNOR_DTR_PROTO);
    } else {
        // octal automatically enters the 4-byte address mode on the basis of 3-byte address mode
        ret |= validation_spi_nor_4B_addr_enable(nor, 0);
    }

    ret |= validation_default_init(nor);

    ret |= validation_miron_octal_dtr_enable(nor, v_context.proto == SNOR_PROTO_8_8_8_DTR, v_context.dsp_en);

    return ret;
}

static int validation_gd25lb_init(struct spi_nor *nor) {
    int ret = 0;

    if ((v_context.proto != SNOR_PROTO_1_1_1)
            && (v_context.proto != SNOR_PROTO_1_1_4)
            && (v_context.proto != SNOR_PROTO_1_4_4)
            && (v_context.proto != SNOR_PROTO_1_4_4_DTR)
            && (v_context.proto != SNOR_PROTO_4_4_4_DTR)) {
        ssdk_printf(SSDK_CRIT, "%s do not support this proto: %x\r\n",
            nor->info.name, v_context.proto);
        return -1;
    }

    //ret |= validation_spi_nor_soft_reset(nor);

    ret |= validation_spi_nor_4B_addr_enable(nor, nor->info.size > 0x1000000);

    ret |= validation_default_init(nor);

    ret |= validation_giga_quad_dtr_enable(nor, SNOR_INST_LANS(v_context.proto) == 2, v_context.dsp_en);

    return ret;
}

static int validation_setup(validation_case_type_t type) {
    int ret = 0;

    memset(&v_context, 0, sizeof(validation_context_t));
    v_context.proto = SNOR_PROTO_1_1_1;
    v_context.type = type;
    v_context.addr_width = 3;
    v_context.name = get_validation_case_name(type);

    switch (type) {
    case XSPI_MASTER_1S_1S_1S_3B:
        v_context.proto = SNOR_PROTO_1_1_1;
        v_context.addr_width = 3;
        break;
    case XSPI_MASTER_1S_1S_2S:
        v_context.proto = SNOR_PROTO_1_1_2;
        break;
    case XSPI_MASTER_1S_2S_2S:
        v_context.proto = SNOR_PROTO_1_2_2;
        break;
    case XSPI_MASTER_1S_2D_2D:
        v_context.proto = SNOR_PROTO_1_2_2_DTR;
        break;
    case XSPI_MASTER_1S_1S_4S:
        v_context.proto = SNOR_PROTO_1_1_4;
        break;
    case XSPI_MASTER_1S_4S_4S:
        v_context.proto = SNOR_PROTO_1_4_4;
        break;
    case XSPI_MASTER_1S_4D_4D:
        v_context.proto = SNOR_PROTO_1_4_4_DTR; // 1s-4d-4d
        break;
    case XSPI_MASTER_4S_4D_4D:
        v_context.proto = SNOR_PROTO_4_4_4_DTR; // 4s-4d-4d;
        break;
    case XSPI_MASTER_1S_1S_8S:
        v_context.proto = SNOR_PROTO_1_1_8;
        break;
    case XSPI_MASTER_1S_8S_8S:
        v_context.proto = SNOR_PROTO_1_8_8;
        break;
    case XSPI_MASTER_1S_8D_8D:
        v_context.proto = SNOR_PROTO_1_8_8_DTR;
        break;
    case XSPI_MASTER_8D_8D_8D_DQS:
        v_context.proto = SNOR_PROTO_8_8_8_DTR;
        v_context.dsp_en = 1;
        break;
    case XSPI_MASTER_8D_8D_8D:
        v_context.proto = SNOR_PROTO_8_8_8_DTR;
        break;
    case XSPI_MASTER_8D_8D_8D_NOCACHE:
        v_context.no_cache = 1;
        v_context.proto = SNOR_PROTO_8_8_8_DTR;
        break;
    case XSPI_MASTER_8D_8D_8D_PARALLEL:
        v_context.dev_mode = SPI_NOR_DEV_PARALLEL_MODE;
        v_context.proto = SNOR_PROTO_8_8_8_DTR;
        break;
    case XSPI_MASTER_8D_8D_8D_LOCKSTEP_LOOP:
        v_context.dev_mode = SPI_NOR_DEV_LOCKSTEP_MODE;
        v_context.proto = SNOR_PROTO_8_8_8_DTR;
        break;
    case XSPI_MASTER_8D_8D_8D_LOCKSTEP_SIGNAL:
        v_context.dev_mode = SPI_NOR_DEV_LOCKSTEP_MODE;  // SPI_NOR_DEV_SIGNAL_LOCKSTEP_MODE;
        v_context.proto = SNOR_PROTO_8_8_8_DTR;
        break;
    case XSPI_MASTER_HYPERFLASH: // AUTO detect this mode
    case XSPI_MASTER_HYPERRAM: // AUTO detect this mode
        break;
    case XSPI_MASTER_MODE3:
        break;
    case XSPI_MASTER_CRC:
        v_context.crc_en = 1;
        break;
    case XSPI_MASTER_WP:
        break;
    case XSPI_MASTER_AUTO_POLLING:
        break;
    case XSPI_MASTER_IO_TIMING:
        break;
    case XSPI_MASTER_DIFF_CLOCK: // hyperflash
        break;
    case XSPI_MASTER_PARA_FIFO_SIZE:
        v_context.dev_mode = SPI_NOR_DEV_PARALLEL_MODE;
        break;
    case XSPI_MASTER_INDIRECT_AXI_MODE:
    case XSPI_MASTER_HYPERRAM_LOCKSTEP:
    case XSPI_MASTER_HYPERFLASH_LOCKSTEP:
        v_context.dev_mode = SPI_NOR_DEV_LOCKSTEP_MODE;
        break;
    case XSPI_MASTER_HYPERRAM_PARALLEL:
    case XSPI_MASTER_HYPERFLASH_PARALLEL:
        v_context.dev_mode = SPI_NOR_DEV_PARALLEL_MODE;
        break;
    case XSPI_MASTER_PARALLEL_CRC:
        v_context.dev_mode = SPI_NOR_DEV_PARALLEL_MODE;
        v_context.crc_en = 1;
        break;
    case XSPI_MASTER_LOCKSTEP_READ_ERROR:
        v_context.dev_mode = SPI_NOR_DEV_LOCKSTEP_MODE;
        break;
    case XSPI_MASTER_SLV_XIP_MODE:
        v_context.proto = SNOR_PROTO_1_8_8_DTR;
        v_context.slv_xip_mode = true;
        break;
    default:
        break;
    }

    return ret;
}

static const char* get_validation_case_name(validation_case_type_t type) {
    switch (type) {
    case XSPI_MASTER_1S_1S_1S_3B: return "xspi|ospi master 1s-1s-1s_3B";
    case XSPI_MASTER_1S_1S_2S: return "xspi|ospi master 1s-1s-1s";
    case XSPI_MASTER_1S_2S_2S: return "xspi|ospi master 1s-1s-2s";
    case XSPI_MASTER_1S_2D_2D: return "xspi|ospi master 1s-2d-2d";
    case XSPI_MASTER_1S_1S_4S: return "xspi|ospi master 1s-1s-4s";
    case XSPI_MASTER_1S_4S_4S: return "xspi|ospi master 1s-4s-4s";
    case XSPI_MASTER_1S_4D_4D: return "xspi|ospi master 1s-4d-4d";
    case XSPI_MASTER_4S_4D_4D: return "xspi|ospi master 4s-4d-4d";
    case XSPI_MASTER_1S_1S_8S: return "xspi|ospi master 1s-1s-8s";
    case XSPI_MASTER_1S_8S_8S: return "xspi|ospi master 1s-8s-8s";
    case XSPI_MASTER_1S_8D_8D: return "xspi|ospi master 1s-8d-8d";
    case XSPI_MASTER_8D_8D_8D_DQS: return "xspi|ospi master 8d-8d-8d_DQS";
    case XSPI_MASTER_8D_8D_8D: return "xspi|ospi master 8d-8d-8d";
    case XSPI_MASTER_8D_8D_8D_NOCACHE: return "xspi|ospi master 8d-8d-8d_NOCACHE";
    case XSPI_MASTER_8D_8D_8D_PARALLEL: return "xspi|ospi master 8d-8d-8d parallel";
    case XSPI_MASTER_8D_8D_8D_LOCKSTEP_LOOP: return "xspi|ospi master 8d-8d-8d lockstep loop";
    case XSPI_MASTER_8D_8D_8D_LOCKSTEP_SIGNAL: return "xspi|ospi master 8d-8d-8d lockstep signal";
    case XSPI_MASTER_HYPERFLASH: return "xspi|ospi master hyperflash";
    case XSPI_MASTER_HYPERRAM: return "xspi|ospi master hyperram";
    case XSPI_MASTER_MODE3: return "xspi|ospi master spi mode3";
    case XSPI_MASTER_CRC: return "xspi|ospi master crc checking";
    case XSPI_MASTER_WP: return "xspi|ospi master write protect";
    case XSPI_MASTER_AUTO_POLLING: return "xspi|ospi master auto polling";
    case XSPI_MASTER_IO_TIMING: return "xspi|ospi master io";
    case XSPI_MASTER_DIFF_CLOCK: return "xspi|ospi master clock";
    case XSPI_MASTER_PARA_FIFO_SIZE: return "xspi|ospi master para fifo";
    case XSPI_MASTER_INDIRECT_AXI_MODE: return "xspi|ospi master indirect axi mode";
    case XSPI_MASTER_HYPERRAM_LOCKSTEP: return "xspi|ospi master hyperram lockstep";
    case XSPI_MASTER_HYPERRAM_PARALLEL: return "xspi|ospi master hyperram parallel";
    case XSPI_MASTER_HYPERFLASH_LOCKSTEP: return "xspi|ospi master hyperflash lockstep";
    case XSPI_MASTER_HYPERFLASH_PARALLEL: return "xspi|ospi master hyperflash parallel";
    case XSPI_MASTER_PARALLEL_CRC: return "xspi|ospi master parallel crc checking";
    case XSPI_MASTER_LOCKSTEP_READ_ERROR: return "xspi|ospi master lockstep read error";
    default:  return "Unknow";
    }
}

static int validation_flash_init(void *param) {
    struct spi_nor *nor = param;
    int ret = -1;

    if (!v_context.name) {
        ssdk_printf(SSDK_CRIT, "Need setup firstly!\r\n");
        return -1;
    } else if (nor->hyperbus_mode) {
        return 0; // Do nothing for hyperbus mode
    } else {
        for (uint8_t i = 0; i < ARRAY_SIZE(spibus_flash_table); i++) {
            if (!memcmp(spibus_flash_table[i].flash_id, nor->info.flash_id, 2)) {
                ssdk_printf(SSDK_CRIT, "Device %s matched\r\n", nor->info.name);
                return spibus_flash_table[i].init(nor);
            }
        }
    }
    ssdk_printf(SSDK_CRIT, "This device(%s) not support this validation case: %s\r\n",
        nor->info.name, v_context.name);
    return ret;
}

static int norflash_init(int argc, char *argv[]) {
    int port_id = -1;
    int init_case = -1;
    int ret = 0;

    if (argc > 1) {
        int temp_num = atoi(argv[0]);
        if (temp_num >= 0 && temp_num < CONTROLLER_ID_MAX) {
            port_id = temp_num;
        }
        temp_num = atoi(argv[1]);
        if (temp_num >= 0 && temp_num < XSPI_MASTER_CASE_MAX_NUM) {
            init_case = temp_num;
        }
    }

    if (init_case == -1 || port_id == -1) {
        ssdk_printf(SSDK_CRIT, "Usage: norflash_init port_id case_id\r\n");
        ssdk_printf(SSDK_CRIT, "case_id |\t case description\r\n");
        for (uint32_t i = 0; i < XSPI_MASTER_CASE_MAX_NUM; i++) {
            ssdk_printf(SSDK_CRIT, "\t%d \t %s\r\n", i, get_validation_case_name(i));
        }
        return CLI_FALSE;
    }

    validation_setup(init_case);
    default_config.dev_mode = v_context.dev_mode;

    switch (port_id) {
    case XSPI1_PORTA:
    case XSPI1_PORTB:
        ret = validation_xspi_host_init(&default_xspi_config[port_id - XSPI1_PORTA]);
        break;
    case OSPI1_PORT:
    case OSPI2_PORT:
        ret = validation_ospi_host_init(&default_ospi_config[port_id - OSPI1_PORT]);
        break;
    }

    if (ret) {
        ssdk_printf(SSDK_CRIT, "Host init failed\r\n");
        return CLI_FALSE;
    }

    ret = spi_nor_init(&v_context.flash, &v_context.spi_nor_host, &default_config);
    if (ret) {
        ssdk_printf(SSDK_CRIT, "Flash init failed\r\n");
        return CLI_FALSE;
    }

    if (default_config.xfer_mode == SPI_NOR_XFER_DMA_MODE) {
        if (port_id <= XSPI1_PORTB) {
            v_context.flash.dma_xfer_config = validation_xspi_dma_config;
        } else {
            v_context.flash.dma_xfer_config = validation_ospi_dma_config;
        }
        v_context.flash.dma_stop = validation_dma_stop;
    }

    v_context.info = spi_nor_get_info(&v_context.flash);
    v_context.port_id = port_id;

    if (v_context.slv_xip_mode) {
        validation_miron_xip_enter(&v_context.flash);
        spi_nor_xip_mode_enter(&v_context.flash);
    }

    return CLI_FALSE;
}
CLI_CMD("norflash_init", "\r\nnorflash_init:\r\n norflash_init command\r\n", norflash_init);

#define RX_TX_BUFFER_SIZE (2048u)
static uint8_t rx_buf[RX_TX_BUFFER_SIZE + 32u] __ALIGNED(32) = {0};
static uint8_t tx_buf[RX_TX_BUFFER_SIZE + 32u] __ALIGNED(32) = {"hello norflash!"};
static int xspi_flash_memcmp(flash_addr_t addr, uint8_t code, flash_size_t len) {
    uint32_t i, j, temp_index;
    uint32_t *ptr;
    flash_addr_t temp_addr = addr;
    flash_size_t temp_size = len;
    uint32_t mask = (code) | (code << 8) | (code << 16) | (code << 24);

    while (temp_size > 0) {
        uint32_t read_size = MIN(temp_size, RX_TX_BUFFER_SIZE);

        memset(rx_buf, 0x55, read_size);
        /* rx_buf address cache invalidate */
        if (spi_nor_read(&v_context.flash, temp_addr, rx_buf, read_size)) {
            ssdk_printf(SSDK_CRIT, "\r\n compare(%x) check, read error\n", code);
            return -1;
        }
        ptr = (uint32_t *) rx_buf;

        for (i = 0; i < (read_size / 4); i++, ptr++) {
            if (*ptr != mask) {
                ssdk_printf(SSDK_CRIT, "\r\n compare(%x) check error: [%llx] 0x%x \r\n",
                    code, temp_addr + i * 4, *ptr);
                return -1;
            }
        }

        if (read_size % 4) {
            for (j = 0; j < (read_size % 4); j++) {
                temp_index = (i * 4) + j;
                if (rx_buf[temp_index] != code) {
                    ssdk_printf(SSDK_CRIT, "\r\n compare(%x) check error: [%llx] 0x%x \r\n",
                        code, temp_addr + temp_index, rx_buf[temp_index]);
                    return -1;
                }
            }
        }

        temp_addr += read_size;
        temp_size -= read_size;
    }

    return 0;
}

/*  check buf is blank(FF), size=len */
static int xspi_blank_check(flash_addr_t addr, flash_size_t len) {
    return xspi_flash_memcmp(addr, 0xFF, len);
}

/*  check src == dst, size=len */
static int xspi_compare_check(uint8_t *src, uint8_t *dst, flash_size_t len)
{
    uint32_t i, j, temp_index;
    uint32_t *p1 = (uint32_t *)src;
    uint32_t *p2 = (uint32_t *)dst;

    for (i = 0; i < (len / 4); i++, p1++, p2++) {
        if (*p1 != *p2) {
            ssdk_printf(SSDK_CRIT, "\r\n compare check error: [%d] 0x%x != 0x%x \r\n",
                i * 4, *p1, *p2);
            return -1;
        }
    }

    if (len % 4) {
        for (j = 0; j < (len % 4); j++) {
            temp_index = (i * 4) + j;
            if (src[temp_index] != dst[temp_index]) {
                ssdk_printf(SSDK_CRIT, "\r\n compare check error: [%d] 0x%x != 0x%x \r\n",
                    temp_index, src[temp_index], dst[temp_index]);
                return -1;
            }
        }
    }
    return 0;
}

static int xspi_flash_memset(flash_addr_t addr, uint8_t code, flash_size_t len) {
    int ret;
    int step = 0;
    ssdk_printf(SSDK_EMERG, "memset addr:0x%llx len:0x%llx to 0x%x\r\n",
        addr, len, code);

    ssdk_printf(SSDK_EMERG, "\t\t step%d: do flash erase: \r\n", step++);
    ret = spi_nor_erase(&v_context.flash, ROUNDDOWN(addr, v_context.info->sector_size),
        ROUNDUP(len + addr - ROUNDDOWN(addr, v_context.info->sector_size),
                v_context.info->sector_size));
    if (ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t-- erase error(%d)--\r\n", ret);
        return -1;
    } else if (0xFF == code) {
        return 0;
    }

    flash_addr_t temp_addr = addr;
    flash_size_t temp_size = len;
    while (temp_size > 0) {
        uint32_t write_size = MIN(temp_size, RX_TX_BUFFER_SIZE);
        ssdk_printf(SSDK_EMERG, "\t\t step%d: do flash write: \r\n", step++);
        memset(tx_buf, code, write_size);
        /* write tx_buf to flash address */
        ret = spi_nor_write(&v_context.flash, temp_addr, (const uint8_t *)tx_buf, write_size);
        if (ret) {
            ssdk_printf(SSDK_EMERG, "\t\t\t\t-- write error(%d)--\r\n", ret);
            return -3;
        } else {
            temp_addr += write_size;
            temp_size -= write_size;
            ssdk_printf(SSDK_EMERG, "\t\t\t\t residue %llx  \r\n", temp_size);
        }
    }

    return 0;
}

static int xspi_common_test_once(flash_addr_t addr, flash_size_t len)
{
    int32_t ret = 0;
    int step = 0;
    uint32_t i;

    ssdk_printf(SSDK_EMERG, "[Common test] addr:0x%llx len:0x%llx\r\n", addr, len);

    ssdk_printf(SSDK_EMERG, "\t\t step%d: test flash erase: \r\n", step++);
    /* erase flash addr,size = len */
    ret = spi_nor_erase(&v_context.flash, addr, ROUNDUP(len, v_context.info->sector_size));
    if (ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t-- erase error(%d)--\r\n", ret);
        return -1;
    } else if (xspi_blank_check(addr, len)) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t-- blank check error--\r\n");
        return -2;
    } else {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
    }

    flash_addr_t temp_addr = addr;
    flash_size_t temp_size = len;
    while (temp_size > 0) {
        uint32_t write_size = MIN(temp_size, RX_TX_BUFFER_SIZE);
        ssdk_printf(SSDK_EMERG, "\t\t step%d: test flash write: \r\n", step++);
        for (i = 0; i < write_size; i++) {
            tx_buf[i] = i & 0xFF;
        }
        /* write tx_buf to flash address */
        ret = spi_nor_write(&v_context.flash, temp_addr, (const uint8_t *)tx_buf, write_size);
        if (ret) {
            ssdk_printf(SSDK_EMERG, "\t\t\t\t-- write error(%d)--\r\n", ret);
            return -3;
        } else {
            ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
        }

        ssdk_printf(SSDK_EMERG, "\t\t step%d: test flash read: \r\n", step++);
        memset(rx_buf, 0x55, write_size);
        /* read flash addr to rx_buf */
        ret =spi_nor_read(&v_context.flash, temp_addr, rx_buf, write_size);
        if (ret) {
            ssdk_printf(SSDK_EMERG, "\t\t\t\t-- read error(%d)--\r\n", ret);
            return -4;
        } else if (xspi_compare_check(tx_buf, rx_buf, write_size)) {
            ssdk_printf(SSDK_EMERG, "\t\t\t\t-- compare check error--\r\n");
            return -5;
        } else {
            ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
        }
        temp_addr += write_size;
        temp_size -= write_size;
    }

    return 0;
}

static int xspi_common_test(uint32_t start_sector)
{
    int32_t ret = 0;
    uint32_t i;
    flash_addr_t addr;
    uint32_t sector_num = v_context.info->size / v_context.info->sector_size;

    if (start_sector >= sector_num) {
        return -1;
    }

    for (i = start_sector; i < sector_num; i++) {
        addr = v_context.info->sector_size * i;
        ssdk_printf(SSDK_EMERG, "[common test] sector:%d, addr:0x%llx\r\n", i, addr);
        ret = xspi_common_test_once(addr, 0x800);  // len=2K
        if (ret) {
            break;
        }
    }
    return ret;
}

static int xspi_faultinject_addr_test()
{
    int32_t ret = 0;
    int step = 0;
    uint32_t sector_num = v_context.info->size / v_context.info->sector_size;
    flash_addr_t addr_base = (rand() & (sector_num - 1)) * v_context.info->sector_size;
    ssdk_printf(SSDK_EMERG, "[faultinject addr test] \r\n");


    ssdk_printf(SSDK_EMERG, "\t\t step%d: test faultinject erase addr: \r\n", step++);
    ret = spi_nor_erase(&v_context.flash, addr_base - 1U, v_context.info->sector_size);
    if (!ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
    }

    ssdk_printf(SSDK_EMERG, "\t\t step%d: test faultinject write addr: \r\n", step++);
    ret = spi_nor_write(&v_context.flash, addr_base - 1U, tx_buf, v_context.info->page_size * 2);
    if (!ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
    }

    ssdk_printf(SSDK_EMERG, "\t\t step%d: test faultinject read addr: \r\n", step++);
    ret = spi_nor_read(&v_context.flash, addr_base - 1U, rx_buf, v_context.info->page_size * 2);
    if (!ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
    }

    return 0;
}

static int xspi_faultinject_buff_test()
{
    int32_t ret = 0;
    int step = 0;
    uint32_t sector_num = v_context.info->size / v_context.info->sector_size;
    flash_addr_t addr = (rand() & (sector_num - 1)) * v_context.info->sector_size;
    ssdk_printf(SSDK_EMERG, "[faultinject buff test] \r\n");

    ssdk_printf(SSDK_EMERG, "\t\t step%d: test faultinject write buff: \r\n", step++);
    ret = spi_nor_write(&v_context.flash, addr, tx_buf + 1, v_context.info->page_size * 2);
    if (!ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
    }

    ssdk_printf(SSDK_EMERG, "\t\t step%d: test faultinject read buff: \r\n", step++);
    ret = spi_nor_read(&v_context.flash, addr, rx_buf + 1, v_context.info->page_size * 2);
    if (!ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
    }
    return 0;
}

static int xspi_faultinject_size_test()
{
    int32_t ret = 0;
    int step = 0;
    uint32_t sector_num = v_context.info->size / v_context.info->sector_size;
    flash_addr_t addr = (rand() & (sector_num - 1)) * v_context.info->sector_size;
    ssdk_printf(SSDK_EMERG, "[faultinject size test] \r\n");


    ssdk_printf(SSDK_EMERG, "\t\t step%d: test faultinject erase size: \r\n", step++);
    ret = spi_nor_erase(&v_context.flash, addr, v_context.info->sector_size - 1U);
    if (!ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
    }

    ssdk_printf(SSDK_EMERG, "\t\t step%d: test faultinject write size: \r\n", step++);
    ret = spi_nor_write(&v_context.flash, addr, tx_buf, v_context.info->page_size * 2 + 1U);
    if (!ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
    }

    ssdk_printf(SSDK_EMERG, "\t\t step%d: test faultinject read size: \r\n", step++);
    ret = spi_nor_read(&v_context.flash, addr, rx_buf, v_context.info->page_size * 2 + 1U);
#if XSPI_USE_DIRECT_MODE
    if (ret) {
#else
    if (!ret) {
#endif
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t PASS \r\n");
    }
    return 0;
}

static int xspi_stress_test(uint32_t max_loop)
{
    int32_t ret = 0;
    uint32_t loop = 0;
    uint32_t sector_num = v_context.info->size / v_context.info->sector_size;
    flash_addr_t addr;
    flash_size_t len = 0;

    while (loop++ < max_loop) {
        addr = (rand() & (sector_num - 1)) * v_context.info->sector_size;
        len = ROUNDUP((rand() & 0x7FF), 32U);
        // len = 32;
        ssdk_printf(SSDK_EMERG, "[stress test] loop:%d, addr:0x%llx, len:0x%llx\r\n",
            loop, addr, len);
        ret = xspi_common_test_once(addr, len);
        if (ret) {
            break;
        }
    }
    return ret;
}

#ifdef CONFIG_DW_RTC
#include <rtc_cfg.h>

static int xspi_performance_test()
{
    int32_t ret = 0;
    uint64_t tick1 = 0, tick2 = 0;
    struct rtc_time rtc_ts;
    int step = 0;
    uint32_t speed = 0;
    uint32_t sector_num = v_context.info->size / v_context.info->sector_size;
    flash_addr_t addr = (rand() & (sector_num - 1)) * v_context.info->sector_size;

    ssdk_printf(SSDK_EMERG, "[performance test] addr:0x%#llx\r\n", addr);

    epoch_to_rtc(&rtc_ts, 1);
    rtc_set_time(&g_rtc1_dev, &rtc_ts);

    ssdk_printf(SSDK_EMERG, "\t\t step%d: test erase performance: \r\n", step++);
    rtc_get_time_us(&g_rtc1_dev, &tick1);
    ret = spi_nor_erase(&v_context.flash, addr, v_context.info->sector_size);
    if (ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        rtc_get_time_us(&g_rtc1_dev, &tick2);
        speed = v_context.info->sector_size * 1000u / (tick2 - tick1);
        ssdk_printf(SSDK_EMERG, "\t\t\t\t A Sector(0x%x B) erase time: %lld ms\r\n",
            v_context.info->sector_size, (tick2 - tick1) / 1000u);
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Unit conversion: %d KB/s\r\n", speed);
    }

    ssdk_printf(SSDK_EMERG, "\t\t step%d: test write performance: \r\n", step++);
    rtc_get_time_us(&g_rtc1_dev, &tick1);
    ret = spi_nor_write(&v_context.flash, addr, tx_buf, v_context.info->page_size);
    if (ret) {
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Failed \r\n");
        return -step;
    } else {
        rtc_get_time_us(&g_rtc1_dev, &tick2);
        speed = (v_context.info->page_size * 1000U) / (tick2 - tick1);
        ssdk_printf(SSDK_EMERG, "\t\t\t\t A Page(0x%x B) write time: %lld us\r\n",
            v_context.info->page_size, tick2 - tick1);
        ssdk_printf(SSDK_EMERG, "\t\t\t\t Unit conversion: %d KB/s\r\n", speed);
    }

    return 0;
}
#endif

#define SAF_A_ENTRY_OFFSET      (0x87000u)
#define SAF_B_ENTRY_OFFSET      (0x387000u)
typedef void (*fv_v) (void);
static void validation_xip_jump() {
    addr_t direct_base;
    uint8_t *entry;

    switch (v_context.port_id) {
    case XSPI1_PORTA:
        direct_base = default_xspi_config[0].direct_base;
        break;
    case XSPI1_PORTB:
        direct_base = default_xspi_config[1].direct_base;
        break;
    case OSPI1_PORT:
        direct_base = default_ospi_config[0].ahb_base;
        break;
    case OSPI2_PORT:
        direct_base = default_ospi_config[1].ahb_base;
        break;
    default:
        return;
    }
    entry = (void *)(direct_base + SAF_A_ENTRY_OFFSET);
    ((fv_v)entry)();
}

static int norflash_test(int argc, char *argv[]){
    int test_case = 0;
    int ret = 0;

    if (argc > 0) {
        test_case = atoi(argv[0]);
    }

    switch (test_case) {
        case 1:
            ret = xspi_common_test(0);
            break;
        case 2:
            ret = xspi_faultinject_addr_test();
            break;
        case 3:
            ret = xspi_faultinject_buff_test();
            break;
        case 4:
            ret = xspi_faultinject_size_test();
            break;
        case 5:
            if (argc > 1) {
                ret = xspi_stress_test(atoi(argv[1]));
            } else {
                ret = xspi_stress_test(1000000U);
            }
            break;
#ifdef CONFIG_DW_RTC
        case 6:
            ret = xspi_performance_test();
            break;
#endif
        case 7:
            validation_xip_jump();
            break;
        default:
            ssdk_printf(SSDK_ERR, "Error case id!\r\n");
            ret = -1;
            break;
    }

    if (ret) {
        ssdk_printf(SSDK_CRIT, "case-%d Failed!\r\n", test_case);
    } else {
        ssdk_printf(SSDK_CRIT, "case-%d Success!\r\n", test_case);
    }

    return CLI_FALSE;
}
CLI_CMD("norflash_test", "\r\nnorflash_test:\r\n norflash_test command\r\n", norflash_test);

static int norflash_config(int argc, char *argv[]) {
    enum spi_memory_type mem_type = SPI_AUTO_DETECT;
    if (argc < 2) {
        ssdk_printf(SSDK_CRIT, "[xfer mode] [hyperbus mode]");
        return CLI_FALSE;
    }
    default_config.xfer_mode = atoi(argv[0]);
    mem_type = atoi(argv[1]);
    default_config.hyperbus_mode = !!mem_type;
    default_config.mem_type = (default_config.hyperbus_mode) ? mem_type : 0;
    ssdk_printf(SSDK_CRIT, "config set successfully!\r\n");
    return CLI_FALSE;
}
CLI_CMD("norflash_config", "\r\nnorflash_config:\r\n norflash_config command\r\n", norflash_config);

static int rd_reg(int argc, char *argv[])
{
    int ret = 0;
    int cmd = -1;
    int reg = 0;
    uint32_t len = 0;
    uint8_t data[2];

    switch (argc) {
    case 2:
        reg = atoi(argv[1]);
        len = SNOR_INST_LANS(v_context.flash.reg_proto) == 3 ? 2 : 1;
    case 1:
        cmd = atoi(argv[0]);
        break;
    default:
        break;
    }

    struct spi_nor_cmd r_cmd = {
        .opcode = (uint8_t)cmd,
        .addr_bytes = v_context.flash.addr_width,
        .inst_type = SNOR_INST_LANS(v_context.flash.reg_proto),
        .dummy = 8,
    };
    ret = spi_nor_reg_read(&v_context.flash, &r_cmd, reg, data, len);
    if (ret) {
        ssdk_printf(SSDK_CRIT, "read Failed!\r\n");
    } else {
        ssdk_printf(SSDK_CRIT, "read Success: %x %x!\r\n", data[0], data[1]);
    }
    return CLI_FALSE;
}
CLI_CMD("rd_reg", "\r\nrd_reg:\r\n rd_reg command\r\n", rd_reg);
