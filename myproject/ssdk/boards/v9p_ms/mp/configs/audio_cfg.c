#include <stdio.h>
#include <stdlib.h>
#include <CLI.h>
#include <cmsis_os2.h>
#ifdef CONFIG_AUDIO_MANAGER
#include "am_api.h"
#include "au_rpc_svc.h"
#endif
#ifdef CONFIG_AUDIO_HUB_SVC
#include "ahub_dev.h"
#include "ahub_svc.h"
#endif
#if defined(CONFIG_AUDIO_MANAGER) || defined(CONFIG_AUDIO_HUB_SVC)
#include "au_stream_svc.h"
#endif
#include <audio_cfg.h>

osEventFlagsId_t audio_init_flag = NULL;

static void audio_init_thread(void* arg)
{
#ifdef CONFIG_AUDIO_MANAGER
    am_init();
    au_rpc_service_init();
#endif
#ifdef CONFIG_AUDIO_HUB_SVC
    audio_hub_service_init();
#endif
#if defined(CONFIG_AUDIO_MANAGER) || defined(CONFIG_AUDIO_HUB_SVC)
    audio_stream_service_init();
#endif
    osEventFlagsSet(audio_init_flag, 1);
    osThreadExit();
}

void board_audio_init(void)
{
    audio_init_flag = osEventFlagsNew(NULL);
    osEventFlagsClear(audio_init_flag, 0);
    osThreadAttr_t attr = {
        .name = "audio init", .stack_size = 4096, .priority = osPriorityNormal};
    osThreadNew(audio_init_thread, NULL, &attr);
}

void wait_audio_init_done(void)
{
    osEventFlagsWait(audio_init_flag, 1, osFlagsNoClear, osWaitForever);
}