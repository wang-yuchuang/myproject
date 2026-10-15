
/**
 * @file dcf_rpc.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dcf rpc header file
 *
 * Revision History:
 * -----------------
 */
#ifndef __INCLUDE_DCF_RPC_H__
#define __INCLUDE_DCF_RPC_H__
/* =========================== include file session ======================== */
#include "rpmsg_rpc.h"
#include "rpmsg_service.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/

/* =========================== structure define session ==================== */
typedef struct rpmsg_rpc_service {
    struct ipcc_rpc_server rpc_server;
    struct ipcc_rpc_client rpc_client;
    bool ready;
}rpmsg_rpc_service_t;

typedef rpmsg_rpc_service_t* ipcc_service_handle_t;

/* =========================== function declare session ==================== */
rpmsg_rpc_service_t* dcf_get_service_handle(int32_t rproc);
int32_t __dcf_ping(ipcc_service_handle_t dp);
int32_t __dcf_gettimeofday(ipcc_service_handle_t dp);
int __dcf_call(ipcc_service_handle_t dp, rpc_call_request_t *request,
                  rpc_call_result_t *result, uint32_t timeout);

int32_t dcf_rpc_service_init(rpmsg_service_t* service);

int32_t dcf_rpc_service_deinit();

int dcf_setup_func(ipcc_service_handle_t dp, rpc_server_impl_t *tables, int num);

int dcf_setup_func_detail(ipcc_service_handle_t dp, uint32_t cmd, rpc_server_func_t fn);

void start_ipcc_rpc_service(rpc_server_impl_t *tables, int num);

void dcf_register_rpc_implement(rpc_server_impl_t *fn);

void dcf_register_rpc_implement_detailed(uint32_t cmd, rpc_server_func_t fn);

/* =========================== function implementation session ============= */
/* the helper function for system default ipcc services */
inline static int dcf_call(int rproc, rpc_call_request_t *request, rpc_call_result_t *result, uint32_t timeout)
{
    return __dcf_call(dcf_get_service_handle(rproc), request, result, timeout);
}

inline static int dcf_ping(int rproc)
{
    return __dcf_ping(dcf_get_service_handle(rproc));
}

inline static int dcf_gettimeofday(int rproc)
{
    return __dcf_gettimeofday(dcf_get_service_handle(rproc));
}

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __INCLUDE_DCF_RPC_H__ */
/* end of header file dcf_rpc.h */
