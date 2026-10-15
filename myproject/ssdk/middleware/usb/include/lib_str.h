/**
 * @file lib_str.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: String library header file for USB Device & Host.
 *
 * Revision History:
 * -----------------
 */


#ifndef  LIB_STR_MODULE_PRESENT
#define  LIB_STR_MODULE_PRESENT


#include  <string.h>
#include  <cpu.h>


static inline
CPU_SIZE_T   Str_Len            (const  CPU_CHAR      *pstr)
{
    return strlen(pstr);
}

static inline
CPU_CHAR    *Str_Copy_N         (       CPU_CHAR      *pstr_dest,
                                 const  CPU_CHAR      *pstr_src,
                                        CPU_SIZE_T     len_max)
{
    return strncpy(pstr_dest, pstr_src, len_max);
}

CPU_CHAR    *Str_FmtNbr_Int32U  (       CPU_INT32U     nbr,
                                        CPU_INT08U     nbr_dig,
                                        CPU_INT08U     nbr_base,
                                        CPU_CHAR       lead_char,
                                        CPU_BOOLEAN    lower_case,
                                        CPU_BOOLEAN    nul,
                                        CPU_CHAR      *pstr);

#endif

