/*
 * sdpe_rpc_mbuf.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_RPC_MBUF_H
#define SDPE_RPC_MBUF_H

/*
 * @brief Initializes rpc mbuf dev.
 * @param null
 * @return init result
 */
#ifdef CONFIG_SDPE_RPC_MBUF
int sdpe_rpc_mbuf_init(void);
#else
#define sdpe_rpc_mbuf_init()
#endif

/*
 * @brief deinitializes rpc mbuf dev.
 * @param null
 * @return deinit result
 */
#ifdef CONFIG_SDPE_RPC_MBUF
void sdpe_rpc_mbuf_deinit(void);
#else
#define sdpe_rpc_mbuf_deinit()
#endif

/*
 * @brief get mbuf.
 * @param len tx data len
 * @return tx data addr
 */
#ifdef CONFIG_SDPE_RPC_MBUF
void *sdpe_rpc_alloc_mbuf(uint32_t len);
#else
#define sdpe_rpc_alloc_mbuf(len)
#endif

/*
 * @brief free mbuf.
 * @param data mbuf ptr
 * @return null
 */
#ifdef CONFIG_SDPE_RPC_MBUF
void sdpe_rpc_free_mbuf(void *data);
#else
#define sdpe_rpc_free_mbuf(data)
#endif

#endif
