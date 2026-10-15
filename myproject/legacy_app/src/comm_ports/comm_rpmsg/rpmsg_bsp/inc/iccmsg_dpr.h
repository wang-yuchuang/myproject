#ifndef __ICCMSG_DPR_H__
#define __ICCMSG_DPR_H__
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "iccmsg_endian.h"

/* debug level */
#define LOG_EMERG       0  /* System is unusable */
#define LOG_ALERT       1  /* Action must be taken immediately */
#define LOG_CRIT        2  /* Critical conditions */
#define LOG_ERR         3  /* Error conditions */
#define LOG_WARNING     4  /* Warning conditions */
#define LOG_NOTICE      5  /* Normal, but significant, condition */
#define LOG_INFO        6  /* Informational message */
#define LOG_DEBUG       7  /* Debug-level message */

#define CONFIG_DEBUG
#define CONFIG_DEBUG_LEVEL      LOG_INFO


/* printf */
#ifdef CONFIG_DEBUG
#define LOG_PRINT(level, ...)    \
    do { \
        if ((level) <= CONFIG_DEBUG_LEVEL) { \
            (level > LOG_NOTICE) ? \
                printf(__VA_ARGS__) : \
                printf("%s %d: " __VA_ARGS__, __FUNCTION__, __LINE__); \
        } \
    } while (0)
#else
#define LOG_PRINT(level, x...)
#endif



#define ICCMSG_FRAME_MAX_LEN        492
#define ICCMSG_FRAME_HEAD_MAX_LEN   8
#define ICCMSG_FRAME_TAIL_MAX_LEN   8
#define ICCMSG_FRAME_DATA_LEN_LEN   2
#define ICCMSG_FRAME_CHECKSUM_LEN   4



#define ICCMSG_TX_REPEAT_COUNT      3
#define ICCMSG_TX_TIMEOUT_MS        500
#define ICCMSG_EPT_BASE_NUM         500
#define ICC_CHANNELS_MAX_NUM        100
#define ICC_EPT_NUM_MAX             (ICCMSG_EPT_BASE_NUM+ICC_CHANNELS_MAX_NUM)        
#define ICCMSG_TX_QUE_SIZE          256
#define ICCMSG_RX_QUE_SIZE          256
#define ICCMSG_DEFAULT_CACHE_SIZE   256
#define ICCMSG_ACK_EVENT            (1U << 0)   // bit 0: 收到 ACK
#define ICCMSG_FRM_MIN_LEN          17      //9+sizeof(iccmsg_data_type_t)
#define ICCMSG_FRAME_DATA_MAX_LEN   (ICCMSG_FRAME_MAX_LEN-ICCMSG_FRM_MIN_LEN)



typedef enum {
    SUM=0,
    CRC32,
}checksum_t;

typedef enum {
    NONE=0,     //no encrypt
    TEA,
    AES128,
}encrypt_t;

/*
    frame contains default index:
    |index1     |index2(fix 2 bytes)    |index3     |index4         |index5         |index6     |
    |-----------|-----------------------|-----------|---------------|---------------|-----------|
    |frame_head |data_len               |frame_data | frame_encrypt |frame_checksum |frame_tail |

    frame_data:
    |index1     |index2         |
    |-----------|---------------|
    |data_type  |original data  |

    frame_len = head_len + 2 + data_len + chksum_len + tail_len
*/

#define     DATA_TYPE_SINGLE_FRAME      0       //单帧数据
#define     DATA_TYPE_MULTI_FRAME_HEAD  1       //多帧数据头
#define     DATA_TYPE_MULTI_FRAME       2       //多帧数据
#define     DATA_TYPE_ACK               3

// iccmsg 网络层数据结构
typedef struct {
    uint8_t id;         // 消息id，代表消息用途，比如同一通道可能同时传输升级文件与多帧控制报文，用于区分
    uint8_t type;       // 消息类型，单帧消息，多帧消息，ACK帧消息
    uint8_t ack_flag;   // 是否需要ACK，ICCMSG_ACK_NO_NEED/ICCMSG_ACK_NEED
    uint8_t rsv1;       // 字节对齐预留
    uint8_t rsv2;       // 字节对齐预留
    uint8_t rsv3;       // 字节对齐预留
    uint16_t len;       // 原始数据长度
    uint32_t index;     // 当传输多帧消息时，index字段用于标识当前帧的序号，从0开始递增，当传输单帧消息时，index字段 unused.: 
}__attribute__ ((packed)) iccmsg_data_type_t;

typedef struct {
    uint32_t   len;         // 总包长度
    uint32_t   crc32;       // 总包数据校验值
    uint32_t   pack_num;    // 分包数量
}iccmsg_data_info_t;        // 总包信息，分包传输时第一帧数据 original data 内容

typedef struct {
    uint8_t frame_head[ICCMSG_FRAME_HEAD_MAX_LEN];
    uint8_t frame_tail[ICCMSG_FRAME_HEAD_MAX_LEN];
    uint8_t head_len;
    uint8_t tail_len;
    checksum_t checksum_type;
}frame_conf_t;

typedef struct {
    uint8_t* buf;   // 缓存数据内存，需要动态申请
    uint32_t len;   // 缓存长度
}que_msg_t;

// 消息方向
typedef enum {
    ICCMSG_DIR_RX = 0,  // 接收
    ICCMSG_DIR_TX       // 发送
} iccmsg_direction_t;

// 通道类型
typedef enum {
    ICCMSG_CH_IPCC = 0,
    ICCMSG_CH_VIRTIO
} iccmsg_channel_type_t;

// ack
typedef enum {
    /* 发送初始状态 */
    ICCMSG_ACK = 0,             //  发送ACK
    ICCMSG_ACK_NO_NEED,         //  无需ACK
    ICCMSG_ACK_NEED,            //  需要ACK

    /* 发送过程 */
    ICCMSG_CACHE_CREATE,        //  创建发送消息缓存
    ICCMSG_ACK_WAIT,            //  等待ACK
    ICCMSG_ACK_GOT,             //  收到ACK
    ICCMSG_ACK_TIMEOUT,         //  等待ACK超时
    ICCMSG_ASYNC_SEND_NOACK,    //  异步发送NOACK消息
    ICCMSG_SEND_FINISHED,       //  发送成功

    /* 接收状态 */
    ICCMSG_RX_RECORD,           //  接收数据收录缓存记录
    ICCMSG_RX_VERIFIED,         //  接收数据验证通过
    ICCMSG_RX_QUE_PUT_WITH_ACK, //  接收数据通过消息队列发出
    ICCMSG_RX_QUE_PUT_NO_ACK,   //  接收数据通过消息队列发出

} iccmsg_msg_status_t;





typedef struct {
    frame_conf_t fc;
} iccmsg_dpr_t;







// 协议帧配置
int iccmsg_frm_cfg(frame_conf_t* frm_cfg, 
                    uint8_t *head, 
                    uint8_t head_len, 
                    uint8_t *tail, 
                    uint8_t tail_len, 
                    checksum_t chksum);

int get_frame_data_len(const uint8_t *buf, const frame_conf_t *fc);


// 验证函数
int iccmsg_verify(const uint8_t *buf, uint32_t len, const frame_conf_t *fc);

// 数据打包,返回打包后的帧长度
int iccmsg_pack(uint8_t *dest, 
                const uint8_t *orig_data, 
                uint32_t orig_len, 
                const frame_conf_t *fc,
                iccmsg_data_type_t data_type);

int is_ack_frame(const frame_conf_t *fc, const uint8_t *frame, uint32_t frame_len);

int get_frame_data_max_len(const frame_conf_t *fc);
int get_frm_data_pos(const frame_conf_t *fc);
int get_origin_data_pos(const frame_conf_t *fc);








#endif
