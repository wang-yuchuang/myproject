#ifndef _STR_H_
#define _STR_H_

#include "image_cfg.h"
enum STR_MASTER {
    STR_AP1,
    STR_AP2
};

#if CONFIG_HAS_AP1
#define STR_VOTE_AP1    (1<<STR_AP1)
#define STR_AP1_TO_SAF  (SAF_AP1_MEMBASE + 0x100000)
#else
#define STR_VOTE_AP1    (0)
#define STR_AP1_TO_SAF  (0)
#endif

#if CONFIG_HAS_AP2
#define STR_VOTE_AP2    (1<<STR_AP2)
#define STR_AP2_TO_SAF  (SAF_AP2_MEMBASE + 0x100000)
#else
#define STR_VOTE_AP2    (0)
#define STR_AP2_TO_SAF  (0)
#endif

#define STR_VOTE_ALL ( STR_VOTE_AP1 | STR_VOTE_AP2 )
#define STR_RTC_FLAG    0xf1890050

#if ENABLE_AUDIO_MANAGER
#define STR_AM_PATHS_SIZE 2 * 1024
#define STR_AM_PATHS_ADDR SAF_SEC_MEMBASE+SAF_SEC_MEMSIZE - STR_AM_PATHS_SIZE
#endif

bool is_str_enter(enum STR_MASTER master);
bool is_str_resume(enum STR_MASTER master);
uint32_t get_str_resume_entry(enum STR_MASTER master);
void clr_str_flag(enum STR_MASTER master);
void set_str_flag_to_rtc(enum STR_MASTER master);
void str_vote(enum STR_MASTER master);
bool check_str_all(void);
void str_clr_vote(void);
void get_str_status_early(void);
void set_str_resume_early(void);
__WEAK void power_config_before_str(void);
#endif
