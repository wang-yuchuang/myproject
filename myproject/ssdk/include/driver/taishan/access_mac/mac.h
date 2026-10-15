/*
 * access_mac.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ACCESS/MAC(master domain assignment controller) driver header.
 *
 * Revision History:
 * -----------------
 */
#ifndef _MAC_H_
#define _MAC_H_
#include "mac_reg.h"
#include "regs_base.h"

#define MAC_BASE APB_MAC_BASE
#define MAC_MDA_ADDR(x) (MAC_BASE + MDA_OFF(x))
#define MAC_MAA_ADDR(x) (MAC_BASE + MAA_OFF(x))

void mac_set_mda(uint32_t msid, uint32_t did);
void mac_set_acs(uint32_t did);
void mac_unlock_mda(uint32_t msid);
void mac_set_maa_pri(uint32_t msid, uint32_t data);
void mac_set_maa_secure(uint32_t msid, uint32_t data);
#endif /*_MAC_H_*/