/**
 * @file cu.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef CU_H_
#define CU_H_

void cu_console_init(void);

void cu_buffer_print_log(const char *data, uint32_t size);

void cu_transmit_print_log(void);

#endif /* CU_H_ */
