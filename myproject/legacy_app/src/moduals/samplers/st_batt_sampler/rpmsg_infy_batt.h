#ifndef __RPMSG_INFY_BATT_H
#define __RPMSG_INFY_BATT_H

#include "st_batt_sampler.h"


// 关键宏定义
#define BATT_CELL_MAX_NUM        416     // 每组最大电芯数

// 单个电池组的数据结构
typedef struct tagBMT_BATT_CELL_DATA
{
    /* ========== 协议解析后的单体数据（逐位展开） ========== */
    BYTE    byCellBalanceStat[BATT_CELL_MAX_NUM];      // 均充状态：0=空闲, 1=均充
    BYTE    byCellDischargeStat[BATT_CELL_MAX_NUM];    // 均放状态：0=空闲, 1=均放
    USHORT  usCellVolt[BATT_CELL_MAX_NUM];             // 单体电压，u16，单位1mV
    SHORT   sCellTemp[BATT_CELL_MAX_NUM];              // 单体温度，s16，单位0.1℃
    UINT    uiCellResReal[BATT_CELL_MAX_NUM];          // 内阻实部，u32，单位1μΩ
    UINT    uiCellResImag[BATT_CELL_MAX_NUM];          // 内阻虚部，u32，单位1μΩ
    BYTE    byCellIcStat[BATT_CELL_MAX_NUM];           // IC状态，u8 4bit 代表一个AFE IC状态
    
    /* ========== 业务层换算后的数据 ========== */
    float   fCellVolt[BATT_CELL_MAX_NUM];              // 换算后电压(V) = raw * 0.001
    float   fCellTemp[BATT_CELL_MAX_NUM];              // 换算后温度(℃) = raw * 0.1
    float   fCellRes[BATT_CELL_MAX_NUM];               // 换算后内阻(mΩ) = raw * 0.01
    /* ========== 状态管理 ========== */
    BYTE    byGroupIndex;                              // 组号索引 (0=组1, 1=组2)
    BYTE    bDataValid;                                // 数据有效标志
    _DATE_TIME tmLastUpdate;                           // 最后更新时间
} BMT_BATT_CELL_DATA;

typedef enum tagID_CMD_YFY_BATT_RPMSG
{
    //S2C: S,Server-BMS  C:Client-Monitor
    _RPMSG_R_CMD_S2C_STATUS = 0,
    _RPMSG_R_CMD_S2C_CELL_CHARG_BLANCE_STATUS_GRP1,
    _RPMSG_R_CMD_S2C_CELL_CHARG_BLANCE_STATUS_GRP2, 
    _RPMSG_R_CMD_S2C_CELL_DISCHARG_BLANCE_STATUS_GRP1,
    _RPMSG_R_CMD_S2C_CELL_DISCHARG_BLANCE_STATUS_GRP2,
    _RPMSG_R_CMD_S2C_CELL_VOLTAGE_GRP1,
    _RPMSG_R_CMD_S2C_CELL_VOLTAGE_GRP2,
    _RPMSG_R_CMD_S2C_CELL_TEMPERATURE_GRP1,
    _RPMSG_R_CMD_S2C_CELL_TEMPERATURE_GRP2,
    // _RPMSG_R_CMD_S2C_CELL_RESISTANCE_REAL_GRP1_1, // 放到 faq 里
    // _RPMSG_R_CMD_S2C_CELL_RESISTANCE_REAL_GRP1_2,
    // _RPMSG_R_CMD_S2C_CELL_RESISTANCE_REAL_GRP2_1,
    // _RPMSG_R_CMD_S2C_CELL_RESISTANCE_REAL_GRP2_2,
    // _RPMSG_R_CMD_S2C_CELL_RESISTANCE_IMAGE_GRP1_1,
    // _RPMSG_R_CMD_S2C_CELL_RESISTANCE_IMAGE_GRP1_2,
    // _RPMSG_R_CMD_S2C_CELL_RESISTANCE_IMAGE_GRP2_1,
    // _RPMSG_R_CMD_S2C_CELL_RESISTANCE_IMAGE_GRP2_2,
    // _RPMSG_R_CMD_S2C_CELL_IC_STATUS_GRP1,
    // _RPMSG_R_CMD_S2C_CELL_IC_STATUS_GRP2,

    _WHOLEINFO_HEARTBEAT,
    // _WHOLEINFO_HIS_FAULT_GRP1,
    // _WHOLEINFO_HIS_FAULT_GRP2,
    // _WHOLEINFO_HIS_EVENT_GRP1,
    // _WHOLEINFO_HIS_EVENT_GRP2,
    _WHOLEINFO_BOARD_SAMPLE,

    _WHOLEINFO_STATUS,
    _WHOLEINFO_GLOBAL,
    _WHOLEINFO_POWER,
    _WHOLEINFO_LIMIT,
    _WHOLEINFO_FEATURE,
    _WHOLEINFO_ENERGY_CURR,
    _WHOLEINFO_ENERGY_TOTAL,
    _WHOLEINFO_ALARM_FAULT_0,
    _WHOLEINFO_ALARM_FAULT_1,
    _WHOLEINFO_TIME,
    _WHOLEINFO_FAULT,
    _WHOLEINFO_SOS,
    _WHOLEINFO_DOD,
    // _WHOLEINFO_DOD_REC_GRP1,
    // _WHOLEINFO_DOD_REC_GRP2,
    // _WHOLEINFO_SOS_REC_GRP1,
    // _WHOLEINFO_SOS_REC_GRP2,
    _WHOLEINFO_AUTH,
    _WHOLEINFO_IMIU4_GRP1,
    _WHOLEINFO_IMIU4_GRP2,
    _WHOLEINFO_CTRL_FEEDBACK,
    _WHOLEINFO_BSU02,
    _WHOLEINFO_IMEU3_SAMP_FAULT,
    // _WHOLEINFO_RES_QUERY_FEEDBACK,

    _MAX_YFY_RPMSG_CMD,
} ID_CMD_YFY_BATT_RPMSG;


typedef enum RpmsgChannelType
{
    CELL_CHANNEL = 0,
    BMS_STATUS_CHANNEL,

} CHANNEL_TYPE;


typedef BOOL (*RPMSG_YFY_BATT_SAMPLE_PROC)(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME *pMsgFrame);

typedef struct tagYFYBattRpmsgPro
{
	ID_CMD_YFY_BATT_RPMSG           emMsgType;
	int  			                nFuncCode;
	int  		        	        nMainIndex;

	RPMSG_YFY_BATT_SAMPLE_PROC      pfnCmdHandler;

}TAG_YFY_BATT_PRO_RPMSG;


#define  SYS_COM     9
#define  SYS_ADCAN   15
#define  MAIN_CNTL_GEN  86
#define  MAIN_CNTL_ADVAN  71
#define  MAIN_CNTL_HARD   53
#define  IMIU4   21
#define  BSU02U   20

/* 03整体信息 命令参数数组尺寸 */
#define  AUTH_SWITCH_CMD   2    /* 权限切换命令 */
#define  GRP1_CTRL_CMD     3    /* 组1系统控制命令 */
#define  GRP2_CTRL_CMD     3    /* 组2系统控制命令 */
#define  USER_CTRL_CMD     2    /* 用户控制命令 */
#define  DEBUG_CMD         8    /* 系统调试命令 */
#define  CALIB_CMD         2    /* 采样校准命令 */

#define BMU02_FRAME_MAX_LEN     1024    // 单帧最大长度（规约要求）
#define BMU02_FRAME_ID_LEN      8       // ID域固定长度
#define BMU02_FRAME_DATA_HDR_LEN 3      // DATA域头部固定长度

#define  RPMSG_SOI		0xABCD
#define  RPMSG_EOI		0xEF

#define BMS_GROUP_CONFIG_CNT    13

typedef struct {
    
    uint32_t data_len;       // BYTE0-BYTE3: DATA域总长度（大端）
    uint8_t  func_code;      // BYTE4: 功能码（2/3/6/7/8/4/5/9/0/1/11/12/13）
    uint8_t  msg_type;       // BYTE5: 信息类型（主索引）[0,255]
    uint8_t  dst_addr;       // BYTE6: 目标设备地址 [0,254] 或 广播0xff
    uint8_t  src_addr;       // BYTE7: 源设备地址 [0,254]

} BMU02_Frame_ID_t;

typedef struct {
    uint8_t frame_flag;      // DATA0: 帧标识 0x0c=无确认通信, 0x0d=需确认通信
    uint8_t sub_idx_cnt;     // DATA1: 子索引数目 [1,255]
    uint8_t sub_idx_start;   // DATA2: 起始子索引 [0,254]
} BMU02_Frame_DataHdr_t;

typedef struct {
    BMU02_Frame_ID_t      id_hdr;    // 8字节ID域
    BMU02_Frame_DataHdr_t data_hdr;  // 3字节DATA域头部
    uint8_t               data_body[1]; // 柔性数组：可变长数据体起始地址
} BMU02_Frame_t;


typedef enum 
{
    SCALE_1 = 0,   // 默认 1
    SCALE_01,      // * 0.1
    SCALE_10,      // * 10
} ScaleType_e;

typedef enum
{   
    VAR_TYPE_FLOAT = 0,
    VAR_TYPE_ENUM,
    VAR_TYPE_INT
}VarType_e;

/* ================================================================
 * BmsGetType_e — 统一GET枚举
 * 合并06参数信息GET + 03整体信息命令参数GET
 * 06值保持100+(BmsGroupConfigs数组索引), 03值保持0+(WholeCmdConfigs数组索引)
 * ================================================================ */
typedef enum
{
    /* 05参数信息 GET */
    SYS_COMMON_GET          = 105,   /* 系统通用参数 */
    SYS_ADVANCED_GET        = 106,       /* 系统高级参数 */
    MAIN_CTRL_GENERAL_GET   = 107,           /* 主控通用参数 */
    MAIN_CTRL_ADVANCED_GET  = 108,          /* 主控高级参数 */
    MAIN_CTRL_HARDWARE_GET  = 109,          /* 主控硬件参数 */
    IMIU4_NORMAL_GET,                /* IMIU4常规参数 */
    BSU02U2_NORMAL_GET,              /* BSU02U2常规参数 */

    /* 03整体信息命令参数 GET */
    WHOLE_AUTH_SWITCH_GET   = 64,    /* 64 权限切换命令 */
    WHOLE_GRP1_CTRL_GET     = 65,             /* 65 组1系统控制命令 */
    WHOLE_GRP2_CTRL_GET     = 66,             /* 66 组2系统控制命令 */
    WHOLE_USER_CTRL_GET     = 69,             /* 69 用户控制命令 */
    WHOLE_DEBUG_CMD_GET     = 71,             /* 71 系统调试命令 */
    WHOLE_CALIB_CMD_GET     = 72,             /* 72 采样校准命令 */
    // WHOLE_IMEU3_HW_PARAM_GET= 75,             /* 75 IMEU3硬件参数当前值 */ // todo 仅可读
    WHOLE_IMEU3_HW_PARAM_SETBACK= 76,             /* 76 IMEU3硬件参数设置值 */
    // WHOLE_HARD_SOFT_WARE_GET= 76,             /* 76 IMEU3硬件参数设置值 */
    WHOLE_SLAVE_DEV_HW_SW_GET= 81,             /* 81 下级设备软硬件版本号查询回复 */
    WHOLE_SLAVE_DEV_SN_GET= 82,             /* 82 下级设备序列号查询回复 */
    WHOLE_PACK_SN_GET= 83,             /* 83 PACK序列号查询修改 */
    WHOLE_CELL_SN_GET= 84,        /* 84 电芯序列号查询回复 */
    WHOLE_CELL_RES_GET= 85,        /* 85 内阻查询指令 */

        // WHOLE_TIME_SYNC_GET     = 68,             /* 68 校时信息 */
    WHOLE_TIME_SYNC_GET     = 0,             /* 0 校时信息 */
    WHOLE_DEVICE_INFO_GET     = 2,             /* 设备信息 */
    WHOLE_DEVICE_SN_GET     = 3,             /* 设备序列号 */

} BmsGetType_e;

typedef struct {
    const BYTE* typeMap;   
    const BYTE* scaleMap;  
    USHORT baseVarID;  
    USHORT endId;      
    BYTE   DataBit;
    BmsGetType_e BmsPraType ;
} BmsGroupConfig_t;


/* ================================================================
 * BmsParameterInfo_t — 读响应数据结构
 * 调用 Bat_Rpmsg_GetPar 后, 参数值按小端序连续存于 stAR_Frame.Data[3...]
 *   Data[0] = 帧标识(0x0C), Data[1] = 子索引数目, Data[2] = 起始子索引
 * ================================================================ */
typedef struct tagBmsParameterInfo_t
{
    BYTE             byFrameFlag;        /* 0x0C 无确认通信 */
    BYTE             bySubIndexNumber;   /* 返回的子索引个数 */
    BYTE             bStartSubIndex;     /* 起始子索引 */
    BmsGetType_e     emSetParType;       /* 参数类型 */
    AR_FRAME         stAR_Frame;         /* 完整AR响应帧 */
    BYTE            Data[AR_MAX_DATA_LEN];
} BmsParameterInfo_t;

/* ================================================================
 * 读:  指定主索引 + 起始子索引 + 子索引数目 → BMU返回多个参数
 * 写:  指定主索引 + 起始子索引 + 子索引数目 + 参数值 → 下发到BMU
 *
 * 返回: 0 成功, -1 失败
 * ================================================================ */

int Bat_Rpmsg_GetPar(STBATT_SAMPLER_ROUGH *pRough,
                     BmsGetType_e type,
                     BYTE startSubIdx,
                     BYTE subIdxNum,
                     BmsParameterInfo_t *pResp);

int Bat_Rpmsg_SetPar(STBATT_SAMPLER_ROUGH *pRough,
                     BYTE mainIdx,
                     BYTE startSubIdx,
                     BYTE subIdxNum,
                     const BYTE *pData,
                     int nDataLen);


typedef enum
{
    /* 基础系统信息类 */
    E_SYS_STATUS_INFO,        // 系统状态信息
    E_SYS_GLOBAL_INFO,        // 系统整体信息
    E_SYS_POWER_INFO,         // 系统电量信息
    E_SYS_LIMIT_INFO,         // 限值信息
    E_SYS_FEATURE_INFO,       // 特征信息

    /* 能量&电量统计 */
    E_ENERGY_CURR_STAT,       // 当前能量统计信息
    E_ENERGY_TOTAL_STAT,      // 累计电量(能)统计信息

    /* 故障告警 */
    E_FAULT_GROUP1_ALARM,     // 组1故障告警信息
    E_FAULT_GROUP2_ALARM,     // 组2故障告警信息

    /* 统计类 */
    E_TIME_STAT_INFO,         // 时间统计信息
    E_FAULT_STAT_INFO,        // 故障统计信息
    E_BAT_SAFE_STAT,          // 电池安全等级统计信息
    E_DOD_STAT_INFO,          // 放电深度统计信息

    /* 权限相关 */
    E_AUTH_LEVEL_STATE,       // 权限级别状态
    E_AUTH_SWITCH_CMD,        // 权限切换命令

    /* 系统控制命令 */
    E_CTRL_GROUP1_CMD,        // 组1系统控制命令
    E_CTRL_GROUP2_CMD,        // 组2系统控制命令

    /* 硬件单元信息 */
    E_IMIU4_INFO,             // IMIU4信息
    E_USER_CTRL_CMD,          // 用户控制命令
    E_CTRL_FEEDBACK_INFO,     // 控制命令反馈信息
    E_SYS_DEBUG_CMD,          // 系统调试命令

    /* 校准 & 采样 */
    E_IMIU4_CALIB_CMD,        // 采样校准命令(对IMIU4)
    E_BSU02U2_PACK_INFO,      // BSU02U2板级及PACK采样信息

    E_ENUM_MAX
} InfoCmdType_e;

/* 03整体信息数据交换: 将RPMSG解析数据同步到STBATT_SAMPLER_ROUGH已有字段 */
void Bat_Rpmsg_ExchangeData(STBATT_SAMPLER_ROUGH *pRough, int nBattIdx);

/* 03整体信息命令参数: 读 */
int Bat_Rpmsg_GetWholeCmd(STBATT_SAMPLER_ROUGH *pRough,
                          BmsGetType_e type,
                          BYTE startSubIdx,
                          BYTE subIdxNum,
                          BmsParameterInfo_t *pResp);

/* 03整体信息命令参数: 写 */
int Bat_Rpmsg_SetWholeCmd(STBATT_SAMPLER_ROUGH *pRough,
                          BYTE mainIdx,
                          BYTE startSubIdx,
                          BYTE subIdxNum,
                          const BYTE *pData,
                          int nDataLen);

#endif