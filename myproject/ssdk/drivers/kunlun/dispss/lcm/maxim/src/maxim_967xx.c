#include <stdlib.h>
#include "maxim_rw.h"
#include "maxim_967xx.h"

struct max_reg_data {
    uint16_t reg_addr;
    u8 val;
    uint16_t sleep;
};

static uint16_t max_serdes_video_transimt[MAX_NUM][1] = {
    [MAX96789] {
        0x02,
    },
};

static uint16_t max_ctrl_reg[MAX_NUM][1] = {
    [MAX96789] {
        0x10,
    },
    [MAX96772] {
        0x10,
    },
};

static uint16_t max_reg_mnl[MAX_NUM][1] = {
    [MAX96789] {
        0x12,
    },
};


static uint16_t max_serdes_lock_check[MAX_NUM][2] = {
    [MAX96789] {
        0x13,
        0x3,
    },
};

static uint16_t max_serdes_pclkdetct[MAX_NUM][4] = {
    [MAX96789] {
        0x102,
        0x10a,
        0x112,
        0x11a,
    },
};

static uint16_t max_dsi_port_en_reg[MAX_NUM][1] = {
    [MAX96789] {
        0x308,
    },
};

static uint16_t max_start_video_reg[MAX_NUM][1] = {
    [MAX96789] {
        0x311,
    }
};

static uint16_t max_mipi_phy_reg[MAX_NUM][1] = {
    [MAX96789] {
        0x330,
    },
};
static uint16_t max_mipi_phy_cfg[MAX_NUM][MIPI_PHY_CFG_MAX] = {
    [MAX96789] {
        0x4,
        0x5,
        0x6,
    }
};

static uint16_t max_mipi_lanes_reg[MAX_NUM][1] = {
    [MAX96789] {
        0x331,
    },
};

static uint16_t max_dsi_cfg[MAX_NUM][2] = {
    [MAX96789] {
        0x389,
        0x399,
    },
};

static uint16_t max_serdes_dsi_video_timing[MAX_NUM][14] = {
    [MAX96789] {
        0x0385,
        0x0386,
        0x0387,
        0x03A5,
        0x03A6,
        0x03A7,
        0x03A8,
        0x03A9,
        0x03AA,
        0x03AB,
        0x03AC,
        0x03AD,
        0x03AE,
        0x03A4,
    },
};

static uint16_t pipe_video_timing_reg[MAX_NUM][27] = {
    [MAX96789] {
        0x01CA,
        0x01CB,
        0x01CC,
        0x01CD,
        0x01CE,
        0x01CF,
        0x01D0,
        0x01D1,
        0x01D2,
        0x01D3,
        0x01D4,
        0x01D5,
        0x01D6,
        0x01D7,
        0x01D8,
        0x01D9,
        0x01DA,
        0x01DB,
        0x01DC,
        0x01DD,
        0x01DE,
        0x01DF,
        0x01E0,
        0x01E1,
        0x01E2,
        0x01E3,
        0x01E4,
    },
};

static uint16_t test_pattern_reg[MAX_NUM][5] = {
    [MAX96789] {
        0x01C8,
        0x01E5,
        0x01E6,
        0x0007,
    },
};

/* deserdes start */
static uint16_t stream_id[MAX_NUM][1] = {
    [MAX96789] {
        0x53,
    },
    [MAX96772] {
        0x50,
    },
};

static uint16_t link_training_state[MAX_NUM][2] = {
    [MAX96772] {
        0x7f0,
        0x7f1,
    },
};

static uint16_t dpcd_cmd_reg[MAX_NUM][2] = {
    [MAX96772] {
        0xE776,
        0xE777,
    },
};

static uint16_t dpcd_data_reg[MAX_NUM][4] = {
    [MAX96772] {
        0xE778,
        0xE779,
        0xE77A,
        0xE77C,
    },
};

static uint16_t dp_link_rate[MAX_NUM][2] = {
    [MAX96772] {
        0xE790,
        0xE791,
    },
};

static uint16_t dp_lane_cont[MAX_NUM][2] = {
    [MAX96772] {
        0xE792,
        0xE793,
    },
};


static uint16_t dp_link_trainning_timing[MAX_NUM][16] = {
    [MAX96772] {
        0xE794,
        0xE795,
        0xE796,
        0xE797,
        0xE798,
        0xE799,
        0xE79A,
        0xE79B,
        0xE79C,
        0xE79D,
        0xE79E,
        0xE79F,
        0xE7A0,
        0xE7A1,
        0xE7A2,
        0xE7A3,
    },
};
static uint16_t dp_link_trainning_hword[MAX_NUM][2] = {
    [MAX96772] {
        0xE7A4,
        0xE7A5,
     },
};
static uint16_t dp_link_trainning_mvid[MAX_NUM][2] = {
    [MAX96772] {
        0xE7A6,
        0xE7A7,
    },
};

#define DP_LINK_TRAINING_CONFIG_REG_NUM 22
static uint16_t dp_link_training_config[MAX_NUM][DP_LINK_TRAINING_CONFIG_REG_NUM] = {
    [MAX96772] {
        0xE7A8,
        0xE7A9,
        0xE7AA,
        0xE7AB,
        0xE7AC,
        0xE7AD,
        0xE7B0,
        0xE7B1,
        0xE7B2,
        0xE7B3,
        0xE7B4,
        0xE7B5,
        0xE7B6,
        0xE7B7,
        0xE7B8,
        0xE7B9,
        0xE7BA,
        0xE7BB,
        0xE7BC,
        0xE7BD,
        0xE7BE,
        0xE7BF,
    },
};
/* deserdes end */


void maxim_ldo_reg_enable(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr)
{
	uint8_t val = 0;
	maxim_read_reg(i2c_handle,slave_addr, max_ctrl_reg[ic_id][0], &val);
	val = val | 0x04;
	maxim_write_reg(i2c_handle,slave_addr,max_ctrl_reg[ic_id][0], val);
}

void maxim_ldo_reg_mnl(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr)
{
	uint8_t val = 0;
	maxim_read_reg(i2c_handle,slave_addr, max_reg_mnl[ic_id][0], &val);
	val = val | 0x10;
	maxim_write_reg(i2c_handle,slave_addr,max_reg_mnl[ic_id][0], val);
}

void maxim_reset_link(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, bool enable)
{
	uint8_t val = 0;
	maxim_read_reg(i2c_handle,slave_addr, max_ctrl_reg[ic_id][0], &val);
	if (enable)
		val = val | 0x40;
	else
		val &= ~(1 << 6);
	maxim_write_reg(i2c_handle,slave_addr,max_ctrl_reg[ic_id][0], val);
}


bool maxim_serdes_check_lock_check(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr)
{
    uint8_t val = 0;
    maxim_read_reg(i2c_handle,slave_addr,max_serdes_lock_check[ic_id][0], &val);
    if ((val >> max_serdes_lock_check[ic_id][1]) & 0x01)
        return true;
    else
        return false;
}

bool maxim_serdes_pclk_detect(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, uint8_t pipe)
{
    uint8_t val = 0;
    uint8_t reg = 0;

    switch (pipe) {
        case VIDEO_CHX:
            reg = 0;
            break;
        case VIDEO_CHY:
            reg = 1;
            break;
        case VIDEO_CHZ:
            reg = 2;
            break;
        case VIDEO_CHU:
            reg = 3;
            break;
        default:
            printf("input pipe:%d err\n", pipe);
            return false;
    }

    maxim_read_reg(i2c_handle,slave_addr,max_serdes_pclkdetct[ic_id][reg], &val);

    if (val & 0x80)
        return true;
    else
        return false;
}

void maxim_serdes_oneshot_rst(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr)
{
    uint8_t val = 0;
    maxim_read_reg(i2c_handle, slave_addr, max_ctrl_reg[ic_id][0], &val);

    val |= 0x20;
    maxim_write_reg(i2c_handle, slave_addr, max_ctrl_reg[ic_id][0], val);
}

void maxim_serdes_enable_video_transmit(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, uint8_t pipe, uint8_t enable)
{
    uint8_t val = 0;

    maxim_read_reg(i2c_handle, slave_addr, max_serdes_video_transimt[ic_id][0], &val);

    if (enable)
        val |= pipe;
    else
        val &= ~pipe;

    maxim_write_reg(i2c_handle, slave_addr, max_serdes_video_transimt[ic_id][0], val);
}

void maxim_port_dsi_enable(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr,
    enum dsi_port port, enum viedo_pipe pipe, bool enable)
{
    uint8_t val = 0;
    uint8_t offset = port + 4;
    maxim_read_reg(i2c_handle, slave_addr, max_dsi_port_en_reg[ic_id][0], &val);

    if (enable) {
        val |= 1 << offset;
        if (port == DSI_PORTA)
            val &= ~pipe;
        else
            val |= pipe;
    } else
        val &= ~(1 << offset);

    val |= 1 << 6;

    maxim_write_reg(i2c_handle, slave_addr, max_dsi_port_en_reg[ic_id][0], val);
}

void maxim_pipe_video_start(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr,
    enum dsi_port port, enum viedo_pipe pipe, bool enable)
{
    uint8_t val = 0;
    uint8_t offset = port * 4;
    maxim_read_reg(i2c_handle, slave_addr, max_start_video_reg[ic_id][0], &val);

    if (enable)
        val |= pipe << offset;
    else
        val &= ~ (pipe << offset);

    maxim_write_reg(i2c_handle, slave_addr, max_start_video_reg[ic_id][0], val);
}

void maxim_dsi_eotp_cfg(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr,
    enum dsi_port port, bool enable)
{
    uint8_t val = 0;

    maxim_read_reg(i2c_handle, slave_addr, max_dsi_cfg[ic_id][port], &val);

    if (enable)
        val &= ~(1 << 4);
    else
        val |= (1 << 4);

    maxim_write_reg(i2c_handle, slave_addr, max_dsi_cfg[ic_id][port], val);
}

void maxim_serdes_dsi_phy_cfg(i2c_adap_dev_t *i2c_handle, uint8_t ic_id,
    uint8_t slave_addr, enum mipi_phy_cfg cfg, uint8_t lane_num)
{
    uint8_t val = 0;

    lane_num -= 1;
    switch (cfg) {
        case MIPI_PHY_CFG_A_2x4:
            val |= lane_num;
            break;
        case MIPI_PHY_CFG_B_2x4:
            val |= (lane_num << 4);
            break;
        case MIPI_PHY_CFG_AB_2x4:
            val = (lane_num) | (lane_num << 4);
            break;
        default:
            LOGE("input phy cfg err\n");
            return;
    }
    val = (lane_num) | (lane_num << 4);

    maxim_write_reg(i2c_handle, slave_addr, max_mipi_phy_reg[ic_id][0], max_mipi_phy_cfg[ic_id][cfg]);
    maxim_write_reg(i2c_handle,slave_addr, max_mipi_lanes_reg[ic_id][0], val);
}

void maxim_serdes_dsi_en_dwshew(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr,struct display_timing *timing)
{
    uint8_t i = 0;
    uint16_t hfp = 0, hbp = 0, hsync = 0, hactive = 0;
    uint16_t vfp = 0, vbp = 0, vsync = 0, vactive = 0;
    hactive = timing->hactive;
    hfp = timing->hfront_porch;
    hbp = timing->hback_porch;
    hsync = timing->hsync_len;
    vactive = timing->vactive;
    vfp = timing->vfront_porch;
    vbp = timing->vback_porch;
    vsync = timing->vsync_len;

    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], hsync & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], vsync & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], (vsync >> 8 & 0xff) | (hsync >> 8 & 0xff));
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], vfp & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], (vbp & 0xf) | (vfp >> 8 & 0xff));
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], (vbp >> 4) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], vactive & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], (vactive >> 8) & 0xf);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], hfp & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], (hbp & 0xf) | (hfp >> 8 & 0xff));
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], (hbp >> 4) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], hactive & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i++], (hactive >> 8) & 0x1f);
    maxim_write_reg(i2c_handle,slave_addr, max_serdes_dsi_video_timing[ic_id][i], 0xc1);
}

void maxim_set_link_config(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, enum link_cfg cfg)
{
    u8 value = -1;

    maxim_read_reg(i2c_handle,slave_addr,max_ctrl_reg[ic_id][0], &value);
    value = value & 0xfc;
    value = value | cfg;

    maxim_write_reg(i2c_handle,slave_addr,max_ctrl_reg[ic_id][0], value);
}

void maxim_set_pipe_video_timing(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, enum viedo_pipe pipe, struct display_timing *timing)
{
    uint8_t i = 0, reg_offset = 0;
    uint16_t hfp = 0, hbp = 0, hsync = 0, hactive = 0;
    uint16_t vfp = 0, vbp = 0, vsync = 0, vactive = 0;
    uint16_t htotal = 0, vtotal = 0;
    uint16_t hs_low = 0, de_low = 0;
    uint32_t vs_high = 0, vs_low  = 0, v2d_time = 0;
    uint8_t offset = 0;

    hactive = timing->hactive;
    hfp = timing->hfront_porch;
    hbp = timing->hback_porch;
    hsync = timing->hsync_len;
    vactive = timing->vactive;
    vfp = timing->vfront_porch;
    vbp = timing->vback_porch;
    vsync = timing->vsync_len;

    htotal = hactive + hfp + hbp + hsync;
    vtotal = vactive + vfp + vbp + vsync;

    vs_high = vsync * htotal;   //0x1cd 0x1ce 0x1cf
    vs_low  = (vfp + vbp + vactive) * htotal; //0x1d0 0x1d1 0x1d2

    hs_low = htotal - hsync; //0x1d8 0x1d9
    v2d_time = (vsync + vbp)*htotal + hsync + hbp;
    de_low = hbp+hfp+hsync;


    if (pipe & VIDEO_PIPEX)
        offset = 0;
    else if (pipe & VIDEO_PIPEY)
        offset = 1;
    else if (pipe & VIDEO_PIPEZ)
        offset = 2;
    else if (pipe & VIDEO_PIPEU)
        offset = 3;
    else {
        LOGE("input video pipe:%d err\n", pipe);
        return;
    }

    reg_offset = offset * 0x43;

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, 0x00);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, 0x00);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, 0x00);                    //vsync delay

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (vs_high >> 16) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (vs_high >> 8) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, vs_high & 0xff);          //vsync high

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (vs_low >> 16) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (vs_low >> 8) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, vs_low & 0xff);           //vsync low

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, 0x00);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, 0x00);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, 0x00);                    //vsync to hsync

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (hsync >> 8) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, hsync & 0xff);            //hsync high

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (hs_low >> 8) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, hs_low & 0xff);           //hsync low

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (vtotal >> 8) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, vtotal & 0xff);           //hsync cnt

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (v2d_time >> 16) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (v2d_time >> 8) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, v2d_time & 0xff);         //vsync to de

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (hactive >> 8) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, hactive & 0xff);          //de high

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (de_low >> 8) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, de_low & 0xff);           //de low

    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, (vactive >> 8) & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, pipe_video_timing_reg[ic_id][i++] + reg_offset, vactive & 0xff);          //de cnt
};

/* deserdes */
void maxim_set_streamid(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, enum viedo_pipe pipe,uint8_t streameid)
{
    uint8_t reg = 0;
    uint8_t offset = 0;

    if (pipe & VIDEO_PIPEX)
        offset = 0;
    else if (pipe & VIDEO_PIPEY)
        offset = 1;
    else if (pipe & VIDEO_PIPEZ)
        offset = 2;
    else if (pipe & VIDEO_PIPEU)
        offset = 3;
    else {
        LOGE("input video pipe:%d err\n", pipe);
        return;
    }

    reg = stream_id[ic_id][0] + offset * 4;

    maxim_write_reg(i2c_handle,slave_addr, reg, streameid);
}

void maxim_set_test_pattern(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, struct display_timing *timing)
{
    uint8_t i = 0;

    maxim_set_streamid(i2c_handle, ic_id, slave_addr, VIDEO_PIPEX, 0);
    maxim_set_streamid(i2c_handle, ic_id, slave_addr, VIDEO_PIPEY, 1);
    maxim_set_streamid(i2c_handle, ic_id, slave_addr, VIDEO_PIPEZ, 2);
    maxim_set_streamid(i2c_handle, ic_id, slave_addr, VIDEO_PIPEU, 3);

    maxim_write_reg(i2c_handle,slave_addr, test_pattern_reg[ic_id][i++], 0x0);

    maxim_set_pipe_video_timing(i2c_handle,ic_id, slave_addr, VIDEO_PIPEX, timing);

    maxim_write_reg(i2c_handle,slave_addr, test_pattern_reg[ic_id][i++], 0x2);
    maxim_write_reg(i2c_handle,slave_addr, test_pattern_reg[ic_id][i++], 0x4);
    maxim_write_reg(i2c_handle,slave_addr, test_pattern_reg[ic_id][i++], 0x0);

    maxim_write_reg(i2c_handle,slave_addr, test_pattern_reg[ic_id][0], 0xE3);
}

void maxim_link_training_timing_config(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, struct display_timing *timing)
{
    uint8_t i = 0;
    uint16_t hfp = 0, hbp = 0, hsync = 0, hactive = 0;
    uint16_t vfp = 0, vbp = 0, vsync = 0, vactive = 0;
    hactive = timing->hactive;
    hfp = timing->hfront_porch;
    hbp = timing->hback_porch;
    hsync = timing->hsync_len;
    vactive = timing->vactive;
    vfp = timing->vfront_porch;
    vbp = timing->vback_porch;
    vsync = timing->vsync_len;


    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], hactive & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], (hactive >> 8) & 0xff);

    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], hfp & 0xff); //hfp
    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], (hfp >> 8) & 0xff);

    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], hsync & 0xff);//hsync
    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], (hsync >> 8) & 0xff);

    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], hbp & 0xff);//hbp
    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], (hbp >> 8) & 0xff);

    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], vactive & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], (vactive >> 8) & 0xff);

    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], vfp & 0xff);//vfp
    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], (vfp >> 8) & 0xff);

    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], vsync & 0xff);//vsync
    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], (vsync >> 8) & 0xff);

    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i++], vbp & 0xff);//vbp
    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_timing[ic_id][i], (vbp >> 8) & 0xff);
}

void maxim_dp_link_training_hword_set(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, uint16_t hactive, uint16_t n_lane, uint16_t bpp)
{
    u32 hword  = 0;
    uint8_t i = 0;
    hword = hactive * bpp / 16 - n_lane;

    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_hword[ic_id][i++], hword & 0xff);
    maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_hword[ic_id][i++], (hword >> 8) & 0xff);
}

void maxim_dp_link_training_mvid_set(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, uint16_t pclk, uint16_t link_rate_x100)
{
    uint8_t i = 0;
    u32 mvid = 0;

    mvid = pclk * 0x8000 / (link_rate_x100);

	maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_mvid[ic_id][i++], mvid & 0xff);
	maxim_write_reg(i2c_handle,slave_addr, dp_link_trainning_mvid[ic_id][i++], (mvid >> 8) & 0xff);
}

void maxim_dp_set_link_rate(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, enum edp_link_rate link_rate)
{
    maxim_write_reg(i2c_handle,slave_addr, dp_link_rate[ic_id][0], link_rate);
    maxim_write_reg(i2c_handle,slave_addr, dp_link_rate[ic_id][1], 0x00);
}

void maxim_dp_set_lane_cont(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, uint8_t lane_cont)
{
    maxim_write_reg(i2c_handle,slave_addr, dp_lane_cont[ic_id][0], lane_cont);
    maxim_write_reg(i2c_handle,slave_addr, dp_lane_cont[ic_id][1], 0x00);
}

void maxim_dp_dpcd_cmd(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, enum link_cmd_type cmd)
{
    maxim_write_reg(i2c_handle,slave_addr, dpcd_cmd_reg[ic_id][0], cmd);
    maxim_write_reg(i2c_handle,slave_addr, dpcd_cmd_reg[ic_id][1], 0x80);
}

uint8_t maxim_dp_dpcd_read_data(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, uint16_t dpcd_reg)
{
    u8 val = 0;

    val = dpcd_reg & 0xff;
    maxim_write_reg(i2c_handle,slave_addr, dpcd_data_reg[ic_id][0], val);

    val = (dpcd_reg >> 8) & 0xff;
    maxim_write_reg(i2c_handle,slave_addr, dpcd_data_reg[ic_id][1], val);

    maxim_write_reg(i2c_handle,slave_addr, dpcd_cmd_reg[ic_id][0], LINK_CMD_READ_DPCD_REG);
    maxim_write_reg(i2c_handle,slave_addr, dpcd_cmd_reg[ic_id][1], 0x80);

retry:
    val = 0;
    udelay(100 * 1000);
    maxim_read_reg(i2c_handle,slave_addr, dpcd_cmd_reg[ic_id][1], &val);
    if (val&0x80)
        goto retry;

    val = 0;
    maxim_read_reg(i2c_handle,slave_addr, dpcd_data_reg[ic_id][2], &val);
    return val;
}

void maxim_dp_link_training_config(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr, struct display_timing *timing)
{
    uint8_t i = 0;

    u8 training_val[] = {
        0x00,
        0x80,
        0x40,
        0x00,
        0x00,
        0x00,
        0x01,
        0x00,
        0x50,
        0x00,
        0x00,
        0x40,
        0x6C,
        0x20,
        0x07,
        0x00,
        0x01,
        0x00,
        0x00,
        0x00,
        0x52,
        0x00,
    };
    maxim_link_training_timing_config(i2c_handle, ic_id, slave_addr, timing);

    for (i = 0; i < DP_LINK_TRAINING_CONFIG_REG_NUM; i++) {
        maxim_write_reg(i2c_handle,slave_addr, dp_link_training_config[ic_id][i], training_val[i]);
    }
}

bool maxim_dp_get_link_training_state(i2c_adap_dev_t *i2c_handle, uint8_t ic_id, uint8_t slave_addr)
{
    uint8_t val_h = 0, val_l = 0;
    uint16_t result = 0;

    maxim_read_reg(i2c_handle,slave_addr, link_training_state[ic_id][0], &val_l);
    maxim_read_reg(i2c_handle,slave_addr, link_training_state[ic_id][1], &val_h);

    result = (uint16_t)(val_h << 8) | val_l;
    if (result == 0x1)
        return true;
    else
        return false;
}

