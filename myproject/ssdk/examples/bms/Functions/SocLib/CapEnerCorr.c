/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CapEnerCorr.c
** @Founder      : ZQD808
** @Date         : 2026.03.13
** @Function     : 总容量总电能自动标定模块
** @Instructions :
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "PublicInfo.h"
#include "GroupPara.h"
#include "CapInfoCalc.h"
#include "ParaInterface.h"
#include "EnerInterface.h"
#include "SocInterface.h"
#include "EnerInfoCalc.h"
#include "EnerInterface.h"
#include "CapEnerCorr.h"
#include "CapInfoCalc.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
static u8 sCapEnerDropExpFlag[2] = {0};  /*总容量总能量衰减异常标志(Bit0:充电总容量检测衰减 bit1:放电总容量检测衰减 bit2:充电总能量衰减 bit3:放电总能量衰减  bit4:充电检测超时 bit5:放电检测超时)[0正常 1异常]*/

t_CapECorr *sCapECorrVHP = NULL;     /*总容量总能量检测记录高压点信息*/
t_CapECorr *sCapECorrVLP = NULL;     /*总容量总能量检测记录低压点信息*/

extern const u16 cTempCapPercent[]; /*不同温度下可用总容量百分比(0.1%)*/
extern const u16 cTempEnerPercent[];/*不同温度下可用总能量百分比(0.1%)*/
extern const u16 cCurrCapPercent[]; /*不同电流下可用总容量百分比(0.1%)*/
extern const u16 cCurrEnerPercent[];/*不同电流下可用总能量百分比(0.1%)*/

extern u16 gGBmuGenPara_102[];      /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
/*=============================================================================================
** @Name      : static u8 GetCapCheckTimeoutReq(u8 clunum, u8 sort)
** @Input     : clunum:簇编号 sort:容量检测点 0低压点(放空请求) 1高压电(充满请求)
** @Output    : 需满充满放请求
** @Function  : 获取容量检测超时需满充满放请求
** @The notes : 30天未满充满放
**===========================================================================================*/
static u8 GetCapCheckTimeoutReq(u8 clunum, u8 sort);
/*=============================================================================================
** @Name      : static void ChgCorrTotalCapEnerDeal(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 充电总容量总能量自动标定处理
** @The notes :
**===========================================================================================*/
static void ChgCorrTotalCapEnerDeal(u8 clunum);
/*=============================================================================================
** @Name      : static void ChgCorrTotalCapEnerDeal(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 充电总容量总能量自动标定处理
** @The notes :
**===========================================================================================*/
static void ChgCorrTotalCapEnerDeal(u8 clunum);
/*=============================================================================================
** @Name      :void CapEnerCorrInfoInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时参数配置还没初始化，按2簇内存分配
**===========================================================================================*/
void CapEnerCorrInfoInit(void)
{
    static u8 FirstInitFlag = 0;
    if(FirstInitFlag == 1)
    {
        return;
    }
    sCapECorrVHP = ExtMalloc( 2 * sizeof(t_CapECorr)); 
    if(sCapECorrVHP == NULL)
    {
       ssdk_printf(SSDK_WARNING, "%s: sCapECorrVHP null!!!\r\n", __func__);
       return;
    }
    else
    {
        printf("malloc sCapECorrVHP %d success!!!\r\n", 2 * sizeof(t_CapECorr));
        memset(sCapECorrVHP, 0, 2 * sizeof(t_CapECorr));
    }
    sCapECorrVLP = ExtMalloc(2 * sizeof(t_CapECorr)); 
    if(sCapECorrVLP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVLP null!!!\r\n", __func__);
        return;
    }
    else
    {
        printf("malloc sCapECorrVLP %d success!!!\r\n",2 * sizeof(t_CapECorr));
        memset(sCapECorrVLP, 0, 2 *sizeof(t_CapECorr));
    }
    FirstInitFlag = 1;
}
/*=============================================================================================
** @Name      : void TotalCapEnerCorrInit(void)
** @Input     : void
** @Output    : void
** @Function  : 总容量总能量自动标定初始化
** @The notes :
**===========================================================================================*/
void TotalCapEnerCorrInit(void)
{
    u32 eepInfo[19] = {0};
    u8 clunum = 0;
    if(sCapECorrVLP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVLP null!!!\r\n", __func__);
        return;
    }
    if(sCapECorrVHP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVHP null!!!\r\n", __func__);
        return;
    }
    for(clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        //读取EEP记录信息
        if(TRUE == SocReadCapEnerCorrInfoHook(clunum, eepInfo, 19))
        {
            //低压点
            sCapECorrVLP[clunum].effect = eepInfo[1];
            sCapECorrVLP[clunum].timer = eepInfo[2];
            sCapECorrVLP[clunum].chgCap = eepInfo[3];
            sCapECorrVLP[clunum].dhgCap = eepInfo[4];
            sCapECorrVLP[clunum].chgEner = eepInfo[5];
            sCapECorrVLP[clunum].dhgEner = eepInfo[6];
            sCapECorrVLP[clunum].allCap = eepInfo[7];
            sCapECorrVLP[clunum].allEner = eepInfo[8];
            sCapECorrVLP[clunum].date = eepInfo[9];

            //高压点
            sCapECorrVHP[clunum].effect = eepInfo[10];
            sCapECorrVHP[clunum].timer = eepInfo[11];
            sCapECorrVHP[clunum].chgCap = eepInfo[12];
            sCapECorrVHP[clunum].dhgCap = eepInfo[13];
            sCapECorrVHP[clunum].chgEner = eepInfo[14];
            sCapECorrVHP[clunum].dhgEner = eepInfo[15];
            sCapECorrVHP[clunum].allCap = eepInfo[16];
            sCapECorrVHP[clunum].allEner = eepInfo[17];
            sCapECorrVHP[clunum].date = eepInfo[18];
        }
        else
        {
            //记录无效
            sCapECorrVHP[clunum].effect = 0;
            sCapECorrVLP[clunum].effect = 0;
            sCapECorrVHP[clunum].date = 0;
            sCapECorrVLP[clunum].date = 0;
        }
    }
}
/*=============================================================================================
** @Name      : void TotalCapEnerCorrTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组总容量总能量自动标定任务
** @The notes : 1s
**===========================================================================================*/
void TotalCapEnerCorrTask(void)
{
    u16 state = 0;
    static u8 sFlag[2] = {0};
    u8 clunum = 0;
    for(clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        state = 0;
        if((eMODE_EMERG == GetGWorkModeAPI(clunum))       //非正常模式不检测
            || (eMODE_DEBUG == GetGWorkModeAPI(clunum))   //非正常模式不检测
            || (1 == CheckHisCapEnerClrState(clunum)))    //发生历史电量被清零
        {
            if(0 == sFlag[clunum])
            {
                ClrAllCapEnerCorrInfo(clunum);            //清除记录的容量检测信息
                sFlag[clunum] = 1;
            }
        }
        else if(1 == EnerGetGroupBattReadyHook(clunum))   //电池数据就绪
        {
            sFlag[clunum] = 0;

            state = GetGChgDhgStateAPI(clunum);           //充放电状态

            if(eCURR_DHG != state)                  //正在充电(或空闲)
            {
                ChgCorrTotalCapEnerDeal(clunum);
            }

            if(eCURR_CHG != state)                 //正在放电(或空闲)
            {
                DhgCorrTotalCapEnerDeal(clunum);
            }
        }
    }
}
/*=============================================================================================
** @Name      : u8 GetCapEnerDropExpAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 容量衰减异常标志(Bit0:充电总容量检测衰减 bit1:放电总容量检测衰减 bit2:充电总能量衰减 bit3:放电总能量衰减  bit4:充电检测超时 bit5:放电检测超时)[0正常 1异常]
** @Function  : 获取总容量总能量衰减异常标志
** @The notes :
**===========================================================================================*/
u8 GetCapEnerDropExpAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    return(sCapEnerDropExpFlag[clunum]);
}
/*=============================================================================================
** @Name      : static void ChgCorrTotalCapEnerDeal(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 充电总容量总能量自动标定处理
** @The notes :
**===========================================================================================*/
static void ChgCorrTotalCapEnerDeal(u8 clunum)
{
    u32 allCap = 0;
    u32 chgCap = 0;
    u32 dhgCap = 0;
    u32 calcCap = 0;
    u32 allEner = 0;
    u32 chgEner = 0;
    u32 dhgEner = 0;
    u32 calcEner = 0;
    static s8 sFinTemp[2] = {0};
    static s16 sFinCurr[2] = {0};
    static u32 sTime[2] = {0};
    if(sCapECorrVLP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVLP null!!!\r\n", __func__);
        return;
    }
    if(sCapECorrVHP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVHP null!!!\r\n", __func__);
        return;
    }
    //for(clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    //{
        if(clunum >= 2)
        {
            return;
        }
        if(eCURR_CHG == GetGChgDhgStateAPI(clunum))                                       //正在充电
        {
            sCapECorrVHP[clunum].timer++;                                                   //计算充电时长
        }

        if(1 == EnerGetGroupVHLimStateHook(clunum))                                               //充电到高压点
        {
            if((0 == sCapECorrVHP[clunum].effect)                                                 //未记录高压点信息
                || (1 == sCapECorrVLP[clunum].effect)                                             //已记录低压点未处理
                || (GetGHisChgCapAPI(clunum) > (sCapECorrVHP[clunum].chgCap + 5))                 //再次充电到截止
                || (GetGHisChgEnerAPI(clunum) > (sCapECorrVHP[clunum].chgEner + 40)))             //再次充电到截止
            {
                //记录高压点信息
                sCapECorrVHP[clunum].effect = 1;                                                  //高压记录点有效
                sCapECorrVHP[clunum].chgCap = GetGHisChgCapAPI(clunum);                           //高压时充电电量
                sCapECorrVHP[clunum].dhgCap = GetGHisDhgCapAPI(clunum);                           //高压时放电电量
                sCapECorrVHP[clunum].chgEner = GetGHisChgEnerAPI(clunum);                         //高压时充电电能
                sCapECorrVHP[clunum].dhgEner = GetGHisDhgEnerAPI(clunum);                         //高压时放电电能
                SocWriteCapEnerCorrInfoHook(clunum, 1, (u32*)&sCapECorrVHP[clunum], 6);           //保存高压点信息
            }

            if(1 == sCapECorrVLP[clunum].effect)                                            //已记录低压点
            {
                if(IsInside(CAP_CHECK_MIN_T, sCapECorrVHP[clunum].timer, CAP_CHECK_MAX_T))  //充电时长有效
                {
                    allCap = GetGroupTotalCapAPI(clunum);                                 //获得总容量(mAH)
                    chgCap = sCapECorrVHP[clunum].chgCap - sCapECorrVLP[clunum].chgCap;             //低压到高压过程的充电电量
                    chgCap = chgCap * CAP_CHG_DHG_PER / 1000;                       //根据充放电容量比计算对应的有效充电容量
                    dhgCap = sCapECorrVHP[clunum].dhgCap - sCapECorrVLP[clunum].dhgCap;             //低压到高压过程的放电电量(中间可能有放电过程)

                    if((chgCap > dhgCap) && (sCapECorrVHP[clunum].chgCap >= sCapECorrVLP[clunum].chgCap) && (sCapECorrVHP[clunum].dhgCap >= sCapECorrVLP[clunum].dhgCap))//计算容量正确(充电量大)
                    {
                        calcCap = (chgCap - dhgCap) * 1000;                         //计算的实际充电容量(mAH)

                        calcCap = CalcTotalCapEnerByCTCorr(clunum, 0, calcCap, sFinCurr[clunum], sFinTemp[clunum]);//计算电流温度修正后的充电容量(mAH)

                        //计算充电容量与放电容量误差20%
                        if(1 == IsInside((sCapECorrVLP[clunum].allCap * 80), (calcCap * 100), (sCapECorrVLP[clunum].allCap * 120)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 0);
                            calcCap = (calcCap + sCapECorrVLP[clunum].allCap) / 2;         //计算需修正的总容量
                        }
                        //计算充电容量与原容量误差30%
                        else if(1 == IsInside((allCap * 70), (calcCap * 100), (allCap * 130)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 0);

                            if(allCap != GetGroupStandCapAPI(clunum))                     //非第1次修正
                            {
                                calcCap = (calcCap + allCap) / 2;                   //计算需修正的总容量(取1/2)
                            }
                            else                                                    //第1次修正
                            {
                                calcCap = (calcCap * 20 + allCap * 12) / 32;        //计算需修正的总容量(取2/3)
                            }
                        }
                        //本次计算充电容量与上次计算充电电容量误差20%
                        else if(1 == IsInside((sCapECorrVHP[clunum].allCap * 80), (calcCap * 100), (sCapECorrVHP[clunum].allCap * 120)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 0);
                            calcCap = (calcCap + sCapECorrVHP[clunum].allCap) / 2;         //计算需修正的总容量
                        }
                        //计算第1次修正充电容量与原容量误差40%
                        else if((1 == IsInside((allCap * 60), (calcCap * 100), (allCap * 140)))
                            && (allCap == GetGroupStandCapAPI(clunum)))                   //未修正过总容量
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 0);
                            calcCap = (calcCap * 20 + allCap * 12) / 32;            //计算需修正的总容量(取2/3)
                        }
                        //本次计算总容量错误
                        else
                        {
                            BitSet(sCapEnerDropExpFlag[clunum], 0);                         //计算容量衰减异常
                            //calcCap = 0;                                          //总容量清零
                            calcCap = (calcCap + sCapECorrVHP[clunum].allCap) / 2;          //总容量取最近两次平均值
                        }

                        if(0 == BitGet(sCapEnerDropExpFlag[clunum], 0))                     //测定总容量正常
                        {
                            if((ABS(calcCap, allCap) > (allCap / 100))              //修正量超过1%
                                && (allCap != GetGroupStandCapAPI(clunum)))               //非第一次修正
                            {
                                if(calcCap > allCap)
                                {
                                    if(calcCap > (allCap + (allCap / 100)))
                                    {
                                        calcCap = allCap + (allCap / 100);          //只取修正1%
                                    }
                                }
                                else
                                {
                                    if(calcCap < (allCap - (allCap * 3 / 100)))
                                    {
                                        calcCap = allCap - (allCap * 3 / 100);      //只取修正3%
                                    }
                                }
                            }

                            CorrGroupTotalCapAPI(clunum, calcCap);                          //修正总容量
                        }
                    }

                    allEner = GetGroupTotalEnerAPI(clunum);                               //获得总能量(1Wh)
                    chgEner = sCapECorrVHP[clunum].chgEner - sCapECorrVLP[clunum].chgEner;          //低压到高压过程的充电电能
                    chgEner = chgEner * ENER_CHG_DHG_PER / 1000;                    //根据充放电能量比计算对应的有效充电能量
                    dhgEner = sCapECorrVHP[clunum].dhgEner - sCapECorrVLP[clunum].dhgEner;          //低压到高压过程的放电电能(中间可能有放电过程)

                    if((chgEner > dhgEner) && (sCapECorrVHP[clunum].chgEner >= sCapECorrVLP[clunum].chgEner) && (sCapECorrVHP[clunum].dhgEner >= sCapECorrVLP[clunum].dhgEner))//计算能量正确(充电量大)
                    {
                        calcEner = (chgEner - dhgEner) * 100;                       //计算的实际充电能量(Wh)

                        calcEner = CalcTotalCapEnerByCTCorr(clunum, 1, calcEner, sFinCurr[clunum], sFinTemp[clunum]);//计算电流温度修正后的充电能量(Wh)

                        //计算充电能量与放电能量误差30%
                        if(1 == IsInside((sCapECorrVLP[clunum].allEner * 70), (calcCap * 100), (sCapECorrVLP[clunum].allEner * 130)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 2);
                            calcEner = (calcEner + sCapECorrVLP[clunum].allEner) / 2;       //计算需修正的总能量
                        }
                        //计算充电能量与原能量误差30%
                        else if(1 == IsInside((allEner * 70), (calcEner * 100), (allEner * 130)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 2);

                            if(allEner != GetGroupStandEnerAPI(clunum))                   //非第1次修正
                            {
                                calcEner = (calcEner + allEner) / 2;                //计算需修正的总能量(取1/2)
                            }
                            else                                                    //第1次修正
                            {
                                calcEner = (calcEner * 20 + allEner * 12) / 32;     //计算需修正的总能量(取2/3)
                            }
                        }
                        //本次计算充电能量与上次计算充电电能量误差20%
                        else if(1 == IsInside((sCapECorrVHP[clunum].allEner * 80), (calcEner * 100), (sCapECorrVHP[clunum].allEner * 120)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 2);
                            calcEner = (calcEner + sCapECorrVHP[clunum].allEner) / 2;       //计算需修正的总能量
                        }
                        //计算第1次修正充电能量与原能量误差40%
                        else if((1 == IsInside((allEner * 60), (calcEner * 100), (allEner * 140)))
                            && (allEner == GetGroupStandEnerAPI(clunum)))                 //未修正过总能量
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 2);
                            calcEner = (calcEner * 20 + allEner * 12) / 32;         //计算需修正的总能量(取2/3)
                        }
                        //本次计算总能量错误
                        else
                        {
                            BitSet(sCapEnerDropExpFlag[clunum], 2);                         //计算能量衰减异常
                            //calcEner = 0;                                         //总能量清零
                            calcEner = (calcEner + sCapECorrVHP[clunum].allEner) / 2;       //总能量取最近两次平均值
                        }

                        if(0 == BitGet(sCapEnerDropExpFlag[clunum], 2))                     //测定总能量正常
                        {
                            if((ABS(calcEner, allEner) > (allEner / 100))           //修正量超过1%
                                && (allEner != GetGroupStandEnerAPI(clunum)))             //非第一次修正
                            {
                                if(calcEner > allEner)
                                {
                                    if(calcEner > (allEner + (allEner / 100)))
                                    {
                                        calcEner = allEner + (allEner / 100);       //只取修正1%
                                    }
                                }
                                else
                                {
                                    if(calcEner < (allEner - (allEner * 3 / 100)))
                                    {
                                        calcEner = allEner - (allEner * 3 / 100);   //只取修正3%
                                    }
                                }
                            }

                            CorrGroupTotalEnerAPI(clunum, calcEner);                        //修正总能量
                            //CorrGroupStandEnerAPI(calcCap);                       //修正标称能量(充电能量大当做标称容量)
                        }
                    }
                }

                //记录高压点计算的总容量(无法计算或计算错误时为0)
                sCapECorrVHP[clunum].allCap = calcCap;                                      //更新计算到的充电容量
                sCapECorrVHP[clunum].allEner = calcEner;                                    //更新计算到的充电能量
                //计算有效且容量/能量均已修正才更新修正日期
                if((calcCap > 0) && (calcEner > 0)
                    && (0 == BitGet(sCapEnerDropExpFlag[clunum], 0))
                    && (0 == BitGet(sCapEnerDropExpFlag[clunum], 2)))
                {
                    sCapECorrVHP[clunum].date = GetCapCheckNowDate();                       //更新高压时容量检测日期
                }
                SocWriteCorrTotalCapEnerHook(clunum, 1, (u32*)&sCapECorrVHP[clunum].allCap, 3);     //保存高压点充电容量充电能量

                //到高压点后则下一步等待低压，因此需清除低压点,防止再次高压时误修正
                sTime[clunum] = 0;
                sCapECorrVLP[clunum].effect = 0;
                sCapECorrVLP[clunum].timer = 0;
                SocWriteCapEnerCorrInfoHook(clunum, 0, (u32*)&sCapECorrVLP[clunum], 2);             //清除低压点信息
            }
        }
        //非充电截止
        else
        {

            //记录实时电流温度(充电截止前电流温度)
            sFinCurr[clunum] = GetGSampOutCurrAPI(clunum);
            sFinTemp[clunum] = (GetGCellAvgTempAPI(clunum) + 5) / 10;

            //刚上电
            if((0 == sTime[clunum]) && (sCapECorrVHP[clunum].timer > (CAP_CHECK_RECORD_T * 2)))
            {
                sTime[clunum] = sCapECorrVHP[clunum].timer;
            }
            //每10min记录一次充电时间
            else if(sCapECorrVHP[clunum].timer >= (sTime[clunum] + CAP_CHECK_RECORD_T))
            {
                SocWriteCapEnerCorrTimeHook(clunum, 1, sCapECorrVHP[clunum].timer);
                sTime[clunum] = sCapECorrVHP[clunum].timer;
            }

            //长时间未充电高压(满充),需充满请求
            if(TRUE == GetCapCheckTimeoutReq(clunum, 1))
            {
                //记录充电容量检测超时异常
                //BitSet(sCapEnerDropExpFlag, 4);
            }
            //未达到容量标定时间
            else
            {
                //清除放电容量检测超时
                BitClr(sCapEnerDropExpFlag[clunum], 4);
            }
        }
    //}
}
/*=============================================================================================
** @Name      : static void DhgCorrTotalCapEnerDeal(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 放电总容量总能量自动标定处理
** @The notes :
**===========================================================================================*/
static void DhgCorrTotalCapEnerDeal(u8 clunum)
{
    u32 allCap = 0;
    u32 chgCap = 0;
    u32 dhgCap = 0;
    u32 calcCap = 0;
    u32 allEner = 0;
    u32 chgEner = 0;
    u32 dhgEner = 0;
    u32 calcEner = 0;
    static s8 sFinTemp[2] = {0};
    static s16 sFinCurr[2] = {0};
    static u32 sTime[2] = {0};
    if(sCapECorrVLP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVLP null!!!\r\n", __func__);
        return;
    }
    if(sCapECorrVHP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVHP null!!!\r\n", __func__);
        return;
    }
    //for(clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    //{
        if(clunum >= 2)
        {
            return;
        }
        if(eCURR_DHG == GetGChgDhgStateAPI(clunum))                                       //正在放电
        {
            sCapECorrVLP[clunum].timer++;                                                   //计算放电时长
        }

        if(1 == EnerGetGroupVLLimStateHook(clunum))                                         //放电到低压点
        {
            if((0 == sCapECorrVLP[clunum].effect)                                           //未记录低压点信息
                || (1 == sCapECorrVHP[clunum].effect)                                       //已记录高压点未处理
                || (GetGHisDhgCapAPI(clunum) > (sCapECorrVLP[clunum].dhgCap + 5))                 //再次放电到截止
                || (GetGHisDhgEnerAPI(clunum) > (sCapECorrVLP[clunum].dhgEner + 40)))             //再次放电到截止
            {
                //记录低压点
                sCapECorrVLP[clunum].effect = 1;                                            //低压记录点有效
                sCapECorrVLP[clunum].chgCap = GetGHisChgCapAPI(clunum);                           //低压时充电电量
                sCapECorrVLP[clunum].dhgCap = GetGHisDhgCapAPI(clunum);                           //低压时放电电量
                sCapECorrVLP[clunum].chgEner = GetGHisChgEnerAPI(clunum);                         //高压时充电电能
                sCapECorrVLP[clunum].dhgEner = GetGHisDhgEnerAPI(clunum);                         //高压时放电电能
                SocWriteCapEnerCorrInfoHook(clunum, 0, (u32*)&sCapECorrVLP[clunum], 6);             //保存低压点信息
            }

            if(1 == sCapECorrVHP[clunum].effect)                                            //已记录高压点
            {
                if(IsInside(CAP_CHECK_MIN_T, sCapECorrVLP[clunum].timer, CAP_CHECK_MAX_T))  //放电时长有效
                {
                    allCap = GetGroupTotalCapAPI(clunum);                                 //获得总容量(mAH)
                    dhgCap = sCapECorrVLP[clunum].dhgCap - sCapECorrVHP[clunum].dhgCap;             //高压到低压过程的放电电量
                    chgCap = sCapECorrVLP[clunum].chgCap - sCapECorrVHP[clunum].chgCap;             //高压到低压过程的充电电量(中间可能有充电过程)
                    chgCap = chgCap * CAP_CHG_DHG_PER / 1000;                       //根据充放电容量比计算对应的有效充电容量

                    if((dhgCap > chgCap) && (sCapECorrVLP[clunum].dhgCap >= sCapECorrVHP[clunum].dhgCap) && (sCapECorrVLP[clunum].chgCap >= sCapECorrVHP[clunum].chgCap))//计算容量正确(放电量大)
                    {
                        calcCap = (dhgCap - chgCap) * 1000;                         //计算的实际放电容量(mAH)

                        calcCap = CalcTotalCapEnerByCTCorr(clunum, 0, calcCap, sFinCurr[clunum], sFinTemp[clunum]);//计算电流温度修正后的放电容量(mAH)

                        //计算放电容量与充电容量误差20%
                        if(1 == IsInside((sCapECorrVHP[clunum].allCap * 80), (calcCap * 100), (sCapECorrVHP[clunum].allCap * 120)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 1);
                            calcCap = (calcCap + sCapECorrVHP[clunum].allCap) / 2;         //计算需修正的总容量
                        }
                        //计算放电容量与原容量误差30%
                        else if(1 == IsInside((allCap * 70), (calcCap * 100), (allCap * 130)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 1);

                            if(allCap != GetGroupStandCapAPI(clunum))                     //非第1次修正
                            {
                                calcCap = (calcCap + allCap) / 2;                   //计算需修正的总容量(取1/2)
                            }
                            else                                                    //第1次修正
                            {
                                calcCap = (calcCap * 20 + allCap * 12) / 32;        //计算需修正的总容量(取2/3)
                            }
                        }
                        //本次计算放电容量与上次计算放电电容量误差20%
                        else if(1 == IsInside((sCapECorrVLP[clunum].allCap * 80), (calcCap * 100), (sCapECorrVLP[clunum].allCap * 120)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 1);
                            calcCap = (calcCap + sCapECorrVLP[clunum].allCap) / 2;          //计算需修正的总容量
                        }
                        //计算第1次修正放电容量与原容量误差40%
                        else if((1 == IsInside((allCap * 60), (calcCap * 100), (allCap * 140)))
                            && (allCap == GetGroupStandCapAPI(clunum)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 1);
                            calcCap = (calcCap * 20 + allCap * 12) / 32;            //计算需修正的总容量(取2/3)
                        }
                        //本次计算总容量错误
                        else
                        {
                            BitSet(sCapEnerDropExpFlag[clunum], 1);                         //记录容量衰减异常
                            //calcCap = 0;                                          //总容量清零
                            calcCap = (calcCap + sCapECorrVLP[clunum].allCap) / 2;          //总容量取最近两次平均值
                        }

                        if(0 == BitGet(sCapEnerDropExpFlag[clunum], 1))
                        {
                            if((ABS(calcCap, allCap) > (allCap / 100))              //修正量超过1%
                                && (allCap != GetGroupStandCapAPI(clunum)))               //非第一次修正
                            {
                                if(calcCap > allCap)
                                {
                                    if(calcCap > (allCap + (allCap / 100)))
                                    {
                                        calcCap = allCap + (allCap / 100);          //只取修正1%
                                    }
                                }
                                else
                                {
                                    if(calcCap < (allCap - (allCap * 3 / 100)))
                                    {
                                        calcCap = allCap - (allCap * 3 / 100);      //只取修正3%
                                    }
                                }
                            }

                            CorrGroupTotalCapAPI(clunum, calcCap);                          //修正总容量
                        }
                    }

                    allEner = GetGroupTotalEnerAPI(clunum);                               //获得总能量(Wh)
                    dhgEner = sCapECorrVLP[clunum].dhgEner - sCapECorrVHP[clunum].dhgEner;          //高压到低压过程的放电电能
                    chgEner = sCapECorrVLP[clunum].chgEner - sCapECorrVHP[clunum].chgEner;          //高压到低压过程的充电电能(中间可能有充电过程)
                    chgEner = chgEner * ENER_CHG_DHG_PER / 1000;                    //根据充放电能量比计算对应的有效充电能量

                    if((dhgEner > chgEner) && (sCapECorrVLP[clunum].dhgEner >= sCapECorrVHP[clunum].dhgEner) && (sCapECorrVLP[clunum].chgEner >= sCapECorrVHP[clunum].chgEner))//计算容量正确(放电量大)
                    {
                        calcEner = (dhgEner - chgEner) * 100;                       //计算的实际放电能量(Wh)

                        calcEner = CalcTotalCapEnerByCTCorr(clunum, 1, calcEner, sFinCurr[clunum], sFinTemp[clunum]);//计算电流温度修正后的放电能量(Wh)

                        //计算放电能量与原能量误差30%
                        if(1 == IsInside((allEner * 70), (calcEner * 100), (allEner * 130)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 3);

                            if(allEner != GetGroupStandEnerAPI(clunum))                   //非第1次修正
                            {
                                calcEner = (calcEner + allEner) / 2;                //计算需修正的总能量(取1/2)
                            }
                            else                                                    //第1次修正
                            {
                                calcEner = (calcEner * 20 + allEner * 12) / 32;     //计算需修正的总能量(取2/3)
                            }
                        }
                        //本次计算放电能量与上次计算放电电能量误差20%
                        else if(1 == IsInside((sCapECorrVLP[clunum].allEner * 80), (calcEner * 100), (sCapECorrVLP[clunum].allEner * 120)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 3);
                            calcEner = (calcEner + sCapECorrVLP[clunum].allEner) / 2;       //计算需修正的总能量
                        }
                        //计算放电能量与充电能量误差30%
                        else if(1 == IsInside((sCapECorrVHP[clunum].allEner * 70), (calcEner * 100), (sCapECorrVHP[clunum].allEner * 130)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 3);
                            //calcEner = (calcEner + sCapECorrVHP.allEner) / 2;     //计算需修正的总能量
                            calcEner = (calcEner * 20 + sCapECorrVHP[clunum].allEner * 12) / 32;//计算需修正的总能量(取2/3)
                        }
                        //计算第1次修正放电能量与原能量误差40%
                        else if((1 == IsInside((allEner * 60), (calcEner * 100), (allEner * 140)))
                            && (allEner == GetGroupStandEnerAPI(clunum)))
                        {
                            BitClr(sCapEnerDropExpFlag[clunum], 3);
                            calcEner = (calcEner * 20 + allEner * 12) / 32;         //计算需修正的总能量(取2/3)
                        }
                        //本次计算总能量错误
                        else
                        {
                            BitSet(sCapEnerDropExpFlag[clunum], 3);                         //记录能量衰减异常
                            //calcEner = 0;                                         //总能量清零
                            calcEner = (calcEner + sCapECorrVLP[clunum].allEner) / 2;       //总能量取最近两次平均值
                        }

                        if(0 == BitGet(sCapEnerDropExpFlag[clunum], 3))
                        {
                            if((ABS(calcEner, allEner) > (allEner / 100))           //修正量超过1%
                                && (allEner != GetGroupStandEnerAPI(clunum)))             //非第一次修正
                            {
                                if(calcEner > allEner)
                                {
                                    if(calcEner > (allEner + (allEner / 100)))
                                    {
                                        calcEner = allEner + (allEner / 100);      //只取修正1%
                                    }
                                }
                                else
                                {
                                    if(calcEner < (allEner - (allEner * 3 / 100)))
                                    {
                                        calcEner = allEner - (allEner * 3 / 100);   //只取修正3%
                                    }
                                }
                            }

                            CorrGroupTotalEnerAPI(clunum, calcEner);                        //修正总容量
                        }
                    }
                }

                //记录低压点计算的总容量(无法计算或计算错误时为0)
                sCapECorrVLP[clunum].allCap = calcCap;                                      //更新计算到的放电容量
                sCapECorrVLP[clunum].allEner = calcEner;                                    //更新计算到的放电能量
                //计算有效且容量/能量均已修正才更新修正日期
                if((calcCap > 0) && (calcEner > 0)
                    && (0 == BitGet(sCapEnerDropExpFlag[clunum], 1))
                    && (0 == BitGet(sCapEnerDropExpFlag[clunum], 3)))
                {
                    sCapECorrVLP[clunum].date = GetCapCheckNowDate();                       //更新低压时容量检测日期
                }
                SocWriteCorrTotalCapEnerHook(clunum, 0, (u32*)&sCapECorrVLP[clunum].allCap, 3);     //保存低压点放电容量放电能量

                //到低压点后则下一步等待高压，因此需清除高压点,防止再次低压时误修正
                sTime[clunum] = 0;
                sCapECorrVHP[clunum].effect = 0;
                sCapECorrVHP[clunum].timer = 0;
                SocWriteCapEnerCorrInfoHook(clunum, 1, (u32*)&sCapECorrVHP[clunum], 2);             //清除高压点信息
            }
        }
        else
        {
            //记录实时电流温度(放电截止前电流温度)
            sFinCurr[clunum] = GetGSampOutCurrAPI(clunum);
            sFinTemp[clunum] = (GetGCellAvgTempAPI(clunum) + 5) / 10;

            //刚上电
            if((0 == sTime[clunum]) && (sCapECorrVLP[clunum].timer > (CAP_CHECK_RECORD_T * 2)))
            {
                sTime[clunum] = sCapECorrVLP[clunum].timer;
            }
            //每10min记录一次放电时间
            else if(sCapECorrVLP[clunum].timer >= (sTime[clunum] + CAP_CHECK_RECORD_T))
            {
                SocWriteCapEnerCorrTimeHook(clunum, 0, sCapECorrVLP[clunum].timer);
                sTime[clunum] = sCapECorrVLP[clunum].timer;
            }

            //长时间未放电欠压(满放),需放空请求
            if(TRUE == GetCapCheckTimeoutReq(clunum,0))
            {
                //记录放电容量检测超时异常
            // BitSet(sCapEnerDropExpFlag, 5);
            }
            //未达到容量标定时间
            else
            {
                //清除放电容量检测超时
                BitClr(sCapEnerDropExpFlag[clunum], 5);
            }
        }
    //}
}

/*=============================================================================================
** @Name      : static void ClrAllCapEnerCorrInfo(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除所有总容量总能量检测记录信息
** @The notes : 清除EEPROM并复位当前检测值
**===========================================================================================*/
static void ClrAllCapEnerCorrInfo(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    if(sCapECorrVLP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVLP null!!!\r\n", __func__);
        return;
    }
    if(sCapECorrVHP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVHP null!!!\r\n", __func__);
        return;
    }
    //异常标志
    sCapEnerDropExpFlag[clunum] = 0;

    //低压点
    sCapECorrVLP[clunum].effect = 0;
    sCapECorrVLP[clunum].timer = 0;
    sCapECorrVLP[clunum].chgCap = 0;
    sCapECorrVLP[clunum].dhgCap = 0;
    sCapECorrVLP[clunum].allCap = 0;
    sCapECorrVLP[clunum].chgEner = 0;
    sCapECorrVLP[clunum].dhgEner = 0;
    sCapECorrVLP[clunum].allEner = 0;
    sCapECorrVLP[clunum].date = 0;

    //高压点
    sCapECorrVHP[clunum].effect = 0;
    sCapECorrVHP[clunum].timer = 0;
    sCapECorrVHP[clunum].chgCap = 0;
    sCapECorrVHP[clunum].dhgCap = 0;
    sCapECorrVHP[clunum].allCap = 0;
    sCapECorrVHP[clunum].chgEner = 0;
    sCapECorrVHP[clunum].dhgEner = 0;
    sCapECorrVHP[clunum].allEner = 0;
    sCapECorrVHP[clunum].date = 0;

    //初始化EEPROM数据为0
    SocWriteCapEnerCorrInfoHook(clunum, 0, (u32*)&sCapECorrVLP[clunum], 9);
    SocWriteCapEnerCorrInfoHook(clunum, 1, (u32*)&sCapECorrVHP[clunum], 9);
}
/*=============================================================================================
** @Name      : static u8 CheckHisCapEnerClrState(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 0:未清零 1已清零
** @Function  : 检测记录高低压点后历史电量是否清零
** @The notes : 被清零后原记录点无效
**===========================================================================================*/
static u8 CheckHisCapEnerClrState(u8 clunum)
{
    u32 hisChgCap = 0;
    u32 hisDhgCap = 0;
    u32 hisChgEner = 0;
    u32 hisDhgEner = 0;
    if(clunum >= 2)
    {
        return (0);
    }
    if(sCapECorrVLP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVLP null!!!\r\n", __func__);
        return (0);
    }
    if(sCapECorrVHP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVHP null!!!\r\n", __func__);
        return (0);
    }
    //允许记录值误差10AH/10kWh
    hisChgCap = (GetGHisChgCapAPI(clunum) + 10) * 1000;
    hisDhgCap = (GetGHisDhgCapAPI(clunum) + 10) * 1000;
    hisChgEner = (GetGHisChgEnerAPI(clunum) + 100) * 100;
    hisDhgEner = (GetGHisDhgEnerAPI(clunum) + 100) * 100;

    //已记录高压点
    if(1 == sCapECorrVHP[clunum].effect)
    {
        //记录值比累计值小(说明累计值被清零)
        if((sCapECorrVHP[clunum].chgCap > hisChgCap)
            || (sCapECorrVHP[clunum].dhgCap > hisDhgCap)
            || (sCapECorrVHP[clunum].chgEner > hisChgEner)
            || (sCapECorrVHP[clunum].dhgEner > hisDhgEner))
        {
            //历史值已经过清零
            return(1);
        }
    }

    if(1 == sCapECorrVLP[clunum].effect)
    {
        //记录值比累计值小(说明累计值被清零)
        if((sCapECorrVLP[clunum].chgCap > hisChgCap)
            || (sCapECorrVLP[clunum].dhgCap > hisDhgCap)
            || (sCapECorrVLP[clunum].chgEner > hisChgEner)
            || (sCapECorrVLP[clunum].dhgEner > hisDhgEner))
        {
            //历史值已经过清零
            return(1);
        }
    }

    return(0);
}

/*=============================================================================================
** @Name      : static u32 CalcTotalCapEnerByCTCorr(u8 clunum, u8 sort, u32 data, s16 curr, s8 temp)
** @Input     : clunum:簇编号 sort:类型(0:总容量 1:总能量) data:修正前检测到的总容量(1mAH)或总能量(1Wh) curr:标定电流(0.1A) temp:标定温度(1℃)
** @Output    : 电流温度修正后的总容量(1mAH)总能量(1Wh)
** @Function  : 计算总容量或总能量标定值经电流和温度修正后的真实值
** @The notes : 根据总容量标定终止时的电流和温度
**===========================================================================================*/
static u32 CalcTotalCapEnerByCTCorr(u8 clunum, u8 sort, u32 data, s16 curr, s8 temp)
{
    u8 index = 0;
    u16 currPer = 0;
    u32 corrPer = 0;
    u32 aftData = 0;
    const u16 *perTab = NULL;

    if(clunum >= 2)
    {
        return (0);
    }
    /*修正总容量*/
    if(0 == sort)
    {
        /*获取电流修正容量系数表*/
        perTab = cCurrCapPercent;
    }
    /*修正总能量*/
    else if(1 == sort)
    {
        /*获取电流修正能量系数表*/
        perTab = cCurrEnerPercent;
    }
    else
    {
        return(data);
    }

    //根据标称容量计算当前电流倍率(0.1C)
    currPer = (u32)ABS(0, curr) * 1000 / GetGroupStandCapAPI(clunum);

    /*将电流倍率转化为百分率表索引([0.2C,2C]对应0~9)*/
    if(currPer <= (CAPENER_PER_FST_C / 2))
    {
        /*电流很小不修正*/
        corrPer = 1000;
    }
    else if(currPer <= CAPENER_PER_FST_C)
    {
        /*电流在起始点*/
        index = 0;
        corrPer = perTab[index];
    }
    else if(currPer >= ((s8)CAPENER_PER_FST_C + (CAPENER_PER_SUB_C * (CAPENER_C_PER_LEN - 1))))
    {
        index = CAPENER_C_PER_LEN - 1;
        corrPer = perTab[index];
    }
    else
    {
        /*取下端*/
        index = (currPer - CAPENER_PER_FST_C) / CAPENER_PER_SUB_C;
        corrPer = perTab[index] + (((s16)perTab[index + 1] - (s16)perTab[index]) * ((currPer - CAPENER_PER_FST_C) % CAPENER_PER_SUB_C) / CAPENER_PER_SUB_C);
    }

    /*求电流修正后真实值*/
    aftData = data * 1000 / corrPer;

    /*修正总容量*/
    if(0 == sort)
    {
        /*获取温度修正容量系数表*/
        perTab = cTempCapPercent;
    }
    /*修正总能量*/
    else if(1 == sort)
    {
        /*获取温度修正能量系数表*/
        perTab = cTempEnerPercent;
    }
    else
    {
        return(aftData);
    }

    /*将温度转化为百分率表索引([-30,60]对应0~9)*/
    if(temp <= (s8)CAPENER_PER_FST_T)
    {
        index = 0;
        corrPer = perTab[index];
    }
    else if(temp >= ((s8)CAPENER_PER_FST_T + (CAPENER_PER_SUB_T * (CAPENER_T_PER_LEN - 1))))
    {
        index = CAPENER_T_PER_LEN - 1;
        corrPer = perTab[index];
    }
    else
    {
        /*取下端*/
        index = (temp + (0 - (s8)CAPENER_PER_FST_T)) / CAPENER_PER_SUB_T;
        corrPer = perTab[index] + (((s16)perTab[index + 1] - (s16)perTab[index]) * ((temp + (0 - (s8)CAPENER_PER_FST_T)) % CAPENER_PER_SUB_T) / CAPENER_PER_SUB_T);
    }

    /*求增加温度修正后真实值*/
    aftData = aftData * 1000 / corrPer;

    return(aftData);
}

/*=============================================================================================
** @Name      : static u32 GetCapCheckNowDate(void)
** @Input     : void
** @Output    : 当时记录日期
** @Function  : 获取容量检测过欠压当时记录日期
** @The notes : 年月日(如20220610)
**===========================================================================================*/
static u32 GetCapCheckNowDate(void)
{
    return(((u32)GetSysDateClockAPI().year * 10000) + ((u32)GetSysDateClockAPI().month * 100) + GetSysDateClockAPI().day);
}

/*=============================================================================================
** @Name      : static u8 GetCapCheckTimeoutReq(u8 clunum, u8 sort)
** @Input     : clunum:簇编号 sort:容量检测点 0低压点(放空请求) 1高压电(充满请求)
** @Output    : 需满充满放请求
** @Function  : 获取容量检测超时需满充满放请求
** @The notes : 30天未满充满放
**===========================================================================================*/
static u8 GetCapCheckTimeoutReq(u8 clunum, u8 sort)
{
    u16 year = 0;
    u8 month = 0;
    u8 day = 0;
    u8 num = 0;
    if(clunum >= 2)
    {
        return (FALSE);
    }
    if(sCapECorrVLP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVLP null!!!\r\n", __func__);
        return (FALSE);
    }
    if(sCapECorrVHP == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sCapECorrVHP null!!!\r\n", __func__);
        return (FALSE);
    }

    //检测是否需要放空
    if(0 == sort)
    {
        //取上次放电截止日期
        year = sCapECorrVLP[clunum].date / 10000;
        month = (sCapECorrVLP[clunum].date / 100) % 100;
        day = sCapECorrVLP[clunum].date % 100;
    }
    //检测是否需要充满
    else if(1 == sort)
    {
        //取上次充电截止日期
        year = sCapECorrVHP[clunum].date / 10000;
        month = (sCapECorrVHP[clunum].date / 100) % 100;
        day = sCapECorrVHP[clunum].date % 100;
    }
    //参数错误不需要容量标定
    else
    {
        return(FALSE);
    }

    //时间记录合法
    if((year >= 2000) && (year <= 2200) && (month >= 1) && (month <= 12) && (day >= 1) && (day <= 31))
    {
        //相隔至少1年/1个月或本月30天
        if((GetSysDateClockAPI().year >= (year + 2)) || (GetSysDateClockAPI().month >= (month + (CAP_CHECK_CYC_T / 31) + 2)) || (GetSysDateClockAPI().day >= (day + CAP_CHECK_CYC_T)))
        {
            //需要容量标定
            return(TRUE);
        }
        //年月没回头(同年同月)时间有效
        else if((GetSysDateClockAPI().year == year) && (GetSysDateClockAPI().month == month) && (GetSysDateClockAPI().day >= day))
        {
            //计算本月天数
            num = GetSysDateClockAPI().day - day;

            //满足间隔天数
            if(num >= CAP_CHECK_CYC_T)
            {
                //需要容量标定
                return(TRUE);
            }
            //未满足间隔天数
            else
            {
                //不需要容量标定
                return(FALSE);
            }
        }
        //年月没回头(同年月份增大)时间有效
        else if((GetSysDateClockAPI().year == year) && (GetSysDateClockAPI().month >= (month + 1)))
        {
            //计算跨月份天数
            num = (GetSysDateClockAPI().month - month) * 30;

            //日期比之前小(未满1月)
            if(GetSysDateClockAPI().day < day)
            {
                //计算缺少的天数
                num -= (day - GetSysDateClockAPI().day);
            }
            //日期比之前大(满1月)
            else
            {
                //计算多的天数
                num += (GetSysDateClockAPI().day - day);
            }

            //满足间隔天数
            if(num >= CAP_CHECK_CYC_T)
            {
                //需要容量标定
                return(TRUE);
            }
            //未满足间隔天数
            else
            {
                //不需要容量标定
                return(FALSE);
            }
        }
        //跨年(至少跨月:12->1)
        else if(GetSysDateClockAPI().year >= (year + 1))
        {
            //计算跨年天数
            num = (GetSysDateClockAPI().year - year) * 365;

            //月份比之前小(未满1年)
            if(GetSysDateClockAPI().month < month)
            {
                //计算缺少的月份天数
                num -= ((month - GetSysDateClockAPI().month) * 30);
            }
            //月份比之前大(满1年)
            else
            {
                //计算多的月份天数
                num += ((GetSysDateClockAPI().month - month) * 30);
            }

            //日期比之前小(未满1月)
            if(GetSysDateClockAPI().day < day)
            {
                //计算缺少的天数
                num -= (day - GetSysDateClockAPI().day);
            }
            //日期比之前大(满1月)
            else
            {
                //计算多的天数
                num += (GetSysDateClockAPI().day - day);
            }

            //满足间隔天数
            if(num >= CAP_CHECK_CYC_T)
            {
                //需要容量标定
                return(TRUE);
            }
            //未满足间隔天数
            else
            {
                //不需要容量标定
                return(FALSE);
            }
        }
        //日期无效
        else
        {
            //需要容量标定
            return(TRUE);
        }
    }

    return(TRUE);
}