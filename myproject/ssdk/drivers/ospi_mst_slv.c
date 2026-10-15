/**
 * @file ospi_mst_slv.c
 * @brief spi master driver C code.
 *
 * @copyright Copyright (c) 2023  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifdef CONFIG_CDNS_OSPI_MST_SLV
#include <reg.h>
#include <stdio.h>
#include <compiler.h>
#include <string.h>
#include <debug.h>
#include <param.h>
#include <board.h>

#include "arch/cache.h"
#include "driver/spi_nor.h"
#include "driver/ospi_mst_slv.h"


#define PROTO(_opcode, _dq) \
        (_opcode << SNOR_OPCODE_PROTO_LSB | _dq)
#define ID_PROTO(dummy, _dq) \
        (dummy << SNOR_READID_DUMMY_LSB | _dq)

#define WRITE_TRIGGER_CMD 0xfe01
#define READ_TRIGGER_CMD 0xfd02
#define WRITE_PACKET0_CMD 0xfc03
#define WRITE_PACKET1_CMD 0xfb04
#define WRITE_WDATA_CMD 0xfa05
#define WRITE_PAYLOAD_CMD 0xf906
#define WRITE_STATUS_CMD 0xf807
#define READ_ID_CMD 0xf708
#define READ_RDATA_CMD 0xf609
#define READ_RDATA_CRC_CMD 0xf50a
#define READ_STATUS_CMD 0xf40b

#define MST_READ_SLV_STATUS_CMD //check slaver status, may not be defined for debug
//#define TRAINING_RXDATA_TODO_TEST
//#define CONFIG_MST_SLV_SINGLE_LINE_SDR_MODE
//#define CONFIG_MST_SLV_QUAD_LINE_SDR_MODE
//#define CONFIG_MST_SLV_OCTAL_LINE_SDR_MODE   //20mhz
//#define CONFIG_MST_SLV_SINGLE_LINE_DTR_MODE
//#define CONFIG_MST_SLV_QUAD_LINE_DTR_MODE
//#define CONFIG_MST_SLV_OCTAL_LINE_DTR_MODE //50mhz

//#define ENABLE_OSPI_MST_SLV_CTL_DEBUG
#ifdef ENABLE_OSPI_MST_SLV_CTL_DEBUG
#define OSPI_MST_SLV_CTL_DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define OSPI_MST_SLV_CTL_DEBUG(format, ...)
#endif

const  uint8_t mst_training_pattern[16] __ALIGNED(CONFIG_ARCH_CACHE_LINE) = {
    0x44, 0x1c, 0x39, 0x05, 0xd3, 0x7a, 0x3c, 0x04,
    0x16, 0x42, 0x0c, 0x8b, 0x7d, 0x12, 0x89, 0xa2,
};

uint8_t mst_training_buf[16] __ALIGNED(CONFIG_ARCH_CACHE_LINE) = {0};

struct packet1 {
    uint32_t burst_type : 2;
    uint32_t size : 2;
    uint32_t len : 4;
    uint32_t cache : 4;
    uint32_t prot : 3;
    uint32_t is_read : 1;
    uint32_t sid : 4;
    uint32_t id : 2;
    uint32_t remain : 2;
    uint32_t user : 8;
};

static struct flash_info default_spi_nor_slaver_ids = {
    .name = "default",
    .flash_id = {0x00},
    .read_proto = PROTO(READ_RDATA_CMD, SNOR_PROTO_1_1_1),
    .write_proto = PROTO(WRITE_WDATA_CMD, SNOR_PROTO_1_1_1),
    .read_dummy = 11,
    .page_size = 64,
};

//taishan taishanL dtr mode is not supported
static struct flash_info spi_nor_slaver_ids = {
    .name = "ospi_mst_slv",
    .flash_id = {0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89},//slaver id0: 01234567, id1: 89abcdef
    .page_size = 64,
#ifdef CONFIG_MST_SLV_SINGLE_LINE_DTR_MODE
    .read_proto = PROTO(READ_RDATA_CMD, SNOR_PROTO_1_1_1_DTR),
    .write_proto = PROTO(WRITE_WDATA_CMD, SNOR_PROTO_1_1_1_DTR),
    .read_dummy = 11,
#endif

#ifdef CONFIG_MST_SLV_SINGLE_LINE_SDR_MODE
    .read_proto = PROTO(READ_RDATA_CMD, SNOR_PROTO_1_1_1),
    .write_proto = PROTO(WRITE_WDATA_CMD, SNOR_PROTO_1_1_1),
    .read_dummy = 11,
#endif

#ifdef CONFIG_MST_SLV_QUAD_LINE_DTR_MODE
    .read_proto = PROTO(READ_RDATA_CMD, SNOR_PROTO_4_4_4_DTR),
    .write_proto = PROTO(WRITE_WDATA_CMD, SNOR_PROTO_4_4_4_DTR),
    .read_dummy = 11,
#endif

#ifdef CONFIG_MST_SLV_QUAD_LINE_SDR_MODE
    .read_proto = PROTO(READ_RDATA_CMD, SNOR_PROTO_4_4_4),
    .write_proto = PROTO(WRITE_WDATA_CMD, SNOR_PROTO_4_4_4),
    .read_dummy = 11,
#endif

#ifdef CONFIG_MST_SLV_OCTAL_LINE_DTR_MODE
    .read_proto = PROTO(READ_RDATA_CMD, SNOR_PROTO_8_8_8_DTR),
    .write_proto = PROTO(WRITE_WDATA_CMD, SNOR_PROTO_8_8_8_DTR),
    .read_dummy = 11,//50mhz taihang
#endif

#ifdef CONFIG_MST_SLV_OCTAL_LINE_SDR_MODE
    .read_proto = PROTO(READ_RDATA_CMD, SNOR_PROTO_8_8_8),
    .write_proto = PROTO(WRITE_WDATA_CMD, SNOR_PROTO_8_8_8),
    //depends on master's ddr&&sdr mode and frequency,and the type of slaver soc
    .read_dummy = 11,//slaver taishanL 50mhz,dummy10; 100mhz,dummy11;
#endif
};

/**
 * @brief spi nor read register
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] cmd slaver cmd ptr
 * @param[in] addr slaver read address
 * @param[in] buf read to buf addr
 * @param[in] length read size
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int32_t ospi_master_reg_read(struct spi_nor *flash_handle,
                                    struct spi_nor_cmd *cmd, flash_addr_t addr,
                                    uint8_t *buf, flash_size_t length)
{
    int32_t ret;

    flash_handle->host->ops->prepare(flash_handle, SPI_NOR_OPS_LOCK);
    ret = flash_handle->host->ops->reg_read(flash_handle, cmd, addr, buf, length);
    flash_handle->host->ops->unprepare(flash_handle, SPI_NOR_OPS_LOCK);

    return ret;
}

/**
 * @brief spi nor write register
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] cmd slaver cmd ptr
 * @param[in] addr slaver read address
 * @param[in] buf write from buf addr
 * @param[in] length write size
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int32_t ospi_master_reg_write(struct spi_nor *flash_handle,
                                     struct spi_nor_cmd *cmd, flash_addr_t addr,
                                     uint8_t *buf, flash_size_t length)
{
    int32_t ret;

    flash_handle->host->ops->prepare(flash_handle, SPI_NOR_OPS_LOCK);
    ret = flash_handle->host->ops->reg_write(flash_handle, cmd, addr, buf, length);
    flash_handle->host->ops->unprepare(flash_handle, SPI_NOR_OPS_LOCK);

    return ret;
}
/**
 * @brief spi nor read device info
 * @param[in] nor spi slaver ptr.
 * @retval 0: success
 * @retval other: failed
 */
static int32_t ospi_master_read_slaver_id(struct spi_nor *nor)
{

    int32_t ret = 0;
    uint8_t id[8];

    struct flash_info *info;

    struct spi_nor_cmd read_cmd = {
        .opcode = READ_ID_CMD,
        .dummy = (spi_nor_slaver_ids.read_dummy) & 0xff,
        .addr_bytes = 4, //slaver byte
        .inst_type = SNOR_INST_LANS(spi_nor_slaver_ids.read_proto),
    };
    nor->reg_proto = (spi_nor_slaver_ids.read_proto) & SNOR_PROTO_MASK;
    read_cmd.dummy -= 1;
    ssdk_printf(SSDK_CRIT, "octal_dtr_en: %d, dummy = %d, inst_type = %d, \
proto = %x\n", nor->octal_dtr_en, read_cmd.dummy, read_cmd.inst_type,
                nor->reg_proto);
    ret = ospi_master_reg_read(nor, &read_cmd, 0, id, 8);

    if (ret) return ret;

    ssdk_printf(SSDK_CRIT, "slaver id0: %x, id1: %x \n", *(uint32_t *)id,
                *(uint32_t *)(id + 4));
    info = &spi_nor_slaver_ids;

    if (!memcmp(info->flash_id, id, 8)) {
        memcpy(&(nor->info), info, sizeof(struct flash_info));
        ssdk_printf(SSDK_CRIT, "read slaver_id success! \n");
        return 0;
    }
    else {
        ssdk_printf(SSDK_CRIT, "read slaver_id failed! \n");
    }

    return ret;
}

/**
 * @brief spi nor get slaver current status
 * @param[in] nor spi norflash ptr.
 * @param[out] status get slaver current status
 * @return slaver current status
 */
static int32_t ospi_master_read_status_cmd(struct spi_nor *nor, uint32_t addr,
        uint32_t *buff,
        uint32_t len)
{
    int32_t ret = 0;
    struct spi_nor_cmd w_cmd = {
        .opcode = READ_STATUS_CMD,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = (spi_nor_slaver_ids.read_dummy) & 0xff,
    };
    //depends on master's ddr&&sdr mode and frequency,and the type of slaver soc
    //default 100mhz
    // if(clk_get_rate(nor->host->clk)/2 >= 100000000u){
    // w_cmd.dummy = (spi_nor_slaver_ids.read_dummy) & 0xff;
    // }else{
    // w_cmd.dummy = ((spi_nor_slaver_ids.read_dummy) & 0xff) - 1;
    // }

    OSPI_MST_SLV_CTL_DEBUG("read status cmd dummy = %d \n", w_cmd.dummy);
    ret = ospi_master_reg_read(nor, &w_cmd, addr, (uint8_t *)buff, len);

    if (ret) {
        return ret;
    }

    return 0;
}

/**
 * @brief master write trigger command to slaver
 * @param[in] nor spi norflash ptr.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
static int32_t ospi_master_write_trigger_cmd(struct spi_nor *nor)
{
    int32_t ret = 0;
    struct spi_nor_cmd w_cmd = {
        .opcode = WRITE_TRIGGER_CMD,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };
    ret = ospi_master_reg_write(nor, &w_cmd, 0, 0, 0);

    if (ret) {
        return ret;
    }

    return ret;
}

/**
 * @brief master read trigger command to slaver
 * @param[in] nor spi norflash ptr.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
static int32_t ospi_master_read_trigger_cmd(struct spi_nor *nor)
{
    int32_t ret = 0;
    struct spi_nor_cmd w_cmd = {
        .opcode = READ_TRIGGER_CMD,
        .addr_bytes = 0,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };
    ret = ospi_master_reg_write(nor, &w_cmd, 0, 0, 0);

    if (ret) {
        return ret;
    }

    return ret;
}

/**
 * @brief master write packet0 command to slaver
 * @param[in] nor spi norflash ptr.
 * @param[in] master transmit address.
 * @param[in] master transmit packet0.
 * @param[in] master transmit packet0 length.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
static int32_t ospi_master_write_packet0_cmd(struct spi_nor *nor, uint32_t addr,
        uint32_t *buff,
        uint32_t len)
{
    int32_t ret = 0;
    struct spi_nor_cmd w_cmd = {
        .opcode = WRITE_PACKET0_CMD,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };
    ret = ospi_master_reg_write(nor, &w_cmd, addr, (uint8_t *)buff, len);

    if (ret) {
        return ret;
    }

    return ret;
}

/**
 * @brief ospi_master_write_packet1_cmd
 * @param[in] transmit source address
 * @param[in] transmit source length
 * @param[in] axi size
 * @param[in] transmit address
 */
static int32_t ospi_master_write_packet1_cmd(struct spi_nor *nor, uint32_t addr,
        uint32_t *buff,
        uint32_t len)
{
    int32_t ret = 0;
    struct spi_nor_cmd w_cmd = {
        .opcode = WRITE_PACKET1_CMD,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
        .dummy = 0,
    };
    ret = ospi_master_reg_write(nor, &w_cmd, addr, (uint8_t *)buff, len);

    if (ret) {
        return ret;
    }

    return ret;
}

/**
 * @brief adjust buffer contents by axi information
 * @param[in] transmit source address
 * @param[in] transmit source length
 * @param[in] axi size
 * @param[in] transmit address
 */
static void ospi_master_indirect_adjust_read_data_by_axi_size(void *src_data,
        uint32_t len, uint8_t axi_size, uint32_t addr)
{
    uint32_t *p_data_32 = src_data;
    uint32_t i;

    if (axi_size == 0) {
        uint8_t *p_data_8 = src_data;

        for (i = 0; i < len / 4; i++) {
            uint8_t *tmp = (uint8_t *)p_data_32;
            p_data_8[i] = tmp[((addr % 4) + i) % 4];
            p_data_32++;
        }

        p_data_8[len / 4] = 0;
    }
    else if (axi_size == 1) {
        uint16_t *p_data_16 = src_data;

        for (i = 0; i < len / 4; i++) {
            uint16_t *tmp = (uint16_t *)p_data_32;
            p_data_16[i] = tmp[(((addr % 4) >> 1) + i) % 2];
            p_data_32++;
        }

        p_data_16[len / 4] = 0;
    }
    else if (axi_size == 2) {
        p_data_32[len / 4] = 0;
    }
}

/**
 * @brief adjust buffer contents by axi information
 * @param[in] transmit source address
 * @param[in] transmit source length
 * @param[out] transmit destination address
 * @param[in] axi size
 * @param[in] axi length
 * @param[in] transmit address
 */
static void ospi_master_indirect_write_adjust_data_by_axi_size(
    const void *src_data,
    uint32_t len, void *dst_data, uint8_t axi_size, uint8_t axi_len, uint32_t addr)
{
    uint32_t *p_data_dst = (uint32_t *)dst_data;
    const int8_t *p_src = src_data;
    uint8_t start_offset;
    int8_t tmp_buf[64] = {0};

    memcpy(tmp_buf + addr % (1 << axi_size), src_data, len);
    p_src = tmp_buf;
#define DATA_BUS_BYTES 4
    start_offset = (addr % DATA_BUS_BYTES) >> (axi_size);

    if (axi_size == 0) {
        uint8_t *p_data = (uint8_t *)p_src;

        for (int32_t i = 0; i <= axi_len; i++) {
            p_data_dst[i] = (p_data[i] << ((((i + start_offset) % 4)) * 8));
        }
    }
    else if (axi_size == 1) {
        uint16_t *p_data = (uint16_t *)p_src;

        for (int32_t i = 0; i <= axi_len; i++) {
            p_data_dst[i] = (p_data[i] << (((i + start_offset) % 2) * 16));
        }

    }
    else if (axi_size == 2) {
        uint32_t *p_data = (uint32_t *)p_src;

        for (int32_t i = 0; i <= axi_len; i++) {
            p_data_dst[i] = p_data[i];
        }
    }
}

/**
 * @brief entry function for read spi slaver devices.
 * @param[in] spi nor ptr.
 * @param[in] transmit slaver address.
 * @param[in] spi axi transmit packet information.
 * @param[in] read slaver buf.
 * @param[in] read slaver buf length.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
uint16_t ospi_master_write_data(struct spi_nor *nor, uint32_t packet0,
                                struct packet1 packet1, const void *buf, uint32_t len)
{
    uint8_t tmp_tx_buf[96] = {0};
    uint32_t status = 0;
    ospi_master_write_packet1_cmd(nor, packet0, (uint32_t *)&packet1,
                                  sizeof(packet1));
    ospi_master_write_packet0_cmd(nor, packet0, &packet0, sizeof(packet0));
    ospi_master_indirect_write_adjust_data_by_axi_size(buf, len, tmp_tx_buf,
            packet1.size,
            packet1.len, packet0);
    OSPI_MST_SLV_CTL_DEBUG("send write data \n");
    ospi_master_write(nor, packet0, tmp_tx_buf, (packet1.len + 1) * 4);
    OSPI_MST_SLV_CTL_DEBUG("send write trigger cmd \n");
    ospi_master_write_trigger_cmd(nor);
#ifdef MST_READ_SLV_STATUS_CMD
    ospi_master_read_status_cmd(nor, packet0, &status, sizeof(status));
    OSPI_MST_SLV_CTL_DEBUG("write status  = 0x%x \n", status);
    int32_t times = 0;

    while (!(status & (1u << 1))) { //0x40034003   //0x40014001  0x38
        ssdk_printf(SSDK_CRIT, "write status  = 0x%x \n", status);
        ospi_master_read_status_cmd(nor, packet0, &status, sizeof(status));

        if (times ++ == 10) {
            udelay(1000);
            break;
        }
    }

#endif
    return (status & 0xffff);
}

/**
 * @brief entry function for read spi slaver devices.
 * @param[in] spi nor ptr.
 * @param[in] transmit slaver address.
 * @param[in] spi axi transmit packet information.
 * @param[in] read slaver buf.
 * @param[in] read slaver buf length.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
uint16_t ospi_master_read_data(struct spi_nor *nor, uint32_t packet0,
                               struct packet1 packet1, const void *buf, uint32_t len)
{
    uint32_t status = 0;

    ospi_master_write_packet1_cmd(nor, packet0, (uint32_t *)&packet1,
                                  sizeof(packet1));
    ospi_master_write_packet0_cmd(nor, packet0, &packet0, sizeof(packet0));
    OSPI_MST_SLV_CTL_DEBUG("send read trigger cmd \n");
    ospi_master_read_trigger_cmd(nor);
#ifdef MST_READ_SLV_STATUS_CMD
    ospi_master_read_status_cmd(nor, packet0, &status, sizeof(status));
    OSPI_MST_SLV_CTL_DEBUG("read status  = 0x%x \n", status);
    int32_t times = 0;

    while (!(status & (1u << 2))) { //0x40054005   //0x40014001  0x38
        ssdk_printf(SSDK_CRIT, "read status  = 0x%x \n", status);
        ospi_master_read_status_cmd(nor, packet0, &status, sizeof(status));

        if (times ++ == 10) {
            udelay(1000);
            break;
        }
    }

#endif
    OSPI_MST_SLV_CTL_DEBUG("read data \n");
    ospi_master_read(nor, packet0, buf, len);
    //hexdump(buf, 0x50);

    return (status & 0xffff);
}

/**
 * @brief entry function for read spi slaver devices.
 * @param[in] spi nor ptr.
 * @param[in] spi slaver address.
 * @param[in] read slaver buf length.
 * @param[in] read slaver buf.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
int32_t ospi_master_indirect_read(struct spi_nor *nor, const uint32_t addr,
                                  const uint32_t rdlen, const uint8_t *buff)
{
    struct packet1 packet1 = {0};
    uint16_t status = 0;
    uint32_t axisize = 2;

    if (!IS_ALIGNED(addr, 4) || !IS_ALIGNED(buff, 4)) {
        ssdk_printf(SSDK_CRIT, "addr: 0x%x invalid\n", addr);
        return -1;
    }
    else if (!IS_ALIGNED(rdlen, 4)) {
        ssdk_printf(SSDK_CRIT, "rdlen: 0x%x invalid\n", rdlen);
        return -2;
    }

    packet1.burst_type = 1;
    packet1.size = axisize;
    packet1.cache = 0;
    packet1.prot = 0;
    packet1.is_read = 1;
    packet1.sid = 1;//indirect ignored
    packet1.id = 1;
    packet1.remain = 0;
    packet1.user = 0;

    int32_t wr_index_count[64] = {0}; // 4 16 32 64
    int32_t tmp_len = rdlen;

    for (int32_t index = 63; index >= 3; index -= 4) {
        while (tmp_len) {
            wr_index_count[index] = tmp_len / (index + 1);
            tmp_len %= (index + 1);
            break;
        }
    }

    int32_t tmp_addr = addr;

    for (int32_t index = 63; index >= 3; index -= 4) {
        while (wr_index_count[index]) {
            int32_t tmp_index = index;
            OSPI_MST_SLV_CTL_DEBUG("%2d %2d \n", index, wr_index_count[index]);
            packet1.len = (tmp_index + 1) / 4 - 1;
            uint32_t tmp_wrlen = packet1.size ? packet1.size * 2 * (packet1.len + 1) :
                                 packet1.len + 1;
            OSPI_MST_SLV_CTL_DEBUG("read addr = 0x%x, size = 0x%x, len = 0x%x, rdlen = 0x%x\n",
                                   tmp_addr,
                                   packet1.size, packet1.len, tmp_wrlen);
            status = ospi_master_read_data(nor, tmp_addr, packet1, buff, tmp_wrlen);
#ifdef MST_READ_SLV_STATUS_CMD

            if (!(status & (1u << 2))) {
                ssdk_printf(SSDK_CRIT, "read status error : %x\n", status);
                return status;
            }

#endif

            if (packet1.size < 2) {
                ospi_master_indirect_adjust_read_data_by_axi_size(buff, tmp_wrlen, packet1.size,
                        tmp_addr);
            }

            tmp_addr += tmp_wrlen;
            buff += tmp_wrlen;
            wr_index_count[index]--;
        }

    }

    return status;
}

/**
 * @brief entry function for write spi slaver devices.
 * @param[in] spi nor ptr.
 * @param[in] spi slaver address.
 * @param[in] write slaver buf length.
 * @param[in] write slaver buf.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
int32_t ospi_master_indirect_write(struct spi_nor *nor, const uint32_t addr,
                                   const uint32_t wrlen, uint8_t *buff)
{
    struct packet1 packet1 = {0};
    uint32_t axisize = 2;
    uint16_t status = 0;
    uint32_t write_size = 0;

    if (!IS_ALIGNED(addr, 4) || !IS_ALIGNED(buff, 4)) {
        ssdk_printf(SSDK_CRIT, "addr: 0x%x invalid\n", addr);
        return -1;
    }
    else if (!IS_ALIGNED(wrlen, 4)) {
        ssdk_printf(SSDK_CRIT, "wrlen: 0x%x invalid\n", wrlen);
        return -2;
    }

    packet1.burst_type = 1;
    packet1.size = axisize;
    packet1.cache = 0;
    packet1.prot = 0;
    packet1.is_read = 0;
    packet1.sid = 0;//indirect ignored
    packet1.id = 1;
    packet1.remain = 0;
    packet1.user = 0;

    int32_t tmp_addr = addr;
    int32_t tmp_wrlen = wrlen;

    // write_size = MIN((nor->info->page_size - tmp_addr % nor->info->page_size),
    // tmp_wrlen);

    write_size = MIN((nor->info.page_size - tmp_addr % nor->info.page_size),
                     tmp_wrlen);

    if (write_size > 0) {
        packet1.len = write_size / 4 - 1;
    }

    OSPI_MST_SLV_CTL_DEBUG("top write addr = 0x%x, size = 0x%x, len = 0x%x, write_size = 0x%x\n",
                           tmp_addr, packet1.size, packet1.len, write_size);
    status =  ospi_master_write_data(nor, tmp_addr, packet1, buff,
                                     write_size);//pass
#ifdef MST_READ_SLV_STATUS_CMD

    if (!(status & (1u << 1))) {
        ssdk_printf(SSDK_CRIT, "indirect_write read status error : %x\n", status);
        return status;
    }

#endif

    tmp_wrlen -= write_size;
    tmp_addr += write_size;
    buff += write_size;

    int32_t wr_index_count[64] = {0};

    for (int32_t index = 63; index >= 3; index -= 4) {
        while (tmp_wrlen) {
            wr_index_count[index] = tmp_wrlen / (index + 1);
            tmp_wrlen %= (index + 1);
            break;
        }
    }

    for (int32_t index = 63; index >= 3; index -= 4) {
        while (wr_index_count[index]) {
            int32_t tmp_index = index;
            OSPI_MST_SLV_CTL_DEBUG("%2d %2d \n", index, wr_index_count[index]);
            packet1.len = (tmp_index + 1) / 4 - 1;
            write_size = packet1.size ? packet1.size * 2 * (packet1.len + 1) : packet1.len +
                         1;

            OSPI_MST_SLV_CTL_DEBUG("write addr = 0x%x, size = 0x%x, len = 0x%x, write_size = 0x%x\n",
                                   tmp_addr, packet1.size, packet1.len, write_size);
            status =  ospi_master_write_data(nor, tmp_addr, packet1, buff,
                                             write_size);//pass
#ifdef MST_READ_SLV_STATUS_CMD

            if (!(status & (1u << 1))) {
                ssdk_printf(SSDK_CRIT, "write status error : %x\n", status);
                return status;
            }

#endif
            tmp_addr += write_size;
            buff += write_size;
            wr_index_count[index]--;
        }

    }

    return status;
}

/**
 * @brief records the status of the current read/write transmission
 * @param[in] spi nor ptr.
 * @param[in] spi slaver read/write options.
 * @param[in] read/write slaver buf ptr.
 * @param[in] read/write slaver buf length.
 */
static void inline ospi_master_setup_xfer(struct spi_nor *nor,
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
                "spinor setup xfer: type = %d, addr = 0x%llx, size = %lld\n", opt, addr, size);
}

/**
 * @brief returns when the read/write transfer is complete
 * @param[in] spi nor ptr.
 */
static void inline ospi_master_xfer_comp(struct spi_nor *nor)
{
    nor->xfer_info.size = 0;
    nor->xfer_info.opt_result = FLASH_OPT_COMPLETE;
    nor->host->ops->unprepare(nor, nor->xfer_info.opt_type);
    nor->xfer_info.opt_type = 0;
}

/**
 * @brief returns when a read/write transfer error occurs
 * @param[in] spi nor ptr.
 */
static void inline ospi_master_xfer_error(struct spi_nor *nor)
{
    nor->xfer_info.size = 0;
    nor->xfer_info.opt_result = FLASH_OPT_FAILED;
    nor->host->ops->unprepare(nor, nor->xfer_info.opt_type);
    nor->xfer_info.opt_type = 0;
    ssdk_printf(SSDK_CRIT, "spi_nor xfer failed\n");
}

/**
 * @brief entry function for read spi slaver devices.
 * @param[in] spi nor ptr.
 * @param[in] spi slaver address.
 * @param[in] read slaver buf.
 * @param[in] read slaver buf length.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
int32_t ospi_master_read(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                         flash_size_t size)
{
    int32_t ret;

    nor->host->ops->prepare(nor, SPI_NOR_OPS_READ);
    ospi_master_setup_xfer(nor, FLASH_OPT_READ, addr, buf, size);
    ret = nor->host->ops->read(nor, addr, buf, size);

    if (ret) {
        ospi_master_xfer_error(nor);
    }
    else if (!nor->async_mode) {
        ospi_master_xfer_comp(nor);
    }

    return ret;
}

/**
 * @brief entry function for write spi slaver devices.
 * @param[in] spi nor ptr.
 * @param[in] spi slaver address.
 * @param[in] write slaver buf.
 * @param[in] write slaver buf length.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
int32_t ospi_master_write(struct spi_nor *nor, flash_addr_t addr,
                          const uint8_t *buf,
                          flash_size_t size)
{
    int32_t ret;

    nor->host->ops->prepare(nor, SPI_NOR_OPS_WRITE);
    ospi_master_setup_xfer(nor, FLASH_OPT_WRITE, addr, (uint8_t *)buf, size);
    ret = nor->host->ops->write(nor, addr, buf, size);

    if (ret) {
        ospi_master_xfer_error(nor);
    }
    else if (!nor->async_mode) {
        ospi_master_xfer_comp(nor);
    }

    return ret;
}

/**
 * @brief entry function for init spi slaver devices.
 * @param[in] spi nor ptr.
 * @param[in] spi nor_host config ptr.
 * @param[in] spi nor config config.
 * @return int32_t
 * @retval 0: success
 * @retval other: failed
 */
int32_t ospi_master_init(struct spi_nor *nor, struct spi_nor_host *host,
                         const struct spi_nor_config *config)
{
    int32_t ret = 0;

    memset(nor, 0, sizeof(struct spi_nor));

    nor->id = config->id;
    nor->cs = config->cs;
    nor->baudrate = config->baudrate;
    nor->xfer_mode = config->xfer_mode;
    nor->dev_mode = config->dev_mode;
    nor->host = host;

    nor->reg_proto = SNOR_PROTO_1_1_1;
    nor->addr_width = 4;
    nor->cmd_bytes = 2;
    nor->mst_slv_mode = true;

    memcpy(&(nor->info), &default_spi_nor_slaver_ids, sizeof(struct flash_info));

    nor->octal_dtr_en  = spi_nor_slaver_ids.read_proto & SNOR_DTR_PROTO;
    ssdk_printf(SSDK_CRIT, "nor->octal_dtr_en = %d \n", nor->octal_dtr_en);

    nor->dqs_en = 0;
    nor->phy_en = 0;

    if (nor->octal_dtr_en == 0) { //sdr mode
        if (nor->host->clk != NULL) {
            clk_set_rate(nor->host->clk, 20 * 1000 * 1000);
            ssdk_printf(SSDK_CRIT, "%d spinor host clock rate is %u!\n", __LINE__,
                        clk_get_rate(nor->host->clk));
        }
    }else{
        if (nor->host->clk != NULL) {
            clk_set_rate(nor->host->clk, 50 * 1000 * 1000);//ddr mode
            ssdk_printf(SSDK_CRIT, "%d spinor host clock rate is %u!\n", __LINE__,
                        clk_get_rate(nor->host->clk));
        }
    }

    ret  = ospi_master_read_slaver_id(nor);

    if (ret) {
        return ret;
    }

    if (nor->host->clk != NULL) {
        clk_set_rate(nor->host->clk, nor->host->ref_clk_hz);
        ssdk_printf(SSDK_CRIT, "%d spinor host clock rate is %u!\n", __LINE__,
                    clk_get_rate(nor->host->clk));
    }

    if (nor->octal_dtr_en == 0) {
        nor->dqs_en = 0;
        nor->phy_en = 1;
        ret = nor->host->ops->training(nor, 0, 0, 0, 0);
    }
    else {
        nor->dqs_en = 1;
        nor->phy_en = 1;
#ifdef TRAINING_RXDATA_TODO_TEST
        uint32_t training_addr = 0x500000;//slaver iram1
        ospi_master_indirect_write(nor, training_addr, sizeof(mst_training_pattern),
                                   mst_training_pattern);
        nor->host->ops->training(nor, training_addr, mst_training_buf,
                                 mst_training_pattern,
                                 sizeof(mst_training_buf));
#else
        ret = nor->host->ops->training(nor, 0, 0, 0,
                                       0);//mst-slv mode don't need to training
#endif
    }

    return ret;
}
#endif
