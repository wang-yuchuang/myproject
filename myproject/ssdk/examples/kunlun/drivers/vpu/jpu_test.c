/**
 * @file jpu_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: test jpu implent file
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <strings.h>

#include <arch/cache.h>
#include <cmsis_os2.h>
#include <malloc.h>
#include <CLI.h>
#include <CLI_type.h>
#include <driver/timer.h>
#include <timer_cfg.h>
#include <regs_base.h>
#include <res_loader.h>
#include <sdm_display.h>
#include <disp_data_type.h>

#include <driver/kunlun/vpu/vpu.h>
#include <driver/kunlun/vpu/sdrv_jpu.h>
#include <vpu_cfg.h>
/* =========================== marco define session ======================== */
#define get_systick() timer_get_global_time(g_syscnt_timer)

#define EXTRA_FMT   "vpu_test: "
#define TOSTRING(x) (#x)
#define TEST_VPU_LOG(format, ...) ssdk_printf(SSDK_EMERG, EXTRA_FMT "[tick: %lld]" format, get_systick()/1000, ##__VA_ARGS__)

#define JPU_HAL_TEST_RESOURCE_PATH "early_app/BootAnimation/splash_5760x720_high_quality_3s.mjpeg"
#define JPU_HAL_TEST_ENCODE_YUV_PATH "early_app/BootAnimation/output.yuv"
#define JPU_HAL_TEST_ENCODE_VERIFY_JPEG_PATH "early_app/BootAnimation/output.jpg"
#define ENCODE_WIDTH 800
#define ENCODE_HEIGHT 480
#define FPATH_LEN   128

#define DISPLAY_SCREEN_INDEX INFOTAINMENT
// #define DISPLAY_SCREEN_INDEX ENTERTAINMENT
/* =========================== enumeration define session ================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */

static vpu_dev_t* dev = NULL;
/* =========================== structure define session ==================== */
/* type of subcommand function */
typedef void (*cmd_entry)(void *);
typedef struct subCmd_mgr {
    const char* sub_cmd;
    int32_t par_num;
    cmd_entry entry;
}subCmd_mgr_t;

typedef struct {
    unsigned int size;
    unsigned int offset;
    const char *path;
} jpu_hal_dec_test_data_t;

typedef struct {
    unsigned int input_size;
    unsigned int input_offset;
    void *input_buffer;
    int read_finished;

    // verify
    void *verify_buffer;
    unsigned int verify_size;
    unsigned int verify_offset;

    // output
    void *output_buffer;
    unsigned int output_size;
    unsigned int output_offset;
} jpu_hal_enc_test_data_t;

/* =========================== function implementation session ============= */
void dec_fill_input_buffer(void *private_ptr, int expect_size, void *input_buffer,
                           int *fill_input_size)
{
    int ret = 0;
    jpu_hal_dec_test_data_t *test_data = (jpu_hal_dec_test_data_t *)private_ptr;

    if (test_data->offset >= test_data->size) {
        printf("read resource finished\n");
        *fill_input_size = 0;
        return;
    }

    ret = res_load(test_data->path, input_buffer, expect_size, test_data->offset);
    test_data->offset += ret;
    *fill_input_size = ret;

    printf("expect_size:%d ret:%d\n", expect_size, ret);
    return;
}

void encoder_dec_fill_input_buffer(void *private_ptr, int expect_size, void *input_buffer,
                                   int *fill_input_size)
{
    int read_size = 0;
    int rest_of_size = 0;
    jpu_hal_enc_test_data_t *test_data = (jpu_hal_enc_test_data_t *)private_ptr;

    rest_of_size = test_data->output_size - test_data->output_offset;
    read_size = rest_of_size < expect_size ? rest_of_size : expect_size;

    memcpy(input_buffer, test_data->output_buffer + test_data->output_offset, read_size);

    test_data->output_offset += read_size;
    *fill_input_size = read_size;

    printf("expect_size:%d read_size:%d\n", expect_size, read_size);
    return;
}

void enc_fill_input_buffer(void *private_ptr, int expect_size, void *input_buffer,
                           int *fill_input_size)
{
    jpu_hal_enc_test_data_t *test_data = (jpu_hal_enc_test_data_t *)private_ptr;

    if (test_data->input_offset + expect_size > test_data->input_size) {
        printf("Error, too much want read, test_data->input_offset:%d expect_size:%d\n",
               test_data->input_offset, expect_size);
        *fill_input_size = 0;
        return;
    }

    memcpy(input_buffer, test_data->input_buffer + test_data->input_offset, expect_size);
    test_data->input_offset += expect_size;

    *fill_input_size = expect_size;

    if (test_data->input_offset >= test_data->input_size) {
        printf("read resource finished\n");
        test_data->read_finished = 1;
    }

    return;
}

void read_output_buffer(void *private_ptr, void *output_buffer, int output_size)
{
    jpu_hal_enc_test_data_t *test_data = (jpu_hal_enc_test_data_t *)private_ptr;

    for (int i = 0; i < output_size; i++) {
        if (((uint8_t *)output_buffer)[i] !=
            ((uint8_t *)test_data->verify_buffer)[test_data->verify_offset + i]) {
            printf("[%s][%d] Data does not match, "
                   "test_data->verify_offset + i:%d test_data->verify_buffer[%d]:%d\n",
                   __func__, __LINE__, test_data->verify_offset + i, test_data->verify_offset + i,
                   ((uint8_t *)test_data->verify_buffer)[test_data->verify_offset + i]);
        }
    }

    memcpy(test_data->output_buffer + test_data->output_offset, output_buffer, output_size);

    test_data->output_offset += output_size;
    test_data->output_size += output_size;
    test_data->verify_offset += output_size;

    printf("output_size:%d\n", output_size);
    return;
}

int show_decode_picture(display_handle* display, decoder_output_t* decData)
{
    int n_bufs = 1;
    struct sdm_buffer sdmbuf[1] = {0};
    struct sdm_post_config postConfig = {0};

    if (!decData->y_ptr || !decData->u_ptr || !decData->v_ptr) {
        printf("ERR: Only support yuv420 buffer show on screen\n");
        return -1;
    }

    postConfig.bufs = sdmbuf;
    postConfig.n_bufs = n_bufs;

    for (int i = 0; i < n_bufs; i++) {
        struct sdm_buffer *buf = &postConfig.bufs[i];

        buf->addr[0] = (unsigned long)decData->y_ptr;
        buf->addr[1] = (unsigned long)decData->u_ptr;
        buf->addr[2] = (unsigned long)decData->v_ptr;

        buf->layer = 1;
        buf->layer_dirty = 1;
        buf->alpha = 0xff;
        buf->alpha_en = 1;
        buf->ckey = 0;
        buf->ckey_en = 0;
        buf->fmt = COLOR_YUV420P;
        buf->layer_en = 1;
        buf->src.x = 0;
        buf->src.y = 0;
        buf->src.w = decData->output_width;
        buf->src.h = decData->output_height;

        buf->start.x = 0;
        buf->start.y = 0;
        buf->start.w = decData->pic_width;
        buf->start.h = decData->pic_height;

        // src_stride
        buf->src_stride[0] = decData->output_width;
        buf->src_stride[1] = decData->output_width / 2;
        buf->src_stride[2] = decData->output_width / 2;

        // 1920x720 is screen size
        buf->dst.x = 0;
        buf->dst.y = 0;
        buf->dst.w = 1920;
        buf->dst.h = 720;
        buf->z_order = 0;
    }

    return sdm_post(display, &postConfig);
}

int32_t test_jpu_reqDec_channel(jpu_hal_dec_test_data_t* testDecData)
{
    decoder_opt_t dec_data;

    testDecData->path = JPU_HAL_TEST_RESOURCE_PATH;
    testDecData->size = res_size(testDecData->path);
    testDecData->offset = 0;

    dec_data.dec_feeding_method = FEED_METHOD_FIXED_SIZE;
    dec_data.fill_buffer_callback = dec_fill_input_buffer;
    dec_data.output_cbcrInterleave = CBCR_SEPARATED;
    dec_data.output_packed_format = PACKED_FORMAT_NONE;
    dec_data.output_format = FORMAT_420;
    dec_data.user_private_ptr = testDecData;

    return vpu_acquire_chan(dev, VPU_REQ_DEC_CHAN_TYPE, &dec_data);
}

void jpu_hal_test_decode(void* argv)
{
    int32_t cnt = 0;
    int32_t ret = VPU_CODEC_RET_ERROR;
    jpu_hal_dec_test_data_t dec_test_data;
    decoder_output_t out_data;
    display_handle* disp_handle = hal_get_display_handle(DISPLAY_SCREEN_INDEX);
    int32_t chan_id = test_jpu_reqDec_channel(&dec_test_data);

    do {
        ret = vpu_chan_codec_sync(dev, chan_id);
        if (vpu_device_ctrl(dev, chan_id, VPU_CODEC_CTLCMD_GET_DECOUT_DATA, &out_data) == 0 &&
            out_data.is_valid) {
            // display
            show_decode_picture(disp_handle, &out_data);
        }
        printf("[debug][%lld]jpu hal decompress times: %d, ret: %d\n", get_systick(), cnt++, ret);
    } while (ret != VPU_CODEC_RET_BITSTREAM_DONE && ret != VPU_CODEC_RET_ERROR);

    vpu_release_chan(dev, chan_id);

    if (disp_handle)
        sdm_clear_display(disp_handle);

    return;
}

void jpu_test_multi_dec(void* argv)
{
    jpu_hal_test_decode(argv);
    osThreadExit();
}

void callback(uint16_t chanId, int32_t ret, void* usrData)
{
    *(int32_t *)usrData = ret;
}

void jpu_hal_test_decode_async(void* argv)
{
    int32_t ret = VPU_CODEC_RET_ERROR;
    decoder_output_t out_data;
    jpu_hal_dec_test_data_t dec_test_data;
    display_handle* disp_handle = hal_get_display_handle(DISPLAY_SCREEN_INDEX);
    int32_t chan_id = test_jpu_reqDec_channel(&dec_test_data);
    int32_t decoder_ret = 0;

    do {
        ret = vpu_chan_codec_async(dev, chan_id, callback, &decoder_ret);
        osDelay(30);
        if (vpu_device_ctrl(dev, chan_id, VPU_CODEC_CTLCMD_GET_DECOUT_DATA, &out_data) == 0 &&
            out_data.is_valid) {
            // display
            show_decode_picture(disp_handle, &out_data);
        }
    }while (ret != -1 && decoder_ret != VPU_CODEC_RET_BITSTREAM_DONE && decoder_ret != VPU_CODEC_RET_ERROR);

    vpu_release_chan(dev, chan_id);

    if (disp_handle)
        sdm_clear_display(disp_handle);

    return;
}

/**
 * @brief multi dec ins test
 *
 * @param argc arguement count
 * @param argv
 */
void jpu_hal_multi_dec_test_task(void* argv)
{
#define THREAD_NUMBER 2
    // osThreadId_t thread[THREAD_NUMBER];

    for (int32_t i = 0; i < THREAD_NUMBER; i++) {
        osThreadNew(jpu_test_multi_dec, NULL, NULL);
        osDelay(2000);
    }
    return;
}

/****************************************************************************************
**@brief encoder test implement
****************************************************************************************/
int32_t test_jpu_reqEnc_channel(jpu_hal_enc_test_data_t* testEncData)
{
    encoder_opt_t enc_data;
    int32_t ret = 0;

    // encoder test data
    testEncData->input_size = res_size(JPU_HAL_TEST_ENCODE_YUV_PATH);
    testEncData->input_buffer = memalign(1024, testEncData->input_size);
    ret = res_load(JPU_HAL_TEST_ENCODE_YUV_PATH, testEncData->input_buffer,
                   JPU_CEIL(1024, testEncData->input_size), 0);
    if (ret < 0) {
        printf("res_load failed; testEncData->verify_size:%d testEncData->verify_buffer:%p\n",
               testEncData->verify_size, testEncData->verify_buffer);
        return ret;
    }
    testEncData->input_offset = 0;
    testEncData->read_finished = 0;
    testEncData->verify_size = res_size(JPU_HAL_TEST_ENCODE_VERIFY_JPEG_PATH);
    testEncData->verify_offset = 0;
    testEncData->verify_buffer = memalign(1024, testEncData->verify_size);
    ret = res_load(JPU_HAL_TEST_ENCODE_VERIFY_JPEG_PATH, testEncData->verify_buffer,
                   JPU_CEIL(1024, testEncData->verify_size), 0);
    if (ret < 0) {
        printf("res_load failed; testEncData->verify_size:%d testEncData->verify_buffer:%p\n",
               testEncData->verify_size, testEncData->verify_buffer);
        return ret;
    }
    testEncData->output_size = 0;
    testEncData->output_offset = 0;
    testEncData->output_buffer = memalign(1024, testEncData->verify_size);

    // encoder option data
    enc_data.user_private_ptr = testEncData;
    enc_data.fill_buffer_callback = enc_fill_input_buffer;
    enc_data.read_buffer_callback = read_output_buffer;
    enc_data.pic_width = 800;
    enc_data.pic_height = 480;
    enc_data.frame_format = FORMAT_420;
    enc_data.package_format = 0;

    return vpu_acquire_chan(dev, VPU_REQ_ENC_CHAN_TYPE, &enc_data);
}

void jpu_hal_test_encode(void* argv)
{
    int ret = 0;
    jpu_hal_enc_test_data_t enc_test_data;
    int32_t enc_chan_id = test_jpu_reqEnc_channel(&enc_test_data);

    while (!enc_test_data.read_finished) {
        ret = vpu_chan_codec_sync(dev, enc_chan_id);
        if (ret < 0) {
            printf("jpu encompress failed\n");
            break;
        }
    }

    // decoder
    {
        // decode the output jpg and display
        enc_test_data.output_offset = 0;

        // request decoder channel
        decoder_opt_t dec_data;
        // init
        // decode_jpu = hal_jpu_hw_init();
        dec_data.user_private_ptr = &enc_test_data;
        dec_data.fill_buffer_callback = encoder_dec_fill_input_buffer;
        dec_data.dec_feeding_method =
            FEED_METHOD_FIXED_SIZE; // defalut set to FEEDING_METHOD_FIXED_SIZE
        dec_data.output_cbcrInterleave =
            CBCR_SEPARATED; // defalut set to CBCR_SEPARATED
        dec_data.output_packed_format =
            PACKED_FORMAT_NONE; // defalut set to PACKED_FORMAT_NONE
        // Only valid when output_packed_format is set to PACKED_FORMAT_NONE
        dec_data.output_format = FORMAT_420; // defalut set to FORMAT_444

        int32_t chan_id = vpu_acquire_chan(dev, 0, &dec_data);

        // decode
        decoder_output_t out_data;
        display_handle* disp_handle = hal_get_display_handle(DISPLAY_SCREEN_INDEX);

        do {
            ret = vpu_chan_codec_sync(dev, chan_id);
            if (vpu_device_ctrl(dev, chan_id, VPU_CODEC_CTLCMD_GET_DECOUT_DATA, &out_data) == 0 &&
                out_data.is_valid) {
                // display
                show_decode_picture(disp_handle, &out_data);
                printf("[debug]show encoder image\n");
            }
            printf("[debug]show encoder image out data: wi: %d, %d heigh: %d, %d\n", out_data.output_width, out_data.pic_width, out_data.output_height, out_data.pic_height);
        } while (ret != VPU_CODEC_RET_BITSTREAM_DONE && ret != VPU_CODEC_RET_ERROR);

        vpu_release_chan(dev, chan_id);

        osDelay(2000);
        if (disp_handle)
            sdm_clear_display(disp_handle);
    }

        vpu_release_chan(dev, enc_chan_id);


        if (enc_test_data.verify_buffer)
            free(enc_test_data.verify_buffer);

        if (enc_test_data.input_buffer)
            free(enc_test_data.input_buffer);

        if (enc_test_data.output_buffer)
            free(enc_test_data.output_buffer);

    return;
}

void jpu_hal_deinit(void* argv)
{
    printf("deinit jpu device\n");
    vpu_deinit(jpu_dev);
}

/* sub command manager table */
static subCmd_mgr_t s_subCmd_mgr_tbl[] = {
    {"dec_sync"     , 0, jpu_hal_test_decode},
    {"dec_async"    , 0, jpu_hal_test_decode_async},
    {"enc_sync"     , 0, jpu_hal_test_encode},
    {"dec_multi"    , 0, jpu_hal_multi_dec_test_task},
    {"deinit"       , 0, jpu_hal_deinit},
    {NULL}
};

static void subCmd_help_msg()
{
    printf("jpu_test [sub cmd]:\n");
    printf("    dec_sync: test decoder sync api\n");
    printf("    dec_async: test decoder async api\n");
    printf("    enc_sync: test encoder sync api\n");
    printf("    dec_multi: test multi task decoder\n");
    printf("    deinit: deinit jpu device\n");
    return;
}

int32_t vpu_test_main(int32_t argc, char* argv[])
{
    int32_t validpar_index = 1;
    dev = jpu_dev;

    // parameter check
    if (argc == 0 ||
        strncmp(argv[0], "?", strlen("?")) == 0 ||
        strncmp(argv[0], "-h", strlen("-h")) == 0) {
            subCmd_help_msg();
            return 0;
    }

    // command parser
    int32_t i = -1;
    int32_t subcmd_parnum = argc - validpar_index;
    while (subcmd_parnum >= 0 &&
            s_subCmd_mgr_tbl[++i].sub_cmd != NULL) {
        if (strncmp(argv[validpar_index - 1], s_subCmd_mgr_tbl[i].sub_cmd, strlen(argv[validpar_index - 1])) == 0 &&
            strlen(argv[validpar_index - 1]) == strlen(s_subCmd_mgr_tbl[i].sub_cmd)) {
            if (subcmd_parnum == s_subCmd_mgr_tbl[i].par_num) {
                s_subCmd_mgr_tbl[i].entry((void *)&argv[validpar_index]);
            }
            else {
                TEST_VPU_LOG("parameter number invalid for sub cmd: %s\n", s_subCmd_mgr_tbl[i].sub_cmd);
            }
            break;
        }
        else {
            // continue check subcmd
        }
    }

    if (i == -1 ||
        s_subCmd_mgr_tbl[i].sub_cmd == NULL) {
        TEST_VPU_LOG("do not support sub cmd: %s for vpu\n", argv[1]);
        return 0;
    }

    return 0;
}

CLI_CMD("jpu_test", "jpu function test. jpu_test -h for more information\n", vpu_test_main);
/* end of file jpu_test.c */