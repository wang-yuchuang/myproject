/**
 * @file disp_panels.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __DISP_LCM_H__
#define __DISP_LCM_H__
#include "disp_hal.h"
#include "sdm_panel.h"
#include <driver/i2c.h>
#ifdef SUPPORT_BOARDINFO
#include <boardinfo_hwid_usr.h>
#endif

extern struct sdm_panel mipi_hsd123_serdes_1920x720_lcd;
extern struct sdm_panel mipi_hsd123_serdes_1920x720_2_lcd;
extern struct sdm_panel lvds_hsd123_serdes_1920x720_lcd;
extern struct sdm_panel lvds_hsd101_serdes_1280x800_lcd;
extern struct sdm_panel lvds_kd070_serdes_1024x600_lcd;
extern struct sdm_panel lvds_hsd123_serdes_1920x720_x9u_b_lcd;
extern struct sdm_panel lvds_WallfacerA2_hsd123_serdes_1920x720_lcd;
extern struct sdm_panel mipi_WallfacerA2_hsd123_serdes_1920x720_lcd;
extern struct sdm_panel mipi_WallfacerA2_hsd123_serdesl_1920x720_2_lcd;
extern struct sdm_panel mipi_WallfacerA_hsd160_serdes_2560x1600_lcd;
extern struct sdm_panel lvds_kd070_max_serdes_1024x600_lcd;
extern struct sdm_panel lvds_lmt070_serdes_800x480_lcd;

extern struct sdm_panel_info **registered_panels[];

int sdm_panel_probe(display_handle *handle, int sub_id, struct sdm_panel_info *panels[], int n_panels);
int sdm_connect_panel(display_handle *handle, int sub_id, struct sdm_panel *panel);
int sdm_panel_disconnect(display_handle *handle, int sub_id, struct sdm_panel *panel);
bool is_need_checklink(i2c_adap_dev_t *adap, uint32_t slave_addr, uint8_t reg);
int panel_check_link(struct sdm_panel_info * panel_info);
int sdm_panel_default_pwmbl_init(struct backlight_info *backlight, int dispid);
#endif