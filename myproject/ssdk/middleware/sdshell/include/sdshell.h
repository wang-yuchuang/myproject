#ifndef __SDSHELL_SERVICE_H__
#define __SDSHELL_SERVICE_H__
#include "driver/kunlun/virt_uart/virt_uart.h"
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <rpc/rpmsg_common.h>

#define SDSHELL_SERVICE_NAME        "sdshell-service"
#define SDSHELL_MP_NAME             "sdshell-mp"
#define SDSHELL_CMDBUF_LEN          (128)
#define SDSHELL_TX_STACK_SIZE       (1536)

#define CPSR_MODE_MASK              0x1f
#define CPSR_MODE_ABT               0x17
#define CPSR_MODE_UND               0x1b

#define MAX_IPCC_BUFF_SIZE          256
#define CMD_RXBUF_SIZE              64
#define LINE_TXBUF_SIZE             384

#define RPMSG_RRPOC_MP              MBOX_RPROC_MP
#define RPMSG_RRPOC_AP1             MBOX_RPROC_AP1
#define RPMSG_IPCC                  1

typedef enum {
    SDSHELL_MSG_READ = 1,
    SDSHELL_MSG_CLEAR,
    SDSHELL_MSG_RUN_CMD,
    SDSHELL_MSG_TARGET_SET,
    SDSHELL_MSG_QUIT,
    SDSHELL_MSG_PRINTON,
    SDSHELL_MSG_PRINTOFF,
    SDSHELL_MSG_RUN_CMD_FEEDBACK, /* frontend -> backend */
    SDSHELL_MSG_MAX,
} msg_type_t;

typedef struct sdshell_msg {
    uint32_t type;
    uint32_t size;
    uint8_t data[0];
} sdshell_msg_t;

void sdshell_console_init();
#endif /* __SDSHELL_SERVICE_H__ */
