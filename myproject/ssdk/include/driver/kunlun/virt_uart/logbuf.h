#ifndef _SDRV_VIRT_UART_COMMON_H_
#define _SDRV_VIRT_UART_COMMON_H_
#include <image_cfg.h>

#define SDRV_LOGBUF_HEADER    SEC_MEMBASE
#define SDRV_LOGBUF_SIZE      0x200000
#define SDRV_RAMOOPS_SIZE     0x100000

#define SDRV_LOGBUG_SEGMENT_NUM	 (2)

typedef enum {
    SAFETY_R5 = 0,
    SECURE_R5,
    MPC_R5,
    AP1_A55,
    AP1_USER,
    AP2_A55,
    AP2_USER,
    SDRV_CORE_MAX
} core_type_t;

void *sdrv_logbuf_get_bank_hdr(core_type_t ctype);
int sdrv_logbuf_putc(void *hdr, char c);
int sdrv_logbuf_get_last_buf_and_size(void *hdr, char **buf, uint32_t* size, uint32_t len);
int sdrv_logbuf_get_new_buf(void *hdr, char *buf, uint32_t size);
int sdrv_logbuf_update_tail(void *hdr);
void sdrv_logbuf_clear(void *hdr);
int sdrv_logbuf_is_enable(void);
int timestamp_is_enable(void);
void sdrv_logbuf_clear_buf(void* hdr);
int sdrv_logbuf_get_new_log_size(void* hdr);
int sdrv_logbuf_get_bank_buf_size(void* hdr);
int sdrv_logbuf_get_new_buf_and_size(void* hdr, char** buf, int* size, int len);
void sdrv_logbuf_update_bk_tail1(void* hdr);
#endif /* _SDRV_VIRT_UART_COMMON_H_ */
