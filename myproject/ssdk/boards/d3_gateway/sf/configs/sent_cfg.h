
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_SENT_CFG_H
#define BOARDS_SENT_CFG_H

#ifdef CONFIG_SENT

#include <driver/sent.h>

#define SENT_DEV_NUM 2

/* exported timer devices */
extern sent_dev_t *g_sent[SENT_DEV_NUM];

/* sent init functions */
extern void board_sent_init(void);

#endif /* CONFIG_SENT */

#endif /* BOARDS_SENT_CFG_H */
