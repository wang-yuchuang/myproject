/*
 * sdpe_rpc_common.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_RPC_COMMON_H
#define SDPE_RPC_COMMON_H

#include <compiler.h>
#include <param.h>
#include <cmsis_os2.h>
#include <debug.h>

#ifndef SDPE_RPC_PACKED_BEGIN
#define SDPE_RPC_PACKED_BEGIN
#endif

#ifndef SDPE_RPC_PACKED_END
#define SDPE_RPC_PACKED_END __PACKED
#endif

#define SDPE_RPC_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "sdpe_rpc:"

#if SDPE_RPC_DEBUG_LVL >= SSDK_EMERG
#define SDPE_RPC_ALWAYS(format, ...) \
    ssdk_printf(SSDK_EMERG, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDPE_RPC_ALWAYS(format, ...)
#endif

#if SDPE_RPC_DEBUG_LVL >= SSDK_ERR
#define SDPE_RPC_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDPE_RPC_ERR(format, ...)
#endif

#if SDPE_RPC_DEBUG_LVL >= SSDK_WARNING
#define SDPE_RPC_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDPE_RPC_WARN(format, ...)
#endif

#if SDPE_RPC_DEBUG_LVL >= SSDK_INFO
#define SDPE_RPC_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDPE_RPC_INFO(format, ...)
#endif

#define SDPE_RPC_CREATE_MSGID(service, msg)     (service << 8) + msg
#define SDPE_RPC_GET_SERVICE_ID(id)             (uint8_t)(id >> 8)
#define SDPE_RPC_GET_MSG_ID(id)                 (uint8_t)(id & 0xff)

#define SDPE_RPC_GET_SHARE_BUF(structure)                   \
({                                                          \
    static structure static_msg __SECTION(".sdpe_rpc_buf"); \
    (&static_msg);                                          \
})

#define SDPE_RPC_GET_MULTI_SHARE_BUF(structure, num, id)            \
({                                                                  \
    static structure static_msg[num] __SECTION(".sdpe_rpc_buf");    \
    (&static_msg[id]);                                              \
})

#define SDPE_RPC_OFFSET_OF(structure, member)               \
    ((uintptr_t) &(((structure *) 0)->member))

#define SDPE_RPC_COPY_MSG_DATA(member, size, addr)          \
{                                                           \
    new_config->member = (void*)addr;                       \
    memcpy(new_config->member, config->member, size);       \
    addr += ALIGN(size, 4);                                 \
}

#endif
