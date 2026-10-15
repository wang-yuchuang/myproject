/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_SPI_CFG_H_
#define BOARDS_SPI_CFG_H_

#ifdef CONFIG_SPI

//#define SPI1_CFG_ENABLE
//#define SPI2_CFG_ENABLE
//#define SPI3_CFG_ENABLE
#define SPI4_CFG_ENABLE
//#define SPI5_CFG_ENABLE
#define SPI6_CFG_ENABLE
//#define SPI7_CFG_ENABLE
//#define SPI8_CFG_ENABLE
//#define SPI9_CFG_ENABLE
//#define SPI10_CFG_ENABLE
//#define SPI11_CFG_ENABLE
//#define SPI12_CFG_ENABLE

extern void board_spi_init(void);
#endif

#endif /* BOARDS_SPI_CFG_H */

