#ifndef _COMM_RPMSG_H__
#define _COMM_RPMSG_H__

#include "rpmsg_bsp/inc/iccmsg_ll.h"
#include "rpmsg_bsp/inc/iccmsg_dpr.h"
#include "rpmsg_bsp/inc/iccmsg_queue.h"
#include "rpmsg_bsp/inc/iccmsg_common.h"
#include "rpmsg_bsp/inc/iccmsg_hal.h"
#include <semaphore.h>

#define Rpmsg_CommOpen			HAL_CommOpen
#define Rpmsg_CommAccept		HAL_CommAccept
#define Rpmsg_CommRead			HAL_CommRead
#define Rpmsg_CommWrite			HAL_CommWrite
#define Rpmsg_CommControl		HAL_CommControl
#define Rpmsg_CommClose			HAL_CommClose

#define MAX_EVENTS                          256
#define DEFAULT_QUEUE_SIZE                  256

typedef struct RpmsgPortDriver
{
    iccmsg_hal_t*        stRpmsg_Hal;
    uint8_t                nFrame_Head;
    uint8_t                nFrame_Tail;
    char                szRemoteName[32];
    int                 nRemote_ept
        
}RPMEG_PORT_DRV;


#endif 