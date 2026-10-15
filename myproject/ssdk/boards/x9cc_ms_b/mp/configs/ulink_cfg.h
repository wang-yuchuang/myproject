#ifndef __ULINK_CFG_HEAD__
#define __ULINK_CFG_HEAD__

#include "image_cfg.h"

#define LINK_AB_PCIE   0x18000000
#define LINK_AB_DDR_A5 (SERVICES_MEMBASE)
#define LINK_AB_DDR_R5 (LINK_AB_DDR_A5 - 0x10000000)

#define CLIENT_BASE   LINK_AB_PCIE
#define SERVER_BASE   LINK_AB_DDR_R5

#define ULINK_GPIO_TX -1
#define ULINK_GPIO_RX -1

#define DEFAULT_RPROC 3
#endif //__ULINK_HEAD__
