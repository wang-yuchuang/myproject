/**
 * @file lnk_sym.h
 * @brief IAR link symbols.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __LNK_SYM_H__
#define __LNK_SYM_H__

#ifdef __ICCARM__
#define __commands_start COMMANDS$$Base
#define __commands_end COMMANDS$$Limit
#define _heap_start HEAP$$Base
#define _heap_end HEAP$$Limit
#define __except_stack_start EXCEPTION_STACK$$Base
#define __except_stack_end EXCEPTION_STACK$$Limit
#define __data_start DATA$$Base
#define __data_start_rom DATA$$Base
#define __data_end DATA$$Limit
#define __bss_start BSS$$Base
#define __bss_end BSS$$Limit

#ifndef ASSEMBLY
#include <stdint.h>
extern uintptr_t EXCEPTION_STACK$$Base[];
extern uintptr_t EXCEPTION_STACK$$Limit[];
extern uintptr_t DATA$$Base[];
extern uintptr_t DATA$$Limit[];
extern uintptr_t BSS$$Base[];
extern uintptr_t BSS$$Limit[];
#endif /* ASSEMBLY */
#endif /* __ICCARM__ */

#endif /* __LNK_SYM_H__ */
