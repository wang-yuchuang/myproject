#include <CLI.h>
#include <am_api.h>
#include <cmsis_os2.h>
#include <malloc.h>
#include <res_loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_res.h"
#include "driver/snd_driver.h"
#include "param.h"

#define TEST_PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K                                \
    "PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K"
#define TEST_PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K                               \
    "PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K"
#define TEST_PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K                              \
    "PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K"
#define TEST_PATH_BT_PLAYBACK_TO_MAIN_SPK_16K "PATH_BT_PLAYBACK_TO_MAIN_SPK_16K"
#define TEST_PATH_BT_CAPTURE_FROM_MAIN_MIC_16K                                 \
    "PATH_BT_CAPTURE_FROM_MAIN_MIC_16K"
#define TEST_PATH_BT_PLAYBACK_TO_MAIN_SPK_8K "PATH_BT_PLAYBACK_TO_MAIN_SPK_8K"
#define TEST_PATH_BT_CAPTURE_FROM_MAIN_MIC_8K "PATH_BT_CAPTURE_FROM_MAIN_MIC_8K"
#define TEST_PATH_CLUSTER_PLAYBACK_TO_MAIN_SPK_48K                             \
    "PATH_CLUSTER_PLAYBACK_TO_MAIN_SPK_48K"
#define TEST_PATH_CLUSTER_CAPTURE_FROM_MAIN_MIC_48K                            \
    "PATH_CLUSTER_CAPTURE_FROM_MAIN_MIC_48K"
#define TEST_PATH_FM_PLAYBACK_TO_MAIN_SPK_48K "PATH_FM_PLAYBACK_TO_MAIN_SPK_48K"
#define TEST_PATH_TBOX_PLAYBACK_TO_MAIN_SPK_48K                                \
    "PATH_TBOX_PLAYBACK_TO_MAIN_SPK_48K"
#define TEST_PATH_NAV_PLAYBACK_TO_MAIN_SPK_48K                                 \
    "PATH_NAV_PLAYBACK_TO_MAIN_SPK_48K"
#define TEST_PATH_PLAYBACK_TO_REARSEAT_L_48K "PATH_PLAYBACK_TO_REARSEAT_L_48K"
#define TEST_PATH_PLAYBACK_TO_REARSEAT_R_48K "PATH_PLAYBACK_TO_REARSEAT_R_48K"
#define TEST_PATH_CAPTURE_FROM_REARSEAT_L_48K "PATH_CAPTURE_FROM_REARSEAT_L_48K"
#define TEST_PATH_CAPTURE_FROM_REARSEAT_R_48K "PATH_CAPTURE_FROM_REARSEAT_R_48K"
#define TEST_PATH_PHONE_PLAYBACK_TO_MAIN_SPK_48K                               \
    "PATH_PHONE_PLAYBACK_TO_MAIN_SPK_48K"
#define TEST_PATH_PHONE_CAPTURE_FROM_MAIN_MIC_48K                              \
    "PATH_PHONE_CAPTURE_FROM_MAIN_MIC_48K"

#ifdef CONFIG_CDNS_I2S_SC

#define SND_XFER_COMPLETE (0x1u)
#define CHNNELS_COUNT 2
#define DMA_PERIOD_FRAME_COUNT 1024
#define DMA_PERIOD_SIZE DMA_PERIOD_FRAME_COUNT *CHNNELS_COUNT * 2
#define DMA_PERIOD_COUNT 2
#define DMA_BUF_TOTAL_SIZE DMA_PERIOD_SIZE *DMA_PERIOD_COUNT

typedef struct snd_player {
    snd_dev_t *dev;
    osEventFlagsId_t event;
    osThreadId_t thread;
    bool running;
    uint8_t *pcm_data;
    int32_t pcm_size;
    int32_t pcm_ptr;
    snd_params_t *params;
} snd_player_t;

extern snd_dev_t *i2s_sc1_dev;
static snd_player_t app_player;

static snd_params_t pcm_param = {
    .fmt = SND_DAIFMT_STANDARD_PHILLIPS | SND_DAIFMT_CODEC_MASTER,
    .channels = SND_CHANNEL_NUM_STEREO,
    .sample_width = SND_SAMPLE_WIDTH_16BITS,
    .sample_rate = SND_SR_48000,
    .period_size = DMA_PERIOD_SIZE,
    .period_count = DMA_PERIOD_COUNT,
};
#endif

static void *audio_pcm_ptr = NULL;
static int audio_pcm_size = 0;

static int load_pcm_from_path(const char *path, void **buf, int *size)
{
    int ret = 0;
    *size = ROUNDUP(res_size(path), 32);
    *buf = memalign(32, *size);

    if (!(*buf)) {
        printf("Create audio pcm fail.\n");
        return -1;
    }

    ret = res_load(path, *buf, *size, 0);
    if (ret < 0) {
        printf("can't load audio res!\n");
        free(buf);
        buf = NULL;
        return ret;
    }

    printf("size 0x%x %p.\n", *size, *buf);
    return ret;
}

#ifdef CONFIG_CDNS_I2S_SC
static void player_callback(void *context, snd_state_t state)
{
    snd_player_t *player = (snd_player_t *)context;

    if (player->event) {
        osEventFlagsSet(player->event, SND_XFER_COMPLETE);
    }
}

static void player_thread(void *pcArg)
{
    snd_player_t *player = (snd_player_t *)pcArg;
    snd_params_t *params = player->params;
    snd_dev_t *dev = player->dev;
    int len;
    int32_t temp = 0;

    while (player->running) {
        osEventFlagsWait(player->event, SND_XFER_COMPLETE, osFlagsWaitAll,
                         osWaitForever);

        temp = player->pcm_size - player->pcm_ptr;

        if (temp > params->period_size) {
            len = snd_write(dev, player->pcm_data + player->pcm_ptr,
                            params->period_size);

            player->pcm_ptr += len;
        } else {
            if (temp == 0) { /* data transfer over */
                player->running = false;
                player->pcm_ptr = 0;
            } else {
                len = snd_write(dev, player->pcm_data + player->pcm_ptr, temp);
                player->pcm_ptr += len;
            }
        }
    }

    snd_stop(dev, SND_STREAM_PLAYBACK);
    snd_close(dev, SND_STREAM_PLAYBACK);

    osEventFlagsDelete(player->event);
    osThreadExit();
    player->event = NULL;
    player->thread = NULL;
}

static void player_init(snd_player_t *player)
{
    osThreadAttr_t attr = {
        .name = "player_thread",
        .stack_size = 4096,
        .priority = osPriorityNormal1,
    };

    player->running = true;
    player->event = osEventFlagsNew(NULL);
    player->pcm_ptr = 0;
    player->thread = osThreadNew(player_thread, player, &attr);
}

static void audio_play(void *buf, int size)
{
    snd_player_t *player = &app_player;
    snd_dev_t *dev = NULL;
    bool ret;
    int len;

    dev = player->dev = i2s_sc1_dev;
    player->pcm_data = buf;
    player->pcm_size = size;
    player->params = &pcm_param;
    player_init(player);

    ret = snd_open(dev, player->params, player_callback, (void *)player,
                   SND_STREAM_PLAYBACK);
    if (!ret) {
        printf("snd_open err\r\n");
        return;
    }

    len =
        snd_write(dev, player->pcm_data + player->pcm_ptr, DMA_BUF_TOTAL_SIZE);
    if (len < 0) {
        printf("snd_write err\r\n");
        return;
    }

    player->pcm_ptr += len;

    ret = snd_start(dev, SND_STREAM_PLAYBACK);
    if (!ret) {
        printf("snd_start err\r\n");
        return;
    }
}
#endif

static int audiomanager_test_main(int argc, char *argv[])
{
    int vol = 80;
    int mute = 0;

    if (!strcmp(argv[0], "help")) {
        printf("am start/stop/setvol/mute/switch "
               "hifip/hific/safetyp/btp/btc/clusterp/fmp/tboxp \r\n");
        printf("am play\r\n");
    } else if (!strcmp(argv[0], "play")) {
        printf("am play...\r\n");
        audio_play(audio_pcm_ptr, audio_pcm_size);
    } else if (!strcmp(argv[0], "init")) {
        load_pcm_from_path(FAST_AUDIO_PATH, &audio_pcm_ptr, &audio_pcm_size);
        printf("am init pcm (%p)-0x%x,\r\n", audio_pcm_ptr, audio_pcm_size);
    } else if (!strcmp(argv[0], "start")) {
        printf("am start ");
        if (!strcmp(argv[1], "hifip")) {
            printf("PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_start_path(TEST_PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "hific")) {
            printf("PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K \r\n");
            am_start_path(TEST_PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K, vol);
        } else if (!strcmp(argv[1], "safetyp")) {
            printf("PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_start_path(TEST_PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "btp")) {
            printf("PATH_BT_PLAYBACK_TO_MAIN_SPK_16K \r\n");
            am_start_path(TEST_PATH_BT_PLAYBACK_TO_MAIN_SPK_16K, vol);
        } else if (!strcmp(argv[1], "btc")) {
            printf("PATH_BT_CAPTURE_FROM_MAIN_MIC_16K \r\n");
            am_start_path(TEST_PATH_BT_CAPTURE_FROM_MAIN_MIC_16K, vol);
        } else if (!strcmp(argv[1], "clusterp")) {
            printf("PATH_CLUSTER_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_start_path(TEST_PATH_CLUSTER_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "fmp")) {
            printf("PATH_FM_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_start_path(TEST_PATH_FM_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "tboxp")) {
            printf("PATH_TBOX_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_start_path(TEST_PATH_TBOX_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "navp")) {
            printf("PATH_NAV_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_start_path(TEST_PATH_NAV_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "phonep")) {
            printf("PATH_PHONE_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_start_path(TEST_PATH_PHONE_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "phonec")) {
            printf("PATH_PHONE_CAPTURE_FROM_MAIN_MIC_48K \r\n");
            am_start_path(TEST_PATH_PHONE_CAPTURE_FROM_MAIN_MIC_48K, vol);
        }
    } else if (!strcmp(argv[0], "stop")) {
        printf("am stop ");
        if (!strcmp(argv[1], "hifip")) {
            printf("PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_stop_path(TEST_PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K);
        } else if (!strcmp(argv[1], "hific")) {
            printf("PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K \r\n");
            am_stop_path(TEST_PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K);
        } else if (!strcmp(argv[1], "safetyp")) {
            printf("PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_stop_path(TEST_PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K);
        } else if (!strcmp(argv[1], "btp")) {
            printf("PATH_BT_PLAYBACK_TO_MAIN_SPK_16K \r\n");
            am_stop_path(TEST_PATH_BT_PLAYBACK_TO_MAIN_SPK_16K);
        } else if (!strcmp(argv[1], "btc")) {
            printf("PATH_BT_CAPTURE_FROM_MAIN_MIC_16K \r\n");
            am_stop_path(TEST_PATH_BT_CAPTURE_FROM_MAIN_MIC_16K);
        } else if (!strcmp(argv[1], "clusterp")) {
            printf("PATH_CLUSTER_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_stop_path(TEST_PATH_CLUSTER_PLAYBACK_TO_MAIN_SPK_48K);
        } else if (!strcmp(argv[1], "fmp")) {
            printf("PATH_FM_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_stop_path(TEST_PATH_FM_PLAYBACK_TO_MAIN_SPK_48K);
        } else if (!strcmp(argv[1], "tboxp")) {
            printf("PATH_TBOX_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_stop_path(TEST_PATH_TBOX_PLAYBACK_TO_MAIN_SPK_48K);
        } else if (!strcmp(argv[1], "navp")) {
            printf("PATH_NAV_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_stop_path(TEST_PATH_NAV_PLAYBACK_TO_MAIN_SPK_48K);
        } else if (!strcmp(argv[1], "phonep")) {
            printf("PATH_PHONE_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_stop_path(TEST_PATH_PHONE_PLAYBACK_TO_MAIN_SPK_48K);
        } else if (!strcmp(argv[1], "phonec")) {
            printf("PATH_PHONE_CAPTURE_FROM_MAIN_MIC_48K \r\n");
            am_stop_path(TEST_PATH_PHONE_CAPTURE_FROM_MAIN_MIC_48K);
        }
    } else if (!strcmp(argv[0], "setvol")) {
        printf("am setvol ");
        vol = atoi(argv[2]);
        if (!strcmp(argv[1], "hifip")) {
            printf("PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_set_path_vol(TEST_PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "safetyp")) {
            printf("PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_set_path_vol(TEST_PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "btp")) {
            printf("PATH_BT_PLAYBACK_TO_MAIN_SPK_16K \r\n");
            am_set_path_vol(TEST_PATH_BT_PLAYBACK_TO_MAIN_SPK_16K, vol);
        } else if (!strcmp(argv[1], "clusterp")) {
            printf("PATH_CLUSTER_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_set_path_vol(TEST_PATH_CLUSTER_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "fmp")) {
            printf("PATH_FM_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_set_path_vol(TEST_PATH_FM_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "tboxp")) {
            printf("PATH_TBOX_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_set_path_vol(TEST_PATH_TBOX_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "navp")) {
            printf("PATH_NAV_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_set_path_vol(TEST_PATH_NAV_PLAYBACK_TO_MAIN_SPK_48K, vol);
        } else if (!strcmp(argv[1], "phonep")) {
            printf("PATH_PHONE_PLAYBACK_TO_MAIN_SPK_48K \r\n");
            am_set_path_vol(TEST_PATH_PHONE_PLAYBACK_TO_MAIN_SPK_48K, vol);
        }
    } else if (!strcmp(argv[0], "mute")) {
        printf("am mute ");
        mute = atoi(argv[2]);
        if (!strcmp(argv[1], "hific")) {
            printf("PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K \r\n");
            am_mute_path(TEST_PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K, mute);
        } else if (!strcmp(argv[1], "btc")) {
            printf("PATH_BT_CAPTURE_FROM_MAIN_MIC_16K \r\n");
            am_mute_path(TEST_PATH_BT_CAPTURE_FROM_MAIN_MIC_16K, mute);
        } else if (!strcmp(argv[1], "phonec")) {
            printf("PATH_PHONE_CAPTURE_FROM_MAIN_MIC_48K \r\n");
            am_mute_path(TEST_PATH_PHONE_CAPTURE_FROM_MAIN_MIC_48K, mute);
        }
    }

    return 0;
}

CLI_CMD("am",
        "audiomanager\r\n\tam start/stop/setvol/mute/switch pathname",
        audiomanager_test_main);