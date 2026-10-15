/* ******************* 
 * can uds 31h服务的实现
 **********************/
#include <string.h>
#include "can_uds_network_layer.h"
#include "sid_internal_header.h"
#include "can_uds_services_layer.h"


static int sid_ret = DEST_ACK_WAIT;         //在没收到消息之前，都是WAIT状态   

/* 解析 0x31 例程控制服务的响应
 * 0x31 用于启动/停止/请求 ECU 执行特定例程（如擦除、校验等自定义例程），
 * 升级流程中可用它触发目标设备执行擦除或完整性校验例程 */
void sid31_routine_ctrl_ack_solve(unsigned char *msg_buff,unsigned short msg_len)
{
    if(msg_buff[0] == (ds_routine_ctrl | UDS_SRV_RET_ID_OFFSET)){//如果是正响应
        if((msg_buff[1] == 0x01) || (msg_buff[1] == 0x02)){
            d_printf("correct 31h ack\r\n");
            sid_ret = DEST_ACK_POSITIVE;//正响应
        }else{
            d_printf("wrong 31h ack\r\n");
            sid_ret = DEST_ACK_WRONG_PARA;
        }
    }else if(msg_buff[0] == 0x7f){//如果是否定响应
        d_printf("nega 31h ack (%02x)\r\n",msg_buff[2]);
        sid_ret = DEST_ACK_NEGATIVE; uds_capture_fail_info(msg_buff);
    }else {//其他错误的值
        d_printf("unknow 31h ack\r\n");
        sid_ret = DEST_ACK_ERROR; g_uds_fail_sid = msg_buff[1]; g_uds_fail_nrc = 0xE2;
    }
}

/* 查询 0x31 服务的当前 ACK 状态 */
int sid31_get_ack_status(unsigned char cur_status)
{//参数返回读出的数据的数组缓存和数据长度
    return sid_ret;
}

/* 复位 0x31 服务的状态，为下一次请求做准备 */
void sid31_clear_status(void)
{
    sid_ret = DEST_ACK_WAIT;
}