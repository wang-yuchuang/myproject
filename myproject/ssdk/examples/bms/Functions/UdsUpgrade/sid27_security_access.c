/* ******************* 
 * can uds 27h服务的实现
 **********************/
#include <string.h>
#include "can_uds_network_layer.h"
#include "sid_internal_header.h"
#include "can_uds_services_layer.h"


//--- 内部结构体定义 ---


//--- 内部变量定义 ---
struct SECURITY_CTRL security_ctrl = {
    .level = SECURITY_LEVEL0,
    .seed[0] = 0,
    .seed[1] = 0,
    .seed[2] = 0,
    .seed[3] = 0,
};
static int sid_ret = DEST_ACK_WAIT;//在没收到消息之前，都是WAIT状态
static unsigned long recv_seed = 0;

// 肯定响应码
// byte0   -  67H
// byte1   -  0x01(request seed) or 0x02(send key)
// byte2~5 -  request seed：内容为返回的seed值（4字节）
//            send key: 内容为本次计算用的seed值（即上一次request-seed的值），不关心
/* 解析 0x27 安全访问服务的响应
 * 0x27 用于安全解锁（先请求 seed 再回送 key），升级流程在进入编程会话后、擦除下载前必须解锁：
 * request seed 正响应携带 4 字节 seed，tester 据此算出 key 后以 send key 发给 ECU */
void sid27_security_access_ack_solve(unsigned char *msg_buff,unsigned short msg_len)
{
    if(msg_buff[0] == (ds_security_access | UDS_SRV_RET_ID_OFFSET)){//如果是正响应
        if(msg_buff[1] == SECURITY_REQ_SEED || 
           msg_buff[1] == SECURITY_SEND_KEY){
            d_printf("correct 27h ack\r\n");
            recv_seed  = (msg_buff[2]);    //LSB，小端表示
            recv_seed += (msg_buff[3]<<8);
            recv_seed += (msg_buff[4]<<16);
            recv_seed += (msg_buff[5]<<24);//MSB
            d_printf("correct 27h ack\r\n");
            sid_ret = DEST_ACK_POSITIVE;//正响应
        }else{
            d_printf("wrong 27h ack %d\r\n",__LINE__);
            sid_ret = DEST_ACK_WRONG_PARA;//错误的响应值
        }
    }else if(msg_buff[0] == 0x7f){//如果是否定响应
        sid_ret = DEST_ACK_NEGATIVE; uds_capture_fail_info(msg_buff);
        d_printf("nega 27h ack (%02x)\r\n",msg_buff[2]);
    }else {//其他错误的值
        d_printf("unknow 27h ack\r\n");
        sid_ret = DEST_ACK_ERROR; g_uds_fail_sid = msg_buff[1]; g_uds_fail_nrc = 0xE2;
    }
}

/* 查询 0x27 服务的 ACK 状态，并通过出参回传解析出的 seed 值（供上层计算 key） */
int sid27_get_ack_status(unsigned char cur_status,unsigned long *dest_seed)
{
    *dest_seed = recv_seed;
    return sid_ret;
}

/* 复位 0x27 服务的状态，为下一次安全访问流程做准备 */
void sid27_clear_status(void)
{
    sid_ret = DEST_ACK_WAIT;
}