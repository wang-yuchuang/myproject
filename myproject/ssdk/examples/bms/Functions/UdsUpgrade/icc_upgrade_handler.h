#ifndef __ICC_UPGRADE_HANDLER_H__
#define __ICC_UPGRADE_HANDLER_H__

#include <stdint.h>

#define ICC_CMD_FILE_START          0x01
#define ICC_CMD_FILE_START_ACK      0x02
#define ICC_CMD_FILE_CHUNK_REQ      0x03
#define ICC_CMD_FILE_CHUNK_ACK      0x04
#define ICC_CMD_ABORT               0x05
#define ICC_CMD_UPGRADE_PROGRESS    0x10
#define ICC_CMD_UPGRADE_RESULT      0x20

#define ICC_START_STATUS_OK             0x00
#define ICC_START_STATUS_ERR_TYPE       0x01
#define ICC_START_STATUS_ERR_ADDR       0x02
#define ICC_START_STATUS_ERR_ADDR_NUM   0x03
#define ICC_START_STATUS_ERR_MEMORY     0x04

#define ICC_CHUNK_REQ_STATUS_OK      0x00
#define ICC_CHUNK_REQ_STATUS_GAP     0x01
#define ICC_CHUNK_REQ_STATUS_DUP     0x02

#define ICC_ABORT_REASON_USER        1
#define ICC_ABORT_REASON_TRANSFER    2
#define ICC_ABORT_REASON_SEQ         3
#define ICC_ABORT_REASON_TIMEOUT     4
#define ICC_ABORT_REASON_CRC         5

#define UDS_FAIL_CAN_TX              0xE0
#define UDS_FAIL_CAN_RX_TIMEOUT      0xE1
#define UDS_FAIL_INTERNAL            0xE2
#define UDS_FAIL_SID_TIMEOUT         0xE3
#define UDS_FAIL_FILE_READ           0xE4
#define UDS_FAIL_MEM_ALLOC           0xE5

#define ICC_UPGRADE_RESULT_OK        0
#define ICC_UPGRADE_RESULT_FAIL      1

typedef enum {
    STATE_IDLE = 0,
    STATE_FILE_RECEIVING,
    STATE_FILE_CRC_CHECK,
    STATE_UPGRADING,
    STATE_ABORTING,
} upgrade_state_t;

extern uint8_t  *g_upgrade_file_buf;
extern uint32_t  g_upgrade_file_size;
extern uint16_t  g_upgrade_chunk_size;
extern uint16_t  g_upgrade_chunk_count;
extern uint32_t  g_upgrade_file_crc32;
extern uint8_t   g_upgrade_dev_type;
extern uint8_t   g_upgrade_dev_num;
extern uint8_t   g_upgrade_dev_addr[16];
extern upgrade_state_t g_upgrade_state;

void icc_upgrade_handle_msg(const uint8_t *data, uint32_t len);

void icc_upgrade_send_msg(uint8_t cmd, const uint8_t *payload, uint32_t payload_len);

void icc_upgrade_send_start_ack_ex(uint8_t status,
        uint32_t total_size, uint16_t chunk_size, uint16_t chunk_count,
        uint32_t crc32, uint8_t dev_type, uint8_t dev_num,
        const uint8_t *dev_addrs);

void icc_upgrade_send_abort(uint8_t reason);

void icc_upgrade_process(void);

void icc_upgrade_cleanup(void);

#endif
