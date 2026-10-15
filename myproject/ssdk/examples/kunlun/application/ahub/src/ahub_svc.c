#include <CLI.h>
#include <debug.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ahub.h"

#include <res_loader.h>
#include <malloc.h>
#include <driver/wav_header.h>
#include "audio_res.h"
#include "param.h"
#include "ahub_dev.h"

/* here define mix policy ep, */
static ep_mix_params_t playback_mix_policy0[] = {
    {AHUB_ENDPOINT_ID0, 0, 2, MIX_NORMAL}, // same id duplicate to different buffer
    {AHUB_ENDPOINT_ID0, 1, 1, MIX_NORMAL}, // same id duplicate to different buffer

};

static ep_mix_params_t playback_mix_policy1[] = {
    {AHUB_ENDPOINT_ID1, 0, 2, MIX_NORMAL}, ///< QNX or Linux sound
    {AHUB_ENDPOINT_ID2, 0, 2, MIX_NORMAL}, ///< media bus + system bus
    {AHUB_ENDPOINT_ID3, 2, 2, MIX_NORMAL}, ///< media bus + system bus
    {AHUB_ENDPOINT_ID4, 0, 2, MIX_NORMAL}, ///< phone ring bus
    {AHUB_ENDPOINT_ID5, 0, 2, EXC_MODE}, ///< phone bus
    {AHUB_ENDPOINT_ID6, 0, 2, MAY_DUCK_MODE}, ///< Nav bus MAY_DUCK_MODE or EXC_MODE
    {AHUB_ENDPOINT_ID7, 0, 2, MIX_NORMAL}, ///< MP local player.
#ifdef AHUB_TEST_VER
    {AHUB_ENDPOINT_ID11,0, 2, 0}, ///< Loopback endpoint
#endif
};

static ep_fetch_params_t capture_fetch_policy0[] = {
    {AHUB_ENDPOINT_ID8, 0, 2, 0}, ///< Android Slot 1,2
    {AHUB_ENDPOINT_ID9, 2, 2, 0}, ///< Android Slot 3,4
#ifdef AHUB_TEST_VER
    {AHUB_ENDPOINT_ID10, 0, 2, 0},///< Linux Slot 1,2 Don't allow linux record from Main MIC.
    {AHUB_ENDPOINT_ID11,0, 2, 0}, ///< Loopback endpoint
#endif
};

static int load_pcm_from_path(const char *path, void **buf, int *size)
{
    int ret = 0;
    *size = ROUNDUP(res_size(path), 32);
    *buf = memalign(32, *size);

    if (!(*buf)) {
        ssdk_printf(SSDK_CRIT,"Create audio pcm fail.\n");
        return -1;
    }

    ret = res_load(path, *buf, *size, 0);
    if (ret < 0) {
        ssdk_printf(SSDK_CRIT,"can't load audio res!\n");
        free(*buf);
        buf = NULL;
        return ret;
    }

    ssdk_printf(SSDK_INFO,"size 0x%x %p.\n", *size, *buf);
    return ret;
}

static int load_wav(const char *path, void **buf, int *size,
                              wav_header_t *wav_info, data_header_t *data_info)
{
    int ret = 0;
    size_t wav_header_size = 0;
    char *wav_info_buf = memalign(CONFIG_ARCH_CACHE_LINE, ROUNDUP(sizeof(wav_header_t) + sizeof(data_header_t),
                              CONFIG_ARCH_CACHE_LINE));
    if (!wav_info_buf) {
        ssdk_printf(SSDK_CRIT,"Create audio wav header fail.\n");
        return -1;
    }

    /* load wav header*/
    ret = res_load(path, wav_info_buf,
                   ROUNDUP(sizeof(wav_header_t), CONFIG_ARCH_CACHE_LINE), 0);
    if (ret < 0) {
        free(wav_info_buf);
        ssdk_printf(SSDK_CRIT,"can't load audio head!\n");
        return -1;
    }
    /* memcpy wav header to wav_info*/
    memcpy(wav_info, wav_info_buf, sizeof(wav_header_t));
    memcpy(data_info, wav_info_buf + wav_info->fmt_sz + 20,
           sizeof(data_header_t));

    free(wav_info_buf);

    wav_header_size = wav_info->fmt_sz + 20 + sizeof(data_header_t);

    /* load res size*/
    *size = ROUNDUP(res_size(path), CONFIG_ARCH_CACHE_LINE);
    if (*size <= sizeof(wav_header_t)) {
        ssdk_printf(SSDK_CRIT,"Create audio wav fail. \n");
        return -1;
    }
    //*size = *size - sizeof(wav_header_t);
    *buf = memalign(CONFIG_ARCH_CACHE_LINE, *size);

    if (!(*buf)) {
        ssdk_printf(SSDK_CRIT,"Create audio wav fail.\n");
        return -1;
    }

    ret = res_load(path, *buf, *size, wav_header_size);
    if (ret < 0) {
        ssdk_printf(SSDK_CRIT,"can't load audio res!\n");
        free(*buf);
        buf = NULL;
        return ret;
    }

    ssdk_printf(SSDK_INFO,"size 0x%x() %p sz(%d) sr(%d) ch(%d).\n", *size, *buf,
           data_info->data_sz, wav_info->sample_rate, wav_info->num_channels);
    return ret;
}

static void free_wav(void **buf)
{
    if (*buf)
        free(*buf);
    *buf = NULL;
}

//----------------------------service
//init---------------------------------------------------

int audio_hub_service_init(void)
{
    int ret = AHUB_RET_ERR_UNKNOWN;
    stream_params_t params;

    /* 1. init audio hub */
    ret = ahub_init();

    if (ret != AHUB_RET_OK) {
        ssdk_printf(SSDK_CRIT, "Error in %s: ahub_init failed!\n", __func__);
        return ret;
    }
    /* here init audio hub hw device */

    /* 2. here config and register audio hub stream */
    {
        /* 2.1 setup playback stream AHUB_PLAYBACK_STREAM_ID0's parameter*/
        params.channels = p_ahub_snd_params[SAF_SND_DEV_TX_PARAM]->channels; // SND_CHANNEL_NUM_STEREO;
        // params.channel_mask = 0x3;
        params.sample_rate = p_ahub_snd_params[SAF_SND_DEV_TX_PARAM]->sample_rate; // SND_SR_48000;
        params.sample_bytes =
            AHUB_SAMPLE_BYTES; // should equal snd_params[0].sample_bytes / 8
        /* 2.2 setup playback stream input mix matrix*/
        params.ep_policy_count = ARRAY_SIZE(playback_mix_policy0);
        params.mix_policy = playback_mix_policy0;
        /* 2.3 setup playback stream parameter and register write_func_ptr */
        ret = ahub_create_playback_stream(AHUB_PLAYBACK_STREAM_ID0, params);
        if (ret != AHUB_RET_OK) {
            ssdk_printf(SSDK_CRIT,
                        "Error in %s: ahub_create_playback_stream failed!\n",
                        __func__);
            return ret;
        }
    }
    /* 3. here config and register audio hub stream */
    {
        /* 3.1 setup playback stream AHUB_PLAYBACK_STREAM_ID0's parameter*/
        params.channels = p_ahub_snd_params[AP_MAIN_DEV_TX_PARAM]->channels; // SND_CHANNEL_NUM_STEREO;
        // params.channel_mask = 0x3;
        params.sample_rate = p_ahub_snd_params[AP_MAIN_DEV_TX_PARAM]->sample_rate; // SND_SR_48000;
        params.sample_bytes =
            AHUB_SAMPLE_BYTES; // should equal snd_params[0].sample_bytes / 8
        /* 3.2 setup playback stream input mix matrix*/
        params.ep_policy_count = ARRAY_SIZE(playback_mix_policy1);
        params.mix_policy = playback_mix_policy1;
        /* 3.3 setup playback stream parameter and register write_func_ptr */
        ret = ahub_create_playback_stream(AHUB_PLAYBACK_STREAM_ID1, params);
        if (ret != AHUB_RET_OK) {
            ssdk_printf(SSDK_CRIT,
                        "Error in %s: ahub_create_playback_stream failed!\n",
                        __func__);
            return ret;
        }
    }
    /* 4. here config and register audio hub capture stream */
    {
        /* 4.1 setup capture stream AHUB_CAPTURE_STREAM_ID0's parameter*/
        params.channels = p_ahub_snd_params[AP_MAIN_DEV_RX_PARAM]->channels; // SND_CHANNEL_NUM_STEREO;
        // params.channel_mask = 0x3;
        params.sample_rate = p_ahub_snd_params[AP_MAIN_DEV_RX_PARAM]->sample_rate; // SND_SR_48000;
        params.sample_bytes =
            AHUB_SAMPLE_BYTES; // should equal snd_params[0].sample_bytes / 8

        params.ep_policy_count = ARRAY_SIZE(capture_fetch_policy0);
        params.fetch_policy = capture_fetch_policy0;

        ret = ahub_create_capture_stream(AHUB_CAPTURE_STREAM_ID0, params);
        if (ret != AHUB_RET_OK) {
            ssdk_printf(SSDK_CRIT,
                        "Error in %s: ahub_create_playback_stream failed!\n",
                        __func__);
            return ret;
        }
    }
    return AHUB_RET_OK;
}

//----------------------------service
//deinit---------------------------------------------------
static void audio_hub_service_deinit(void)
{
    ahub_remove_playback_stream(AHUB_PLAYBACK_STREAM_ID0);
    ahub_remove_playback_stream(AHUB_PLAYBACK_STREAM_ID1);
    ahub_remove_capture_stream(AHUB_CAPTURE_STREAM_ID0);
    return;
}

static osThreadId_t ep0_write_thread;
static osThreadAttr_t ahub_ep0_wr_task_attr = {
    .name = "ahub wr to ep0",
    .stack_size = 4096,
    .priority = osPriorityRealtime,
};

static osThreadId_t ep1_write_thread;
static osThreadAttr_t ahub_ep1_wr_task_attr = {
    .name = "ahub wr to ep1",
    .stack_size = 4096,
    .priority = osPriorityRealtime,
};

/*  write data to input endpoint */
static void stream0_write_ep(void *para)
{
    uint32_t ep_id = (uint32_t)para;
    void *audio_pcm_ptr = NULL;
    int audio_pcm_size = 0;
    int data_len = 0;
    int ret = AHUB_RET_ERR_UNKNOWN;
    endpoint_params_t ep_params;
    wav_header_t wav_info;
    data_header_t data_info;

    ret = load_wav(FAST_AUDIO_WAV_PATH, &audio_pcm_ptr, &audio_pcm_size,
                   &wav_info, &data_info);

    if (ret < 0) {
        osThreadExit();
        ssdk_printf(SSDK_CRIT, "Error in %s: load wav failed!\n", __func__);
        return;
    }

    ep_params.endpoint_id = ep_id;
    ep_params.channels =
        SND_CHANNEL_NUM_STEREO; // ep input should fixed to stereo
    ep_params.sample_rate = wav_info.sample_rate;
    ep_params.sample_bytes = AHUB_SAMPLE_BYTES; // 16bits

    ret = ahub_register_ep_to_playback_stream(AHUB_PLAYBACK_STREAM_ID0,
                                              ep_params);
    if (ret != AHUB_RET_OK) {
        osThreadExit();
        ssdk_printf(
            SSDK_CRIT,
            "Error in %s: ahub_register_ep_to_playback_stream failed!\n",
            __func__);
    }
    int offset = 0;
    data_len = wav_info.sample_rate * AHUB_PLAYBACK_PERIOD_MS * ep_params.channels *
               ep_params.sample_bytes / 1000;
    do {
        if (ahub_write_to_ep(ep_id, (audio_pcm_ptr + offset), data_len) !=
            AHUB_RET_OK) {
            break;
        }
        offset += data_len;
        // printf("offset (%d) ep(%d).\r\n", offset,ep_id);

    } while (offset < data_info.data_sz);

    ssdk_printf(SSDK_INFO,"audio_hub_write_ep0 quit with offset %d %d.\n", offset, data_len);
    ahub_unregister_ep_from_playback_stream(AHUB_PLAYBACK_STREAM_ID0, ep_id);
    free_wav(&audio_pcm_ptr);

    osThreadExit();
}

static osThreadId_t ep2_write_thread;
static osThreadAttr_t ahub_ep2_wr_task_attr = {
    .name = "ahub wr to ep2",
    .stack_size = 4096,
    .priority = osPriorityRealtime,
};

static osThreadId_t ep3_write_thread;
static osThreadAttr_t ahub_ep3_wr_task_attr = {
    .name = "ahub wr to ep3",
    .stack_size = 4096,
    .priority = osPriorityRealtime,
};

static osThreadId_t ep6_write_thread;
static osThreadAttr_t ahub_ep6_wr_task_attr = {
    .name = "ahub wr to ep6",
    .stack_size = 4096,
    .priority = osPriorityRealtime,
};

#define AU_STEREO_441K_WAV_PATH "early_app/BootAnimation/cov_stereo_44.1k_16bits.wav"
#define AU_STEREO_16K_WAV_PATH "early_app/BootAnimation/cov_stereo_16k_16bits.wav"
#define AU_MONO_8K_WAV_PATH "early_app/BootAnimation/cov_mono_8k_16bits.wav"

/*  write data to input endpoint */
static void stream1_write_ep(void *para)
{
    uint32_t ep_id = (uint32_t)para;
    void *audio_pcm_ptr = NULL;
    int audio_pcm_size = 0;
    int data_len = 0;
    int ret = AHUB_RET_ERR_UNKNOWN;
    endpoint_params_t ep_params;
    wav_header_t wav_info;
    data_header_t data_info;

    ret = load_wav(FAST_AUDIO_WAV_PATH, &audio_pcm_ptr,
                   &audio_pcm_size, &wav_info, &data_info);

    if (ret < 0) {
        osThreadExit();
        ssdk_printf(SSDK_CRIT, "Error in %s: load wav failed!\n", __func__);
        return;
    }

    ep_params.endpoint_id = ep_id;
    ep_params.channels =
        SND_CHANNEL_NUM_STEREO; // ep input should fixed to stereo
    ep_params.sample_rate = SND_SR_48000;
    ep_params.sample_bytes = AHUB_SAMPLE_BYTES; // 16bits

    ret = ahub_register_ep_to_playback_stream(AHUB_PLAYBACK_STREAM_ID1,
                                              ep_params);

    if (ret != AHUB_RET_OK) {
        osThreadExit();
        ssdk_printf(
            SSDK_CRIT,
            "Error in %s: ahub_register_ep_to_playback_stream failed!\n",
            __func__);
    }

    int offset = 0;
    data_len = ep_params.sample_rate * AHUB_PLAYBACK_PERIOD_MS * ep_params.channels *
               ep_params.sample_bytes / 1000;
    do {
        if (ahub_write_to_ep(ep_id, (audio_pcm_ptr + offset), data_len) !=
            AHUB_RET_OK) {
            break;
        }
        offset += data_len;

    } while (offset < data_info.data_sz);
    ssdk_printf(SSDK_INFO,"audio_hub_write_ep0 quit with offset %d %d.\n", offset, data_len);
    ahub_unregister_ep_from_playback_stream(AHUB_PLAYBACK_STREAM_ID1, ep_id);
    free_wav(&audio_pcm_ptr);

    osThreadExit();
}

static osThreadId_t ep11_read_thread;
static osThreadAttr_t ahub_ep11_rd_task_attr = {
    .name = "ahub rd to ep7",
    .stack_size = 4096,
    .priority = osPriorityRealtime,
};

/*  rd data to input endpoint */
static void stream0_read_ep(void *para)
{
    uint32_t ep_id = (uint32_t)para;
    // void * rd_buff = NULL;

    int data_len = 0;
    int ret = AHUB_RET_ERR_UNKNOWN;
    endpoint_params_t ep_params;

    if (ret < 0) {
        osThreadExit();
        ssdk_printf(SSDK_CRIT, "Error in %s: load wav failed!\n", __func__);
        return;
    }

    ep_params.endpoint_id = ep_id;
    ep_params.channels =
        SND_CHANNEL_NUM_STEREO; // ep input should fixed to stereo
    ep_params.sample_rate = SND_SR_48000;
    ep_params.sample_bytes = AHUB_SAMPLE_BYTES; // 16bits

    ret =
        ahub_register_ep_to_capture_stream(AHUB_CAPTURE_STREAM_ID0, ep_params);
    if (ret != AHUB_RET_OK) {
        osThreadExit();
        ssdk_printf(SSDK_CRIT,
                    "Error in %s:ahub_register_ep_to_capture_stream failed!\n",
                    __func__);
    }
    //-------------loopback
    ep_params.endpoint_id = ret = ahub_register_ep_to_playback_stream(
        AHUB_PLAYBACK_STREAM_ID1, ep_params);
    if (ret != AHUB_RET_OK) {
        osThreadExit();
        ssdk_printf(
            SSDK_CRIT,
            "Error in %s: ahub_register_ep_to_playback_stream failed!\n",
            __func__);
    }

    int cnt = 0;
    data_len = SND_SR_48000 * AHUB_PLAYBACK_PERIOD_MS * ep_params.channels *
               ep_params.sample_bytes / 1000;
    // rd_buff = memalign(32,data_len);
    do {

        osDelay(10);
        cnt++;

    } while (cnt < 1000);
    printf("stream0_read_ep quit with offset %d %d.\n", cnt, data_len);
    ahub_unregister_ep_from_capture_stream(AHUB_CAPTURE_STREAM_ID0, ep_id);
    ahub_unregister_ep_from_playback_stream(AHUB_PLAYBACK_STREAM_ID1, ep_id);
    // ahub_dump_data(data_len, AHUB_DUMP_BUFF_SIZE);
    osThreadExit();
}

void ahub_ani_audio_task(void *para)
{
    stream1_write_ep((void *)AHUB_ENDPOINT_ID7);
}

static int ahub_test(int argc, char *argv[])
{
    int epid = AHUB_ENDPOINT_ID0;
    int volin = EP_0DB_GAIN;
    int volout = EP_0DB_GAIN;
    if (!strcmp(argv[0], "init")) {

        ssdk_printf(SSDK_CRIT,"audio hub init.\n");
        audio_hub_service_init();
        return 0;

    } else if (!strcmp(argv[0], "ep0")) {
        ssdk_printf(SSDK_CRIT,"audio hub write to ep0.\n");

        ep0_write_thread =
            osThreadNew(stream0_write_ep, (void *)AHUB_ENDPOINT_ID0,
                        &ahub_ep0_wr_task_attr);
        return 0;

    } else if (!strcmp(argv[0], "ep1")) {
        ssdk_printf(SSDK_CRIT,"audio hub write to ep1.\n");
        ep1_write_thread =
            osThreadNew(stream0_write_ep, (void *)AHUB_ENDPOINT_ID1,
                        &ahub_ep1_wr_task_attr);
        return 0;

    } else if (!strcmp(argv[0], "ep2")) {
        printf("audio hub write to ep2.\n");
        ep2_write_thread =
            osThreadNew(stream1_write_ep, (void *)AHUB_ENDPOINT_ID2,
                        &ahub_ep2_wr_task_attr);
        return 0;

    } else if (!strcmp(argv[0], "ep3")) {
        ssdk_printf(SSDK_CRIT,"audio hub write to ep3.\n");
        ep3_write_thread =
            osThreadNew(stream1_write_ep, (void *)AHUB_ENDPOINT_ID3,
                        &ahub_ep3_wr_task_attr);
        return 0;

    } else if (!strcmp(argv[0], "ep6")) {
        ssdk_printf(SSDK_CRIT,"audio hub write to ep6 for nav.\n");
        ep6_write_thread =
                    osThreadNew(stream1_write_ep, (void *)AHUB_ENDPOINT_ID6,
                        &ahub_ep6_wr_task_attr);

        return 0;

    } else if (!strcmp(argv[0], "ep7")) {
        ssdk_printf(SSDK_CRIT,"audio hub write to ep6 for nav.\n");
        ep6_write_thread =
                    osThreadNew(stream1_write_ep, (void *)AHUB_ENDPOINT_ID7,
                        &ahub_ep6_wr_task_attr);

        return 0;

    } else if (!strcmp(argv[0], "ep11")) {
        ssdk_printf(SSDK_CRIT,"audio hub read/write to ep11.\n");
        ep11_read_thread = osThreadNew(
            stream0_read_ep, (void *)AHUB_ENDPOINT_ID11, &ahub_ep11_rd_task_attr);
        return 0;

    }else if (!strcmp(argv[0], "setinvol")) {
        /*setinvol epid invol outvol*/
        if(argc != 3){
            ssdk_printf(SSDK_CRIT,"setinvol numb should be 3.\n");
            return 0;
        }

        epid = atoi(argv[1]);
        volin = atoi(argv[2]);

        ahub_set_ep_input_gain(epid,volin);
        ssdk_printf(SSDK_CRIT,"setinvol ep:%d vol %d.\n",epid,volin);

        return 0;
    } else if (!strcmp(argv[0], "setoutvol")) {
        /*setoutvol epid invol outvol*/
        if(argc != 3){
            ssdk_printf(SSDK_CRIT,"setoutvol numb should be 3.\n");
            return 0;
        }
        epid = atoi(argv[1]);
        volout = atoi(argv[2]);

        ahub_set_ep_output_gain(epid,volout);
        ssdk_printf(SSDK_CRIT,"setoutvol ep:%d vol %d.\n",epid,volout);

        return 0;
    }else if (!strcmp(argv[0], "deinit")) {

        ssdk_printf(SSDK_CRIT,"audio hub deinit.\n");
        audio_hub_service_deinit();
        return 0;

    } else if (!strcmp(argv[0], "tst")) {

        ssdk_printf(SSDK_CRIT,"audio hub test.\n");
        return 0;
    }
    return 0;
}

CLI_CMD("ahub", "ahub init/wr/deinit", ahub_test);
