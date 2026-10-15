/* ******************* 
 * can uds 10h服务的实现
 **********************/
#include <string.h>
#include "can_uds_network_layer.h"
#include "sid_internal_header.h"
#include "can_uds_services_layer.h"


static int sid_ret = DEST_ACK_WAIT;//在没收到消息之前，都是WAIT状态

/* 解析 0x10 诊断会话控制服务的响应
 * 0x10 用于切换 ECU 会话模式（默认/编程/扩展会话），是升级流程的第一步：
 * 必须先进入编程会话(PROGRAMMING_SESSION)才能执行后续的擦除与下载
 * 正响应返回请求的会话类型；负响应(0x7F)记录失败信息；其他为协议错误 */
void sid10_diagnostic_session_ctrl_ack_solve(unsigned char *msg_buff,unsigned short msg_len)
{
    if(msg_buff[0] == (ds_diagnostic_session_ctrl | UDS_SRV_RET_ID_OFFSET)){//如果是正响应
        if(msg_buff[1] == DEFAULT_SESSION || 
           msg_buff[1] == PROGRAMMING_SESSION || 
           msg_buff[1] == EXTENDED_SESSION){
            d_printf("correct 10h ack\r\n");
            sid_ret = DEST_ACK_POSITIVE;//正响应
        }else{
            d_printf("wrong 10h ack %d\r\n",__LINE__);
            sid_ret = DEST_ACK_WRONG_PARA;//响应值不支持
        }
    }else if(msg_buff[0] == 0x7f){//如果是否定响应
        sid_ret = DEST_ACK_NEGATIVE; uds_capture_fail_info(msg_buff);
        d_printf("nega 10h ack (%02x)\r\n",msg_buff[2]);
    }else {//其他错误的值
        d_printf("unknow 10h ack\r\n");
        sid_ret = DEST_ACK_ERROR; g_uds_fail_sid = msg_buff[1]; g_uds_fail_nrc = 0xE2;
    }
}

/* 查询 0x10 服务的当前 ACK 状态，供升级状态机判断本次请求的响应结果 */
int sid10_get_ack_status(unsigned char cur_status)
{
    return sid_ret;
}

/* 复位 0x10 服务的状态，为下一次请求做准备 */
void sid10_clear_status(void)
{
    sid_ret = DEST_ACK_WAIT;
}