/*
 * property_cfg.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef __PROPERTY_CFG_H__
#define __PROPERTY_CFG_H__
#ifdef CONFIG_PROPERTY
#include <sd_property.h>

extern struct sys_property_value ss_properties[];
extern int board_property_get_this_proc();
extern int board_property_get_array_size();

#endif /* CONFIG_PROPERTY*/
#endif /* __PROPERTY_CFG_H__ *//* EOF */
