#ifdef CONFIG_AUDIO_HUB_SVC
#include <board.h>
#include <driver/clk.h>
#include <driver/scr.h>
#include <kunlun/snd/i2s/cdns_i2s_sc.h>
#include <ahub_dev.h>
#include <ahub_svc.h>
#include <scr_hw.h>
#include <types.h>

#include "clock_cfg.h"
#include "i2s_sc_cfg.h"


#ifdef CONFIG_CDNS_I2S_SC
extern snd_dev_t *i2s_sc1_dev;
extern snd_dev_t *i2s_sc8_dev;
#endif

#define AHUB_SND_SAMPLE_WIDTH (SND_SAMPLE_WIDTH_16BITS)
#define AHUB_SND_SAMPLE_RATE (SND_SR_48000)
#define AHUB_SND_PERIOD_TIME (20) /* ms - fixed to ahub lib */
#define AHUB_SND_PERIOD_SIZE(channels) (AHUB_SND_PERIOD_TIME * AHUB_SND_SAMPLE_RATE * channels * (AHUB_SND_SAMPLE_WIDTH / 8) / 1000)
#define AHUB_SND_PERIOD_COUNT (2)

static const snd_params_t ahub_snd_params[AHUB_SND_DEV_PARAM_NUMB] = {
    {
        .fmt = SND_DAIFMT_STANDARD_PHILLIPS | SND_DAIFMT_CODEC_MASTER,
        .channels = SND_CHANNEL_NUM_STEREO,
        .sample_width = AHUB_SND_SAMPLE_WIDTH,
        .sample_rate = AHUB_SND_SAMPLE_RATE,
        .period_size = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_STEREO),
        .period_count = AHUB_SND_PERIOD_COUNT,
        .xrun_enable = true,
        .start_threshold = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_STEREO) *
                           1,
        .stop_threshold = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_STEREO) *
                          AHUB_SND_PERIOD_COUNT * 2,
    },
    {
        .fmt = SND_DAIFMT_STANDARD_PHILLIPS | SND_DAIFMT_CODEC_MASTER,
        .channels = SND_CHANNEL_NUM_STEREO,
        .sample_width = AHUB_SND_SAMPLE_WIDTH,
        .sample_rate = AHUB_SND_SAMPLE_RATE,
        .period_size = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_STEREO),
        .period_count = AHUB_SND_PERIOD_COUNT,
        .xrun_enable = true,
        .start_threshold = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_STEREO) *
                           1,
        .stop_threshold = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_STEREO) *
                          AHUB_SND_PERIOD_COUNT * 2,
    },
    {
        .fmt = SND_DAIFMT_DSP_A | SND_DAIFMT_CODEC_MASTER,
        .channels = SND_CHANNEL_NUM_8CHANS,
        .slots = SND_CHANNEL_NUM_8CHANS,
        .tx_slot_mask = 0xff,
        .rx_slot_mask = 0xf,
        .slot_width = SND_SAMPLE_WIDTH_32BITS,
        .sample_width = AHUB_SND_SAMPLE_WIDTH,
        .sample_rate = AHUB_SND_SAMPLE_RATE,
        .period_size = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_8CHANS),
        .period_count = AHUB_SND_PERIOD_COUNT,
        .xrun_enable = true,
        .start_threshold = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_8CHANS) *
                           1,
        .stop_threshold = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_8CHANS) *
                          AHUB_SND_PERIOD_COUNT * 2,
    },
    {
        // rx param
        .fmt = SND_DAIFMT_DSP_A | SND_DAIFMT_CODEC_MASTER,
        .channels = SND_CHANNEL_NUM_4CHANS,
        .slots = SND_CHANNEL_NUM_8CHANS, // channel
        .tx_slot_mask = 0xff,
        .rx_slot_mask = 0x0f,
        .slot_width = SND_SAMPLE_WIDTH_32BITS,
        .sample_width = AHUB_SND_SAMPLE_WIDTH,
        .sample_rate = AHUB_SND_SAMPLE_RATE,
        .period_size = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_4CHANS),
        .period_count = AHUB_SND_PERIOD_COUNT,
        .xrun_enable = true,
        .start_threshold = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_4CHANS) *
                           1,
        .stop_threshold = AHUB_SND_PERIOD_SIZE(SND_CHANNEL_NUM_4CHANS) *
                          AHUB_SND_PERIOD_COUNT * 2,
    },
};

void board_ahub_init(void)
{
#ifdef CONFIG_CDNS_I2S_SC
    ahub_safety_dev_init(i2s_sc1_dev, &ahub_snd_params[SAF_SND_DEV_TX_PARAM],
                         &ahub_snd_params[SAF_SND_DEV_RX_PARAM]);
    ahub_ap_main_dev_init(i2s_sc8_dev, &ahub_snd_params[AP_MAIN_DEV_TX_PARAM],
                          &ahub_snd_params[AP_MAIN_DEV_RX_PARAM]);
#endif
}
#endif
