
//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_FEE_CFG_H
#define BOARDS_FEE_CFG_H

#if defined(CONFIG_FEE_BLOCK) || defined(CONFIG_FEE_EEPROM)

#include <lib/fee/fee_block.h>
#include <lib/fee/fee_eeprom.h>

/* fee init functions */
extern void board_fee_init(void);

#endif

#endif /* BOARDS_FEE_CFG_H */
