/*
 * Copyright (c) 2017 Simon Goldschmidt
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
 * Author: Simon Goldschmdit <goldsimon@gmx.de>
 *
 */
#include <arch/sys_arch.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#include <stdbool.h>
#include "debug.h"
#include "board/board.h"

void sys_init(void)
{
}

u32_t sys_now(void)
{
  return current_time();
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn func, void *arg, int stacksize, int prio)
{
  osThreadAttr_t attr = {
    .name = name,
    .stack_size = stacksize,
    .priority = prio
  };

  osThreadId_t t = osThreadNew(func, arg, &attr);
  LWIP_ASSERT("sys_thread_new: failed", t);

  return t;
}

err_t sys_thread_exit()
{
    osThreadExit();
    return ERR_OK;
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
  LWIP_ASSERT("sem != NULL", sem);

  sem->sem = osSemaphoreNew(~0, count, NULL);
  LWIP_ASSERT("sem->sem != NULL", sem->sem);
  sem->magic = SEM_MAGIC;
  return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
  LWIP_ASSERT("sem != NULL", sem);
  LWIP_ASSERT("sem->sem != NULL", sem->sem);

  osSemaphoreDelete(sem->sem);
}

int sys_sem_valid(sys_sem_t *sem)
{
  LWIP_ASSERT("sem != NULL", sem);

  return sem->magic == SEM_MAGIC;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
  LWIP_ASSERT("sem != NULL", sem);

  sem->magic = 'mes';
}

void sys_sem_signal(sys_sem_t *sem)
{
  LWIP_ASSERT("sem != NULL", sem);
  LWIP_ASSERT("sem->sem != NULL", sem->sem);

  osSemaphoreRelease(sem->sem);
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  LWIP_ASSERT("sem != NULL", sem);
  LWIP_ASSERT("sem->sem != NULL", sem->sem);

  uint32_t start = sys_now();

  osStatus_t err = osSemaphoreAcquire(sem->sem,
                    timeout ? timeout : osWaitForever);
  if (err == osErrorTimeout)
    return SYS_ARCH_TIMEOUT;

  return sys_now() - start;
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
  LWIP_ASSERT("mbox != NULL", mbox);

  mbox->msgq = osMessageQueueNew(size, sizeof(void *), NULL);
  if (!mbox->msgq)
    return ERR_MEM;

  mbox->magic = MBOX_MAGIC;

  return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
  LWIP_ASSERT("mbox != NULL", mbox);
  LWIP_ASSERT("mbox->msgq != NULL", mbox->msgq);

  osMessageQueueDelete(mbox->msgq);
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
  LWIP_ASSERT("mbox != NULL", mbox);
  LWIP_ASSERT("mbox->msgq != NULL", mbox->msgq);

  osMessageQueuePut(mbox->msgq, &msg, 0, osWaitForever);
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
  LWIP_ASSERT("mbox != NULL", mbox);
  LWIP_ASSERT("mbox->msgq != NULL", mbox->msgq);

  osStatus_t res;

  res = osMessageQueueGet(mbox->msgq, msg, NULL, 0);
  if (res == osErrorResource)
    return SYS_MBOX_EMPTY;

  return 0;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
  LWIP_ASSERT("mbox != NULL", mbox);
  LWIP_ASSERT("mbox->msgq != NULL", mbox->msgq);

  osStatus_t res;
  uint32_t start = sys_now();

  res = osMessageQueueGet(mbox->msgq, msg, NULL,
                          timeout ? timeout : osWaitForever);
  if (res == osErrorTimeout)
      return SYS_ARCH_TIMEOUT;

  return sys_now() - start;
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
  LWIP_ASSERT("mbox != NULL", mbox);
  LWIP_ASSERT("mbox->msgq != NULL", mbox->msgq);

  osStatus_t res;

  res = osMessageQueuePut(mbox->msgq, &msg, 0, 0);
  if (res == osErrorResource)
    return ERR_MEM;

  return ERR_OK;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
  return sys_mbox_trypost(mbox, msg);
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
  LWIP_ASSERT("mbox != NULL", mbox);

  return mbox->magic == MBOX_MAGIC;
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
  LWIP_ASSERT("mbox != NULL", mbox);

  mbox->magic = 'xobm';
}

err_t sys_mutex_new(sys_mutex_t *mutex)
{
  LWIP_ASSERT("mutex != NULL", mutex);

  mutex->lock = osMutexNew(NULL);
  if (!mutex->lock)
    return ERR_MEM;

  return ERR_OK;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
  LWIP_ASSERT("mutex != NULL", mutex);
  LWIP_ASSERT("mutex->lock != NULL", mutex->lock);

  osMutexAcquire(mutex->lock, osWaitForever);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
  LWIP_ASSERT("mutex != NULL", mutex);
  LWIP_ASSERT("mutex->lock != NULL", mutex->lock);

  osMutexRelease(mutex->lock);
}

void sys_mutex_free(sys_mutex_t *mutex)
{
  LWIP_ASSERT("mutex != NULL", mutex);
  LWIP_ASSERT("mutex->lock != NULL", mutex->lock);

  osMutexDelete(mutex->lock);
}

#if LWIP_TCPIP_CORE_LOCKING
/* Flag the core lock held. A counter for recursive locks. */
static u8_t lwip_core_lock_count;
static osThreadId_t lwip_core_lock_holder_thread;

void sys_lock_tcpip_core(void)
{
   sys_mutex_lock(&lock_tcpip_core);
   if (lwip_core_lock_count == 0) {
     lwip_core_lock_holder_thread = osThreadGetId();
   }
   lwip_core_lock_count++;
}

void sys_unlock_tcpip_core(void)
{
   lwip_core_lock_count--;
   if (lwip_core_lock_count == 0) {
       lwip_core_lock_holder_thread = 0;
   }
   sys_mutex_unlock(&lock_tcpip_core);
}
#endif

static osThreadId_t lwip_tcpip_thread;

void sys_mark_tcpip_thread(void)
{
  lwip_tcpip_thread = osThreadGetId();
}

void sys_check_core_locking(void)
{
  if (lwip_tcpip_thread != 0) {
    osThreadId_t current_thread = osThreadGetId();
    /* Embedded systems should check we are NOT in an interrupt context here.
     * We use osThreadGetState to check it.
     */
    LWIP_ASSERT("sys_check_core_locking called in IRQ context",
                osThreadError != osThreadGetState(current_thread));

#if LWIP_TCPIP_CORE_LOCKING
    LWIP_ASSERT("Function called without core lock",
                current_thread == lwip_core_lock_holder_thread && lwip_core_lock_count > 0);
#else /* LWIP_TCPIP_CORE_LOCKING */
    LWIP_ASSERT("Function called from wrong thread", current_thread == lwip_tcpip_thread);
#endif /* LWIP_TCPIP_CORE_LOCKING */
  }
}
