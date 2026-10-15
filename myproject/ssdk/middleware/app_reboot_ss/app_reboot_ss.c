/**
 * @file app_reboot_ss.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <board.h>
#include <rstgen_cfg.h>
#include <reg.h>
#include <regs_base.h>
#include "reboot_cfg.h"
#include "remap.h"
#include "rstgen_cfg.h"
#include <CLI.h>
#include <mbox_cfg.h>
#include "driver/gpio.h"
#include "driver/kunlun/gpio/sdrv_gpio.h"

#include <sys_diagnosis.h>

#include "rtc_cfg.h"
#include "driver/pinctrl.h"

#include "driver/watchdog.h"
#include "driver/kunlun/watchdog/sdrv_watchdog_drv.h"
#include "str_api.h"
#include "kunlun_sem_lld.h"
#include "arch/cache.h"
#include <driver/kunlun/reset/sdrv_reset_drv.h>
#include "sd_errcode.h"
#ifdef CONFIG_SUPPORT_STR_MODE
#include <ddr_lowpower.bin.h>
#include <str_cfg.h>
#endif

#ifdef CONFIG_CDNS_OSPI
#include <../../boards/d9_ref/safety/configs/ospi_cfg.h>
#endif


/* =========================== marco define session ======================== */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#define SDRV_REG_ROM        0
#define SDRV_REG_REMAP      1
#define SDRV_REG_SDPE       2
#define SDRV_REG_MCAL       3
#define SDRV_REG_BOOTREASON 4
#define SDRV_REG_HWID       5
#define SDRV_REG_STATUS     6
#define SDRV_REG_UNUSED     7

#ifdef CONFIG_ARCH_CHIP_X9_THANG
#define PINCTRL_RTC_PIN_START   253
#else
#define PINCTRL_RTC_PIN_START   163
#endif

#define WAKEUP_SOURCE_NULL      0x0
#define WAKEUP_SOURCE_RTC1      0x1
#define WAKEUP_SOURCE_RTC2      0x2
#define WAKEUP_SOURCE_EXT1      0x4
#define WAKEUP_SOURCE_EXT2      0x8

#define PMU_EXT_CTRL            (APB_PMU_BASE + 0x00)
#define PMU_RTC_CTRL            (APB_PMU_BASE + 0x04)
#define PMU_RST_CTRL            (APB_PMU_BASE + 0x0c)
#define PMU_CTRL_0              (APB_PMU_BASE + 0x10)

#define CACHE_LINE 32
#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define HIGH_LEVEL              1
#define DEFAULT_MOD_NO          0

/* =========================== enumeration define session ================== */

/* =========================== structure define session ==================== */
struct reboot_reason_desc;
typedef int (*reboot_action)(const struct reboot_reason_desc *);
struct reboot_reason_desc {
    const char *desc;
    board_halt_reason reason;
    reboot_action action;
};

#pragma pack(push)
#pragma pack(4)
typedef union reboot_args {
    struct {
        uint32_t reason: 4;
        uint32_t source: 4;
        uint32_t para:   24;
    } args;
    uint32_t val;
} reboot_args_t;
#pragma pack(pop)

/* =========================== function declare session ==================== */
static void hard_wdt_clear(int32_t wdtId);
/* =========================== global value define session ================= */
extern pinctrl_t g_pinctrl_rtc;
/* =========================== static value define session ================= */
static int global_reset_signal;
static bool dom_reboot_en = true;
/* =========================== function implementation session ============= */
#ifdef CONFIG_NOTIFY_SEC_REBOOT
#include <malloc.h>
#include <rpmsg_service.h>
#include <rpmsg_channel.h>

#define DOMAIN_EXCEPTION_PROC_EPT   340

#ifndef _def_domain_cpu_id_t
#define _def_domain_cpu_id_t
typedef enum {
    DP_CR5_SAF,
    DP_CR5_SEC,
    DP_CR5_MPC,
    DP_CA_AP1,
    DP_CA_AP2,
    DP_DSP_V,
    DP_CPU_MAX,
#if (defined ULINK_SIDE_A) || (defined ULINK_SIDE_B)
    R_DP_CR5_SAF = DP_CPU_MAX,
    R_DP_CR5_SEC,
    R_DP_CR5_MPC,
    R_DP_CA_AP1,
    R_DP_CA_AP2,
    R_DP_DSP_V,
    ALL_DP_CPU_MAX,
#endif
} domain_cpu_id_t;
#endif

enum domain_exception_type
{
    E_RECONFIG_AP1 = 0,
    E_REBOOT_DOM   = 0x1000,
    E_STR_DOM,
    E_SHUTDOWN_DOM,
    E_CMD_ACK      = 0x2000,
};

enum reboot_dom_target{
    E_AP1_DOM = 0x1,
    E_AP2_DOM,
    E_MP_DOM,
    E_SEC_DOM,
    E_SAF_DOM,
    E_AP_DOM,
    E_GLOBAL_DOM,
};

struct domain_exception_msg{
    uint32_t type;        /* option: domain_exception_type */
    uint32_t target;     /* if cmd == E_REBOOT_DOM, target option: E_AP1_DOM ... E_SAF_DOM */
    uint32_t reason;
    uint8_t reserved[48];/* must be zero */
    uint32_t crc32;      /* crc32 = crc32(struct  domain_exception_msg), crc32 filed must set zero before calculating crc32 */
} __PACKED;

static uint32_t crc_table[256] = {
    0x00000000UL, 0x77073096UL, 0xee0e612cUL, 0x990951baUL, 0x076dc419UL,
    0x706af48fUL, 0xe963a535UL, 0x9e6495a3UL, 0x0edb8832UL, 0x79dcb8a4UL,
    0xe0d5e91eUL, 0x97d2d988UL, 0x09b64c2bUL, 0x7eb17cbdUL, 0xe7b82d07UL,
    0x90bf1d91UL, 0x1db71064UL, 0x6ab020f2UL, 0xf3b97148UL, 0x84be41deUL,
    0x1adad47dUL, 0x6ddde4ebUL, 0xf4d4b551UL, 0x83d385c7UL, 0x136c9856UL,
    0x646ba8c0UL, 0xfd62f97aUL, 0x8a65c9ecUL, 0x14015c4fUL, 0x63066cd9UL,
    0xfa0f3d63UL, 0x8d080df5UL, 0x3b6e20c8UL, 0x4c69105eUL, 0xd56041e4UL,
    0xa2677172UL, 0x3c03e4d1UL, 0x4b04d447UL, 0xd20d85fdUL, 0xa50ab56bUL,
    0x35b5a8faUL, 0x42b2986cUL, 0xdbbbc9d6UL, 0xacbcf940UL, 0x32d86ce3UL,
    0x45df5c75UL, 0xdcd60dcfUL, 0xabd13d59UL, 0x26d930acUL, 0x51de003aUL,
    0xc8d75180UL, 0xbfd06116UL, 0x21b4f4b5UL, 0x56b3c423UL, 0xcfba9599UL,
    0xb8bda50fUL, 0x2802b89eUL, 0x5f058808UL, 0xc60cd9b2UL, 0xb10be924UL,
    0x2f6f7c87UL, 0x58684c11UL, 0xc1611dabUL, 0xb6662d3dUL, 0x76dc4190UL,
    0x01db7106UL, 0x98d220bcUL, 0xefd5102aUL, 0x71b18589UL, 0x06b6b51fUL,
    0x9fbfe4a5UL, 0xe8b8d433UL, 0x7807c9a2UL, 0x0f00f934UL, 0x9609a88eUL,
    0xe10e9818UL, 0x7f6a0dbbUL, 0x086d3d2dUL, 0x91646c97UL, 0xe6635c01UL,
    0x6b6b51f4UL, 0x1c6c6162UL, 0x856530d8UL, 0xf262004eUL, 0x6c0695edUL,
    0x1b01a57bUL, 0x8208f4c1UL, 0xf50fc457UL, 0x65b0d9c6UL, 0x12b7e950UL,
    0x8bbeb8eaUL, 0xfcb9887cUL, 0x62dd1ddfUL, 0x15da2d49UL, 0x8cd37cf3UL,
    0xfbd44c65UL, 0x4db26158UL, 0x3ab551ceUL, 0xa3bc0074UL, 0xd4bb30e2UL,
    0x4adfa541UL, 0x3dd895d7UL, 0xa4d1c46dUL, 0xd3d6f4fbUL, 0x4369e96aUL,
    0x346ed9fcUL, 0xad678846UL, 0xda60b8d0UL, 0x44042d73UL, 0x33031de5UL,
    0xaa0a4c5fUL, 0xdd0d7cc9UL, 0x5005713cUL, 0x270241aaUL, 0xbe0b1010UL,
    0xc90c2086UL, 0x5768b525UL, 0x206f85b3UL, 0xb966d409UL, 0xce61e49fUL,
    0x5edef90eUL, 0x29d9c998UL, 0xb0d09822UL, 0xc7d7a8b4UL, 0x59b33d17UL,
    0x2eb40d81UL, 0xb7bd5c3bUL, 0xc0ba6cadUL, 0xedb88320UL, 0x9abfb3b6UL,
    0x03b6e20cUL, 0x74b1d29aUL, 0xead54739UL, 0x9dd277afUL, 0x04db2615UL,
    0x73dc1683UL, 0xe3630b12UL, 0x94643b84UL, 0x0d6d6a3eUL, 0x7a6a5aa8UL,
    0xe40ecf0bUL, 0x9309ff9dUL, 0x0a00ae27UL, 0x7d079eb1UL, 0xf00f9344UL,
    0x8708a3d2UL, 0x1e01f268UL, 0x6906c2feUL, 0xf762575dUL, 0x806567cbUL,
    0x196c3671UL, 0x6e6b06e7UL, 0xfed41b76UL, 0x89d32be0UL, 0x10da7a5aUL,
    0x67dd4accUL, 0xf9b9df6fUL, 0x8ebeeff9UL, 0x17b7be43UL, 0x60b08ed5UL,
    0xd6d6a3e8UL, 0xa1d1937eUL, 0x38d8c2c4UL, 0x4fdff252UL, 0xd1bb67f1UL,
    0xa6bc5767UL, 0x3fb506ddUL, 0x48b2364bUL, 0xd80d2bdaUL, 0xaf0a1b4cUL,
    0x36034af6UL, 0x41047a60UL, 0xdf60efc3UL, 0xa867df55UL, 0x316e8eefUL,
    0x4669be79UL, 0xcb61b38cUL, 0xbc66831aUL, 0x256fd2a0UL, 0x5268e236UL,
    0xcc0c7795UL, 0xbb0b4703UL, 0x220216b9UL, 0x5505262fUL, 0xc5ba3bbeUL,
    0xb2bd0b28UL, 0x2bb45a92UL, 0x5cb36a04UL, 0xc2d7ffa7UL, 0xb5d0cf31UL,
    0x2cd99e8bUL, 0x5bdeae1dUL, 0x9b64c2b0UL, 0xec63f226UL, 0x756aa39cUL,
    0x026d930aUL, 0x9c0906a9UL, 0xeb0e363fUL, 0x72076785UL, 0x05005713UL,
    0x95bf4a82UL, 0xe2b87a14UL, 0x7bb12baeUL, 0x0cb61b38UL, 0x92d28e9bUL,
    0xe5d5be0dUL, 0x7cdcefb7UL, 0x0bdbdf21UL, 0x86d3d2d4UL, 0xf1d4e242UL,
    0x68ddb3f8UL, 0x1fda836eUL, 0x81be16cdUL, 0xf6b9265bUL, 0x6fb077e1UL,
    0x18b74777UL, 0x88085ae6UL, 0xff0f6a70UL, 0x66063bcaUL, 0x11010b5cUL,
    0x8f659effUL, 0xf862ae69UL, 0x616bffd3UL, 0x166ccf45UL, 0xa00ae278UL,
    0xd70dd2eeUL, 0x4e048354UL, 0x3903b3c2UL, 0xa7672661UL, 0xd06016f7UL,
    0x4969474dUL, 0x3e6e77dbUL, 0xaed16a4aUL, 0xd9d65adcUL, 0x40df0b66UL,
    0x37d83bf0UL, 0xa9bcae53UL, 0xdebb9ec5UL, 0x47b2cf7fUL, 0x30b5ffe9UL,
    0xbdbdf21cUL, 0xcabac28aUL, 0x53b39330UL, 0x24b4a3a6UL, 0xbad03605UL,
    0xcdd70693UL, 0x54de5729UL, 0x23d967bfUL, 0xb3667a2eUL, 0xc4614ab8UL,
    0x5d681b02UL, 0x2a6f2b94UL, 0xb40bbe37UL, 0xc30c8ea1UL, 0x5a05df1bUL,
    0x2d02ef8dUL
};

/* ========================================================================= */
#define DO1 crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8)
#define DO8 DO1; DO1; DO1; DO1; DO1; DO1; DO1; DO1
/* ========================================================================= */
static uint32_t crc32(uint32_t crc, const unsigned char *buf, uint32_t len)
{
    if (buf == NULL) return 0UL;

    crc = crc ^ 0xffffffffUL;
    while (len >= 8) {
        DO8;
        len -= 8;
    }
    if (len) do {
        DO1;
    } while (--len);
    return crc ^ 0xffffffffUL;
}
#endif

static int global_reset_sig_get(void)
{
    return global_reset_signal;
}

static inline void reboot_reason_set(board_halt_reason reason)
{
    writel(reason, SDRV_GENERAL_REG(SDRV_REG_BOOTREASON));
}

static int32_t boot_ap_reason(const struct reboot_reason_desc *desc)
{
    if (desc == NULL) {
        ssdk_printf(SSDK_ERR, "parameter error for %s\n", __func__);
        return -1;
    }

    reboot_reason_set(desc->reason);

    boot_ap1();

    return 0;
}

static int32_t global_reboot(const struct reboot_reason_desc *desc)
{
    // sw reset
    udelay(3000);
#ifdef CONFIG_CDNS_OSPI
#ifndef CONFIG_XIP_MODE
    board_norflash_deinit();   /* XIP 下禁止 (2026-09-04 B4): R 核代码正从 NOR XIP 执行，
                                * deinit 会切断正在取指的指令源 → 复位流程崩溃整板卡死。
                                * emmc_only(ref) 代码在 SRAM 执行，deinit 无碍 → 保持原行为 */
#endif
#endif
    reset_ctl_assert(&rstsig_sw_self_rst);
    reset_ctl_deassert(&rstsig_sw_self_rst);
    return 0;
}

static int32_t reboot_help(void)
{
    printf("reboot [cmd]: \n");
    printf("\tsoc: reboot soc \n");
    printf("\tbootloader: reboot ap and run fastboot mode\n");
    printf("\trecovery: reboot ap and run recovery mode\n");
    return 0;
}

static int32_t reboot_cmd(int32_t argc, char *argv[])
{
    const struct reboot_reason_desc reboot_desc[] = {
        { "bootloader", HALT_REASON_SW_UPDATE, boot_ap_reason},
        { "recovery",   HALT_REASON_SW_RECOVERY, boot_ap_reason},
        { "soc",    HALT_REASON_SW_GLOBAL_POR, global_reboot},
        { "help",    -1, (reboot_action)reboot_help},
    };

    for (int32_t i = 0; i < ARRAY_SIZE(reboot_desc); i++) {
        if (strcmp(argv[0], reboot_desc[i].desc) == 0 &&
                strlen(argv[0]) == strlen(reboot_desc[i].desc)) {
            reboot_desc[i].action(&reboot_desc[i]);
            break;
        }
    }

    return 0;
}

int32_t cmd_dom_reboot_en(int32_t argc, char *argv[])
{
    dom_reboot_en = strtoul(argv[0], NULL, 0);
    printf("Domain reboot enable[%d] success\n", dom_reboot_en);
    return 0;
}

static int do_notify_reboot_to_mp(int32_t signal)
{
    int ret = -1;
    mbox_chan_t *chan;
    sdrv_mbox_chan_req_t chan_req;
    int32_t msg_payload = signal;

    chan_req.rproc = MBOX_RPROC_MP;
    chan_req.dest_addr = 0x7F;
    chan_req.src_addr = 0x7F;

    chan = mbox_request_channel(g_mbox_dev, &chan_req);

    if (NULL == chan) {
        ssdk_printf(0, "%s: mbox request channel failed!!\n", __func__);
        return ret;
    }

    ret = mbox_send(chan, (uint8_t *)&msg_payload, sizeof(int32_t), 1000);

    if (ret) {
        ssdk_printf(0, "%s: send msg fail\n", __func__);
        return ret;
    }

    mbox_release_channel(chan);

    return ret;
}

void hard_wdt_clear(int32_t wdtId)
{
    addr_t base = 0;

    switch (wdtId) {
        case WDT5____ovflow_int:
            base = 0xf0a20024;
            break;

        case WDT6____ovflow_int:
            base = 0xf0a30024;
            break;

        default:
            break;
    }

    if (base != 0) {
        // clear watchdog
        uint32_t val = readl(base);
        writel(val | 1 << 8, base);
    }
}

static void rtc_domain_pin_config(void)
{
    size_t i;
    uint32_t pin_list[] = { PINCTRL_RTC_PIN_START + 5,  //PAD_CONFIG_SYS_PWR_ON
                            PINCTRL_RTC_PIN_START + 8,  //PAD_CONFIG_SYS_CTRL0
                            PINCTRL_RTC_PIN_START + 9,  //PAD_CONFIG_SYS_CTRL1
                            PINCTRL_RTC_PIN_START + 10, //PAD_CONFIG_SYS_CTRL2
                            PINCTRL_RTC_PIN_START + 11  //PAD_CONFIG_SYS_CTRL3
                          };

    for (i = 0; i < ARRAY_SIZE(pin_list); i++)
        pinctrl_config(&g_pinctrl_rtc, pin_list[i],
                       PINCTRL_CONFIG_PULL, PINCTRL_PULL_NOPULL);
}

static void rtc_clk_switch_xtal(void)
{
    if ((readl(APB_XTAL_RTC_BASE) & 0x80000003) == 0x80000002) {
        /* Oscillator setting from register. */
        RMWREG32(APB_RC_RTC_BASE, 14, 1, 0);

        /* select xtal_32k */
        RMWREG32(APB_RC_RTC_BASE + 4, 0, 1, 1);

        /* disable osc (rc_rtc) */
        RMWREG32(APB_RC_RTC_BASE, 0, 1, 0);
    }
}

static void reboot_rtc_pre_config(void)
{
    rtc_domain_pin_config();
    rtc_clk_switch_xtal();
}

#ifdef CONFIG_RTC
static int reboot_rtc_alarm_set(rtc_dev_t *dev, int timeout)
{
    struct rtc_time tm;
    struct rtc_wkalrm alarm;
    uint64_t expect_sec;
    int ret = -1;

    /* rtc init. */
    rtc_init(dev);

    /* get default rtc time */
    ret = rtc_get_time(dev, &tm);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s rtc set time failed\n", __func__);
        return ret;
    }

    expect_sec = rtc_to_epoch(&tm);
    expect_sec += timeout;
    epoch_to_rtc(&alarm.tm, expect_sec);

    /* the rtc must be enabled before seting rtc alarm args,
     * otherwise the function will block here.
     */
    ret = rtc_set_alarm(dev, &alarm, NULL);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s rtc set alarm failed\n", __func__);
        return ret;
    }

    /* enable rtc alarm */
    ret = rtc_enable_alarm(dev, true);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s rtc enable alarm failed\n", __func__);
        return ret;
    }

    return 0;
}
#endif

static int32_t pmu_enable_wakeup(bool en, uint8_t source)
{
    if (WAKEUP_SOURCE_NULL == source) {
        ssdk_printf(SSDK_ERR, "invalid wakeup source: %d\n", source);
        return -1;
    }

    if (source == WAKEUP_SOURCE_RTC1) {
        RMWREG32(PMU_RTC_CTRL, 0, 1, en ? 1 : 0);
    }
    else if (source == WAKEUP_SOURCE_RTC2) {
        RMWREG32(PMU_RTC_CTRL, 1, 1, en ? 1 : 0);
    }
    else if (source == WAKEUP_SOURCE_EXT1) {
        RMWREG32(PMU_EXT_CTRL, 14, 1, en ? 1 : 0);
    }
    else if (source == WAKEUP_SOURCE_EXT2) {
        RMWREG32(PMU_EXT_CTRL, 21, 1, en ? 1 : 0);
    }

    return 0;
}

static void pmu_shut_down(void)
{
    RMWREG32(PMU_RST_CTRL, 9, 1, 1);  /* POR cell down */

    RMWREG32(PMU_CTRL_0, 8, 1, 0);    /* PMU power down */
    udelay(1000);
    RMWREG32(PMU_CTRL_0, 8, 1, 1);

    while (1);
}

static void reboot_wakeup_config(reboot_args_t reboot_args)
{
    if ((reboot_args.args.source & WAKEUP_SOURCE_RTC1) &&
            reboot_args.args.para) {
#ifdef CONFIG_RTC
        reboot_rtc_alarm_set(&g_rtc1_dev, reboot_args.args.para);
#endif
        pmu_enable_wakeup(true, WAKEUP_SOURCE_RTC1);
    }
    else {
        pmu_enable_wakeup(false, WAKEUP_SOURCE_RTC1);
    }

    if (reboot_args.args.source & WAKEUP_SOURCE_EXT1) {
        pmu_enable_wakeup(true, WAKEUP_SOURCE_EXT1);
    }
    else {
        pmu_enable_wakeup(false, WAKEUP_SOURCE_EXT1);
    }

    if (reboot_args.args.source & WAKEUP_SOURCE_EXT2) {
        pmu_enable_wakeup(true, WAKEUP_SOURCE_EXT2);
    }
    else {
        pmu_enable_wakeup(false, WAKEUP_SOURCE_EXT2);
    }
}

static void system_power_off(reboot_args_t reboot_args)
{
    ssdk_printf(SSDK_INFO, "system shutdown\n");

    arch_irq_disable();

    reboot_rtc_pre_config();
    reboot_wakeup_config(reboot_args);

#ifdef CONFIG_SUPPORT_STR_MODE
    power_config_before_shutdown();
#endif

    pmu_shut_down();
}

#ifdef CONFIG_SUPPORT_CRASHDUMP_FEATURE
#include "logflag.h"
static int sdrv_exception_handle(int signal)
{
    switch (signal) {
        case WDT5____ovflow_int:
            sdrv_ap_exp_flag_set(1);
            sdrv_crashdump_flag_set(1);
            printf("Observed ap1 exception reboot\n");
            break;

        case WDT6____ovflow_int:
            sdrv_ap_exp_flag_set(1);
#ifndef CONFIG_HAS_AP1
            sdrv_crashdump_flag_set(1);
            printf("ap2 dump set\n");
#endif
            printf("Observed ap2 exception reboot\n");
            break;

        case WDT4____ovflow_int:
            printf("Observed mp exception reboot\n");
            break;

        default:
            printf("unhandle reboot signal!\n");
            return 0;
    }

    return 0;
}
#endif

int32_t module_sw_reboot(int32_t signal, void *args)
{
    ssdk_printf(SSDK_CRIT, "receive sem signal: %d\n", signal);

    //@TODO reboot other core
    switch (signal) {
        case WDT5____ovflow_int:
            printf("Observed ap1 wdt reboot\n");
            hard_wdt_clear(WDT5____ovflow_int);
            boot_ap1();
            break;

        case WDT6____ovflow_int:
            printf("Observed ap2 wdt reboot\n");
            hard_wdt_clear(WDT6____ovflow_int);
            boot_ap2();
            break;

        case WDT4____ovflow_int:
            printf("Observed mp wdt reboot\n");
            break;

        default:
            printf("unhandle reboot signal!\n");
            ERRCODE_PRINT(MOD_ID_RESET, DEFAULT_MOD_NO, RESET_ERR_UNHANDLE_SIG, signal);
            return 0;
    }

    return 0;
}

#ifdef CONFIG_SUPPORT_STR_MODE
static void switch_to_os(void *addr, void *base, uint32_t size)
{

    memcpy(addr, base, size);
    arch_clean_cache_range((addr_t)addr, ROUNDUP(size, CACHE_LINE));
    vector_remap((addr_t)addr);
#if (defined CONFIG_BOARD_D9360_REF) || (defined CONFIG_BOARD_D9342_REF)
    reset_ctl_reset(&rstsig_cr5_sec);
#else
    reset_ctl_reset(&rstsig_cr5_saf);
#endif
    while (1);
}

static void str_reboot_or_shutdown(int signal, reboot_args_t reboot_args,
                                   int global_reset_src)
{
    if (WDT5____ovflow_int == signal) {
        if (is_str_enter(STR_AP1)) {
            str_vote(STR_AP1);
            sem_map_signal(SEM1, signal, SEM_INTR_CPU, false);
        }
    }
    else if (WDT6____ovflow_int == signal) {
        if (is_str_enter(STR_AP2)) {
            str_vote(STR_AP2);
            sem_map_signal(SEM1, signal, SEM_INTR_CPU, false);
        }
    }
    else {
        printf("str only support AP1/AP2 now\n");
    }

    if (check_str_all()) {
        str_clr_vote();
        set_str_flag_to_rtc(STR_AP1);
        set_str_flag_to_rtc(STR_AP2);
        reboot_rtc_pre_config();
        reboot_wakeup_config(reboot_args);
        power_config_before_str();
        switch_to_os((void *)0x00100000, (void *)ddr_lowpower_image,
                     sizeof(ddr_lowpower_image));
    }
}
#endif

#ifdef CONFIG_NOTIFY_SEC_REBOOT
static struct rpmsg_channel_t *reboot_chan = NULL;
static struct domain_exception_msg *reboot_msg = NULL;

void reboot_notify(uint32_t cmd, uint32_t target, uint32_t reason)
{
    char *buf;
    int len = 0, ret = 0;
    uint32_t crc32_val;

    if (dom_reboot_en == false) {
        global_reboot(NULL);
    }

    len = sizeof(struct domain_exception_msg);
    reboot_msg = memalign(CACHE_LINE, len);
    if (!reboot_msg) {
        ssdk_printf(SSDK_ALERT, "failed to allocate reboot_msg\n");
        return;
    }
    memset(reboot_msg, 0, len);
    reboot_msg->type = cmd;
    reboot_msg->target = target;
    reboot_msg->reason = reason;
    crc32_val = crc32(0, (void *)reboot_msg, len);
    reboot_msg->crc32 = crc32_val;

    buf = (void *)reboot_msg;
    if (reboot_chan) {
        ret = rpmsg_channel_send(reboot_chan, DOMAIN_EXCEPTION_PROC_EPT, buf, len, 500);
        if (ret < 0)
            ssdk_printf(SSDK_CRIT, "app reboot ss failed ipcc channel send\n");
    } else {
        ssdk_printf(SSDK_CRIT, "app reboot ss failed create ipcc channel\n");
    }
    free(buf);
}
#endif

int32_t reboot_routine(int32_t signal, void *args)
{
    reboot_args_t reboot_args;
    uint16_t err_code = RESET_ERR_NO_ERR;

    reboot_args.val = readl(SDRV_GENERAL_REG(SDRV_REG_BOOTREASON));
    reboot_args.args.source |= WAKEUP_SOURCE_EXT1;
    printf("get reboot args: 0x%x, signal: %d\n", reboot_args.val, signal);

#ifdef CONFIG_SUPPORT_STR_MODE
    if (is_str_enter(STR_AP1) || is_str_enter(STR_AP2)) {
#ifdef CONFIG_NOTIFY_SEC_REBOOT
        hard_wdt_clear(signal);
        reboot_notify(E_STR_DOM, E_GLOBAL_DOM, 0);
        return 0;
#else
        str_reboot_or_shutdown(signal, reboot_args, WDT5____ovflow_int);
        return 0;
#endif
    }
#endif
    switch (reboot_args.args.reason) {
        case HALT_REASON_SW_RESET:
#ifdef CONFIG_NOTIFY_SEC_REBOOT
            hard_wdt_clear(signal);
            if (signal == WDT5____ovflow_int) {
                reboot_notify(E_REBOOT_DOM, E_AP1_DOM, reboot_args.val);
            } else if (signal == WDT6____ovflow_int) {
                reboot_notify(E_REBOOT_DOM, E_AP2_DOM, reboot_args.val);
            } else {
                global_reboot(NULL);
            }
#else
            module_sw_reboot(signal, args);
#endif
            break;

        case HALT_REASON_HW_WATCHDOG:
        case HALT_REASON_SW_PANIC:
            hard_wdt_clear(signal);
            err_code = reboot_args.args.reason == HALT_REASON_HW_WATCHDOG ?
                       RESET_ERR_HW_WATCHDOG : RESET_ERR_SW_PANIC;

            ERRCODE_PRINT(MOD_ID_RESET, DEFAULT_MOD_NO, err_code, signal);

#ifdef CONFIG_SUPPORT_CRASHDUMP_FEATURE
            sdrv_exception_handle(signal);
#endif

#ifdef CONFIG_SDLOG_CRASH_DUMP

#if defined(CONFIG_CDNS_OSPI) && defined(CONFIG_DISK_NOR_FLASH)
            sdlog_dump_crash_log(signal, reboot_args.args.reason);
#else
            if (signal != WDT3____ovflow_int) {
                sdlog_dump_crash_log(signal, reboot_args.args.reason);
            }
#endif

#endif
#ifdef CONFIG_DEBUG_VER
            break;
#endif
#ifdef CONFIG_NOTIFY_SEC_REBOOT
            if (signal == WDT5____ovflow_int) {
                reboot_notify(E_REBOOT_DOM, E_AP1_DOM, reboot_args.val);
            } else if (signal == WDT6____ovflow_int) {
                reboot_notify(E_REBOOT_DOM, E_AP2_DOM, reboot_args.val);
            } else {
                global_reboot(NULL);
            }
#else
            global_reboot(NULL);
#endif
            break;

        case HALT_REASON_POR:
            system_power_off(reboot_args);
            break;

        case HALT_REASON_SW_GLOBAL_POR:
            global_reboot(NULL);
            break;

        case HALT_REASON_SW_RECOVERY:
        case HALT_REASON_SW_UPDATE:
#ifdef CONFIG_NOTIFY_SEC_REBOOT
            hard_wdt_clear(signal);
            reboot_notify(E_REBOOT_DOM, E_AP1_DOM, reboot_args.val);
            break;
#else
            module_sw_reboot(signal, args);
            break;
#endif

        case HALT_REASON_UNKNOWN:
        default:
            reboot_reason_set(HALT_REASON_HW_WATCHDOG);
            printf("Ignored unknown reboot reason: 0x%x!\n",
                   reboot_args.args.reason);
            break;
    }

    do_notify_reboot_to_mp(signal);

    return 0;
}

int32_t app_reboot_ss_init(int sig)
{
    global_reset_signal = sig;
    reboot_reason_set(HALT_REASON_HW_WATCHDOG);
    sysd_register_daemon_handler(reboot_routine, NULL, WDT6____ovflow_int);
    sysd_register_daemon_handler(reboot_routine, NULL, WDT5____ovflow_int);
    sysd_register_daemon_handler(reboot_routine, NULL, WDT4____ovflow_int);
    sysd_register_daemon_handler(reboot_routine, NULL, WDT3____ovflow_int);
#ifdef CONFIG_RELEASE_VER
    pmic_reboot_enable();
#endif

#ifdef CONFIG_NOTIFY_SEC_REBOOT
    struct rpmsg_instance *rpmsg_dev;
    rpmsg_dev = rpmsg_get_instance(RPMSG_TYPE_IPCC, DP_CR5_SEC);
    if (rpmsg_dev) {
        reboot_chan = rpmsg_channel_create(rpmsg_dev, "reboot_notify", DOMAIN_EXCEPTION_PROC_EPT);
        if (reboot_chan) {
            rpmsg_channel_start(reboot_chan, NULL, NULL);
        } else {
            ssdk_printf(SSDK_CRIT, "app reboot ss failed create ipcc channel\n");
        }
    } else {
        ssdk_printf(SSDK_CRIT, "app reboot ss failed to get rpmsg device\n");
    }
#endif
    return 0;
}

#ifdef CONFIG_SUPPORT_CRASHDUMP_FEATURE
static void wdt_start(uint32_t wdg_idx, uint32_t timeout)
{
    static watchdog_device_t g_wdt;
    static sdrv_watchdog_device_t g_sdrv_wdt;
    g_sdrv_wdt.base = wdg_idx;

    if (sdrv_watchdog_init(&g_wdt, &g_sdrv_wdt)) {
        printf("sdrv_watchdog_init fail.\n");
        return;
    }

    if (watchdog_set_timeout(&g_wdt, timeout)) {
        printf("watchdog_set_timeout fail.\n");
        return;
    }

    if (watchdog_start(&g_wdt)) {
        printf("watchdog_start fail.\n");
        return;
    }
}

static int cmd_startwdt(int argc, char *argv[])
{
    if (argc < 1) {
        printf("startwdt [ap1|ap2]\n");
        return CLI_FALSE;
    }

    if (strncmp(argv[0], "ap1", strlen("ap1")) == 0) {
        wdt_start(APB_WDT5_BASE, 1);
    }
    else if (strncmp(argv[0], "ap2", strlen("ap2")) == 0) {
        wdt_start(APB_WDT6_BASE, 1);
    }
    else
        printf("Don't support this process core: %s\n", argv[0]);

    return CLI_FALSE;
}

CLI_CMD("startwdt", "startwdt [ap1|ap2]", cmd_startwdt);
#endif

int32_t cmd_boot_ap1_wrapper(int32_t argc, char *argv[])
{
#ifdef CONFIG_NOTIFY_SEC_REBOOT
    reboot_notify(E_REBOOT_DOM, E_AP1_DOM, HALT_REASON_SW_RESET);
    do_notify_reboot_to_mp(WDT5____ovflow_int);
#else
    boot_ap1();
#endif
    return 0;
}

int32_t cmd_boot_ap2_wrapper(int32_t argc, char *argv[])
{
#ifdef CONFIG_NOTIFY_SEC_REBOOT
    reboot_notify(E_REBOOT_DOM, E_AP2_DOM, HALT_REASON_SW_RESET);
    do_notify_reboot_to_mp(WDT6____ovflow_int);
#else
    boot_ap2();
#endif
    return 0;
}

CLI_CMD("ap1", "boot ap1", cmd_boot_ap1_wrapper);
CLI_CMD("ap2", "boot ap2", cmd_boot_ap2_wrapper);
CLI_CMD("dom_reboot_en", "domain reboot enable", cmd_dom_reboot_en);
CLI_CMD("reboot", "reboot [cmd]; reboot system with some commands"
        , reboot_cmd);
/* end of file app_boot_ss.c */
