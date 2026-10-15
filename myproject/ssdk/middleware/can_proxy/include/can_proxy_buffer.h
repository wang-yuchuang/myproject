#ifndef __CAN_PROXY_BUFFER_H__
#define __CAN_PROXY_BUFFER_H__

#include <stdint.h>
#include <stdbool.h>

#include <can_proxy_if.h>

#define CP_BUFFER_SIZE  100
#define CP_BUFFER_COUNT  10

#define __CP_CREATE_PFRAME(type,payload_size,frm_name) \
    uint8_t frm_name##_space[sizeof(type) + payload_size];\
    type* frm_name = (type*)&frm_name##_space[0]

enum {
    DST_AP,
    DST_SAF,
};

bool can_proxy_buffer_init(void);
bool cp_if_frame_post(cp_if_frame_t *frame, int dst);
size_t cp_if_frame_get(cp_if_frame_t *frame, int dst, size_t max_data_len);

#endif