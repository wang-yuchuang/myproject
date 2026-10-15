/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : auxdev_sampler_main.c
** @Founder      : NJX01203
** @Date         : 2026.07.01
** @Function     : Combined RS485 sampler for U3Z water chiller + IMLU2 light strip on shared UART5
** @Instructions : 从零重写，参考备份框架和 R-core AuxDevTactic.c。
**                 修复备份的问题:
**                 1. Device ID 统一 (水冷机 961/960, 灯带 966/970)
**                 2. 灯带 Control 使用数据库真实 paramId(2100), 非 sentinel
**                 3. 灯带每次 Sample 检查颜色是否变化, 变化才发送
**===============================================================================================*/

#include "auxdev_sampler_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*=================================================================================================
** @Name      : getMonotonicMs
** @Function  : 获取单调时钟的毫秒时间戳，用于总线仲裁超时判断
**===============================================================================================*/
static UINT getMonotonicMs(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (UINT)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}

/*=================================================================================================
** @Name      : pushParamFloat / pushParamInt / pushParamEnum
** @Function  : Push_VarValue 的类型便捷封装
**===============================================================================================*/
static int pushParamFloat(HANDLE hSampler, int nDevId, int nParamId, float fVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.fValue = fVal;
    return Push_VarValue(SETTYPE_RAW_SP, hSampler, nDevId, nParamId, varV);
}

static int pushParamInt(HANDLE hSampler, int nDevId, int nParamId, long lVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.lValue = lVal;
    return Push_VarValue(SETTYPE_RAW_SP, hSampler, nDevId, nParamId, varV);
}

static int pushParamEnum(HANDLE hSampler, int nDevId, int nParamId, long emVal)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.emValue = (_ENUM)emVal;
    return Push_VarValue(SETTYPE_RAW_SP, hSampler, nDevId, nParamId, varV);
}

/*=================================================================================================
** @Name      : pushParamInvalid
** @Function  : 推送无效值 (SAMP_INVALID_VALUE)，框架据此自动置 bRawInvalid=TRUE
**===============================================================================================*/
static void pushParamInvalid(HANDLE hSampler, int nDevId, int nParamId)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));
    varV.lValue = SAMP_INVALID_VALUE;
    Push_VarValue(SETTYPE_RAW_SP, hSampler, nDevId, nParamId, varV);
}

/*=================================================================================================
** @Name      : pushChillerParams
** @Function  : 将水冷机解析后的 14 个参数推送到框架
**===============================================================================================*/
static void pushChillerParams(HANDLE hSampler, const AUX_DEV *pDev)
{
    const int nDevId = pDev->nDevId;
    const U3Z_STATE *pS = &pDev->chillerState;

    pushParamFloat(hSampler, nDevId, CHILLER_PARAM_NTC1TEMP,    pS->ntc1Temp);
    pushParamFloat(hSampler, nDevId, CHILLER_PARAM_NTC2TEMP,    pS->ntc2Temp);
    pushParamFloat(hSampler, nDevId, CHILLER_PARAM_NTC3TEMP,    pS->ntc3Temp);
    pushParamFloat(hSampler, nDevId, CHILLER_PARAM_NTC4TEMP,    pS->ntc4Temp);
    pushParamInt  (hSampler, nDevId, CHILLER_PARAM_DI,          (long)pS->di);
    pushParamInt  (hSampler, nDevId, CHILLER_PARAM_DO,          (long)pS->doVal);
    pushParamInt  (hSampler, nDevId, CHILLER_PARAM_PWM1,        (long)pS->pwm1);
    pushParamInt  (hSampler, nDevId, CHILLER_PARAM_PWM3,        (long)pS->pwm3);
    pushParamInt  (hSampler, nDevId, CHILLER_PARAM_PT1000STATE, (long)pS->pt1000State);
    pushParamInt  (hSampler, nDevId, CHILLER_PARAM_ALARM,       (long)pS->alarm);
    pushParamEnum (hSampler, nDevId, CHILLER_PARAM_PUMPSTATE,   (long)pS->pumpState);
    pushParamFloat(hSampler, nDevId, CHILLER_PARAM_PS1PRES,     pS->ps1Pres);
    pushParamFloat(hSampler, nDevId, CHILLER_PARAM_PS2PRES,     pS->ps2Pres);
    pushParamEnum (hSampler, nDevId, CHILLER_PARAM_COMMSTAT,    0);  /* 0=通信正常 */
    /* 主机侧状态 (非设备上报): 开关机/控制模式随运行态推送 */
    pushParamEnum (hSampler, nDevId, CHILLER_PARAM_POWER_STATE, (long)pDev->powerSwitch);
    pushParamEnum (hSampler, nDevId, CHILLER_PARAM_MODE_STATE,  (long)pDev->ctrlMode);

}

/*=================================================================================================
** @Name      : invalidateChillerParams
** @Function  : 通信故障时将 13 个水冷机数据参数标记为无效 (COMMSTAT 除外)
**===============================================================================================*/
static void invalidateChillerParams(HANDLE hSampler, const AUX_DEV *pDev)
{
    const int nDevId = pDev->nDevId;
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_NTC1TEMP);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_NTC2TEMP);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_NTC3TEMP);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_NTC4TEMP);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_DI);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_DO);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_PWM1);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_PWM3);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_PT1000STATE);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_ALARM);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_PUMPSTATE);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_PS1PRES);
    pushParamInvalid(hSampler, nDevId, CHILLER_PARAM_PS2PRES);
}


/*=================================================================================================
** @Name      : pollChiller
** @Function  : 轮询水冷机: 发送查询帧 -> 等待响应 -> 解析 -> 推送参数
**              占用 UART5 总线 AUXDEV_UART5_TIMESLICE_MS ms
**===============================================================================================*/
static BOOL pollChiller(AUXDEV_SAMPLER *pSamp, COMMPORT *pComm, HANDLE hPort)
{
    u8        txBuf[AUXDEV_U3Z_TX_BUF_LEN];
    u8        queryData;
    int       nTxLen, nWritten, nRead;
    UINT      tStart, tElapsed;
    U3Z_STATE st;

    if(pComm == NULL || pComm->Write == NULL || pComm->Read == NULL)
        return FALSE;

    /* 组帧 — 协议 V1.02: 0x21 查询必须携带 1 字节 DataType (0x00=状态信息)。
     * 旧实现发空查询(DataLen=0), 设备按版本查询应答(DataType=0x01 版本帧), 状态解析永远失败 */
    queryData = U3Z_DATATYPE_STATUS;
    nTxLen = u3z_build_frame(U3Z_CMD_QUERY_STATUS, &queryData, 1, pSamp->frameId++, txBuf, sizeof(txBuf));
    if(nTxLen <= 0) return FALSE;
    if(pSamp->frameId > 255) pSamp->frameId = 0;

    /* 清 RX 缓冲 */
    if(pComm->Control != NULL)
        pComm->Control(hPort, COMM_PURGE_RXCLEAR, NULL, 0);

    /* 发送 */
    nWritten = pComm->Write(hPort, txBuf, nTxLen);
    if(nWritten != nTxLen) return FALSE;

    /* 接收 (最长等待 AUXDEV_UART5_TIMESLICE_MS) */
    pSamp->chiller.nRxLen = 0;
    tStart = getMonotonicMs();
    while(pSamp->chiller.nRxLen < (int)sizeof(pSamp->chiller.rxBuf))
    {
        nRead = pComm->Read(hPort,
                            pSamp->chiller.rxBuf + pSamp->chiller.nRxLen,
                            (int)sizeof(pSamp->chiller.rxBuf) - pSamp->chiller.nRxLen);
        if(nRead > 0)
        {
            pSamp->chiller.nRxLen += nRead;
            /* 收到 EOI 标志着帧完整 */
            if(pSamp->chiller.nRxLen >= U3Z_MIN_FRAME_LEN
               && pSamp->chiller.rxBuf[pSamp->chiller.nRxLen - 1] == U3Z_EOI)
                break;
        }
        tElapsed = getMonotonicMs() - tStart;
        if(tElapsed >= AUXDEV_UART5_TIMESLICE_MS) break;
    }

    /* 解析 */
    memset(&st, 0, sizeof(st));
    if(!u3z_parse_frame(pSamp->chiller.rxBuf, pSamp->chiller.nRxLen, &st))
        return FALSE;

    /* 成功: 更新状态、推送参数、标记总线占用 */
    pSamp->chiller.chillerState = st;
    if(pSamp->chiller.nCommFailCnt > 0) pSamp->chiller.nCommFailCnt = 0;
    if(!pSamp->chiller.bCommOK)
    {
        pSamp->chiller.bCommOK = TRUE;
        SetCommStatus(pSamp->hSampler, pSamp->chiller.nDevId, TRUE);
        pushParamEnum(pSamp->hSampler, pSamp->chiller.nDevId, CHILLER_PARAM_COMMSTAT, 0); /* 0=通信正常 */
    }
    pushChillerParams(pSamp->hSampler, &pSamp->chiller);

    pSamp->bUart5BusBusy = TRUE;
    pSamp->nUart5BusFreeTimeMs = getMonotonicMs() + AUXDEV_UART5_TIMESLICE_MS;
    return TRUE;
}

/*=================================================================================================
** @Name      : sendUpperStatus
** @Function  : 向水冷机发送 CMD 0x11 上层状态报文 (A枪充电状态=powerSwitch)
**              文档要求每隔 2 秒发送或状态变化立即发送
**===============================================================================================*/
static BOOL sendUpperStatus(AUXDEV_SAMPLER *pSamp, COMMPORT *pComm, HANDLE hPort)
{
    u8   txBuf[AUXDEV_U3Z_TX_BUF_LEN];
    u8   rxBuf[AUXDEV_U3Z_RX_BUF_LEN];
    int  nTxLen, nWritten, nRead, nRxLen;
    UINT tStart, tElapsed;
    u8   data[3];

    if(pComm == NULL || pComm->Write == NULL || pComm->Read == NULL)
        return FALSE;

    /* CMD 0x11 Data: DataType=0x00(上级设备状态), DataNo=1(A枪), Data=powerSwitch */
    data[0] = 0x00;
    data[1] = 0x01;
    data[2] = pSamp->chiller.powerSwitch;

    nTxLen = u3z_build_frame(U3Z_CMD_UPPER_STATUS, data, 3, pSamp->frameId++, txBuf, sizeof(txBuf));
    if(nTxLen <= 0) return FALSE;
    if(pSamp->frameId > 255) pSamp->frameId = 0;

    if(pComm->Control != NULL)
        pComm->Control(hPort, COMM_PURGE_RXCLEAR, NULL, 0);

    nWritten = pComm->Write(hPort, txBuf, nTxLen);
    if(nWritten != nTxLen) return FALSE;

    /* 接收响应 (等待 RTN, 最长 AUXDEV_UART5_TIMESLICE_MS) */
    nRxLen = 0;
    tStart = getMonotonicMs();
    while(nRxLen < (int)sizeof(rxBuf))
    {
        nRead = pComm->Read(hPort, rxBuf + nRxLen, (int)sizeof(rxBuf) - nRxLen);
        if(nRead > 0)
        {
            nRxLen += nRead;
            if(nRxLen >= U3Z_MIN_FRAME_LEN && rxBuf[nRxLen - 1] == U3Z_EOI)
                break;
        }
        tElapsed = getMonotonicMs() - tStart;
        if(tElapsed >= AUXDEV_UART5_TIMESLICE_MS) break;
    }

    /* 标记总线占用 */
    pSamp->bUart5BusBusy = TRUE;
    pSamp->nUart5BusFreeTimeMs = getMonotonicMs() + AUXDEV_UART5_TIMESLICE_MS;
    return TRUE;
}


/*=================================================================================================
** @Name      : refreshLed
** @Function  : 发送灯带颜色帧 (仅当颜色变化时调用)
**===============================================================================================*/
static BOOL refreshLed(AUXDEV_SAMPLER *pSamp, COMMPORT *pComm, HANDLE hPort)
{
    const u8 *pBuf;
    int       nLen, nWritten;

    if(pComm == NULL || pComm->Write == NULL) return FALSE;

    pBuf = imlu2_get_color_buffer(pSamp->led.pendingColor);
    if(pBuf == NULL) return FALSE;
    nLen = imlu2_get_buffer_len();

    if(pComm->Control != NULL)
        pComm->Control(hPort, COMM_PURGE_RXCLEAR, NULL, 0);

    nWritten = pComm->Write(hPort, (void *)pBuf, nLen);
    if(nWritten != nLen) return FALSE;

    pSamp->led.lastSentColor = pSamp->led.pendingColor;
    pSamp->led.bColorDirty = FALSE;

    pSamp->bUart5BusBusy = TRUE;
    pSamp->nUart5BusFreeTimeMs = getMonotonicMs() + AUXDEV_UART5_TIMESLICE_MS;
    return TRUE;
}

/*=================================================================================================
** @Name      : queryLedOnline
** @Function  : 向灯带发送 41H 软件版本查询帧并读取应答, 判定在线状态。
**              应答 ~21C36200...(RTN=00)...\r 即在线; 不解析 INFO/CHKSUM。
**              占用 UART5 总线 AUXDEV_UART5_TIMESLICE_MS ms
**===============================================================================================*/
static BOOL queryLedOnline(AUXDEV_SAMPLER *pSamp, COMMPORT *pComm, HANDLE hPort)
{
    const u8 *pBuf;
    int       nLen, nWritten, nRead;
    UINT      tStart, tElapsed;

    if(pComm == NULL || pComm->Write == NULL || pComm->Read == NULL)
        return FALSE;

    pBuf = imlu2_get_version_query_buffer();
    nLen = imlu2_get_version_query_len();

    /* 清 RX 缓冲 */
    if(pComm->Control != NULL)
        pComm->Control(hPort, COMM_PURGE_RXCLEAR, NULL, 0);

    /* 发送 41H 查询帧 */
    nWritten = pComm->Write(hPort, (void *)pBuf, nLen);
    if(nWritten != nLen) return FALSE;

    /* 接收 (最长等待 AUXDEV_UART5_TIMESLICE_MS) */
    pSamp->led.nRxLen = 0;
    tStart = getMonotonicMs();
    while(pSamp->led.nRxLen < (int)sizeof(pSamp->led.rxBuf))
    {
        nRead = pComm->Read(hPort,
                            pSamp->led.rxBuf + pSamp->led.nRxLen,
                            (int)sizeof(pSamp->led.rxBuf) - pSamp->led.nRxLen);
        if(nRead > 0)
        {
            pSamp->led.nRxLen += nRead;
            if(pSamp->led.nRxLen >= IMLU2_MIN_FRAME_LEN
               && pSamp->led.rxBuf[pSamp->led.nRxLen - 1] == '\r')
                break;
        }
        tElapsed = getMonotonicMs() - tStart;
        if(tElapsed >= AUXDEV_UART5_TIMESLICE_MS) break;
    }

    /* 标记总线占用 */
    pSamp->bUart5BusBusy = TRUE;
    pSamp->nUart5BusFreeTimeMs = getMonotonicMs() + AUXDEV_UART5_TIMESLICE_MS;

    return (BOOL)imlu2_parse_version_response(pSamp->led.rxBuf, pSamp->led.nRxLen);
}

/*=================================================================================================
** @Name      : ledDecide
** @Function  : LED 灯带自动颜色判决 (参考 R-core AuxDevTactic.c:587 JudgeImlu2LightStripColor)
**              读两簇(501/502) DPR 状态参数, 按优先级返回颜色. 只读 DPR 不占总线.
**              优先级: 故障(系统/电池) > 充电 > 放电 > 空闲/待机
** @Notes     : DPR 读不到值时 emValue=0, 各参数 0 值均不触发任何灯 (安全默认)
**              devId=200 是 CCU/STS 运行实例 (SQL Belonged_DeviceModel=201)
/*=================================================================================================*/
static IMLU2_COLOR ledDecide(SAMPLER *pSampler)
{
    DATA_PROCESSOR *pDPR = (DATA_PROCESSOR *)pSampler->hDPR;
    VAR_VALUE        varV;
    BOOL             charging, dischrg, sysFault, battFault;
    int              nDev, n;
    long             emRun[LED_DPR_BATT_GROUP_NUM];
    long             emWork[LED_DPR_BATT_GROUP_NUM];

    if(pDPR == NULL) return IMLU2_GREEN;

    /* 1. 两簇聚合 (501/502): Run Status(param 550) + Alarm Status(param 552)
     *    RunStatus: 4=充电 5=放电
     *    AlarmStatus: 0=正常 1=告警 2=保护 3=故障 (由 work_status==5 映射)
     *    两簇共用一个水冷机/灯带, 任一簇生效即可 */
    charging  = FALSE;
    dischrg   = FALSE;
    battFault = FALSE;
    for(n = 0; n < LED_DPR_BATT_GROUP_NUM; n++)
    {
        nDev = LED_DPR_DEV_BATTGRP + n;   /* 501, 502 */

        memset(&varV, 0, sizeof(varV));
        pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, nDev, LED_DPR_PARAM_RUNSTATUS, &varV, sizeof(varV));
        emRun[n] = varV.emValue;
        if(emRun[n] == 4) charging = TRUE;
        if(emRun[n] == 5) dischrg  = TRUE;

        memset(&varV, 0, sizeof(varV));
        pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, nDev, LED_DPR_PARAM_WORKSTATUS, &varV, sizeof(varV));
        emWork[n] = varV.emValue;
        if(emWork[n] == STBATT_ALMSTAT_FAULT) battFault = TRUE;   /* 告警状态=故障 */
    }

    /* 2. System Fault (devId=200, paramId=103): ENUM 0=Normal 1=Available 2=Faulted 3=Upgrading */
    memset(&varV, 0, sizeof(varV));
    pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, LED_DPR_DEV_CCU, LED_DPR_PARAM_SYSSTATUS, &varV, sizeof(varV));
    sysFault = (varV.emValue == 2);

    /* 优先级判决: 故障 > 充电 > 放电 > 空闲/待机 */
    if(sysFault || battFault)   return IMLU2_RED;
    if(charging)                return IMLU2_YELLOW;
    if(dischrg)                 return IMLU2_BLUE;
    return IMLU2_GREEN;
}

/*=================================================================================================
** @Name      : Initialize
** @Function  : sampler 框架入口，初始化数据结构并打开 CommPort
**===============================================================================================*/
DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
    SAMPLER         *pSampler = (SAMPLER *)hSamplerSelf;
    AUXDEV_SAMPLER  *pSamp;

    pSampler->pDataHeap = (void *)malloc(sizeof(AUXDEV_SAMPLER));
    if(pSampler->pDataHeap == NULL)
    {
        TRACE("[auxdev] Initialize: malloc failed\n");
        return FALSE;
    }
    memset(pSampler->pDataHeap, 0, sizeof(AUXDEV_SAMPLER));
    pSamp = (AUXDEV_SAMPLER *)pSampler->pDataHeap;

    pSamp->hSampler   = hSamplerSelf;
    pSamp->nSamplerId = pSampler->info.nSamplerID;

    /* 水冷机设备初始化 */
    pSamp->chiller.nDevId    = CHILLER_DEV_ID;
    pSamp->chiller.nDevModel = CHILLER_DEV_MODEL;
    pSamp->chiller.role      = AUXDEV_ROLE_CHILLER;
    pSamp->chiller.bCommOK   = FALSE;

    /* 水冷机安装类型 (param 2099): 已安装 -> bCommOK 初始 TRUE, 未安装 -> FALSE。
     * 灯带 (IMLU2) 通过 41H 版本查询独立检测在线, 不再镜像水冷机状态。 */
    {
        DATA_PROCESSOR *pDPR = (DATA_PROCESSOR *)pSampler->hDPR;
        if(pDPR != NULL && pDPR->Get != NULL)
        {
            VAR_VALUE varV;
            memset(&varV, 0, sizeof(varV));
            if(ERR_DPR_OK == pDPR->Get(hSamplerSelf, GETTYPE_ST_VAR, CHILLER_DEV_ID,
                                       CHILLER_PARAM_INSTALL_TYPE, &varV, sizeof(varV)))
            {
                pSamp->chiller.bInstalled = (varV.emValue == 1);   /* 0=未安装, 1=已安装 */
                pSamp->chiller.bCommOK    = pSamp->chiller.bInstalled;
            }
        }
    }

    /* 灯带设备初始化 */
    pSamp->led.nDevId         = LED_DEV_ID;
    pSamp->led.nDevModel      = LED_DEV_MODEL;
    pSamp->led.role           = AUXDEV_ROLE_LED;
    pSamp->led.lastSentColor  = IMLU2_BLACK;
    pSamp->led.pendingColor   = IMLU2_BLACK;
    pSamp->led.bColorDirty    = TRUE;
    pSamp->led.bCommOK       = TRUE;   /* 灯带无安装判断: 始终监控通信。必须初始 TRUE，
                                          否则失败路径 && bCommOK 死锁，首次断线不告警 */

    pSamp->frameId            = 0;
    pSamp->nCycleCnt          = 0;
    pSamp->bUart5BusBusy      = FALSE;
    pSamp->nUart5BusFreeTimeMs = 0;

    /* 初始状态推送: 通信未建立，标记故障 + 数据无效
     * 防止 UI 因数据库默认值 0 而误显示"通信正常" */
    pushParamEnum(hSamplerSelf, pSamp->chiller.nDevId, CHILLER_PARAM_COMMSTAT, 1);  /* 1=通信故障 */
    invalidateChillerParams(hSamplerSelf, &pSamp->chiller);
    pushParamEnum(hSamplerSelf, pSamp->led.nDevId, LED_PARAM_COMMSTAT, 1);   /* 1=通信故障 (待 41H 心跳确认) */

    /* 获取 CommPort */
    if(GetCommPort_ById(hSamplerSelf, AUXDEV_COMMPORT_ID, pSampler->pUsedCommPort) == NULL)
    {
        TRACE("[auxdev] Initialize WARNING: CommPort %d not available, sampler idle\n", AUXDEV_COMMPORT_ID);
        pSamp->bInited = FALSE;
        return TRUE;
    }

    /* 打开 CommPort */
    int nErrCode = 0;
    pSampler->hCommPort = pSampler->pUsedCommPort->Open(
        pSampler->pUsedCommPort->info.szPortAttri,
        pSampler->info.szSamplerSetting,
        COMM_CLIENT_MODE, 1000, &nErrCode);
    if(pSampler->hCommPort == NULL || nErrCode != ERR_COMM_OK)
    {
        TRACE("[auxdev] Initialize WARNING: Open(%s/%s) failed err=%d, sampler idle\n",
               pSampler->pUsedCommPort->info.szPortAttri,
               pSampler->info.szSamplerSetting, nErrCode);
        pSamp->bInited = FALSE;
        return TRUE;
    }
    pSampler->pUsedCommPort->Control(pSampler->hCommPort, COMM_PURGE_RXCLEAR, NULL, 0);
    pSampler->pUsedCommPort->Control(pSampler->hCommPort, COMM_PURGE_TXCLEAR, NULL, 0);

    pSamp->bInited = TRUE;
    return TRUE;
}

/*=================================================================================================
** @Name      : Sample
** @Function  : sampler 框架周期调用入口 (500ms 周期)
**              仲裁 UART5 总线，水冷机/灯带分时复用
**===============================================================================================*/
DLLExport int Sample(HANDLE hSamplerThis)
{
    SAMPLER         *pSampler = (SAMPLER *)hSamplerThis;
    AUXDEV_SAMPLER  *pSamp;

    if(pSampler == NULL || pSampler->pDataHeap == NULL) return 0;
    pSamp = (AUXDEV_SAMPLER *)pSampler->pDataHeap;
    static int s_nTick = 0;
    if(!pSamp->bInited)
    {
        /* 端口未打开 (无硬件): 周期性重推故障状态,
         * 防止 DPR 数据库默认值 (COMMSTAT=0 "通信正常") 覆盖 Initialize 的推送 */
        if((++s_nTick % 20) == 1)   /* 每 ~10 秒重推一次 */
        {
            pushParamEnum(hSamplerThis, pSamp->chiller.nDevId, CHILLER_PARAM_COMMSTAT, 1);
            invalidateChillerParams(hSamplerThis, &pSamp->chiller);
            pushParamEnum(hSamplerThis, pSamp->led.nDevId, LED_PARAM_COMMSTAT, 1);
        }
        return 0;
    }

    pSamp->nCycleCnt++;


    /* 总线仲裁: 如果忙且未超时，跳过本轮 */
    if(pSamp->bUart5BusBusy)
    {
        if(getMonotonicMs() < pSamp->nUart5BusFreeTimeMs) return 0;
        pSamp->bUart5BusBusy = FALSE;
    }

    /* LED 自动判决: 每周期判决 (只读 DPR 不占总线), 颜色变化才标记 dirty */
    IMLU2_COLOR autoColor = ledDecide(pSampler);
    if(autoColor != pSamp->led.lastSentColor)
    {
        pSamp->led.pendingColor = autoColor;
        pSamp->led.bColorDirty  = TRUE;
    }

    /* 水冷机安装门控: 未安装时不轮询、不下发开关机, 仅保留灯带心跳 */
    BOOL bDoChiller = pSamp->chiller.bInstalled
                      && (pSamp->nCycleCnt % AUXDEV_CHILLER_POLL_EVERY_N_CYCLES) == 0;
    BOOL bDoPower   = pSamp->chiller.bInstalled
                      && (pSamp->nCycleCnt % 4) == 1;
    BOOL bDoLed     = (pSamp->nCycleCnt % 4) == 3;

    /* 水冷机: 每 N 个周期轮询一次 */
    if(bDoChiller)
    {
        BOOL bOk = pollChiller(pSamp, pSampler->pUsedCommPort, pSampler->hCommPort);
        if(!bOk)
        {
            pSamp->chiller.nCommFailCnt++;
            if(pSamp->chiller.nCommFailCnt >= AUXDEV_COMM_FAULT_LIMIT
               && pSamp->chiller.bCommOK)
            {
                pSamp->chiller.bCommOK = FALSE;
                TRACE("[auxdev] CHILLER COMM FAIL: nCommFailCnt=%d -> SetCommStatus(961, FALSE)\n",
                       pSamp->chiller.nCommFailCnt);
                SetCommStatus(hSamplerThis, pSamp->chiller.nDevId, FALSE);
                pushParamEnum(hSamplerThis, pSamp->chiller.nDevId, CHILLER_PARAM_COMMSTAT, 1);
                invalidateChillerParams(hSamplerThis, &pSamp->chiller);
            }
        }
    }
    /* 水冷机开关机: 每 4 个周期 (2秒) 发送 CMD 0x11 上层状态 */
    else if(bDoPower)
    {
        /* 自动模式: 读两簇(501/502) param 550 (Run Status) 决定开关机
         *   任一簇充电(4)/放电(5) -> 开机; 否则关机 */
        if(pSamp->chiller.ctrlMode == 0)  /* Auto */
        {
            DATA_PROCESSOR *pDPR = (DATA_PROCESSOR *)pSampler->hDPR;
            if(pDPR != NULL)
            {
                VAR_VALUE varV;
                BOOL bRun = FALSE;
                int  n, nDev;

                /* 两簇聚合: 任一簇充电(4)/放电(5) -> 开机 */
                for(n = 0; n < LED_DPR_BATT_GROUP_NUM; n++)
                {
                    nDev = LED_DPR_DEV_BATTGRP + n;   /* 501, 502 */
                    memset(&varV, 0, sizeof(varV));
                    pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, nDev, LED_DPR_PARAM_RUNSTATUS, &varV, sizeof(varV));
                    if(varV.emValue == 4 || varV.emValue == 5)
                    {
                        bRun = TRUE;
                        break;
                    }
                }

                u8 desired = bRun ? 1 : 0;
                if(pSamp->chiller.powerSwitch != desired)
                {
                    pSamp->chiller.powerSwitch = desired;
                }
            }
        }
        /* 手动模式: powerSwitch 由用户通过 param 2201 设置, Control() 已保存 */

        sendUpperStatus(pSamp, pSampler->pUsedCommPort, pSampler->hCommPort);
    }
    /* 灯带时间片 (cycle % 4 == 3): 颜色变化优先刷新; 否则发 41H 版本查询做在线心跳 */
    else if(bDoLed)
    {
        if(pSamp->led.bColorDirty)
        {
            refreshLed(pSamp, pSampler->pUsedCommPort, pSampler->hCommPort);
        }
        else
        {
            BOOL bOk = queryLedOnline(pSamp, pSampler->pUsedCommPort, pSampler->hCommPort);
            if(bOk)
            {
                pSamp->led.nCommFailCnt = 0;
                if(!pSamp->led.bCommOK)
                {
                    pSamp->led.bCommOK = TRUE;
                    SetCommStatus(hSamplerThis, pSamp->led.nDevId, TRUE);
                    pushParamEnum(hSamplerThis, pSamp->led.nDevId, LED_PARAM_COMMSTAT, 0);
                }
            }
            else
            {
                pSamp->led.nCommFailCnt++;
                if(pSamp->led.nCommFailCnt >= AUXDEV_COMM_FAULT_LIMIT && pSamp->led.bCommOK)
                {
                    pSamp->led.bCommOK = FALSE;
                    SetCommStatus(hSamplerThis, pSamp->led.nDevId, FALSE);
                    pushParamEnum(hSamplerThis, pSamp->led.nDevId, LED_PARAM_COMMSTAT, 1);
                }
            }
        }
    }

    return 0;
}

/*=================================================================================================
** @Name      : Control
** @Function  : sampler 框架控制入口。
**              当 SettingParameter(IsNeedSync_Sampler=1) 被修改时，框架调用此函数。
**              目前只处理水冷机安装类型/开关机/控制模式 (961)。
**===============================================================================================*/
DLLExport int Control(HANDLE hSamplerThis, PARAM_TYPE emParaType,
                      int nDevId, int nParamId, VAR_VALUE *pVarVal,
                      int nValSize, int nTimeOut)
{
    SAMPLER         *pSampler = (SAMPLER *)hSamplerThis;
    AUXDEV_SAMPLER  *pSamp;

    (void)emParaType; (void)nValSize; (void)nTimeOut;
    if(pSampler == NULL || pSampler->pDataHeap == NULL || pVarVal == NULL) return -1;
    pSamp = (AUXDEV_SAMPLER *)pSampler->pDataHeap;
    if(!pSamp->bInited) return -1;

    /* 水冷机安装类型 (param 2099): 更新安装门控并清残留通信告警, 返回 0 让 DPR 持久化。
     * sampler_system.c 的 Control(param 104) 通过 Push_VarValue 把
     * DEVINSTALLCONFIG->WaterChiller 分发到这里。返回 0 后，
     * sampler_loader 会用 SETTYPE_RAW_BACK_SETT 把值写回 DPR，
     * 进而触发 Dump_ParamValue + Save_SettingParam，
     * displayExpression '[$,2,2099]==1' 才能正确求值。 */
    if(nDevId == CHILLER_DEV_ID && nParamId == CHILLER_PARAM_INSTALL_TYPE)
    {
        BOOL bInstalled = (pVarVal->emValue != 0);
        pSamp->chiller.bInstalled   = bInstalled;
        pSamp->chiller.bCommOK      = bInstalled;
        pSamp->chiller.nCommFailCnt = 0;
        /* 清残留通信告警, 让状态机重新检测: 未安装则不轮询、不重触发;
         * 已安装则假定通信正常, 轮询失败后重新触发告警 (与 Initialize 一致) */
        SetCommStatus(hSamplerThis, pSamp->chiller.nDevId, TRUE);
        TRACE("[auxdev] CHILLER install type -> %d: SetCommStatus(961, TRUE)\n",
              (int)pVarVal->emValue);
        if(bInstalled)
        {
            pushParamEnum(hSamplerThis, pSamp->chiller.nDevId, CHILLER_PARAM_COMMSTAT, 0);
        }
        else
        {
            pushParamEnum(hSamplerThis, pSamp->chiller.nDevId, CHILLER_PARAM_COMMSTAT, 1);
            invalidateChillerParams(hSamplerThis, &pSamp->chiller);
        }
        return 0;
    }

    /* 水冷机开关机 (param 2201): 保存到运行时, Sample() 周期发 CMD 0x11 */
    if(nDevId == CHILLER_DEV_ID && nParamId == CHILLER_PARAM_POWER_SWITCH)
    {
        pSamp->chiller.powerSwitch = (u8)pVarVal->emValue;
        return 0;
    }

    /* 水冷机控制模式 (param 2202): 0=自动 1=手动 */
    if(nDevId == CHILLER_DEV_ID && nParamId == CHILLER_PARAM_CTRL_MODE)
    {
        pSamp->chiller.ctrlMode = (u8)pVarVal->emValue;
        return 0;
    }

    return -1;
}

/*=================================================================================================
** @Name      : JigTest
** @Function  : 生产测试接口 (空实现)
**===============================================================================================*/
DLLExport int JigTest(HANDLE hSamplerThis, void *pJigData)
{
    (void)hSamplerThis; (void)pJigData;
    return 0;
}
