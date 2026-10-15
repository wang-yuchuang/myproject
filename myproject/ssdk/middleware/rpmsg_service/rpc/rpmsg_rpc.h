/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#ifndef __IPCC_RPC_H__
#define __IPCC_RPC_H__

#include <cmsis_os2.h>
// #include "ipcc_device.h"
#include <rpmsg.h>
#include <rpmsg_channel.h>
#include "rpmsg_echo.h"
#include "dcf_msg.h"

#define IPCC_RPC_MAX_PARAMS     (8)
#define IPCC_RPC_MAX_RESULT     (4)
#define IPCC_RPC_FUNC_MAX       (16)
#define IPCC_RPC_NO_FLAGS       (0)

/* Here defines default RPC cmd */
#define IPCC_RPC_REQ_PING           (0x1000)
#define IPCC_RPC_ACK_PING           (IPCC_RPC_REQ_PING+1)
#define IPCC_RPC_REQ_GETTIMEOFDAY   (0x1002)
#define IPCC_RPC_ACK_GETTIMEOFDAY   (IPCC_RPC_REQ_GETTIMEOFDAY+1)
/* Here defines system level RPC cmd */
#define SYS_RPC_REQ_BASE            (0x2000)
#define SYS_RPC_REQ_SET_PROPERTY    (SYS_RPC_REQ_BASE + 0)
#define SYS_RPC_REQ_GET_PROPERTY    (SYS_RPC_REQ_BASE + 1)
#define SYS_RPC_REQ_UNAME           (SYS_RPC_REQ_BASE + 2)
#define SYS_RPC_REQ_LIST            (SYS_RPC_REQ_BASE + 3)
#define SYS_RPC_REQ_LOG_ON          (SYS_RPC_REQ_BASE + 4)
#define SYS_RPC_REQ_LOG_OFF         (SYS_RPC_REQ_BASE + 5)
#define SYS_RPC_REQ_INQUIRY         (SYS_RPC_REQ_BASE + 6)
	/* Here defined module level RPC cmd */
#define MOD_RPC_REQ_BASE            (0x3000)
#define MOD_RPC_REQ_TEST            (MOD_RPC_REQ_BASE + 0)
#define MOD_RPC_REQ_OPEN_DC         (MOD_RPC_REQ_BASE + 1)
#define MOD_RPC_REQ_CLOS_DC         (MOD_RPC_REQ_BASE + 2)
#define MOD_RPC_REQ_SET_DCPARAM     (MOD_RPC_REQ_BASE + 3)
#define MOD_RPC_REQ_GET_DCPARAM     (MOD_RPC_REQ_BASE + 4)
/* Here defined Sample RPC cmd */
#define MOD_RPC_REQ_SSP_IOCTL       (MOD_RPC_REQ_BASE + 6)
	/* Here defined backlight RPC cmd */
#define MOD_RPC_REQ_BL_IOCTL        (MOD_RPC_REQ_BASE + 8)
	/* Here defined safe touch screen cmd */
#define MOD_RPC_REQ_STS_IOCTL       (MOD_RPC_REQ_BASE + 10)
	/* Here defined display controller cmd */
#define MOD_RPC_REQ_DC_IOCTL        (MOD_RPC_REQ_BASE + 16)
	/* Here defined camera controller cmd */
#define MOD_RPC_REQ_SCS_IOCTL       (MOD_RPC_REQ_BASE + 24)
    /* Here defined audio service cmd */
#define MOD_RPC_REQ_AUDIO_SERVICE   (MOD_RPC_REQ_BASE + 32)
/* Safe CAN proxy ioctl RPC cmd */
#define MOD_RPC_REQ_CP_IOCTL        (MOD_RPC_REQ_BASE + 0x40)

/** err type macro define */
#define NO_ERROR                (0)
#define ERR_GENERIC             (-1)
#define ERR_NOT_FOUND           (-2)
#define ERR_NOT_READY           (-3)
#define ERR_NO_MSG              (-4)
#define ERR_NO_MEMORY           (-5)
#define ERR_ALREADY_STARTED     (-6)
#define ERR_NOT_VALID           (-7)
#define ERR_INVALID_ARGS        (-8)
#define ERR_NOT_ENOUGH_BUFFER   (-9)
#define ERR_NOT_SUSPENDED       (-10)
#define ERR_OBJECT_DESTROYED    (-11)
#define ERR_NOT_BLOCKED         (-12)
#define ERR_TIMED_OUT           (-13)
#define ERR_ALREADY_EXISTS      (-14)
#define ERR_CHANNEL_CLOSED      (-15)
#define ERR_OFFLINE             (-16)
#define ERR_NOT_ALLOWED         (-17)
#define ERR_BAD_PATH            (-18)
#define ERR_ALREADY_MOUNTED     (-19)
#define ERR_IO                  (-20)
#define ERR_NOT_DIR             (-21)
#define ERR_NOT_FILE            (-22)
#define ERR_RECURSE_TOO_DEEP    (-23)
#define ERR_NOT_SUPPORTED       (-24)
#define ERR_TOO_BIG             (-25)
#define ERR_CANCELLED           (-26)
#define ERR_NOT_IMPLEMENTED     (-27)
#define ERR_CHECKSUM_FAIL       (-28)
#define ERR_CRC_FAIL            (-29)
#define ERR_CMD_UNKNOWN         (-30)
#define ERR_BAD_STATE           (-31)
#define ERR_BAD_LEN             (-32)
#define ERR_BUSY                (-33)
#define ERR_THREAD_DETACHED     (-34)
#define ERR_I2C_NACK            (-35)
#define ERR_ALREADY_EXPIRED     (-36)
#define ERR_OUT_OF_RANGE        (-37)
#define ERR_NOT_CONFIGURED      (-38)
#define ERR_NOT_MOUNTED         (-39)
#define ERR_FAULT               (-40)
#define ERR_NO_RESOURCES        (-41)
#define ERR_BAD_HANDLE          (-42)
#define ERR_ACCESS_DENIED       (-43)
#define ERR_PARTIAL_WRITE       (-44)

typedef struct _rpc_call_request_ {
    struct dcf_message hdr;
    uint32_t         cmd;
    uint32_t         param[IPCC_RPC_MAX_PARAMS];
} rpc_call_request_t;

typedef struct _rpc_call_result_ {
    struct dcf_message hdr;
    uint32_t         ack;
    uint32_t         retcode;
    uint32_t         result[IPCC_RPC_MAX_RESULT];
} rpc_call_result_t;

#define DCF_RPC_REQLEN         ((IPCC_RPC_MAX_PARAMS + 4)*sizeof(uint32_t))
#define DCF_RPC_REQ_INITVALUE  {{0, }, 0,}
#define DCF_RPC_RES_INITVALUE  {0,}

#define DCF_MSG_INIT_HDR(msg, type, len, flags) \
		({msg->msg_type = type; \
		msg->opflags  = flags; \
		msg->msg_len  = len; \
		msg->option[0].u.dat32 = 0x0;})

#define DCF_INIT_RPC_HDR(msg, sz) \
		DCF_MSG_INIT_HDR((msg), COMM_MSG_RPCALL, \
		sz, DCF_MSGF_STD)

#define DCF_INIT_RPC_REQ(req, op) \
		({DCF_INIT_RPC_HDR(&(req).hdr, 4); \
		(req).cmd = op;})

#define DCF_INIT_RPC_REQ1(req, op, param1) \
		({DCF_INIT_RPC_HDR(&(req).hdr, 8); \
		(req).cmd = op; \
		(req).param[0] = param1;})

#define DCF_INIT_RPC_REQ4(req, op, param1, param2, param3, param4) \
		({DCF_INIT_RPC_HDR(&(req).hdr, 20); \
		(req).cmd = op;          \
		(req).param[0] = param1; \
		(req).param[1] = param2; \
		(req).param[2] = param3; \
		(req).param[3] = param4;})

#define DCF_RPC_PARAM(req, type) (type *)(&req.param[0])

typedef struct ipcc_rpc_client {
    uint32_t         service_id;
    rpmsg_channel_t  rpchan;

} *rpc_client_handle_t;

typedef struct ipcc_rpc_server *rpc_server_handle_t;

typedef rpc_call_result_t (*rpc_server_func_t)(rpc_server_handle_t hserver,
                                               rpc_call_request_t *request);

typedef struct _rpc_server_impl_ {
    uint32_t         command;
    rpc_server_func_t   func;
    uint32_t         flags;

} rpc_server_impl_t;

typedef struct ipcc_rpc_server {
    uint32_t             service_id;
    struct rpmsg_instance  *rpdev;
    rpmsg_channel_t      rpchan;
    uint32_t             status;

    rpc_server_impl_t   *func_table;
    uint32_t             func_max;
    uint32_t             func_used;
    osThreadId_t         rpc_thread;

} *rpc_server_handle_t;

int ipcc_rpc_client_init(rpc_client_handle_t hclient,
                       struct rpmsg_instance *dev, uint32_t client_id, uint32_t server_id);

void ipcc_rpc_client_destroy(rpc_client_handle_t hclient);

int32_t ipcc_rpc_client_call(rpc_client_handle_t hclient,
                                rpc_call_request_t *request,
                                rpc_call_result_t *result, uint32_t timeout);

int32_t ipcc_rpc_server_init(rpc_server_handle_t handle,
                        struct rpmsg_instance *dev, int server_id, int threads);

void ipcc_rpc_server_exit(rpc_server_handle_t handle);

int32_t ipcc_rpc_server_start(rpc_server_handle_t hserver);

int32_t ipcc_rpc_server_stop(rpc_server_handle_t hserver);

int32_t ipcc_rpc_setup_implement(rpc_server_handle_t hserver,
                                         rpc_server_impl_t *tables, int num);
void ipcc_rpc_server_exit(rpc_server_handle_t hserver);

#endif //__IPCC_RPC_H__
