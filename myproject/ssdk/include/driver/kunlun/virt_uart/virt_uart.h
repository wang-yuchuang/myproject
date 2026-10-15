#ifndef __DEV_VUART_H__
#define __DEV_VUART_H__
#include <stdbool.h>
#include <sys/types.h>
#include "logbuf.h"

int  virt_uart_init(core_type_t ctype);
#ifdef CONFIG_PARALLEL_MODE
int  virt_uart_init_in_parallel_mode(core_type_t ctype);
#endif
int  virt_uart_putc(char c);
int  virt_uart_is_available(void);

#ifdef CONFIG_SDRV_VIRT_CONSOLE
int  sdshell_wake_thread(void);
int  virt_uart_getc(int port, bool wait);
void virt_uart_clear_buf(void);
int  virt_uart_get_last_buf_and_size(char **buf, uint32_t *size, uint32_t len);
int  virt_uart_get_new_buf(char *buf, uint32_t size);
int  virt_uart_update_tail(void);
#endif

#endif
