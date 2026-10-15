#include "sdm_panel.h"
#include "disp_panels.h"
#include <string.h>
#include "maxim_rw.h"
#include "maxim_967xx.h"

static struct display_timing timing = {
    .hactive = 2560,
    .hfront_porch = 84,
    .hback_porch = 60,
    .hsync_len = 32,
    .vactive = 1600,
    .vfront_porch = 3,
    .vback_porch = 24,
    .vsync_len = 5,
    .pll_clk = 1890000000,
};

static struct info_mipi mipi_data = {
    .burst_mode = 0,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 1620000,
};


static int panel_pre_power_on(struct sdm_panel_info *panel_info)
{
    i2c_adap_dev_t *i2c_handle = panel_info->serdes->adap;
    uint32_t max96789_i2c_addr = panel_info->serdes->ser_addr;
    uint32_t max96772_i2c_addr = panel_info->serdes->des_addr;

    maxim_set_link_config(i2c_handle, MAX96772, max96772_i2c_addr, LINK_CFG_DUAL);
    maxim_ldo_reg_enable(i2c_handle, MAX96789, max96789_i2c_addr);
    maxim_ldo_reg_mnl(i2c_handle, MAX96789, max96789_i2c_addr);
    maxim_reset_link(i2c_handle, MAX96789, max96789_i2c_addr, true);
    maxim_set_link_config(i2c_handle, MAX96789, max96789_i2c_addr, LINK_CFG_DUAL);

    udelay(150 * 1000);

    maxim_reset_link(i2c_handle, MAX96789, max96789_i2c_addr, false);

    maxim_serdes_enable_video_transmit(i2c_handle, MAX96789, max96789_i2c_addr, VIDEO_CH_ALL, false);

    maxim_serdes_dsi_phy_cfg(i2c_handle, MAX96789, max96789_i2c_addr, MIPI_PHY_CFG_A_2x4, 4);
    maxim_port_dsi_enable(i2c_handle, MAX96789, max96789_i2c_addr, DSI_PORTB, VIDEO_PIPEY, false);
    maxim_port_dsi_enable(i2c_handle, MAX96789, max96789_i2c_addr, DSI_PORTA, VIDEO_PIPEX, true);
    maxim_pipe_video_start(i2c_handle, MAX96789, max96789_i2c_addr, DSI_PORTB, VIDEO_PIPEALL , false);
    maxim_pipe_video_start(i2c_handle, MAX96789, max96789_i2c_addr, DSI_PORTA, VIDEO_PIPEX, true);
    maxim_dsi_eotp_cfg(i2c_handle, MAX96789, max96789_i2c_addr, DSI_PORTA, false);

    return 0;
}

static int panel_post_begin(struct sdm_panel_info *panel_info)
{
    i2c_adap_dev_t *i2c_handle = panel_info->serdes->adap;
    uint32_t max96789_i2c_addr = panel_info->serdes->ser_addr;
    uint32_t max96772_i2c_addr = panel_info->serdes->des_addr;

    /************************************************************************************************/
    /* 3. Config 96772                                      */
    /************************************************************************************************/

    maxim_serdes_enable_video_transmit(i2c_handle, MAX96789, max96789_i2c_addr, VIDEO_CH_ALL, true);

    udelay(100 * 1000);

    maxim_set_streamid(i2c_handle, MAX96772, max96772_i2c_addr, VIDEO_PIPEX, 0);

    maxim_dp_link_training_config(i2c_handle, MAX96772, max96772_i2c_addr, &timing);
    maxim_dp_set_link_rate(i2c_handle, MAX96772, max96772_i2c_addr, LINK_RATE_2G_7);
    maxim_dp_set_lane_cont(i2c_handle, MAX96772, max96772_i2c_addr, 4);
    maxim_dp_link_training_mvid_set(i2c_handle, MAX96772, max96772_i2c_addr, timing.pll_clk/7000000, 270);
    maxim_dp_link_training_hword_set(i2c_handle, MAX96772, max96772_i2c_addr, timing.hactive ,4, 24);
    maxim_dp_dpcd_cmd(i2c_handle, MAX96772, max96772_i2c_addr, LINK_CMD_RUN_LINK_TRAINING);

    //maxim_set_test_pattern(i2c_handle, MAX96789, max96789_i2c_addr, &timing);

    //for backlight configuration, max96789 gpio3 pass through max96772 gpio3
    maxim_write_reg(i2c_handle, max96789_i2c_addr, 0x2C7, 0x03);
    maxim_write_reg(i2c_handle, max96772_i2c_addr, 0x2B9, 0x90);//0x04

    return 0;
}

static int panel_post_end(struct sdm_panel_info *panel_info)
{
    return 0;
}

struct sdm_panel mipi_WallfacerA_hsd160_serdes_2560x1600_lcd = {
    .panel_name = "mipi_WallfacerA_hsd160_serdes_2560x1600_lcd",
    .if_type = IF_TYPE_DSI,
    .fps = 60,
    .display_timing = &timing,
    .mipi = &mipi_data,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
    .panel_pre_power_on = panel_pre_power_on,
};

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

static int maxim_write_reg1(void* i2c_handle, u8 address, u16 reg, u8 val)
{
    u8 buf[3];
    int ret = -1;

    buf[0] = reg >> 8;
    buf[1] = reg & 0xff;
    buf[2] = val;

    ret = hal_i2c_write_reg_data(i2c_handle, address, buf, 2, buf + 2, 1);
    //printf("max_write:psg-96755/752 :addr=0x%x reg=0x%x, val=%x,ret=%x\n",
                //address,reg, val,ret);
    if (ret < 0) {
        dprintf(CRITICAL, "%s:ivi-96755/752 error:addr=0x%x reg=0x%x, val=%x\n",
                __func__, address,reg, val);
        return ret;
    }

    return ret;
}

static int maxim_read_reg1(void* i2c_handle, u8 address, u16 reg, u8 *val)
{
    u8 buf[2];
    int ret = -1;
    u8 addr[2];

    memset(buf, 0, sizeof(buf));
    addr[0] = reg >> 8;
    addr[1] = reg & 0xff;

    ret = hal_i2c_read_reg_data(i2c_handle, address, addr, 2, buf, 1);
    if (ret < 0) {
        dprintf(CRITICAL, "%s:ivi-96755/752 error:read addr=0x%x reg=0x%x,ret=%d\n", __func__,address, reg,ret);
        return ret;
    }

    *val = buf[0];
    return 0;
}

static int write_max_reg_data(uint8_t addr, uint16_t reg, uint8_t val)
{
    void *i2c_handle;

    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C16);

    maxim_write_reg1(i2c_handle, addr, reg, val);

    printf("\n write dev:0x%x reg:0x%04x -> 0x%02x\n", addr, reg, val);
    hal_i2c_release_handle(i2c_handle);
    return 0;
}

static int read_max_dpcd_reg_data(uint16_t reg)
{
    void *i2c_handle;
    uint8_t val = 0;
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C16);

    val = maxim_dp_dpcd_read_data(i2c_handle, MAX96772, 0x48, reg);

    printf("\n reg %x -> 0x%02x\n", reg, val);
    hal_i2c_release_handle(i2c_handle);
    return 0;
}


static int read_max_register(uint8_t addr, uint16_t reg)
{
    u8 val = 0;
    void *i2c_handle;

    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C16);

    val = 0;
    maxim_read_reg1(i2c_handle, addr, reg, &val);    /* get hs\vs\de check */

    printf("read 0x%04x=0x%02x \n", reg, val);

    hal_i2c_release_handle(i2c_handle);

    return 0;
}

static void max_aux_conf(uint8_t addr, uint16_t val1, uint16_t val2)
{
    write_max_reg_data(addr, 0xE778, val2);
	write_max_reg_data(addr, 0xE779, val1);
	write_max_reg_data(addr, 0xE776, 0x10);
	write_max_reg_data(addr, 0xE777, 0x80);
	read_max_register(addr, 0xE77A);
	read_max_register(addr, 0xE77B);
}

static int dump_serdes_register(int argc, const cmd_args *argv)
{
    uint8_t addr = argv[1].u;
    uint16_t reg = argv[2].u;

    if (argc == 3) {
        read_max_register(addr, reg);
    }
    else if (argc == 4) {
        uint8_t val = argv[3].u;
        write_max_reg_data(addr, reg, val);
    }
    else if (argc == 2) {
        uint16_t reg = argv[1].u;
        read_max_dpcd_reg_data(reg);
    }
    else if (argc == 5) {
        uint16_t val1 = argv[3].u;
		uint16_t val2 = argv[4].u;
        max_aux_conf(addr, val1, val2);
    }

    return 0;
}


STATIC_COMMAND_START
STATIC_COMMAND("dump_serdes", "dump serdes register",  (console_cmd)&dump_serdes_register)
STATIC_COMMAND_END(dump_serdes);
#endif
