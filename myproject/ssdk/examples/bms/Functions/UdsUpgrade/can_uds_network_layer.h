/* ******************* 
// can uds的网络层接口 头文件
 **********************/
#ifndef __CAN_UDS_NETWORK_LAYER_H__
#define __CAN_UDS_NETWORK_LAYER_H__
#include "can_uds_arch.h"
//------宏定义
#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define ECU_MASK                    0x79014010
// error define
#define ERR_SN_WRONG                0xff    // in CF receiving, SN is wrong
#define ERR_DL_WRONG                0xfe    // in CF receiving, data len is wrong
#define ERR_RECV_END                0xfd    // CF receive all
#define ERR_SF_DL_WRR               0xfc    // SF DL wrong
#define ERR_SF_DAT_WRR              0xfb    // SF data style wrong
// end of error define

#define MAX_SF_PACK_LEN	            64      // 单帧最大长度,实际上到不了
#define MAX_MF_PACK_LEN	            1040    // 多帧最大长度
#define MAX_SEND_BUF_LEN	        1040    // 最大发送缓存

typedef enum {
    UDS_UPGRADE_IDLE                  = 0,
    UDS_UPGRADE_SWITCH_SESSION        = 1,
    UDS_UPGRADE_SWITCH_SECURITY_LEVEL = 2,
    UDS_UPGRADE_READ_DEST_INFO        = 3,
    UDS_UPGRADE_WRITE_DEST_INFO       = 4,
    UDS_UPGRADE_PRE_ACTION            = 5,
    UDS_UPGRADE_REQ_DOWNLOAD          = 6,
    UDS_UPGRADE_DOWNLOADING           = 7,
    UDS_UPGRADE_CHECK_INTEGRITY       = 8,
    UDS_UPGRADE_EXIT                  = 9,
    UDS_UPGRADE_DEST_RESET            = 10,
    UDS_UPGRADE_ERROR                 = 11,
} UPGRADE_STEP_E;

//结构体定义
//--------数据类型定义-------
struct SF_PACK// 单帧包结构
{
	unsigned short data_len;
	unsigned char data_buf[MAX_SF_PACK_LEN];
} __attribute__((aligned(4)));

struct MF_PACK// 多帧包结构
{
	unsigned short data_len;
	unsigned char data_buf[MAX_MF_PACK_LEN];
	unsigned char frame_sta;
	unsigned char cur_sn;
	unsigned short cur_recv_idx;
} __attribute__((aligned(4)));

struct UDS_SEND_BUF// 发送缓存结构
{
	unsigned char sw;// control send buf is open or close
	unsigned short len;
	unsigned char buf[MAX_SEND_BUF_LEN];
} __attribute__((aligned(4)));

struct FC_STATUS// 流控帧状态结构
{
	unsigned char fs;//flow ctrl status, {FC_FS_CTS = 0,FC_FS_WT = 1,FC_FS_OVFLW = 2}
	unsigned char bs;// baud size max
	unsigned char stmin;// skip time min
};

struct UDS_DEV_CONFIG
{
    unsigned long tx_id_type:1;
    unsigned long rx_phy_id_type:1;
    unsigned long rx_fuc_id_type:1;
    unsigned long :29;
    unsigned long tx_id;
    unsigned long rx_phy_id;
    unsigned long rx_fuc_id;
    unsigned char stmin;
    unsigned char pack_with_crc;
};

//枚举定义
enum{SF_UDS_TYPE = 0,FF_UDS_TYPE,CF_UDS_TYPE,FC_UDS_TYPE};
enum{FC_FS_CTS = 0,FC_FS_WT = 1,FC_FS_OVFLW = 2,FC_FS_UNUSED = 0xff};
enum{FRAME_ERR_SN_WRONG = 0,FRAME_ERR_LEN_WRONG};
enum{DEFAULT_SESSION = 1, PROGRAMMING_SESSION = 2,EXTENDED_SESSION =3};

//外部访问变量
extern struct UDS_DEV_CONFIG g_uds_config;

//--- 函数声明 ---
unsigned char send_uds_SF(struct SF_PACK *sf_pack);
unsigned char send_uds_FF(struct MF_PACK *mf_pack);
unsigned char send_uds_CF(struct MF_PACK *mf_pack);
unsigned char send_uds_FC(unsigned char fs,unsigned char bs,unsigned char stmin);

unsigned char check_uds_type(struct CAN_DATA *p_can_data);// return uds type
unsigned char analyze_uds_SF(struct CAN_DATA *p_can_data,struct SF_PACK *sf_pack);//return success or fail
unsigned short analyze_uds_FF(struct CAN_DATA *p_can_data,struct MF_PACK *mf_pack);// return success or fail
unsigned char analyze_uds_CF(struct CAN_DATA *p_can_data,struct MF_PACK *mf_pack);// return success or fail
unsigned char analyze_uds_FC(struct CAN_DATA *p_can_data,struct FC_STATUS *fc_sta);// return fc status 

void reset_sf_buf(struct SF_PACK *sf_pack);
void reset_mf_buf(struct MF_PACK *mf_pack);
void reset_fc_sta(struct FC_STATUS *fc_sta);
#endif