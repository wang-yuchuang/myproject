/**
 * @file rpmsg_echo.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef RPMSG_ECHO_H_
#define RPMSG_ECHO_H_

#include <compiler.h>
#include <types.h>

#define RPMSG_ECHO_EPT_ADDR     30
#define COMM_MSG_CCM_ECHO       0xA5
#define COMM_MSG_CCM_ACK        0xA6
#define COMM_MSG_CCM_BOOT_READY  0xB5

#define COMM_MSG_CCM_DROP       0xAA

#ifdef CONFIG_RPMSG_WITH_LK
/* DCF message definition
 * msg base header is 4 B long
 * msg extention header is 4 bytes indicated by opflag
 * msg body 0 ~ 4K-1 bytes data filled by use case
 */
struct dcf_msg_extend {
    union {
        uint32_t dat32;
        struct {
            uint8_t src;    /* app-id for source, 0 if not used */
            uint8_t dst;    /* app-id for destination, 0 if not used */
        } addr;
    } u;
} __PACKED;

struct dcf_message {
    uint32_t msg_type : 8;  /* dcf_msg_type */
    /* Optional features
        SHORT: 1 msg without body
        FRAGMENT: 1
        SYNC: 1
        ACK:  1
        CKSM: 1, checksum exist in the end of packet
    */
    uint32_t opflags  : 8;
    uint32_t msg_len  : 12; /* msg body len */
    uint32_t reserved : 4;  /* future use */
    struct dcf_msg_extend option[1];  /* msg option if exist indicated by flag */
    uint8_t data[0];            /* payload start */
} __PACKED;

#define DCF_MSGF_STD        (0x00)
#define DCF_MSGF_OPT        (0x01)
#define DCF_MSGF_SHT        (0x02)
#define DCF_MSGF_FRA        (0x04)
#define DCF_MSGF_SYN        (0x08)
#define DCF_MSGF_ACK        (0x10)
#define DCF_MSGF_CKS        (0x20)
#define DCF_MSGF_TMS        (0x40)

/* type:COMM_MSG_RPCALL message format
 * the RPC message struct defined ipcc_rpc.h
 */

/* type:
 * COMM_MSG_CCM_* header
 */
struct dcf_ccm_hdr {
    struct dcf_message dmsg;
    uint32_t     seq;
    uint32_t     time[8];
    char    data[];
};
#else
struct rpmsg_echo_hdr {
    uint32_t  magic;
    uint32_t  seq;
    uint32_t  len;
    uint32_t  time[4];
    uint8_t   data[0];
} __PACKED;
#endif /* CONFIG_RPMSG_WITH_LK */

void rpmsg_service_echo_init(struct rpmsg_instance *rpmsg_dev, uint8_t dev_type);

void echo_reset_boot_ready_count(void);

#endif /* RPMSG_ECHO_H_ */
