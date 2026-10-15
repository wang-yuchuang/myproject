#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <reg.h>
#include "str_api.h"
#include "arch/cache.h"

#define CACHE_LINE 32

#ifdef CONFIG_SUPPORT_STR_MODE
static uint32_t vote_flag = 0;
bool is_str_enter(enum STR_MASTER master)
{
    uint32_t v;
    uint32_t base = 0;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return false;
    }

    if (!base)
        return false;

    arch_invalidate_cache_range(base, CACHE_LINE);
    v = readl(base);
    if ((v & 0xffff) == 0x1234)
        return true;

    return false;
}
bool is_str_resume(enum STR_MASTER master)
{
    uint32_t v;
    uint32_t base = 0;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return false;
    }
    if (!base)
        return false;

    arch_invalidate_cache_range(base, CACHE_LINE);
    v = readl(base);

    if ((v & 0xffff) == 0x8765)
        return true;

    return false;
}
void clr_str_flag(enum STR_MASTER master)
{
    uint32_t base = 0;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            writel(0x0, STR_RTC_FLAG);//AP1 set rtc flag only
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            writel(0x0, STR_RTC_FLAG);//AP2 set rtc flag only
            break;

        default:
            return;
    }
    if (base) {
        writel(0, base);
        arch_clean_cache_range(base, CACHE_LINE);
    }
}
void set_str_flag_to_rtc(enum STR_MASTER master)
{
    uint32_t v;
    uint32_t base = 0;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            writel(0x12345678, STR_RTC_FLAG);//AP1 set rtc flag only
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            writel(0x12345678, STR_RTC_FLAG);//AP1 set rtc flag only
            break;

        default:
            return;
    }

    if (!base)
        return;

    v = readl(base);
    v &= 0xffff0000;
    v |= 0x8765;
    writel(v, base);
    arch_clean_cache_range(base, CACHE_LINE);
}
uint32_t get_str_resume_entry(enum STR_MASTER master)
{
    uint32_t base = 0;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return 0;
    }
    if (!base)
        return 0;
    arch_invalidate_cache_range(base, CACHE_LINE);
    return readl(base + 0x4);
}
void str_vote(enum STR_MASTER master)
{
    vote_flag |= 1 << master;
}
bool check_str_all(void)
{
    return (vote_flag == STR_VOTE_ALL);
}
void str_clr_vote(void)
{
    vote_flag = 0;
}
static bool get_str_flag_from_reg(void)
{
    uint32_t str_rtc_flg = readl(STR_RTC_FLAG);

    if (str_rtc_flg == 0x12345678)
        return true;

    return false;
}
static void set_str_resume(enum STR_MASTER master, bool flag)
{
    uint32_t base;
    writel(0, STR_RTC_FLAG);

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return;
    }

    if (flag) {
        writel(0x8765, base);
    }
    else {
        writel(0, base);
        writel(0, base + 4);
    }
}

void set_str_resume_early(void)
{
    bool str_flag = false;

    str_flag = get_str_flag_from_reg();

    if (str_flag) {
#if CONFIG_HAS_AP1
        set_str_resume(STR_AP1, str_flag);
#endif
#if CONFIG_HAS_AP2
        set_str_resume(STR_AP2, str_flag);
#endif
    }
}
#else
bool is_str_enter(enum STR_MASTER master) {return false;}
bool is_str_resume(enum STR_MASTER master) {return false;}
uint32_t get_str_resume_entry(enum STR_MASTER master) {return 0;}
void clr_str_flag(enum STR_MASTER master) {}
void set_str_flag_to_rtc(enum STR_MASTER master) {}
void str_vote(enum STR_MASTER master) {}
bool check_str_all(void) {return false;}
void str_clr_vote(void) {}
void set_str_resume_early(void) {}
__WEAK void power_config_before_str(void) {};
#endif
