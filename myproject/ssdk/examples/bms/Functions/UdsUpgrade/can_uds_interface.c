/* ******************* 
// can uds对外的接口实现
 **********************/
#include "can_uds_interface.h"
#include "can_uds_network_layer.h"
#include "can_uds_services_layer.h"
#include "can_uds_arch.h"
#include "DEVCAN.h"
#include <string.h>
#include <debug.h>

extern void uds_upgrade_proc(void);

static uint8_t g_uds_upgrade_active = 0;

/* 设置升级流程的激活标志：active=1 时才允许路由 CAN 帧进入 UDS 协议栈，
 * 用于在非升级阶段屏蔽诊断报文，避免误触发升级逻辑 */
void uds_set_upgrade_active(uint8_t active)
{
    g_uds_upgrade_active = active;
    ssdk_printf(SSDK_DEBUG, "[UDSIF] upgrade_active=%d\r\n", active);
}

/* CAN 接收帧的入口路由：把底层 CAN 驱动收到的 frame 转成 UDS 内部数据格式后投递给协议栈
 * 仅在升级激活且帧指针有效时处理，是“对外接口层”接收方向的唯一入口 */
void uds_can_route_frame(e_CANNUM canNum, can_frame_t *frame)
{
    struct CAN_DATA can_data;

    if (!g_uds_upgrade_active || frame == NULL)
        return;

    memset(&can_data, 0, sizeof(struct CAN_DATA));
    can_data.idt.id = frame->id;
    can_data.idt.type = (frame->ide == CAN_EXTID) ? 1 : 0;//扩展帧记为1，标准帧记为0
    can_data.idt.port = (unsigned char)canNum;
    can_data.dlc = (unsigned char)frame->len;
    memcpy(can_data.data, frame->data, (frame->len > 64) ? 64 : frame->len);//防止超过缓冲长度

    uds_recv_data(&can_data);
}


// --- 内部变量 ---

// --- 内部函数声明 ---

// --- 内部函数定义 --- 


// --- 外部函数定义 ---
extern int uds_buf_alloc(void);

/* UDS 协议栈初始化：分配内部缓冲区并配置本端/对端 CAN ID、时序参数
 * 其中本端为被测设备（tx_id=LOCAL_DEVICE_CAN_ID），对端为 tester（rx=REMOTE_TESTER_CAN_ID） */
void init_uds(void)
{
    if (!uds_buf_alloc())
        return;
    g_uds_config.tx_id = LOCAL_DEVICE_CAN_ID;
    g_uds_config.tx_id_type = 1;//扩展帧
    g_uds_config.rx_phy_id = REMOTE_TESTER_CAN_ID;
    g_uds_config.rx_phy_id_type = 1;//扩展帧
    g_uds_config.rx_fuc_id = REMOTE_TESTER_CAN_ID;
    g_uds_config.rx_fuc_id_type = 1;//扩展帧
    g_uds_config.stmin = 1;//设置stmin为5ms
    g_uds_config.pack_with_crc = 0;//默认传输包不带crc
	reset_uds_send_buf();
}

/* 设备端协议栈周期处理：仅做物理层的收发调度（收一帧、发一帧），不包含升级业务 */
void uds_proc_device(void)
{
    uds_phy_recv(0,FALSE);
    uds_phy_send(FALSE);
}

/* tester 端协议栈周期处理：在收发调度之外额外驱动升级主流程状态机（uds_upgrade_proc），
 * 是升级业务的实际推进入口 */
void uds_proc_tester(void)
{
    uds_phy_recv(0, FALSE);
    uds_phy_send(FALSE);
    uds_upgrade_proc();
}

/* 接收数据的统一入口：把外部传入的 CAN 数据交给物理层接收处理 */
void uds_recv_data(struct CAN_DATA *p_can_data)
{
    uds_phy_recv(p_can_data,FALSE);
}