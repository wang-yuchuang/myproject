/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DodRcdStatis.c
** @Founder      : ZQD808
** @Date         : 2026.03.10
** @Function     : 放电深度DOD记录统计模块
** @Instructions : 统计DOD次数(放电转充电新增一次)、Now/First/Max/Avg深度值、历史记录
**                V3:环形缓冲(每条记录10字与协议mIndex15/16同构,写一条仅20字节,消除旧版移位丢条问题)
**                支持双簇、掉电不丢失、可清零
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "ParaRead.h"
#include "ParaInterface.h"
#include "StaInterface.h"
#include "ParaWrite.h"
#include "DodRcdStatis.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
#include "IccMsgUser.h"      /*ICC_DT_U16*/
#include "IccMsgTabOper.h"  /*ICCReqIdleTxRcvTab UPM推送*/

/*==============================================================================
** SIMULATION: 假 DOD 记录推送开关(仅用于板端验证 A核 DOD 链路)
** 1=启用:每10s经 ICCReqIdleTxRcvTab 构造假 rec[10] 推送 A核(mIndex15/16),
**   绕开真实充放电触发与 EEPROM 写入; 0=关闭(量产/正常逻辑)
**============================================================================*/
#define DOD_RPMSG_SIM   0

extern u16 gGBmuGenPara_102[];                          /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];                       /*主控高级参数值*/
extern u16 *gGStatistDodInfo_62;                        /*放电深度统计信息*/
extern u16 *gGHisDodRing;                            /*历史DOD记录环形缓冲(内部,协议出口mIndex15/16)*/
extern u16 *gGHisDodInfo1_15;                       /*组1历史DOD记录上传快照(最新一条,ICC mIndex15推送/查询出口)*/
extern u16 *gGHisDodInfo2_16;                       /*组2历史DOD记录上传快照(最新一条,ICC mIndex16推送/查询出口)*/
extern u8 *txData;                                     /*ICC发送缓冲区(ICC框架初始化时分配)*/

/*=============================================================================================
** @Static variable definition
**===========================================================================================*/
/*双簇状态变量(按簇拆分防止串扰)*/
static u8  sRcdDod[CLUTER_MAX_NUM]     = {0};           /*标志:本次已放电,等待充电时记录一次DOD*/
static u16 sCTimer[CLUTER_MAX_NUM]     = {0};           /*充电持续计时器(单位1s)*/
static u16 sDTimer[CLUTER_MAX_NUM]     = {0};           /*放电持续计时器(单位1s)*/
static u16 sHisSoc[CLUTER_MAX_NUM]     = {0};           /*历史SOC值*/
static u16 sEepDod[CLUTER_MAX_NUM]     = {0xffff, 0xffff}; /*上次写入EEPROM的DOD值*/
static u32 sHisChgCap[CLUTER_MAX_NUM]  = {0};           /*上次记录时累计充电量*/
static u32 sHisDhgCap[CLUTER_MAX_NUM]  = {0};           /*上次记录时累计放电量*/
static u32 sHisChgEner[CLUTER_MAX_NUM] = {0};           /*上次记录时累计充电能*/
static u32 sHisDhgEner[CLUTER_MAX_NUM] = {0};           /*上次记录时累计放电能*/

#if DOD_RPMSG_SIM
static u16 sDodSimSeqId[CLUTER_MAX_NUM] = {0};  /*模拟记录序号(按簇自增)*/
static u16 sDodSimTimer = 0;                    /*模拟节拍(1s计数,每10拍推一条)*/
#endif

/*=============================================================================================
** @Static function declaration
**===========================================================================================*/
/*=================================================================================================
** @Name      : static void GroupDodStatisRcdDeal(u8 clunum, u16 *newDodInfo, u8 infoNum)
** @Input     : clunum:簇编号 newDodInfo:新记录DOD信息(DOD值/充电量/放电量/充电能/放电能) infoNum:信息个数(5个有效)
** @Output    : void
** @Function  : 环形写入一条新DOD记录(自带时间戳),次数+1,重算Max/Avg
** @The notes : DOD变化需要记录时调用;先写记录再写头Id(掉电一致性)
**===============================================================================================*/
static void GroupDodStatisRcdDeal(u8 clunum, u16 *newDodInfo, u8 infoNum);
#if DOD_RPMSG_SIM
static void GroupDodStatisRcdSimPush(u8 clunum);
#endif

/*=============================================================================================
** @Interface function definition
**===========================================================================================*/
/*=================================================================================================
** @Name      : void GroupDodStatisRcdInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组放电深度DOD统计记录初始化
** @The notes : 读取EEPROM中DOD统计值和历史DOD记录V2(环形缓冲)
**===============================================================================================*/
void GroupDodStatisRcdInit(void)
{
    u16 info16[5] = {0};
    u16 base = 0;
    u16 k = 0;

    if(gGStatistDodInfo_62 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistDodInfo_62 null!!!\r\n", __func__);
        return;
    }
    if(gGHisDodRing == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGHisDodRing null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        /*读取放电深度DOD统计*/
        if(TRUE == ParaReadStoreDODStaInfo(clunum, info16, 5))
        {
            if(clunum == 0)
            {
                gGStatistDodInfo_62[eDodStat62_1NowDod] = info16[1];
                gGStatistDodInfo_62[eDodStat62_1FstDod] = info16[2];
                gGStatistDodInfo_62[eDodStat62_1MaxDod] = info16[3];
                gGStatistDodInfo_62[eDodStat62_1AvgDod] = info16[4];
            }
            else
            {
                gGStatistDodInfo_62[eDodStat62_2NowDod] = info16[1];
                gGStatistDodInfo_62[eDodStat62_2FstDod] = info16[2];
                gGStatistDodInfo_62[eDodStat62_2MaxDod] = info16[3];
                gGStatistDodInfo_62[eDodStat62_2AvgDod] = info16[4];
            }
        }
        else
        {
            if(clunum == 0)
            {
                gGStatistDodInfo_62[eDodStat62_1NowDod] = 1000 - gGBmuGenPara_102[eBmuGenPara102_1NowSoc];
                gGStatistDodInfo_62[eDodStat62_1FstDod] = gGStatistDodInfo_62[eDodStat62_1NowDod];
                gGStatistDodInfo_62[eDodStat62_1MaxDod] = gGStatistDodInfo_62[eDodStat62_1NowDod];
                gGStatistDodInfo_62[eDodStat62_1AvgDod] = gGStatistDodInfo_62[eDodStat62_1NowDod];
            }
            else
            {
                gGStatistDodInfo_62[eDodStat62_2NowDod] = 1000 - gGBmuGenPara_102[eBmuGenPara102_2NowSoc];
                gGStatistDodInfo_62[eDodStat62_2FstDod] = gGStatistDodInfo_62[eDodStat62_2NowDod];
                gGStatistDodInfo_62[eDodStat62_2MaxDod] = gGStatistDodInfo_62[eDodStat62_2NowDod];
                gGStatistDodInfo_62[eDodStat62_2AvgDod] = gGStatistDodInfo_62[eDodStat62_2NowDod];
            }
        }

        /*读取历史DOD记录V2(标志+Id+50条环形记录,布局与RAM镜像完全一致)
          注:E2数据无效时读函数已向data[]回写默认值;E2未就绪路径未触碰data[],需显式清零*/
        base = clunum * HISDOD_RING_LEN;
        if(FALSE == ParaReadStoreHisDodInfoV2(clunum, &gGHisDodRing[base], PARA_DOD_HIS_V2_LEN))
        {
            for(k = 0; k < HISDOD_RING_LEN; k++)
            {
                gGHisDodRing[base + k] = 0;
            }
            gGHisDodRing[base + eHisDodRing_Flag] = PARA_CFG_FLAG_U16;
        }
    }
}

/*=================================================================================================
** @Name      : void GroupDodStatisRcdTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组放电深度DOD统计记录任务
** @The notes : 1s周期,双簇循环,放电转充电(充电持续60s确认)新增一次DOD
**===============================================================================================*/
void GroupDodStatisRcdTask(void)
{
    u8 flag = 0;
    u16 dodInfo[5] = {0};
    u16 nowDodIdx = 0;
    u16 fstDodIdx = 0;
    u16 nowSoc = 0;
    u16 nowDod = 0;

    if(gGStatistDodInfo_62 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistDodInfo_62 null!!!\r\n", __func__);
        return;
    }

#if DOD_RPMSG_SIM
    /*模拟:每120s向A核推送一条假DOD记录(双簇都推,验证用,绕开真实充放电触发)*/
    if (++sDodSimTimer >= 120)
    {
        sDodSimTimer = 0;
        GroupDodStatisRcdSimPush(0);
        GroupDodStatisRcdSimPush(1);
    }
#endif
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }

        flag = 0;

        /*计算当前簇的DOD统计值索引*/
        if(clunum == 0)
        {
            nowDodIdx = eDodStat62_1NowDod;
            fstDodIdx = eDodStat62_1FstDod;
        }
        else
        {
            nowDodIdx = eDodStat62_2NowDod;
            fstDodIdx = eDodStat62_2FstDod;
        }

        nowSoc = GetGBattSocAPI(clunum);
        nowDod = gGStatistDodInfo_62[nowDodIdx];

        /*初始值异常则更新*/
        if((0 == sHisChgCap[clunum]) || (sHisChgCap[clunum] > GetGHisChgCapAPI(clunum)))
        {
            sHisChgCap[clunum] = GetGHisChgCapAPI(clunum);
        }
        if((0 == sHisDhgCap[clunum]) || (sHisDhgCap[clunum] > GetGHisDhgCapAPI(clunum)))
        {
            sHisDhgCap[clunum] = GetGHisDhgCapAPI(clunum);
        }
        if((0 == sHisChgEner[clunum]) || (sHisChgEner[clunum] > GetGHisChgEnerAPI(clunum)))
        {
            sHisChgEner[clunum] = GetGHisChgEnerAPI(clunum);
        }
        if((0 == sHisDhgEner[clunum]) || (sHisDhgEner[clunum] > GetGHisDhgEnerAPI(clunum)))
        {
            sHisDhgEner[clunum] = GetGHisDhgEnerAPI(clunum);
        }

        /*系统初始化期不处理*/
        if(eWORK_INIT == GetGWorkStateAPI(clunum))
        {
            sCTimer[clunum] = 0;
            sDTimer[clunum] = 0;
            sHisSoc[clunum] = nowSoc;
            continue;
        }

        /*放电状态*/
        if(eCURR_DHG == GetGChgDhgStateAPI(clunum))
        {
            /*当前放电深度大于统计值*/
            if((1000 - nowSoc) > nowDod)
            {
                sDTimer[clunum] = 0;
                sRcdDod[clunum] = 1;
                gGStatistDodInfo_62[nowDodIdx] = 1000 - nowSoc;

                /*放电起始DOD不能大于当前DOD*/
                if(gGStatistDodInfo_62[fstDodIdx] > gGStatistDodInfo_62[nowDodIdx])
                {
                    gGStatistDodInfo_62[fstDodIdx] = gGStatistDodInfo_62[nowDodIdx];
                }
            }
            /*持续放电3min*/
            else if(sDTimer[clunum] >= 180)
            {
                /*SOC下降5%或SOC低于3%*/
                if(((nowSoc + 50) <= sHisSoc[clunum]) || (nowSoc < 30))
                {
                    sDTimer[clunum] = 0;
                    sRcdDod[clunum] = 1;
                    gGStatistDodInfo_62[fstDodIdx] = 1000 - sHisSoc[clunum];
                    gGStatistDodInfo_62[nowDodIdx] = 1000 - nowSoc;
                }
            }
            else
            {
                sDTimer[clunum]++;
            }

            sCTimer[clunum] = 0;
        }
        /*非放电状态*/
        else
        {
            /*当前放电深度大于统计值*/
            if((1000 - nowSoc) > nowDod)
            {
                sRcdDod[clunum] = 1;
                gGStatistDodInfo_62[nowDodIdx] = 1000 - nowSoc;
            }

            /*转为充电状态,需要更新当前DOD*/
            if((eCURR_CHG == GetGChgDhgStateAPI(clunum)) && (1 == sRcdDod[clunum]) && (0xffff != sEepDod[clunum]))
            {
                /*持续充电1min确认*/
                if(sCTimer[clunum] >= 60)
                {
                    /*生成DOD记录信息*/
                    dodInfo[0] = gGStatistDodInfo_62[nowDodIdx];
                    dodInfo[1] = GetGHisChgCapAPI(clunum) - sHisChgCap[clunum];
                    dodInfo[2] = GetGHisDhgCapAPI(clunum) - sHisDhgCap[clunum];
                    dodInfo[3] = GetGHisChgEnerAPI(clunum) - sHisChgEner[clunum];
                    dodInfo[4] = GetGHisDhgEnerAPI(clunum) - sHisDhgEner[clunum];

                    /*环形写入一条新的DOD记录(含时间戳)*/
                    GroupDodStatisRcdDeal(clunum, dodInfo, 5);

                    /*更新历史容量能量记录基准*/
                    sHisChgCap[clunum] = GetGHisChgCapAPI(clunum);
                    sHisDhgCap[clunum] = GetGHisDhgCapAPI(clunum);
                    sHisChgEner[clunum] = GetGHisChgEnerAPI(clunum);
                    sHisDhgEner[clunum] = GetGHisDhgEnerAPI(clunum);

                    /*中间计数器清零*/
                    sCTimer[clunum] = 0;
                    sRcdDod[clunum] = 0;
                    flag = 1;
                }
                else
                {
                    /*等待持续时间*/
                    sCTimer[clunum]++;
                }
            }
            /*非充电状态不需要记录DOD*/
            else
            {
                sCTimer[clunum] = 0;
            }

            /*充电时记录放电起始信息*/
            if(eCURR_CHG == GetGChgDhgStateAPI(clunum))
            {
                sDTimer[clunum] = 0;
                sHisSoc[clunum] = nowSoc;
            }
        }

        /*首次上电*/
        if(0xffff == sEepDod[clunum])
        {
            /*首次上电强制记录一次DOD(防止遗漏当前未记录)*/
            sRcdDod[clunum] = 1;

            /*记录当前已记录DOD值*/
            sEepDod[clunum] = gGStatistDodInfo_62[nowDodIdx];
        }
        /*历史DOD变化(发生新记录)*/
        else if(1 == flag)
        {
            /*写入EEPROM全部历史DOD值*/
            ParaWriteStoreDODStaInfo(clunum, 1, &gGStatistDodInfo_62[nowDodIdx], 4);
            sEepDod[clunum] = gGStatistDodInfo_62[nowDodIdx];
        }
        /*DOD变化超过5%或非放电状态DOD已变化*/
        else if((ABS(gGStatistDodInfo_62[nowDodIdx], sEepDod[clunum]) >= 50)
            || ((ABS(gGStatistDodInfo_62[nowDodIdx], sEepDod[clunum]) >= 1)
            && (eCURR_DHG != GetGChgDhgStateAPI(clunum))))
        {
            /*写入外部EEPROM当前DOD值*/
            ParaWriteStoreDODStaInfo(clunum, 1, &gGStatistDodInfo_62[nowDodIdx], 1);
            sEepDod[clunum] = gGStatistDodInfo_62[nowDodIdx];
        }
    }
}

/*=================================================================================================
** @Name      : void GroupClrDodStatisRcdAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除电池组放电深度DOD统计记录信息
** @The notes : 清除历史记录V2(环形整块)、统计值,并持久化到EEPROM
**===============================================================================================*/
void GroupClrDodStatisRcdAPI(u8 clunum)
{
    u16 k = 0;
    u16 zero = 0;
    u16 base = 0;

    if(clunum >= 2)
    {
        return;
    }
    if(gGStatistDodInfo_62 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGStatistDodInfo_62 null!!!\r\n", __func__);
        return;
    }
    if(gGHisDodRing == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGHisDodRing null!!!\r\n", __func__);
        return;
    }

    base = clunum * HISDOD_RING_LEN;

    /*清除历史记录环形缓冲整块(标志+Id+50条记录)*/
    gGHisDodRing[base + eHisDodRing_Flag] = PARA_CFG_FLAG_U16;
    gGHisDodRing[base + eHisDodRing_Id] = 0;
    for(k = 0; k < (HISDOD_REC_NUM * eHisDodRec_Len); k++)
    {
        gGHisDodRing[base + eHisDodRing_Rec + k] = 0;
    }

    /*清除DOD统计值*/
    if(clunum == 0)
    {
        gGStatistDodInfo_62[eDodStat62_1NowDod] = 0;
        gGStatistDodInfo_62[eDodStat62_1FstDod] = 0;
        gGStatistDodInfo_62[eDodStat62_1MaxDod] = 0;
        gGStatistDodInfo_62[eDodStat62_1AvgDod] = 0;
    }
    else
    {
        gGStatistDodInfo_62[eDodStat62_2NowDod] = 0;
        gGStatistDodInfo_62[eDodStat62_2FstDod] = 0;
        gGStatistDodInfo_62[eDodStat62_2MaxDod] = 0;
        gGStatistDodInfo_62[eDodStat62_2AvgDod] = 0;
    }

    /*持久化清除到EEPROM*/
    zero = 0;
    if(clunum == 0)
    {
        ParaWriteStoreDODStaInfo(clunum, 1, &gGStatistDodInfo_62[eDodStat62_1NowDod], 4);
    }
    else
    {
        ParaWriteStoreDODStaInfo(clunum, 1, &gGStatistDodInfo_62[eDodStat62_2NowDod], 4);
    }
    /*持久化清除历史DOD记录V2整块到EEPROM(掉电不丢失)*/
    ParaWriteStoreHisDodInfoV2(clunum, 0, &gGHisDodRing[base], PARA_DOD_HIS_V2_LEN);

    /*重置static状态变量(全部,防止基准残留导致下次记录dodInfo错误)*/
    sRcdDod[clunum] = 0;
    sCTimer[clunum] = 0;
    sDTimer[clunum] = 0;
    sEepDod[clunum] = 0xffff;
    sHisSoc[clunum] = 0;
    sHisChgCap[clunum] = 0;
    sHisDhgCap[clunum] = 0;
    sHisChgEner[clunum] = 0;
    sHisDhgEner[clunum] = 0;
}

/*=================================================================================================
** @Static function definition
**===========================================================================================*/
/*=================================================================================================
** @Name      : static void GroupDodStatisRcdDeal(u8 clunum, u16 *newDodInfo, u8 infoNum)
** @Input     : clunum:簇编号 newDodInfo:新记录DOD信息(DOD值/充电量/放电量/充电能/放电能) infoNum:信息个数(5个有效)
** @Output    : void
** @Function  : 环形写入一条新DOD记录(10字,与协议mIndex15/16同构),重算Max/Avg统计并推送A核
** @The notes : 写槽位=Id%N,记录id=Id+1;先写记录(20B)再写头Id,掉电时至多丢当前一条,环形状态自洽
**              推送:单发即清不重试,数据源为最新一条快照gGHisDodInfo1_15/_2_16(已注册ICC查询表);丢帧时A核靠帧内序号Id跳号检测
**===============================================================================================*/
static void GroupDodStatisRcdDeal(u8 clunum, u16 *newDodInfo, u8 infoNum)
{
    u8 i = 0;
    u16 id = 0;
    u16 newId = 0;
    u16 num = 0;
    u16 slot = 0;
    u16 roff = 0;
    u16 maxDod = 0;
    u32 sumDod = 0;
    u16 base = 0;
    u16 maxDodIdx = 0;
    u16 avgDodIdx = 0;
    u16 rec[eHisDodRec_Len] = {0};
    s16 sslot = 0;
    t_SysClock nowClock;
    u16 *pDodTx = NULL;

    /*参数值无效*/
    if((NULL == newDodInfo) || (5 != infoNum))
    {
        return;
    }
    if(clunum >= 2)
    {
        return;
    }

    base = clunum * HISDOD_RING_LEN;

    /*计算当前簇的Max/Avg索引*/
    if(clunum == 0)
    {
        maxDodIdx = eDodStat62_1MaxDod;
        avgDodIdx = eDodStat62_1AvgDod;
    }
    else
    {
        maxDodIdx = eDodStat62_2MaxDod;
        avgDodIdx = eDodStat62_2AvgDod;
    }

    /*环形写入:写槽位=当前Id%N(第newId条记录落在(newId-1)%N槽位)*/
    id = gGHisDodRing[base + eHisDodRing_Id];
    if(id > 60000)
    {
        id = 0;                                             /*回绕保护(重新计数)*/
    }
    newId = id + 1;
    slot = id % HISDOD_REC_NUM;
    roff = base + eHisDodRing_Rec + (slot * eHisDodRec_Len);

    /*生成新记录(10字,协议mIndex15/16子索引0~9:序号/年/月日/时分/秒/DOD/放电量/充电量/放电能/充电能)*/
    nowClock = GetSysDateClockAPI();
    rec[eHisDodRec_Id]     = newId;
    rec[eHisDodRec_Y]      = nowClock.year;
    rec[eHisDodRec_MD]     = ((u16)nowClock.month << 8) | (u16)nowClock.day;
    rec[eHisDodRec_HM]     = ((u16)nowClock.hour  << 8) | (u16)nowClock.minute;
    rec[eHisDodRec_S]      = nowClock.second;
    rec[eHisDodRec_Dod]    = newDodInfo[0];
    rec[eHisDodRec_DhgCap] = newDodInfo[2];                /*放电量(单次差值,单位同mIndex56)*/
    rec[eHisDodRec_ChgCap] = newDodInfo[1];                /*充电量(单次差值,单位同mIndex56)*/
    rec[eHisDodRec_DhgEner] = newDodInfo[4];               /*放电能(单次差值,单位同mIndex56)*/
    rec[eHisDodRec_ChgEner] = newDodInfo[3];               /*充电能(单次差值,单位同mIndex56)*/

    /*先写记录到E2(仅20字节,减少磨损),再更新RAM与头Id(掉电一致性)*/
    ParaWriteStoreHisDodInfoRecV2(clunum, slot, rec);
    for(i = 0; i < eHisDodRec_Len; i++)
    {
        gGHisDodRing[roff + i] = rec[i];
    }
    gGHisDodRing[base + eHisDodRing_Id] = newId;
    ParaWriteStoreHisDodInfoV2(clunum, eHisDodRing_Id, &gGHisDodRing[base + eHisDodRing_Id], 1);

    /*UPM推送新记录到A核(0x5A):簇1→mIndex15,簇2→mIndex16,子索引0~9共10字,与协议表一致*/
    /*单发即清不重试;先刷最新一条快照(ICC查询表注册)再以其为帧数据;丢帧由A核靠序号Id跳号发现*/
    pDodTx = (0 == clunum) ? gGHisDodInfo1_15 : gGHisDodInfo2_16;
    if(pDodTx != NULL)
    {
        for(i = 0; i < eHisDodRec_Len; i++)
        {
            pDodTx[i] = rec[i];
        }
        (void)ICCReqIdleTxRcvTab(eICCFUN_UPM, pDodTx, ICC_DT_U16, (0 == clunum) ? 15 : 16, 0,
                                 eHisDodRec_Len, 0, 0x5A, eICCCHA_FAQ);
    }

    /*遍历有效记录重算Max/Avg(从最新向最旧回溯,统计口径与记录条数严格一致)*/
    num = (newId < HISDOD_REC_NUM) ? newId : HISDOD_REC_NUM;
    maxDod = 0;
    sumDod = 0;
    for(i = 0; i < num; i++)
    {
        sslot = (s16)(((s32)newId - 1 - (s32)i) % HISDOD_REC_NUM);
        if(sslot < 0)
        {
            sslot += HISDOD_REC_NUM;
        }
        roff = base + eHisDodRing_Rec + ((u16)sslot * eHisDodRec_Len);
        sumDod += gGHisDodRing[roff + eHisDodRec_Dod];
        if(gGHisDodRing[roff + eHisDodRec_Dod] > maxDod)
        {
            maxDod = gGHisDodRing[roff + eHisDodRec_Dod];
        }
    }

    /*更新统计记录*/
    gGStatistDodInfo_62[maxDodIdx] = maxDod;
    gGStatistDodInfo_62[avgDodIdx] = (u16)(sumDod / num);
}

#if DOD_RPMSG_SIM
/*=================================================================================================
** @Name      : static void GroupDodStatisRcdSimPush(u8 clunum)
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 【模拟用】构造一条假DOD记录(rec[10])并经UPM推送A核(mIndex15/16),
**              与真实 GroupDodStatisRcdDeal 推送同参同构;仅用于板端验证,量产前关闭
**===============================================================================================*/
static void GroupDodStatisRcdSimPush(u8 clunum)
{
    u16 rec[eHisDodRec_Len] = {0};
    t_SysClock nowClock;

    if (clunum >= 2)
    {
        return;
    }

    /*ICC框架未初始化时 txData 为 NULL,跳过推送避免写空指针崩溃*/
    if (txData == NULL)
    {
        return;
    }

    nowClock = GetSysDateClockAPI();
    sDodSimSeqId[clunum]++;

    rec[eHisDodRec_Id]      = sDodSimSeqId[clunum];
    rec[eHisDodRec_Y]       = nowClock.year;
    rec[eHisDodRec_MD]      = ((u16)nowClock.month << 8) | (u16)nowClock.day;
    rec[eHisDodRec_HM]      = ((u16)nowClock.hour  << 8) | (u16)nowClock.minute;
    rec[eHisDodRec_S]       = nowClock.second;
    rec[eHisDodRec_Dod]     = (u16)(100 + ((sDodSimSeqId[clunum] * 37) % 800)); /*100~899伪随机DOD(0.1%)*/
    rec[eHisDodRec_DhgCap]  = (u16)(sDodSimSeqId[clunum] * 10);   /*放电量*/
    rec[eHisDodRec_ChgCap]  = (u16)(sDodSimSeqId[clunum] * 7);    /*充电量*/
    rec[eHisDodRec_DhgEner] = (u16)(sDodSimSeqId[clunum] * 13);   /*放电能*/
    rec[eHisDodRec_ChgEner] = (u16)(sDodSimSeqId[clunum] * 9);    /*充电能*/

    ssdk_printf(SSDK_WARNING, "%s: [SIM] clu%d seq=%u dod=%u\r\n",
                __func__, clunum, rec[eHisDodRec_Id], rec[eHisDodRec_Dod]);

    u16 *pDodTx = (0 == clunum) ? gGHisDodInfo1_15 : gGHisDodInfo2_16;   /*与真实推送同源*/
    if (pDodTx != NULL)
    {
        for (u8 i = 0; i < eHisDodRec_Len; i++)
        {
            pDodTx[i] = rec[i];
        }
        (void)ICCReqIdleTxRcvTab(eICCFUN_UPM, pDodTx, ICC_DT_U16, (0 == clunum) ? 15 : 16, 0,
                                 eHisDodRec_Len, 0, 0x5A, eICCCHA_FAQ);
    }
}
#endif