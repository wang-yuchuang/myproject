#include <stdlib.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <board.h>
#include "reg.h"

#define SYS_CNT_CNTCVUREAD      (0x4)
#define MILLION                 (1000000)

uint64_t semidrive_get_systime_us(void)
{
    uint64_t syscnt_low, syscnt_high, syscnt;
    int freq_mhz;

    syscnt_low = readl(APB_SYS_CNT_RO_BASE);
    syscnt_high = readl(APB_SYS_CNT_RO_BASE + SYS_CNT_CNTCVUREAD);

    syscnt = (syscnt_high << 32) | syscnt_low;

    freq_mhz = CKGEN_SYS_CNT / MILLION;

    return syscnt / freq_mhz;
}
