/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : FireTactic.c
** @Founder      : NJX
** @Date         : 2026.08.18
** @Function     : 消防4级告警策略(方案C移植自100.37平台AuxDevTactic.c TacFireAlarmCtrlTask@1329-2916)
** @Instructions : 500ms任务节拍(与老平台一致);L1=Pack内单体高温+温升,L2=同芯持续高温/气溶胶+邻芯高温/
**                 邻芯温升,L3=可燃气体+L2,L4=灭火剂喷放后复合终判(接触器下电);
**                 含烟感二次上电去误报状态机;相对老平台修复4缺陷并数组化重写5×PACK复制粘贴;
**                 阈值/锁存已按BMU05消防策略技术方案V1.2校准(L3锁存,气体阈值H2/CO分列)
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "AuxDevMsg.h"
#include "PortCheck.h"
#include "FireTactic.h"
#include "DiagInterface.h"
#include "DiagFilter.h"
#include <stdio.h>
#include <stdlib.h>
#include "DiagCheck.h"
#include "CLI.h"
#include "TacUser.h"

extern u8 gGHardPara_104[];                                 /*主控硬件参数配置信息*/
extern u16 gGBmuHigLevPara_103[];                           /*主控高级参数配置信息*/

#if(FIRE_BENCH_INJECT_EN)
extern u8  *gImeu3BoardInfo1;                             /*IMEU3板级信息1(注入直写)*/
extern u16 *gImeu3BoardInfo2;                             /*IMEU3板级信息2(注入直写)*/
#endif

/*调试打印宏(测试观测用,FIRE_DEBUG_LOG_EN=0时编译期消除,量产前改回0)*/
#if(FIRE_DEBUG_LOG_EN)
#define FIRE_LOG(...)       do{ printf(__VA_ARGS__); }while(0)
#else
#define FIRE_LOG(...)       do{ }while(0)
#endif

/*=============================================================================================
** @Data type definition
**===========================================================================================*/
/*单个PACK消防运行状态(数组化重写老平台5×PACK复制粘贴段)*/
typedef struct
{
    u8 level1;                                              /*L1告警标志(一次恢复机会:温度回落自动清除一次,再触发即锁存)*/
    u8 level2;                                              /*L2告警标志(触发后锁存)*/
    u8 l1Pos;                                               /*L1触发位置(PACK内1基,老平台sFireLevel1Pos)*/
    u8 l2Pos;                                               /*L2触发位置(PACK内1基,老平台sFireLevel2Pos)*/
    u8 l1TimeCnt;                                           /*L1高温持续计数 500ms/tick*/
    u8 riseCellId;                                          /*温升窗口跟踪芯(PACK内0基,老缺陷④修复对象)*/
    s16 riseBaseTemp;                                       /*温升窗口基准温度 0.1℃*/
    u8 riseWinCnt;                                          /*温升窗口内计时 500ms/tick*/
    u8 rise5sCnt;                                           /*连续温升达标窗口数(老平台sRiseTime5s)*/
    u8 l2aTimeCnt;                                          /*L2a同芯高温持续计数*/
    u8 l2bTimeCnt;                                          /*L2b气溶胶+邻芯高温持续计数*/
    u8 l2cTimeCnt;                                          /*L2c邻芯温升持续计数*/
    s16 nbBaseTemp;                                         /*L2c邻芯温升窗口基准 0.1℃*/
    u8 nbWinCnt;                                            /*L2c邻芯温升窗口计时*/
    u8 nbRiseOk;                                            /*L2c邻芯本窗温升达标标志*/
    u8 riseInit;                                            /*冷启动温升窗首次基准初始化哨兵(修复:首发5s窗以0为基准虚窗)*/
    u8 l1RecCnt;                                           /*L1一次恢复回落确认计数 500ms/tick*/
    u8 l1RecUsed;                                          /*L1恢复机会已用标志(仅一次,用后L1再触发锁存)*/
}t_FirePackState;

/*单个簇消防运行状态(L3/L4为簇级/IMEU3全局输入)*/
typedef struct
{
    u8 level3;                                              /*L3告警标志(触发后锁存:方案V1.2第六节禁止自动恢复,气体回落/IMEU3断线不清除)*/
    u8 level4;                                              /*L4告警标志(触发后锁存)*/
    u8 l3TimeCnt;                                           /*L3持续计数*/
    u8 l4CntH2Cab;                                          /*L4条件②H2+柜温持续计数(老缺陷①修复:分支独立计数)*/
    u8 l4CntCoCab;                                          /*L4条件③CO+柜温持续计数*/
    u8 l4CntH2Smok;                                         /*L4条件④H2+烟感持续计数*/
    u8 l4CntCoSmok;                                         /*L4条件⑤CO+烟感持续计数*/
}t_FireCluState;

/*=============================================================================================
** @Static variable definition
**===========================================================================================*/
static t_FirePackState sFirePack[CLUTER_MAX_NUM][MAX_BSU_NUM];      /*PACK消防状态[簇][PACK]*/
static t_FireCluState sFireClu[CLUTER_MAX_NUM];                     /*簇级消防状态*/
static u8 sCtrlStep = 0;                                   /*输出分步控制(0:声光 1:灭火+下电,动作周期1s)*/
static u16 sAlarmTime = 0;                                 /*消防告警激活时间 500ms/tick(告警清除复位,修复老缺陷③)*/
static u8 sSmgOffStep = 0;                                 /*烟感电源核实步骤(移植GroupTactic.c:318-392休眠状态机)*/
static u8 sSmgOffFlag = 0;                                 /*烟感电源已断电核实标志*/
static u16 sSmgOffTime = 0;                                /*烟感核实计时 500ms/tick*/
static u8 sSmgAlarmState = eSmogVry_None;                  /*烟感核实结果e_SmogVryState*/
static u8 sSmgPrevState = eSmogVry_None;                     /*烟感状态变化打印跟踪(调试)*/
static u8 sFireBootLog = 0;                                  /*任务启动横幅一次性打印标记(调试)*/

#if(FIRE_BENCH_INJECT_EN)
static u8  sBenchCmd  = 0;                 /*台架注入目标等级[0,4]0停止*/
static s16 sBenchTemp = 250;                /*台架注入温度0.1℃(斜坡起点25.0℃)*/
static void FireBenchInjectTask(void);
#endif

/*=============================================================================================
** @Function implementation
**===========================================================================================*/
/*=============================================================================================
** @Name      : static void FireFindPackMaxTemp(u8 clunum, u16 start, u16 count, s16 *maxTemp, u8 *maxIdx)
** @Input     : clunum:簇号[0,CLUTER_MAX_NUM)  start:PACK温度起始组内索引  count:PACK温度点数
**               maxTemp:最高温度输出0.1℃  maxIdx:最高温PACK内序号输出(0基)
** @Output    : void
** @Function  : 查找PACK内最高单体温度及其序号(跳过无效哨兵值)
** @The notes : GetGCellTempAPI返回0.1℃(头注释1℃为文档错误),<=-410为断线/无效值
**===========================================================================================*/
static void FireFindPackMaxTemp(u8 clunum, u16 start, u16 count, s16 *maxTemp, u8 *maxIdx)
{
    u16 i;
    s16 temp;
    s16 maxT = FIRE_TEMP_INVALID;
    u8 maxI = 0;

    if((maxTemp == NULL) || (maxIdx == NULL))
    {
        return;
    }
    for(i = 0; i < count; i++)
    {
        temp = GetGCellTempAPI(clunum, start + i);
        if(temp > FIRE_TEMP_INVALID)
        {
            if(temp > maxT)
            {
                maxT = temp;
                maxI = (u8)i;
            }
        }
    }
    *maxTemp = maxT;
    *maxIdx = maxI;
}

/*=============================================================================================
** @Name      : static s16 FireCheckNeighborTemp(u8 clunum, u16 start, u8 trackIdx, u8 bsuTNum, u16 packCnt, s16 th)
** @Input     : clunum:簇号  start:PACK温度起始组内索引  trackIdx:触发芯PACK内序号0基
**               bsuTNum:单模组温度数  packCnt:PACK温度点数  th:邻芯温度阈值0.1℃
** @Output    : 返回有效邻芯最高温度0.1℃,无有效邻芯返回FIRE_TEMP_INVALID
** @Function  : 获取触发芯物理相邻温度点最高温(模组内相邻,老平台48芯=4模组×12的邻芯规则泛化)
** @The notes : 模组内首点仅看后邻,末点仅看前邻,中间点看双邻(对应老平台pos∈{1,13,25,37}/{12,24,36,48}规则)
**===========================================================================================*/
static s16 FireCheckNeighborTemp(u8 clunum, u16 start, u8 trackIdx, u8 bsuTNum, u16 packCnt, s16 th)
{
    u16 prevIdx;
    u16 nextIdx;
    s16 temp;
    s16 maxT = FIRE_TEMP_INVALID;

    if((bsuTNum == 0) || ((u16)trackIdx >= packCnt))
    {
        return(FIRE_TEMP_INVALID);
    }
    /*前邻:非模组内首点*/
    if((trackIdx % bsuTNum) != 0)
    {
        temp = GetGCellTempAPI(clunum, start + trackIdx - 1);
        if(temp > FIRE_TEMP_INVALID)
        {
            maxT = temp;
        }
    }
    /*后邻:非模组内末点*/
    if(((trackIdx % bsuTNum) != (bsuTNum - 1)) && (((u16)trackIdx + 1) < packCnt))
    {
        temp = GetGCellTempAPI(clunum, start + trackIdx + 1);
        if((temp > FIRE_TEMP_INVALID) && (temp > maxT))
        {
            maxT = temp;
        }
    }
    return(maxT);
}

/*=============================================================================================
** @Name      : static void FireSmogVerifyTask(void)
** @Input     : void
** @Output    : void
** @Function  : 烟感告警二次上电去误报状态机(控制IMEU3 PDUOUT DO3烟感电源)
** @The notes : 移植GroupTactic.c:318-392休眠状态机(原DO控制行全为注释);时序改为500ms/tick:
**               断电2s→重新上电→5s内再报警或15s窗内报警=确认,窗口满无报警=误报;
**               核实结果存sSmgAlarmState供L4条件④⑤使用
**===========================================================================================*/
static void FireSmogVerifyTask(void)
{
    u8 alarm;

    alarm = GetGSmogIoWarStateAPI();
    if(0 == sSmgOffStep)
    {
        /*发生烟感告警*/
        if(1 == alarm)
        {
            /*未复位过电源:断电核实*/
            if(0 == sSmgOffFlag)
            {
                sSmgAlarmState = eSmogVry_Checking;
                SendCanImeu3DoCtrlMsg(IMEU3_PDUIN_DEVADDR, 1, FIRE_DO_SMOK_PWR, 0);    /*断开烟感电源*/
                sSmgOffStep = 1;
                sSmgOffFlag = 1;
                sSmgOffTime = 0;
            }
            /*已复位过电源:直接确认*/
            else
            {
                sSmgAlarmState = eSmogVry_Confirmed;
            }
        }
        /*未发生烟感告警*/
        else
        {
            sSmgOffStep = 0;
            sSmgOffTime = 0;
            sSmgOffFlag = 0;
            sSmgAlarmState = eSmogVry_None;
            SendCanImeu3DoCtrlMsg(IMEU3_PDUIN_DEVADDR, 1, FIRE_DO_SMOK_PWR, 1);        /*保持闭合烟感电源*/
        }
    }
    /*断电保持,等待烟感完全下电*/
    else if(1 == sSmgOffStep)
    {
        SendCanImeu3DoCtrlMsg(IMEU3_PDUIN_DEVADDR, 1, FIRE_DO_SMOK_PWR, 0);            /*保持断开烟感电源*/
        if(sSmgOffTime < 60000)
        {
            sSmgOffTime++;
        }
        if(sSmgOffTime >= FIRE_SMOK_VRY_OFF_T)
        {
            sSmgOffStep = 2;
            sSmgOffTime = 0;
        }
    }
    /*重新上电,确认窗口*/
    else if(2 == sSmgOffStep)
    {
        SendCanImeu3DoCtrlMsg(IMEU3_PDUIN_DEVADDR, 1, FIRE_DO_SMOK_PWR, 1);            /*重新闭合烟感电源*/
        if(sSmgOffTime < 60000)
        {
            sSmgOffTime++;
        }
        /*二次上电后快速确认*/
        if((sSmgOffTime >= FIRE_SMOK_VRY_FAST_T) && (1 == alarm))
        {
            sSmgAlarmState = eSmogVry_Confirmed;
            sSmgOffStep = 0;
        }
        /*确认窗口满*/
        else if(sSmgOffTime >= FIRE_SMOK_VRY_WAIT_T)
        {
            if(1 == alarm)
            {
                sSmgAlarmState = eSmogVry_Confirmed;                                    /*确认有烟感告警,保持sSmgOffFlag=1防重复核实*/
            }
            else
            {
                sSmgAlarmState = eSmogVry_None;                                         /*二次上电后无告警,判定误报*/
                sSmgOffFlag = 0;                                                        /*误报清零,下次告警重新断电核实*/
            }
            sSmgOffStep = 0;
        }
    }
    else
    {
        sSmgOffStep = 0;
    }
    if(sSmgPrevState != sSmgAlarmState)
    {
        FIRE_LOG("[FIRE] smog: %d -> %d\r\n", sSmgPrevState, sSmgAlarmState);
        sSmgPrevState = sSmgAlarmState;
    }
}

/*=============================================================================================
** @Name      : static void FireChkPackLevel(u8 clunum, u8 packId, u16 start, u16 packCnt, u8 bsuTNum)
** @Input     : clunum:簇号  packId:PACK号  start:PACK温度起始组内索引  packCnt:PACK温度点数  bsuTNum:单模组温度数
** @Output    : void
** @Function  : 单PACK消防L1/L2告警判定(L1:最高温≥阈值持续+同芯温升;L2:同芯持续高温/气溶胶+邻芯高温/邻芯温升)
** @The notes : L1/L2触发后锁存(消防事件不自恢复);温升窗口跟踪芯切换时重置窗口(修复老平台缺陷④:
**               老平台最高温芯变化时冻结陈旧基准值)
**===========================================================================================*/
static void FireChkPackLevel(u8 clunum, u8 packId, u16 start, u16 packCnt, u8 bsuTNum)
{
    t_FirePackState *ps;
    s16 maxTemp;
    s16 trackTemp;
    s16 nbTemp;
    u16 aeroConc;
    u8 maxIdx;

    ps = &sFirePack[clunum][packId];
    FireFindPackMaxTemp(clunum, start, packCnt, &maxTemp, &maxIdx);
    if(maxTemp <= FIRE_TEMP_INVALID)
    {
        return;                                             /*PACK无有效温度数据*/
    }

    /*温升窗口机制(L1判定前持续运行,L2触发后停更)*/
    if((0 == ps->level1) || (0 == ps->level2))
    {
        /*跟踪芯切换或冷启动首窗:重置窗口基准(修复老平台缺陷④与冷启动虚窗)*/
        if((0 == ps->riseInit) || (ps->riseCellId != maxIdx))
        {
            ps->riseInit = 1;
            ps->riseCellId = maxIdx;
            ps->riseBaseTemp = maxTemp;
            ps->riseWinCnt = 0;
        }
        ps->riseWinCnt++;
        if(ps->riseWinCnt >= FIRE_RISE_WIN_T)
        {
            trackTemp = GetGCellTempAPI(clunum, start + ps->riseCellId);
            if((trackTemp > FIRE_TEMP_INVALID) && ((trackTemp - ps->riseBaseTemp) >= FIRE_L1_RISE_TH))
            {
                if(ps->rise5sCnt < 250)
                {
                    ps->rise5sCnt++;
                }
            }
            else
            {
                ps->rise5sCnt = 0;
            }
            ps->riseWinCnt = 0;
            ps->riseBaseTemp = trackTemp;
        }
    }

    /*L1一次恢复机会:触发后最高温回落持续10tick且机会未用则清除一次,再触发即锁存
    **(评审决策:L1去锁存改一次恢复,L2/L3/L4保持锁存;恢复后诊断57经BtoA滤波自动消警,禁充禁放随链解除)*/
    if((1 == ps->level1) && (0 == ps->l1RecUsed))
    {
        if(maxTemp < FIRE_L1_TEMP_TH)
        {
            if(ps->l1RecCnt < 250)
            {
                ps->l1RecCnt++;
            }
            if(ps->l1RecCnt >= FIRE_L1_TIME_TH)
            {
                ps->level1 = 0;
                ps->l1TimeCnt = 0;
                ps->l1RecCnt = 0;
                ps->l1RecUsed = 1;
                FIRE_LOG("[FIRE] L1 recover(one-shot used): clu%d pack%d maxT=%d\r\n", clunum, packId, maxTemp);
            }
        }
        else
        {
            ps->l1RecCnt = 0;
        }
    }

    /*L1判定:最高温≥阈值持续10tick且发生有效温升窗*/
    if(0 == ps->level1)
    {
        if(maxTemp >= FIRE_L1_TEMP_TH)
        {
            if(ps->l1TimeCnt < 250)
            {
                ps->l1TimeCnt++;
            }
        }
        else
        {
            ps->l1TimeCnt = 0;
        }
        if((ps->l1TimeCnt >= FIRE_L1_TIME_TH) && (ps->rise5sCnt >= 1))
        {
            ps->level1 = 1;
            ps->l1Pos = maxIdx + 1;
            ps->l2Pos = maxIdx + 1;
            FIRE_LOG("[FIRE] L1 trig: clu%d pack%d pos%d maxT=%d cnt=%d\r\n", clunum, packId, ps->l1Pos, maxTemp, ps->l1TimeCnt);
        }
    }
    /*L2判定:仅L1已触发时跟随L1位置*/
    else if(0 == ps->level2)
    {
        trackTemp = GetGCellTempAPI(clunum, start + ps->l1Pos - 1);

        /*条件a:同芯≥阈值持续且连续温升达标窗口数足够*/
        if(trackTemp >= FIRE_L2A_TEMP_TH)
        {
            if(ps->l2aTimeCnt < 250)
            {
                ps->l2aTimeCnt++;
            }
        }
        else
        {
            ps->l2aTimeCnt = 0;
        }

        /*条件b:气溶胶≥阈值且邻芯≥阈值持续*/
        nbTemp = FireCheckNeighborTemp(clunum, start, (u8)(ps->l1Pos - 1), bsuTNum, packCnt, FIRE_L2B_NB_TEMP_TH);
        aeroConc = 0;
        if(gPackStat_95 != NULL)
        {
            aeroConc = gPackStat_95[clunum][packId][ePackStat95_AeroConc];
        }
        if((aeroConc >= FIRE_L2B_AERO_TH) && (nbTemp >= FIRE_L2B_NB_TEMP_TH))
        {
            if(ps->l2bTimeCnt < 250)
            {
                ps->l2bTimeCnt++;
            }
        }
        else
        {
            ps->l2bTimeCnt = 0;
        }

        /*条件c:邻芯温升≥阈值持续(10s窗口基准)*/
        if(nbTemp > FIRE_TEMP_INVALID)
        {
            if(0 == ps->nbWinCnt)
            {
                ps->nbBaseTemp = nbTemp;
            }
            ps->nbWinCnt++;
            if(ps->nbWinCnt >= FIRE_L2C_TIME_TH)
            {
                ps->nbRiseOk = ((nbTemp - ps->nbBaseTemp) >= FIRE_L2C_RISE_TH) ? 1 : 0;
                ps->nbWinCnt = 0;
                ps->nbBaseTemp = nbTemp;
            }
        }
        else
        {
            ps->nbWinCnt = 0;
            ps->nbRiseOk = 0;
        }
        if(1 == ps->nbRiseOk)
        {
            if(ps->l2cTimeCnt < 250)
            {
                ps->l2cTimeCnt++;
            }
        }
        else
        {
            ps->l2cTimeCnt = 0;
        }

        /*三条件任一达标触发L2*/
        if(((ps->l2aTimeCnt >= FIRE_L2A_TIME_TH) && (ps->rise5sCnt >= FIRE_L2A_RISE_CNT))
            || (ps->l2bTimeCnt >= FIRE_L2B_TIME_TH)
            || (ps->l2cTimeCnt >= FIRE_L2C_TIME_TH))
        {
            ps->level2 = 1;
            ps->l2Pos = ps->l1Pos;
            FIRE_LOG("[FIRE] L2 trig: clu%d pack%d pos%d a=%d/%d b=%d/%d c=%d/%d\r\n", clunum, packId, ps->l2Pos, ps->l2aTimeCnt, FIRE_L2A_TIME_TH, ps->l2bTimeCnt, FIRE_L2B_TIME_TH, ps->l2cTimeCnt, FIRE_L2C_TIME_TH);
        }
    }
}

/*=============================================================================================
** @Name      : static void FireChkCluLevel(u8 clunum)
** @Input     : clunum:簇号[0,CLUTER_MAX_NUM)
** @Output    : void
** @Function  : 簇级L3/L4告警判定
** @The notes : L3=(H2≥阈值||CO≥阈值)&&L2持续10tick,触发后锁存(方案V1.2第六节三级禁止自动恢复,
**               气体回落/IMEU3断线均不清除);
**               L4=灭火剂喷放反馈(DI5)有效时五路并联条件各持续20tick(老平台缺陷①修复:各分支独立计数),
**               条件①烟感故障自检默认禁用(待H4确认gImeu3AlmInfo bit映射)
**===========================================================================================*/
static void FireChkCluLevel(u8 clunum)
{
    t_FireCluState *cs;
    u16 h2Raw;
    u16 coRaw;
    u16 diState;
    u8 perfluState;
    u8 cabTempOk;
    u8 smokOk;
    u8 level2Any;
    u8 i;
    s16 cabTemp;                              /*51表℃+50偏移码,2026-09-14解码修正*/
    s16 cabTempMax;

    cs = &sFireClu[clunum];
    h2Raw = GetImeu3BoardInfo2API(eIMEUBoardInfo2_H2Raw);
    coRaw = GetImeu3BoardInfo2API(eIMEUBoardInfo2_CORaw);
    diState = GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI1_16);
    perfluState = (u8)BitGet(diState, 4);                    /*DI5气溶胶喷放反馈*/
    level2Any = 0;
    for(i = 0; i < MAX_BSU_NUM; i++)
    {
        if(1 == sFirePack[clunum][i].level2)
        {
            level2Any = 1;
            break;
        }
    }

    /*L3判定*/
    if((1 == level2Any) && ((h2Raw >= FIRE_L3_H2_TH) || (coRaw >= FIRE_L3_CO_TH)))
    {
        if(cs->l3TimeCnt < 250)
        {
            cs->l3TimeCnt++;
        }
    }
    else
    {
        cs->l3TimeCnt = 0;
    }
    if((0 == cs->level3) && (cs->l3TimeCnt >= FIRE_L3_TIME_TH))
    {
        cs->level3 = 1;                                      /*L3锁存:方案V1.2第六节三级禁止自动恢复,气体回落/断线不清除;已知差距:RAM锁存重启即清零,掉电存储未实现*/
        FIRE_LOG("[FIRE] L3 trig: clu%d h2=%d co=%d\r\n", clunum, h2Raw, coRaw);
    }

    /*L4判定:喷放反馈有效后五路并联(条件①默认禁用)*/
    if(0 == cs->level4)
    {
        cabTempMax = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_SenTem) - 50;      /*51表全域+50偏移已确认(CANFD协议表+IMEU3/100.37双侧源码佐证)*/
        cabTemp = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_485Tem1) - 50;
        if(cabTemp > cabTempMax)
        {
            cabTempMax = cabTemp;
        }
        cabTemp = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_485Tem2) - 50;
        if(cabTemp > cabTempMax)
        {
            cabTempMax = cabTemp;
        }
        cabTempOk = (cabTempMax >= FIRE_L4_CABTEMP_TH) ? 1 : 0;
        smokOk = (eSmogVry_Confirmed == sSmgAlarmState) ? 1 : 0;

        if(1 == perfluState)
        {
            /*条件②H2+柜温*/
            if((h2Raw >= FIRE_L4_H2_TH) && (1 == cabTempOk))
            {
                if(cs->l4CntH2Cab < 250)
                {
                    cs->l4CntH2Cab++;
                }
            }
            else
            {
                cs->l4CntH2Cab = 0;
            }
            /*条件③CO+柜温*/
            if((coRaw >= FIRE_L4_CO_TH) && (1 == cabTempOk))
            {
                if(cs->l4CntCoCab < 250)
                {
                    cs->l4CntCoCab++;
                }
            }
            else
            {
                cs->l4CntCoCab = 0;
            }
            /*条件④H2+烟感*/
            if((h2Raw >= FIRE_L4_H2_TH) && (1 == smokOk))
            {
                if(cs->l4CntH2Smok < 250)
                {
                    cs->l4CntH2Smok++;
                }
            }
            else
            {
                cs->l4CntH2Smok = 0;
            }
            /*条件⑤CO+烟感*/
            if((coRaw >= FIRE_L4_CO_TH) && (1 == smokOk))
            {
                if(cs->l4CntCoSmok < 250)
                {
                    cs->l4CntCoSmok++;
                }
            }
            else
            {
                cs->l4CntCoSmok = 0;
            }
            /*条件①烟感故障+柜温:待H4确认gImeu3AlmInfo bit映射后启用(老平台烟感故障位分支)*/
#if(FIRE_SMOK_FAULT_EN)
#endif
            if((cs->l4CntH2Cab >= FIRE_L4_TIME_TH) || (cs->l4CntCoCab >= FIRE_L4_TIME_TH)
                || (cs->l4CntH2Smok >= FIRE_L4_TIME_TH) || (cs->l4CntCoSmok >= FIRE_L4_TIME_TH))
            {
                cs->level4 = 1;
                FIRE_LOG("[FIRE] L4 trig: clu%d h2=%d co=%d cab=%d smok=%d cnt=%d/%d/%d/%d\r\n", clunum, h2Raw, coRaw, cabTempMax, smokOk, cs->l4CntH2Cab, cs->l4CntCoCab, cs->l4CntH2Smok, cs->l4CntCoSmok);
            }
        }
        else
        {
            cs->l4CntH2Cab = 0;
            cs->l4CntCoCab = 0;
            cs->l4CntH2Smok = 0;
            cs->l4CntCoSmok = 0;
        }
    }
}

/*=============================================================================================
** @Name      : static u8 FireIsAlarmActive(void)
** @Input     : void
** @Output    : 0:无 1:任一簇L1或L2告警激活
** @Function  : 查询消防声光告警激活状态
** @The notes : void
**===========================================================================================*/
static u8 FireIsAlarmActive(void)
{
    u8 clunum;
    u8 packId;

    for(clunum = 0; clunum < CLUTER_MAX_NUM; clunum++)
    {
        for(packId = 0; packId < MAX_BSU_NUM; packId++)
        {
            if((1 == sFirePack[clunum][packId].level1) || (1 == sFirePack[clunum][packId].level2))
            {
                return(1);
            }
        }
    }
    return(0);
}

/*=============================================================================================
** @Name      : static u8 FireIsPerfluAllow(void)
** @Input     : void
** @Output    : 0:禁止 1:允许灭火触发
** @Function  : 查询灭火触发条件(仅三级L3,方案V1.2动作矩阵:二级灭火需'项目消防矩阵批准',836不批准)
** @The notes : L3=温度+气体双证据,允许禁充禁放+声光+灭火+泄爆;L2(含L2b气溶胶+邻芯)仅升二级告警,
**               不上灭火;灭火喷放后DI5反馈有效才进L4自杀下电
**===========================================================================================*/
static u8 FireIsPerfluAllow(void)
{
    u8 clunum;

    for(clunum = 0; clunum < CLUTER_MAX_NUM; clunum++)
    {
        if(1 == sFireClu[clunum].level3)
        {
            return(1);
        }
    }
    return(0);
}

/*=============================================================================================
** @Name      : static u8 FireIsLevel4Active(void)
** @Input     : void
** @Output    : 0:无 1:任一簇L4告警激活
** @Function  : 查询接触器下电条件
** @The notes : void
**===========================================================================================*/
static u8 FireIsLevel4Active(void)
{
    u8 clunum;

    for(clunum = 0; clunum < CLUTER_MAX_NUM; clunum++)
    {
        if(1 == sFireClu[clunum].level4)
        {
            return(1);
        }
    }
    return(0);
}

/*=============================================================================================
** @Name      : static void FireOutputCtrlTask(void)
** @Input     : void
** @Output    : void
** @Function  : 消防输出分步控制(sCtrlStep交替,动作周期1s,与老平台一致)
** @The notes : 调试模式整体让位A核ICC直控(声光DO2/mIndex=71,与老平台手控路径一致);偶数步:声光DO2
**               (L1||L2激活,告警清除自动关闭);奇数步:灭火触发DO1(仅L3,836图主动消防预留位,受
**               FIRE_PERFLU_EN门控,喷放状态由DI5反馈)+L4自杀下电(断PK1-4簇接触器+DO5=0断QFP5
**               直流辅源/EPO双路,老平台QF1分闸的836等价实现;一次性动作,DO5保活在AuxDevTactic
**               按L4闭锁防回拉)
**===========================================================================================*/
static void FireOutputCtrlTask(void)
{
    /*调试模式跳过自动输出,由A核通过ICC直控*/
    if(eMODE_DEBUG == GetGWorkModeAPI(0))
    {
        return;
    }

    if(0 == (sCtrlStep & 0x01))
    {
        /*声光报警器控制*/
        if(1 == FireIsAlarmActive())
        {
            SendCanImeu3DoCtrlMsg(IMEU3_PDUIN_DEVADDR, 1, FIRE_DO_ZHS, 1);
            if(sAlarmTime < 60000)
            {
                sAlarmTime++;                               /*告警激活计时,清除时复位(修复老平台缺陷③)*/
            }
        }
        else
        {
            SendCanImeu3DoCtrlMsg(IMEU3_PDUIN_DEVADDR, 1, FIRE_DO_ZHS, 0);
            sAlarmTime = 0;
        }
    }
    else
    {
        /*灭火触发控制(836图DO1主动消防预留位,硬件确认FFD实装前由FIRE_PERFLU_EN闭锁)*/
        if((1 == FIRE_PERFLU_EN) && (1 == FireIsPerfluAllow()))
        {
            SendCanImeu3DoCtrlMsg(IMEU3_PDUIN_DEVADDR, 1, FIRE_DO_AEROSOL, 1);
        }
        /*L4自杀下电:断四路簇接触器+DO5=0(远程急停:KA1/KA3失电断主接触器+广播EPO)
        **+DO4=0(QFP5分闸断直流辅源,预留方案2;一次性不恢复,闭锁在AuxDevTactic防回拉)*/
        if(1 == FireIsLevel4Active())
        {
            TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, ePSTATE_OFF);
            TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, ePSTATE_OFF);
            TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, ePSTATE_OFF);
            TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, ePSTATE_OFF);
            SendCanImeu3DoCtrlMsg(IMEU3_PDUIN_DEVADDR, 1, FIRE_DO_EPO, 0);
            SendCanImeu3DoCtrlMsg(IMEU3_PDUIN_DEVADDR, 1, FIRE_DO_QFP5, 0);
        }
    }
    sCtrlStep++;
}

/*=============================================================================================
** @Name      : static void FireReportDiagTask(void)
** @Input     : void
** @Output    : void
** @Function  : 消防告警诊断位上核间上报(阶段2)
**              映射:L1→eERR_ID_FIRE_EWAR(57告警信息4bit9消防一级告警) L2→eERR_ID_FIRE1_WAR(58bit10消防二级告警)
**              L3→eERR_ID_FIRE3_WAR(59bit11消防三级告警,协议预留位) L4→eERR_ID_FIRE4_WAR(60bit12消防四级告警,协议预留位)
**              烟感确认→eERR_ID_BATT_SMOG(197电池组烟雾报警)
**              经Diag滤波后写入gAlmFault1_57/gAlmFault2_58,ICC主索引57/58周期1s自动上传A核
** @The notes : L1/L2为PACK级,任一PACK触发即上簇级告警;烟感为IMEU3全局输入,上报所有使能簇
**===========================================================================================*/
static void FireReportDiagTask(void)
{
    u8 clunum;
    u8 packId;
    u8 clusterNum;
    u8 l1Any;
    u8 l2Any;

    clusterNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum];
    if(clusterNum > CLUTER_MAX_NUM)
    {
        clusterNum = CLUTER_MAX_NUM;
    }

    for(clunum = 0; clunum < clusterNum; clunum++)
    {
        l1Any = 0;
        l2Any = 0;
        for(packId = 0; packId < MAX_BSU_NUM; packId++)
        {
            if(1 == sFirePack[clunum][packId].level1)
            {
                l1Any = 1;
            }
            if(1 == sFirePack[clunum][packId].level2)
            {
                l2Any = 1;
            }
        }

        DiagSetErrNowStateAPI(clunum, eERR_ID_FIRE_EWAR, (1 == l1Any) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_FIRE1_WAR, (1 == l2Any) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_FIRE3_WAR, (1 == sFireClu[clunum].level3) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_FIRE4_WAR, (1 == sFireClu[clunum].level4) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_BATT_SMOG, (eSmogVry_Confirmed == sSmgAlarmState) ? eErr_B : eErr_A);
    }
}

/*=============================================================================================
** @Name      : void TacFireAlarmCtrlTask(void)
** @Input     : void
** @Output    : void
** @Function  : 消防4级告警控制任务(500ms任务节拍驱动,注册于OSUserHook.c Task500ms)
** @The notes : 使能门控gGHardPara_104[eGHardPara104_Imeu3En]&0x02(bit1=PDU外部IMEU3,与老平台一致);
**               PACK遍历范围由gGBmuHigLevPara_103[BsuNum/ConNum/BsuTNum/TSumNum]动态决定
**===========================================================================================*/
void TacFireAlarmCtrlTask(void)
{
    u8 clunum;
    u8 packId;
    u8 bsuNum;
    u8 conNum;
    u8 bsuTNum;
    u16 tSumNum;
    u16 perPackCnt;
    u16 start;

    if(0 == sFireBootLog)
    {
        sFireBootLog = 1;
        FIRE_LOG("[FIRE] boot: TACTIC_EN=%d Imeu3En=%d PROFILE=%d\r\n", FIRE_TACTIC_EN, (gGHardPara_104[eGHardPara104_Imeu3En] & 0x02) ? 1 : 0, FIRE_DEBUG_PROFILE);
    }
    #if(FIRE_BENCH_INJECT_EN)
    /*台架数据注入(须在使能门控前: 首拍置位Imeu3En位后当拍即可通过门控)*/
    FireBenchInjectTask();
    #endif
    if(0 == FIRE_TACTIC_EN)
    {
        return;
    }
    /*PDU外部IMEU3未使能*/
    if((gGHardPara_104[eGHardPara104_Imeu3En] & 0x02) == 0)
    {
        return;
    }

    /*烟感去误报核实*/
    FireSmogVerifyTask();

    /*PACK参数解析与合法性约束*/
    bsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
    conNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];
    bsuTNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum];
    tSumNum = gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum];
    if((0 == bsuNum) || (0 == conNum) || (0 == bsuTNum) || (0 == tSumNum))
    {
        FireOutputCtrlTask();
        return;
    }
    if(bsuNum > MAX_BSU_NUM)
    {
        bsuNum = MAX_BSU_NUM;
    }
    perPackCnt = (u16)conNum * (u16)bsuTNum;
    if(tSumNum < (u16)bsuNum * perPackCnt)
    {
        bsuNum = (u8)(tSumNum / perPackCnt);
        if(0 == bsuNum)
        {
            FireOutputCtrlTask();
            return;
        }
    }

    /*逐簇逐PACK判定L1/L2*/
    for(clunum = 0; clunum < CLUTER_MAX_NUM; clunum++)
    {
        if(clunum >= (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum])
        {
            break;
        }
        for(packId = 0; packId < bsuNum; packId++)
        {
            start = (u16)packId * perPackCnt;
            FireChkPackLevel(clunum, packId, start, perPackCnt, bsuTNum);
        }
        FireChkCluLevel(clunum);
    }

    /*消防告警诊断位上核间上报*/
    FireReportDiagTask();

    /*输出分步控制*/
    FireOutputCtrlTask();
}

/*=============================================================================================
** @Name      : u8 TacGetFireLevelAPI(u8 clunum)
** @Input     : clunum:簇号[0,CLUTER_MAX_NUM)
** @Output    : 当前簇最高消防告警等级e_FireLevel
** @Function  : 获取簇消防告警等级(阶段2核间告警位上报接口)
** @The notes : void
**===========================================================================================*/
u8 TacGetFireLevelAPI(u8 clunum)
{
    t_FireCluState *cs;
    u8 level;
    u8 packId;

    if(clunum >= CLUTER_MAX_NUM)
    {
        return((u8)eFireLevel_None);
    }
    cs = &sFireClu[clunum];
    level = (u8)eFireLevel_None;
    for(packId = 0; packId < MAX_BSU_NUM; packId++)
    {
        if(1 == sFirePack[clunum][packId].level2)
        {
            level = (u8)eFireLevel_2;
            break;
        }
        if(1 == sFirePack[clunum][packId].level1)
        {
            level = (u8)eFireLevel_1;
        }
    }
    if((1 == cs->level3) && (level < (u8)eFireLevel_3))
    {
        level = (u8)eFireLevel_3;
    }
    if(1 == cs->level4)
    {
        level = (u8)eFireLevel_4;
    }
    return(level);
}

/*=============================================================================================
** @Name      : u8 TacGetFireLevelPosAPI(u8 clunum)
** @Input     : clunum:簇号[0,CLUTER_MAX_NUM)
** @Output    : 首个L1/L2触发PACK内位置(1基),无告警返回0
** @Function  : 获取簇消防告警触发位置(阶段2核间告警位上报接口)
** @The notes : 返回PACK内温度点序号,全局组内索引=位置-1+packId*perPackCnt(消费方自行换算)
**===========================================================================================*/
u8 TacGetFireLevelPosAPI(u8 clunum)
{
    u8 packId;

    if(clunum >= CLUTER_MAX_NUM)
    {
        return(0);
    }
    for(packId = 0; packId < MAX_BSU_NUM; packId++)
    {
        if(1 == sFirePack[clunum][packId].level2)
        {
            return(sFirePack[clunum][packId].l2Pos);
        }
    }
    for(packId = 0; packId < MAX_BSU_NUM; packId++)
    {
        if(1 == sFirePack[clunum][packId].level1)
        {
            return(sFirePack[clunum][packId].l1Pos);
        }
    }
    return(0);
}

/*=============================================================================================
** @Name      : u8 TacGetFireSmogVryStateAPI(void)
** @Input     : void
** @Output    : 烟感核实结果e_SmogVryState
** @Function  : 获取烟感去误报核实状态
** @The notes : void
**===========================================================================================*/
u8 TacGetFireSmogVryStateAPI(void)
{
    return(sSmgAlarmState);
}

/*=============================================================================================
** @Name      : static void FireBenchInjectTask(void)
** @Input     : void
** @Output    : void
** @Function  : 台架消防数据注入(无传感器桌面验证)
** @The notes : 生产阈值下走真实判定链: 温度斜坡+8(0.8℃)/tick注入簇0全部单体,
**              L1/L2靠斜坡自然先后触发; L3注入H2=3000, L4注入DI5喷放反馈+柜温75℃+烟感DI(bit3);
**              每拍保持BSU/IMEU3通讯保活(DiagRcvDevMsgAddAPI)防DiagCheck断线清零,
**              Imeu3En/SmogPort仅改RAM不落EEPROM, 停止注入后由断线清零逻辑自然回落
**===========================================================================================*/
#if(FIRE_BENCH_INJECT_EN)
static void FireBenchInjectTask(void)
{
    u8  ecu;
    u8  num;
    u8  i;
    u8  tPerEcu;
    s16 target;

    if(0 == sBenchCmd)
    {
        return;
    }

    /*RAM置位使能(不动EEPROM): bit0=IMEU3通讯检测 bit1=消防任务门控*/
    gGHardPara_104[eGHardPara104_Imeu3En] |= 0x03;
    /*烟感端口检测参数默认8=常开(DI bit3高有效), 为0时RAM置默认*/
    if(0 == gGBmuHigLevPara_103[eBmuHigLevPara103_SmogPort])
    {
        gGBmuHigLevPara_103[eBmuHigLevPara103_SmogPort] = 8;
    }

    /*通讯保活: 内部BSU从控逐个 + PDU内部IMEU3(30), 防断线清零逻辑生效*/
    for(i = 0; i < (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]; i++)
    {
        DiagRcvDevMsgAddAPI(i);
    }
    DiagRcvDevMsgAddAPI(IMEU3_PDUIN_DEVADDR);

    /*温度斜坡: +0.8℃/tick, 目标L1=85℃ L2及以上=95℃(L1/L2先后自然触发)*/
    target = (sBenchCmd >= 2) ? 950 : 850;
    if(sBenchTemp < target)
    {
        sBenchTemp += 8;
        if(sBenchTemp > target)
        {
            sBenchTemp = target;
        }
    }

    /*簇0全部单体温度注入(经InputBsuCellTemp边界检查写gGCellTempInfo_1)*/
    tPerEcu = (u8)(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]);
    for(ecu = 0; ecu < (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]; ecu++)
    {
        for(num = 0; num < tPerEcu; num++)
        {
            InputBsuCellTemp(0, ecu, num, sBenchTemp);
        }
    }

    /*IMEU3字段直写(保活防清零, 数组GroupInfoInit已分配)*/
    if(gImeu3BoardInfo2 != NULL)
    {
        gImeu3BoardInfo2[eIMEUBoardInfo2_H2Raw] = (sBenchCmd >= 3) ? 1000 : 0;         /*L3可燃气体H2≥800ppm原码*/
        gImeu3BoardInfo2[eIMEUBoardInfo2_CORaw] = 0;
        gImeu3BoardInfo2[eIMEUBoardInfo2_DI1_16] = (sBenchCmd >= 4) ? 0x0018 : 0x0000; /*bit3烟感+bit4喷放反馈*/
    }
    if(gImeu3BoardInfo1 != NULL)
    {
        gImeu3BoardInfo1[eIMEUBoardInfo1_SenTem] = (sBenchCmd >= 4) ? 125 : 75;         /*L4柜温≥70℃(℃+50偏移码:125=75℃,75=25℃)*/
    }
}

/*=============================================================================================
** @Name      : static int fire_inject_main(int argc, char *argv[])
** @Input     : argc/argv: CLI参数
** @Output    : CLI结果
** @Function  : 台架消防数据注入命令
** @The notes : fire_inject <0-4> 0停止 1:L1 2:L2 3:+H2 4:+DI5/柜温/烟感
**===========================================================================================*/
static int fire_inject_main(int argc, char *argv[])
{
    u8 level;

    if(argc == 0)
    {
        printf("用法: fire_inject <0-4>\r\n  0:停止注入  1:L1(85℃)  2:L2(95℃)  3:+H2  4:+DI5/柜温/烟感\r\n");
        printf("当前: cmd=%d temp=%d.%d℃\r\n", sBenchCmd, sBenchTemp / 10, sBenchTemp % 10);
        return 0;
    }
    level = (u8)strtoul(argv[0], NULL, 0);
    if(level > 4)
    {
        printf("错误: 等级范围[0,4]\r\n");
        return CLI_FALSE;
    }
    sBenchCmd = level;
    if(0 == level)
    {
        printf("注入停止(数据回落由通讯断线清零逻辑自然处理)\r\n");
    }
    else
    {
        printf("注入等级=%d (从当前温度继续斜坡)\r\n", level);
    }
    return 0;
}
CLI_CMD("fire_inject",
        "fire_inject"
        "\r\n\t fire_inject <0-4> 台架消防数据注入",
        fire_inject_main);
#endif

