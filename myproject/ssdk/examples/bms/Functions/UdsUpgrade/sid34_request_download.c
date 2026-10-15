/* ******************* 
 * can uds 34h服务的实现
 **********************/
#include <string.h>
#include "can_uds_network_layer.h"
#include "sid_internal_header.h"
#include "can_uds_services_layer.h"

/// @brief 
struct TRANSFER_CTRL cur_transfer_ctrl = {
    .direction = TRANSFER_DOWNLOAD,
    .block_counter = 0,
};
static int sid_ret = DEST_ACK_WAIT;         //在没收到消息之前，都是WAIT状态
//static unsigned char length_width = 0;      //block_nums的数据域的宽度(单位为字节-占用几个字节)，这里默认为占用2字节
static unsigned long max_bytes_in_block = 0;//最大支持的包总数

/// @brief SID34 请求下载
/// @param msg_buff 
/// @param msg_len 
/* 解析 0x34 请求下载服务的响应
 * 0x34 用于向 ECU 请求开始一次数据下载，正响应会返回后续每个传输块允许的最大字节数
 * （max_bytes_in_block），tester 据此确定 0x36 每次传输的数据块大小 */
void sid34_request_download_ack_solve(unsigned char *msg_buff,unsigned short msg_len)
{
    if(msg_buff[0] == (ds_request_download | UDS_SRV_RET_ID_OFFSET)){//如果是正响应
        //length_width = msg_buff[1];     //这里不做解析，默认为2字节，最大64K单包
        if(msg_buff[1] == 0x20){//长度格式字节应为 0x20（2 字节长度域）
            max_bytes_in_block = (unsigned long)(msg_buff[2]<<8);   //MSB
            max_bytes_in_block += msg_buff[3];                      //LSB
            d_printf("correct 34h ack\r\n");
            sid_ret = DEST_ACK_POSITIVE;//正响应
        }else{
             d_printf("wrong 34h ack\r\n");
            sid_ret = DEST_ACK_WRONG_PARA;//负响应
        }

    }else if(msg_buff[0] == 0x7f){//如果是否定响应
        d_printf("nega 34h ack (%02x)\r\n",msg_buff[2]);
        sid_ret = DEST_ACK_NEGATIVE; uds_capture_fail_info(msg_buff);
    }else {//其他错误的值
        d_printf("unknow 34h ack\r\n");
        sid_ret = DEST_ACK_ERROR; g_uds_fail_sid = msg_buff[1]; g_uds_fail_nrc = 0xE2;
    }
}

/* 查询 0x34 服务的 ACK 状态，并通过出参回传每块最大字节数 */
int sid34_get_ack_status(unsigned char cur_status,unsigned long *p_block_nums)
{//参数返回读出的数据的数组缓存和数据长度
    *p_block_nums = (unsigned long)max_bytes_in_block;
    return sid_ret;
}

/* 复位 0x34 服务的状态并清零最大块长度，为下一次下载请求做准备 */
void sid34_clear_status(void)
{
    sid_ret = DEST_ACK_WAIT;
    max_bytes_in_block = 0;
}