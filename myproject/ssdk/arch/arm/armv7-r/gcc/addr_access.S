/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/
#include "asm_macros.h"

#define FUNCTION(x) .global x; .type x,STT_FUNC; x:
#define DATA(x) .global x; .type x,STT_OBJECT; x:

#define LOCAL_FUNCTION(x) .type x,STT_FUNC; x:
#define LOCAL_DATA(x) .type x,STT_OBJECT; x:


FUNCTION(writel_)
    set_fault_handler   .Larch_write_to_reg_fault
    str     r0, [r1, #0]
    mov     r0, #0
    bx      lr
.Larch_write_to_reg_fault:
    mov     r0, #-1
    bx      lr
FUNCTION(readl_)
    set_fault_handler   .Larch_read_from_reg_fault
    ldr     r0, [r0], #0
    bx      lr
.Larch_read_from_reg_fault:
    mov     r0, #-1
    bx      lr
