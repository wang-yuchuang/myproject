/*
 * svcall.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SV call header file.
 *
 * Revision History:
 * -----------------
 */

#ifndef ARM_V7R_SVCALL_H_
#define ARM_V7R_SVCALL_H_

/* Cortex-R system calls ************************************************************/

/* SYS call 0:
 *
 * int arm_saveusercontext(uint32_t *saveregs);
 */

#define SYS_save_context          (0)

/* SYS call 1:
 *
 * void arm_fullcontextrestore(uint32_t *restoreregs) noreturn_function;
 */

#define SYS_restore_context       (1)

/* SYS call 2:
 *
 * void arm_switchcontext(void);
 */

#define SYS_switch_context        (2)

#endif /* ARM_V7R_SVCALL_H_ */
