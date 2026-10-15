#ifndef _SEC_RTCD_H
#define _SEC_RTCD_H
#include <stdlib.h>

#include "driver/rtc.h"

#define ERR_SIG_PADDR_UNC 0
#define ERR_SIG_PUSER_UNC 1
#define ERR_SIG_PCTL0_UNC 2
#define ERR_SIG_PCTL1_UNC 3
#define ERR_SIG_PWDATA_UNC 4
#define ERR_SIG_PWDATA_COR 5
#define ERR_SIG_PWDATA_FATAL 6
#define ERR_SIG_SELFTEST_MODE 8
#define ERR_SIG_PARITY_EJ 9

#define WKUP_ENABLE 0
#define WKUP_IRQ 1
#define WKUP_REQ 2
#define WKUP_CLEAR 4
#define WKUP_OV_CLR 5
#define WKUP_DISABLE_CLR 6

rtc_dev_t *sec_rtc_probe(rtc_dev_t *dev, const rtc_config_t *cfg);
#endif