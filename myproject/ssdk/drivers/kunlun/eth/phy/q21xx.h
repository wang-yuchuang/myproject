/*
* q21xx.h
*
* Copyright (c) 2021 Semidrive Semiconductor.
* All rights reserved.
*
* Description: eth phy q21xx drv head file
*
* Revision History:
* -----------------
*/
#ifndef __DW_ADC_H__
#define __DW_ADC_H__

#include "driver/kunlun/eth/phy.h"

// 1000 BASE-T1 Operating mode
#define MRVL_88Q211X_MODE_LEGACY    0x06B0
#define MRVL_88Q211X_MODE_DEFAULT   0x0000
#define MRVL_88Q211X_MODE_ADVERTISE 0x0002

// Current Speed
#define MRVL_88Q2112_1000BASE_T1    0x0001
#define MRVL_88Q2112_100BASE_T1     0x0000

typedef enum {
    /* Transceiver disabled */
    ETH_PHY_MODE_DOWN = 0x00,
    /* transceiver enabled */
    ETH_PHY_MODE_ACTIVE
} eth_phy_mode_type;

typedef enum {
    /* No physical Ethernet connection established */
    ETH_PHY_LINK_STATE_DOWN = 0x00,
    /* Physical Ethernet connection established */
    ETH_PHY_LINK_STATE_ACTIVE
} eth_phy_linkstate_type;

typedef enum {
    /* Driver is not yet configured */
    ETH_PHY_STATE_UNINIT = 0,
    /* 0x01 Driver is configured */
    ETH_PHY_STATE_INIT

} eth_phy_state_type;

typedef enum {
    /* 10MBIT Ethernet connection */
    ETH_PHY_BAUD_RATE_10MBIT = 0,
    /* 0x01 100MBIT Ethernet connection */
    ETH_PHY_BAUD_RATE_100MBIT,
    /* 0x02 1000MBIT Ethernet connection */
    ETH_PHY_BAUD_RATE_1000MBIT
} eeth_phy_baudrate_type;

typedef enum {
    /* normal operation */
    ETH_PHY_PHYTESTMODE_NONE = 0,
    /* 0x01 test transmitter droop */
    ETH_PHY_PHYTESTMODE_1,
    /* 0x02 test master timing jitter */
    ETH_PHY_PHYTESTMODE_2,
    /* 0x03 test slave timing jitter */
    ETH_PHY_PHYTESTMODE_3,
    /* 0x04 test transmitter distortion */
    ETH_PHY_PHYTESTMODE_4,
    /* 0x05 test power spectral density (PSD) mask */
    ETH_PHY_PHYTESTMODE_5
} eth_phy_test_mode_type;

typedef enum {
    /* 0x00 normal operation */
    ETH_PHY_PHYLOOPBACK_NONE = 0,
    /* 0x01 internal loopback */
    ETH_PHY_PHYLOOPBACK_INTERNAL,
    /* external loopback */
    ETH_PHY_PHYLOOPBACK_EXTERNAL,
    /* remote loopback */
    ETH_PHY_PHYLOOPBACK_REMOTE
} eth_phy_loopback_mode_type;

typedef enum {
    /* normal operation */
    ETH_PHY_PHYTXMODE_NORMAL = 0,
    /* 0x01 transmitter disabled */
    ETH_PHY_PHYTXMODE_TX_OFF,
    /* 0x02 scrambler disabled */
    ETH_PHY_PHYTXMODE_SCRAMBLER_OFF
} eth_phy_tx_mode_type;

int32_t ma211x_check_link(phy_dev_t *dev);
int32_t ma211x_get_speed(phy_dev_t *dev);
int32_t ma211x_config_init(phy_dev_t *dev);

void enable_transmitters(phy_dev_t *dev);
void disable_transmitters(phy_dev_t *dev);
void set_tx_mode(phy_dev_t *dev, uint32_t mode);
uint32_t get_signal_quality(phy_dev_t *dev);
void set_loopback_mode(phy_dev_t *dev, uint32_t mode);
int32_t set_test_mode(phy_dev_t *dev, uint32_t mode);
uint32_t phy_is_duplex_mode(phy_dev_t *dev);

#endif

