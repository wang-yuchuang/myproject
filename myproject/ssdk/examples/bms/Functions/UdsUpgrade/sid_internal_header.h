/* *********************************
 * can uds 内部服务处理函数的头文件声明
 ***********************************/
#ifndef __SID_INTERNAL_HEADER_H__
#define __SID_INTERNAL_HEADER_H__
#include <debug.h>

#ifdef CONFIG_DEBUG
#define d_printf(x...)  printf(x)
#else
#define d_printf(x...)
#endif
//宏定义
#define UDS_SRV_RET_ID_OFFSET           0x40

#define SECURITY_REQ_SEED               0x01
#define SECURITY_SEND_KEY               0x02

//
#define COMPRESS_METHOD_NONE            (0<<4)  //34H服务中,压缩模式使用高4bit,目前不支持压缩
#define ENCRYPT_METHOD_NONE             (0)     //34H服务中,加密模式使用低4bit,目前不支持加密

enum {
    DEST_ACK_WAIT = 0,       //等待中
    DEST_ACK_POSITIVE = 1,   //收到正响应
    DEST_ACK_NEGATIVE = -2,  //收到负响应
    DEST_ACK_WRONG_PARA = -3,//收到正响应，但是参数内容是错的
    DEST_ACK_ERROR = -4,     //收到的是错误的响应值
};
//用于安全访问的相关控制结构体
typedef enum {
    SECURITY_LEVEL0 = 0,
    SECURITY_LEVEL1 = 1,
} SECURITY_ACCESS_LEVEL_E;
struct SECURITY_CTRL {
    SECURITY_ACCESS_LEVEL_E level;
    unsigned char seed[4];
};

//用于数据传输中的通信控制结构体
typedef enum {
    TRANSFER_DOWNLOAD = 0,
    TRANSFER_UPLOAD = 1
} TRANSFER_DIRECTION_E;
struct TRANSFER_CTRL {
    TRANSFER_DIRECTION_E direction;     //记录当前传输方向
    unsigned char block_counter;        //记录接收到的当前block计数
    unsigned char last_block_counter;   //记录上一次接收到到的block，用于和当前block计数对比
    unsigned long prog_addr;            //记录当前要写入的逻辑编程地址，当持续收到传输数据包，变量会增长
};

//用于services层使用的内部变量声明
extern unsigned char cur_session_type;
extern unsigned long pg_memory_address,pg_memory_size;
extern struct TRANSFER_CTRL cur_transfer_ctrl;


//基于SID的每种服务数据的独立处理函数声明
void sid10_diagnostic_session_ctrl_ack_solve(unsigned char *msg_buff,unsigned short msg_len);          // SID-10, 诊断会话控制
void sid11_ecu_reset_ask_solve(unsigned char *msg_buff,unsigned short msg_len);                        // SID-11，ECU复位
void sid22_read_data_by_id_ack_solve(unsigned char *msg_buff,unsigned short msg_len);                  // SID-22，根据数据ID读取数据
void sid27_security_access_ack_solve(unsigned char *msg_buff,unsigned short msg_len);                  // SID-27，安全访问
void sid31_routine_ctrl_ack_solve(unsigned char *msg_buff,unsigned short msg_len);                     // SID-31，例程控制
void sid34_request_download_ack_solve(unsigned char *msg_buff,unsigned short msg_len);                 // SID-34，请求下载 (tester -> device)
void sid36_transfer_data_ack_solve(unsigned char *msg_buff,unsigned short msg_len);                    // SID-36，传输数据

int sid10_get_ack_status(unsigned char cur_status);
void sid10_clear_status(void);
int sid22_get_ack_status(unsigned char cur_status,unsigned char **did_data,unsigned short *p_len);
void sid22_clear_status(void);
int sid27_get_ack_status(unsigned char cur_status,unsigned long *dest_seed);
void sid27_clear_status(void);
int sid31_get_ack_status(unsigned char cur_status);
void sid31_clear_status(void);
int sid34_get_ack_status(unsigned char cur_status,unsigned long *p_block_nums);
void sid34_clear_status(void);
int sid36_get_ack_status(unsigned char cur_status,unsigned char *p_block_sn, unsigned char *p_busy);
void sid36_clear_status(void);
int sid11_get_ack_status(unsigned char cur_status);
void sid11_clear_status(void);

//用于services层的公用函数
void uds_send_data_active(unsigned char *msg_buff,unsigned short len);
void uds_ack_accept(unsigned char *msg_buff,unsigned short len);
void uds_ack_refuse(unsigned char ds_type,unsigned char para);

extern uint8_t g_uds_fail_sid;
extern uint8_t g_uds_fail_nrc;
void uds_capture_fail_info(const unsigned char *msg_buff);

#endif