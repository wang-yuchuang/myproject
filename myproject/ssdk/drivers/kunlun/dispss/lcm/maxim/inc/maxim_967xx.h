#ifndef _MAXIM_DSI_H
#define _MAXIM_DSI_H

#include "sys/types.h"
#include "sdm_panel.h"

#ifndef u16
#define u16 unsigned short
#endif

enum {
    MAX96789 = 0,
    MAX96772,
    MAX_NUM,
};

enum {
    VIDEO_CHX = 0x10,
    VIDEO_CHY = 0x20,
    VIDEO_CHZ = 0x40,
    VIDEO_CHU = 0x80,
    VIDEO_CH_ALL = 0xf0,
};

enum viedo_pipe {
    VIDEO_PIPEX = 0x1,
    VIDEO_PIPEY = 0x2,
    VIDEO_PIPEZ = 0x4,
    VIDEO_PIPEU = 0x8,
    VIDEO_PIPEALL = 0xf,
};

enum dsi_port {
    DSI_PORTA = 0,
    DSI_PORTB,
};

enum mipi_phy_cfg {
    MIPI_PHY_CFG_A_2x4 = 0,
    MIPI_PHY_CFG_B_2x4,
    MIPI_PHY_CFG_AB_2x4,
    MIPI_PHY_CFG_MAX,
};

enum link_cfg{
    LINK_CFG_DUAL  = 0x00,
    LINK_CFG_LINKA = 0x01,
    LINK_CFG_LINKB = 0x02,
    LINK_CFG_SPLITTER = 0x3,
    LINK_CFG_AUTO = 0x11,
};


enum edp_link_rate{
    LINK_RATE_1G_62 = 0x6,
    LINK_RATE_2G_16 = 0x8,
    LINK_RATE_2G_43 = 0x9,
    LINK_RATE_2G_7  = 0xA,
    LINK_RATE_3G_24 = 0xC,
    LINK_RATE_4G_32 = 0x10,
    LINK_RATE_5G_4  = 0x1A,
    LINK_RATE_8G_1  = 0x1E,
};

enum link_cmd_type{
    LINK_CMD_REBOOT_TRAINING = 0x1,
    LINK_CMD_RUN_LINK_TRAINING = 0x2,
    LINK_CMD_READ_DPCD_REG = 0x10,
    LINK_CMD_WRITE_DPCD_REG = 0x20,
};

#define MEXIM_SERDES_LOG 1

void maxim_ldo_reg_enable(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr);
void maxim_ldo_reg_mnl(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr);
void maxim_reset_link(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, bool enable);
bool maxim_serdes_check_lock_check(i2c_adap_dev_t *adap, uint8_t serdes_id, uint8_t slave_addr);
void maxim_serdes_dsi_en_dwshew(i2c_adap_dev_t *adap, uint8_t serdes_id, uint8_t slave_addr,struct display_timing *timing);
void maxim_serdes_oneshot_rst(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr);
void maxim_serdes_enable_video_transmit(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, uint8_t pipe, uint8_t enable);
bool maxim_serdes_pclk_detect(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, uint8_t pipe);
void maxim_set_link_config(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, enum link_cfg cfg);
void maxim_set_test_pattern(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, struct display_timing *timing);
void maxim_set_streamid(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, enum viedo_pipe pipe,uint8_t streameid);
void maxim_serdes_dsi_phy_cfg(i2c_adap_dev_t *adap, uint8_t ic_id,
    uint8_t slave_addr, enum mipi_phy_cfg cfg, uint8_t lane_num);
void maxim_port_dsi_enable(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr,
    enum dsi_port port, enum viedo_pipe pipe, bool enable);
void maxim_pipe_video_start(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr,
    enum dsi_port port, enum viedo_pipe pipe, bool enable);
void maxim_dsi_eotp_cfg(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr,
    enum dsi_port port, bool enable);

void maxim_dp_link_training_hword_set(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, u16 hactive, u16 n_lane, u16 bpp);
void maxim_dp_link_training_mvid_set(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, u16 pclk, u16 link_rate_100);
void maxim_dp_set_link_rate(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, enum edp_link_rate link_rate);
void maxim_dp_set_lane_cont(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, uint8_t lane_cont);
void maxim_dp_dpcd_cmd(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, enum link_cmd_type cmd);
void maxim_dp_link_training_config(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, struct display_timing *timing);
bool maxim_dp_get_link_training_state(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr);
uint8_t maxim_dp_dpcd_read_data(i2c_adap_dev_t *adap, uint8_t ic_id, uint8_t slave_addr, uint16_t dpcd_reg);

#endif

