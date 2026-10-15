// #define _TRACE_PRINT_  
// #include "st_batt_sampler.h"
#include "rpmsg_infy_batt.h"
// #include  "imeu2_can.h"
// #include  "imeu3_can.h"

void RefreshBmslArmOrFault(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx);

#define LOG_FILE_PATH "/tmp/my_app.log"

#define LOG_TO_FILE(fmt, ...)                                      \
    do {                                                           \
        FILE *fp = fopen(LOG_FILE_PATH, "a");                      \
        if (fp != NULL) {                                          \
            fprintf(fp, fmt, ##__VA_ARGS__);                  \
            fclose(fp);                                            \
        } else {                                                   \
            TRACED("无法打开日志文件！\n");                         \
        }                                                          \
    } while (0)

static BMT_BATT_CELL_DATA           s_infy_BmtBattCellData[MAX_SINGLE_STBATT_NUM];  
static TAG_YFY_BATT_PRO_RPMSG		s_infy_Batt_Rpmsg_Pro[_MAX_YFY_RPMSG_CMD];

// 建立一个住主索引和电池组数据的映射关系，方便解析函数根据主索引找到对应的电池组数据结构
static int GetGroupIndexByMainIndex(int nMainIndex)
{
    switch(nMainIndex)
    {
        // case 0x00: // _RPMSG_R_CMD_S2C_CELL_CHARG_BLANCE_STATUS_GRP1
        // case 0x02: // _RPMSG_R_CMD_S2C_CELL_DISCH
        // case 0x0A: // _RPMSG_R_CMD_S2C_CELL_VOLTAGE_GRP1
        // case 0x1E: // _RPMSG_R_CMD_S2C_CELL_TEMPERATURE_GRP1
        // case 0x0F: // _WHOLEINFO_DOD_REC_GRP1
        // case 0x11: // _WHOLEINFO_SOS_REC_GRP1
        case 0X0F:
        case 0X11:
        case 20:
        case 22:
        case 24:
        case 26:
        case 28:
        case 29:
        case 32:
        case 33:
        case 36:
        case 43:
        case 0X39:
        case 0X43:
            return 0; // 组1
        // case 0x01: // _RPMSG_R_CMD_S2C_CELL_CHARG_BLANCE_STATUS_GRP2
        // case 0x03: // _RPMSG_R_CMD_S2C_CELL_DISCH
        // case 0x14: // _RPMSG_R_CMD_S2C_CELL_VOLTAGE_GRP2
        // case 0x28: // _RPMSG_R_CMD_S2C_CELL_TEMPERATURE_GRP2
        // case 0x10: // _WHOLEINFO_DOD_REC_GRP2
        // case 0x12: // _WHOLEINFO_SOS_REC_GRP2
        case 0X10:
        case 0X12:
        case 21:
        case 23:
        case 25:
        case 27:
        case 30:
        case 31:
        case 34:
        case 35:
        case 37:
        case 44:
        case 0X3A:
        case 0X44:
            return 1; // 组2
        default:
            return -1; // 无效主索引
    }
}

// 历史故障(0x0A/0x0B) / 历史事件(0x0C/0x0D) 主索引 -> 电池组号
static int GetHisRecordGroupIndex(int nMainIndex)
{
    switch (nMainIndex)
    {
        case 0x0A: case 0x0C: case 0x0F: case 0x1C: case 0x1D: case 0x20: case 0x21: case 0x24: return 0; // 组1
        case 0x0B: case 0x0D: case 0x10: case 0x1E: case 0x1F: case 0x22: case 0x23: case 0x25: return 1; // 组2
        default:              return -1; // 无效主索引
    }
}

static SHORT DatatoSHORT(BYTE* InputData)
{
    SHORT svalue = (SHORT)InputData[0] + (SHORT)(InputData[1] << 8);
    return svalue;
}

static USHORT DatatoUSHORT(BYTE* InputData)
{
    USHORT usvalue = (USHORT)InputData[0] + (USHORT)(InputData[1] << 8);
    return usvalue;
}


static float DatatoFloat(BYTE* InputData)
{
    UINT raw = (UINT)InputData[0] | ((UINT)InputData[1] << 8) | ((UINT)InputData[2] << 16) | ((UINT)InputData[3] << 24);
    float f;
    memcpy(&f, &raw, sizeof(float));
    return f;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/25 12:00:31
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_Cell_ChargeBalanceStatus(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    int nMainIndex = pMsgFrame->ID_MainIndex;
    BYTE *pOutStat;
    int max_cells, max_bytes, remainder;
    int i;

    int nGroupIndex = GetGroupIndexByMainIndex(nMainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_SINGLE_STBATT_NUM)
    {
        return FALSE;
    }

    pOutStat = &s_infy_BmtBattCellData[nGroupIndex].byCellBalanceStat[0];

    max_cells = (BATT_CELL_MAX_NUM < 416) ? BATT_CELL_MAX_NUM : 416;
    max_bytes = max_cells / 8; // 每字节8个电芯状态
    remainder = max_cells % 8;

    for (i = 0; i < max_bytes; i++)
    {
        pOutStat[i * 8 + 0] = (pMsgFrame->Data[i] >> 0) & 0x01;
        pOutStat[i * 8 + 1] = (pMsgFrame->Data[i] >> 1) & 0x01;
        pOutStat[i * 8 + 2] = (pMsgFrame->Data[i] >> 2) & 0x01;
        pOutStat[i * 8 + 3] = (pMsgFrame->Data[i] >> 3) & 0x01;
        pOutStat[i * 8 + 4] = (pMsgFrame->Data[i] >> 4) & 0x01;
        pOutStat[i * 8 + 5] = (pMsgFrame->Data[i] >> 5) & 0x01;
        pOutStat[i * 8 + 6] = (pMsgFrame->Data[i] >> 6) & 0x01;
        pOutStat[i * 8 + 7] = (pMsgFrame->Data[i] >> 7) & 0x01;
    }

    if (remainder > 0)
    {
        BYTE val = pMsgFrame->Data[max_bytes];
        for (i = 0; i < remainder; i++)
        {
            pOutStat[max_bytes * 8 + i] = (val >> i) & 0x01;
        }
    }

    TRACED("Parse_S2C_Cell_ChargeBalanceStatus nGroupIndex:[%d], max_bytes:[%d], remainder:[%d]\n", 
        nGroupIndex, max_bytes, remainder);

    return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/25 13:55:12
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_Cell_DisChargeBalanceStatus(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    int nMainIndex = pMsgFrame->ID_MainIndex;
    BYTE *pOutStat;
    int max_cells, max_bytes, remainder;
    int i;

    int nGroupIndex = GetGroupIndexByMainIndex(nMainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_SINGLE_STBATT_NUM)
    {
        return FALSE;
    }

    pOutStat = &s_infy_BmtBattCellData[nGroupIndex].byCellDischargeStat[0];

    max_cells = (BATT_CELL_MAX_NUM < 416) ? BATT_CELL_MAX_NUM : 416;
    max_bytes = max_cells / 8; // 每字节8个电芯状态
    remainder = max_cells % 8;

    for (i = 0; i < max_bytes; i++)
    {
        pOutStat[i * 8 + 0] = (pMsgFrame->Data[i] >> 0) & 0x01;
        pOutStat[i * 8 + 1] = (pMsgFrame->Data[i] >> 1) & 0x01;
        pOutStat[i * 8 + 2] = (pMsgFrame->Data[i] >> 2) & 0x01;
        pOutStat[i * 8 + 3] = (pMsgFrame->Data[i] >> 3) & 0x01;
        pOutStat[i * 8 + 4] = (pMsgFrame->Data[i] >> 4) & 0x01;
        pOutStat[i * 8 + 5] = (pMsgFrame->Data[i] >> 5) & 0x01;
        pOutStat[i * 8 + 6] = (pMsgFrame->Data[i] >> 6) & 0x01;
        pOutStat[i * 8 + 7] = (pMsgFrame->Data[i] >> 7) & 0x01;
    }

    if (remainder > 0)
    {
        BYTE val = pMsgFrame->Data[max_bytes];
        for (i = 0; i < remainder; i++)
        {
            pOutStat[max_bytes * 8 + i] = (val >> i) & 0x01;
        }
    }

    TRACED("Parse_S2C_Cell_DisChargeBalanceStatus nGroupIndex:[%d], max_bytes:[%d], remainder:[%d]\n", 
        nGroupIndex, max_bytes, remainder);

    return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/25 13:56:57
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_CellVoltage(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    int nMainIndex = pMsgFrame->ID_MainIndex;
    int copyLen;

    int nGroupIndex = GetGroupIndexByMainIndex(nMainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_SINGLE_STBATT_NUM)
    {
        return FALSE;
    }

    if (pMsgFrame->ID_Datalen < 8)
    {
        return FALSE; 
    }

    copyLen = pMsgFrame->ID_Datalen - 8;

    memcpy(s_infy_BmtBattCellData[nGroupIndex].usCellVolt, pMsgFrame->Data, copyLen);

    TRACED("Parse_S2C_CellVoltage nGroupIndex:[%d], copyLen:[%d]\n", 
        nGroupIndex, copyLen);

    return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/25 13:57:27
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_CellTemperature(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    int nMainIndex = pMsgFrame->ID_MainIndex;
    int copyLen;

    int nGroupIndex = GetGroupIndexByMainIndex(nMainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_SINGLE_STBATT_NUM)
    {
        return FALSE;
    }

    if (pMsgFrame->ID_Datalen < 8)
    {
        return FALSE; 
    }

    copyLen = pMsgFrame->ID_Datalen - 8;

    memcpy(s_infy_BmtBattCellData[nGroupIndex].sCellTemp, pMsgFrame->Data, copyLen);

    TRACED("Parse_S2C_CellTemperature nGroupIndex:[%d], copyLen:[%d]\n", 
        nGroupIndex, copyLen);

    return TRUE;
}

static BOOL Parse_S2C_CellResReal(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    int nMainIndex = pMsgFrame->ID_MainIndex;
    int copyLen;

    int nGroupIndex = GetGroupIndexByMainIndex(nMainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_SINGLE_STBATT_NUM)
    {
        return FALSE;
    }

    if (pMsgFrame->ID_Datalen < 8)
    {
        return FALSE; 
    }

    copyLen = pMsgFrame->ID_Datalen - 8 -3*4;

    memcpy(s_infy_BmtBattCellData[nGroupIndex].uiCellResReal + (nMainIndex%2)*416/2, pMsgFrame->Data+3*4, copyLen);

    TRACED("Parse_S2C_CellResReal nGroupIndex:[%d], copyLen:[%d]\n", 
        nGroupIndex, copyLen);

    return TRUE;
}

static BOOL Parse_S2C_CellResImag(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    int nMainIndex = pMsgFrame->ID_MainIndex;
    int copyLen;

    int nGroupIndex = GetGroupIndexByMainIndex(nMainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_SINGLE_STBATT_NUM)
    {
        return FALSE;
    }

    if (pMsgFrame->ID_Datalen < 8)
    {
        return FALSE; 
    }

    copyLen = pMsgFrame->ID_Datalen - 8 - 3*4; // -3*4 是 时间戳，先不处理

    memcpy(s_infy_BmtBattCellData[nGroupIndex].uiCellResImag + (nMainIndex%2)*416/2, pMsgFrame->Data+3*4, copyLen);

    TRACED("Parse_S2C_CellResImag nGroupIndex:[%d], copyLen:[%d]\n", 
        nGroupIndex, copyLen);

    return TRUE;
}

static BOOL Parse_S2C_CellIcStat(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    int nMainIndex = pMsgFrame->ID_MainIndex;
    int i;

    int nGroupIndex = GetGroupIndexByMainIndex(nMainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_SINGLE_STBATT_NUM)
    {
        return FALSE;
    }

    if (pMsgFrame->ID_Datalen < 8)
    {
        return FALSE; 
    }

    for(i=0; i<BATT_CELL_MAX_NUM; ++i)
    {
        if(i%2)
        {
            s_infy_BmtBattCellData[nGroupIndex].byCellIcStat[i] = (BYTE)((pMsgFrame->Data[i]>>4)&0x0F);
        }
        else
        {
            s_infy_BmtBattCellData[nGroupIndex].byCellIcStat[i] = (BYTE)(pMsgFrame->Data[i]&0x0F);
        }
    }


    TRACED("Parse_S2C_CellIcStat nGroupIndex:[%d], copyLen:[%d]\n", 
        nGroupIndex, copyLen);

    return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/25 13:57:50
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void YFY_Init_RoughData_RPMSG(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int n)
{
	//int nBattPackCellNum = 0;

    A_BLOB_SINGLE_STBATT_RUNINFO* pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];

    memset(s_infy_BmtBattCellData[n].byCellBalanceStat, 0, sizeof(s_infy_BmtBattCellData[n].byCellBalanceStat));
    memset(s_infy_BmtBattCellData[n].byCellDischargeStat, 0, sizeof(s_infy_BmtBattCellData[n].byCellDischargeStat));
    memset(s_infy_BmtBattCellData[n].usCellVolt, 0, sizeof(s_infy_BmtBattCellData[n].usCellVolt));
    memset(s_infy_BmtBattCellData[n].sCellTemp, 0, sizeof(s_infy_BmtBattCellData[n].sCellTemp));
    memset(s_infy_BmtBattCellData[n].uiCellResReal, 0, sizeof(s_infy_BmtBattCellData[n].uiCellResReal));
    memset(s_infy_BmtBattCellData[n].uiCellResImag, 0, sizeof(s_infy_BmtBattCellData[n].uiCellResImag));
    memset(s_infy_BmtBattCellData[n].fCellVolt, 0, sizeof(s_infy_BmtBattCellData[n].fCellVolt));
    memset(s_infy_BmtBattCellData[n].fCellTemp, 0, sizeof(s_infy_BmtBattCellData[n].fCellTemp));
    memset(s_infy_BmtBattCellData[n].fCellRes, 0, sizeof(s_infy_BmtBattCellData[n].fCellRes));

    return;
}

/**************************************************************************
                    03整体信息数据解析函数
 * 来源: YFY_BMU02_核间通信通信协议表(03整体信息)
 * 单位规则: NA→USHORT, 0.1→float×0.1f, 1E-3→USHORT千分比
 * 说明: 只解析pResp中的数据，通信层由外部实现
 *************************************************************************/

/* ---------- 数据转换辅助函数 ---------- */

/* 4字节转ULONG（用于累计电量等） */
static ULONG DatatoULONG(BYTE* InputData)
{
    ULONG ulvalue = (ULONG)InputData[0] 
                  + ((ULONG)InputData[1] << 8)
                  + ((ULONG)InputData[2] << 16)
                  + ((ULONG)InputData[3] << 24);
    return ulvalue;
}

/* ---------- A_BLOB_STATUS_STAT 解析函数 ---------- */

/* 主索引50: 状态信息 → pRough->StatusStat.status_info */
static void Parse_StatusInfo(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    if (!pMsgFrame || !pRough) return;
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1 */
    pRough->StatusStat.status_info[0].protect_status         = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->StatusStat.status_info[0].work_status             = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->StatusStat.status_info[0].charge_discharge_status = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->StatusStat.status_info[0].work_mode              = DatatoUSHORT(&pMsgFrame->Data[9]);
    /* 组2 */
    pRough->StatusStat.status_info[1].protect_status          = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->StatusStat.status_info[1].work_status              = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->StatusStat.status_info[1].charge_discharge_status  = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->StatusStat.status_info[1].work_mode               = DatatoUSHORT(&pMsgFrame->Data[17]);
    TRACEI("[状态信息] 组1: 保护=%u 工作=%u 充放电=%u 模式=%u\n",
           pRough->StatusStat.status_info[0].protect_status, pRough->StatusStat.status_info[0].work_status,
           pRough->StatusStat.status_info[0].charge_discharge_status, pRough->StatusStat.status_info[0].work_mode);
    TRACEI("[状态信息] 组2: 保护=%u 工作=%u 充放电=%u 模式=%u\n",
           pRough->StatusStat.status_info[1].protect_status, pRough->StatusStat.status_info[1].work_status,
           pRough->StatusStat.status_info[1].charge_discharge_status, pRough->StatusStat.status_info[1].work_mode);
}

/* 主索引51: 全局采样信息 → pRough->StatusStat.global_info */
static void Parse_GlobalInfo(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1: [0.1单位转float] */
    pRough->StatusStat.global_info[0].total_voltage     = (float)DatatoUSHORT(&pMsgFrame->Data[3]) * 0.1f;
    pRough->StatusStat.global_info[0].total_current     = (float)((short)DatatoUSHORT(&pMsgFrame->Data[5])) * 0.1f;
    pRough->StatusStat.global_info[0].output_power      = (float)((short)DatatoUSHORT(&pMsgFrame->Data[7])) * 0.1f;
    pRough->StatusStat.global_info[0].precharge_voltage = (float)DatatoUSHORT(&pMsgFrame->Data[9]) * 0.1f;
    
    /* 组2: 字节8-15 */
    pRough->StatusStat.global_info[1].total_voltage      = (float)DatatoUSHORT(&pMsgFrame->Data[11]) * 0.1f;
    pRough->StatusStat.global_info[1].total_current      = (float)((short)DatatoUSHORT(&pMsgFrame->Data[13])) * 0.1f;
    pRough->StatusStat.global_info[1].output_power       = (float)((short)DatatoUSHORT(&pMsgFrame->Data[15])) * 0.1f;
    pRough->StatusStat.global_info[1].precharge_voltage  = (float)DatatoUSHORT(&pMsgFrame->Data[17]) * 0.1f;
    TRACED("111 [全局采样] 组1: 总压=%.1fV 总流=%.1fA 功率=%.1fkW 预充=%.1fV\n",
           pRough->StatusStat.global_info[0].total_voltage, pRough->StatusStat.global_info[0].total_current,
           pRough->StatusStat.global_info[0].output_power, pRough->StatusStat.global_info[0].precharge_voltage);
    TRACED("222 [全局采样] 组2: 总压=%.1fV 总流=%.1fA 功率=%.1fkW 预充=%.1fV\n",
           pRough->StatusStat.global_info[1].total_voltage, pRough->StatusStat.global_info[1].total_current,
           pRough->StatusStat.global_info[1].output_power, pRough->StatusStat.global_info[1].precharge_voltage);
}

/* 主索引53: 限值信息 → pRough->StatusStat.limit_info */
static void Parse_LimitInfo(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1 */
    pRough->StatusStat.limit_info[0].max_charge_current    = (float)DatatoUSHORT(&pMsgFrame->Data[3]) * 0.1f;
    pRough->StatusStat.limit_info[0].max_discharge_current = (float)DatatoUSHORT(&pMsgFrame->Data[5]) * 0.1f;
    pRough->StatusStat.limit_info[0].max_charge_power      = (float)DatatoUSHORT(&pMsgFrame->Data[7]) * 0.1f;
    pRough->StatusStat.limit_info[0].max_discharge_power   = (float)DatatoUSHORT(&pMsgFrame->Data[9]) * 0.1f;
    pRough->StatusStat.limit_info[0].max_charge_voltage    = (float)DatatoUSHORT(&pMsgFrame->Data[11]) * 0.1f;
    pRough->StatusStat.limit_info[0].max_discharge_voltage = (float)DatatoUSHORT(&pMsgFrame->Data[13]) * 0.1f;
    /* 组2 */
    pRough->StatusStat.limit_info[1].max_charge_current     = (float)DatatoUSHORT(&pMsgFrame->Data[15]) * 0.1f;
    pRough->StatusStat.limit_info[1].max_discharge_current  = (float)DatatoUSHORT(&pMsgFrame->Data[17]) * 0.1f;
    pRough->StatusStat.limit_info[1].max_charge_power       = (float)DatatoUSHORT(&pMsgFrame->Data[19]) * 0.1f;
    pRough->StatusStat.limit_info[1].max_discharge_power    = (float)DatatoUSHORT(&pMsgFrame->Data[21]) * 0.1f;
    pRough->StatusStat.limit_info[1].max_charge_voltage     = (float)DatatoUSHORT(&pMsgFrame->Data[23]) * 0.1f;
    pRough->StatusStat.limit_info[1].max_discharge_voltage  = (float)DatatoUSHORT(&pMsgFrame->Data[25]) * 0.1f;
    TRACEI("[限值信息] 组1: 充电流限=%.1f 放电流限=%.1f 充功率限=%.1f 放功率限=%.1f 充电压限=%.1f 放电压限=%.1f\n",
           pRough->StatusStat.limit_info[0].max_charge_current, pRough->StatusStat.limit_info[0].max_discharge_current,
           pRough->StatusStat.limit_info[0].max_charge_power, pRough->StatusStat.limit_info[0].max_discharge_power,
           pRough->StatusStat.limit_info[0].max_charge_voltage, pRough->StatusStat.limit_info[0].max_discharge_voltage);
    TRACEI("[限值信息] 组2: 充电流限=%.1f 放电流限=%.1f 充功率限=%.1f 放功率限=%.1f 充电压限=%.1f 放电压限=%.1f\n",
           pRough->StatusStat.limit_info[1].max_charge_current, pRough->StatusStat.limit_info[1].max_discharge_current,
           pRough->StatusStat.limit_info[1].max_charge_power, pRough->StatusStat.limit_info[1].max_discharge_power,
           pRough->StatusStat.limit_info[1].max_charge_voltage, pRough->StatusStat.limit_info[1].max_discharge_voltage);
}

/* 主索引61: 安全等级统计 → pRough->StatusStat.sos_stat */
static void Parse_SosStat(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1 */
    pRough->DiagStat.sos_stat[0].current_sos_value = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->DiagStat.sos_stat[0].current_sos_reason = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->DiagStat.sos_stat[0].predict_sos_value = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->DiagStat.sos_stat[0].predict_sos_reason = DatatoUSHORT(&pMsgFrame->Data[9]);
    /* 组2 */
    pRough->DiagStat.sos_stat[1].current_sos_value = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->DiagStat.sos_stat[1].current_sos_reason = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->DiagStat.sos_stat[1].predict_sos_value = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->DiagStat.sos_stat[1].predict_sos_reason = DatatoUSHORT(&pMsgFrame->Data[17]);
    TRACEI("[安全等级] 组1: 当前SOS=%u 原因=0x%04X 预测SOS=%u 原因=0x%04X\n",
           pRough->DiagStat.sos_stat[0].current_sos_value, pRough->DiagStat.sos_stat[0].current_sos_reason,
           pRough->DiagStat.sos_stat[0].predict_sos_value, pRough->DiagStat.sos_stat[0].predict_sos_reason);
    TRACEI("[安全等级] 组2: 当前SOS=%u 原因=0x%04X 预测SOS=%u 原因=0x%04X\n",
           pRough->DiagStat.sos_stat[1].current_sos_value, pRough->DiagStat.sos_stat[1].current_sos_reason,
           pRough->DiagStat.sos_stat[1].predict_sos_value, pRough->DiagStat.sos_stat[1].predict_sos_reason);
}


/* ---------- A_BLOB_ENERGY_STAT 解析函数 ---------- */

/* 主索引52: 电量信息 → pRough->EnergyStatus.power_info */
static void Parse_PowerInfo(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1 */
    pRough->EnergyStatus.power_info[0].soc             = DatatoUSHORT(&pMsgFrame->Data[3]) * 0.1;
    pRough->EnergyStatus.power_info[0].soe             = DatatoUSHORT(&pMsgFrame->Data[5]) * 0.1;
    pRough->EnergyStatus.power_info[0].soh             = DatatoUSHORT(&pMsgFrame->Data[7]) * 0.1;
    pRough->EnergyStatus.power_info[0].total_capacity  = (float)DatatoUSHORT(&pMsgFrame->Data[9]) * 0.1f;
    /* 组2 */
    pRough->EnergyStatus.power_info[1].soc              = DatatoUSHORT(&pMsgFrame->Data[11]) * 0.1;
    pRough->EnergyStatus.power_info[1].soe              = DatatoUSHORT(&pMsgFrame->Data[13]) * 0.1;
    pRough->EnergyStatus.power_info[1].soh              = DatatoUSHORT(&pMsgFrame->Data[15]) * 0.1;
    pRough->EnergyStatus.power_info[1].total_capacity   = (float)DatatoUSHORT(&pMsgFrame->Data[17]) * 0.1f;
    TRACEI("[电量信息] 组1: SOC=%u SOE=%u SOH=%u 总容量=%.1fAH\n",
           pRough->EnergyStatus.power_info[0].soc, pRough->EnergyStatus.power_info[0].soe,
           pRough->EnergyStatus.power_info[0].soh, pRough->EnergyStatus.power_info[0].total_capacity);
    TRACEI("[电量信息] 组2: SOC=%u SOE=%u SOH=%u 总容量=%.1fAH\n",
           pRough->EnergyStatus.power_info[1].soc, pRough->EnergyStatus.power_info[1].soe,
           pRough->EnergyStatus.power_info[1].soh, pRough->EnergyStatus.power_info[1].total_capacity);
}

/* 主索引55: 当前能量统计 → pRough->EnergyStatus.energy_curr_stat */
static void Parse_EnergyCurrStat(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1 */
    pRough->EnergyStatus.energy_curr_stat[0].remaining_energy = (float)DatatoUSHORT(&pMsgFrame->Data[3]) * 0.1f;
    pRough->EnergyStatus.energy_curr_stat[0].total_energy = (float)DatatoUSHORT(&pMsgFrame->Data[5]) * 0.1f;
    pRough->EnergyStatus.energy_curr_stat[0].single_charge_energy = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->EnergyStatus.energy_curr_stat[0].single_discharge_energy = DatatoUSHORT(&pMsgFrame->Data[9]);
    /* 组2 */
    pRough->EnergyStatus.energy_curr_stat[1].remaining_energy = (float)DatatoUSHORT(&pMsgFrame->Data[11]) * 0.1f;
    pRough->EnergyStatus.energy_curr_stat[1].total_energy = (float)DatatoUSHORT(&pMsgFrame->Data[13]) * 0.1f;
    pRough->EnergyStatus.energy_curr_stat[1].single_charge_energy = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->EnergyStatus.energy_curr_stat[1].single_discharge_energy = DatatoUSHORT(&pMsgFrame->Data[17]);
    TRACEI("[当前能量] 组1: 剩余=%.1fkWh 总电能=%.1fkWh 单充=%uAH 单放=%uAH\n",
           pRough->EnergyStatus.energy_curr_stat[0].remaining_energy, pRough->EnergyStatus.energy_curr_stat[0].total_energy,
           pRough->EnergyStatus.energy_curr_stat[0].single_charge_energy, pRough->EnergyStatus.energy_curr_stat[0].single_discharge_energy);
    TRACEI("[当前能量] 组2: 剩余=%.1fkWh 总电能=%.1fkWh 单充=%uAH 单放=%uAH\n",
           pRough->EnergyStatus.energy_curr_stat[1].remaining_energy, pRough->EnergyStatus.energy_curr_stat[1].total_energy,
           pRough->EnergyStatus.energy_curr_stat[1].single_charge_energy, pRough->EnergyStatus.energy_curr_stat[1].single_discharge_energy);
}

/* 主索引56: 累计电量统计 → pRough->EnergyStatus.energy_total_stat */
static void Parse_EnergyTotalStat(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1 */
    pRough->EnergyStatus.energy_total_stat[0].total_charge_energy = DatatoULONG(&pMsgFrame->Data[3]);
    pRough->EnergyStatus.energy_total_stat[0].total_discharge_energy = DatatoULONG(&pMsgFrame->Data[7]);
    pRough->EnergyStatus.energy_total_stat[0].total_charge_power = (float)DatatoUSHORT(&pMsgFrame->Data[11]) * 0.1f;
    pRough->EnergyStatus.energy_total_stat[0].total_discharge_power = (float)DatatoUSHORT(&pMsgFrame->Data[13]) * 0.1f;
    /* 组2 */
    pRough->EnergyStatus.energy_total_stat[1].total_charge_energy = DatatoULONG(&pMsgFrame->Data[15]);
    pRough->EnergyStatus.energy_total_stat[1].total_discharge_energy = DatatoULONG(&pMsgFrame->Data[19]);
    pRough->EnergyStatus.energy_total_stat[1].total_charge_power = (float)DatatoUSHORT(&pMsgFrame->Data[23]) * 0.1f;
    pRough->EnergyStatus.energy_total_stat[1].total_discharge_power = (float)DatatoUSHORT(&pMsgFrame->Data[25]) * 0.1f;
    TRACEI("[累计电量] 组1: 充电=%luAH 放电=%luAH 充电能=%.1fkWh 放电能=%.1fkWh\n",
           (unsigned long)pRough->EnergyStatus.energy_total_stat[0].total_charge_energy,
           (unsigned long)pRough->EnergyStatus.energy_total_stat[0].total_discharge_energy,
           pRough->EnergyStatus.energy_total_stat[0].total_charge_power, pRough->EnergyStatus.energy_total_stat[0].total_discharge_power);
    TRACEI("[累计电量] 组2: 充电=%luAH 放电=%luAH 充电能=%.1fkWh 放电能=%.1fkWh\n",
           (unsigned long)pRough->EnergyStatus.energy_total_stat[1].total_charge_energy,
           (unsigned long)pRough->EnergyStatus.energy_total_stat[1].total_discharge_energy,
           pRough->EnergyStatus.energy_total_stat[1].total_charge_power, pRough->EnergyStatus.energy_total_stat[1].total_discharge_power);
}

/* 主索引62: 放电深度统计 → pRough->EnergyStatus.dod_stat */
static void Parse_DodStat(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1 */
    pRough->EnergyStatus.dod_stat[0].current_dod = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->EnergyStatus.dod_stat[0].start_dod = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->EnergyStatus.dod_stat[0].history_max_dod = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->EnergyStatus.dod_stat[0].history_avg_dod = DatatoUSHORT(&pMsgFrame->Data[9]);
    /* 组2 */
    pRough->EnergyStatus.dod_stat[1].current_dod = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->EnergyStatus.dod_stat[1].start_dod = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->EnergyStatus.dod_stat[1].history_max_dod = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->EnergyStatus.dod_stat[1].history_avg_dod = DatatoUSHORT(&pMsgFrame->Data[17]);
    TRACEI("[放电深度] 组1: 当前=%u 起始=%u 历史最大=%u 历史平均=%u\n",
           pRough->EnergyStatus.dod_stat[0].current_dod, pRough->EnergyStatus.dod_stat[0].start_dod,
           pRough->EnergyStatus.dod_stat[0].history_max_dod, pRough->EnergyStatus.dod_stat[0].history_avg_dod);
    TRACEI("[放电深度] 组2: 当前=%u 起始=%u 历史最大=%u 历史平均=%u\n",
           pRough->EnergyStatus.dod_stat[1].current_dod, pRough->EnergyStatus.dod_stat[1].start_dod,
           pRough->EnergyStatus.dod_stat[1].history_max_dod, pRough->EnergyStatus.dod_stat[1].history_avg_dod);
}

/* 将 DOD 历史记录 append-only 写入 BmsRunData.data 的 Data_DODHistory 表 */
static void LogOut_DODHistory(const HIS_DOD_RECORD* pRec, int nGroupIndex)
{
    if (pRec == NULL) return;

    sqlite3* pDb = Open_Lock_Database(DB_TYPE_BMSDATA);
    if (pDb == NULL)
    {
        TRACEI("LogOut_DODHistory: Open_Lock_Database(BMSDATA) failed\n");
        return;
    }

    sqlite3_stmt* stat = NULL;
    const char* szSql =
        "INSERT INTO Data_DODHistory "
        "(Group_No, Seq_Id, Time_Year, Time_MonthDay, Time_HourMinute, Time_Second, "
        "Dod, Dischg_Qty, Chrg_Qty, Dischg_Energy, Chrg_Energy, Save_Time) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?);";
    int nRet = sqlite3_prepare_v2(pDb, szSql, -1, &stat, NULL);
    if (nRet != SQLITE_OK)
    {
        TRACEI("LogOut_DODHistory: prepare failed [%d][%s]\n", nRet, sqlite3_errmsg(pDb));
        Close_UnLock_Database(DB_TYPE_BMSDATA);
        return;
    }

    sqlite3_bind_int(stat, 1, nGroupIndex);
    sqlite3_bind_int(stat, 2, (int)pRec->seq_id);
    sqlite3_bind_int(stat, 3, (int)pRec->time_year);
    sqlite3_bind_int(stat, 4, (int)pRec->time_month_day);
    sqlite3_bind_int(stat, 5, (int)pRec->time_hour_minute);
    sqlite3_bind_int(stat, 6, (int)pRec->time_second);
    sqlite3_bind_int(stat, 7, (int)pRec->dod);
    sqlite3_bind_int(stat, 8, (int)pRec->dischg_qty);
    sqlite3_bind_int(stat, 9, (int)pRec->chrg_qty);
    sqlite3_bind_int(stat, 10, (int)pRec->dischg_energy);
    sqlite3_bind_int(stat, 11, (int)pRec->chrg_energy);
    sqlite3_bind_int64(stat, 12, (sqlite3_int64)time(NULL));

    nRet = sqlite3_step(stat);
    if (nRet != SQLITE_DONE)
    {
        TRACEI("LogOut_DODHistory: step failed [%d][%s]\n", nRet, sqlite3_errmsg(pDb));
    }
    sqlite3_finalize(stat);
    Close_UnLock_Database(DB_TYPE_BMSDATA);
}

/* 主索引15/16: DOD历史记录 → pRough->stHisDODRecord[] + 落库 Data_DODHistory */
static BOOL Parse_DODRecord(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    if (!pMsgFrame || !pRough) return FALSE;

    int nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_BATT_GROUP_NUM) return FALSE;

    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }

    HIS_DOD_RECORD* pRec = &pRough->stHisDODRecord[nGroupIndex];
    pRec->seq_id           = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRec->time_year        = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRec->time_month_day   = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRec->time_hour_minute = DatatoUSHORT(&pMsgFrame->Data[9]);
    pRec->time_second      = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRec->dod              = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRec->dischg_qty       = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRec->chrg_qty         = DatatoUSHORT(&pMsgFrame->Data[17]);
    pRec->dischg_energy    = DatatoUSHORT(&pMsgFrame->Data[19]);
    pRec->chrg_energy      = DatatoUSHORT(&pMsgFrame->Data[21]);

    TRACEI("[DOD历史] 组%d: seq=%u %u-%02u-%02u %02u:%02u:%02u DOD=%u 放电量=%u 充电量=%u 放电能=%u 充电能=%u\n",
           nGroupIndex+1, pRec->seq_id,
           pRec->time_year, pRec->time_month_day>>8, pRec->time_month_day&0xFF,
           pRec->time_hour_minute>>8, pRec->time_hour_minute&0xFF, pRec->time_second,
           pRec->dod, pRec->dischg_qty, pRec->chrg_qty, pRec->dischg_energy, pRec->chrg_energy);

    LogOut_DODHistory(pRec, nGroupIndex);
    return TRUE;
}

/* 将 SOS 历史记录 append-only 写入 BmsRunData.data 的 Data_SOSHistory 表 */
static void LogOut_SOSHistory(const HIS_SOS_RECORD* pRec, int nGroupIndex)
{
    if (pRec == NULL) return;

    sqlite3* pDb = Open_Lock_Database(DB_TYPE_BMSDATA);
    if (pDb == NULL)
    {
        TRACEI("LogOut_SOSHistory: Open_Lock_Database(BMSDATA) failed\n");
        return;
    }

    sqlite3_stmt* stat = NULL;
    const char* szSql =
        "INSERT INTO Data_SOSHistory "
        "(Group_No, Seq_Id, Time_Year, Time_MonthDay, Time_HourMinute, Time_Second, "
        "Sos_Level, Reason_Bitmap, Event_Type, Reserved_1, Reserved_2, Save_Time) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?);";
    int nRet = sqlite3_prepare_v2(pDb, szSql, -1, &stat, NULL);
    if (nRet != SQLITE_OK)
    {
        TRACEI("LogOut_SOSHistory: prepare failed [%d][%s]\n", nRet, sqlite3_errmsg(pDb));
        Close_UnLock_Database(DB_TYPE_BMSDATA);
        return;
    }

    sqlite3_bind_int(stat, 1, nGroupIndex);
    sqlite3_bind_int(stat, 2, (int)pRec->seq_id);
    sqlite3_bind_int(stat, 3, (int)pRec->time_year);
    sqlite3_bind_int(stat, 4, (int)pRec->time_month_day);
    sqlite3_bind_int(stat, 5, (int)pRec->time_hour_minute);
    sqlite3_bind_int(stat, 6, (int)pRec->time_second);
    sqlite3_bind_int(stat, 7, (int)pRec->sos_level);
    sqlite3_bind_int(stat, 8, (int)pRec->reason_bitmap);
    sqlite3_bind_int(stat, 9, (int)pRec->event_type);
    sqlite3_bind_int(stat, 10, (int)pRec->reserved_1);
    sqlite3_bind_int(stat, 11, (int)pRec->reserved_2);
    sqlite3_bind_int64(stat, 12, (sqlite3_int64)time(NULL));

    nRet = sqlite3_step(stat);
    if (nRet != SQLITE_DONE)
    {
        TRACEI("LogOut_SOSHistory: step failed [%d][%s]\n", nRet, sqlite3_errmsg(pDb));
    }
    sqlite3_finalize(stat);
    Close_UnLock_Database(DB_TYPE_BMSDATA);
}

/* 主索引17/18: SOS历史记录 → pRough->stHisSOSRecord[] + 落库 Data_SOSHistory
 * 含seq去重: 同组序号与上一条相同则丢弃(重发/重复帧),序号回绕(u16)时放行 */
static BOOL Parse_SOSRecord(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    if (!pMsgFrame || !pRough) return FALSE;

    int nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_BATT_GROUP_NUM) return FALSE;

    HIS_SOS_RECORD* pRec = &pRough->stHisSOSRecord[nGroupIndex];
    USHORT nNewSeq = DatatoUSHORT(&pMsgFrame->Data[3]);

    /*seq去重: 与上一条相同丢掉(重复帧);回绕(新seq<旧seq)放行*/
    if ((pRec->seq_id != 0) && (nNewSeq == pRec->seq_id))
    {
        TRACEI("[SOS历史] 组%d: seq=%u 重复,丢弃\n", nGroupIndex+1, nNewSeq);
        return TRUE;
    }

    pRec->seq_id           = nNewSeq;
    pRec->time_year        = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRec->time_month_day   = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRec->time_hour_minute = DatatoUSHORT(&pMsgFrame->Data[9]);
    pRec->time_second      = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRec->sos_level        = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRec->reason_bitmap    = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRec->event_type       = DatatoUSHORT(&pMsgFrame->Data[17]);
    pRec->reserved_1       = DatatoUSHORT(&pMsgFrame->Data[19]);
    pRec->reserved_2       = DatatoUSHORT(&pMsgFrame->Data[21]);

    TRACEI("[SOS历史] 组%d: seq=%u %u-%02u-%02u %02u:%02u:%02u 等级=%u 原因=0x%04X 事件=%u\n",
           nGroupIndex+1, pRec->seq_id,
           pRec->time_year, pRec->time_month_day>>8, pRec->time_month_day&0xFF,
           pRec->time_hour_minute>>8, pRec->time_hour_minute&0xFF, pRec->time_second,
           pRec->sos_level, pRec->reason_bitmap, pRec->event_type);

    LogOut_SOSHistory(pRec, nGroupIndex);
    return TRUE;
}

static void Parse_FeatureInfo(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    if (!pMsgFrame || !pRough) return;
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    pRough->StatusStat.feature_info[0].cell_total_voltage = (float)DatatoSHORT(&pMsgFrame->Data[3]) * 0.1f;
    pRough->StatusStat.feature_info[0].cell_avg_voltage = DatatoSHORT(&pMsgFrame->Data[5]);
    pRough->StatusStat.feature_info[0].cell_avg_temperature = (float)DatatoSHORT(&pMsgFrame->Data[7]) * 0.1f;
    pRough->StatusStat.feature_info[0].hvbox_temperature = (float)DatatoSHORT(&pMsgFrame->Data[9]) * 0.1f;
    pRough->StatusStat.feature_info[0].max_voltage_index = DatatoSHORT(&pMsgFrame->Data[11]);
    pRough->StatusStat.feature_info[0].max_voltage_value = DatatoSHORT(&pMsgFrame->Data[13]);
    pRough->StatusStat.feature_info[0].min_voltage_index = DatatoSHORT(&pMsgFrame->Data[15]);
    pRough->StatusStat.feature_info[0].min_voltage_value = DatatoSHORT(&pMsgFrame->Data[17]);
    pRough->StatusStat.feature_info[0].max_temperature_index = DatatoSHORT(&pMsgFrame->Data[19]);
    pRough->StatusStat.feature_info[0].max_temperature_value = (float)DatatoSHORT(&pMsgFrame->Data[21]) * 0.1f;
    pRough->StatusStat.feature_info[0].min_temperature_index = DatatoSHORT(&pMsgFrame->Data[23]);
    pRough->StatusStat.feature_info[0].min_temperature_value = (float)DatatoSHORT(&pMsgFrame->Data[25]) * 0.1f;
    pRough->StatusStat.feature_info[1].cell_total_voltage = (float)DatatoSHORT(&pMsgFrame->Data[27]) * 0.1f;
    pRough->StatusStat.feature_info[1].cell_avg_voltage = DatatoSHORT(&pMsgFrame->Data[29]);
    pRough->StatusStat.feature_info[1].cell_avg_temperature = (float)DatatoSHORT(&pMsgFrame->Data[31]) * 0.1f;
    pRough->StatusStat.feature_info[1].hvbox_temperature = (float)DatatoSHORT(&pMsgFrame->Data[33]) * 0.1f;
    pRough->StatusStat.feature_info[1].max_voltage_index = DatatoSHORT(&pMsgFrame->Data[35]);
    pRough->StatusStat.feature_info[1].max_voltage_value = DatatoSHORT(&pMsgFrame->Data[37]);
    pRough->StatusStat.feature_info[1].min_voltage_index = DatatoSHORT(&pMsgFrame->Data[39]);
    pRough->StatusStat.feature_info[1].min_voltage_value = DatatoSHORT(&pMsgFrame->Data[41]);
    pRough->StatusStat.feature_info[1].max_temperature_index = DatatoSHORT(&pMsgFrame->Data[43]);
    pRough->StatusStat.feature_info[1].max_temperature_value = (float)DatatoSHORT(&pMsgFrame->Data[45]) * 0.1f;
    pRough->StatusStat.feature_info[1].min_temperature_index = DatatoSHORT(&pMsgFrame->Data[47]);
    pRough->StatusStat.feature_info[1].min_temperature_value = (float)DatatoSHORT(&pMsgFrame->Data[49]) * 0.1f;
    TRACEI("[特征信息] 组1: 总压=%.1fV 均压=%d 均温=%.1f 箱温=%.1f 最大V[%u]=%d 最小V[%u]=%d 最高T[%u]=%.1f 最低T[%u]=%.1f\n",
           pRough->StatusStat.feature_info[0].cell_total_voltage, (int)pRough->StatusStat.feature_info[0].cell_avg_voltage,
           pRough->StatusStat.feature_info[0].cell_avg_temperature, pRough->StatusStat.feature_info[0].hvbox_temperature,
           pRough->StatusStat.feature_info[0].max_voltage_index, (int)pRough->StatusStat.feature_info[0].max_voltage_value,
           pRough->StatusStat.feature_info[0].min_voltage_index, (int)pRough->StatusStat.feature_info[0].min_voltage_value,
           pRough->StatusStat.feature_info[0].max_temperature_index, pRough->StatusStat.feature_info[0].max_temperature_value,
           pRough->StatusStat.feature_info[0].min_temperature_index, pRough->StatusStat.feature_info[0].min_temperature_value);
    TRACEI("[特征信息] 组2: 总压=%.1fV 均压=%d 均温=%.1f 箱温=%.1f 最大V[%u]=%d 最小V[%u]=%d 最高T[%u]=%.1f 最低T[%u]=%.1f\n",
           pRough->StatusStat.feature_info[1].cell_total_voltage, (int)pRough->StatusStat.feature_info[1].cell_avg_voltage,
           pRough->StatusStat.feature_info[1].cell_avg_temperature, pRough->StatusStat.feature_info[1].hvbox_temperature,
           pRough->StatusStat.feature_info[1].max_voltage_index, (int)pRough->StatusStat.feature_info[1].max_voltage_value,
           pRough->StatusStat.feature_info[1].min_voltage_index, (int)pRough->StatusStat.feature_info[1].min_voltage_value,
           pRough->StatusStat.feature_info[1].max_temperature_index, pRough->StatusStat.feature_info[1].max_temperature_value,
           pRough->StatusStat.feature_info[1].min_temperature_index, pRough->StatusStat.feature_info[1].min_temperature_value);
}

static void Parse_HeartbeatInfo(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    if (!pMsgFrame || !pRough) return;
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    pRough->stHeartbeatInfo.heartbeat_hw_version = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->stHeartbeatInfo.device_id = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->stHeartbeatInfo.sw_version_l = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->stHeartbeatInfo.sw_version_h = DatatoUSHORT(&pMsgFrame->Data[9]);
    TRACEI("[心跳信息] HWver=%u DevId=%u SWver=%u.%u\n",
           pRough->stHeartbeatInfo.heartbeat_hw_version, pRough->stHeartbeatInfo.device_id,
           pRough->stHeartbeatInfo.sw_version_h, pRough->stHeartbeatInfo.sw_version_l);
}

static void Parse_HisFaultRecord(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame, int nGrp)
{
    if (!pMsgFrame || !pRough || nGrp < 0 || nGrp >= MAX_BATT_GROUP_NUM) return;
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    pRough->stHisFaultRecord[nGrp].fault_type = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->stHisFaultRecord[nGrp].fault_code = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->stHisFaultRecord[nGrp].time_year = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->stHisFaultRecord[nGrp].time_month_day = DatatoUSHORT(&pMsgFrame->Data[9]);
    pRough->stHisFaultRecord[nGrp].time_hour_minute = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->stHisFaultRecord[nGrp].time_second = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->stHisFaultRecord[nGrp].aux_info_1 = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->stHisFaultRecord[nGrp].aux_info_2 = DatatoUSHORT(&pMsgFrame->Data[17]);
    pRough->stHisFaultRecord[nGrp].aux_info_3 = DatatoUSHORT(&pMsgFrame->Data[19]);
    pRough->stHisFaultRecord[nGrp].aux_info_4 = DatatoUSHORT(&pMsgFrame->Data[21]);
    pRough->stHisFaultRecord[nGrp].aux_info_5 = DatatoUSHORT(&pMsgFrame->Data[23]);
    TRACEI("[历史故障] 组%d: 类型=%u 代码=%u %u-%02u-%02u %02u:%02u:%02u\n",
           nGrp+1, pRough->stHisFaultRecord[nGrp].fault_type, pRough->stHisFaultRecord[nGrp].fault_code,
           pRough->stHisFaultRecord[nGrp].time_year, pRough->stHisFaultRecord[nGrp].time_month_day>>8,
           pRough->stHisFaultRecord[nGrp].time_month_day&0xFF, pRough->stHisFaultRecord[nGrp].time_hour_minute>>8,
           pRough->stHisFaultRecord[nGrp].time_hour_minute&0xFF, pRough->stHisFaultRecord[nGrp].time_second);
}

// 历史故障事件上报：读取 stHisFaultRecord[nGrp]，组装 EVENT_BMS_HISTFAULT 并发送
static void Send_BmsHistFaultEvent(STBATT_SAMPLER_ROUGH* pRough, int nGrp)
{
    HIS_FAULT_RECORD* pSrc = &pRough->stHisFaultRecord[nGrp];
    EVENT_BMS_HISTFAULT evFault;
    ZERO_POBJS(&evFault, 1);
    evFault.nBattGrp   = nGrp;
    evFault.nFaultType = pSrc->fault_type;
    evFault.nFaultCode = pSrc->fault_code;
    struct tm tmFault;
    memset(&tmFault, 0, sizeof(tmFault));
    tmFault.tm_year = pSrc->time_year - 1900;
    tmFault.tm_mon  = ((pSrc->time_month_day >> 8) & 0xFF) - 1;
    tmFault.tm_mday = pSrc->time_month_day & 0xFF;
    tmFault.tm_hour = (pSrc->time_hour_minute >> 8) & 0xFF;
    tmFault.tm_min  = pSrc->time_hour_minute & 0xFF;
    tmFault.tm_sec  = pSrc->time_second;
    evFault.tmFault       = mktime(&tmFault);
    evFault.nProtectState = (pSrc->aux_info_1 >> 8) & 0xFF;
    evFault.nWorkState    = pSrc->aux_info_1 & 0xFF;
    evFault.nCurrent      = (SHORT)pSrc->aux_info_2;
    evFault.nCustom3      = pSrc->aux_info_3;
    evFault.nCustom4      = pSrc->aux_info_4;
    evFault.nCustom5      = pSrc->aux_info_5;
    SendMsg(pRough->hSampler, EVENT_TYPE_BMS_HISTFAULT,
        EVENT_MSG_BEGIN, &evFault, sizeof(EVENT_BMS_HISTFAULT));
}

// 历史事件上报：读取 stHisEventRecord[nGrp]，组装 EVENT_BMS_HISTEVENT 并发送
static void Send_BmsHistEventEvent(STBATT_SAMPLER_ROUGH* pRough, int nGrp)
{
    HIS_EVENT_RECORD* pSrc = &pRough->stHisEventRecord[nGrp];
    EVENT_BMS_HISTEVENT evEvent;
    ZERO_POBJS(&evEvent, 1);
    evEvent.nBattGrp   = nGrp;
    evEvent.nEventType = pSrc->event_type;
    evEvent.nEventCode = pSrc->event_code;
    struct tm tmEvent;
    memset(&tmEvent, 0, sizeof(tmEvent));
    tmEvent.tm_year = pSrc->time_year - 1900;
    tmEvent.tm_mon  = ((pSrc->time_month_day >> 8) & 0xFF) - 1;
    tmEvent.tm_mday = pSrc->time_month_day & 0xFF;
    tmEvent.tm_hour = (pSrc->time_hour_minute >> 8) & 0xFF;
    tmEvent.tm_min  = pSrc->time_hour_minute & 0xFF;
    tmEvent.tm_sec  = pSrc->time_second;
    evEvent.tmEvent       = mktime(&tmEvent);
    evEvent.nProtectState = (pSrc->aux_info_1 >> 8) & 0xFF;
    evEvent.nWorkState    = pSrc->aux_info_1 & 0xFF;
    evEvent.nCurrent      = (SHORT)pSrc->aux_info_2;
    evEvent.nCustom3      = pSrc->aux_info_3;
    evEvent.nCustom4      = pSrc->aux_info_4;
    evEvent.nCustom5      = pSrc->aux_info_5;
    SendMsg(pRough->hSampler, EVENT_TYPE_BMS_HISTEVENT,
        EVENT_MSG_BEGIN, &evEvent, sizeof(EVENT_BMS_HISTEVENT));
}

static void Parse_HisEventRecord(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame, int nGrp)
{
    if (!pMsgFrame || !pRough || nGrp < 0 || nGrp >= MAX_BATT_GROUP_NUM) return;
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    pRough->stHisEventRecord[nGrp].event_type = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->stHisEventRecord[nGrp].event_code = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->stHisEventRecord[nGrp].time_year = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->stHisEventRecord[nGrp].time_month_day = DatatoUSHORT(&pMsgFrame->Data[9]);
    pRough->stHisEventRecord[nGrp].time_hour_minute = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->stHisEventRecord[nGrp].time_second = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->stHisEventRecord[nGrp].aux_info_1 = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->stHisEventRecord[nGrp].aux_info_2 = DatatoUSHORT(&pMsgFrame->Data[17]);
    pRough->stHisEventRecord[nGrp].aux_info_3 = DatatoUSHORT(&pMsgFrame->Data[19]);
    pRough->stHisEventRecord[nGrp].aux_info_4 = DatatoUSHORT(&pMsgFrame->Data[21]);
    pRough->stHisEventRecord[nGrp].aux_info_5 = DatatoUSHORT(&pMsgFrame->Data[23]);
    TRACEI("[历史事件] 组%d: 类型=%u 代码=%u %u-%02u-%02u %02u:%02u:%02u\n",
           nGrp+1, pRough->stHisEventRecord[nGrp].event_type, pRough->stHisEventRecord[nGrp].event_code,
           pRough->stHisEventRecord[nGrp].time_year, pRough->stHisEventRecord[nGrp].time_month_day>>8,
           pRough->stHisEventRecord[nGrp].time_month_day&0xFF, pRough->stHisEventRecord[nGrp].time_hour_minute>>8,
           pRough->stHisEventRecord[nGrp].time_hour_minute&0xFF, pRough->stHisEventRecord[nGrp].time_second);
}

static void Parse_BoardSampleInfo(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    if (!pMsgFrame || !pRough) return;
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    pRough->stBoardSampleInfo.vcc_1v8_feedback = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->stBoardSampleInfo.vcc_3v3_feedback = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->stBoardSampleInfo.v_4g_3v5_feedback = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->stBoardSampleInfo.vcc_5v_feedback = DatatoUSHORT(&pMsgFrame->Data[9]);
    pRough->stBoardSampleInfo.vcch_5v_feedback = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->stBoardSampleInfo.vcc_12v_feedback = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->stBoardSampleInfo.temp_ad = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->stBoardSampleInfo.temp_ad1 = DatatoUSHORT(&pMsgFrame->Data[17]);
    pRough->stBoardSampleInfo.gpio_output = DatatoUSHORT(&pMsgFrame->Data[19]);
    pRough->stBoardSampleInfo.gpio_input = DatatoUSHORT(&pMsgFrame->Data[21]);
    pRough->stBoardSampleInfo.shiftreg_output = DatatoUSHORT(&pMsgFrame->Data[23]);
    pRough->stBoardSampleInfo.shiftreg_input_1 = DatatoUSHORT(&pMsgFrame->Data[25]);
    pRough->stBoardSampleInfo.shiftreg_input_2 = DatatoUSHORT(&pMsgFrame->Data[27]);
    TRACEI("[板级采样] 1v8=%u 3v3=%u 4g3v5=%u 5v=%u 5vh=%u 12v=%u TA=%u TA1=%u OUT=%u IN=%u SR=%u SR1=%u SR2=%u\n",
           pRough->stBoardSampleInfo.vcc_1v8_feedback, pRough->stBoardSampleInfo.vcc_3v3_feedback,
           pRough->stBoardSampleInfo.v_4g_3v5_feedback, pRough->stBoardSampleInfo.vcc_5v_feedback,
           pRough->stBoardSampleInfo.vcch_5v_feedback, pRough->stBoardSampleInfo.vcc_12v_feedback,
           pRough->stBoardSampleInfo.temp_ad, pRough->stBoardSampleInfo.temp_ad1,
           pRough->stBoardSampleInfo.gpio_output, pRough->stBoardSampleInfo.gpio_input,
           pRough->stBoardSampleInfo.shiftreg_output, pRough->stBoardSampleInfo.shiftreg_input_1,
           pRough->stBoardSampleInfo.shiftreg_input_2);
}

/* 主索引59: 时间统计 → pRough->DiagStat.time_stat */
static void Parse_TimeStat(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1: [整数单位] */
    pRough->DiagStat.time_stat[0].system_run_time = DatatoUSHORT(&pMsgFrame->Data[3]);          /* [1Hour] */
    pRough->DiagStat.time_stat[0].charge_available_time = DatatoUSHORT(&pMsgFrame->Data[5]);    /* [1Min] */
    pRough->DiagStat.time_stat[0].discharge_available_time = DatatoUSHORT(&pMsgFrame->Data[7]); /* [1Min] */
    pRough->DiagStat.time_stat[0].cycle_count = DatatoUSHORT(&pMsgFrame->Data[9]);              /* [1次] */
    
    /* 组2: 字节8-15 */
    pRough->DiagStat.time_stat[1].system_run_time = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->DiagStat.time_stat[1].charge_available_time = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->DiagStat.time_stat[1].discharge_available_time = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->DiagStat.time_stat[1].cycle_count = DatatoUSHORT(&pMsgFrame->Data[17]);
    TRACEI("[时间统计] 组1: 运行=%uH 可充电=%uMin 可放电=%uMin 循环=%u次\n",
           pRough->DiagStat.time_stat[0].system_run_time, pRough->DiagStat.time_stat[0].charge_available_time,
           pRough->DiagStat.time_stat[0].discharge_available_time, pRough->DiagStat.time_stat[0].cycle_count);
    TRACEI("[时间统计] 组2: 运行=%uH 可充电=%uMin 可放电=%uMin 循环=%u次\n",
           pRough->DiagStat.time_stat[1].system_run_time, pRough->DiagStat.time_stat[1].charge_available_time,
           pRough->DiagStat.time_stat[1].discharge_available_time, pRough->DiagStat.time_stat[1].cycle_count);
}

/* 主索引60: 故障统计 → pRough->DiagStat.fault_stat */
static void Parse_FaultStat(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    /* 组1: [NA] */
    pRough->DiagStat.fault_stat[0].monthly_alarm_count = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->DiagStat.fault_stat[0].monthly_fault_count = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->DiagStat.fault_stat[0].total_alarm_count = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->DiagStat.fault_stat[0].total_fault_count = DatatoUSHORT(&pMsgFrame->Data[9]);
    
    /* 组2: 字节8-15 */
    pRough->DiagStat.fault_stat[1].monthly_alarm_count = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->DiagStat.fault_stat[1].monthly_fault_count = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->DiagStat.fault_stat[1].total_alarm_count = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->DiagStat.fault_stat[1].total_fault_count = DatatoUSHORT(&pMsgFrame->Data[17]);
    TRACEI("[故障统计] 组1: 月告警=%u 月故障=%u 累计告警=%u 累计故障=%u\n",
           pRough->DiagStat.fault_stat[0].monthly_alarm_count, pRough->DiagStat.fault_stat[0].monthly_fault_count,
           pRough->DiagStat.fault_stat[0].total_alarm_count, pRough->DiagStat.fault_stat[0].total_fault_count);
    TRACEI("[故障统计] 组2: 月告警=%u 月故障=%u 累计告警=%u 累计故障=%u\n",
           pRough->DiagStat.fault_stat[1].monthly_alarm_count, pRough->DiagStat.fault_stat[1].monthly_fault_count,
           pRough->DiagStat.fault_stat[1].total_alarm_count, pRough->DiagStat.fault_stat[1].total_fault_count);
}

/* 主索引63: 权限状态 → pRough->DiagStat.auth_state (全局) */
static void Parse_AuthState(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    pRough->StatFeedback.auth_state.current_auth_level = DatatoUSHORT(&pMsgFrame->Data[3]);  /* [NA] */
    pRough->StatFeedback.auth_state.current_auth_port = DatatoUSHORT(&pMsgFrame->Data[5]);   /* [NA] */
    TRACEI("[权限状态] 级别=%u(0普通1运维2高级3超级) 端口=%u(0-4CAN 5-9RS485 10核间)\n",
           pRough->StatFeedback.auth_state.current_auth_level, pRough->StatFeedback.auth_state.current_auth_port);
}

/* 主索引70: 控制反馈 → pRough->DiagStat.ctrl_feedback (全局) */
static void Parse_CtrlFeedback(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    pRough->StatFeedback.ctrl_feedback.insulation_detect_result = DatatoUSHORT(&pMsgFrame->Data[3]); /* [NA] */
    pRough->StatFeedback.ctrl_feedback.reserved = DatatoUSHORT(&pMsgFrame->Data[5]);                  /* [NA] */
    TRACEI("[控制反馈] 绝缘检测=%u(0自动1检测中2停止3完成4超时) 预留=%u\n",
           pRough->StatFeedback.ctrl_feedback.insulation_detect_result, pRough->StatFeedback.ctrl_feedback.reserved);
}


/* ---------- 对外接口：解析03整体信息 ---------- */

/* 解析03整体信息数据并填充到pRough成员
 * 参数:
 *   pRough - 采样数据结构体指针
 *   mainIdx - 主索引(50-74)
 *   pResp - 已读取的响应数据(由外部通信层填充)
 * 返回:

/* ---------- 主索引57/58: 告警故障信息 ---------- */

/* 主索引57,58: 组1/2告警故障信息 → pRough->stBmsAlarmFault[0] */
static void Parse_AlarmFault(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }

    int nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_SINGLE_STBATT_NUM)
    {
        return FALSE;
    }
    /* alarm_info 1-10: 字节0-19 */
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_1 = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_2 = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_3 = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_4 = DatatoUSHORT(&pMsgFrame->Data[9]);
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_5 = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_6 = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_7 = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_8 = DatatoUSHORT(&pMsgFrame->Data[17]);
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_9 = DatatoUSHORT(&pMsgFrame->Data[19]);
    pRough->stBmsAlarmFault[nGroupIndex].alarm_info_10 = DatatoUSHORT(&pMsgFrame->Data[21]);
    
    /* fault_info 1-8: 字节20-35 */
    pRough->stBmsAlarmFault[nGroupIndex].fault_info_1 = DatatoUSHORT(&pMsgFrame->Data[23]);
    pRough->stBmsAlarmFault[nGroupIndex].fault_info_2 = DatatoUSHORT(&pMsgFrame->Data[25]);
    pRough->stBmsAlarmFault[nGroupIndex].fault_info_3 = DatatoUSHORT(&pMsgFrame->Data[27]);
    pRough->stBmsAlarmFault[nGroupIndex].fault_info_4 = DatatoUSHORT(&pMsgFrame->Data[29]);
    pRough->stBmsAlarmFault[nGroupIndex].fault_info_5 = DatatoUSHORT(&pMsgFrame->Data[31]);
    pRough->stBmsAlarmFault[nGroupIndex].fault_info_6 = DatatoUSHORT(&pMsgFrame->Data[33]);
    pRough->stBmsAlarmFault[nGroupIndex].fault_info_7 = DatatoUSHORT(&pMsgFrame->Data[35]);
    pRough->stBmsAlarmFault[nGroupIndex].fault_info_8 = DatatoUSHORT(&pMsgFrame->Data[37]);
    TRACEI("[告警故障组1] alarm: %u %u %u %u %u %u %u %u %u %u fault: %u %u %u %u %u %u %u %u\n",
           pRough->stBmsAlarmFault[nGroupIndex].alarm_info_1, pRough->stBmsAlarmFault[nGroupIndex].alarm_info_2,
           pRough->stBmsAlarmFault[nGroupIndex].alarm_info_3, pRough->stBmsAlarmFault[nGroupIndex].alarm_info_4,
           pRough->stBmsAlarmFault[nGroupIndex].alarm_info_5, pRough->stBmsAlarmFault[nGroupIndex].alarm_info_6,
           pRough->stBmsAlarmFault[nGroupIndex].alarm_info_7, pRough->stBmsAlarmFault[nGroupIndex].alarm_info_8,
           pRough->stBmsAlarmFault[nGroupIndex].alarm_info_9, pRough->stBmsAlarmFault[nGroupIndex].alarm_info_10,
           pRough->stBmsAlarmFault[nGroupIndex].fault_info_1, pRough->stBmsAlarmFault[nGroupIndex].fault_info_2,
           pRough->stBmsAlarmFault[nGroupIndex].fault_info_3, pRough->stBmsAlarmFault[nGroupIndex].fault_info_4,
           pRough->stBmsAlarmFault[nGroupIndex].fault_info_5, pRough->stBmsAlarmFault[nGroupIndex].fault_info_6,
           pRough->stBmsAlarmFault[nGroupIndex].fault_info_7, pRough->stBmsAlarmFault[nGroupIndex].fault_info_8);
}

/*************************************************************************
 * Parse_Imiu4Info - 解析主索引67: IMIU4信息
 * 字节布局: SHORT*16 + float*8 = 64字节
 *************************************************************************/
static void Parse_Imiu4Info(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    int nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);
    if (nGroupIndex < 0 || nGroupIndex >= MAX_BATT_GROUP_NUM) return FALSE;

    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }

    IMIU4_INFO* pImu4 = &pRough->stImiu4Info[nGroupIndex];

    pImu4->battery_total_voltage = DatatoSHORT(&pMsgFrame->Data[3]);
    pImu4->link_voltage          = DatatoSHORT(&pMsgFrame->Data[5]);
    pImu4->hall_5v_sample        = DatatoSHORT(&pMsgFrame->Data[7]);
    pImu4->hall_signal_sample    = DatatoSHORT(&pMsgFrame->Data[9]);
    pImu4->hall_current_low      = DatatoSHORT(&pMsgFrame->Data[11]);
    pImu4->hall_current_high     = DatatoSHORT(&pMsgFrame->Data[13]);
    pImu4->shunt_current_low     = DatatoSHORT(&pMsgFrame->Data[15]);
    pImu4->shunt_current_high    = DatatoSHORT(&pMsgFrame->Data[17]);
    pImu4->shunt_temperature     = DatatoSHORT(&pMsgFrame->Data[19]);
    pImu4->hv_chip_5v_feedback   = DatatoSHORT(&pMsgFrame->Data[21]);
    pImu4->hv_chip_minus5v_feedback = DatatoSHORT(&pMsgFrame->Data[23]);
    pImu4->positive_insulation_res  = DatatoSHORT(&pMsgFrame->Data[33]);
    pImu4->negative_insulation_res  = DatatoSHORT(&pMsgFrame->Data[37]);
    pImu4->ntc1_temperature         = DatatoSHORT(&pMsgFrame->Data[41]);
    pImu4->ntc2_temperature         = DatatoSHORT(&pMsgFrame->Data[45]);
    pImu4->dido_gpio_signal     = DatatoSHORT(&pMsgFrame->Data[49]);
    pImu4->board_5v_feedback    = DatatoSHORT(&pMsgFrame->Data[51]);
    pImu4->system_work_mode     = DatatoSHORT(&pMsgFrame->Data[53]);
    pImu4->alarm_info_1         = DatatoSHORT(&pMsgFrame->Data[55]);
    pImu4->alarm_info_2         = DatatoSHORT(&pMsgFrame->Data[57]);
    pImu4->alarm_info_3         = DatatoSHORT(&pMsgFrame->Data[59]);
    pImu4->fault_info_1         = DatatoSHORT(&pMsgFrame->Data[61]);
    pImu4->fault_info_2         = DatatoSHORT(&pMsgFrame->Data[63]);
    pImu4->fault_info_3         = DatatoSHORT(&pMsgFrame->Data[65]);
    TRACEI("[IMIU4] bat_volt=%d link_volt=%d hall5v=%d hall_sig=%.1f\n",
           (int)pImu4->battery_total_voltage, (int)pRough->stImiu4Info[iGrpId].link_voltage,
           (int)pImu4->hall_5v_sample, pRough->stImiu4Info[iGrpId].hall_signal_sample);
}

/*************************************************************************
 * Parse_Bsu02PackSampleInfo - 解析主索引74: BSU02U2板级及PACK采样信息
 * 字节布局: USHORT*20 + float*8 = 40 + 32 = 72字节 (实际USHORT=2, float=4)
 *  0:USHORT,1:USHORT,2:USHORT,3:USHORT,4:float,5:float,
 *  6:USHORT,7:USHORT,8:USHORT,9:USHORT,
 *  10:float,11:float,12:float,13:float,14:float,
 *  15:USHORT,16:USHORT,17:USHORT,18:USHORT,
 *  19-27:USHORT(预留)
 * Total: 20*2 + 8*4 = 40 + 32 = 72 bytes
 *************************************************************************/
static void Parse_Bsu02PackSampleInfo(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    if (!pMsgFrame || !pRough) return;
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }

    pRough->stBsu02Info.pt1000_selfcheck_temp_1   = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->stBsu02Info.pt1000_selfcheck_temp_2   = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->stBsu02Info.water_selfcheck_freq_1    = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->stBsu02Info.water_selfcheck_freq_2    = DatatoUSHORT(&pMsgFrame->Data[9]);
    pRough->stBsu02Info.pt1000_5v_pullup_feedback = DatatoFloat(&pMsgFrame->Data[11]);
    pRough->stBsu02Info.twelve_v_output_feedback  = DatatoFloat(&pMsgFrame->Data[15]);
    pRough->stBsu02Info.pt1000_temp_1             = DatatoUSHORT(&pMsgFrame->Data[19]);
    pRough->stBsu02Info.pt1000_temp_2             = DatatoUSHORT(&pMsgFrame->Data[21]);
    pRough->stBsu02Info.water_freq_1              = DatatoUSHORT(&pMsgFrame->Data[23]);
    pRough->stBsu02Info.water_freq_2              = DatatoUSHORT(&pMsgFrame->Data[25]);
    pRough->stBsu02Info.hv_interlock_pwm_amplitude = DatatoFloat(&pMsgFrame->Data[27]);
    pRough->stBsu02Info.hv_interlock_pwm_duty      = DatatoFloat(&pMsgFrame->Data[31]);
    pRough->stBsu02Info.fg_pwm_amplitude           = DatatoFloat(&pMsgFrame->Data[35]);
    pRough->stBsu02Info.fg_pwm_duty                = DatatoFloat(&pMsgFrame->Data[39]);
    pRough->stBsu02Info.aerosol_concentration      = DatatoFloat(&pMsgFrame->Data[43]);
    pRough->stBsu02Info.aerosol_sensor_type        = DatatoUSHORT(&pMsgFrame->Data[47]);
    pRough->stBsu02Info.aerosol_status_fault       = DatatoUSHORT(&pMsgFrame->Data[49]);
    pRough->stBsu02Info.afe_bot_enum_count         = DatatoUSHORT(&pMsgFrame->Data[51]);
    pRough->stBsu02Info.afe_top_enum_count         = DatatoUSHORT(&pMsgFrame->Data[53]);
    pRough->stBsu02Info.reserved_19                = DatatoUSHORT(&pMsgFrame->Data[55]);
    pRough->stBsu02Info.reserved_20                = DatatoUSHORT(&pMsgFrame->Data[57]);
    pRough->stBsu02Info.reserved_21                = DatatoUSHORT(&pMsgFrame->Data[59]);
    pRough->stBsu02Info.reserved_22                = DatatoUSHORT(&pMsgFrame->Data[61]);
    pRough->stBsu02Info.reserved_23                = DatatoUSHORT(&pMsgFrame->Data[63]);
    pRough->stBsu02Info.reserved_24                = DatatoUSHORT(&pMsgFrame->Data[65]);
    pRough->stBsu02Info.reserved_25                = DatatoUSHORT(&pMsgFrame->Data[67]);
    pRough->stBsu02Info.reserved_26                = DatatoUSHORT(&pMsgFrame->Data[69]);
    pRough->stBsu02Info.reserved_27                = DatatoUSHORT(&pMsgFrame->Data[71]);
    TRACEI("[BSU02] pt1k=%u,%u water=%u,%u pt5v=%.1f 12v=%.1f pt_t=%u,%u wf=%u,%u\n",
           pRough->stBsu02Info.pt1000_selfcheck_temp_1, pRough->stBsu02Info.pt1000_selfcheck_temp_2,
           pRough->stBsu02Info.water_selfcheck_freq_1, pRough->stBsu02Info.water_selfcheck_freq_2,
           pRough->stBsu02Info.pt1000_5v_pullup_feedback, pRough->stBsu02Info.twelve_v_output_feedback,
           pRough->stBsu02Info.pt1000_temp_1, pRough->stBsu02Info.pt1000_temp_2,
           pRough->stBsu02Info.water_freq_1, pRough->stBsu02Info.water_freq_2);
}

// 单位 1
static void Parse_Imeu3SampleAndFaultInfo(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    if (!pMsgFrame || !pRough) return;
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }

    pRough->stimeu3SampleInfo.ntc1_temp = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->stimeu3SampleInfo.ntc2_temp = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->stimeu3SampleInfo.ntc3_temp = DatatoUSHORT(&pMsgFrame->Data[7]);
    pRough->stimeu3SampleInfo.ntc4_temp = DatatoUSHORT(&pMsgFrame->Data[9]);
    pRough->stimeu3SampleInfo.ntc5_temp = DatatoUSHORT(&pMsgFrame->Data[11]);
    pRough->stimeu3SampleInfo.ntc6_temp = DatatoUSHORT(&pMsgFrame->Data[13]);
    pRough->stimeu3SampleInfo.ntc7_temp = DatatoUSHORT(&pMsgFrame->Data[15]);
    pRough->stimeu3SampleInfo.ntc_or_pt1000_8_temp = DatatoUSHORT(&pMsgFrame->Data[17]);
    pRough->stimeu3SampleInfo.ntc_or_pt1000_9_temp = DatatoUSHORT(&pMsgFrame->Data[19]);
    pRough->stimeu3SampleInfo.ntc_or_pt1000_10_temp = DatatoUSHORT(&pMsgFrame->Data[21]);
    pRough->stimeu3SampleInfo.ntc_or_pt1000_11_temp = DatatoUSHORT(&pMsgFrame->Data[23]);
    pRough->stimeu3SampleInfo.ntc_or_pt1000_12_temp = DatatoUSHORT(&pMsgFrame->Data[25]);
	pRough->stimeu3SampleInfo.temp_sor_temp = DatatoUSHORT(&pMsgFrame->Data[27]);
	pRough->stimeu3SampleInfo.humi_sor_humi = DatatoUSHORT(&pMsgFrame->Data[29]);
	pRough->stimeu3SampleInfo.temp_humi_sor_temp_485_1 = DatatoUSHORT(&pMsgFrame->Data[31]);
	pRough->stimeu3SampleInfo.temp_humi_sor_humi_485_1 = DatatoUSHORT(&pMsgFrame->Data[33]);
	pRough->stimeu3SampleInfo.temp_humi_sor_temp_485_2 = DatatoUSHORT(&pMsgFrame->Data[35]);
	pRough->stimeu3SampleInfo.temp_humi_sor_humi_485_2 = DatatoUSHORT(&pMsgFrame->Data[37]);
	pRough->stimeu3SampleInfo.temp_humi_sor_temp_485_3 = DatatoUSHORT(&pMsgFrame->Data[39]);
	pRough->stimeu3SampleInfo.temp_humi_sor_humi_485_3 = DatatoUSHORT(&pMsgFrame->Data[41]);
	pRough->stimeu3SampleInfo.temp_humi_sor_temp_485_4 = DatatoUSHORT(&pMsgFrame->Data[43]);
	pRough->stimeu3SampleInfo.temp_humi_sor_humi_485_4 = DatatoUSHORT(&pMsgFrame->Data[45]);
	pRough->stimeu3SampleInfo.dehumi_temp = DatatoUSHORT(&pMsgFrame->Data[47]);
	pRough->stimeu3SampleInfo.dehumi_humi = DatatoUSHORT(&pMsgFrame->Data[49]);
	pRough->stimeu3SampleInfo.dehumi_start = DatatoUSHORT(&pMsgFrame->Data[51]);
	pRough->stimeu3SampleInfo.dehumi_stop = DatatoUSHORT(&pMsgFrame->Data[53]);
	pRough->stimeu3SampleInfo.dehumi_workstat = DatatoUSHORT(&pMsgFrame->Data[55]);
	pRough->stimeu3SampleInfo.dehumi_addr = DatatoUSHORT(&pMsgFrame->Data[57]);
	pRough->stimeu3SampleInfo.di1_16 = DatatoUSHORT(&pMsgFrame->Data[59]);
	pRough->stimeu3SampleInfo.di17_22 = DatatoUSHORT(&pMsgFrame->Data[61]);
	pRough->stimeu3SampleInfo.do1_16 = DatatoUSHORT(&pMsgFrame->Data[63]);
	pRough->stimeu3SampleInfo.do17_24 = DatatoUSHORT(&pMsgFrame->Data[65]);
	pRough->stimeu3SampleInfo.x1_connect_stat = DatatoUSHORT(&pMsgFrame->Data[67]);
	pRough->stimeu3SampleInfo.combgas_h2 = DatatoUSHORT(&pMsgFrame->Data[69]);
	pRough->stimeu3SampleInfo.combgas_co2 = DatatoUSHORT(&pMsgFrame->Data[71]);
	pRough->stimeu3SampleInfo.resv1 = DatatoUSHORT(&pMsgFrame->Data[73]);
	pRough->stimeu3SampleInfo.resv2 = DatatoUSHORT(&pMsgFrame->Data[75]);
	pRough->stimeu3SampleInfo.flood_freq = DatatoUSHORT(&pMsgFrame->Data[77]);
	pRough->stimeu3SampleInfo.pre_sor_volt_1 = DatatoUSHORT(&pMsgFrame->Data[79]);
	pRough->stimeu3SampleInfo.pre_sor_volt_2 = DatatoUSHORT(&pMsgFrame->Data[81]);
	pRough->stimeu3SampleInfo.pre_sor_volt_3 = DatatoUSHORT(&pMsgFrame->Data[83]);
	pRough->stimeu3SampleInfo.pre_sor_volt_4 = DatatoUSHORT(&pMsgFrame->Data[85]);
	pRough->stimeu3SampleInfo.pre_sor_volt_5 = DatatoUSHORT(&pMsgFrame->Data[87]);
	pRough->stimeu3SampleInfo.pre_sor_volt_6 = DatatoUSHORT(&pMsgFrame->Data[89]);
	pRough->stimeu3SampleInfo.pwm_freq_1 = DatatoUSHORT(&pMsgFrame->Data[91]);
	pRough->stimeu3SampleInfo.pwm_freq_2 = DatatoUSHORT(&pMsgFrame->Data[93]);
	pRough->stimeu3SampleInfo.pwm_freq_3 = DatatoUSHORT(&pMsgFrame->Data[95]);
	pRough->stimeu3SampleInfo.pwm_freq_4 = DatatoUSHORT(&pMsgFrame->Data[97]);
	pRough->stimeu3SampleInfo.pwm_freq_5 = DatatoUSHORT(&pMsgFrame->Data[99]);
	pRough->stimeu3SampleInfo.pwm_freq_6 = DatatoUSHORT(&pMsgFrame->Data[101]);
	pRough->stimeu3SampleInfo.alarm_info1 = DatatoUSHORT(&pMsgFrame->Data[103]);
	pRough->stimeu3SampleInfo.alarm_info2 = DatatoUSHORT(&pMsgFrame->Data[105]);
	pRough->stimeu3SampleInfo.alarm_info3 = DatatoUSHORT(&pMsgFrame->Data[107]);

    TRACEI("[imeu3] ntc1_temp=%u, ntc2_temp=%u, ntc3_temp=%u, ntc4_temp=%u, ntc5_temp=%u, ntc6_temp=%u, ntc7_temp=%u, ntc_or_pt1000_8_temp=%u, ntc_or_pt1000_9_temp=%u, ntc_or_pt1000_10_temp=%u, ntc_or_pt1000_11_temp=%u, ntc_or_pt1000_12_temp=%u, 
            temp_sor_temp=%u, humi_sor_humi=%u, 
            temp_humi_sor_temp_485_1=%u, temp_humi_sor_humi_485_1=%u, temp_humi_sor_temp_485_2=%u, temp_humi_sor_humi_485_2=%u, temp_humi_sor_temp_485_3=%u, temp_humi_sor_humi_485_3=%u, temp_humi_sor_temp_485_4=%u, temp_humi_sor_humi_485_4=%u, 
            dehumi_temp=%u, dehumi_humi=%u, dehumi_start=%u, dehumi_stop=%u, dehumi_workstat=%u, dehumi_addr=%u, di1_16=%u, di17_22=%u, do1_16=%u, do17_24=%u, 
            x1_connect_stat=%u, combgas_h2=%u, combgas_co2=%u, resv1=%u, resv2=%u, flood_freq=%u, pre_sor_volt_1=%u, pre_sor_volt_2=%u, pre_sor_volt_3=%u, pre_sor_volt_4=%u, pre_sor_volt_5=%u, pre_sor_volt_6=%u, pwm_freq_1=%u, pwm_freq_2=%u, pwm_freq_3=%u, pwm_freq_4=%u, pwm_freq_5=%u, pwm_freq_6=%u, alarm_info1=%u, alarm_info2=%u, alarm_info3=%u, \n",
        pRough->stimeu3SampleInfo.ntc1_temp, pRough->stimeu3SampleInfo.ntc2_temp, pRough->stimeu3SampleInfo.ntc3_temp,
        pRough->stimeu3SampleInfo.ntc4_temp, pRough->stimeu3SampleInfo.ntc5_temp, pRough->stimeu3SampleInfo.ntc6_temp,
        pRough->stimeu3SampleInfo.ntc7_temp, pRough->stimeu3SampleInfo.ntc_or_pt1000_8_temp, pRough->stimeu3SampleInfo.ntc_or_pt1000_9_temp,
        pRough->stimeu3SampleInfo.ntc_or_pt1000_10_temp, pRough->stimeu3SampleInfo.ntc_or_pt1000_11_temp, pRough->stimeu3SampleInfo.ntc_or_pt1000_12_temp,
        pRough->stimeu3SampleInfo.temp_sor_temp, pRough->stimeu3SampleInfo.humi_sor_humi,pRough->stimeu3SampleInfo.temp_humi_sor_temp_485_1,
        pRough->stimeu3SampleInfo.temp_humi_sor_humi_485_1, pRough->stimeu3SampleInfo.temp_humi_sor_temp_485_2,pRough->stimeu3SampleInfo.temp_humi_sor_humi_485_2,
        pRough->stimeu3SampleInfo.temp_humi_sor_temp_485_3, pRough->stimeu3SampleInfo.temp_humi_sor_humi_485_3, pRough->stimeu3SampleInfo.temp_humi_sor_temp_485_4,
        pRough->stimeu3SampleInfo.temp_humi_sor_humi_485_4, pRough->stimeu3SampleInfo.dehumi_temp, pRough->stimeu3SampleInfo.dehumi_humi,
        pRough->stimeu3SampleInfo.dehumi_start, pRough->stimeu3SampleInfo.dehumi_stop, pRough->stimeu3SampleInfo.dehumi_workstat,
        pRough->stimeu3SampleInfo.dehumi_addr, pRough->stimeu3SampleInfo.di1_16, pRough->stimeu3SampleInfo.di17_22,
        pRough->stimeu3SampleInfo.do1_16, pRough->stimeu3SampleInfo.do17_24, pRough->stimeu3SampleInfo.x1_connect_stat,
        pRough->stimeu3SampleInfo.combgas_h2, pRough->stimeu3SampleInfo.combgas_co2, pRough->stimeu3SampleInfo.resv1,
        pRough->stimeu3SampleInfo.resv2, pRough->stimeu3SampleInfo.flood_freq,
        pRough->stimeu3SampleInfo.pre_sor_volt_1, pRough->stimeu3SampleInfo.pre_sor_volt_2, pRough->stimeu3SampleInfo.pre_sor_volt_3,
        pRough->stimeu3SampleInfo.pre_sor_volt_4, pRough->stimeu3SampleInfo.pre_sor_volt_5, pRough->stimeu3SampleInfo.pre_sor_volt_6,
        pRough->stimeu3SampleInfo.pwm_freq_1, pRough->stimeu3SampleInfo.pwm_freq_2, pRough->stimeu3SampleInfo.pwm_freq_3,
        pRough->stimeu3SampleInfo.pwm_freq_4, pRough->stimeu3SampleInfo.pwm_freq_5, pRough->stimeu3SampleInfo.pwm_freq_6,
        pRough->stimeu3SampleInfo.alarm_info1, pRough->stimeu3SampleInfo.alarm_info2, pRough->stimeu3SampleInfo.alarm_info3);
}

/* 主索引86: 控制反馈 → pRough->DiagStat.ctrl_feedback (全局) */
static void Parse_ResQueryFeedback(STBATT_SAMPLER_ROUGH* pRough, AR_FRAME* pMsgFrame)
{
    {
        int db = pMsgFrame->ID_Datalen - 8;
        TRACEI("[帧头] MainIdx=%d FunCode=%d DataLen=%d DstAddr=%d SrcAddr=%d Data(%d):",
               pMsgFrame->ID_MainIndex, pMsgFrame->ID_FunCode, pMsgFrame->ID_Datalen,
               pMsgFrame->ID_DstAddr, pMsgFrame->ID_SrcAddr, db);
        for (int i = 0; i < db && i < 256; i++) TRACEI(" %02X", pMsgFrame->Data[i]);
        TRACEI("\n");
    }
    pRough->StatFeedback.res_feedback.current_res_freq = DatatoUSHORT(&pMsgFrame->Data[3]);
    pRough->StatFeedback.res_feedback.query_stat = DatatoUSHORT(&pMsgFrame->Data[5]);
    pRough->StatFeedback.res_feedback.reserved = DatatoUSHORT(&pMsgFrame->Data[7]);
    TRACEI("[不同频点内阻查询反馈] 当前内阻频点/频率信息=%u, 查询是否完成=%u, 预留=%u\n",
           pRough->StatFeedback.res_feedback.current_res_freq,
           pRough->StatFeedback.res_feedback.query_stat,
           pRough->StatFeedback.res_feedback.reserved);
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/25 13:57:59
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void YFY_Batt_Rpmsg_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	//Ctrl DO5, Break Batt Contactor
	int n = 0;
	// for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	// {
	// 	pSTBatt_Rough->nCtrlTimes[n]	 = 0;
    // 	pSTBatt_Rough->nPowerONTimes[n]	 = 0;
	// 	CtrlBattCabinetContactor(pSTBatt_Rough, n, FALSE);
	// }
    pSTBatt_Rough->bRpmsgPortReady = FALSE;
 	
	//---------Init cmd-------------
	TAG_YFY_BATT_PRO_RPMSG *p = &s_infy_Batt_Rpmsg_Pro[0];
	INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_CHARG_BLANCE_STATUS_GRP1,         0x0B, 		0x14,			Parse_S2C_Cell_ChargeBalanceStatus);
	INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_CHARG_BLANCE_STATUS_GRP2,         0x0B, 		0x15,			Parse_S2C_Cell_ChargeBalanceStatus);
    INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_DISCHARG_BLANCE_STATUS_GRP1,      0x0B, 		0x16,			Parse_S2C_Cell_DisChargeBalanceStatus);
    INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_DISCHARG_BLANCE_STATUS_GRP2,      0x0B, 		0x17,			Parse_S2C_Cell_DisChargeBalanceStatus);
    INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_VOLTAGE_GRP1,                     0x0B, 		0x18,			Parse_S2C_CellVoltage);
    INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_VOLTAGE_GRP2,                     0x0B, 		0x19,			Parse_S2C_CellVoltage);
    INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_TEMPERATURE_GRP1,                 0x0B, 		0x1A,           Parse_S2C_CellTemperature);
    INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_TEMPERATURE_GRP2,                 0x0B, 		0x1B,           Parse_S2C_CellTemperature);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_RESISTANCE_REAL_GRP1_1,           0x0B, 		0x1C,           Parse_S2C_CellResReal);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_RESISTANCE_REAL_GRP1_2,           0x0B, 		0x1D,           Parse_S2C_CellResReal);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_RESISTANCE_REAL_GRP2_1,           0x0B, 		0x1E,           Parse_S2C_CellResReal);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_RESISTANCE_REAL_GRP2_2,           0x0B, 		0x1F,           Parse_S2C_CellResReal);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_RESISTANCE_IMAGE_GRP1_1,          0x0B, 		0x20,           Parse_S2C_CellResImag);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_RESISTANCE_IMAGE_GRP1_2,          0x0B, 		0x21,           Parse_S2C_CellResImag);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_RESISTANCE_IMAGE_GRP2_1,          0x0B, 		0x22,           Parse_S2C_CellResImag);// todo change mainId
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_RESISTANCE_IMAGE_GRP2_2,          0x0B, 		0x23,           Parse_S2C_CellResImag);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_IC_STATUS_GRP1,                   0x0B, 		0x24,           Parse_S2C_CellIcStat);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_RPMSG_R_CMD_S2C_CELL_IC_STATUS_GRP2,                   0x0B, 		0x25,           Parse_S2C_CellIcStat);

    // 02
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_HEARTBEAT,           0x00, 		0x01,           Parse_HeartbeatInfo);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_BOARD_SAMPLE,        0x00, 		0x0E,           Parse_BoardSampleInfo);
    // 03
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_STATUS,              0x00, 		0x32,           Parse_StatusInfo);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_GLOBAL,              0x00, 		0x33,           Parse_GlobalInfo);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_POWER,               0x00, 		0x34,           Parse_PowerInfo);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_LIMIT,               0x00, 		0x35,           Parse_LimitInfo);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_FEATURE,             0x00, 		0x36,           Parse_FeatureInfo);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_ENERGY_CURR,         0x00, 		0x37,           Parse_EnergyCurrStat);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_ENERGY_TOTAL,        0x00, 		0x38,           Parse_EnergyTotalStat);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_ALARM_FAULT_0,       0x00, 		0x39,           Parse_AlarmFault);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_ALARM_FAULT_1,       0x00, 		0x3A,           Parse_AlarmFault);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_TIME,                0x00, 		0x3B,           Parse_TimeStat);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_FAULT,               0x00, 		0x3C,           Parse_FaultStat);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_SOS,                 0x00, 		0x3D,           Parse_SosStat);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_DOD,                 0x00, 		0x3E,           Parse_DodStat);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_DOD_REC_GRP1,       0x00, 		0x0F,           Parse_DODRecord);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_DOD_REC_GRP2,       0x00, 		0x10,           Parse_DODRecord);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_SOS_REC_GRP1,       0x00, 		0x11,           Parse_SOSRecord);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_SOS_REC_GRP2,       0x00, 		0x12,           Parse_SOSRecord);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_AUTH,                0x00, 		0x3F,           Parse_AuthState);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_IMIU4_GRP1,          0x00, 		0x43,           Parse_Imiu4Info);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_IMIU4_GRP2,          0x00, 		0x44,           Parse_Imiu4Info);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_CTRL_FEEDBACK,       0x00, 		0x46,           Parse_CtrlFeedback);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_BSU02,               0x00, 		0x4A,           Parse_Bsu02PackSampleInfo);
    INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_IMEU3_SAMP_FAULT,    0x00, 		0x4D,           Parse_Imeu3SampleAndFaultInfo);
    // INIT_YFY_RPMSG_BATT_CMD(p,		_WHOLEINFO_RES_QUERY_FEEDBACK,  0x00, 		0x56,           Parse_ResQueryFeedback);
	
    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
    {
	    YFY_Init_RoughData_RPMSG(pSTBatt_Rough, n);
	}
	return;
}

#define RPMSG_SOI       0xABCD
#define RPMSG_EOI		0xEF
// #define RPMSG_MIN_LEN	5

#define ADDR_R_KERNEL          50
#define ADDR_A_KERNEL_MIN     90
#define ADDR_A_KERNEL_MAX     99
#define FRAME_FLAG      0x0c


/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/25 15:15:52
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
TAG_YFY_BATT_PRO_RPMSG* Find_RPMsg_Batt_Frame(IN BYTE* pRaw, IN int nReadLen, OUT AR_FRAME* pMsgFrame, CHANNEL_TYPE ChannelTyep)
{
    int n, m;
    BYTE* byPos = pRaw;
    UINT nParsedDataLen = 0;

    TAG_YFY_BATT_PRO_RPMSG* pFoundMsg = NULL;
    //Notice: only find the first frame, if have multi frames, ignore the followed others
    
    for(n = 0; n < nReadLen; n++)
    {
        // 1. Found SOI
        if((byPos[n] == (BYTE)(RPMSG_SOI >> 8)) && 
            (byPos[n+1] == (BYTE)(RPMSG_SOI & 0xFF)))
        {
            // 2. Get Data Length
            nParsedDataLen = (UINT)(byPos[n+2] | (byPos[n+3] << 8) | (byPos[n+4] << 16) | (byPos[n+5] << 24));

            // 3. data
            if (!AR_ProcessFrame(&byPos[n+2], pMsgFrame))
            {
                TRACED("Process Frame[%02X] failed! \n", byPos[n+2]);
                continue;
            }

            // Check
            if((pMsgFrame->ID_SrcAddr != ADDR_R_KERNEL) || 
                (pMsgFrame->ID_DstAddr < ADDR_A_KERNEL_MIN) ||
                (pMsgFrame->ID_DstAddr > ADDR_A_KERNEL_MAX))
            {
                continue;
            }

            if((ChannelTyep == CELL_CHANNEL) && (pMsgFrame->ID_FunCode != FUNCODE_SIG_UPLOAD))
            {
                continue;
            }

            for(m = 0; m < _MAX_YFY_RPMSG_CMD; m++)
            {
                if(s_infy_Batt_Rpmsg_Pro[m].nMainIndex == pMsgFrame->ID_MainIndex)
                {
                    TRACED("Found pFoundMsg！RPMSG_CMD:[%d]\n", m);
                    pFoundMsg = &s_infy_Batt_Rpmsg_Pro[m];
                    break;
                }
            }

            if (pFoundMsg == NULL)
            {
                continue;
            }

            // check data length
            if((nParsedDataLen > AR_MAX_DATA_LEN) || (pMsgFrame->ID_Datalen != nParsedDataLen))
            {
                TRACED("Found Msg[%02X] but the datalen[%d] is INVALID\n", byPos[n + 2], nParsedDataLen);
				//return NULL;
				pFoundMsg = NULL;
				continue;
            } 

            // 4. EOI
            if (byPos[n + 2 + nParsedDataLen] != RPMSG_EOI)
            {
                TRACED("Found Msg[%02X] but the EOI[%02X] is INVALID \n", byPos[n + 2],
                    byPos[n + 2 + nParsedDataLen]);
                // return NULL;
                pFoundMsg = NULL;
                continue;
            }
            break;
        }
    }
    return pFoundMsg;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/25 14:14:42
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void YFY_ReadAndParseRpmsgData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
    AR_FRAME strMsgFrame;
    BYTE byBuff[TCP_BUFF_LEN];   // SOI（2） + EOI（1） + DataLen（2） + DataLen MAX（1024）//TODO:静态数组，清空或者用返回字节读取
    memset(byBuff, 0, sizeof(byBuff));

    int nReadBytes = pSTBatt_Rough->pRpmsgComm.Read(pSTBatt_Rough->hRpmsgDev, (char*)byBuff, sizeof(byBuff));

    if(nReadBytes > 0)
    {
        int num = 0;
        int line = 0;

        LOG_TO_FILE("Read Rpmsg Cell info Count:%d\n", nReadBytes);

        LOG_TO_FILE("SOI:[%02X][%02X]\n", byBuff[0], byBuff[1]);

        for(int i = 2; i < nReadBytes - 1; i++)
        {
            if(num == 0)
            {
                LOG_TO_FILE("[%03d]: ", line++);
            }

            LOG_TO_FILE("[%02X]", byBuff[i]);

            num++;

            if(num == 8)
            {
                num = 0;
                LOG_TO_FILE("\n");
            }
        }

        if(num != 0)
        {
            LOG_TO_FILE("\n");
        }

        LOG_TO_FILE("EOI:[%02X]\n", byBuff[nReadBytes - 1]);
    }
    else
    {
        LOG_TO_FILE("Read Rpmsg Cell info Failed! Count:%d\n", nReadBytes);
        return;
    }
#if 0
    BYTE rxbuf[23] = {0xAB, 0XCD, 0x12, 0x00, // SOI + DataLen
                       0x12,0x00,0x00,0x00,   //数据长度
					   0x0B, 				//功能码	-单体上传 11
					   0x00,				//主索引	-0
					   0x5A,				//目标设备地址	-A核
					   0x32,				//源设备地址	-R核
					   0x01,0x00,			//DATA_1
					   0x01,0x00,			//DATA_2
					   0x05,0x00,			//DATA_3	
                       0x09,0x00,			//DATA_4
                       0x05,0x00,			//DATA_5	
                       0xEF
					   };

    // 虚拟数据测试
    int nReadBytes = sizeof(rxbuf);

    memcpy(byBuff, rxbuf, sizeof(rxbuf));
#endif

    BYTE* pFoundPos = byBuff;
    do
    {
        // 根据接收到的原始数据帧找到解析函数
        TAG_YFY_BATT_PRO_RPMSG* pFrm = Find_RPMsg_Batt_Frame(pFoundPos, nReadBytes, &strMsgFrame, CELL_CHANNEL);
        if (pFrm == NULL)
        {
            TRACED("---Get Frm Is NULL---\n");
            break;
        }
        pFrm->pfnCmdHandler(pSTBatt_Rough, &strMsgFrame);

        pFoundPos += (3+strMsgFrame.ID_Datalen);
        nReadBytes -= (3+strMsgFrame.ID_Datalen);

    } while(nReadBytes > 0);
    // } while(0);
#if 0
    TRACED("---------------------------strACDCGrpInfo -----------------------------\n");
    BYTE* pData = (BYTE*)(&s_infy_BmtBattCellData[20].byCellBalanceStat[0]);

    for (int n = 0; n < 416; n++)
    {
        TRACED("pData[%d] = [%02x]\n", n, *pData);
        pData++;
    }
#endif
    return;
}

void ReadAndParseBmsStatus(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
    AR_FRAME strMsgFrame;
    BYTE byBuff[TCP_BUFF_LEN];   // SOI（2） + EOI（1） + DataLen（2） + DataLen MAX（1024）//TODO:静态数组，清空或者用返回字节读取
    memset(byBuff, 0, sizeof(byBuff));

    int nReadBytes = pSTBatt_Rough->Rpmsg_cycle_info.Read(pSTBatt_Rough->RpmsgHanle_cycle, (char*)byBuff, sizeof(byBuff));

    if(nReadBytes <= 0)
    {
        return;
    }
    // if(nReadBytes != 0)
    // {
    //     TRACEI("Rpmsg_cycle_info 获取到读写返回的值\n");
    //     int i;
    //     for(i = 0; i < nReadBytes; ++i)
    //     {
    //         TRACEI("%02X ", byBuff[i]);
    //     }
    //     TRACEI("\n");
    //     TRACEI("结束\n");
    // }

    BYTE* pFoundPos = byBuff;
    do
    {
        // 根据接收到的原始数据帧找到解析函数
        TAG_YFY_BATT_PRO_RPMSG* pFrm = Find_RPMsg_Batt_Frame(pFoundPos, nReadBytes, &strMsgFrame, BMS_STATUS_CHANNEL);
        if (pFrm == NULL)
        {
            TRACED("---Get Frm Is NULL---\n");
            break;
        }
        pFrm->pfnCmdHandler(pSTBatt_Rough, &strMsgFrame);

        TRACEI("RpmsgHanle_cycle 数据长度 = [%d], 功能码 = [%d], 信息类型 = [%d]\n", strMsgFrame.ID_Datalen, strMsgFrame.ID_FunCode, strMsgFrame.ID_MainIndex);

        pFoundPos += (3+strMsgFrame.ID_Datalen);
        nReadBytes -= (3+strMsgFrame.ID_Datalen);

    } while(nReadBytes > 0);
    // } while(0);

    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/25 14:11:05
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void YFY_Batt_Rpmsg_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
    // if (pSTBatt_Rough->emBattMFG == BATT_MFG_INFY_SELF_RPMSG)
	{
        //---  YFY_Batt_Rpmsg_Sample Running  ---//  
		// if(YFY_JudgeIfNeedReconfig(pSTBatt_Rough))
		{
			YFY_ReadAndParseRpmsgData(pSTBatt_Rough);
			//TRACED("Here Waitting Batt Reconfig\n"); 
		}
    }
}

/*********************************************************************************
*  
*  FUNCTION NAME :  map cell info
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/04/27 10:16:47
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void MapRawData_2_InfyBMSBlob(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
    int n;

    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
    {
        if(n >= pSTBatt_Rough->nSystemCfgBattNum) break;
        A_BLOB_IFY_BMS_DATAINFO* pBlobIfyBMS = &pSTBatt_Rough->blobIfyBMS[n];

        memcpy(pBlobIfyBMS->byCellBalanceStat_836,           s_infy_BmtBattCellData[n].byCellBalanceStat, 	    BATT_CELL_MAX_NUM * sizeof(BYTE));
        memcpy(pBlobIfyBMS->byCellDischargeStat_836,         s_infy_BmtBattCellData[n].byCellDischargeStat,     BATT_CELL_MAX_NUM * sizeof(BYTE));
        memcpy(pBlobIfyBMS->usCellVolt_836,                  s_infy_BmtBattCellData[n].usCellVolt,              BATT_CELL_MAX_NUM * sizeof(USHORT));
        memcpy(pBlobIfyBMS->sCellTemp_836,                   s_infy_BmtBattCellData[n].sCellTemp,               BATT_CELL_MAX_NUM * sizeof(SHORT));
        memcpy(pBlobIfyBMS->uiCellResReal_836,               s_infy_BmtBattCellData[n].uiCellResReal,           BATT_CELL_MAX_NUM * sizeof(UINT));
        memcpy(pBlobIfyBMS->uiCellResImag_836,               s_infy_BmtBattCellData[n].uiCellResImag,           BATT_CELL_MAX_NUM * sizeof(UINT));
        memcpy(pBlobIfyBMS->byCellIcStat,                    s_infy_BmtBattCellData[n].byCellIcStat,            BATT_CELL_MAX_NUM * sizeof(BYTE));
    }

    return;
}

/**************************************************************************************************************************
                                     BMS读取数据和写数据
*************************************************************************************************************************/

static const BYTE Paramtype_SysCom[SYS_COM] = {
    [1 ... 4] = VAR_TYPE_ENUM
};

static const BYTE Paramtype_SysAdvan[SYS_ADCAN] = {
    [1] = VAR_TYPE_ENUM
};

static const BYTE Paramtype_MainCntlAdvan[MAIN_CNTL_ADVAN] = {
    [23 ... 29] = VAR_TYPE_ENUM,  
    [39 ... 40] = VAR_TYPE_ENUM,
    [53] = VAR_TYPE_ENUM,
    [57] = VAR_TYPE_ENUM
};

static const BYTE Paramtype_MainCntlHard[MAIN_CNTL_HARD] = {
    [1 ... 2] = VAR_TYPE_ENUM,  
    [7] = VAR_TYPE_ENUM,  
    [13] = VAR_TYPE_ENUM
};

static const BYTE Paramtype_GrpCtn[3] = {
    [0 ... 2] = VAR_TYPE_ENUM,
};

static const BYTE Paramtype_User[1] = {
    [0] = VAR_TYPE_ENUM,
};

static const BYTE Paramtype_SysDbug[8] = {
    [0] = VAR_TYPE_ENUM,
};

static const BYTE Paramtype_CalibImiu4[2] = {
    [0] = VAR_TYPE_ENUM,
};

static const BYTE Paramtype_Imeu3HW[12] = {
    [0 ... 7] = VAR_TYPE_ENUM,
    [9 ... 11] = VAR_TYPE_ENUM,
};

/**************************************************************************
                                 功能：参数缩放比例定义
 *************************************************************************/
static const float fScales[] = { 1.0f, 0.1f, 10.0f};

static const BYTE ParamScales_MainCntlGen[MAIN_CNTL_GEN] = {
    [2 ... 10] = SCALE_01,  
    [24 ... 39] = SCALE_01,
    [50 ... 53] = SCALE_01,
    [68 ... 75] = SCALE_01,
    [80 ... 81] = SCALE_01
};

static const BYTE ParamScales_MainCntlAdvan[MAIN_CNTL_ADVAN] = {
    [41 ... 42] = SCALE_01,  
    [45 ... 52] = SCALE_01,
    [65 ... 66] = SCALE_01,
    [69 ... 70] = SCALE_01
};

static const BYTE ParamScales_MainCntlHard[MAIN_CNTL_HARD] = {
    [3 ... 4] = SCALE_01,  
    [9] = SCALE_10,
    [10 ... 11] = SCALE_01,
    [12] = SCALE_10,
    [15 ... 16] = SCALE_10,
    [17 ... 20] = SCALE_01
};

/**************************************************************************
                              配置对象表
 *************************************************************************/
static const BmsGroupConfig_t BmsGroupConfigs[] = {
    [0]  = { .typeMap = Paramtype_SysCom,       .scaleMap = NULL,                      .baseVarID = 620 , .endId = 628, .BmsPraType = SYS_COMMON_GET},
    [1]  = { .typeMap = Paramtype_SysAdvan,     .scaleMap = NULL,                      .baseVarID = 629 , .endId = 643, .BmsPraType = SYS_ADVANCED_GET},
    [2]  = { .typeMap = NULL,                   .scaleMap = ParamScales_MainCntlGen,   .baseVarID = 644 , .endId = 729, .BmsPraType = MAIN_CTRL_GENERAL_GET},
    [3]  = { .typeMap = Paramtype_MainCntlAdvan,.scaleMap = ParamScales_MainCntlAdvan, .baseVarID = 730 , .endId = 801, .BmsPraType = MAIN_CTRL_ADVANCED_GET},
    [4]  = { .typeMap = Paramtype_MainCntlHard, .scaleMap = ParamScales_MainCntlHard,  .baseVarID = 802 , .endId = 854, .DataBit = 1, .BmsPraType = MAIN_CTRL_HARDWARE_GET},

    [5]  = { .typeMap = NULL,                   .scaleMap = NULL,                       .baseVarID = 120 , .endId = 121, .BmsPraType = WHOLE_AUTH_SWITCH_GET},
    [6]  = { .typeMap = Paramtype_GrpCtn,       .scaleMap = NULL,                       .baseVarID = 122 , .endId = 124, .BmsPraType = WHOLE_GRP1_CTRL_GET},
    [7]  = { .typeMap = Paramtype_GrpCtn,       .scaleMap = NULL,                       .baseVarID = 125 , .endId = 127, .BmsPraType = WHOLE_GRP2_CTRL_GET},
    [8]  = { .typeMap = Paramtype_User,         .scaleMap = NULL,                       .baseVarID = 128 , .endId = 129, .BmsPraType = WHOLE_USER_CTRL_GET},
    [9]  = { .typeMap = Paramtype_SysDbug,      .scaleMap = NULL,                       .baseVarID = 130 , .endId = 139, .BmsPraType = WHOLE_DEBUG_CMD_GET},
    [10] = { .typeMap = Paramtype_CalibImiu4,   .scaleMap = NULL,                       .baseVarID = 140 , .endId = 143, .BmsPraType = WHOLE_CALIB_CMD_GET},
    [11] = { .typeMap = Paramtype_Imeu3HW,      .scaleMap = NULL,                       .baseVarID = 171 , .endId = 182, .DataBit = 1, .BmsPraType = WHOLE_IMEU3_HW_PARAM_SETBACK},// todo

    [12] = { .typeMap = NULL,                   .scaleMap = NULL,                       .baseVarID = 144 , .endId = 147, .BmsPraType = WHOLE_TIME_SYNC_GET},
    [13] = { .typeMap = NULL,                   .scaleMap = NULL,                       .baseVarID = 148 , .endId = 170, .DataBit = 1, .BmsPraType = WHOLE_DEVICE_SN_GET},
};

BOOL GetDataInfo(int nParamId, DATA_INFO* data_info)
{
    for (int i = 0; i < BMS_GROUP_CONFIG_CNT; ++i) {
        if (nParamId >= BmsGroupConfigs[i].baseVarID && nParamId <= BmsGroupConfigs[i].endId) {
            int id = (BYTE)(nParamId - BmsGroupConfigs[i].baseVarID);

            data_info->dataType = BmsGroupConfigs[i].typeMap ? BmsGroupConfigs[i].typeMap[id] : VAR_TYPE_FLOAT;
            data_info->datalen = BmsGroupConfigs[i].DataBit;
            
            return TRUE;
        }
    }
    return FALSE;
}

int GetBmsConfigIdx(BmsGetType_e PType)
{
    int i = 0;
    for(i = 0; i < 13; i++)
    {
        if(BmsGroupConfigs[i].BmsPraType == PType)
        {
            return i;
        }
    }
    return -1;
}

static void Push_Sys_Common_Get(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, AR_FRAME* strMsgFrame, int IdxNumners, int startSubIdx)
{
    int i = 0;

    VAR_VALUE varV;
    int dataByteIdx = 3;

    // BmsGetType_e PType = strMsgFrame->ID_MainIndex;
    if(strMsgFrame->ID_MainIndex >= 100 && strMsgFrame->ID_MainIndex < 105)
    {
        strMsgFrame->ID_MainIndex += 5;
    }
    else if(strMsgFrame->ID_MainIndex == 75)
    {
        strMsgFrame->ID_MainIndex += 1;
    }
    int Idx = GetBmsConfigIdx(strMsgFrame->ID_MainIndex);

    if(Idx == -1)
    {
        // TRACEI("not find mainidx info\n");
        return;
    }

    // printf("获取到读写返回的值\n");
        
    // for(i = 0; i < strMsgFrame->ID_Datalen - 8; ++i)
    // {
    //     printf("%02X ", strMsgFrame->Data[i]);
    // }
    // printf("\n");
    // printf("结束\n");
    
    const BmsGroupConfig_t* pCfg = &BmsGroupConfigs[Idx];

    for(i = 0; i < IdxNumners; ++i)
    {
        int CurrentSubIdx = startSubIdx + i;

        USHORT rawData;
        if(pCfg->DataBit == 1)
        {
            rawData = (USHORT)strMsgFrame->Data[dataByteIdx];
            dataByteIdx += 1;
        }
        else
        {
            rawData = DatatoUSHORT(&strMsgFrame->Data[dataByteIdx]);
            dataByteIdx += 2;
        }

        // printf("rawData = %d", rawData);
       
        BYTE varType = (pCfg->typeMap != NULL) ? pCfg->typeMap[CurrentSubIdx] : 0;

        if(varType == VAR_TYPE_ENUM)
        {
            varV.emValue = (_ENUM)rawData;
            
        }
        else
        {
            BYTE scaleType = (pCfg->scaleMap != NULL) ? pCfg->scaleMap[CurrentSubIdx] : 0;

            if (scaleType >= sizeof(fScales)/sizeof(fScales[0])) scaleType = 0;
            
            varV.fValue = (float)rawData * fScales[scaleType];
        }

        // printf("BMS dev id = [%d], sub id = [%d]  curval = [%f]\n", 500, pCfg->baseVarID + CurrentSubIdx, varV.fValue);

        Push_VarValue(SETTYPE_READ_VALUE, pSTBatt_Rough->hSampler, 500, pCfg->baseVarID + CurrentSubIdx, varV);
    }
}

void Find_RPMsg_Batt_Frame_FAQ(IN BYTE* pRaw, IN int nReadLen, OUT AR_FRAME* pMsgFrame)
{
    int n, m;
    BYTE* byPos = pRaw;
    UINT nParsedDataLen = 0;

    for(n = 0; n < nReadLen; n++)
    {
        // 1. Found SOI
        if((byPos[n] == (BYTE)(RPMSG_SOI >> 8)) && 
            (byPos[n+1] == (BYTE)(RPMSG_SOI & 0xFF)))
        {
            // 2. Get Data Length
            nParsedDataLen = (UINT)(byPos[n+2] | (byPos[n+3] << 8) | (byPos[n+4] << 16) | (byPos[n+5] << 24));

            // 3. data
            if (!AR_ProcessFrame(&byPos[n+2], pMsgFrame))
            {
                TRACE("Process Frame[%02X] failed! \n", byPos[n+2]);
                continue;
            }

            // // Check
            // if((pMsgFrame->ID_SrcAddr != ADDR_R_KERNEL) || 
            //     (pMsgFrame->ID_DstAddr < ADDR_A_KERNEL_MIN) ||
            //     (pMsgFrame->ID_DstAddr > ADDR_A_KERNEL_MAX)  ||)
            // {
            //     continue;
            // }

            // check data length
            if((nParsedDataLen > AR_MAX_DATA_LEN) || (pMsgFrame->ID_Datalen != nParsedDataLen))
            {
                TRACE("Found Msg[%02X] but the datalen[%d] is INVALID\n", byPos[n + 2], nParsedDataLen);

				continue;
            } 

            // 4. EOI
            if (byPos[n + 2 + nParsedDataLen] != RPMSG_EOI)
            {
                TRACE("Found Msg[%02X] but the EOI[%02X] is INVALID \n", byPos[n + 2],
                    byPos[n + 2 + nParsedDataLen]);
                continue;
            }
            break;
        }
    }
    return;
}

void ReadAndParseConfigParameters(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
    AR_FRAME strMsgFrame;
    BYTE byBuff[65536 + 3];   // SOI（2） + EOI（1） + DataLen（2） + DataLen MAX（1024）//TODO:静态数组，清空或者用返回字节读取
    memset(byBuff, 0, sizeof(byBuff));

    int nReadBytes = pSTBatt_Rough->Rpmsg_faq_info.Read(pSTBatt_Rough->RpmsgHanle_faq, (char*)byBuff, sizeof(byBuff));

    if(nReadBytes != 0)
    {
        TRACEI("获取到读写返回的值\n");
        int i;
        for(i = 0; i < nReadBytes; ++i)
        {
            TRACEI("%02X ", byBuff[i]);
        }
        TRACEI("\n");
        TRACEI("结束\n");
    }

    BYTE* pFoundPos = byBuff;
    int nRemaining = nReadBytes;
    while (nRemaining > 0)
    {
        memset(&strMsgFrame, 0, sizeof(strMsgFrame));
        Find_RPMsg_Batt_Frame_FAQ(pFoundPos, nRemaining, &strMsgFrame);
        if (strMsgFrame.ID_Datalen == 0)
        {
            TRACEI("---Get Frm Is NULL---\n");
            break;
        }

        // 按信息类型分发：0x0A/0x0B 历史故障、0x0C/0x0D 历史事件、其它为参数响应
        switch (strMsgFrame.ID_MainIndex)
        {
            case 0x0A:  // 历史故障 组1
            case 0x0B:  // 历史故障 组2
            {
                int nGrp = GetHisRecordGroupIndex(strMsgFrame.ID_MainIndex);
                Parse_HisFaultRecord(pSTBatt_Rough, &strMsgFrame, nGrp);
                Send_BmsHistFaultEvent(pSTBatt_Rough, nGrp);
                break;
            }
            case 0x0C:  // 历史事件 组1
            case 0x0D:  // 历史事件 组2
            {
                int nGrp = GetHisRecordGroupIndex(strMsgFrame.ID_MainIndex);
                Parse_HisEventRecord(pSTBatt_Rough, &strMsgFrame, nGrp);
                Send_BmsHistEventEvent(pSTBatt_Rough, nGrp);
                break;
            }
            case 0x0F: 
            case 0x10: // DOD相关信息
            {
                Parse_DODRecord(pSTBatt_Rough, &strMsgFrame);
            }
            case 0x1C:
            case 0x1D:
            case 0x1E:
            case 0x1F: // 
            {
                // int nGrp = GetHisRecordGroupIndex(strMsgFrame.ID_MainIndex);
                Parse_S2C_CellResReal(pSTBatt_Rough, &strMsgFrame);
            }
            case 0x20:
            case 0x21:
            case 0x22:
            case 0x23: // 
            {
                // int nGrp = GetHisRecordGroupIndex(strMsgFrame.ID_MainIndex);
                Parse_S2C_CellResImag(pSTBatt_Rough, &strMsgFrame);
            }
            case 0x24:
            case 0x25: // 
            {
                // int nGrp = GetHisRecordGroupIndex(strMsgFrame.ID_MainIndex);
                Parse_S2C_CellIcStat(pSTBatt_Rough, &strMsgFrame);
            }

            default:  // 参数响应
            {
                int IdxNumners  = strMsgFrame.Data[1];
                int startSubIdx = strMsgFrame.Data[2];
                Push_Sys_Common_Get(pSTBatt_Rough, &strMsgFrame, IdxNumners, startSubIdx);
                break;
            }
        }

        // 跳过一帧：SOI(2) + 帧内容(ID_Datalen) + EOI(1)
        pFoundPos  += (strMsgFrame.ID_Datalen + 3);
        nRemaining -= (strMsgFrame.ID_Datalen + 3);
    }

    return;
}

BOOL GetSetParameter(AR_FRAME* ar_Frame, BYTE FunCode, int nParamId, const BYTE* tx_data, int len)
{
    if (!ar_Frame || !tx_data || len < 0 || len > (AR_MAX_DATA_LEN - 3))
        return FALSE;

    ar_Frame->ID_DstAddr = ADDR_A_KERNEL_MIN;
    ar_Frame->ID_SrcAddr = ADDR_R_KERNEL;
    ar_Frame->ID_FunCode = FunCode;
    ar_Frame->ID_Datalen = BMU02_FRAME_ID_LEN + BMU02_FRAME_DATA_HDR_LEN + len;

    if(FUNCODE_REQ_QUERY == FunCode)
    {
        if(nParamId < 0 && nParamId > 12)
        {
            return FALSE;
        }
        ar_Frame->Data[0] = 0x0C;
        if(nParamId >= 0 && nParamId <= 3)
        {
            ar_Frame->ID_MainIndex = BmsGroupConfigs[nParamId].BmsPraType - 5;
        }
        else
        {
            ar_Frame->ID_MainIndex = BmsGroupConfigs[nParamId].BmsPraType;
        }
        
        ar_Frame->Data[1] = BmsGroupConfigs[nParamId].endId - BmsGroupConfigs[nParamId].baseVarID + 1;
        ar_Frame->Data[2] = 0;  

        return TRUE;
    }

    for (int i = 0; i < BMS_GROUP_CONFIG_CNT; ++i) {
        if (nParamId >= BmsGroupConfigs[i].baseVarID && nParamId <= BmsGroupConfigs[i].endId) {
            ar_Frame->ID_MainIndex = BmsGroupConfigs[i].BmsPraType;
            ar_Frame->Data[0] = 0x0C;
            ar_Frame->Data[1] = 0x01;
            ar_Frame->Data[2] = (BYTE)(nParamId - BmsGroupConfigs[i].baseVarID);
            memcpy(&ar_Frame->Data[3], tx_data, len);

            return TRUE;
        }
    }

    return FALSE;
}

int BMU02_SendFrameByArFrame(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, const AR_FRAME* pArFrame)
{
    if (!pSTBatt_Rough || !pArFrame) {
        TRACEI("[BMU02_SendFrameByArFrame] ERR: 无效参数\n");
        return -1;
    }
    // 1. 从 AR_FRAME 提取字段
    uint8_t src_addr    = pArFrame->ID_SrcAddr;
    uint8_t dst_addr    = pArFrame->ID_DstAddr;
    uint8_t msg_type    = pArFrame->ID_MainIndex;   // BmsPraType
    uint8_t func_code   = pArFrame->ID_FunCode;     // 2
    uint8_t frame_flag  = pArFrame->Data[0];        // 0x0C
    uint8_t sub_idx_cnt = pArFrame->Data[1];        // 1
    uint8_t sub_idx_start = pArFrame->Data[2];      // 偏移量

    // 2. 计算数据体长度：ID_Datalen 包含 ID头 + DATA头 + 数据体
    //    ID_Datalen 已在 GetSetParameter 中被设置为 BMU02_FRAME_ID_LEN + BMU02_FRAME_DATA_HDR_LEN + len
    if (pArFrame->ID_Datalen < BMU02_FRAME_ID_LEN + BMU02_FRAME_DATA_HDR_LEN) {
        TRACEI("[BMU02_SendFrameByArFrame] ERR: ID_Datalen 过小 %d\n", pArFrame->ID_Datalen);
        return -1;
    }
    uint32_t data_body_len = pArFrame->ID_Datalen - BMU02_FRAME_ID_LEN - BMU02_FRAME_DATA_HDR_LEN;
    const uint8_t* data_body = &pArFrame->Data[3];
    // 3. 总帧长（与 ID_Datalen 一致，但重新计算确保安全）
    uint32_t total_len = BMU02_FRAME_ID_LEN + BMU02_FRAME_DATA_HDR_LEN + data_body_len;
    if (total_len > BMU02_FRAME_MAX_LEN) {
        TRACEI("[BMU02_SendFrameByArFrame] ERR: 帧超长 %d > %d\n", total_len, BMU02_FRAME_MAX_LEN);
        return -1;
    }

    // 4. 构建发送缓冲区
    BYTE tx_buf[BMU02_FRAME_MAX_LEN] = {0};
    uint32_t offset = 0;

    tx_buf[offset++] = (total_len >> 0)  & 0xFF;   // 最低字节
    tx_buf[offset++] = (total_len >> 8)  & 0xFF;
    tx_buf[offset++] = (total_len >> 16) & 0xFF;
    tx_buf[offset++] = (total_len >> 24) & 0xFF;   // 最高字节
    tx_buf[offset++] = func_code;
    tx_buf[offset++] = msg_type;
    tx_buf[offset++] = src_addr;
    tx_buf[offset++] = dst_addr;

    // DATA 头 (3字节)
    tx_buf[offset++] = frame_flag;
    tx_buf[offset++] = sub_idx_cnt;
    tx_buf[offset++] = sub_idx_start;

    // 数据体
    if (data_body_len > 0) {
        memcpy(&tx_buf[offset], data_body, data_body_len);
        offset += data_body_len;
    }

    // 5. 调试打印
    TRACEI("发送 BMU02 帧: ");
    for (uint32_t i = 0; i < total_len; ++i) {
        TRACEI("%02X ", tx_buf[i]);
    }
    TRACEI("\n");

    // 6. 发送
    if (pSTBatt_Rough->RpmsgHanle_faq != NULL) {
        int send_ret = pSTBatt_Rough->Rpmsg_faq_info.Write(pSTBatt_Rough->RpmsgHanle_faq, tx_buf, total_len);
        if (send_ret != 0) {
            return 0;
        } else {
            TRACEI("[BMU02_SendFrameByArFrame] ERR: rpmsg 发送失败, ret=%d\n", send_ret);
        }
    } else {
        TRACEI("[BMU02_SendFrameByArFrame] ERR: rpmsg 句柄为空\n");
    }

    return -1;
}



/*************************************************************************
 * Bat_Rpmsg_ExchangeData - 将03整体信息(RPMSG)数据同步到STBATT_SAMPLER_ROUGH已有字段
 * 
 * Args:
 *   pRough - STBATT_SAMPLER_ROUGH结构体指针
 *   nBattIdx - 电池索引 (0 ~ MAX_SINGLE_STBATT_NUM-1)
 *
 * 映射规则: 直接赋值，不转换单位
 * 不处理: 告警/故障位域
 *************************************************************************/
void Bat_Rpmsg_ExchangeData(STBATT_SAMPLER_ROUGH *pRough, int nBattIdx)
{
    if (!pRough || nBattIdx < 0 || nBattIdx >= MAX_SINGLE_STBATT_NUM || nBattIdx >= pRough->nSystemCfgBattNum) return;

    VAR_VALUE varV;

    /* =================================================================
     * 1. A_BLOB_SINGLE_STBATT_RUNINFO (单电池运行信息)
     *    pRough->blobSingleBattInfo[nBattIdx]
     * ================================================================= */

    /* EnergyStatus → blobSingleBattInfo */
    pRough->blobSingleBattInfo[nBattIdx].nSOC
        = pRough->EnergyStatus.power_info[nBattIdx].soc;               /* SOC值(千分比) */
    pRough->blobSingleBattInfo[nBattIdx].nSOH
        = pRough->EnergyStatus.power_info[nBattIdx].soh;               /* SOH值(千分比) */
    pRough->blobSingleBattInfo[nBattIdx].fChrg_Energy
        = pRough->EnergyStatus.energy_curr_stat[nBattIdx].remaining_energy; /* 剩余电能(0.1kWh) */
    pRough->blobSingleBattInfo[nBattIdx].AccuChrgCapacity
        = pRough->EnergyStatus.energy_total_stat[nBattIdx].total_charge_energy;    /* 累计充电电量(1AH) */
    pRough->blobSingleBattInfo[nBattIdx].fAccuDischrgCapacity
        = pRough->EnergyStatus.energy_total_stat[nBattIdx].total_discharge_energy; /* 累计放电电量(1AH) */
    pRough->blobSingleBattInfo[nBattIdx].fAccuChrgEnergy
        = pRough->EnergyStatus.energy_total_stat[nBattIdx].total_charge_power;     /* 累计充电电能(0.1kWh) */
    pRough->blobSingleBattInfo[nBattIdx].fAccuDischrgEnergy
        = pRough->EnergyStatus.energy_total_stat[nBattIdx].total_discharge_power;  /* 累计放电电能(0.1kWh) */
    pRough->blobSingleBattInfo[nBattIdx].nNow_DOD
        = pRough->EnergyStatus.dod_stat[nBattIdx].current_dod;         /* 当前放电深度(千分比) */

    /* StatusStat → blobSingleBattInfo */
    pRough->blobSingleBattInfo[nBattIdx].fPackTotalVolt
        = pRough->StatusStat.global_info[nBattIdx].total_voltage;      /* 电池总压(0.1V) */
    pRough->blobSingleBattInfo[nBattIdx].fPackTotalCurr
        = pRough->StatusStat.global_info[nBattIdx].total_current;      /* 电池总电流(0.1A) */
    // printf("ExchangeData[%d]: Volt=%.1f Curr=%.1f SOC=%u\n", nBattIdx,
    //        pRough->StatusStat.global_info[nBattIdx].total_voltage,
    //        pRough->StatusStat.global_info[nBattIdx].total_current,
    //        pRough->EnergyStatus.power_info[nBattIdx].soc);
    pRough->blobSingleBattInfo[nBattIdx].fAvg_CellTemp
        = pRough->StatusStat.feature_info[nBattIdx].cell_avg_temperature;  /* 单体平均温度(0.1℃) */
    pRough->blobSingleBattInfo[nBattIdx].fMax_CellVolt
        = pRough->StatusStat.feature_info[nBattIdx].max_voltage_value;     /* 单体最大电压(1mV) */
    pRough->blobSingleBattInfo[nBattIdx].fMin_CellVolt
        = pRough->StatusStat.feature_info[nBattIdx].min_voltage_value;     /* 单体最小电压(1mV) */
    pRough->blobSingleBattInfo[nBattIdx].fMax_CellTemp
        = pRough->StatusStat.feature_info[nBattIdx].max_temperature_value; /* 单体最高温度(0.1℃) */
    pRough->blobSingleBattInfo[nBattIdx].fMin_CellTemp
        = pRough->StatusStat.feature_info[nBattIdx].min_temperature_value; /* 单体最低温度(0.1℃) */
    pRough->blobSingleBattInfo[nBattIdx].fAllow_MaxChargeCurr
        = pRough->StatusStat.limit_info[nBattIdx].max_charge_current;      /* 最大充电电流限值(0.1A) */
    pRough->blobSingleBattInfo[nBattIdx].fAllow_MaxDisChargeCurr
        = pRough->StatusStat.limit_info[nBattIdx].max_discharge_current;   /* 最大放电电流限值(0.1A) */
    pRough->blobSingleBattInfo[nBattIdx].fAllow_LmtChargeVolt
        = pRough->StatusStat.limit_info[nBattIdx].max_charge_voltage;      /* 最大充电电压限值(0.1V) */
    pRough->blobSingleBattInfo[nBattIdx].fAllow_LmtDisChargeVolt
        = pRough->StatusStat.limit_info[nBattIdx].max_discharge_voltage;   /* 最大放电电压限值(0.1V) */

    // printf("ExchangeLimit[%d]: ChgVolt=%.1f ChgCurr=%.1f DisChgVolt=%.1f DisChgCurr=%.1f\n", nBattIdx,
    //        pRough->StatusStat.limit_info[nBattIdx].max_charge_voltage,
    //        pRough->StatusStat.limit_info[nBattIdx].max_charge_current,
    //        pRough->StatusStat.limit_info[nBattIdx].max_discharge_voltage,
    //        pRough->StatusStat.limit_info[nBattIdx].max_discharge_current);

    /* 通信状态: 有电压数据则认为BMS通信正常 */
    if (pRough->StatusStat.global_info[nBattIdx].total_voltage > 0.1f)
        pRough->blobSingleBattInfo[nBattIdx].emBattStatus_Run = STBATT_RUNSTAT_IDLE;

    /* DiagStat → blobSingleBattInfo */
    pRough->blobSingleBattInfo[nBattIdx].nLifeCycleTimes
        = pRough->DiagStat.time_stat[nBattIdx].cycle_count;            /* 充放电循环次数(1次) */
    pRough->blobSingleBattInfo[nBattIdx].uiPermissionLevel
        = pRough->StatFeedback.auth_state.current_auth_level;              /* 当前权限级别 */

    /* IMIU4 → blobSingleBattInfo */
    pRough->blobSingleBattInfo[nBattIdx].fInsulation_P_R
        = pRough->stImiu4Info[nBattIdx].positive_insulation_res;                 /* 正极绝缘阻值(kΩ) */
    pRough->blobSingleBattInfo[nBattIdx].fInsulation_N_R
        = pRough->stImiu4Info[nBattIdx].negative_insulation_res;                 /* 负极绝缘阻值(kΩ) */

    /* =================================================================
     * 2. A_BLOB_IFY_BMS_DATAINFO (IFY BMS数据信息)
     *    pRough->blobIfyBMS[nBattIdx]
     * ================================================================= */

    pRough->blobIfyBMS[nBattIdx].fPackTotalVolt
        = pRough->StatusStat.global_info[nBattIdx].total_voltage;      /* 电池总压(0.1V) */
    pRough->blobIfyBMS[nBattIdx].fPackTotalCurr
        = pRough->StatusStat.global_info[nBattIdx].total_current;      /* 电池总电流(0.1A) */
    pRough->blobIfyBMS[nBattIdx].fSOC
        = pRough->EnergyStatus.power_info[nBattIdx].soc;               /* SOC值(千分比) */
    pRough->blobIfyBMS[nBattIdx].fSOH
        = pRough->EnergyStatus.power_info[nBattIdx].soh;               /* SOH值(千分比) */
    pRough->blobIfyBMS[nBattIdx].fOutputPower
        = pRough->StatusStat.global_info[nBattIdx].output_power;       /* 输出功率(0.1kW) */
    pRough->blobIfyBMS[nBattIdx].fCellAddTotalVolt
        = pRough->StatusStat.feature_info[nBattIdx].cell_total_voltage;/* 单体总电压(0.1V) */
    pRough->blobIfyBMS[nBattIdx].usCellAverageVolt
        = (USHORT)pRough->StatusStat.feature_info[nBattIdx].cell_avg_voltage;    /* 单体平均电压(1mV) */
    pRough->blobIfyBMS[nBattIdx].sCellAverageTemp
        = (SHORT)pRough->StatusStat.feature_info[nBattIdx].cell_avg_temperature; /* 单体平均温度(0.1℃) */
    pRough->blobIfyBMS[nBattIdx].usMaxCellVlot_Volt
        = (USHORT)pRough->StatusStat.feature_info[nBattIdx].max_voltage_value;   /* 单体最大电压(1mV) */
    pRough->blobIfyBMS[nBattIdx].usMinCellVlot_Volt
        = (USHORT)pRough->StatusStat.feature_info[nBattIdx].min_voltage_value;   /* 单体最小电压(1mV) */
    pRough->blobIfyBMS[nBattIdx].sMaxCellTemp_Temp
        = (SHORT)pRough->StatusStat.feature_info[nBattIdx].max_temperature_value;/* 单体最高温度(0.1℃) */
    pRough->blobIfyBMS[nBattIdx].sMinCellTemp_Temp
        = (SHORT)pRough->StatusStat.feature_info[nBattIdx].min_temperature_value;/* 单体最低温度(0.1℃) */
    pRough->blobIfyBMS[nBattIdx].fAllow_MaxChargeCurr
        = pRough->StatusStat.limit_info[nBattIdx].max_charge_current;            /* 最大充电电流限值(0.1A) */
    pRough->blobIfyBMS[nBattIdx].fAllow_MaxDisChargeCurr
        = pRough->StatusStat.limit_info[nBattIdx].max_discharge_current;         /* 最大放电电流限值(0.1A) */
    pRough->blobIfyBMS[nBattIdx].fAllow_LmtChargeVolt
        = pRough->StatusStat.limit_info[nBattIdx].max_charge_voltage;            /* 最大充电电压限值(0.1V) */
    pRough->blobIfyBMS[nBattIdx].fAllow_LmtDisChargeVolt
        = pRough->StatusStat.limit_info[nBattIdx].max_discharge_voltage;         /* 最大放电电压限值(0.1V) */

    /* =================================================================
     * 3. A_BLOB_BRANCH_GRP_RECTDATA (支路组整流数据)
     *    pRough->branchGrpData[nBattIdx]
     * ================================================================= */

    pRough->branchGrpData[nBattIdx].fBatt_RealTime_Volt
        = pRough->StatusStat.global_info[nBattIdx].total_voltage;               /* 电池实时电压(0.1V) */
    pRough->branchGrpData[nBattIdx].fBatt_RealTime_Curr
        = pRough->StatusStat.global_info[nBattIdx].total_current;               /* 电池实时电流(0.1A) */
    pRough->branchGrpData[nBattIdx].nSOC
        = pRough->EnergyStatus.power_info[nBattIdx].soc;                        /* SOC值(千分比) */

    /* =================================================================
     * 4. A_BLOB_BRANCH_NODE_CALC_DATA (支路节点计算数据)
     *    pRough->calcNodeData[nBattIdx]
     * ================================================================= */

    pRough->calcNodeData[nBattIdx].nSOC
        = pRough->EnergyStatus.power_info[nBattIdx].soc;                        /* SOC值(千分比) */
    pRough->calcNodeData[nBattIdx].emBatt_WorkMode
        = pRough->emWorkMode[nBattIdx];                                         /* 电池工作模式 */

    /* =================================================================
     * 5. A_BLOB_GROUP_STBATT_RUNINFO (电池组运行信息 - 仅idx=0时赋值)
     *    pRough->blobGroupBattInfo
     * ================================================================= */

    if (nBattIdx == 0)
    {
        pRough->blobGroupBattInfo.fBattBusVolt
            = pRough->StatusStat.global_info[0].total_voltage;                  /* 电池母线电压(0.1V) */
        pRough->blobGroupBattInfo.fBattBusCurr
            = pRough->StatusStat.global_info[0].total_current;                  /* 电池母线电流(0.1A) */
        pRough->blobGroupBattInfo.fAvgSOC
            = pRough->EnergyStatus.power_info[0].soc;                           /* 平均SOC(千分比) */
        pRough->blobGroupBattInfo.fAvgSOH
            = pRough->EnergyStatus.power_info[0].soh;                           /* 平均SOH(千分比) */
        pRough->blobGroupBattInfo.fAll_Allow_LmtChargeVolt
            = pRough->StatusStat.limit_info[0].max_charge_voltage;              /* 允许充电电压限值(0.1V) */
        pRough->blobGroupBattInfo.fAll_Allow_LmtDisChargeVolt
            = pRough->StatusStat.limit_info[0].max_discharge_voltage;           /* 允许放电电压限值(0.1V) */
        pRough->blobGroupBattInfo.fAll_Allow_MaxChargeCurr
            = pRough->StatusStat.limit_info[0].max_charge_current;              /* 允许充电电流限值(0.1A) */
        pRough->blobGroupBattInfo.fAll_Allow_MaxDisChargeCurr
            = pRough->StatusStat.limit_info[0].max_discharge_current;           /* 允许放电电流限值(0.1A) */
        pRough->blobGroupBattInfo.fAll_ChargedEnergy
            = pRough->EnergyStatus.energy_total_stat[0].total_charge_energy;    /* 总充电电能(0.1kWh) */
        pRough->blobGroupBattInfo.fAll_DisChargedEnergy
            = pRough->EnergyStatus.energy_total_stat[0].total_discharge_energy; /* 总放电电能(0.1kWh) */

    }

    switch(pRough->StatusStat.status_info[nBattIdx].charge_discharge_status)
    {
        case 0: varV.emValue = STBATT_RUNSTAT_IDLE; break;
        case 1: varV.emValue = STBATT_RUNSTAT_CHARGE; break;
        case 2: varV.emValue = STBATT_RUNSTAT_DISCHARGE; break;
        default: varV.emValue = STBATT_RUNSTAT_COMM_FAIL;break;
    }
	Push_VarValue(SETTYPE_RAW_SP, pRough->hSampler, 501 + nBattIdx, 550, varV);

	/* 工作状态→告警状态映射 (param 552 SingleBatt Alarm Status): 供灯带/水冷机读取两簇故障状态
	 *   故障只认 work_status==5 → STBATT_ALMSTAT_FAULT(3), 其余 → STBATT_ALMSTAT_NOMAL(0)
	 *   work_status: 0-开机 1-待命 2-启动 3-运行 4-停机 5-故障 6-关机 */
	varV.emValue = (pRough->StatusStat.status_info[nBattIdx].work_status == 5)
	               ? STBATT_ALMSTAT_FAULT : STBATT_ALMSTAT_NOMAL;
	Push_VarValue(SETTYPE_RAW_SP, pRough->hSampler, 501 + nBattIdx, 552, varV);

    /* =================================================================
     * 6. 工作模式 (IMIU4 → pRough)
     * ================================================================= */

    pRough->emWorkMode[nBattIdx]
        = pRough->stImiu4Info[nBattIdx].system_work_mode;                                 /* 系统工作模式 */
}

void Push_BmsRunStatus(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
    VAR_VALUE varV;
    
    varV.blobValue = (BYTE *)&pSTBatt_Rough->EnergyStatus;
    Push_BlobValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 
		500, 509, (void*)varV.blobValue, sizeof(A_BLOB_ENERGY_STAT), BLOB_TYPE_ENERGY_STAT);
    
    varV.blobValue = (BYTE *)&pSTBatt_Rough->StatusStat;
    Push_BlobValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 
		500, 510, (void*)varV.blobValue, sizeof(A_BLOB_STATUS_STAT), BLOB_TYPE_STATUS_STAT);
        
    varV.blobValue = (BYTE *)&pSTBatt_Rough->DiagStat;
    Push_BlobValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 
		500, 511, (void*)varV.blobValue, sizeof(A_BLOB_DIAG_STAT), BLOB_TYPE_DIAG_STAT);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 932, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 933, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_3;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 934, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_4;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 935, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_5;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 936, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_6;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 937, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_7;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 938, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_8;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 939, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_9;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 940, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].alarm_info_10;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 941, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].fault_info_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 942, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].fault_info_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 943, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].fault_info_3;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 944, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].fault_info_4;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 945, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].fault_info_5;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 946, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].fault_info_6;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 947, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].fault_info_7;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 948, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[0].fault_info_8;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 949, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 950, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 951, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_3;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 952, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_4;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 953, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_5;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 954, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_6;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 955, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_7;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 956, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_8;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 957, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_9;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 958, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].alarm_info_10;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 959, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].fault_info_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 960, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].fault_info_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 961, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].fault_info_3;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 962, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].fault_info_4;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 963, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].fault_info_5;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 964, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].fault_info_6;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 965, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].fault_info_7;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 966, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBmsAlarmFault[1].fault_info_8;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 967, varV);


    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].battery_total_voltage;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 972, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].link_voltage;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 973, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].hall_5v_sample;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 974, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[0].hall_signal_sample;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 975, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].hall_current_low;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 976, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].hall_current_high;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 977, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[0].shunt_current_low;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 978, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].shunt_current_high;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 979, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].shunt_temperature;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 980, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[0].hv_chip_5v_feedback;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 981, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[0].hv_chip_minus5v_feedback;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 982, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[0].positive_insulation_res;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 983, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[0].negative_insulation_res;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 984, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[0].ntc1_temperature;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 985, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[0].ntc2_temperature;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 986, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].dido_gpio_signal;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 987, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].board_5v_feedback;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 988, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].system_work_mode;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 989, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].alarm_info_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 990, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].alarm_info_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 991, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].alarm_info_3;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 992, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].fault_info_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 993, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].fault_info_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 994, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[0].fault_info_3;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 995, varV);

    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.pt1000_selfcheck_temp_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 996, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.pt1000_selfcheck_temp_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 997, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.water_selfcheck_freq_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 998, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.water_selfcheck_freq_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 999, varV);
    
    varV.ulValue   =  pSTBatt_Rough->stBsu02Info.pt1000_5v_pullup_feedback;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1000, varV);
    
    varV.ulValue   =  pSTBatt_Rough->stBsu02Info.twelve_v_output_feedback;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1001, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.pt1000_temp_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1002, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.pt1000_temp_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1003, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.water_freq_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1004, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.water_freq_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1005, varV);
    
    varV.ulValue   =  pSTBatt_Rough->stBsu02Info.hv_interlock_pwm_amplitude;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1006, varV);
    
    varV.ulValue   =  pSTBatt_Rough->stBsu02Info.hv_interlock_pwm_duty;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1007, varV);
    
    varV.ulValue   =  pSTBatt_Rough->stBsu02Info.fg_pwm_amplitude;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1008, varV);
    
    varV.ulValue   =  pSTBatt_Rough->stBsu02Info.fg_pwm_duty;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1009, varV);
    
    varV.ulValue   =  pSTBatt_Rough->stBsu02Info.aerosol_concentration;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1010, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.aerosol_sensor_type;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1011, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.aerosol_status_fault;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1012, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.afe_bot_enum_count;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1013, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stBsu02Info.afe_top_enum_count;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1014, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].battery_total_voltage;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1015, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].link_voltage;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1016, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].hall_5v_sample;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1017, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[1].hall_signal_sample;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1018, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].hall_current_low;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1019, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].hall_current_high;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1020, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[1].shunt_current_low;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1021, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].shunt_current_high;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1022, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].shunt_temperature;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1023, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[1].hv_chip_5v_feedback;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1024, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[1].hv_chip_minus5v_feedback;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1025, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[1].positive_insulation_res;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1026, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[1].negative_insulation_res;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1027, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[1].ntc1_temperature;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1028, varV);
    
    varV.fValue   =  pSTBatt_Rough->stImiu4Info[1].ntc2_temperature;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1029, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].dido_gpio_signal;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1030, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].board_5v_feedback;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1031, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].system_work_mode;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1032, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].alarm_info_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1033, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].alarm_info_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1034, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].alarm_info_3;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1035, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].fault_info_1;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1036, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].fault_info_2;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1037, varV);
    
    varV.ulValue  =  pSTBatt_Rough->stImiu4Info[1].fault_info_3;
    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500 , 1038, varV);

    int i;
    for(i = 0; i < 2; ++i)
    {
        RefreshBmslArmOrFault(pSTBatt_Rough, i);
    }
}

void RefreshBmslArmOrFault(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	int nByte = 0;
	VAR_VALUE varV;
	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

    BMS_ALARM_FAULT *stAlarmFault = &pSTBatt_Rough->stBmsAlarmFault[nBattIdx];

    for(nByte = 0; nByte < 16; nByte++)
    {
        varV.emValue = ((stAlarmFault->alarm_info_1 >> nByte) & 0x01);
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 640 + nByte, varV);
    }

    for(nByte = 0; nByte < 16; nByte++)
    {
        varV.emValue = ((stAlarmFault->alarm_info_2 >> nByte) & 0x01);
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 656 + nByte, varV);
    }

    for(nByte = 0; nByte < 16; nByte++)
    {
        varV.emValue = ((stAlarmFault->alarm_info_3 >> nByte) & 0x01);
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 672 + nByte, varV);
    }

    for(nByte = 0; nByte < 16; nByte++)
    {
        varV.emValue = ((stAlarmFault->alarm_info_4 >> nByte) & 0x01);
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 804 + nByte, varV);
    }

    for(nByte = 0; nByte < 16; nByte++)
    {
        varV.emValue = ((stAlarmFault->alarm_info_5 >> nByte) & 0x01);
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 818 + nByte, varV);
    }

    for(nByte = 0; nByte < 16; nByte++)
    {
        varV.emValue = ((stAlarmFault->alarm_info_6 >> nByte) & 0x01);
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 836 + nByte, varV);
    }


    for(nByte = 0; nByte < 16; nByte++)
    {
        varV.emValue = ((stAlarmFault->fault_info_1 >> nByte) & 0x01);
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 704 + nByte, varV);
    }

    for(nByte = 0; nByte < 16; nByte++)
    {
        varV.emValue = ((stAlarmFault->fault_info_3 >> nByte) & 0x01);
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 736 + nByte, varV);
    }

    for(nByte = 0; nByte < 16; nByte++)
    {
        varV.emValue = ((stAlarmFault->fault_info_3 >> nByte) & 0x01);
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 752 + nByte, varV);
    }
    
	return;
}




