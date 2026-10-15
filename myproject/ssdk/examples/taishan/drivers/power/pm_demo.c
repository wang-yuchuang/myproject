#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include <CLI_type.h>
#include <debug.h>
#include <board.h>
#include <driver/pm.h>

#ifdef CONFIG_SDRV_SMC
#include <driver/taishan/smc/sdrv_smc_drv.h>
#endif

#ifdef CONFIG_RTC
#include <rtc_cfg.h>
#define RTC_WAKEUP_INTERVAL_TIME 5
#endif

#if defined(CONFIG_TAISHAN_XSPI) && defined(CONFIG_DISK_NOR_FLASH)
#include "norflash_cfg.h"
#endif

struct driver_para {
    char *name;
    int  result;
};

static bool pm_registered;

static int driver_allowed[3];

static const struct driver_para driver_paras[3] = {
    {"driver1", 0},
    {"driver2", 1},
    {"driver3", 2}
};

static int driver_pm_suspend(struct pm_callback_s *cb, enum pm_state_e state)
{
    struct driver_para *para = (struct driver_para *)cb->cb_arg;

    printf("notify %s suspend to state %d\n", para->name, state);

    return driver_allowed[para->result];
}

static void driver_pm_resume(struct pm_callback_s *cb, enum pm_state_e state)
{
    struct driver_para *para = (struct driver_para *)cb->cb_arg;

    printf("notify %s leave state %d\n", para->name, state);
}

static struct pm_callback_s driver1_pm = {
    .cb_arg = (void *)&driver_paras[0],
    .suspend = driver_pm_suspend,
    .resume = driver_pm_resume,
};

static struct pm_callback_s driver2_pm = {
    .cb_arg = (void *)&driver_paras[1],
    .suspend = driver_pm_suspend,
    .resume = driver_pm_resume,
};

static struct pm_callback_s driver3_pm = {
    .cb_arg = (void *)&driver_paras[2],
    .suspend = driver_pm_suspend,
    .resume = driver_pm_resume,
};

static int pm_register_demo_main(int argc, char *argv[])
{
    if (!pm_registered) {
        pm_registered = true;
        pm_register(&driver1_pm);
        pm_register(&driver2_pm);
        pm_register(&driver3_pm);
    }

    for (int i = 0; i < 3 && i < argc; i++) {
        int allow = atoi(argv[i]);
        driver_allowed[i] = allow;
    }

    for (int i = 0; i < 3; i++) {
        printf("driver%d allowed %d\n", i, driver_allowed[i]);
    }

    return CLI_FALSE;
}

static int pm_request_demo_main(int argc, char *argv[])
{
    int pmstate;

    if (argc == 1) {
        pmstate = atoi(argv[0]);
        if (pmstate >= 0 && pmstate < PM_COUNT) {
            printf("PM request state %d\n", pmstate);
            pm_request(pmstate);
        }
    }

    return CLI_FALSE;
}


CLI_CMD("pm_register",
        "\r\npm_register:\r\n pm_register {driver1-allowed  driver2-allowed  driver3-allowed} \r\n Allowed:0, Not-allowed:1\r\n",
        pm_register_demo_main);

CLI_CMD("pm_request", "\r\npm_request:\r\n Request a pm state: Sleep-2, Hibernate-3\r\n", pm_request_demo_main);


#ifdef CONFIG_RTC
void rtc_wakeup_config(void)
{
    struct rtc_wkalrm wkalrm;
    rtc_enable_alarm(&g_sec_rtc_dev, false);

    rtc_get_time(&g_sec_rtc_dev, &wkalrm.tm);
    printf("cur time %d-%d-%d %d:%d:%d\n", wkalrm.tm.tm_year + 1900, wkalrm.tm.tm_mon,
                wkalrm.tm.tm_mday, wkalrm.tm.tm_hour, wkalrm.tm.tm_min, wkalrm.tm.tm_sec);

    wkalrm.tm.tm_sec += RTC_WAKEUP_INTERVAL_TIME;
    printf("wakeup time %d-%d-%d %d:%d:%d\n", wkalrm.tm.tm_year + 1900, wkalrm.tm.tm_mon,
                wkalrm.tm.tm_mday, wkalrm.tm.tm_hour, wkalrm.tm.tm_min, wkalrm.tm.tm_sec);

    rtc_set_alarm(&g_sec_rtc_dev, &wkalrm, NULL);

    rtc_enable_alarm(&g_sec_rtc_dev, true);
}

static int rtc_wakeup_main(int argc, char *argv[])
{
    rtc_wakeup_config();

#if defined(CONFIG_TAISHAN_XSPI) && defined(CONFIG_DISK_NOR_FLASH)
    board_norflash_deinit();
#endif

    printf("PM request power down!\n");
    pm_request(PM_SHUTDOWN);

    return CLI_FALSE;
}

CLI_CMD("rtc_wakeup", "\r\nrtc_wakeup:\r\n RTC wakeup from powerdown\r\n", rtc_wakeup_main);
#endif
