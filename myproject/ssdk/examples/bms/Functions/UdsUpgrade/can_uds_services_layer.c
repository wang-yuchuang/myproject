/* ******************* 
 * can uds
 **********************/
#include <string.h>
#include <stdio.h>
#include "can_uds_network_layer.h"
#include "can_uds_services_layer.h"
#include "sid_internal_header.h"


enum{
	e2rom_app_sw_id_str = 0,
	e2rom_active_diag_session_id_str,
	e2rom_vehicle_spare_num_id_str,
	e2rom_vehicle_ecu_num_id_str,
	e2rom_vehicle_ecu_sw_ver_id_str,
	e2rom_sys_supplier_id_str,
	e2rom_ecu_manufacturing_id_str,
	e2rom_ecu_serial_num_id_str,
	e2rom_vin_id_str,
	e2rom_vehicle_ecu_hw_num_id_str,
	e2rom_sys_supplier_ecu_hw_id_str,
	e2rom_ecu_hw_ver_str,
	e2rom_sys_supplier_ecu_sw_num_id_str,
	e2rom_sys_supplier_ecu_sw_ver_id_str,
	e2rom_sys_name_or_engine_type_id_str,
	e2rom_ecu_install_date_id_str
};

#define RECV_DATA_COMMING_OVERTIME      500





// --- 
//static unsigned char phy_func_mode = 0;//0 is phy mode, 1 is func mode
unsigned char cur_session_type = DEFAULT_SESSION;
static struct UDS_SEND_BUF *uds_send_buf = NULL;
static struct SF_PACK *single_pack_send_buf = NULL;
static struct MF_PACK *multi_pack_send_buf = NULL;
struct SF_PACK *single_pack_recv_buf = NULL;
struct MF_PACK *multi_pack_recv_buf = NULL;

void uds_buf_free(void);

/*
 * uds_buf_alloc - 分配UDS收发所需的各类缓冲区并清零
 * 说明：为发送缓存(uds_send_buf)、单帧发送/接收包、多帧发送/接收包各自申请
 *       内存；任一分配失败即返回FALSE。发送缓存暂存上层待发的完整服务请求，
 *       单帧/多帧包则分别承载ISO-TP分段前后的数据。
 */
int uds_buf_alloc(void)
{
    extern void *ExtMalloc(size_t);
    extern void ExtFree(void *);

    uds_send_buf = (struct UDS_SEND_BUF *)ExtMalloc(sizeof(struct UDS_SEND_BUF));
    if (uds_send_buf == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc uds_send_buf fail\r\n", __func__);
        uds_buf_free();
        return FALSE;
    }
    else
    {
        memset(uds_send_buf, 0, sizeof(struct UDS_SEND_BUF));
        printf("malloc uds_send_buf %d success\r\n", (int)sizeof(struct UDS_SEND_BUF));
    }

    single_pack_send_buf = (struct SF_PACK *)ExtMalloc(sizeof(struct SF_PACK));
    if (single_pack_send_buf == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc single_pack_send_buf fail\r\n", __func__);
        uds_buf_free();
        return FALSE;
    }
    else
    {
        memset(single_pack_send_buf, 0, sizeof(struct SF_PACK));
        printf("malloc single_pack_send_buf %d success\r\n", (int)sizeof(struct SF_PACK));
    }

    multi_pack_send_buf = (struct MF_PACK *)ExtMalloc(sizeof(struct MF_PACK));
    if (multi_pack_send_buf == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc multi_pack_send_buf fail\r\n", __func__);
        uds_buf_free();
        return FALSE;
    }
    else
    {
        memset(multi_pack_send_buf, 0, sizeof(struct MF_PACK));
        printf("malloc multi_pack_send_buf %d success\r\n", (int)sizeof(struct MF_PACK));
    }

    single_pack_recv_buf = (struct SF_PACK *)ExtMalloc(sizeof(struct SF_PACK));
    if (single_pack_recv_buf == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc single_pack_recv_buf fail\r\n", __func__);
        uds_buf_free();
        return FALSE;
    }
    else
    {
        memset(single_pack_recv_buf, 0, sizeof(struct SF_PACK));
        printf("malloc single_pack_recv_buf %d success\r\n", (int)sizeof(struct SF_PACK));
    }

    multi_pack_recv_buf = (struct MF_PACK *)ExtMalloc(sizeof(struct MF_PACK));
    if (multi_pack_recv_buf == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc multi_pack_recv_buf fail\r\n", __func__);
        uds_buf_free();
        return FALSE;
    }
    else
    {
        memset(multi_pack_recv_buf, 0, sizeof(struct MF_PACK));
        printf("malloc multi_pack_recv_buf %d success\r\n", (int)sizeof(struct MF_PACK));
    }

    return TRUE;
}

uint8_t g_uds_fail_sid = 0;
uint8_t g_uds_fail_nrc = 0;
unsigned long g_uds_last_recv_tick = 0;

/*
 * uds_capture_fail_info - 捕获否定响应(0x7F)中的SID和NRC
 * 说明：仅当报文首字节为0x7F(否定响应)时记录，msg_buff[1]为被拒绝的服务ID、
 *       msg_buff[2]为否定响应码(NRC)，供上层查询失败原因。
 */
void uds_capture_fail_info(const unsigned char *msg_buff)
{
    if (msg_buff && msg_buff[0] == 0x7F) {
        g_uds_fail_sid = msg_buff[1];
        g_uds_fail_nrc = msg_buff[2];
    }
}

/*
 * uds_buf_free - 释放uds_buf_alloc申请的全部缓冲区并置空指针
 * 说明：逐个判断指针非空后释放，避免重复释放或产生野指针。
 */
void uds_buf_free(void)
{
    extern void ExtFree(void *);
    if (uds_send_buf)        { ExtFree(uds_send_buf);        uds_send_buf = NULL; }
    if (single_pack_send_buf){ ExtFree(single_pack_send_buf); single_pack_send_buf = NULL; }
    if (multi_pack_send_buf) { ExtFree(multi_pack_send_buf); multi_pack_send_buf = NULL; }
    if (single_pack_recv_buf){ ExtFree(single_pack_recv_buf); single_pack_recv_buf = NULL; }
    if (multi_pack_recv_buf) { ExtFree(multi_pack_recv_buf); multi_pack_recv_buf = NULL; }
}
static struct FC_STATUS cur_fc_sta;
unsigned char cur_communication_type = COMMU_EN_RX_TX;
unsigned char dtc_update_flg = 0;
unsigned char uds_busy_flag = FALSE;
unsigned short dtc_count = 0;
unsigned long pg_memory_address = 0,pg_memory_size = 0;

void uds_start_send(void);
#if 0 //以下四个函数不区分单帧还是多帧，全部统一处理
void uds_solve_sf_data(struct SF_PACK *sf_pack);
void uds_solve_sf_error(struct SF_PACK *sf_pack,unsigned char err_type);
void uds_solve_mf_data(struct MF_PACK *mf_pack);
void uds_solve_mf_error(struct MF_PACK *mf_pack,unsigned char err_type);
#else
void uds_solve_recv_data(unsigned char *buff, unsigned short len);
void uds_solve_recv_error(unsigned char *buff, unsigned char err_type);
#endif
static void cpy_send_to_sfpack(void);
static void cpy_send_to_mfpack(void);



// --- 函数定义 ---
/*
 * reset_uds_send_buf - 复位发送缓存状态
 * 说明：关闭发送开关sw、清空首字节与长度，使该缓存可被下一次请求复用。
 */
void reset_uds_send_buf(void)
{
    uds_send_buf->sw = FALSE;
    uds_send_buf->buf[0] = 0;
    uds_send_buf->len = 0;
}

/*
 * cpy_send_to_sfpack - 把发送缓存内容拷贝到单帧包
 * 说明：单帧有效载荷上限CAN-FD为62字节、普通CAN为7字节(首字节需留给类型+长度位)，
 *       超出上限部分截断，仅按max_len字节拷贝。
 */
void cpy_send_to_sfpack(void)
{
	unsigned short i = 0;
    unsigned short max_len = 0;
    if(uds_is_canfd()){
        max_len = 62;
    }else{
        max_len = 7;
    }
	if(uds_send_buf->len > max_len)
		single_pack_send_buf->data_len = max_len;
	else
		single_pack_send_buf->data_len = uds_send_buf->len;
	for(i = 0; i <single_pack_send_buf->data_len; i++)
	{
		single_pack_send_buf->data_buf[i] = uds_send_buf->buf[i];
	}
}

/*
 * cpy_send_to_mfpack - 把发送缓存内容拷贝到多帧包
 * 说明：多帧包数据上限为MAX_MF_PACK_LEN(1040)，超出截断，其余按长度整体拷贝。
 */
void cpy_send_to_mfpack(void)
{
	unsigned short i = 0;

	if(uds_send_buf->len > MAX_MF_PACK_LEN)
		multi_pack_send_buf->data_len = MAX_MF_PACK_LEN;
	else
		multi_pack_send_buf->data_len = uds_send_buf->len;
	for(i = 0; i <multi_pack_send_buf->data_len; i++)
	{
		multi_pack_send_buf->data_buf[i] = uds_send_buf->buf[i];
	}
}

/*
 * uds_start_send - 打开发送开关，通知发送状态机有数据待发
 * 说明：仅置位sw标志，实际的单帧/多帧发送由uds_phy_send轮询状态机完成。
 */
void uds_start_send(void)
{
	uds_send_buf->sw = TRUE;
}

/*
 * uds_solve_recv_data - 服务层核心分发：按SID把收到的响应派发给对应处理函数
 * 说明：tester端收到的报文要么是肯定响应(首字节=请求SID+0x40)，要么是否定响应
 *       (首字节=0x7F)。先归一化出原始服务类型srv_type，再switch分发到各
 *       sidXX_*_ack_solve函数；不支持的SID(如2E/19/14/2F/35)直接丢弃。
 */
void uds_solve_recv_data(unsigned char *buff, unsigned short len)
{//tester端需要判断接收的数据是否为正负响应
    unsigned char srv_type = buff[0];
    if(buff[0] == 0x7f){//为否定响应
        srv_type = buff[1];//取第一字节为服务类别判断
    }else{//为肯定响应，减去40H得到原来的服务类型
        srv_type = buff[0] - UDS_SRV_RET_ID_OFFSET;
    }
    
    switch(srv_type)
    {// pack 0 is diagnostic service 's id
    case ds_diagnostic_session_ctrl://支持,SID10
        sid10_diagnostic_session_ctrl_ack_solve(&buff[0],len);
        break;
    case ds_security_access://支持,SID27
        sid27_security_access_ack_solve(&buff[0],len);
        break;
    case ds_tester_present://支持,SID3E
        break;
    case ds_ecu_reset://支持,SID11
        sid11_ecu_reset_ask_solve(&buff[0],len);
        break;
    case ds_communication_ctrl://支持,SID28
        break;
    case ds_ctrl_dtc_setting://不支持,SID85
		break;
    case ds_read_data_by_id://支持,SID22
        sid22_read_data_by_id_ack_solve(&buff[0],len);
        break;
    case ds_write_data_by_id://支持,SID2E
        break;
    case ds_read_dtc_info://不支持,SID14
        break;
    case ds_clr_diagnostic_info://不支持,SID14
        break;
    case ds_io_ctrl_by_id://不支持,SID2F
        break;
    case ds_routine_ctrl://支持,SID31
        sid31_routine_ctrl_ack_solve(&buff[0],len);
        break;
    case ds_request_download://支持,SID34
        sid34_request_download_ack_solve(&buff[0],len);
        break;
    case ds_request_upload://不支持,SID35
        break;
    case ds_transfer_data://支持,SID36
        sid36_transfer_data_ack_solve(&buff[0],len);
        break;
    case ds_request_transferexit://支持,SID37
        break;
	}
}

/*
 * uds_solve_recv_error - 处理接收阶段的错误
 * 说明：目前仅识别SF长度错误(ERR_SF_DL_WRR)与数据格式错误(ERR_SF_DAT_WRR)，
 *       对应NRC 0x13(报文长度/格式非法)；具体的拒绝应答逻辑已注释未启用。
 */
void uds_solve_recv_error(unsigned char *buff, unsigned char err_type)
{
    if(err_type == ERR_SF_DL_WRR || 
       err_type == ERR_SF_DAT_WRR){// send len or data's style wrong  0x13
        //uds_ack_refuse(buff[0],nrc_invalid_message_lenth_or_format);
    }
}

/*
 * uds_send_data_active - 上层主动发起一条UDS请求的入口
 * 说明：把完整请求报文拷入发送缓存并启动发送状态机。此处清零
 *       g_uds_can_tx_fail_count，标志一次新请求边界开始，之后发送失败
 *       计数从0重新累计(用于上层判断重试/失败次数)。
 */
void uds_send_data_active(unsigned char *msg_buff,unsigned short len)
{
    g_uds_can_tx_fail_count = 0;   /* reset at each new request boundary */
    uds_send_buf->len = len;
    memcpy(&uds_send_buf->buf[0],&msg_buff[0],len);
    uds_start_send();
}

/*
 * uds_phy_recv - 物理层接收状态机：处理SF/FF/CF/FC四类报文
 * 说明：sta=0为初始/空闲态，收到FF后回FC(CTS)并转入sta=1等待连续帧；
 *       sta=1期间若超过RECV_DATA_COMMING_OVERTIME(500ms)无后续帧则超时复位。
 *       每收到合法帧刷新g_uds_last_recv_tick，供上层做整体超时判断。
 */
void uds_phy_recv(struct CAN_DATA *p_can_data,unsigned char reset)
{
    static unsigned char sta = 0;
    static unsigned long tick = 0;
    unsigned char frame_type = 0;
    unsigned char tmp_re = 0;

    if(reset == TRUE)
    {
        sta = 0;
        tick = 0;
        uds_busy_flag = FALSE;
    }
    switch(sta)
    {
    case 0:
        if(p_can_data == 0)//如果无数据输入,不做处理
            break;
        frame_type = check_uds_type(p_can_data);
        if (frame_type <= FC_UDS_TYPE) {
            g_uds_last_recv_tick = get_timer_tick();
        }
        if(frame_type == SF_UDS_TYPE){// solve
            tmp_re = analyze_uds_SF(p_can_data, single_pack_recv_buf);
            if(tmp_re == 0xff)
                uds_solve_recv_error(single_pack_recv_buf->data_buf,ERR_SF_DL_WRR);
            else
                uds_solve_recv_data(single_pack_recv_buf->data_buf,single_pack_recv_buf->data_len);
            uds_busy_flag = FALSE;
        }else if(frame_type == FF_UDS_TYPE){// first frame
            uds_busy_flag = TRUE;
            tmp_re = analyze_uds_FF(p_can_data,multi_pack_recv_buf);
            //send FC
            send_uds_FC(FC_FS_CTS,0,g_uds_config.stmin);// continue send , there are no cf,STmin=50ms
            tick = get_timer_tick();
            sta = 1;// 首帧已解析并回FC(CTS)，切到case 1等待连续帧
        }else if(frame_type == CF_UDS_TYPE){// error
            uds_busy_flag = FALSE;
        }else if(frame_type == FC_UDS_TYPE){// if receive FC, update fc status
            analyze_uds_FC(p_can_data, &cur_fc_sta);
        }
        break;
    case 1:
        if((unsigned long)(get_timer_tick() - tick) > RECV_DATA_COMMING_OVERTIME){// if over 500ms,there is no data comming
            sta = 0;
            uds_busy_flag = FALSE;// 
        }else{
            if(p_can_data == 0)//如果无数据输入,不做处理
                break;
            frame_type = check_uds_type(p_can_data);
            if (frame_type <= FC_UDS_TYPE) {
                g_uds_last_recv_tick = get_timer_tick();
            }
            if(frame_type == SF_UDS_TYPE){// if it is single frame ,solve it ,  but this maybe some confuse ?????
                tmp_re = analyze_uds_SF(p_can_data, single_pack_recv_buf);
                if(tmp_re == 0xff)
                    //uds_solve_sf_error(single_pack_recv_buf,ERR_SF_DL_WRR);
                    uds_solve_recv_error(single_pack_recv_buf->data_buf,ERR_SF_DL_WRR);
                else
                    //uds_solve_sf_data(single_pack_recv_buf);// solve data
                    uds_solve_recv_data(single_pack_recv_buf->data_buf,single_pack_recv_buf->data_len);
            }else if(frame_type == CF_UDS_TYPE){
                tmp_re = analyze_uds_CF(p_can_data,multi_pack_recv_buf);// check  SN 
                if(tmp_re == ERR_SN_WRONG){// SN wrong
                    sta = 0;
                    uds_busy_flag = FALSE;// SN错误：回到case 0，本次多帧接收作废
                    //### solve sn wrong###
                    //### need add code ####
                    // N_WRONG_SN N_UDSData.indication
                    //uds_solve_mf_error(multi_pack_recv_buf,ERR_SN_WRONG);
                    uds_solve_recv_error(multi_pack_recv_buf->data_buf,ERR_SN_WRONG);
                }else if(tmp_re == ERR_RECV_END){// all data has been received
                    //uds_solve_mf_data(multi_pack_recv_buf);
                    d_printf("slove CF data: %02x\r\n",multi_pack_recv_buf->data_buf[0]);
                    uds_solve_recv_data(multi_pack_recv_buf->data_buf,multi_pack_recv_buf->data_len);
                    sta = 0;
                    uds_busy_flag = FALSE;// 多帧收齐：分发数据后回到case 0
                }else{// SN is right
                    tick = get_timer_tick();// update timer
                }
            }
        }
        break;
    }
}

/*
 * uds_phy_send - 物理层发送状态机：单帧直发，多帧按 FF -> 等FC -> CF 流程
 * 说明：sta=0判断发送缓存，数据<=max_len走单帧SF，否则发首帧FF并进入sta=1等待
 *       对端FC；sta=1解析FC(CTS继续/WT等待/OVFLW溢出)，并按STmin计算连续帧间隔；
 *       sta=2/3按STmin节奏发送CF、递增SN(0x0F回绕为0)，直到全部发完回到sta=0。
 */
void uds_phy_send(unsigned char reset)
{
    static unsigned char sta = 0;
    static unsigned long tick = 0; 
    static unsigned long delay_tick = 0;
    unsigned short max_len = 0;
    if(reset == TRUE){
        sta = 0;
        tick = 0;
        reset_uds_send_buf();
    }
    if(uds_is_canfd()){//单帧有效数据字节为(64-2=62)
        max_len = 62;
    }else{
        max_len = 7;
    }
    switch(sta)
    {
    case 0:
        if(uds_send_buf->sw == TRUE){// only if uds_send_buf have data and open , just send 
            if(uds_send_buf->len <= max_len){// send single buff
                cpy_send_to_sfpack();// copy send_buf to sf_pack
                send_uds_SF(single_pack_send_buf);
                reset_uds_send_buf();// if send over , reset send_buf
            }else{// multi buff
                cpy_send_to_mfpack();// copy send_buf to mf_pack
                send_uds_FF(multi_pack_send_buf);
                if(uds_is_canfd()){
                    multi_pack_send_buf->cur_recv_idx = 58;
                }else{
                    multi_pack_send_buf->cur_recv_idx = 6;
                }
                tick = get_timer_tick();
                sta = 1;// 首帧已发，进入case 1等待对端FC
                reset_fc_sta(&cur_fc_sta);//reset fc sta
            }
        }
        break;
    case 1:// wait for FC
        if((unsigned long)(get_timer_tick() - tick) > 500){// if over 500ms, the FC is no used,standard is 150ms
            sta = 0;
            reset_uds_send_buf();
        }else if(cur_fc_sta.fs != FC_FS_UNUSED){// there is a new FC comming
            if(cur_fc_sta.fs == FC_FS_CTS){// continue
                sta = 2;
                //STmin编码:0x00-0x7F为毫秒值,0xF1-0xF9为百微秒(0.1-0.9ms),其余非法
                if(cur_fc_sta.stmin <= 0x7f){
                    tick = get_timer_tick();
                    delay_tick = (cur_fc_sta.stmin>0)?cur_fc_sta.stmin:1;//直接指定STmin至少为1ms
                }else if(cur_fc_sta.stmin >= 0xf1 && cur_fc_sta.stmin <= 0xf9){
                    tick= get_timer_tick();
                    delay_tick = 1;//目前不支持微秒的延时，STmin指定为微秒级时，默认为1ms
                }else{
                    tick= get_timer_tick();
                    delay_tick = DEFAULT_STMIN_TIME;// 默认STmin指定由宏定义
                }
            }else if(cur_fc_sta.fs == FC_FS_WT){// wait next fc
                tick = get_timer_tick();//update timer
            }
            //else fs == FC_FS_OVFLW do not solve
        }
        break;
    case 2:
        if(uds_send_buf->sw == TRUE){// only if uds_send_buf have data and open , just send 
            if(uds_send_buf->len <= max_len){// send single buff
                cpy_send_to_sfpack();// copy send_buf to sf_pack
                send_uds_SF(single_pack_send_buf);
                reset_uds_send_buf();// if send over , reset send_buf
            }
        }
        if((unsigned long)(get_timer_tick() - tick) > delay_tick){// 
            sta = 3;
            send_uds_CF(multi_pack_send_buf);// 间隔delay_tick到，发送首帧后的第一帧CF
            multi_pack_send_buf->cur_sn++;// this time , cur_sn is not 0x0f
            multi_pack_send_buf->cur_recv_idx += max_len;
            tick = get_timer_tick();// reset tick
        }
        break;
    case 3:
        if(uds_send_buf->sw == TRUE){// only if uds_send_buf have data and open , just send 
            if(uds_send_buf->len <= max_len){// send single buff
                cpy_send_to_sfpack();// copy send_buf to sf_pack
                send_uds_SF(single_pack_send_buf);
                reset_uds_send_buf();// if send over , reset send_buf
            }
        }
        if(multi_pack_send_buf->cur_recv_idx >= multi_pack_send_buf->data_len){// if all data has been sended,change sta
            sta = 0;
            reset_uds_send_buf();// 全部数据已发完，回到case 0
        }else if((unsigned long)(get_timer_tick() - tick)> delay_tick){// 
            send_uds_CF(multi_pack_send_buf);// 按STmin间隔继续发送后续CF
            if(multi_pack_send_buf->cur_sn >= 0x0f)
                multi_pack_send_buf->cur_sn = 0;// SN计数到0x0F后回绕为0
            else
                multi_pack_send_buf->cur_sn++;
            multi_pack_send_buf->cur_recv_idx += max_len;
            tick = get_timer_tick();// update timer
        }
        break;
    }
}
