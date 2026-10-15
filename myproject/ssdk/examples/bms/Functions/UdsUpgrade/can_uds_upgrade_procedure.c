/* ******************* 
 * can uds
 **********************/

/*
 * 本文件是 UDS(ISO 14229)升级流程的核心状态机,tester 端(BMS 主控)给目标板升级固件。
 *
 * 整体采用两级状态机:
 *   1. 主状态机 sta(类型 UPGRADE_STEP_E): 由 uds_upgrade_proc() 用 switch(sta) 分发,
 *      每一步对应一个子状态机函数,函数返回值 next_sta 决定下一个主状态,出错统一跳 UDS_UPGRADE_ERROR。
 *   2. 子状态机 sub_sta: 每个子状态机函数内部用 static 局部变量 sub_sta 记录本步骤的进度,
 *      每次被主循环调用只推进一个子状态(发送/收应答),从而不阻塞 FreeRTOS 调度。
 *
 * 升级服务流程(SID 语义):
 *   0x10 切换会话(进入编程会话) -> 0x27 安全访问(seed/key 解锁) -> 0x22 读版本号
 *   -> 0x2E 写信息(预留占位) -> 0x31 例程(擦除 flash + 设置带 CRC 传输)
 *   -> 0x34 请求下载(协商分块大小) -> 0x36 分块传输 -> 0x31 整包 CRC 校验 -> 0x11 ECU 复位。
 *
 * 通用超时/重发约定: 每个子状态机在发出请求后记录 wait_time 时间戳,若 STATUS_WAIT_TIME(1s)
 * 内未收到应答则重发,重发 OVERTIMR_RESEND_COUNTER(3)次后仍无应答则置 g_uds_fail_sid/
 * g_uds_fail_nrc 并跳 UDS_UPGRADE_ERROR。
 *
 * ACK 判定约定(ack_sta,取值定义见 sid_internal_header.h):
 *   DEST_ACK_WAIT(0)       等待应答,走超时分支;
 *   DEST_ACK_POSITIVE(1)   收到正响应,进入下一子状态;
 *   DEST_ACK_NEGATIVE(-2)  收到负响应; DEST_ACK_WRONG_PARA(-3) 参数错误; DEST_ACK_ERROR(-4) 应答异常,
 *   三者统一按 ack_sta<0 处理,判定失败。
 */
#include <string.h>
#include "can_uds_network_layer.h"
#include "can_uds_services_layer.h"
#include "sid_internal_header.h"
#include "icc_upgrade_handler.h"

extern void *ExtMalloc(size_t xWantedSize);
extern void ExtFree(void *pv);

static unsigned char *g_down_send_buf = NULL;


#define STATUS_WAIT_TIME  1000      //如果1秒内没有收到任何回应，返回到初始状态
#define OVERTIMR_RESEND_COUNTER           3   //超时重发次数
#define SID34_MEM_ADDRESS_WIDTH           4   //0x34服务中"地址"字段占用的字节宽度
#define SID34_MEM_SIZE_WIDTH              4   //0x34服务中"长度"字段占用的字节宽度
#define MAX_BLOCK_NUM                   1024  //目标板回复的单块(block)大小上限,防止异常值导致越界
unsigned long block_num = 0;                  //0x34协商出的分块大小:0x36每次传输的字节数(不含CRC)
unsigned long  TesterWholePackCrc = 0;        //tester端累计的整包CRC,传输完成后发给目标做整包校验
uint8_t g_uds_percent = 0;                    //升级进度百分比(0~100),供上层显示
//0x31例程控制服务(routineControl)的 subFunction 取值
enum
{
    UDS_ROUTINE_CTRL_START = 0x01,		   // 启动例程
    UDS_ROUTINE_CTRL_STOP = 0x02,		   // 停止例程
    UDS_ROUTINE_CTRL_REQUEST_RESULT = 0x03 // 结束例程后获取结果值
};

#define ROUTINE_ID_ERASE_MEM        0xFF00      //擦除flash(擦除目标存储的flash)
#define ROUTINE_ID_CHECK_MEM        0x0202      //启动整包校验
#define ROUTINE_ID_TRANS_CRC_SET    0x0A01      //传输是否带CRC校验设置

//--- 内部变量 ---
static UPGRADE_STEP_E sta = UDS_UPGRADE_IDLE;   //主状态机当前状态,由 uds_upgrade_proc() 分发
static unsigned char uds_upgrade_flag = 0;      //启动标志:uds_upgrade_start()置1,idle态检测到后开始流程
static unsigned long logic_memory_address = 0,logic_memory_size = 10240;//逻辑起始地址/升级文件大小(先模拟10K的文件)
static unsigned char overtime_counter = 0;      //通用超时重发计数器,各子状态机共用(进入下一状态前清零)


//--- 内部函数声明 --- 
static UPGRADE_STEP_E uds_upgrade_idle_status(void);                    //
static UPGRADE_STEP_E uds_upgrade_switch_security_level_status(void);
static UPGRADE_STEP_E uds_upgrade_switch_session_status(void);
static UPGRADE_STEP_E uds_upgrade_read_dest_info_status(void);
static UPGRADE_STEP_E uds_upgrade_write_dest_info_status(void);
static UPGRADE_STEP_E uds_upgrade_pre_action_status(void);
static UPGRADE_STEP_E uds_upgrade_req_download_status(void);
static UPGRADE_STEP_E uds_upgrade_downloading_status(void);
static UPGRADE_STEP_E uds_upgrade_check_integrity_status(void);
static UPGRADE_STEP_E uds_upgrade_exit_status(void);
static UPGRADE_STEP_E uds_upgrade_dest_reset_status(void);
static UPGRADE_STEP_E  uds_upgrade_error_status(void);


//--- 内部函数 ---

/*
 * 功能: 升级流程的入口/等待状态。等待 uds_upgrade_start() 将 uds_upgrade_flag 置1,
 *       检测到启动请求后清标志并切换到会话切换步骤,否则停留在 IDLE。
 * 子状态: 无(本状态无内部子状态,只有一次标志判断)。
 */
/**
 * @brief idle状态子状态机
 * 
 * @return  UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_idle_status(void)
{
    if(uds_upgrade_flag){//当启动标志位被置1，进入状态机循环切换
        uds_upgrade_flag = 0;//立即将标志清0，防止重复进入
        return UDS_UPGRADE_SWITCH_SESSION;//先进入切换会话类型过程
    }
    return UDS_UPGRADE_IDLE;
}

/*
 * 功能: 0x27 安全访问(securityAccess),通过 seed/key 认证解锁目标板,防止未授权刷写。
 *       tester 先请求种子(seed),再用种子通过 uds_seed_to_key() 算出密钥(key)回发给目标校验。
 * 子状态流程:
 *   REQ_SEED(请求种子) -> RECV_SEED_ACK(收种子应答,得到4字节seed)
 *   -> SEND_KEY(计算并发送key,小端) -> RECV_KEY_ACK(收key应答,正响应则解锁成功)。
 */
/**
 * @brief 切换安全等级子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_switch_security_level_status(void)
{
    enum{
        SWITCH_SECURITY_REQ_SEED = 0,    //请求种子(seed)
        SWITCH_SECURITY_RECV_SEED_ACK,   //等待种子应答
        SWITCH_SECURITY_SEND_KEY,        //发送计算出的密钥(key)
        SWITCH_SECURITY_RECV_KEY_ACK,    //等待密钥应答
    };
#if USE_SECURITY_ACCESS_SERVICE
    static unsigned char sub_sta = 0;
    static unsigned long recv_seed = 0;
    static unsigned long wait_time = 0;
    unsigned long send_key = 0;
    unsigned char send_msg[6] = {0};
    int ack_sta = DEST_ACK_WAIT;
    UPGRADE_STEP_E next_sta = UDS_UPGRADE_SWITCH_SECURITY_LEVEL;
    switch(sub_sta)
    {
    case SWITCH_SECURITY_REQ_SEED:
        send_msg[0] = ds_security_access;
        send_msg[1] = SECURITY_REQ_SEED;     // 向目标板请求SEED
        send_msg[2] = 0;                     // 后四字节为dummy无效的状态
        send_msg[3] = 0;
        send_msg[4] = 0;
        send_msg[5] = 0;
        uds_send_data_active(send_msg,6);
        sub_sta = SWITCH_SECURITY_RECV_SEED_ACK;
        wait_time = get_timer_tick();
        break;
    case SWITCH_SECURITY_RECV_SEED_ACK:
        ack_sta = sid27_get_ack_status(sub_sta,&recv_seed);
        if(ack_sta < 0){//负反馈:收到负响应/参数错误/应答异常,请求种子失败
            sid27_clear_status();
            d_printf("sid27 machine seed request fail(%d)\r\n",__LINE__);
            recv_seed = 0;
            sub_sta = SWITCH_SECURITY_REQ_SEED;
            next_sta = UDS_UPGRADE_ERROR; //错误处理
            overtime_counter = 0; 
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:拿到4字节种子,进入发送key子状态
            sid27_clear_status();
            sub_sta = SWITCH_SECURITY_SEND_KEY;
        }else if((get_timer_tick()- wait_time) > STATUS_WAIT_TIME){//仍处WAIT且超1s,进入超时重发处理
            if(overtime_counter < OVERTIMR_RESEND_COUNTER){//未超重发上限,计数并回到请求种子子状态重发
                overtime_counter++;
                sub_sta = SWITCH_SECURITY_REQ_SEED;
            }else{//重发3次仍无应答,记录失败SID/超时原因并跳错误状态
            d_printf("sid27 wait overtime\r\n");
            recv_seed = 0;
            sub_sta = SWITCH_SECURITY_REQ_SEED;
            g_uds_fail_sid = 0x27; g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT; next_sta = UDS_UPGRADE_ERROR; //错误处理 
            overtime_counter = 0;
            }

        }
        break;
    case SWITCH_SECURITY_SEND_KEY:
        send_key = uds_seed_to_key(recv_seed);              //根据种子按固定算法算出key(seed/key认证核心)
        send_msg[0] = ds_security_access;
        send_msg[1] = SECURITY_SEND_KEY;                    // 向目标板发送计算后的KEY 
        send_msg[2] = (unsigned char)(send_key&0xff);       // 后四字节为计算后的KEY，使用小端数据,LSB在前
        send_msg[3] = (unsigned char)((send_key>>8)&0xff);
        send_msg[4] = (unsigned char)((send_key>>16)&0xff);
        send_msg[5] = (unsigned char)((send_key>>24)&0xff); 
        uds_send_data_active(send_msg,6);
        sub_sta = SWITCH_SECURITY_RECV_KEY_ACK;
        wait_time = get_timer_tick();
        break;
    case SWITCH_SECURITY_RECV_KEY_ACK:
        ack_sta = sid27_get_ack_status(sub_sta,&recv_seed);
        if(ack_sta < 0){//负反馈:key校验失败(密钥错误或目标拒绝解锁)
            sid27_clear_status();
            d_printf("sid27 machine seed verification fail(%d)\r\n",__LINE__);
            recv_seed = 0;
            sub_sta = SWITCH_SECURITY_REQ_SEED;
            next_sta = UDS_UPGRADE_ERROR; //错误处理
            overtime_counter = 0;             
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:解锁成功,回退子状态并跳到读版本号主状态
            sid27_clear_status();
            recv_seed = 0;
            sub_sta = SWITCH_SECURITY_REQ_SEED;               //恢复到第一个子状态
            next_sta = UDS_UPGRADE_READ_DEST_INFO;        //切换到下一个状态机
            overtime_counter = 0;
        }else if((get_timer_tick()- wait_time) > STATUS_WAIT_TIME){//仍处WAIT且超1s,进入超时重发处理
            if(overtime_counter < 4){//重发4-3次
                overtime_counter++;
                sub_sta = SWITCH_SECURITY_REQ_SEED;
            }else{//重发超限,记录失败SID/超时原因并跳错误状态
            d_printf("sid27 wait overtime\r\n");
            recv_seed = 0;
            sub_sta = SWITCH_SECURITY_REQ_SEED;
            g_uds_fail_sid = 0x27; g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT; next_sta = UDS_UPGRADE_ERROR; //错误处理 
            overtime_counter = 0;
            }

        }
        break;
    }
    return next_sta;
    
#else 
    return  UDS_UPGRADE_READ_DEST_INFO;
#endif
}

/*
 * 功能: 0x10 诊断会话控制(diagnosticSessionControl),让目标板进入编程会话(programmingSession),
 *       刷写类服务(0x34/0x36等)必须在编程会话下才被允许。
 * 子状态流程:
 *   SEND_SESSION_CHANGE(发送进入编程会话请求) -> RECV_SESSION_ACK(收应答,正响应进入下一主状态)。
 */
/**
 * @brief 切换会话模式子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_switch_session_status(void)
{
    enum{
        SWITCH_SESSION_SEND_SESSION_CHANGE = 0,   //发送会话切换请求
        SWITCH_SESSION_RECV_SESSION_ACK,          //等待会话切换应答
    };
    static unsigned char sub_sta = 0;
    static unsigned long wait_time = 0;
    unsigned char send_msg[6] = {0};
    int ack_sta = DEST_ACK_WAIT;
    UPGRADE_STEP_E next_sta = UDS_UPGRADE_SWITCH_SESSION;
    switch(sub_sta)
    {
    case SWITCH_SESSION_SEND_SESSION_CHANGE:
        send_msg[0] = ds_diagnostic_session_ctrl;
        send_msg[1] = PROGRAMMING_SESSION;     //要求目标板进入编程会话
        uds_send_data_active(send_msg,2);
        sub_sta = SWITCH_SESSION_RECV_SESSION_ACK;
        wait_time = get_timer_tick();
        break;
    case SWITCH_SESSION_RECV_SESSION_ACK:
        ack_sta = sid10_get_ack_status(sub_sta);
        if(ack_sta < 0){//负反馈:会话切换失败
            sid10_clear_status();
            d_printf("sid10 machine fail(%d)\r\n",__LINE__);
            sub_sta = SWITCH_SESSION_SEND_SESSION_CHANGE;
            next_sta = UDS_UPGRADE_ERROR; //错误处理
            overtime_counter = 0; 
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:进入编程会话成功,跳到安全访问主状态
            sid10_clear_status();
            sub_sta = SWITCH_SESSION_SEND_SESSION_CHANGE;   //恢复到原始状态
            next_sta = UDS_UPGRADE_SWITCH_SECURITY_LEVEL;   //切换到下一个子状态机
            overtime_counter = 0;
        }else if((get_timer_tick()- wait_time) > STATUS_WAIT_TIME){//仍处WAIT且超1s,进入超时重发处理
            if(overtime_counter < OVERTIMR_RESEND_COUNTER) //重发三次
            {
                overtime_counter++;
                sub_sta = SWITCH_SESSION_SEND_SESSION_CHANGE;
            }else{//重发超限,记录失败SID/超时原因并跳错误状态
                d_printf("sid10 wait overtime\r\n");
                sub_sta = SWITCH_SESSION_SEND_SESSION_CHANGE;
                g_uds_fail_sid = 0x10; g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT; next_sta = UDS_UPGRADE_ERROR; //错误处理 
                overtime_counter = 0;
            }
        }
        break;
    }
    return next_sta;
}

/*
 * 功能: 0x22 按标识符读数据(readDataByIdentifier),读取目标板软件版本号 DID(app_sw_id),
 *       用于确认目标板当前固件信息(也可据此判断是否需要升级)。
 * 子状态流程:
 *   SEND_DID(发送要读取的DID,小端) -> RECV_DID_ACK(收应答,正响应得到版本字符串)。
 */
/**
 * @brief 读取目标板信息子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_read_dest_info_status(void)
{
    enum{
        READ_DEST_INFO_SEND_DID = 0,     //发送要读取的DID请求
        READ_DEST_INFO_RECV_DID_ACK,     //等待DID应答
    };
    static unsigned char sub_sta = 0;
    static unsigned long wait_time = 0;
    unsigned char send_msg[3] = {0};
    unsigned char *p_did_data = 0;
    unsigned short len;
    unsigned long req_did = 0;
    int ack_sta = DEST_ACK_WAIT;
    UPGRADE_STEP_E next_sta = UDS_UPGRADE_READ_DEST_INFO;
    switch(sub_sta)
    {
    case READ_DEST_INFO_SEND_DID:
        req_did = app_sw_id;//读目标板的版本号
        send_msg[0] = ds_read_data_by_id;
        send_msg[1] = (unsigned char)(req_did&0xff);        // 要读取的DID，小端顺序，LSB在前
        send_msg[2] = (unsigned char)((req_did>>8)&0xff);
        uds_send_data_active(send_msg,3);
        sub_sta = READ_DEST_INFO_RECV_DID_ACK;
        wait_time = get_timer_tick();
        break;
    case READ_DEST_INFO_RECV_DID_ACK:
        ack_sta = sid22_get_ack_status(sub_sta,&p_did_data,&len);
        if(ack_sta < 0){//负反馈:读版本号失败
            sid22_clear_status();
            d_printf("sid22 machine fail(%d)\r\n",__LINE__);
            sub_sta = READ_DEST_INFO_SEND_DID;
            next_sta = UDS_UPGRADE_ERROR; //错误处理
            overtime_counter = 0;
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:拿到版本字符串,跳到写信息主状态
            d_printf("did data: %s\r\n",p_did_data);
            sid22_clear_status();
            sub_sta = READ_DEST_INFO_SEND_DID;      //恢复到原始状态
            next_sta = UDS_UPGRADE_WRITE_DEST_INFO; //切换到下一个子状态机
            overtime_counter = 0;
        }else if((get_timer_tick()- wait_time) > STATUS_WAIT_TIME){//仍处WAIT且超1s,进入超时重发处理
            if(overtime_counter < OVERTIMR_RESEND_COUNTER){//未超重发上限,计数并回到发送子状态重发
                overtime_counter++;
                sub_sta = READ_DEST_INFO_SEND_DID;
            }else{//重发超限,记录失败SID/超时原因并跳错误状态
            d_printf("sid22 wait overtime\r\n");
            sub_sta = READ_DEST_INFO_SEND_DID;
            g_uds_fail_sid = 0x22; g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT; next_sta = UDS_UPGRADE_ERROR; //错误处理 
            overtime_counter = 0;
            }

        }
        break;
    }
    return next_sta;
}

/*
 * 功能: 0x2E 按标识符写数据(writeDataByIdentifier),预留的占位步骤,
 *       当前不执行任何操作,直接进入预处理步骤,将来可在这一步写入目标板信息。
 */
/**
 * @brief 写入目标板信息子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_write_dest_info_status(void)
{
    //这个步骤可以不用了，预留，将来可能需要在这一步写入一些信息
    return UDS_UPGRADE_PRE_ACTION;
}

/*
 * 功能: 0x31 例程控制(routineControl),升级前的预处理,做两件事:
 *       1) 发送 ROUTINE_ID_ERASE_MEM(0xFF00) 擦除目标 flash;
 *       2) 发送 ROUTINE_ID_TRANS_CRC_SET(0x0A01) 设置后续分块传输是否带 CRC。
 * 子状态流程:
 *   ERASE_MEMORY_SEND(发擦除例程) -> ERASE_MEMORY_SEND_ACK(收擦除应答,擦除允许5s)
 *   -> SET_PACK_WITH_CRC_SEND(发CRC使能例程) -> SET_PACK_WITH_CRC_SEND_ACK(收应答)。
 */
/**
 * @brief 升级预处理子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_pre_action_status(void)
{//目前这个步骤做两个处理，1.发出擦除指令，2.设置子包带CRC(如果目标没有回复或者负反馈，默认认为不带CRC功能)
    enum{
        ERASE_MEMORY_SEND = 0,        //发送擦除例程
        ERASE_MEMORY_SEND_ACK,        //等待擦除应答
        SET_PACK_WITH_CRC_SEND,       //发送"传输带CRC"例程
        SET_PACK_WITH_CRC_SEND_ACK,   //等待CRC设置应答
    };
    static unsigned char sub_sta = 0;
    static unsigned long wait_tick = 0;
    unsigned char send_msg[12] = {0};
    int ack_sta = DEST_ACK_WAIT;
    UPGRADE_STEP_E next_sta = UDS_UPGRADE_PRE_ACTION;
    switch(sub_sta)
    {
    case ERASE_MEMORY_SEND:
        send_msg[0] = ds_routine_ctrl;
        send_msg[1] = UDS_ROUTINE_CTRL_START;         //启动功能    
        send_msg[2] = ((ROUTINE_ID_ERASE_MEM>>8) & 0xff);
        send_msg[3] = (ROUTINE_ID_ERASE_MEM & 0xff);          
        send_msg[4] = 0;
        send_msg[5] = 0;
        send_msg[6] = 0;
        send_msg[7] = 0;
        send_msg[8] = 0;
        send_msg[9] = 0;
        send_msg[10] = 0;
        send_msg[11] = 0;
        uds_send_data_active(send_msg,12);
        wait_tick = get_timer_tick();//记录时间戳，后续判断是否超时
        sub_sta = ERASE_MEMORY_SEND_ACK;
        break;
    case ERASE_MEMORY_SEND_ACK:
        ack_sta = sid31_get_ack_status(sub_sta);
        if(ack_sta < 0){//负反馈:擦除失败
            sid31_clear_status();
            d_printf("sid31 machine erase fail(%d)\r\n",__LINE__);
            sub_sta = ERASE_MEMORY_SEND;
            next_sta = UDS_UPGRADE_ERROR;      //错误处理
            overtime_counter = 0;
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:擦除成功,进入CRC设置子状态
            d_printf("erase success\r\n");
            sid31_clear_status();
            sub_sta = SET_PACK_WITH_CRC_SEND;      //恢复到原始状态
        }else{//(ack_sta == DEST_ACK_WAIT)状态下不做任何处理
            if((get_timer_tick()-wait_tick) > 5000){//擦除时间在5秒内都是允许的，如果超时了，认为对方未收到，重新发
                if(overtime_counter < OVERTIMR_RESEND_COUNTER){//未超重发上限,计数并回到发送子状态重发
                    overtime_counter++;
                    sub_sta = ERASE_MEMORY_SEND;  //恢复到原始状态
                }else{//重发超限,记录失败SID/超时原因并跳错误状态
                    d_printf("sid31 erase wait overtime\r\n");
                    sub_sta = ERASE_MEMORY_SEND;  //恢复到原始状态
                    g_uds_fail_sid = 0x31; g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT; next_sta = UDS_UPGRADE_ERROR; //错误处理 
                    overtime_counter = 0;
                }    
            }
        }
        break;
    case SET_PACK_WITH_CRC_SEND:
        send_msg[0] = ds_routine_ctrl;
        send_msg[1] = UDS_ROUTINE_CTRL_START;         //启动功能    
        send_msg[2] = ((ROUTINE_ID_TRANS_CRC_SET>>8) & 0xff);
        send_msg[3] = (ROUTINE_ID_TRANS_CRC_SET & 0xff);          
        send_msg[4] = g_uds_config.pack_with_crc;
        uds_send_data_active(send_msg,5);
        wait_tick = get_timer_tick();//记录时间戳，后续判断是否超时
        sub_sta = SET_PACK_WITH_CRC_SEND_ACK;
        break;
    case SET_PACK_WITH_CRC_SEND_ACK:
        ack_sta = sid31_get_ack_status(sub_sta);
        if(ack_sta < 0){//负响应，说明目标不支持此子功能，但仍进入下一个状态机
            sid31_clear_status();
            d_printf("pack-with-crc fail\r\n");
            sub_sta = ERASE_MEMORY_SEND;
            g_uds_fail_sid = 0x31; g_uds_fail_nrc = 0xE2; next_sta = UDS_UPGRADE_ERROR;         //错误处理 
            overtime_counter = 0;
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:CRC设置成功,跳到请求下载主状态
            d_printf("pack-with-crc success\r\n");
            sid31_clear_status();
            sub_sta = ERASE_MEMORY_SEND;                 //恢复到原始状态
            next_sta = UDS_UPGRADE_REQ_DOWNLOAD;         //切换到下一个子状态机
            overtime_counter = 0;
        }else{//(ack_sta == DEST_ACK_WAIT)状态下不做任何处理
            if((get_timer_tick()-wait_tick) > STATUS_WAIT_TIME){//仍处WAIT且超1s,进入超时重发处理
                if(overtime_counter < 4){//重发一次
                    overtime_counter++;
                    sub_sta = SET_PACK_WITH_CRC_SEND;  //恢复到原始状态
                }else{//重发超限,记录失败SID/超时原因并跳错误状态
                     d_printf("sid31 pack-with-crc wait overtime\r\n");
                    sub_sta = ERASE_MEMORY_SEND;  //恢复到原始状态
                    g_uds_fail_sid = 0x31; g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT; next_sta = UDS_UPGRADE_ERROR; //错误处理 
                    overtime_counter = 0;
                } 
            }
        }
        break;
    }
    return next_sta;
}

/*
 * 功能: 0x34 请求下载(requestDownload),把升级固件的逻辑地址(4字节)和大小(4字节)告知目标,
 *       目标据此回复本次传输的分块大小 block_num,供后续 0x36 分块传输使用。
 * 子状态流程:
 *   REQ_DOWNLOAD_SEND(发送地址/大小,小端) -> REQ_DOWNLOAD_WAIT_SEND_ACK(收应答得到 block_num,
 *   并对 block_num 做 MAX_BLOCK_NUM 上限保护)。
 */
/**
 * @brief 请求下载子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_req_download_status(void)
{
    enum{
        REQ_DOWNLOAD_SEND = 0,        //发送请求下载
        REQ_DOWNLOAD_WAIT_SEND_ACK,   //等待请求下载应答
    };
    static unsigned char sub_sta = 0;
    static unsigned long wait_tick = 0;
    unsigned char send_msg[11] = {0};

    int ack_sta = DEST_ACK_WAIT;
    UPGRADE_STEP_E next_sta = UDS_UPGRADE_REQ_DOWNLOAD;
    switch(sub_sta)
    {
    case REQ_DOWNLOAD_SEND:
        send_msg[0] = ds_request_download;
        send_msg[1] = COMPRESS_METHOD_NONE|ENCRYPT_METHOD_NONE;             //数据格式(压缩/加密），加密方式是否可以用于
        send_msg[2] = SID34_MEM_ADDRESS_WIDTH | (SID34_MEM_SIZE_WIDTH<<4);  //地址和长度表述字节的宽度，默认为4，不做更改
        send_msg[3] = (unsigned char)(logic_memory_address&0xff);           //填mem地址，使用小端数据,LSB在前
        send_msg[4] = (unsigned char)((logic_memory_address>>8)&0xff);
        send_msg[5] = (unsigned char)((logic_memory_address>>16)&0xff);
        send_msg[6] = (unsigned char)((logic_memory_address>>24)&0xff);
        logic_memory_size = (unsigned long)uds_get_upgrade_bin_len();
        send_msg[7] = (unsigned char)(logic_memory_size&0xff);              //填mem空间大小，使用小端数据,LSB在前    
        send_msg[8] = (unsigned char)((logic_memory_size>>8)&0xff);
        send_msg[9] = (unsigned char)((logic_memory_size>>16)&0xff);
        send_msg[10] = (unsigned char)((logic_memory_size>>24)&0xff);
        uds_send_data_active(send_msg,11);
        wait_tick = get_timer_tick();//记录时间戳，后续判断是否超时
        sub_sta = REQ_DOWNLOAD_WAIT_SEND_ACK;
        block_num = 0;//发送后先清0,等待目标在应答里回填分块大小
        break;
    case REQ_DOWNLOAD_WAIT_SEND_ACK:
        ack_sta = sid34_get_ack_status(sub_sta,&block_num);
        if(ack_sta < 0){//负反馈:请求下载被拒绝
            sid34_clear_status();
            d_printf("sid34 machine fail(%d)\r\n",__LINE__);
            sub_sta = REQ_DOWNLOAD_SEND;
            next_sta = UDS_UPGRADE_ERROR;      //错误处理
             overtime_counter = 0;
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:得到分块大小,初始化整包CRC后进入传输
            if (block_num > MAX_BLOCK_NUM) {//防止目标回复异常大的分块导致后续越界
                block_num = MAX_BLOCK_NUM;
            }
            d_printf("download block num: %d\r\n",block_num);
            sid34_clear_status();
            uds_crc_init(&TesterWholePackCrc);//整包CRC从0xffffffff开始累计
            sub_sta = REQ_DOWNLOAD_SEND;      //恢复到原始状态
            next_sta = UDS_UPGRADE_DOWNLOADING; //切换到下一个子状态机
            overtime_counter = 0;
        }else{//(ack_sta == DEST_ACK_WAIT)状态下不做任何处理
            if((get_timer_tick()-wait_tick) > STATUS_WAIT_TIME){//如果1秒内没有收到任何回应，重新发送
                if(overtime_counter < OVERTIMR_RESEND_COUNTER){//未超重发上限,计数并回到发送子状态重发
                    overtime_counter++;
                    sub_sta = REQ_DOWNLOAD_SEND;
                }else{//重发超限,记录失败SID/超时原因并跳错误状态
                        d_printf("sid34  wait overtime\r\n");
                        sub_sta = REQ_DOWNLOAD_SEND;
                        g_uds_fail_sid = 0x34; g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT; next_sta = UDS_UPGRADE_ERROR;
                        overtime_counter = 0;
                }
            }
        }
        break;
    }
    return next_sta;
}

/*
 * 功能: 0x36 数据传输(transferData),把升级固件按 block_num 分块逐包发送到目标板。
 *       每包带块序号 block_counter(0~255循环),可选择在包尾附加4字节CRC;
 *       同时把每包数据累计进 TesterWholePackCrc,供后续整包校验使用。
 * 子状态流程:
 *   REQ_DOWNLOADING_START(初始化计数器/文件索引) -> REQ_DOWNLOADING_DATA_SEND(取一包数据,
 *   必要时加CRC后发送) -> REQ_DOWNLOADING_DATA_SEND_ACK(收块应答:序号不符重发、校验失败重发、
 *   全部发完跳整包校验)。
 */
/**
 * @brief 升级传输子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */

UPGRADE_STEP_E uds_upgrade_downloading_status(void)
{
    enum{
        REQ_DOWNLOADING_START = 0,        //传输准备(初始化各计数器/文件索引)
        REQ_DOWNLOADING_DATA_SEND,        //取一包数据并发送
        REQ_DOWNLOADING_DATA_SEND_ACK,    //收块应答
    };
    static unsigned char sub_sta = 0;
    unsigned char *send_msg = NULL;
    static unsigned char block_counter = 0;      //块序号(0~255循环),每发一包加1
    static unsigned long file_idx = 0;           //已发送数据的文件偏移
    static unsigned long left_bytes = 0;         //剩余未发送字节数
    static unsigned long wait_tick = 0;
    static unsigned long single_pack_len;        //本包实际数据长度(带CRC时已含4字节CRC)
    static unsigned char sid36_overtime_counter = 0; //0x36专属超时重发计数(上限15,比通用3更宽松)
    unsigned char block_sn = 0,response_status = 0;//目标回填的块序号/否定响应状态码
    unsigned long crc = 0;                       //本包数据计算出的CRC
    unsigned char *p_data = 0;
    static unsigned char resend_num = 0;         //单包重发计数(上限10)
    int ack_sta = DEST_ACK_WAIT;
    UPGRADE_STEP_E next_sta = UDS_UPGRADE_DOWNLOADING;

    if (g_down_send_buf == NULL) {
        g_down_send_buf = ExtMalloc(MAX_SEND_BUF_LEN);
        if (g_down_send_buf == NULL) {
            ssdk_printf(SSDK_WARNING, "%s: malloc g_down_send_buf fail\r\n", __func__);
            next_sta = UDS_UPGRADE_ERROR;
            g_uds_fail_sid = 0x36;
            g_uds_fail_nrc = UDS_FAIL_MEM_ALLOC;
            return next_sta;
        }
    }
    send_msg = g_down_send_buf;

    switch(sub_sta)
    {
    case REQ_DOWNLOADING_START://进行传输的准备
        block_counter = 0; // counter 清0
        file_idx = 0;
        resend_num = 0;
        sid36_overtime_counter = 0;
        sub_sta = REQ_DOWNLOADING_DATA_SEND;
        break;
    case REQ_DOWNLOADING_DATA_SEND:
        send_msg[0] = ds_transfer_data;
        send_msg[1] = block_counter;                 //块序号作为subFunction,目标用它判断是否丢包/乱序
        left_bytes = uds_get_upgrade_bin_len() - file_idx;
        single_pack_len = (left_bytes > block_num) ? block_num : left_bytes;//本包取满block_num或剩余不足取剩余
        if(uds_get_upgrade_data(file_idx,single_pack_len,&p_data) > 0){//按文件偏移取出一包数据
            memcpy(&send_msg[2],p_data,single_pack_len);
        }else{
            sub_sta = REQ_DOWNLOADING_START;
            next_sta = UDS_UPGRADE_ERROR;
            g_uds_fail_sid = 0x36;
            g_uds_fail_nrc = UDS_FAIL_FILE_READ;
            d_printf("sid36 file read fail(%d)\r\n",__LINE__);
            break;
        }
        
        if(g_uds_config.pack_with_crc){//如果包附带CRC功能使能，则实际长度增加4字节，CRC校验值塞在尾部
            crc = uds_crc(0xfffffffful,&send_msg[2],single_pack_len);//对纯数据(不含CRC)算CRC,初值0xffffffff
            send_msg[2+single_pack_len+0] = crc & 0xfful;//CRC按小端附加到数据尾部
            send_msg[2+single_pack_len+1] = (crc>>8) & 0xfful;
            send_msg[2+single_pack_len+2] = (crc>>16) & 0xfful;
            send_msg[2+single_pack_len+3] = (crc>>24) & 0xfful;
            single_pack_len += 4;
        }
        uds_send_data_active(send_msg,2+single_pack_len);
        wait_tick = get_timer_tick();//记录时间戳，后续判断是否超时
        sub_sta = REQ_DOWNLOADING_DATA_SEND_ACK;
        break;
    case REQ_DOWNLOADING_DATA_SEND_ACK:
        ack_sta = sid36_get_ack_status(sub_sta,&block_sn,&response_status);
        if(ack_sta < 0){//负反馈:按 response_status 区分不同错误类型
            sid36_clear_status();
            if(response_status == 1){//否定响应时，需要特殊区分busy状态，busy状态不认为是错误
                //pass，不做任何动作
            }else if(response_status == 2){//单包校验失败重发
                if(resend_num < 10)
                {
                   resend_num++;
                   sub_sta = REQ_DOWNLOADING_DATA_SEND;
                }else{//单包重发10次仍失败,判为CRC校验错误
                     d_printf("sid36 machine pack-with-crc resend num over limit(%d)\r\n",__LINE__);
                     sub_sta = REQ_DOWNLOADING_START;
                     next_sta = UDS_UPGRADE_ERROR;
                     g_uds_fail_sid = 0x36;
                     g_uds_fail_nrc = nrc_single_pack_crc_check_error;
                }
            }else{//其他否定响应,直接判失败
                d_printf("sid36 machine nega ask fail(%d)\r\n",__LINE__);
                sub_sta = REQ_DOWNLOADING_START;
                g_uds_fail_sid = 0x36; g_uds_fail_nrc = 0xE2; next_sta = UDS_UPGRADE_ERROR;
            }
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:目标已收到本包
            d_printf("block-sn [%d]\r\n",block_sn);
            sid36_clear_status();
            if(block_sn != block_counter){//如果回复的block-sn与发送的不符，重新发送该block-sn
                if(resend_num < 10)
                {
                   resend_num++;
                   sub_sta = REQ_DOWNLOADING_DATA_SEND;
                }else{//块序号重发10次仍不符,判为块序号错误
                     d_printf("sid36 machine block sn wrong(%d)\r\n",__LINE__);
                     sub_sta = REQ_DOWNLOADING_START;
                     next_sta = UDS_UPGRADE_ERROR;
                     g_uds_fail_sid = 0x36;
                     g_uds_fail_nrc = nrc_wrong_block_sequence_counter;
                }
            }else{//块序号一致:本包数据有效,累计整包CRC并推进文件索引
                uds_wholepackcrc(&TesterWholePackCrc,&send_msg[2],(g_uds_config.pack_with_crc?(single_pack_len-4):single_pack_len));//只累计纯数据,不含CRC字段
                file_idx += (g_uds_config.pack_with_crc?(single_pack_len-4):single_pack_len);           //文件索引后移
                block_counter = (block_counter+1)%256;//块序号0~255循环递增
                {
                    unsigned long total = uds_get_upgrade_bin_len();
                    if (total > 0)
                        g_uds_percent = (uint8_t)(5 + 90UL * file_idx / total);//传输阶段进度映射到5%~95%
                }
                if(left_bytes <= block_num){//说明这一包已经是最后一包数据的响应，准备退出
                    d_printf("transfer finish, fail-times: %d\r\n",sid36_overtime_counter);
                    sub_sta = REQ_DOWNLOADING_START;          //恢复到原始状态
                    next_sta = UDS_UPGRADE_CHECK_INTEGRITY;
                }else{
                    sub_sta = REQ_DOWNLOADING_DATA_SEND;      //进入发送状态
                }
            }
        }else{//(ack_sta == DEST_ACK_WAIT)状态下判断超时重发处理
            if((get_timer_tick() - wait_tick) > STATUS_WAIT_TIME){//如果1秒内没收到回应，则重新发送同一包
                if(sid36_overtime_counter < 15)
                {
                    sid36_overtime_counter++;
                    sub_sta = REQ_DOWNLOADING_DATA_SEND;
                }else{//超时重发15次仍无应答,判为超时错误
                     d_printf("sid36 machine send overtime (%d)\r\n",__LINE__);
                     sub_sta = REQ_DOWNLOADING_START;
                     next_sta = UDS_UPGRADE_ERROR;
                     g_uds_fail_sid = 0x36;
                     g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT;
                }

            }
        }
        break;
    }
    return next_sta;
}

/*
 * 功能: 0x31 例程控制,发送 ROUTINE_ID_CHECK_MEM(0x0202) 启动整包 CRC 校验,
 *       把传输期间累计的 TesterWholePackCrc 发给目标板,与目标烧录数据算出的CRC比对,
 *       用于确认传输数据完整性。
 * 子状态流程:
 *   CHECK_INTEGRITY_SEND(发送整包CRC,小端) -> CHECK_INTEGRITY_SEND_ACK(收校验结果应答)。
 */
/**
 * @brief 验证完整性子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_check_integrity_status(void)
{//
    enum{
        CHECK_INTEGRITY_SEND = 0,     //发送整包CRC校验例程
        CHECK_INTEGRITY_SEND_ACK,     //等待校验结果应答
    };
    static unsigned char sub_sta = 0;
    static unsigned long wait_tick = 0;
    unsigned char send_msg[8] = {0};
    int ack_sta = DEST_ACK_WAIT;
    UPGRADE_STEP_E next_sta = UDS_UPGRADE_CHECK_INTEGRITY;
    switch(sub_sta)
    {
    case CHECK_INTEGRITY_SEND:
        send_msg[0] = ds_routine_ctrl;
        send_msg[1] = UDS_ROUTINE_CTRL_START;         //启动功能    
        send_msg[2] = ((ROUTINE_ID_CHECK_MEM>>8) & 0xff);
        send_msg[3] = (ROUTINE_ID_CHECK_MEM & 0xff);          
        send_msg[4] = TesterWholePackCrc & 0xff;
        send_msg[5] = (TesterWholePackCrc >> 8) & 0xff;
        send_msg[6] = (TesterWholePackCrc >> 16) & 0xff;
        send_msg[7] = (TesterWholePackCrc >> 24) & 0xff;
        uds_send_data_active(send_msg,8);
        wait_tick = get_timer_tick();//记录时间戳，后续判断是否超时
        sub_sta = CHECK_INTEGRITY_SEND_ACK;
        break;
    case CHECK_INTEGRITY_SEND_ACK:
        ack_sta = sid31_get_ack_status(sub_sta);
        if(ack_sta < 0){//负反馈:整包CRC校验失败(数据可能损坏)
            sid31_clear_status();
            d_printf("sid31 machine CRC fail(%d)\r\n",__LINE__);
            sub_sta = CHECK_INTEGRITY_SEND;
            next_sta = UDS_UPGRADE_ERROR;      //结束流程
            overtime_counter = 0;
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:校验通过,跳到退出主状态
            d_printf("CRC check success\r\n");
            sid31_clear_status();
            sub_sta = CHECK_INTEGRITY_SEND;      //恢复到原始状态
            next_sta = UDS_UPGRADE_EXIT;         //切换到下一个子状态机
            overtime_counter = 0;
        }else{//(ack_sta == DEST_ACK_WAIT)状态下不做任何处理
            if((get_timer_tick()-wait_tick) > STATUS_WAIT_TIME){//如果1秒内没有收到任何回应，重新发送
                if(overtime_counter < OVERTIMR_RESEND_COUNTER){//未超重发上限,计数并回到发送子状态重发
                    overtime_counter++;
                    sub_sta = CHECK_INTEGRITY_SEND;  //恢复到原始状态
                }else{//重发超限,记录失败SID/超时原因并跳错误状态
                    d_printf("sid31 CRC wait overtime\r\n");
                    sub_sta = CHECK_INTEGRITY_SEND;  //恢复到原始状态
                    g_uds_fail_sid = 0x31; g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT; next_sta = UDS_UPGRADE_ERROR; //错误处理 
                    overtime_counter = 0;
                }
            }
        }
        break;
    }
    return next_sta;
}

/*
 * 功能: 升级退出步骤,预留占位,当前不关心整包CRC等收尾处理,直接进入复位步骤。
 */
/**
 * @brief 退出升级子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_exit_status(void)
{//这里流程，可以根据回复，做整包CRC，目前这里不关心整包CRC的结果，所以直接跳过这个阶段
    return UDS_UPGRADE_DEST_RESET;
}


/*
 * 功能: 错误驻留状态。升级过程中任何一步失败都会跳到这里,打印失败日志后保持 ERROR,
 *       需调用 uds_upgrade_reset() 才能回到 IDLE 重新开始。
 */
UPGRADE_STEP_E  uds_upgrade_error_status(void)
{
    d_printf("upgrade fail exited\r\n");
    return UDS_UPGRADE_ERROR;
}
/*
 * 功能: 0x11 ECU复位(ecuReset),通知目标板执行复位使新固件生效,复位成功后回到 IDLE 结束流程。
 * 子状态流程:
 *   DEST_RESET_SEND(发送硬件复位请求) -> DEST_RESET_SEND_ACK(收应答,正响应回到IDLE)。
 */
/**
 * @brief 目标板复位子状态机
 * 
 * @return UPGRADE_STEP_E,返回要切换的状态
 */
UPGRADE_STEP_E uds_upgrade_dest_reset_status(void)
{//复位命令，不用等待回复了
    enum{
        DEST_RESET_SEND = 0,     //发送复位请求
        DEST_RESET_SEND_ACK,     //等待复位应答
    };
    static unsigned char sub_sta = 0;
    static unsigned long wait_time = 0;
    unsigned char send_msg[2] = {0};
    int ack_sta = DEST_ACK_WAIT;
    UPGRADE_STEP_E next_sta = UDS_UPGRADE_DEST_RESET;
    switch(sub_sta)
    {
    case DEST_RESET_SEND:
        send_msg[0] = ds_ecu_reset;
        send_msg[1] = 0x01;         //硬件复位    
        uds_send_data_active(send_msg,2);
        sub_sta = DEST_RESET_SEND_ACK;
        wait_time = get_timer_tick();
        break;
    case DEST_RESET_SEND_ACK:
        ack_sta = sid11_get_ack_status(sub_sta);
        if(ack_sta < 0){//负反馈:复位请求失败
            sid11_clear_status();
            d_printf("sid11 machine fail(%d)\r\n",__LINE__);
            sub_sta = DEST_RESET_SEND;
            next_sta = UDS_UPGRADE_ERROR; //错误处理
            overtime_counter = 0; 
        }else if(ack_sta == DEST_ACK_POSITIVE){//正响应:复位成功,升级流程结束,回到IDLE
            sid11_clear_status();
            sub_sta = DEST_RESET_SEND;   //恢复到原始状态
            next_sta = UDS_UPGRADE_IDLE;  //回到IDLE状态
            overtime_counter = 0;
        }else if((get_timer_tick()- wait_time) > STATUS_WAIT_TIME){//仍处WAIT且超1s,进入超时重发处理
            if(overtime_counter < OVERTIMR_RESEND_COUNTER) //重发三次
            {
                overtime_counter++;
                sub_sta = DEST_RESET_SEND;
            }else{//重发超限,记录失败SID/超时原因并跳错误状态
                d_printf("sid11 wait overtime\r\n");
                sub_sta = DEST_RESET_SEND;
                g_uds_fail_sid = 0x11; g_uds_fail_nrc = UDS_FAIL_SID_TIMEOUT; next_sta = UDS_UPGRADE_ERROR; //错误处理 
                overtime_counter = 0;
            }
        }
        break;
    }
    return next_sta;

}



//--- 外部函数 ---

/**
 * @brief uds_upgrade_start, 启动uds升级，调用此函数后，启动uds升级流程，初始状态为UDS_UPGRADE_IDLE
 * 
 */
void uds_upgrade_start(void)
{
    uds_upgrade_flag = 1;//将升级标志置1，如果处于idle状态下，则进入升级状态
}


/*
 * 功能: 升级流程主调度。每次调用根据主状态 sta 分发到对应子状态机执行一步,
 *       子状态机返回的 next_sta 决定主状态如何跳转;所有子状态机出错时统一返回
 *       UDS_UPGRADE_ERROR,本函数无需额外错误处理。进度百分比在相邻步骤切换时更新。
 */
/**
 * @brief uds_upgrade_proc, uds升级流程的总控制
 * 
 */
void uds_upgrade_proc(void)
{
    switch(sta)
    {
    case UDS_UPGRADE_IDLE:
        sta = uds_upgrade_idle_status();
        break;
    case UDS_UPGRADE_SWITCH_SESSION:
        sta = uds_upgrade_switch_session_status();  //10服务
        if (sta == UDS_UPGRADE_SWITCH_SECURITY_LEVEL) g_uds_percent = 1;
        break;
    case UDS_UPGRADE_SWITCH_SECURITY_LEVEL:
        sta = uds_upgrade_switch_security_level_status();  //27服务，验证密钥
        if (sta == UDS_UPGRADE_READ_DEST_INFO) g_uds_percent = 2;
        break;
    case UDS_UPGRADE_READ_DEST_INFO:
        sta = uds_upgrade_read_dest_info_status();  //22服务，查询版本号
        if (sta == UDS_UPGRADE_WRITE_DEST_INFO) g_uds_percent = 3;
        break;
    case UDS_UPGRADE_WRITE_DEST_INFO:
        sta = uds_upgrade_write_dest_info_status();   //未使用
        if (sta == UDS_UPGRADE_PRE_ACTION) g_uds_percent = 4;
        break;
    case UDS_UPGRADE_PRE_ACTION:
        sta = uds_upgrade_pre_action_status();  //31服务，擦除命令和单包CRC使能
        if (sta == UDS_UPGRADE_REQ_DOWNLOAD) g_uds_percent = 4;
        break;
    case UDS_UPGRADE_REQ_DOWNLOAD:
        sta = uds_upgrade_req_download_status();  //34服务，请求下载
        if (sta == UDS_UPGRADE_DOWNLOADING) g_uds_percent = 5;
        break;
    case UDS_UPGRADE_DOWNLOADING:
        sta = uds_upgrade_downloading_status();   //36服务
        break;
    case UDS_UPGRADE_CHECK_INTEGRITY:
        sta = uds_upgrade_check_integrity_status();  //31服务，整包CRC
        if (sta == UDS_UPGRADE_EXIT) g_uds_percent = 96;
        break;
    case UDS_UPGRADE_EXIT:
        sta = uds_upgrade_exit_status();   //未使用
        if (sta == UDS_UPGRADE_DEST_RESET) g_uds_percent = 97;
        break;
    case UDS_UPGRADE_DEST_RESET:
        sta = uds_upgrade_dest_reset_status();   //11服务，复位
        if (sta == UDS_UPGRADE_IDLE) g_uds_percent = 100;
        break;
    case UDS_UPGRADE_ERROR://不做任何处理，只作为空驻留使用
        sta = uds_upgrade_error_status();;
        break;
    }
}

/**
 * @brief uds_upgrade_reset, uds升级流程复位，状态切换到IDLE
 * 
 */
void uds_upgrade_reset(void)
{
    sta = UDS_UPGRADE_IDLE;
    if (g_down_send_buf != NULL) {
        ExtFree(g_down_send_buf);
        g_down_send_buf = NULL;
    }
}

uint8_t uds_get_upgrade_step(void)
{
    return (uint8_t)sta;
}

uint8_t uds_get_upgrade_percent(void)
{
    return g_uds_percent;
}

void uds_set_upgrade_percent(uint8_t percent)
{
    g_uds_percent = percent;
}

uint8_t uds_get_upgrade_sid(void)
{
    switch (sta) {
    case UDS_UPGRADE_SWITCH_SESSION:        return 0x10;
    case UDS_UPGRADE_SWITCH_SECURITY_LEVEL: return 0x27;
    case UDS_UPGRADE_READ_DEST_INFO:        return 0x22;
    case UDS_UPGRADE_WRITE_DEST_INFO:       return 0x2E;
    case UDS_UPGRADE_PRE_ACTION:            return 0x31;
    case UDS_UPGRADE_REQ_DOWNLOAD:          return 0x34;
    case UDS_UPGRADE_DOWNLOADING:           return 0x36;
    case UDS_UPGRADE_CHECK_INTEGRITY:       return 0x31;
    case UDS_UPGRADE_DEST_RESET:            return 0x11;
    default:                                return 0;
    }
}