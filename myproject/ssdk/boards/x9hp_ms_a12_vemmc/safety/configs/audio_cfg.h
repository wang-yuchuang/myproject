#ifndef __AUDIO_CFG_H__
#define __AUDIO_CFG_H__
#include <cmsis_os2.h>

#ifdef CONFIG_AUDIO_MANAGER
void board_audio_init(void);
void wait_audio_init_done(void);
#endif

#endif /* __AUDIO_CFG_H__ */
