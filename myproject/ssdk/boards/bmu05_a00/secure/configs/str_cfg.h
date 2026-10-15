/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef STR_CFG_H
#define STR_CFG_H

#ifdef CONFIG_SUPPORT_STR_MODE
extern void power_config_before_str(void);
extern void power_config_before_shutdown(void);
#endif

#endif
