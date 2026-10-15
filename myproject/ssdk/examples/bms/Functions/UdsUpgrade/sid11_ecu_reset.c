/* ******************* 
 * can uds 11h服务的实现
 **********************/
#include <string.h>
#include "can_uds_network_layer.h"
#include "sid_internal_header.h"
#include "can_uds_services_layer.h"


static int sid_ret = DEST_ACK_WAIT;         //在没收到消息之前，都是WAIT状态   

/* 解析 0x11 ECU 复位服务的响应
 * 0x11 用于复位目标 ECU（0x01=硬复位，0x02=软复位等），
 * 升级流程中常在下载完成后用它让设备重启以进入新程序 */
void sid11_ecu_reset_ask_solve(unsigned char *msg_buff,unsigned short msg_len)
{
    if(msg_buff[0] == (ds_ecu_reset | UDS_SRV_RET_ID_OFFSET)){//如果是正响应
        if((msg_buff[1] == 0x01) || (msg_buff[1] == 0x02)){//复位类型只认可硬/软复位
            d_printf("correct 11h ack\r\n");
            sid_ret = DEST_ACK_POSITIVE;//正响应
        }else{
            d_printf("wrong 11h ack\r\n");
            sid_ret = DEST_ACK_WRONG_PARA;
        }
    }else if(msg_buff[0] == 0x7f){//如果是否定响应
        d_printf("nega 11h ack \r\n");
        sid_ret = DEST_ACK_NEGATIVE; uds_capture_fail_info(msg_buff);
    }else {//其他错误的值
        d_printf("unknow 11h ack\r\n");
        sid_ret = DEST_ACK_ERROR; g_uds_fail_sid = msg_buff[1]; g_uds_fail_nrc = 0xE2;
    }
}

/* 查询 0x11 服务的当前 ACK 状态 */
int sid11_get_ack_status(unsigned char cur_status)
{//参数返回读出的数据的数组缓存和数据长度
    return sid_ret;
}

/* 复位 0x11 服务的状态，为下一次请求做准备 */
void sid11_clear_status(void)
{
    sid_ret = DEST_ACK_WAIT;
}