#include "../fcgi_defs.h"

// ─── BMS历史事件 事件类型名 Res_ID 映射（LoadLangText_ById 取翻译） ───────────
// 事件类型(eEvent_RcdSort)：5输出口 6输入口 7复用输出口 8复用输入口1 9复用输入口2
//                          10状态量变化 11接收控制命令 12接收调试命令 13接收参数配置 14内部事件 100用户自定义
#define RES_BMSEVENT_TYPE_OUTPUT     9448
#define RES_BMSEVENT_TYPE_INPUT      9449
#define RES_BMSEVENT_TYPE_MUXOUT     9450
#define RES_BMSEVENT_TYPE_MUXIN1     9451
#define RES_BMSEVENT_TYPE_MUXIN2     9452
#define RES_BMSEVENT_TYPE_STATE      9453
#define RES_BMSEVENT_TYPE_CMD        9454
#define RES_BMSEVENT_TYPE_DEBUG      9455
#define RES_BMSEVENT_TYPE_CFG        9456
#define RES_BMSEVENT_TYPE_SELF       9457
#define RES_BMSEVENT_TYPE_USER       9458
#define RES_BMSEVENT_TYPE_UNKNOWN    9459

// 用户自定义事件名 Res_ID（type=100，code 1~12 → 9460 + code - 1）
#define RES_BMSEVENT_USER_BASE       9460
#define RES_BMSEVENT_USER_LEN        12

// 事件类型描述
static const char* GetBmsEventTypeDesc(int nType)
{
    switch(nType)
    {
        case 5:   return LoadLangText_ById(RES_BMSEVENT_TYPE_OUTPUT);
        case 6:   return LoadLangText_ById(RES_BMSEVENT_TYPE_INPUT);
        case 7:   return LoadLangText_ById(RES_BMSEVENT_TYPE_MUXOUT);
        case 8:   return LoadLangText_ById(RES_BMSEVENT_TYPE_MUXIN1);
        case 9:   return LoadLangText_ById(RES_BMSEVENT_TYPE_MUXIN2);
        case 10:  return LoadLangText_ById(RES_BMSEVENT_TYPE_STATE);
        case 11:  return LoadLangText_ById(RES_BMSEVENT_TYPE_CMD);
        case 12:  return LoadLangText_ById(RES_BMSEVENT_TYPE_DEBUG);
        case 13:  return LoadLangText_ById(RES_BMSEVENT_TYPE_CFG);
        case 14:  return LoadLangText_ById(RES_BMSEVENT_TYPE_SELF);
        case 100: return LoadLangText_ById(RES_BMSEVENT_TYPE_USER);
        default:  return LoadLangText_ById(RES_BMSEVENT_TYPE_UNKNOWN);
    }
}

// 事件名称：用户自定义(100)→友好名；其它类型→直接显示原始事件代码
static void GetBmsEventNameStr(int nType, int nCode, char* pOut, int nSize)
{
    if(nType == 100 && nCode >= 1 && nCode <= RES_BMSEVENT_USER_LEN)
    {
        snprintf(pOut, nSize, "%s", LoadLangText_ById(RES_BMSEVENT_USER_BASE + (nCode - 1)));
    }
    else
    {
        snprintf(pOut, nSize, "%d", nCode);
    }
}

// ─── 保护状态描述（复用故障页 Res_ID） ──────────────────────────
static const char* GetBmsEventProtectDesc(int nState)
{
    switch(nState)
    {
        case 0: return LoadLangText_ById(9319);  // 未就绪
        case 1: return LoadLangText_ById(34);    // 正常
        case 2: return LoadLangText_ById(9321);  // 充电截止
        case 3: return LoadLangText_ById(9322);  // 放电截止
        case 4: return LoadLangText_ById(9323);  // 限流
        case 5: return LoadLangText_ById(9324);  // 充放电保护
        default: return LoadLangText_ById(31);   // 未知
    }
}

// ─── 工作状态描述（复用故障页 Res_ID） ──────────────────────────
static const char* GetBmsEventWorkDesc(int nState)
{
    switch(nState)
    {
        case 0: return LoadLangText_ById(9326);  // 初始化
        case 1: return LoadLangText_ById(6710);  // 待机
        case 2: return LoadLangText_ById(9328);  // 启动
        case 3: return LoadLangText_ById(9329);  // 运行
        case 4: return LoadLangText_ById(9330);  // 停止
        case 5: return LoadLangText_ById(9331);  // 故障
        case 6: return LoadLangText_ById(6702);  // 关机
        default: return LoadLangText_ById(31);   // 未知
    }
}

typedef struct tagBmsEventRecord
{
    int     nBattGrp;       // 电池组 0=组1 1=组2
    int     nEventType;     // 事件类型
    int     nEventCode;     // 事件代码
    time_t  tmEvent;        // 事件时间
    int     nProtectState;  // 系统保护状态
    int     nWorkState;     // 系统工作状态
    int     nCurrent;       // 系统输出电流[0.1A] (附件信息2)
    int     nCustom3;       // 用户自定义 (附件信息3)
    int     nCustom4;       // 用户自定义 (附件信息4)
    int     nCustom5;       // 用户自定义 (附件信息5)
}BMS_EVENT_RECORD;

// ─── 从 chargerDataRepo 读取BMS历史事件数据 ──────────────────
// 数据由 rundata_exchange.c 从 Data_BmsHistEvent 表定期刷新
static int LoadBmsEventRecords(FCGI_ROUGH_DATA* pFCGI_Rough, BMS_EVENT_RECORD* pOutBuf, int nMaxRecs)
{
    WEB_CHARGER_RUNDATA* pRepo = &pFCGI_Rough->chargerDataRepo;
    int nTotal = pRepo->nBmsHistEventTotal;
    int n;

    if(nTotal > nMaxRecs) nTotal = nMaxRecs;

    for(n = 0; n < nTotal; n++)
    {
        EVENT_BMS_HISTEVENT* pSrc = &pRepo->bmsHistEvents[n];
        BMS_EVENT_RECORD*     pDst = &pOutBuf[n];

        pDst->nBattGrp      = pSrc->nBattGrp;
        pDst->nEventType    = pSrc->nEventType;
        pDst->nEventCode    = pSrc->nEventCode;
        pDst->tmEvent       = pSrc->tmEvent;
        pDst->nProtectState = pSrc->nProtectState;
        pDst->nWorkState    = pSrc->nWorkState;
        pDst->nCurrent      = pSrc->nCurrent;
        pDst->nCustom3      = pSrc->nCustom3;
        pDst->nCustom4      = pSrc->nCustom4;
        pDst->nCustom5      = pSrc->nCustom5;
    }

    return nTotal;
}

// ─── 查询条件 ──────────────────────────────────────────────
typedef enum tagBmsEventDateCond
{
    BMSEVENT_DATE_ALL = 0,
    BMSEVENT_DATE_DAY,
    BMSEVENT_DATE_WEEK,
    BMSEVENT_DATE_MONTH
}BMSEVENT_DATE_COND;

typedef enum tagBmsEventGroupCond
{
    BMSEVENT_GROUP_ALL = 0,
    BMSEVENT_GROUP_1,
    BMSEVENT_GROUP_2
}BMSEVENT_GROUP_COND;

typedef struct tagBmsEventQueryCond
{
    int                 nPageId;
    int                 nPageVolume;
    BMSEVENT_DATE_COND  emDateCond;
    BMSEVENT_GROUP_COND emGroupCond;
    int                 nEtypeCond;     // 事件类型 0=全部 其它=类型值(5/6/7/8/9/10/11/12/13/14/100)
}BMSEVENT_QUERY_COND;

// ─── 日期条件过滤 ──────────────────────────────────────────
static BOOL IsEventRecMatch_Date(BMS_EVENT_RECORD* pRec, BMSEVENT_DATE_COND emCond)
{
    if(emCond == BMSEVENT_DATE_ALL)
        return TRUE;

    time_t tmNow = time(NULL);
    struct tm gmNow;
    gmtime_r(&tmNow, &gmNow);

    struct tm gmRec;
    gmtime_r(&pRec->tmEvent, &gmRec);

    if(emCond == BMSEVENT_DATE_DAY)
    {
        return (gmRec.tm_year == gmNow.tm_year &&
                gmRec.tm_mon  == gmNow.tm_mon  &&
                gmRec.tm_mday == gmNow.tm_mday);
    }
    else if(emCond == BMSEVENT_DATE_WEEK)
    {
        time_t tmWeekStart = tmNow - (gmNow.tm_wday * SEC_PER_DAY);
        return (pRec->tmEvent >= tmWeekStart);
    }
    else if(emCond == BMSEVENT_DATE_MONTH)
    {
        return (gmRec.tm_year == gmNow.tm_year &&
                gmRec.tm_mon  == gmNow.tm_mon);
    }
    return TRUE;
}

#define BMSEVENT_CSV_PATH  "/opt/appdata/data/bms_hist_event_export.csv"

static BOOL Make_BmsEventExport(FCGI_ROUGH_DATA* pFCGI_Rough, BMSEVENT_QUERY_COND* pCond)
{
    static BMS_EVENT_RECORD eventRecs[WEB_MAX_SUPPORT_BMSHISTEVENT];
    int nRawTotal = LoadBmsEventRecords(pFCGI_Rough, eventRecs, WEB_MAX_SUPPORT_BMSHISTEVENT);

    FILE *fp = fopen(BMSEVENT_CSV_PATH, "wb");
    if(fp == NULL) return FALSE;

    // 写入 UTF-8 BOM 头，否则 Excel 按 GBK 打开会中文乱码
    unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, 1, sizeof(bom), fp);

    // 表头：编号/组/事件时间/事件类型/事件名称/保护状态/工作状态/输出电流/附件3/附件4/附件5
    fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
        LoadLangText_ById(9334),  // 编号
        LoadLangText_ById(9344),  // 组
        LoadLangText_ById(9445),  // 事件时间
        LoadLangText_ById(9447),  // 事件名称
        LoadLangText_ById(9446),  // 事件类型
        LoadLangText_ById(9338),  // 保护状态
        LoadLangText_ById(9339),  // 工作状态
        LoadLangText_ById(9340),  // 输出电流
        LoadLangText_ById(9341),  // 附件信息3
        LoadLangText_ById(9342),  // 附件信息4
        LoadLangText_ById(9343)); // 附件信息5

    int n, nIdx = 0;
    char szTemp[128];
    char szEventName[64];
    for(n = 0; n < nRawTotal; n++)
    {
        BMS_EVENT_RECORD* pRec = &eventRecs[n];

        if(pCond->emGroupCond != BMSEVENT_GROUP_ALL &&
           pRec->nBattGrp != (pCond->emGroupCond - 1))
            continue;
        if(!IsEventRecMatch_Date(pRec, pCond->emDateCond))
            continue;
        if(pCond->nEtypeCond != 0 && pRec->nEventType != pCond->nEtypeCond)
            continue;

        GetBmsEventNameStr(pRec->nEventType, pRec->nEventCode, szEventName, sizeof(szEventName));

        nIdx++;
        fprintf(fp, "%d,%d,%s,%s,%s,%d,%d,%d,%d,%d,%d\n",
            nIdx,
            pRec->nBattGrp + 1,
            TimeToString(Conv_UTC_2LocalTime(pRec->tmEvent), TIME_CHN_FMT, szTemp, 32),
            szEventName,
            GetBmsEventTypeDesc(pRec->nEventType),
            pRec->nProtectState,
            pRec->nWorkState,
            pRec->nCurrent,
            pRec->nCustom3,
            pRec->nCustom4,
            pRec->nCustom5);
    }
    fclose(fp);

    // 导出文件名：设备ID + BMSEventRecord + 日期
    char szExportName[256];
    char szDate[32];
    snprintf(szExportName, sizeof(szExportName), "%s_BMSEventRecord_%s.csv",
             pFCGI_Rough->chargerDataRepo.versionInfos.szDevID,
             TimeToString(Conv_UTC_2LocalTime(time(NULL)), "%Y-%m-%d", szDate, 32));

    if(!UploadFile_2Client(BMSEVENT_CSV_PATH, szExportName))
    {
        unlink(BMSEVENT_CSV_PATH);
        return FALSE;
    }
    unlink(BMSEVENT_CSV_PATH);
    return TRUE;
}

static BOOL Make_BmsEventResult(FCGI_ROUGH_DATA* pFCGI_Rough, BMSEVENT_QUERY_COND* pCond)
{
    static BMS_EVENT_RECORD s_eventRecs[WEB_MAX_SUPPORT_BMSHISTEVENT];
    static int s_nEventTotal = 0;
    static BMSEVENT_QUERY_COND s_lastCond;
    static BOOL s_bFirstQuery = TRUE;

    BOOL bNeedRequery = FALSE;
    if(s_bFirstQuery)
    {
        bNeedRequery = TRUE;
        s_bFirstQuery = FALSE;
    }
    else if(s_lastCond.emDateCond  != pCond->emDateCond  ||
            s_lastCond.emGroupCond != pCond->emGroupCond ||
            s_lastCond.nEtypeCond  != pCond->nEtypeCond  ||
            s_lastCond.nPageVolume != pCond->nPageVolume)
    {
        bNeedRequery = TRUE;
    }
    else if(pCond->nPageId == 0)
    {
        bNeedRequery = TRUE;
    }

    memcpy(&s_lastCond, pCond, sizeof(BMSEVENT_QUERY_COND));

    if(bNeedRequery)
    {
        int nRawTotal = LoadBmsEventRecords(pFCGI_Rough, s_eventRecs, WEB_MAX_SUPPORT_BMSHISTEVENT);

        int n, nFiltered = 0;
        static BMS_EVENT_RECORD filtered[WEB_MAX_SUPPORT_BMSHISTEVENT];
        for(n = 0; n < nRawTotal && nFiltered < WEB_MAX_SUPPORT_BMSHISTEVENT; n++)
        {
            BMS_EVENT_RECORD* pRec = &s_eventRecs[n];
            if(pCond->emGroupCond != BMSEVENT_GROUP_ALL &&
               pRec->nBattGrp != (pCond->emGroupCond - 1))
            {
                continue;
            }
            if(!IsEventRecMatch_Date(pRec, pCond->emDateCond))
            {
                continue;
            }
            if(pCond->nEtypeCond != 0 && pRec->nEventType != pCond->nEtypeCond)
            {
                continue;
            }
            memcpy(&filtered[nFiltered], pRec, sizeof(BMS_EVENT_RECORD));
            nFiltered++;
        }
        memcpy(s_eventRecs, filtered, nFiltered * sizeof(BMS_EVENT_RECORD));
        s_nEventTotal = nFiltered;
    }

    int nTotalPage = (s_nEventTotal == 0) ? 1 :
        ((s_nEventTotal % pCond->nPageVolume) == 0) ?
        (s_nEventTotal / pCond->nPageVolume) : ((s_nEventTotal / pCond->nPageVolume) + 1);

    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", s_nEventTotal);
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
    char szEventName[64];

    for(n = nStart; n < nEnd && n < s_nEventTotal; n++)
    {
        BMS_EVENT_RECORD* pRec = &s_eventRecs[n];
        cJSON* pJRec = cJSON_CreateObject();

        // 序号
        cJSON_AddNumberToObject(pJRec, "idx", n + 1);

        // 电池组
        cJSON_AddNumberToObject(pJRec, "batt_grp", pRec->nBattGrp + 1);

        // 事件时间
        cJSON_AddStringToObject(pJRec, "tm_stored",
            TimeToString(Conv_UTC_2LocalTime(pRec->tmEvent), TIME_CHN_FMT, szTemp, 32));

        // 事件类型
        cJSON_AddStringToObject(pJRec, "event_type", GetBmsEventTypeDesc(pRec->nEventType));
        cJSON_AddNumberToObject(pJRec, "event_type_id", pRec->nEventType);

        // 事件名称 + 事件代码
        GetBmsEventNameStr(pRec->nEventType, pRec->nEventCode, szEventName, sizeof(szEventName));
        cJSON_AddStringToObject(pJRec, "event_name", szEventName);
        cJSON_AddNumberToObject(pJRec, "event_code", pRec->nEventCode);

        // 附件信息
        snprintf(szTemp, sizeof(szTemp), "%d:%s", pRec->nProtectState,
                 GetBmsEventProtectDesc(pRec->nProtectState));
        cJSON_AddStringToObject(pJRec, "protect_state", szTemp);

        snprintf(szTemp, sizeof(szTemp), "%d:%s", pRec->nWorkState,
                 GetBmsEventWorkDesc(pRec->nWorkState));
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

// ─── 页面入口：webpage_handler_bmsevent ──────────────────────
BOOL webpage_handler_bmsevent(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    if(pFCGI_Rough->emHttpReqMethod != HTTP_REQMETHOD_GET)
    {
        Make_Web_OutPut(104, "BMSEVENT page only allows GET!", NULL, NULL);
        return FALSE;
    }

    BMSEVENT_QUERY_COND cond;
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
        if(nTemp < 0 || nTemp > BMSEVENT_DATE_MONTH)
        {
            Make_Web_OutPut(104, "The 'cond_date' is OUT of range!", NULL, NULL);
            return FALSE;
        }
        cond.emDateCond = (BMSEVENT_DATE_COND)nTemp;
    }

    // 解析电池组条件 cond_group: 0=全部 1=组1 2=组2
    if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_group=", &nTemp))
    {
        if(nTemp < 0 || nTemp > BMSEVENT_GROUP_2)
        {
            Make_Web_OutPut(104, "The 'cond_group' is OUT of range!", NULL, NULL);
            return FALSE;
        }
        cond.emGroupCond = (BMSEVENT_GROUP_COND)nTemp;
    }

    // 解析事件类型条件 cond_etype: 0=全部 其它=类型值(5~14,100)
    if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_etype=", &nTemp))
    {
        if(nTemp < 0)
        {
            Make_Web_OutPut(104, "The 'cond_etype' is OUT of range!", NULL, NULL);
            return FALSE;
        }
        cond.nEtypeCond = nTemp;
    }

    TRACEX("bmsevent: pageId=%d page_vol=%d cond_date=%d cond_group=%d\n",
           cond.nPageId, cond.nPageVolume, cond.emDateCond, cond.emGroupCond);

    // 导出模式：不分页，生成CSV文件下载
    int nExport = 0;
    if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "export=", &nExport) && nExport == 1)
    {
        return Make_BmsEventExport(pFCGI_Rough, &cond);
    }

    return Make_BmsEventResult(pFCGI_Rough, &cond);
}
