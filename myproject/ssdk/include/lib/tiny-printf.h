// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022
 * Huafeng.huang <huafeng.huang@semidrive.com>
 */

#ifndef _INCLUDE_TINY_PRINTF_H
#define _INCLUDE_TINY_PRINTF_H

int early_vprintf(const char *fmt, va_list va);
int early_printf(const char *fmt, ...);
int early_sprintf(char *buf, const char *fmt, ...);
int early_snprintf(char *buf, size_t size, const char *fmt, ...);
void print_grouped_ull(unsigned long long int_val, int digits);

#endif
