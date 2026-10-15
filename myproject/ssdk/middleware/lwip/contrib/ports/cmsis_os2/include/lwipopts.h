/**
 * @file
 *
 * lwIP Options Configuration
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef LWIP_LWIPOPTS_H
#define LWIP_LWIPOPTS_H

/*
 * Include user defined options first. Anything not defined in these files
 * will be set to standard values. Override anything you don't like!
 */

// use libc malloc
#define MEM_LIBC_MALLOC 1

// use mem_malloc() which calls malloc()
// instead of creating static memory pools
#define MEMP_MEM_MALLOC 1

// these don't actually affect anything
// unless MEMP_MEM_MALLOC is 0
#define MEM_SIZE (256 * 1024 * 1024)
#define MEMP_NUM_UDP_PCB 128
#define MEMP_NUM_TCP_PCB 128
#define MEMP_NUM_TCP_PCB_LISTEN 128
#define MEMP_NUM_NETBUF 32
#define MEMP_NUM_NETCONN 32
#define MEMP_NUM_NETDB 32

#define LWIP_COMPAT_SOCKETS 1
#define LWIP_SO_RCVTIMEO    1
#define LWIP_SO_RCVBUF      1

#define LWIP_DHCP 0
#define LWIP_AUTOIP 0
#define LWIP_DHCP_AUTOIP_COOP 0

#define LWIP_DNS 0

#define LWIP_NETIF_HOSTNAME 1
#define LWIP_NETIF_API 1
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_HWADDRHINT 1
#define LWIP_NETIF_LOOPBACK 1

#define LWIP_HAVE_LOOPIF 1

#define TCPIP_THREAD_STACKSIZE 4096
#define TCPIP_THREAD_PRIO osPriorityRealtime

#define TCPIP_MBOX_SIZE 16

#define DEFAULT_THREAD_PRIO (osPriorityNormal - 1)
#define DEFAULT_THREAD_STACKSIZE 4096

#define DEFAULT_RAW_RECVMBOX_SIZE 16
#define DEFAULT_UDP_RECVMBOX_SIZE 16
#define DEFAULT_TCP_RECVMBOX_SIZE 16
#define DEFAULT_ACCEPTMBOX_SIZE 16

#define LWIP_STATS_DISPLAY 1
#define LWIP_ERRNO_STDINCLUDE
#define SYS_LIGHTWEIGHT_PROT 0
#define LWIP_DEBUG 1
/**
 * If debug log needed, define macros as following:
 * #define TCPIP_DEBUG (LWIP_DBG_ON | 3)
 * #define SOCKETS_DEBUG (LWIP_DBG_ON | 3)
 */
#define LWIP_RAW 1

/* If using multi-thread, set the two following macros to 1 */
#define LWIP_TCPIP_CORE_LOCKING 0
#define LWIP_TCPIP_CORE_LOCKING_INPUT 0

#if LWIP_TCPIP_CORE_LOCKING
void sys_mark_tcpip_thread(void);
#define LWIP_MARK_TCPIP_THREAD()   sys_mark_tcpip_thread()

void sys_check_core_locking(void);
#define LWIP_ASSERT_CORE_LOCKED()  sys_check_core_locking()

void sys_lock_tcpip_core(void);
#define LOCK_TCPIP_CORE()          sys_lock_tcpip_core()
void sys_unlock_tcpip_core(void);
#define UNLOCK_TCPIP_CORE()        sys_unlock_tcpip_core()
#else
#define LWIP_MARK_TCPIP_THREAD()
#define LWIP_ASSERT_CORE_LOCKED()
#define LOCK_TCPIP_CORE()
#define UNLOCK_TCPIP_CORE()
#endif
#define CACHE_LINE 32
#define MEM_ALIGNMENT CACHE_LINE
#define LWIP_NETIF_TX_SINGLE_PBUF 1
/* TCP Maximum segment size. */
/* Not adjust this param, it's more stable to use the default value 536. */
/* #define TCP_MSS                 1024 */

#endif /* LWIP_LWIPOPTS_H */
