/* ******************* 
 * can uds 36h服务的实现
 **********************/
#include <string.h>
#include "can_uds_network_layer.h"
#include "sid_internal_header.h"
#include "can_uds_services_layer.h"

static int sid_ret = DEST_ACK_WAIT;         //在没收到消息之前，都是WAIT状态
static unsigned char block_sn = 0;          //当前的block-sn
static unsigned char response_status = 0;           //用于判断接收方状态

/* 解析 0x36 数据传输服务的响应
 * 0x36 用于把数据块传给 ECU（每次一个 block-sn 块序号），是升级数据搬运的核心；
 * 正响应返回 block-sn 表示该块被成功接收，负响应会区分“对方忙”(需稍后重发)和
 * “单包 CRC 校验失败”(需重发该包)，tester 据此决定重传策略 */
void sid36_transfer_data_ack_solve(unsigned char *msg_buff,unsigned short msg_len)
{
    if(msg_buff[0] == (ds_transfer_data | UDS_SRV_RET_ID_OFFSET)){//如果是肯定响应
        block_sn = msg_buff[1];                                 //记录block-sn
        d_printf("correct 36h ack,sn[%d]\r\n",block_sn);
        sid_ret = DEST_ACK_POSITIVE;//肯定响应
    }else if(msg_buff[0] == 0x7f){//如果是否定响应
        if(msg_buff[2] == nrc_service_busy){
            response_status = 1;    //对方忙
            d_printf("nega 36h ack busy(%02x)\r\n",msg_buff[2]);
        }else if(msg_buff[2] == nrc_single_pack_crc_check_error){
            response_status = 2;   //单包校验失败，重发
            d_printf("nega 36h ack single_pack_crc_check_error(%02x)\r\n",msg_buff[2]);
        }else{//其他的否定响应打印相关否定响应码
            d_printf("nega 36h ack (%02x)\r\n",msg_buff[2]);
        }
        sid_ret = DEST_ACK_NEGATIVE; uds_capture_fail_info(msg_buff);//否定响应
    }else {//其他错误的值
        d_printf("unknow 36h ack\r\n");
        sid_ret = DEST_ACK_ERROR; g_uds_fail_sid = msg_buff[1]; g_uds_fail_nrc = 0xE2;
    }
}

/* 查询 0x36 服务的 ACK 状态，并通过出参回传本次 block-sn 与接收方状态标志 */
int sid36_get_ack_status(unsigned char cur_status,unsigned char *p_block_sn, unsigned char *p_status)
{//参数返回读出的数据的数组缓存和数据长度
    *p_block_sn = block_sn;
    *p_status = response_status;
    return sid_ret;
}

/* 复位 0x36 服务的状态（含 block-sn 与接收方状态标志），为下一块传输做准备 */
void sid36_clear_status(void)
{
    sid_ret = DEST_ACK_WAIT;
    block_sn = 0;
    response_status = 0;
}