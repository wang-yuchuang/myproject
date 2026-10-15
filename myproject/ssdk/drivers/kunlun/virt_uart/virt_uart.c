#include <stdlib.h>
#include <debug.h>
#include <string.h>
#include <assert.h>
#include <cmsis_os2.h>
#include <regs_base.h>
#include <reg.h>

#include "driver/kunlun/virt_uart/virt_uart.h"
#include "driver/kunlun/virt_uart/logbuf.h"

static void *logbank_hdr = NULL;
static bool inclear; /* avoid accessing the buffer when clearing it */
static bool is_available;

#ifdef CONFIG_PARALLEL_MODE
static core_type_t ctype_backup = SDRV_CORE_MAX;

#define SDRV_GENERAL_REG(n) (APB_RSTGEN_SEC_BASE + (uint32_t)((0x50 + (n)*4) << 10))
#define SDRV_REG_STATUS_DDR_INIT_DONE (1<<1)
#define SDRV_REG_STATUS 6
#endif /* CONFIG_PARALLEL_MODE */

int virt_uart_init(core_type_t ctype)
{
    void *addr;

    if (logbank_hdr != NULL)
        return 0;

    addr = sdrv_logbuf_get_bank_hdr(ctype);

    if (!addr) {
        ssdk_printf(SSDK_ALERT, "virt_uart: failed to get log bank header: [%d]\n", ctype);
        return -1;
    }

    logbank_hdr = addr;
    is_available = 1;
    inclear = 0;

    return 0;
}

#ifdef CONFIG_PARALLEL_MODE
static void virt_uart_init_thread(void* arg)
{
    while (!(readl(SDRV_GENERAL_REG(SDRV_REG_STATUS))
            & SDRV_REG_STATUS_DDR_INIT_DONE)) {
        osDelay(1);
    }

    virt_uart_init(ctype_backup);
    osThreadExit();
}

int virt_uart_init_in_parallel_mode(core_type_t ctype)
{
    ctype_backup = ctype;
    osThreadAttr_t attr = { .name = "virt_uart_init" };
    osThreadId_t thread_id = osThreadNew(virt_uart_init_thread, NULL, &attr);
    if (!thread_id) {
        return -1;
    }

    return 0;
}
#endif

int virt_uart_putc(char c)
{
    if (inclear)
        return 0;

    sdrv_logbuf_putc(logbank_hdr, c);

#ifdef CONFIG_SDRV_VIRT_CONSOLE
    if (c == '\n')
        sdshell_wake_thread();
#endif

    return 0;
}

int virt_uart_is_available(void)
{
    return is_available && sdrv_logbuf_is_enable();
}

#ifdef CONFIG_SDRV_VIRT_CONSOLE
void virt_uart_clear_buf(void)
{
    inclear = 1;
    sdrv_logbuf_clear(logbank_hdr);
    inclear = 0;
}

int virt_uart_get_last_buf_and_size(char **buf, uint32_t *size, uint32_t len)
{
    if (inclear)
        return 0;

    return sdrv_logbuf_get_last_buf_and_size(logbank_hdr, buf, size, len);
}

int virt_uart_get_new_buf(char *buf, uint32_t size)
{
    if (inclear)
        return 0;

    return sdrv_logbuf_get_new_buf(logbank_hdr, buf, size);
}

int virt_uart_update_tail(void)
{
    return sdrv_logbuf_update_tail(logbank_hdr);
}
#endif

