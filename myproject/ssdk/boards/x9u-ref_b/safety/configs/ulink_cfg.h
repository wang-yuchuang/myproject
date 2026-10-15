#ifndef __ULINK_CFG_HEAD__
#define __ULINK_CFG_HEAD__

#include "image_cfg.h"

#define round_up(a,b) (((a - 1) | (b - 1)) + 1)

#define LINK_AB_PCIE   0x18000000
#define LINK_AB_SIZE   0x4000000
#define LINK_AB_DDR_A5 (round_up(SERVICES_MEMBASE, LINK_AB_SIZE))
#define LINK_AB_DDR_R5 (LINK_AB_DDR_A5 - 0x10000000)

#define CLIENT_BASE   LINK_AB_PCIE
#define SERVER_BASE   LINK_AB_DDR_R5

#define ULINK_GPIO_TX 28  /* GPIO A4 */
#define ULINK_GPIO_RX 29  /* GPIO A5 */

#define DEFAULT_RPROC 9
#endif //__ULINK_HEAD__
