#ifdef CONFIG_HANDOVER_CFG
#include <debug.h>
#include <param.h>
#include <reg.h>
#include <types.h>
#include <scr_cfg.h>
#include <scr_hw.h>
#include <sdrv_common_reg.h>

#define AP_ACCESS   1
#define SAF_ACCESS  0

/* handover ops only for safety domain */
typedef struct handover_info {
    const char name[32];
    scr_signal_t signal;
    uint32_t access_mode;
} handover_info_t;

static handover_info_t handover_lst[] = {
    {.name = "sec_storage2", .signal = SCR_SAFETY__apbmux2_sec_storage2_select, .access_mode = AP_ACCESS},
    {.name = "rtc2",         .signal = SCR_SAFETY__apbmux2_rtc2_select,         .access_mode = AP_ACCESS},
};

void board_handover_init(void)
{
    unsigned i;

    for (i = 0; i < ARRAY_SIZE(handover_lst); i++) {
        scr_set(&g_scr_safety, &(handover_lst[i].signal), handover_lst[i].access_mode);
    }

    /* notify AP domain */
    sdrv_common_reg_set_value(SDRV_REG_STATUS, SDRV_REG_STATUS_HANDOVER_DONE,
                              SDRV_REG_STATUS_HANDOVER_DONE);
}
#endif /* CONFIG_AUDIO_MANAGER */
