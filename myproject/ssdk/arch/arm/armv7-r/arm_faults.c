/*
 * Copyright (c) 2008-2014 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <types.h>
#include <bits.h>
#include <debug.h>
#include <cmsis_os2.h>

#include <arch/register.h>
#include <arch/armv7-r/exceptions.h>

#include <board.h>

#include "arm.h"

#if defined(CONFIG_FuncSafetyPkg)
#include "FuncSafetyPkg.h"
#endif


static void dump_mode_regs(uint32_t spsr, uint32_t svc_r13, uint32_t svc_r14)
{
    struct arm_mode_regs regs;
    arm_save_mode_regs(&regs);

    ssdk_printf(SSDK_EMERG, "%c%s r13 0x%08x r14 0x%08x\n", ((spsr & MODE_MASK) == MODE_USR) ? '*' : ' ', "usr", regs.usr_r13, regs.usr_r14);
    ssdk_printf(SSDK_EMERG, "%c%s r13 0x%08x r14 0x%08x\n", ((spsr & MODE_MASK) == MODE_FIQ) ? '*' : ' ', "fiq", regs.fiq_r13, regs.fiq_r14);
    ssdk_printf(SSDK_EMERG, "%c%s r13 0x%08x r14 0x%08x\n", ((spsr & MODE_MASK) == MODE_IRQ) ? '*' : ' ', "irq", regs.irq_r13, regs.irq_r14);
    ssdk_printf(SSDK_EMERG, "%c%s r13 0x%08x r14 0x%08x\n", 'a', "svc", regs.svc_r13, regs.svc_r14);
    ssdk_printf(SSDK_EMERG, "%c%s r13 0x%08x r14 0x%08x\n", ((spsr & MODE_MASK) == MODE_SVC) ? '*' : ' ', "svc", svc_r13, svc_r14);
    ssdk_printf(SSDK_EMERG, "%c%s r13 0x%08x r14 0x%08x\n", ((spsr & MODE_MASK) == MODE_UND) ? '*' : ' ', "und", regs.und_r13, regs.und_r14);
    ssdk_printf(SSDK_EMERG, "%c%s r13 0x%08x r14 0x%08x\n", ((spsr & MODE_MASK) == MODE_SYS) ? '*' : ' ', "sys", regs.sys_r13, regs.sys_r14);

    // dump the bottom of the current stack
    addr_t stack;
    switch (spsr & MODE_MASK) {
        case MODE_FIQ:
            stack = regs.fiq_r13;
            break;
        case MODE_IRQ:
            stack = regs.irq_r13;
            break;
        case MODE_SVC:
            stack = svc_r13;
            break;
        case MODE_UND:
            stack = regs.und_r13;
            break;
        case MODE_SYS:
            stack = regs.sys_r13;
            break;
        default:
            stack = 0;
    }

    if (stack != 0) {
        ssdk_printf(SSDK_EMERG, "bottom of stack at 0x%08x:\n", (unsigned int)stack);
        hexdump((void *)stack, 128);
    }
}

static void dump_fault_frame(struct arm_fault_frame *frame)
{
    osThreadId_t current_thread = osThreadGetId();

    ssdk_printf(SSDK_EMERG, "current_thread %p, name %s\n",
            current_thread, current_thread ? osThreadGetName(current_thread) : "");

    ssdk_printf(SSDK_EMERG, "r0  0x%08x r1  0x%08x r2  0x%08x r3  0x%08x\n", frame->r[0], frame->r[1], frame->r[2], frame->r[3]);
    ssdk_printf(SSDK_EMERG, "r4  0x%08x r5  0x%08x r6  0x%08x r7  0x%08x\n", frame->r[4], frame->r[5], frame->r[6], frame->r[7]);
    ssdk_printf(SSDK_EMERG, "r8  0x%08x r9  0x%08x r10 0x%08x r11 0x%08x\n", frame->r[8], frame->r[9], frame->r[10], frame->r[11]);
    ssdk_printf(SSDK_EMERG, "r12 0x%08x usp 0x%08x ulr 0x%08x pc  0x%08x\n", frame->r[12], frame->usp, frame->ulr, frame->pc);
    ssdk_printf(SSDK_EMERG, "spsr 0x%08x\n", frame->spsr);

    dump_mode_regs(frame->spsr, (uintptr_t)(frame + 1), frame->lr);
}

static void dump_iframe(struct arm_iframe *frame)
{
    ssdk_printf(SSDK_EMERG, "r0  0x%08x r1  0x%08x r2  0x%08x r3  0x%08x\n", frame->r0, frame->r1, frame->r2, frame->r3);
    ssdk_printf(SSDK_EMERG, "r12 0x%08x usp 0x%08x ulr 0x%08x pc  0x%08x\n", frame->r12, frame->usp, frame->ulr, frame->pc);
    ssdk_printf(SSDK_EMERG, "spsr 0x%08x\n", frame->spsr);

    dump_mode_regs(frame->spsr, (uintptr_t)(frame + 1), frame->lr);
}

static void exception_die(struct arm_fault_frame *frame, const char *msg)
{
    ssdk_printf(SSDK_EMERG, "%s", msg);
    dump_fault_frame(frame);

    board_halt(HALT_ACTION_HALT, HALT_REASON_SW_PANIC);
    for (;;);
}

static void exception_die_iframe(struct arm_iframe *frame, const char *msg)
{
    ssdk_printf(SSDK_EMERG, "%s", msg);
    dump_iframe(frame);

    board_halt(HALT_ACTION_HALT, HALT_REASON_SW_PANIC);
    for (;;);
}

void arm_undefined_handler(struct arm_iframe *frame)
{
    /* look at the undefined instruction, figure out if it's something we can handle */
    bool in_thumb = frame->spsr & (1<<5);
    if (in_thumb) {
        frame->pc -= 2;
    } else {
        frame->pc -= 4;
    }

    __UNUSED uint32_t opcode = *(uint32_t *)frame->pc;
    ssdk_printf(SSDK_EMERG, "undefined opcode 0x%x\n", opcode);

#if CONFIG_ARCH_USE_FPU
    if (in_thumb) {
        /* look for a 32bit thumb instruction */
        if (opcode & 0x0000e800) {
            /* swap the 16bit words */
            opcode = (opcode >> 16) | (opcode << 16);
        }

        if (((opcode & 0xec000e00) == 0xec000a00) || // vfp
                ((opcode & 0xef000000) == 0xef000000) || // advanced simd data processing
                ((opcode & 0xff100000) == 0xf9000000)) { // VLD

            ssdk_printf(SSDK_EMERG, "vfp/neon thumb instruction 0x%08x at 0x%x\n", opcode, frame->pc);
        }
    } else {
        /* look for arm vfp/neon coprocessor instructions */
        if (((opcode & 0x0c000e00) == 0x0c000a00) || // vfp
                ((opcode & 0xfe000000) == 0xf2000000) || // advanced simd data processing
                ((opcode & 0xff100000) == 0xf4000000)) { // VLD
            ssdk_printf(SSDK_EMERG, "vfp/neon arm instruction 0x%08x at 0x%x\n", opcode, frame->pc);
        }
    }
#endif

    exception_die_iframe(frame, "undefined abort, halting\n");
    return;
}

#if (defined(CONFIG_FuncSafetyPkg))
struct fault_handler_table_entry
 {
    uint32_t pc;
    uint32_t fault_handler;
};
extern struct fault_handler_table_entry __fault_handler_table_start[];
extern struct fault_handler_table_entry __fault_handler_table_end[];
#endif

void arm_data_abort_handler(struct arm_fault_frame *frame)
{
#if ((defined(CONFIG_FuncSafetyPkg)) && ((FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)))
    if(Inject2bitTcmErrFlg == TRUE)
    {
        Tcm_Inject_2Bit_Err_Exception_Handlr();

    }
    else
    {
#endif

    uint32_t fsr = arm_read_dfsr();
    uint32_t far = arm_read_dfar();

    uint32_t fault_status = (BIT(fsr, 10) ? (1<<4) : 0) |  BITS(fsr, 3, 0);

#if (defined(CONFIG_FuncSafetyPkg))
		struct fault_handler_table_entry *fault_handler;
        for (fault_handler = __fault_handler_table_start; fault_handler < __fault_handler_table_end; fault_handler++)
        {
            //ssdk_printf(SSDK_EMERG, "\n\nfault_handler->pc=0x%x, frame->pc=0x%x",(uint32_t)fault_handler->pc,frame->pc);
            if (fault_handler->pc == frame->pc)
            {
                frame->pc = fault_handler->fault_handler;
                return;
            }
        }
#endif
        ssdk_printf(SSDK_EMERG, "\n\ncpu data abort, ");
        bool write = !!BIT(fsr, 11);

        /* decode the fault status (from table B3-23) */
        switch (fault_status)
        {
            case 0x01: // alignment fault
                ssdk_printf(SSDK_EMERG, "alignment fault on %s\n", write ? "write" : "read");
                break;
            case 0x05:
            case 0x07: // translation fault
                ssdk_printf(SSDK_EMERG, "translation fault on %s\n", write ? "write" : "read");
                break;
            case 0x03:
            case 0x06: // access flag fault
                ssdk_printf(SSDK_EMERG, "access flag fault on %s\n", write ? "write" : "read");
                break;
            case 0x09:
            case 0x0B: // domain fault
                ssdk_printf(SSDK_EMERG, "domain fault, domain %lu\n", BITS_SHIFT(fsr, 7, 4));
                break;
            case 0x0D:
            case 0x0F: // permission fault
                ssdk_printf(SSDK_EMERG, "permission fault on %s\n", write ? "write" : "read");
                break;
            case 0x02: // debug event
                ssdk_printf(SSDK_EMERG, "debug event\n");
                break;
            case 0x08: // synchronous external abort
                ssdk_printf(SSDK_EMERG, "synchronous external abort on %s\n", write ? "write" : "read");
                break;
            case 0x16: // asynchronous external abort
                ssdk_printf(SSDK_EMERG, "asynchronous external abort on %s\n", write ? "write" : "read");
                break;
            case 0x10: // TLB conflict event
            case 0x19: // synchronous parity error on memory access
            case 0x04: // fault on instruction cache maintenance
            case 0x0C: // synchronous external abort on translation table walk
            case 0x0E: //    "
            case 0x1C: // synchronous parity error on translation table walk
            case 0x1E: //    "
            case 0x18: // asynchronous parity error on memory access
            default:
                ssdk_printf(SSDK_EMERG, "unhandled fault\n");
                break;
        }

        ssdk_printf(SSDK_EMERG, "DFAR 0x%x (fault address)\n", far);
        ssdk_printf(SSDK_EMERG, "DFSR 0x%x (fault status register)\n", fsr);

        exception_die(frame, "halting\n");

#if ((defined(CONFIG_FuncSafetyPkg)) && ((FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)))
    }
#endif
}

void arm_prefetch_abort_handler(struct arm_fault_frame *frame)
{
    uint32_t fsr = arm_read_ifsr();
    uint32_t far = arm_read_ifar();

    uint32_t fault_status = (BIT(fsr, 10) ? (1<<4) : 0) |  BITS(fsr, 3, 0);

    ssdk_printf(SSDK_EMERG, "\n\ncpu prefetch abort, ");

    /* decode the fault status (from table B3-23) */
    switch (fault_status) {
        case 0x01: // alignment fault
            ssdk_printf(SSDK_EMERG, "alignment fault\n");
            break;
        case 0x05:
        case 0x07: // translation fault
            ssdk_printf(SSDK_EMERG, "translation fault\n");
            break;
        case 0x03:
        case 0x06: // access flag fault
            ssdk_printf(SSDK_EMERG, "access flag fault\n");
            break;
        case 0x09:
        case 0x0B: // domain fault
            ssdk_printf(SSDK_EMERG, "domain fault, domain %lu\n", BITS_SHIFT(fsr, 7, 4));
            break;
        case 0x0D:
        case 0x0F: // permission fault
            ssdk_printf(SSDK_EMERG, "permission fault\n");
            break;
        case 0x02: // debug event
            ssdk_printf(SSDK_EMERG, "debug event\n");
            break;
        case 0x08: // synchronous external abort
            ssdk_printf(SSDK_EMERG, "synchronous external abort\n");
            break;
        case 0x16: // asynchronous external abort
            ssdk_printf(SSDK_EMERG, "asynchronous external abort\n");
            break;
        case 0x10: // TLB conflict event
        case 0x19: // synchronous parity error on memory access
        case 0x04: // fault on instruction cache maintenance
        case 0x0C: // synchronous external abort on translation table walk
        case 0x0E: //    "
        case 0x1C: // synchronous parity error on translation table walk
        case 0x1E: //    "
        case 0x18: // asynchronous parity error on memory access
        default:
            ssdk_printf(SSDK_EMERG, "unhandled fault\n");
            break;
    }

    ssdk_printf(SSDK_EMERG, "IFAR 0x%x (fault address)\n", far);
    ssdk_printf(SSDK_EMERG, "IFSR 0x%x (fault status register)\n", fsr);

    exception_die(frame, "halting\n");
}
