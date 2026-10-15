#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <driver/uart.h>
#include <board.h>
#include <spinlock.h>
#include <getopt.h>
#include "CLI_type.h"
#include "driver/kunlun/virt_uart/logbuf.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))
#endif
#define LOGTYPE_LEN    (32)

static const char *const logtype_list[] = { "safety",
"secure", "mp", "ap1", "ap1_user", "ap2", "ap2_user"};

static spin_lock_t printf_lock;

void usage(void)
{
    printf("Usage: sdlog -o [type] -u \n");
    printf("View different system logs\n");
    printf("\t-o    [mandatory] Specify the log type (safety | secure | mp | ap1 | ap2)\n");
    printf("\t-u    [optional] Output the user space logs, only valid for ap2\n");
    printf("\t-c    [optional] Clear the log\n");
}

int sdlog_get_logtype_idx(const char *name, int flag)
{
    int i;

    if ((strncmp("ap2", name, 3) == 0) && (flag == 1))
        name = "ap2_user";
	else if ((strncmp("ap1", name, 3) == 0) && (flag == 1))
		name = "ap1_user";

    for (i = 0; i < ARRAY_SIZE(logtype_list); i++) {
        if (strcmp(logtype_list[i], name) == 0) {
            return i;
        }
    }

    return -1;
}

int sdlogc_show_log(int type)
{
    void *addr;
    char *buf[SDRV_LOGBUG_SEGMENT_NUM];
    uint32_t size[SDRV_LOGBUG_SEGMENT_NUM];
    int i, j;
    char last_char = '\n';
    irq_state_t state;

    addr = sdrv_logbuf_get_bank_hdr(type);

    if (!addr) {
        printf("sdrv_logbuf: failed to get log bank header: [%d]\n", type);
        return -1;
    }

    sdrv_logbuf_get_last_buf_and_size(addr, buf, size, ARRAY_SIZE(buf));

    /* To avoid logging to logbuf, use uart_putc to print output */
    state = spin_lock_irqsave(&printf_lock);
    for (i = 0; i < SDRV_LOGBUG_SEGMENT_NUM; i++) {
        if (size[i] > 0) {
            for (j = 0; j < size[i]; j++) {
                board_raw_putc(buf[i][j]);
                last_char = buf[i][j];
            }
        }
    }

    if (last_char != '\n')
        board_raw_putc('\n');

    spin_unlock_irqrestore(&printf_lock, state);

    return 0;
}

int sdlogc_clear_log(int type)
{
    if (type < SDRV_CORE_MAX) {
        void* addr = sdrv_logbuf_get_bank_hdr(type);
        if (addr)
            sdrv_logbuf_clear_buf(addr);
    }
    return 0;
}

int cmd_sdlog(int argc, char *argv[])
{
    spin_lock_init(&printf_lock);

    if (!sdrv_logbuf_is_enable()) {
        printf("(sdrv_logbuf unavailable\n");
        return 0;
    }

    int argc_new = argc+1;
    char** argv_new = (char**)malloc(sizeof(char*)*argc_new);
    argv_new[0] = "sdlog";
    for (int i = 1; i < argc_new; i++) {
        argv_new[i] = argv[i-1];
    }

    int type;
    int opt;
    bool has_logtype = false;
    bool is_user = false;
    bool is_clear = false;
    char log_type[LOGTYPE_LEN] = {0};
    optind = 0; /* must, this variable not be clear */
    while ((opt = getopt(argc_new, argv_new, "o:uch")) != EOF) {
        switch (opt) {
        case 'o':
            {
                has_logtype = true;
                strncpy(log_type, optarg, LOGTYPE_LEN-1);
                break;
            }
        case 'u':
            {
                is_user = true;
                break;
            }
        case 'c':
            {
                is_clear = true;
                break;
            }
        case 'h':
            {
                usage();
                break;
            }
        default:
            {
                break;
            }
        }
    }

    if (has_logtype) {
        type = sdlog_get_logtype_idx(log_type, is_user);
        if ((type < 0) || type >= ARRAY_SIZE(logtype_list)) {
            usage();
            goto exit;
        }
        if (!is_clear) {
            sdlogc_show_log(type);
        }
        else {
            sdlogc_clear_log(type);
        }
    }
    else {
        usage();
        goto exit;
    }

exit:
    if (argv_new) {
        free(argv_new);
        argv_new = NULL;
    }

   return 0;
}

CLI_CMD("sdlog", "sdlog -o [safety|secure|mp|ap1|ap2] [-u] [-c]\n", cmd_sdlog);
