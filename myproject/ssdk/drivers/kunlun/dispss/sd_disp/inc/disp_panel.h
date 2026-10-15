/**
 * @file disp_panel.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __DISP_PANEL_H__
#define __DISP_PANEL_H__

#include <sdm_panel.h>

int mipi_lcm_init(int dsi_index, struct sdm_panel *panel);

#endif //__DISP_PANEL_H__