/*
 * mipicsi-os.h
 *
 * Semidrive platform mipi csi2 header file
 *
 * Copyright (C) 2022, Semidrive  Communications Inc.
 *
 * This file is licensed under a dual GPLv2 or X11 license.
 */
/**
 * @file mipicsi_os.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: mipicsi driver interface for os
 *
 * Revision History:
 * -----------------
 */
#ifndef __SDRV_MIPI_CSI_OS_H__
#define __SDRV_MIPI_CSI_OS_H__

#define MAX_MIPICSI_INST 2

int mipicsi_init(uint32_t id, void *data);
int mipicsi_stream(uint32_t id, int on);

#endif
