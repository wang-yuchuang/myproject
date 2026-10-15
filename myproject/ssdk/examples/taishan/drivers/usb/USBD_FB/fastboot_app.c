/**
 * @file fastboot_app.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: fastboot test & demo application.
 *
 * Revision History:
 * -----------------
 */


#include  "fastboot_common.h"


#define  FB_APP_DL_BUF_SZ       (16*1024u)


static void *fb_dl_buf;


static void cmd_erase_proc( fastboot_t  *fb,
                            const char  *arg,
                            void        *data,
                            unsigned    sz)
{
    printf("fb cmd erase %s\n", arg);
    fastboot_common_okay(fb, "");
    return;
}

static void cmd_reboot_proc(fastboot_t  *fb,
                            const char  *arg,
                            void        *data,
                            unsigned    sz)
{
    printf("fb cmd reboot device %s\n", arg);
    fastboot_common_fail(fb, "not implemented");
}

static void register_commands(void)
{
    static char max_download_size[MAX_RSP_SIZE] = {0};
    USBD_FB_CMD cmd_list[] = {
        {"flash:", USBD_FB_CmdFlash},
        {"erase:", cmd_erase_proc},
        {"reboot", cmd_reboot_proc},
    };

    int fastboot_cmds_count = sizeof(cmd_list) / sizeof(cmd_list[0]);

    for (int i = 0; i < fastboot_cmds_count; i++)
        fastboot_register_cmd(cmd_list[i].cmd, cmd_list[i].handler);

    /* publish variables and their values */
    fastboot_register_var("product",  "taishan");
    fastboot_register_var("serialno", "0123456789");
    fastboot_register_var("dloader-version", "V00.00.01");
    fastboot_register_var("dev-stage", "dloader");

    /* Max download size supported */
    snprintf(max_download_size, MAX_RSP_SIZE, "\t0x%x", FB_APP_DL_BUF_SZ);
    fastboot_register_var("max-download-size",
                          (const char *) max_download_size);
}


#include <CLI_type.h>

static int usbd_fb_app(int argc, char *argv[])
{
    static char run_once = 0;
    fastboot_t *fb;
    LIB_ERR err;

    if (!run_once) {
        run_once = 1;

        register_commands();

        fb_dl_buf = Mem_HeapAlloc(FB_APP_DL_BUF_SZ, 512, NULL, &err);
        if (!fb_dl_buf) {
            printf("fast boot alloc buffer fail\n");
            return CLI_TRUE;
        }

        fb = fastboot_common_init((void *)fb_dl_buf, FB_APP_DL_BUF_SZ);
        printf("fast boot class No.=%d\n", *fb);
    }

    return CLI_FALSE;
}

CLI_CMD("fastboot", "fastboot:\n usb device driver fast boot example application\n", usbd_fb_app);

