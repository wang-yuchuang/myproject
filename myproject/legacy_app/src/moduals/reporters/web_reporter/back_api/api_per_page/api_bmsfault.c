#include "../fcgi_defs.h"

// ─── BMS历史故障 故障名 Res_ID 映射（已迁到 Resources 表，LoadLangText_ById 取翻译） ───────────
// 故障名 Res_ID: 0~127 → 9138 + faultCode；160~207 → 9266 + (faultCode - 160)
#define RES_BMSFAULT_NAME_GRP1_BASE   9138
#define RES_BMSFAULT_NAME_GRP2_BASE   9266
#define RES_BMSFAULT_UNKNOWN_FAULT    9333

// 故障类型
#define RES_BMSFAULT_FT_ALARM_GEN     9314
#define RES_BMSFAULT_FT_ALARM_CLR     9315
#define RES_BMSFAULT_FT_FAULT_GEN     9316
#define RES_BMSFAULT_FT_FAULT_CLR     9317
#define RES_BMSFAULT_FT_INVALID       9318

// 保护状态
#define RES_BMSFAULT_PS_NOT_READY     9319
#define RES_BMSFAULT_PS_NORMAL        34
#define RES_BMSFAULT_PS_CHG_CUTOFF    9321
#define RES_BMSFAULT_PS_DISCHG_CUTOFF 9322
#define RES_BMSFAULT_PS_LIMITED       9323
#define RES_BMSFAULT_PS_PROTECT       9324
#define RES_BMSFAULT_PS_UNKNOWN       31

// 工作状态
#define RES_BMSFAULT_WS_INIT          9326
#define RES_BMSFAULT_WS_STANDBY       6710
#define RES_BMSFAULT_WS_START         9328
#define RES_BMSFAULT_WS_RUN           9329
#define RES_BMSFAULT_WS_STOP          9330
#define RES_BMSFAULT_WS_FAULT         9331
#define RES_BMSFAULT_WS_OFF           6702

static int GetBmsFaultNameResId(int nFaultId)
{
    if(nFaultId >= 0 && nFaultId <= 127)
        return RES_BMSFAULT_NAME_GRP1_BASE + nFaultId;
    else if(nFaultId >= 160 && nFaultId <= 207)
        return RES_BMSFAULT_NAME_GRP2_BASE + (nFaultId - 160);
    else
        return RES_BMSFAULT_UNKNOWN_FAULT;
}

// ─── 故障类型描述 ──────────────────────────────────────────
static const char* GetFaultTypeDesc(int nType)
{
    switch(nType)
    {
        case 1: return LoadLangText_ById(RES_BMSFAULT_FT_ALARM_GEN);
        case 2: return LoadLangText_ById(RES_BMSFAULT_FT_ALARM_CLR);
        case 3: return LoadLangText_ById(RES_BMSFAULT_FT_FAULT_GEN);
        case 4: return LoadLangText_ById(RES_BMSFAULT_FT_FAULT_CLR);
        default: return LoadLangText_ById(RES_BMSFAULT_FT_INVALID);
    }
}

// ─── 保护状态描述 ──────────────────────────────────────────
static const char* GetProtectStateDesc(int nState)
{
    switch(nState)
    {
        case 0: return LoadLangText_ById(RES_BMSFAULT_PS_NOT_READY);
        case 1: return LoadLangText_ById(RES_BMSFAULT_PS_NORMAL);
        case 2: return LoadLangText_ById(RES_BMSFAULT_PS_CHG_CUTOFF);
        case 3: return LoadLangText_ById(RES_BMSFAULT_PS_DISCHG_CUTOFF);
        case 4: return LoadLangText_ById(RES_BMSFAULT_PS_LIMITED);
        case 5: return LoadLangText_ById(RES_BMSFAULT_PS_PROTECT);
        default: return LoadLangText_ById(RES_BMSFAULT_PS_UNKNOWN);
    }
}

// ─── 工作状态描述 ──────────────────────────────────────────
static const char* GetWorkStateDesc(int nState)
{
    switch(nState)
    {
        case 0: return LoadLangText_ById(RES_BMSFAULT_WS_INIT);
        case 1: return LoadLangText_ById(RES_BMSFAULT_WS_STANDBY);
        case 2: return LoadLangText_ById(RES_BMSFAULT_WS_START);
        case 3: return LoadLangText_ById(RES_BMSFAULT_WS_RUN);
        case 4: return LoadLangText_ById(RES_BMSFAULT_WS_STOP);
        case 5: return LoadLangText_ById(RES_BMSFAULT_WS_FAULT);
        case 6: return LoadLangText_ById(RES_BMSFAULT_WS_OFF);
        default: return LoadLangText_ById(RES_BMSFAULT_PS_UNKNOWN);
    }
}

// ─── 按故障ID查找名称 ──────────────────────────────────────
static const char* GetBmsFaultName(int nFaultId)
{
    return LoadLangText_ById(GetBmsFaultNameResId(nFaultId));
}

typedef struct tagBmsFaultRecord
{
    int     nBattGrp;       // 电池组 0=组1 1=组2
    int     nFaultType;     // 0:无效 1:告警产生 2:告警消除 3:故障产生 4:故障消除
    int     nFaultCode;     // 故障代码
    time_t  tmFault;        // 故障时间
    int     nProtectState;  // 系统保护状态
    int     nWorkState;     // 系统工作状态
    int     nCurrent;       // 系统输出电流[0.1A] (附件信息2)
    int     nCustom3;       // 用户自定义 (附件信息3)
    int     nCustom4;       // 用户自定义 (附件信息4)
    int     nCustom5;       // 用户自定义 (附件信息5)
    int     nStatus;        // 0=已恢复(消除) 1=未恢复(产生)
}BMS_FAULT_RECORD;

// ─── 从 chargerDataRepo 读取BMS历史故障数据 ──────────────────
// 数据由 rundata_exchange.c 从 Data_BmsHistFault 表定期刷新
static int LoadBmsFaultRecords(FCGI_ROUGH_DATA* pFCGI_Rough, BMS_FAULT_RECORD* pOutBuf, int nMaxRecs)
{
    WEB_CHARGER_RUNDATA* pRepo = &pFCGI_Rough->chargerDataRepo;
    int nTotal = pRepo->nBmsHistFaultTotal;
    int n;

    if(nTotal > nMaxRecs) nTotal = nMaxRecs;

    for(n = 0; n < nTotal; n++)
    {
        EVENT_BMS_HISTFAULT* pSrc = &pRepo->bmsHistFaults[n];
        BMS_FAULT_RECORD*    pDst = &pOutBuf[n];

        pDst->nBattGrp      = pSrc->nBattGrp;
        pDst->nFaultType    = pSrc->nFaultType;
        pDst->nFaultCode    = pSrc->nFaultCode;
        pDst->tmFault       = pSrc->tmFault;
        pDst->nProtectState = pSrc->nProtectState;
        pDst->nWorkState    = pSrc->nWorkState;
        pDst->nCurrent      = pSrc->nCurrent;
        pDst->nCustom3      = pSrc->nCustom3;
        pDst->nCustom4      = pSrc->nCustom4;
        pDst->nCustom5      = pSrc->nCustom5;
        pDst->nStatus       = pSrc->nStatus;
    }

    return nTotal;
}

// ─── 查询条件 ──────────────────────────────────────────────
typedef enum tagBmsFaultDateCond
{
    BMSFAULT_DATE_ALL = 0,
    BMSFAULT_DATE_DAY,
    BMSFAULT_DATE_WEEK,
    BMSFAULT_DATE_MONTH
}BMSFAULT_DATE_COND;

typedef enum tagBmsFaultGroupCond
{
    BMSFAULT_GROUP_ALL = 0,
    BMSFAULT_GROUP_1,
    BMSFAULT_GROUP_2
}BMSFAULT_GROUP_COND;

typedef struct tagBmsFaultQueryCond
{
    int                 nPageId;
    int                 nPageVolume;
    BMSFAULT_DATE_COND  emDateCond;
    BMSFAULT_GROUP_COND emGroupCond;
    int                 nFaultType;  // 0:全部 1:告警产生 2:告警消除 3:故障产生 4:故障消除
    int                 nType;       // 0:当前(未恢复) 1:历史(全部)
}BMSFAULT_QUERY_COND;

// ─── 日期条件过滤 ──────────────────────────────────────────
static BOOL IsFaultRecMatch_Date(BMS_FAULT_RECORD* pRec, BMSFAULT_DATE_COND emCond)
{
    if(emCond == BMSFAULT_DATE_ALL)
        return TRUE;

    time_t tmNow = time(NULL);
    struct tm gmNow;
    gmtime_r(&tmNow, &gmNow);

    struct tm gmRec;
    gmtime_r(&pRec->tmFault, &gmRec);

    if(emCond == BMSFAULT_DATE_DAY)
    {
        return (gmRec.tm_year == gmNow.tm_year &&
                gmRec.tm_mon  == gmNow.tm_mon  &&
                gmRec.tm_mday == gmNow.tm_mday);
    }
    else if(emCond == BMSFAULT_DATE_WEEK)
    {
        time_t tmWeekStart = tmNow - (gmNow.tm_wday * SEC_PER_DAY);
        return (pRec->tmFault >= tmWeekStart);
    }
    else if(emCond == BMSFAULT_DATE_MONTH)
    {
        return (gmRec.tm_year == gmNow.tm_year &&
                gmRec.tm_mon  == gmNow.tm_mon);
    }
    return TRUE;
}

#define BMSFAULT_CSV_PATH  "/opt/appdata/data/bms_hist_fault_export.csv"

static BOOL Make_BmsFaultExport(FCGI_ROUGH_DATA* pFCGI_Rough, BMSFAULT_QUERY_COND* pCond)
{
    static BMS_FAULT_RECORD faultRecs[WEB_MAX_SUPPORT_HISTORYALARM];
    int nRawTotal = LoadBmsFaultRecords(pFCGI_Rough, faultRecs, WEB_MAX_SUPPORT_HISTORYALARM);

    FILE *fp = fopen(BMSFAULT_CSV_PATH, "wb");
    if(fp == NULL) return FALSE;

    // 写入 UTF-8 BOM 头，否则 Excel 按 GBK 打开会中文乱码
    unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, 1, sizeof(bom), fp);

    fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
        LoadLangText_ById(9334),  // 编号
        LoadLangText_ById(9344),  // 组
        LoadLangText_ById(9335),  // 故障时间
        LoadLangText_ById(9336),  // 故障名称
        LoadLangText_ById(9337),  // 故障状态
        LoadLangText_ById(9338),  // 保护状态
        LoadLangText_ById(9339),  // 工作状态
        LoadLangText_ById(9340),  // 输出电流
        LoadLangText_ById(9341),  // 附件信息3
        LoadLangText_ById(9342),  // 附件信息4
        LoadLangText_ById(9343)); // 附件信息5

    int n, nIdx = 0;
    char szTemp[128];
    for(n = 0; n < nRawTotal; n++)
    {
        BMS_FAULT_RECORD* pRec = &faultRecs[n];

        if(pCond->emGroupCond != BMSFAULT_GROUP_ALL &&
           pRec->nBattGrp != (pCond->emGroupCond - 1))
            continue;
        if(pCond->nFaultType > 0 && pRec->nFaultType != pCond->nFaultType)
            continue;
        if((pCond->nType == 0 && pRec->nStatus != 1) ||
           (pCond->nType == 1 && pRec->nStatus != 0))
            continue;
        if(!IsFaultRecMatch_Date(pRec, pCond->emDateCond))
            continue;

        nIdx++;
        fprintf(fp, "%d,%d,%s,%s,%s,%s,%s,%d,%d,%d,%d\n",
            nIdx,
            pRec->nBattGrp + 1,
            TimeToString(Conv_UTC_2LocalTime(pRec->tmFault), TIME_CHN_FMT, szTemp, 32),
            GetBmsFaultName(pRec->nFaultCode),
            GetFaultTypeDesc(pRec->nFaultType),
            GetProtectStateDesc(pRec->nProtectState),
            GetWorkStateDesc(pRec->nWorkState),
            pRec->nCurrent,
            pRec->nCustom3,
            pRec->nCustom4,
            pRec->nCustom5);
    }
    fclose(fp);

    // 导出文件名：设备ID + BMSFaultRecord + 日期
    char szExportName[256];
    char szDate[32];
    snprintf(szExportName, sizeof(szExportName), "%s_BMSFaultRecord_%s.csv",
             pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
             TimeToString(Conv_UTC_2LocalTime(time(NULL)), "%Y-%m-%d", szDate, 32));

    if(!UploadFile_2Client(BMSFAULT_CSV_PATH, szExportName))
    {
        unlink(BMSFAULT_CSV_PATH);
        return FALSE;
    }
    unlink(BMSFAULT_CSV_PATH);
    return TRUE;
}

static BOOL Make_BmsFaultResult(FCGI_ROUGH_DATA* pFCGI_Rough, BMSFAULT_QUERY_COND* pCond)
{
    static BMS_FAULT_RECORD s_faultRecs[WEB_MAX_SUPPORT_HISTORYALARM];
    static int s_nFaultTotal = 0;
    static BMSFAULT_QUERY_COND s_lastCond;
    static BOOL s_bFirstQuery = TRUE;

    BOOL bNeedRequery = FALSE;
    if(s_bFirstQuery)
    {
        bNeedRequery = TRUE;
        s_bFirstQuery = FALSE;
    }
    else if(s_lastCond.emDateCond  != pCond->emDateCond  ||
            s_lastCond.emGroupCond != pCond->emGroupCond ||
            s_lastCond.nFaultType  != pCond->nFaultType  ||
            s_lastCond.nType       != pCond->nType       ||
            s_lastCond.nPageVolume != pCond->nPageVolume)
    {
        bNeedRequery = TRUE;
    }
    else if(pCond->nPageId == 0)
    {
        bNeedRequery = TRUE;
    }

    memcpy(&s_lastCond, pCond, sizeof(BMSFAULT_QUERY_COND));

    if(bNeedRequery)
    {
        // 从数据源加载原始记录
        int nRawTotal = LoadBmsFaultRecords(pFCGI_Rough, s_faultRecs, WEB_MAX_SUPPORT_HISTORYALARM);

        // 按条件过滤
        int n, nFiltered = 0;
        static BMS_FAULT_RECORD filtered[WEB_MAX_SUPPORT_HISTORYALARM];
        for(n = 0; n < nRawTotal && nFiltered < WEB_MAX_SUPPORT_HISTORYALARM; n++)
        {
            BMS_FAULT_RECORD* pRec = &s_faultRecs[n];
            if(pCond->emGroupCond != BMSFAULT_GROUP_ALL &&
               pRec->nBattGrp != (pCond->emGroupCond - 1))
            {
                continue;
            }
            if(pCond->nFaultType > 0 && pRec->nFaultType != pCond->nFaultType)
            {
                continue;
            }
            if((pCond->nType == 0 && pRec->nStatus != 1) ||
               (pCond->nType == 1 && pRec->nStatus != 0))
            {
                continue;
            }
            if(!IsFaultRecMatch_Date(pRec, pCond->emDateCond))
            {
                continue;
            }
            memcpy(&filtered[nFiltered], pRec, sizeof(BMS_FAULT_RECORD));
            nFiltered++;
        }
        memcpy(s_faultRecs, filtered, nFiltered * sizeof(BMS_FAULT_RECORD));
        s_nFaultTotal = nFiltered;
    }

    int nTotalPage = (s_nFaultTotal == 0) ? 1 :
        ((s_nFaultTotal % pCond->nPageVolume) == 0) ?
        (s_nFaultTotal / pCond->nPageVolume) : ((s_nFaultTotal / pCond->nPageVolume) + 1);

    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", s_nFaultTotal);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", nTotalPage);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", pCond->nPageId);

    cJSON* pJRecList = cJSON_CreateArray();
    if(pCond->nPageId + 1 > nTotalPage)
    {
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
        return TRUE;
    }

    int nStart = pCond->nPageId * pCond->nPageVolume;
    int nEnd   = nStart + pCond->nPageVolume;
    int n;
    char szTemp[128];

    for(n = nStart; n < nEnd && n < s_nFaultTotal; n++)
    {
        BMS_FAULT_RECORD* pRec = &s_faultRecs[n];
        cJSON* pJRec = cJSON_CreateObject();

        // 序号
        cJSON_AddNumberToObject(pJRec, "idx", n + 1);

        // 电池组
        cJSON_AddNumberToObject(pJRec, "batt_grp", pRec->nBattGrp + 1);

        // 存储时间
        cJSON_AddStringToObject(pJRec, "tm_stored",
            TimeToString(Conv_UTC_2LocalTime(pRec->tmFault), TIME_CHN_FMT, szTemp, 32));

        // 故障名称
        cJSON_AddStringToObject(pJRec, "fault_name", GetBmsFaultName(pRec->nFaultCode));

        // 故障代码
        cJSON_AddNumberToObject(pJRec, "fault_code", pRec->nFaultCode);

        // 故障状态
        cJSON_AddStringToObject(pJRec, "fault_status", GetFaultTypeDesc(pRec->nFaultType));
        cJSON_AddNumberToObject(pJRec, "fault_status_id", pRec->nFaultType);

        // 附件信息
        snprintf(szTemp, sizeof(szTemp), "%d:%s", pRec->nProtectState,
                 GetProtectStateDesc(pRec->nProtectState));
        cJSON_AddStringToObject(pJRec, "protect_state", szTemp);

        snprintf(szTemp, sizeof(szTemp), "%d:%s", pRec->nWorkState,
                 GetWorkStateDesc(pRec->nWorkState));
        cJSON_AddStringToObject(pJRec, "work_state", szTemp);

        cJSON_AddNumberToObject(pJRec, "output_current", pRec->nCurrent);

        // 用户自定义
        cJSON_AddNumberToObject(pJRec, "custom_3", pRec->nCustom3);
        cJSON_AddNumberToObject(pJRec, "custom_4", pRec->nCustom4);
        cJSON_AddNumberToObject(pJRec, "custom_5", pRec->nCustom5);

        cJSON_AddItemToArray(pJRecList, pJRec);
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    return TRUE;
}

// ─── 页面入口：webpage_handler_bmsfault ──────────────────────
BOOL webpage_handler_bmsfault(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    if(pFCGI_Rough->emHttpReqMethod != HTTP_REQMETHOD_GET)
    {
        Make_Web_OutPut(104, "BMSFAULT page only allows GET!", NULL, NULL);
        return FALSE;
    }

    BMSFAULT_QUERY_COND cond;
    ZERO_POBJS(&cond, 1);

    // 解析 pageId（默认0）
    if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "pageId=", &cond.nPageId))
    {
        cond.nPageId = 0;
    }

    // 解析 page_vol（默认20）
    if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "page_vol=", &cond.nPageVolume))
    {
        cond.nPageVolume = 20;
    }
    else if(cond.nPageVolume < 10 || cond.nPageVolume > 1000)
    {
        Make_Web_OutPut(104, "The 'page_vol' shall be in range (10 ~ 1000)!", NULL, NULL);
        return FALSE;
    }

    // 解析日期条件 cond_date: 0=全部 1=当天 2=本周 3=本月
    int nTemp = 0;
    if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_date=", &nTemp))
    {
        if(nTemp < 0 || nTemp > BMSFAULT_DATE_MONTH)
        {
            Make_Web_OutPut(104, "The 'cond_date' is OUT of range!", NULL, NULL);
            return FALSE;
        }
        cond.emDateCond = (BMSFAULT_DATE_COND)nTemp;
    }

    // 解析电池组条件 cond_group: 0=全部 1=组1 2=组2
    if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_group=", &nTemp))
    {
        if(nTemp < 0 || nTemp > BMSFAULT_GROUP_2)
        {
            Make_Web_OutPut(104, "The 'cond_group' is OUT of range!", NULL, NULL);
            return FALSE;
        }
        cond.emGroupCond = (BMSFAULT_GROUP_COND)nTemp;
    }

    // 解析故障状态 cond_fstatus: 0=全部 1=告警产生 2=告警消除 3=故障产生 4=故障消除
    if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_fstatus=", &nTemp))
    {
        if(nTemp < 0 || nTemp > 4)
        {
            Make_Web_OutPut(104, "The 'cond_fstatus' is OUT of range!", NULL, NULL);
            return FALSE;
        }
        cond.nFaultType = nTemp;
    }

    // 解析列表类型 type: 0=当前(未恢复) 1=历史(已恢复)，默认历史
    cond.nType = 1;
    if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &nTemp))
    {
        if(nTemp < 0 || nTemp > 1)
        {
            Make_Web_OutPut(104, "The 'type' is OUT of range!", NULL, NULL);
            return FALSE;
        }
        cond.nType = nTemp;
    }

    TRACEX("bmsfault: pageId=%d page_vol=%d type=%d cond_date=%d cond_group=%d cond_fstatus=%d\n",
           cond.nPageId, cond.nPageVolume, cond.nType, cond.emDateCond, cond.emGroupCond, cond.nFaultType);

    // 导出模式：不分页，生成CSV文件下载（当前页 type=0 只导未恢复，历史页 type=1 只导已恢复）
    int nExport = 0;
    if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "export=", &nExport) && nExport == 1)
    {
        return Make_BmsFaultExport(pFCGI_Rough, &cond);
    }

    return Make_BmsFaultResult(pFCGI_Rough, &cond);
}
