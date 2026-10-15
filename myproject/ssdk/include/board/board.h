/**
 * @file board.h
 * @brief board config header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef INCLUDE_BOARD_H
#define INCLUDE_BOARD_H

#include <compiler.h>
#include <types.h>

#define CKGEN_SYS_CNT 3000000

__BEGIN_CDECLS

typedef enum {
    HALT_ACTION_HALT = 0,       // Spin forever.
    HALT_ACTION_REBOOT,         // Reset the CPU.
    HALT_ACTION_SHUTDOWN,       // Shutdown and power off.
} board_halt_action;

typedef enum {
    HALT_REASON_UNKNOWN = 0,
    HALT_REASON_POR,            // Cold-boot
    HALT_REASON_HW_WATCHDOG,    // HW watchdog timer
    HALT_REASON_LOWVOLTAGE,     // LV/Brownout condition
    HALT_REASON_HIGHVOLTAGE,    // High voltage condition.
    HALT_REASON_THERMAL,        // Thermal reason (probably overtemp)
    HALT_REASON_SW_RECOVERY,    // SW triggered reboot in order to enter recovery mode
    HALT_REASON_SW_RESET,       // Generic Software Initiated Reboot
    HALT_REASON_SW_WATCHDOG,    // Reboot triggered by a SW watchdog timer
    HALT_REASON_SW_PANIC,       // Reboot triggered by a SW panic or ASSERT
    HALT_REASON_SW_UPDATE,      // SW triggered reboot in order to begin firmware update
    HALT_REASON_SW_GLOBAL_POR,  // SW triggered global reboot
} board_halt_reason;

/**
 * @brief board init before irq enable
 */
void board_early_init(void);

/**
 * @brief board init after irq enable
 */
void board_init(void);

/**
 * @brief board set console device
 *
 * @param[in] dev console device
 */
void board_set_console(void *dev);

/**
 * @brief board get console device
 *
 * @return console device
 */
void* board_get_console(void);

/**
 * @brief board getc
 *
 * @return input char
 */
char board_getc(void);

/**
 * @brief board putc
 *
 * @param[in] output char
 */
void board_putc(char c);

/**
 * @brief board raw putc
 *
 * @param[in] output char
 */
void board_raw_putc(char c);

/**
 * @brief board udelay
 *
 * @param[in] usecs delay us
 */
void udelay(uint32_t usecs);

/**
 * @brief board get current time
 *
 * @return current time in ms
 */
uint32_t current_time(void);

/**
 * @brief board get current time
 *
 * @return current time in us
 */
uint64_t current_time_hires(void);

#define SSDK_CONSOLE            board_get_console()
#define SSDK_SET_CONSOLE(dev)   board_set_console(dev)

/**
 * @brief board halt
 *
 * @param[in] action halt action
 * @param[in] reason halt reason
 */
void board_halt(board_halt_action action,
                board_halt_reason reason) __NO_RETURN;

__END_CDECLS

#endif /* INCLUDE_BOARD_H */
