/*
 * Copyright (c) 2019 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#ifndef __RPMSG_COMMOM_H__
#define __RPMSG_COMMOM_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)    (sizeof(arr) / sizeof((arr)[0]))
#endif

/* Service endpoint following DCF endpoint management specification */
/* Enpdpoint 0 ~ 255 reserved for Semidrive internal use */
#define SVCMGR_IVI_EPT          (10)
#define SVCMGR_CLU_EPT          (11)
#define LOOPBACK_EPT            (12)
#define PROPERTY_EPT            (13)
#define IPCC_RPC_CLIENT_EPT     (14)
#define IPCC_RPC_SAF_MAIN_EPT   (15)
#define IPCC_RPC_MAIN_EPT       (16)
#define RPMSG_TEST_EPT          (18)
#define IPCC_SYS_EPT            (20)
#define IPCC_ECHO_EPT           (30)
#define RPMSG_ECHO_EPT          (IPCC_ECHO_EPT)
#define IPCC_TTY_EPT            (40)
#define TTY_LINUX_EPT           (40)
// #define SRV_SAMPLE_EPT          (48)
#define ERPC_LINUX_EPT          (50)
#define NET_LINUX_EPT           (60)
#define CLUSTER_EPT             (70)
#define IVI_EPT                 (71)
#define SSYSTEM_EPT             (72)
#define SA_VI2C_EPT             (73)
#define EARLYAPP_EPT            (80)
#define UPDATE_MONITOR_EPT      (81)
#define VIRCAN_IVI_EPT          (90)
#define VIRCAN_CLU_EPT          (91)
#define SLT_TEST_EPT_SAF        (92)
#define SLT_TEST_EPT_SEC        (93)
#define SLT_TEST_EPT_MP         (94)
#define SLT_TEST_EPT_AP1        (95)
#define SLT_TEST_EPT_AP2        (96)
#define SDSHELL_SERVICE_EPT     (97)
#define SDSHELL_MP_EPT          (98)
#define SMS_MONITORP_EPT        (100)
/* Endpoint 101 ~ 112 reserved for can proxy */
#define CAN_PROXY_EPT_START     (101)
#define CAN_PROXY_EPT_END       (112)

#define AUDIO_MONITOR_EPT       (125)
#define BOARD_DIAG_EPT          (126)
#define SDRV_PROPERTY_EPT       (127)

/* Endpoint 130 ~ 133 may be reserved for virtual i2s */
#define SA_VI2S_EPT             (130)

#define SHM_EPT                 (200)

#define SEMIDRIVE_MAX_EPT       (255)

#define SRV_SAMPLE_EPT          (500)

#define SDRRADIO_MONITOR_EPT    (335)
/* Endpoint 256 ~ 1023 reserved for customer or partner */

typedef enum {
    DCF_STATE_Unknown,
    DCF_STATE_Initializing,
    DCF_STATE_Initialized,
    DCF_STATE_Closing,
    DCF_STATE_Closed,
    DCF_STATE_Connected,
} dcf_state_t;

#define RPMSG_TACK_LEN          (4)
#define RPMSG_TACK_CODE         "ACK"
#define RPMSG_PKT_MTU           (RL_BUFFER_PAYLOAD_SIZE)
#define RPMSG_PKT_HDRLEN        (16)

// #include <dcf_msg.h>

#endif //__RPMSG_COMMOM_H__
