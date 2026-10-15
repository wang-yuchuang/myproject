/*
 * CLI_task.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: console function.
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
//#include <malloc.h>
#include  <string.h>
#include <param.h>
#include <board.h>
#include <reg.h>
#include <types.h>

#include "driver/kunlun/tmr/sdrv_tmr_drv.h"
#include "driver/timer.h"

#include "cmsis_os2.h"
#include "CLI_type.h"
//#include "driver/gmac/net.h"

#include "driver/kunlun/eth/dwc_eth.h"
#include "driver/kunlun/eth/phy.h"

static bool g_eth1_rx_pkt;

extern int dwc_eth1_rx(uint8_t *eth_rx_buf, bool poll);
extern int dwc_eth1_txavail(uint8_t *tx_buf, int len);
static uint8_t tx_buf[300];

static void eth_dma_rx_int_cb(dwc_eth_config_t *eth_config)
{

  ssdk_printf(SSDK_ERR, "example_send eth_dma_rx_int_cb ...\n");
        g_eth1_rx_pkt = true;

}

static int Gmac_sendCommand(int argc, char *argv[])
{

		bool eth_pool_rx = false;
		//bool eth_pool_tx = false;

		//char rx_buf[30] ={0};
		static uint8_t v = 0;

		ssdk_printf(SSDK_EMERG, "****gmac_send operation**\n");

    if (!eth_pool_rx) {

        //dwc_eth_enable_dma_rx_int(eth_dma_rx_int_cb);
    }
	osDelay(10);
    //for (;;)
	{

	   memset(tx_buf, v++, sizeof(tx_buf));

          dwc_eth1_txavail(tx_buf, sizeof(tx_buf));

	}

    return CLI_FALSE;
}

CLI_CMD("eth-send", "gmac send msg out", Gmac_sendCommand);
