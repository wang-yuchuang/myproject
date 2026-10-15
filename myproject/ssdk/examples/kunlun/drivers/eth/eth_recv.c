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

#include "driver/kunlun/eth/dwc_eth.h"
#include "driver/kunlun/eth/phy.h"

static bool g_eth1_rx_pkt;
static uint8_t *eth1_rx_buf;
extern int dwc_eth1_rx(uint8_t *eth_rx_buf, bool poll);
static char rx_buf[30] ={0};

static void eth_dma_rx_int_cb(dwc_eth_config_t *eth_config)
{

  ssdk_printf(SSDK_INFO, "eth_dma_rx_int_cb ...\n");
  g_eth1_rx_pkt = true;

}

static int Gmac_recvCommand(int argc, char *argv[])
{

	bool eth_pool_rx = false;

	ssdk_printf(SSDK_INFO, "****eth_recv operation**\n");

    eth1_rx_buf = (uint8_t *)malloc(EQOS_MAX_PACKET_SIZE);

    if (!eth_pool_rx) {

        dwc_eth_enable_dma_rx_int(eth_dma_rx_int_cb);
    }

    for (;;) {

       if ((!eth_pool_rx && g_eth1_rx_pkt) || eth_pool_rx)
       {
		int recv_len;

            g_eth1_rx_pkt = false;

            recv_len = dwc_eth1_rx(eth1_rx_buf, eth_pool_rx);

            memcpy(rx_buf, eth1_rx_buf, 30);

            ssdk_printf(SSDK_EMERG, "eth1_rx_buf : len=%d,dat=%x\n", recv_len,rx_buf[20]);
        }

		osDelay(1);
    }

    return CLI_FALSE;
}

CLI_CMD("eth-recv", "recv from gmac xx byte mes", Gmac_recvCommand);
