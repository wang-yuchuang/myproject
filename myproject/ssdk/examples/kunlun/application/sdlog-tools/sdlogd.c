#include <stdlib.h>
#include <stdio.h>
#include <debug.h>
#include <driver/kunlun/virt_uart/logbuf.h>
#include <cmsis_os2.h>
#include <CLI.h>
#include <malloc.h>
#include <string.h>
#include <board.h>
#include <getopt.h>

#ifdef BIT_MASK
#undef BIT_MASK
#endif
#define BIT_MASK(i)                 (1<<(i))
#define LOGTYPE_LEN                 (32)
#define SDLOGD_POLL_MAX_INTERVAL_MS (10000)

#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#endif
#define ARRAY_SIZE(x)               (sizeof(x)/sizeof(x[0]))

typedef struct _sdlogd_cfg_st {
    osThreadId_t poll_thread;
    uint32_t poll_mask;
    uint32_t poll_interval;
    void* logbuf_hdr[SDRV_CORE_MAX];
    uint32_t update_threshold;
} sdlogd_cfg_st;

typedef struct _sdlogd_msg {
    uint32_t type;
    uint32_t size;
    uint8_t data[0];
} sdlogd_msg_t;

static const char* sdlogd_tag_list[] = {
    [SAFETY_R5] = "[saf]",
    [SECURE_R5] = "[sec]",
    [MPC_R5]    = "[mpc]",
    [AP1_A55]   = "[ap1]",
    [AP1_USER]  = "[ap1]",
    [AP2_A55]   = "[ap2]",
    [AP2_USER]  = "[ap2]",
};

sdlogd_cfg_st sdlogd_cfg;
bool sdlogd_running = false;

#ifdef CONFIG_SDSHELL_BACKEND
bool is_sdshell_backend_running(void);
#endif

static void sdlogd_usage(void)
{
    printf("Usage: sdlogd -o [type] -d [path] [-t] [-l] [-e]\n");
    printf("Scan and store different system logs\n");
    printf("\t-o    [mandatory] Specify the log type (safety | secure | mp | |ap1 | ap2)\n");
    printf("\t-d    [mandatory] Specify the directory to save the log file\n");
    printf("\t-t    [optional] Set the polling interval (unit: millisecond, max: 10000ms, default: 5000ms)\n");
    printf("\t-l    [optional] Set the update threshold (unit: byte, default: 50 bytes)\n");
    printf("\t-r    [optional] Set the check threshold for file rollback (unit: Mbyte, default: 20)\n");
    printf("\t-f    [optional] Set the max number of rollback files (default: 4)\n");
    printf("\t-e    [optional] Enable exception log partition parsing function\n");
}

static int sdlogd_set_log_type(sdlogd_cfg_st* cfg, core_type_t ctype)
{
    if (cfg && ctype < SDRV_CORE_MAX) {
        cfg->poll_mask |= BIT_MASK(ctype);
        return 0;
    }
    return -1;
}

static sdlogd_msg_t* sdlogd_get_log_buffer(void* hdr, core_type_t type, bool new_log)
{
    sdlogd_msg_t* msg;
    int buff_size = 0;
    char* buf[SDRV_LOGBUG_SEGMENT_NUM];
    int size[SDRV_LOGBUG_SEGMENT_NUM] = {0};
    int ret = -1;

    buff_size = sdrv_logbuf_get_bank_buf_size(hdr);
    if (buff_size < 0) {
        return NULL;
    }

    msg = (sdlogd_msg_t*)malloc(sizeof(sdlogd_msg_t)+buff_size);
    if (!msg) {
        return NULL;
    }

    memset(msg, 0, sizeof(sdlogd_msg_t)+buff_size);
    if (!new_log) {
        ret = sdrv_logbuf_get_last_buf_and_size(hdr, buf, (uint32_t*)size, ARRAY_SIZE(size));
    }
    else {
        ret = sdrv_logbuf_get_new_buf_and_size(hdr, buf, size, ARRAY_SIZE(size));
    }
    if (ret) {
        goto fail;
    }

    if (buff_size < size[0]+size[1]) {
        goto fail;
    }

    msg->type = type;
    msg->size = size[0]+size[1];
    memcpy(msg->data, buf[0], size[0]);
    if (size[1] > 0)
        memcpy(msg->data+size[0], buf[1], size[1]);
    return msg;

fail:
    if (msg) {
        free(msg);
        msg = NULL;
    }
    return NULL;
}

#ifdef CONFIG_SDLOGD_OUTPUT_UART
static void sdlogd_output_log_tag(int ctype)
{
    if (ctype < ARRAY_SIZE(sdlogd_tag_list)) {
        for (int i = 0; i < strlen(sdlogd_tag_list[ctype]); i++) {
            board_raw_putc(sdlogd_tag_list[ctype][i]);
        }
    }
}

static void sdlogd_output_uart(int ctype, bool new_log)
{
    sdlogd_cfg_st* cfg = &sdlogd_cfg;
    bool need_tag = true;

    if (ctype < SDRV_CORE_MAX) {
        sdlogd_msg_t* msg = sdlogd_get_log_buffer(cfg->logbuf_hdr[ctype], ctype, new_log);
        if (!msg)
            return;

        sdrv_logbuf_update_bk_tail1(cfg->logbuf_hdr[ctype]);
        for (int i = 0 ; i < msg->size; i++) {
            if (need_tag) {
                need_tag = false;
                sdlogd_output_log_tag(ctype);
            }
            board_raw_putc(msg->data[i]);
            if ('\n' == msg->data[i]) {
                need_tag = true;
            }
        }
    }
}
#endif

static void sdlogd_thread(void* arg)
{
    sdlogd_cfg_st* cfg = &sdlogd_cfg;
    uint32_t start = current_time(), end;
    bool update_time = false;

    while (sdlogd_running) {
        update_time = false;
        end = current_time();
        for (uint32_t i = 0, mask = cfg->poll_mask;
             i < SDRV_CORE_MAX && mask;
             i++, mask>>=1) {
            if (mask&1) {
                int new_size = sdrv_logbuf_get_new_log_size(cfg->logbuf_hdr[i]);
                if ((new_size > cfg->update_threshold)
                    || (new_size > 0 && abs(end-start) > cfg->poll_interval)) {
                    update_time = true;
#ifdef CONFIG_SDLOGD_OUTPUT_UART
#ifdef CONFIG_SDSHELL_BACKEND
                    if (!is_sdshell_backend_running())
#endif
                        sdlogd_output_uart(i, true);
#endif
                }
            }
        }
        if (update_time) {
            start = current_time();
        }
        osDelay(100);
    }

    osThreadExit();
}

static int sdlogd_init(int argc, char* argv[])
{
    sdlogd_cfg_st* cfg = &sdlogd_cfg;
    int opt;

    memset(cfg, 0, sizeof(*cfg));
    cfg->poll_interval = atoi(CONFIG_SDLOGD_POLL_INTERVAL_MS);
    cfg->update_threshold = atoi(CONFIG_SDLOGD_UPDATE_THRESHOLD);
    optind = 0; /* must, this variable not be clear */
    while ((opt = getopt(argc, argv, "d:o:t:l:f:r:e")) != EOF) {
        switch (opt) {
        case 'o':
            {
                char log_type[LOGTYPE_LEN] = {0};
                strncpy(log_type, optarg, LOGTYPE_LEN-1);
                if (!strcasecmp(log_type, "ap1")) {
                    sdlogd_set_log_type(cfg, AP1_A55);
                    sdlogd_set_log_type(cfg, AP1_USER);
                }
                else if (!strcasecmp(log_type, "ap2")) {
                    sdlogd_set_log_type(cfg, AP2_A55);
                    sdlogd_set_log_type(cfg, AP2_USER);
                }
                else if (!strcasecmp(log_type, "safety")) {
                    sdlogd_set_log_type(cfg, SAFETY_R5);
                }
                else if (!strcasecmp(log_type, "secure")) {
                    sdlogd_set_log_type(cfg, SECURE_R5);
                }
                else if (!strcasecmp(log_type, "mp") || !strcasecmp(log_type, "mpc")) {
#ifndef CONFIG_MP
                    sdlogd_set_log_type(cfg, MPC_R5);
#else
                    ssdk_printf(SSDK_ERR, "sdlogd not monitor mp logbuf in mp core.\n");
#endif
                }
                else {
                    ssdk_printf(SSDK_ERR, "log type(%s) not recognized.\n", log_type);
                }
                break;
            }
        case 't':
            {
                cfg->poll_interval = atoi(optarg);
                if (cfg->poll_interval > SDLOGD_POLL_MAX_INTERVAL_MS)
                    cfg->poll_interval = SDLOGD_POLL_MAX_INTERVAL_MS;
                break;
            }
        case 'l':
            {
                cfg->update_threshold = atoi(optarg);
                break;
            }
        default:
            {
                break;
            }
        }
    }

    for (int i = 0; i < SDRV_CORE_MAX; i++) {
        cfg->logbuf_hdr[i] = sdrv_logbuf_get_bank_hdr(i);
        if (BIT_MASK(i) & cfg->poll_mask)
            sdlogd_output_uart(i, false);
    }

    return 0;
}

void sdlogd_entry(int argc, char* argv[])
{
    if (!sdlogd_running) {
        sdlogd_running = true;
        if (!sdlogd_init(argc, argv)) {
            osThreadAttr_t attr = { .name = "sdlogd" };
            osThreadNew(sdlogd_thread, NULL, &attr);
        }
        else {
            ssdk_printf(SSDK_ERR, "sdlogd_init fail.\n");
        }
    }
    else {
        ssdk_printf(SSDK_CRIT, "sdlogd already running...\n");
    }
}

bool is_sdlogd_running(void)
{
    return sdlogd_running;
}

static int cmd_sdlogd(int argc, char* argv[])
{
    if (argc == 1) {
        if (!strcasecmp(argv[0], "stop"))
            sdlogd_running = false;
        else if (!strcasecmp(argv[0], "-h"))
            sdlogd_usage();
    }
    else {
        int argc_new = argc+1;
        char** argv_new = (char**)malloc(sizeof(char*)*argc_new);
        argv_new[0] = "sdlogd";
        for (int i = 1; i < argc_new; i++) {
            argv_new[i] = argv[i-1];
        }
        sdlogd_entry(argc_new, argv_new);
    }
    return 0;
}

CLI_CMD("sdlogd", "sdlogd [stop] -o [c] [-t val] [-l threshold]\n", cmd_sdlogd);
/* EOF */
