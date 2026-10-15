/* ******************* 
 * can uds 22h服务的实现
 **********************/
#include <string.h>
#include "can_uds_network_layer.h"
#include "sid_internal_header.h"
#include "can_uds_services_layer.h"

#define MAX_DID_INFO_LENGTH         8     //最大DID单条信息默认为128字节

static int sid_ret = DEST_ACK_WAIT;//在没收到消息之前，都是WAIT状态
static unsigned char did_read_buff[MAX_DID_INFO_LENGTH] = {0};
static unsigned short did_read_len = 0;

/* 解析 0x22 按 ID 读数据服务的响应
 * 0x22 用于按 DID 读取 ECU 数据；升级流程中用 app_sw_id（应用软件版本 DID）
 * 读取目标版本号以确认待升级设备，正响应的数据会缓存到 did_read_buff */
void sid22_read_data_by_id_ack_solve(unsigned char *msg_buff,unsigned short msg_len)
{//
    int i = 0;
    unsigned short read_did = 0;
    if(msg_buff[0] == (ds_read_data_by_id | UDS_SRV_RET_ID_OFFSET)){//如果是正响应
        read_did = msg_buff[1];
        read_did += (unsigned short)(msg_buff[2] << 8);//DID 为小端：低字节在前，高字节在后
        if(read_did == app_sw_id)
        {
         memset(did_read_buff,0,MAX_DID_INFO_LENGTH);
        for(i=0;i<(msg_len-3);i++){//读出剩下的数据，存入到DID读出缓存里
            if(i >= (MAX_DID_INFO_LENGTH-1))//如果超过缓存限制，截断
                break;
            did_read_buff[i] = msg_buff[i+3];
        }
        did_read_len = i;//记录本次读到的长度
        d_printf("correct 22h ack\r\n");
        sid_ret = DEST_ACK_POSITIVE;//正响应
        }else{
            d_printf("wrong 22h ack %d\r\n",__LINE__);
            sid_ret = DEST_ACK_WRONG_PARA;//响应值不支持
        }
    }else if(msg_buff[0] == 0x7f){//如果是否定响应
        d_printf("nega 22h ack (%02x)\r\n",msg_buff[2]);
        sid_ret = DEST_ACK_NEGATIVE; uds_capture_fail_info(msg_buff);
    }else {//其他错误的值
        d_printf("unknow 22h ack\r\n");
        sid_ret = DEST_ACK_ERROR; g_uds_fail_sid = msg_buff[1]; g_uds_fail_nrc = 0xE2;
    }
}

/* 查询 0x22 服务的 ACK 状态，同时通过出参回传读到的 DID 数据缓存和长度 */
int sid22_get_ack_status(unsigned char cur_status,unsigned char **did_data,unsigned short *p_len)
{//参数返回读出的数据的数组缓存和数据长度
    *did_data = did_read_buff;
    *p_len = did_read_len;
    return sid_ret;
}

/* 复位 0x22 服务的状态并清空 DID 数据缓存，为下一次读数据做准备 */
void sid22_clear_status(void)
{
    sid_ret = DEST_ACK_WAIT;
    memset(did_read_buff,0,MAX_DID_INFO_LENGTH);
}