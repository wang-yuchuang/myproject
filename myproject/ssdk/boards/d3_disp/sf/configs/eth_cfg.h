#ifndef _ETH_CFG_H_
#define _ETH_CFG_H_

#ifdef CONFIG_ETH

extern struct net_driver_s g_eth1_dev;

void board_eth_init(void);

#endif

#endif