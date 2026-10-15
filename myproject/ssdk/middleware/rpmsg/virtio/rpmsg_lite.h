/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * Copyright (c) 2015 Xilinx, Inc.
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _RPMSG_LITE_H
#define _RPMSG_LITE_H

#if defined(__cplusplus)
extern "C" {
#endif

//! @addtogroup rpmsg_lite
//! @{

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define RL_VERSION "2.2.0" /*!< Current RPMsg Lite version */

/* Shared memory "allocator" parameters */
#define RL_WORD_SIZE (sizeof(unsigned long))
#define RL_WORD_ALIGN_UP(a)                                                                                  \
    (((((unsigned long)a) & (RL_WORD_SIZE - 1)) != 0) ? ((((unsigned long)a) & (~(RL_WORD_SIZE - 1))) + 4) : \
                                                        ((unsigned long)a))
#define RL_WORD_ALIGN_DOWN(a)                                                                          \
    (((((unsigned long)a) & (RL_WORD_SIZE - 1)) != 0) ? (((unsigned long)a) & (~(RL_WORD_SIZE - 1))) : \
                                                        ((unsigned long)a))

/* Definitions for device types , null pointer, etc.*/
#define RL_SUCCESS (0)
#define RL_NULL ((void *)0)
#define RL_REMOTE (0)
#define RL_MASTER (1)
#define RL_TRUE (1)
#define RL_FALSE (0)
#define RL_ADDR_ANY (0xFFFFFFFF)
#define RL_RELEASE (0)
#define RL_HOLD (1)
#define RL_DONT_BLOCK (0)
#define RL_BLOCK (0xFFFFFFFF)

/* Error macros. */
#define RL_ERRORS_BASE (-5000)
#define RL_ERR_NO_MEM (RL_ERRORS_BASE - 1)
#define RL_ERR_BUFF_SIZE (RL_ERRORS_BASE - 2)
#define RL_ERR_PARAM (RL_ERRORS_BASE - 3)
#define RL_ERR_DEV_ID (RL_ERRORS_BASE - 4)
#define RL_ERR_MAX_VQ (RL_ERRORS_BASE - 5)
#define RL_ERR_NO_BUFF (RL_ERRORS_BASE - 6)
#define RL_NOT_READY (RL_ERRORS_BASE - 7)
#define RL_ALREADY_DONE (RL_ERRORS_BASE - 8)

/* Init flags */
#define RL_NO_FLAGS (0)

#if defined(__cplusplus)
}
#endif

#endif /* _RPMSG_LITE_H */
