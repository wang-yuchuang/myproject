/*==========================================Copyright(c)===========================================
** Shenzhen Infypower Co., Ltd.
** Power conversion Department
** BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : BalanTactic.c
** @Founder      : NJX01203
** @Date         : 2026.03.31
** @Function     : 均衡控制策略模块接口 
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "GroupPara.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "DiagInterface.h"
#include "BalanTactic.h"
#include "GroupTactic.h"
#include "DiagFilter.h"
#include <stdio.h>
#include <string.h>

/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
s32 (*gGBalanSubVolt)[GROUP_MAX_CELVNUM] = NULL;
u64 (*gGPasBalanReqState)[SLAVE_MAX_NUM] = NULL;

extern void *ExtMalloc(size_t xWantedSize);
extern void ExtFree(void *pv);

extern u16 gGBmuGenPara_102[];                              /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];                           /*主控高级参数值*/
extern u8 gGHardPara_104[];                                 /*主控硬件参数配置信息*/

/*=================================================================================================
** @Static function declaration
**===============================================================================================*/

/*=================================================================================================
** @Name      : void BalanTacticInfoInit(void)
** @Input     : void
** @Output    : void
** @Function  : 均衡模块内存初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===============================================================================================*/
void BalanTacticInfoInit(void)
{
    static u8 FirstInitFlag = 0;
    if(FirstInitFlag >= 1)
    {
        return;
    }
    
    gGBalanSubVolt = ExtMalloc(CLUTER_MAX_NUM * sizeof(*gGBalanSubVolt));
    if(gGBalanSubVolt == NULL)
    {
        printf("malloc gGBalanSubVolt fail!!\r\n");
        return;
    }
    else
    {
        printf("malloc gGBalanSubVolt %d success\r\n", CLUTER_MAX_NUM * sizeof(*gGBalanSubVolt));
        memset(gGBalanSubVolt, 0, CLUTER_MAX_NUM * sizeof(*gGBalanSubVolt));
    }
    
    gGPasBalanReqState = ExtMalloc(CLUTER_MAX_NUM * sizeof(*gGPasBalanReqState));
    if(gGPasBalanReqState == NULL)
    {
        printf("malloc gGPasBalanReqState fail!!\r\n");
        ExtFree(gGBalanSubVolt);
        gGBalanSubVolt = NULL;
        return;
    }
    else
    {
        printf("malloc gGPasBalanReqState %d success\r\n", CLUTER_MAX_NUM * sizeof(*gGPasBalanReqState));
        memset(gGPasBalanReqState, 0, CLUTER_MAX_NUM * sizeof(*gGPasBalanReqState));
    }
    
    FirstInitFlag = 1;
}

/*=================================================================================================
** @Static function declaration (internal functions)
**===============================================================================================*/
/*=================================================================================================
** @Name      : static void ClrBalanCheckData(void)
** @Input     : void
** @Output    : void
** @Function  : 清除均衡检测数据
** @The notes : 压差计算值和均衡请求状态
**===============================================================================================*/
static void ClrBalanCheckData(u8 clunum);

/*=================================================================================================
** @Name      : static void ClrBalanSubVolt(void)
** @Input     : void
** @Output    : void
** @Function  : 清除均衡压差计算值
** @The notes :
**===============================================================================================*/
static void ClrBalanSubVolt(u8 clunum);

/*=================================================================================================
** @Name      : static void CalcCellVoltSubInte(void)
** @Input     : void
** @Output    : void
** @Function  : 计算单体压差积分
** @The notes :
**===============================================================================================*/
static void CalcCellVoltSubInte(u8 clunum);

/*=================================================================================================
** @Name      : static void JudgeBalanReqByPasCir(void)
** @Input     : void
** @Output    : void
** @Function  : 判断被动放电均衡电路均衡请求
** @The notes :
**===============================================================================================*/
static void JudgeBalanReqByPasCir(u8 clunum);

/*=================================================================================================
** @Name      : static u8 GetDhgBalanSubVolt(u16 cell)
** @Input     : cell:电池号
** @Output    : 开启放电均衡的压差 1mv
** @Function  : 获取开启放电均衡的压差
** @The notes :
**===============================================================================================*/
static u8 GetDhgBalanSubVolt(u8 clunum, u16 cell);

/*=================================================================================================
** @Interface function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : void GroupVoltBalanTacticTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电压均衡策略任务
** @The notes : 100ms周期调用
**===============================================================================================*/
void GroupVoltBalanTacticTask(void)
{
    static u8 sState[CLUTER_MAX_NUM] = {0};
    static u16 sTimer[CLUTER_MAX_NUM] = {0};
    static u16 sHisMode[CLUTER_MAX_NUM] = {0};
    
    u8 clunum = 0;
    
    if(gGBalanSubVolt == NULL)
    {
        printf("gGBalanSubVolt is null\r\n");
        return;
    }
    if(gGPasBalanReqState == NULL)
    {
        printf("gGPasBalanReqState is null\r\n");
        return;
    }
    
    for(clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        switch(sState[clunum])
        {
            //检测当前均衡条件
            case 0:
                if(eMODE_DEBUG == GetGWorkModeAPI(clunum))                            //调试模式
                {
                    /*刚进入调试模式关闭所有均衡*/
                    if(sHisMode[clunum] != eMODE_DEBUG)
                    {
                        ClrBalanCheckData(clunum);                                    //清除均衡检测数据
                    }
                    sHisMode[clunum] = eMODE_DEBUG;                                   //记录调试模式
                    sState[clunum] = 0;                                               //不需检测均衡
                }
                else if(0 == gGHardPara_104[eGHardPara104_BalTact])                   //禁用均衡
                {
                    ClrBalanCheckData(clunum);                                        //清除均衡检测数据
                    sState[clunum] = 0;                                               //不需检测均衡
                }
                else                                                                  //非调试模式
                {
                    if((eWORK_INIT == GetGWorkStateAPI(clunum))                       //系统初始化
                        || (eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_INT_COFF))       //从控通信断线
                        || (eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_SAMP_VOFF))      //从控采样断线
                        || (eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_SAMP_EXP))       //从控采样失效
                        || (sHisMode[clunum] == eMODE_DEBUG))                         //刚退出调试模式
                    {
                        ClrBalanCheckData(clunum);                                    //清除均衡检测数据
                        sState[clunum] = 0;                                           //不需检测均衡
                    }
                    else
                    {
                        sState[clunum] = 1;                                           //进入均衡检测
                    }
                    sHisMode[clunum] = GetGWorkModeAPI(clunum);
                }
                break;

            //检测当前均衡策略
            case 1:
                if(1 == gGHardPara_104[eGHardPara104_BalTact])                        //压差积分均衡
                {
                    CalcCellVoltSubInte(clunum);                                      //计算单体压差积分
                    sTimer[clunum]++;
                    if((sTimer[clunum] / 5) >= (u16)gGHardPara_104[eGHardPara104_BalCycT] * 6)//积分定时为200ms周期
                    {
                        sTimer[clunum] = 0;
                        sState[clunum] = 2;                                           //进入均衡判断
                    }
                    else
                    {
                        sState[clunum] = 0;                                             
                    }
                }
                else
                {
                    sState[clunum] = 0;                                               //不需检测均衡
                }
                break;

            //检测均衡状态需求
            case 2:
                if((1 == gGHardPara_104[eGHardPara104_BalTact]))
                {
                    if(0 == gGHardPara_104[eGHardPara104_BalCirc])                    //被动放电均衡电路
                    {
                        JudgeBalanReqByPasCir(clunum);                                //判断被动放电均衡电路均衡请求
                    }
                    else if(1 == gGHardPara_104[eGHardPara104_BalCirc])               //单向主动充电+被动放电均衡电路
                    {
                        ;
                    }
                    else                                                              //参数错误
                    {
                        ClrBalanCheckData(clunum);                                    //清除均衡检测数据
                    }
                    sState[clunum] = 3;
                }
                else
                {
                    sState[clunum] = 0;                                               //不需检测均衡
                }
                break;

            //数据复位
            case 3:
                ClrBalanSubVolt(clunum);                                              //清除压差积分值
                sState[clunum] = 0;
                break;

            default:
                sState[clunum] = 0;
                break;
        }
    }
}

/*=================================================================================================
** @Name      : void SetGroupVoltPasBalanReqState(u8 pack, u32 cellState)
** @Input     : pack:从控设备节点号 cellState:从控电压被动均衡请求状态:按bit位表示,每bit代表从控内一个电池(0:关闭均衡 :开启被动均衡)
** @Output    : 电池组内各
** @Function  : 设置电池组内各从控电压被动均衡请求状态
** @The notes : 建议只在调试模式调用,否则会与均衡策略计算值冲突
**===============================================================================================*/
void SetGroupVoltPasBalanReqState(u8 ecu, u64 cellState)
{
    u8 bsuNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
    if (bsuNum == 0) return;
    
    if(gGPasBalanReqState == NULL)
    {
        printf("gGPasBalanReqState is null\r\n");
        return;
    }
    
    u8 clu = ecu / bsuNum;
    u8 slave = ecu % bsuNum;
    
    if(clu < CLUTER_MAX_NUM && slave < SLAVE_MAX_NUM)
    {
        gGPasBalanReqState[clu][slave] = cellState;
    }
}

/*=================================================================================================
** @Name      : u62 GetGroupVoltPasBalanReqState(u8 pack)
** @Input     : pack:从控设备节点号(即Pack序号)
** @Output    : 电池组内各从控电压被动均衡请求状态:按bit位表示,每bit代表从控内一个电池(0:关闭均衡 :开启被动均衡)
** @Function  : 获取电池组内各从控电压被动均衡请求状态
** @The notes : 包括均衡策略计算值和调试值
**===============================================================================================*/
u64 GetGroupVoltPasBalanReqState(u8 absSlave)
{
    u8 bsuNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
    if (bsuNum == 0) return 0;
    
    if(gGPasBalanReqState == NULL)
    {
        printf("gGPasBalanReqState is null\r\n");
        return 0;
    }
    
    u8 clunum = absSlave / bsuNum;
    u8 slave = absSlave % bsuNum;
    
    if(clunum < CLUTER_MAX_NUM && slave < SLAVE_MAX_NUM) {
        return gGPasBalanReqState[clunum][slave];
    }
    return 0;
}

/*=================================================================================================
** @Static function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : static void ClrBalanCheckData(void)
** @Input     : void
** @Output    : void
** @Function  : 清除均衡检测数据
** @The notes : 压差计算值和均衡请求状态
**===============================================================================================*/
static void ClrBalanCheckData(u8 clunum)
{
    u16 i = 0;
    
    if(gGBalanSubVolt == NULL)
    {
        printf("gGBalanSubVolt is null\r\n");
        return;
    }
    if(gGPasBalanReqState == NULL)
    {
        printf("gGPasBalanReqState is null\r\n");
        return;
    }

    for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]; i++)
    {
        gGBalanSubVolt[clunum][i] = 0; 
    }
    for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]; i++)
    {
        gGPasBalanReqState[clunum][i] = 0; 
    }
}

/*=================================================================================================
** @Name      : static void ClrBalanSubVolt(u8 clunum)
** @Input     : clunum: 簇号
** @Output    : void
** @Function  : 清除均衡压差计算值
** @The notes :
**===============================================================================================*/
static void ClrBalanSubVolt(u8 clunum)
{
    u16 i = 0;
    
    if(gGBalanSubVolt == NULL)
    {
        printf("gGBalanSubVolt is null\r\n");
        return;
    }
    
    for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]; i++)
    {
        gGBalanSubVolt[clunum][i] = 0;
    }
}

/*=================================================================================================
** @Name      : static void CalcCellVoltSubInte(void)
** @Input     : void
** @Output    : void
** @Function  : 计算单体压差积分
** @The notes :
**===============================================================================================*/
static void CalcCellVoltSubInte(u8 clunum)
{
    u16 i = 0;
    
    if(gGBalanSubVolt == NULL)
    {
        printf("gGBalanSubVolt is null\r\n");
        return;
    }
    
    for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]; i++)
    {
        if(0 == gGHardPara_104[eGHardPara104_BalCirc]) //仅被动放电均衡电路
        {
            u16 cellV = GetGCellVoltAPI(clunum, i);
            u16 minV = GetGCellMinVoltAPI(clunum);
            u16 avgV = GetGCellAvgVoltAPI(clunum);
            //(之前改的没增加这个条件，导致放电结束后，最小的电压小于3100MV，并且不满足后面任意的条件，导致均衡压差积分为0，因此后面再怎么均衡都不会开启)
            if((minV > 2900) && (eCURR_IDLE == GetGChgDhgStateAPI(clunum)) && (SInfy_Idletime[clunum] >= 7200)) 
            {
                gGBalanSubVolt[clunum][i] += ((s32)cellV - (s32)minV); 
            }
            else 
            {
                //最低电压必须达到最小限制值
                if(minV >= BALAN_DHG_MIN_VOLT)
                {
                    gGBalanSubVolt[clunum][i] += ((s32)cellV - (s32)minV);    
                }
                //平均电压必须达到最小限制值(且有严重高电压的电池)
                else if((minV >= gGBmuGenPara_102[eBmuGenPara102_VL2Lim]) && (avgV >= BALAN_DHG_MIN_VOLT) && (GetGCellMaxVoltAPI(clunum) >= (avgV + ((u16)gGHardPara_104[eGHardPara104_BalDhgV] * 2))))
                {
                    gGBalanSubVolt[clunum][i] += ((s32)cellV - (s32)avgV);     
                }
                //平均电压必须达到最小限制值(且最小值高于低压保护,且压差严重大)
                else if((avgV >= BALAN_DHG_MIN_VOLT) && (minV >= (gGBmuGenPara_102[eBmuGenPara102_VL2Lim] + 80)) && (GetGCellMaxVoltAPI(clunum) >= (minV + ((u16)gGHardPara_104[eGHardPara104_BalDhgV] * 2))))
                {
                    gGBalanSubVolt[clunum][i] += ((s32)cellV - (s32)minV);     
                }
                else
                {
                    gGBalanSubVolt[clunum][i] = 0;
                }
            }

        }
        else //主动均衡
        {
            gGBalanSubVolt[clunum][i] += ((s32)GetGCellVoltAPI(clunum, i) - (s32)GetGCellAvgVoltAPI(clunum));
        }
    }
}

/*=================================================================================================
** @Name      : static void JudgeBalanReqByPasCir(void)
** @Input     : void
** @Output    : void
** @Function  : 判断被动放电均衡电路均衡请求
** @The notes :
**===============================================================================================*/
static void JudgeBalanReqByPasCir(u8 clunum)
{
    u8 slave = 0;
    u8 absSlave = 0;
    u8 slaNum = 0;
    u8 slaCelNum = 0;
    u8 slaCel = 0;
    u16 clusterCel = 0;
    u8 onNum = 0;
    u8 index0 = 0, index1 = 0, temp1 = 0;
    s32 temp0 = 0;
    
    u8 subCell[SLAVE_MAX_CELVNUM] = {0};
    s32 subVolt[SLAVE_MAX_CELVNUM] = {0};
    
    if(gGBalanSubVolt == NULL)
    {
        printf("gGBalanSubVolt is null\r\n");
        return;
    }
    if(gGPasBalanReqState == NULL)
    {
        printf("gGPasBalanReqState is null\r\n");
        return;
    }

    slaNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];  //5
    slaCelNum = (u8)(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]);  // 12*4

    for(slave = 0; slave < slaNum; slave++)
    {
        absSlave = clunum * slaNum + slave; 
        /*均衡过温保护*/
        if((eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_BAL_TH)) && (((GetGCellMaxTempAPI(clunum) + 5) / 10) >= BALAN_PAS_PRO_TEMP))
        {
            gGPasBalanReqState[clunum][slave] = 0; 
        }
        //判断系统在充电过程且最大单体电压≥3400mv，关闭均衡
        else if((GetGCellMaxVoltAPI(clunum) >= BALAN_DHG_MAX_VOLT) && (eCURR_CHG == GetGChgDhgStateAPI(clunum)))
        {
            gGPasBalanReqState[clunum][slave] = 0; 
        }
        else
        {
            onNum = 0;
            u16 slaFstCel = slaCelNum * slave;  
            /*找各从控的单节电池电压累积超过阈值的电池*/
            for(slaCel = 0; slaCel < slaCelNum; slaCel++)
            {
                clusterCel = slaFstCel + slaCel;

                if((gGBalanSubVolt[clunum][clusterCel] > 0)  /*需要放电均衡*/
                    && (ABS(gGBalanSubVolt[clunum][clusterCel], 0) > ((u32)GetDhgBalanSubVolt(clunum, clusterCel)
                    * ((u32)gGHardPara_104[eGHardPara104_BalCycT] * 30))))  /*周期内差值积分达到放电阀值(每分钟积分300次)*/
                {
                    gGPasBalanReqState[clunum][slave] |= (1ULL << slaCel);  /*设置被动放电均衡放电电路开启*/
                    subVolt[onNum] = gGBalanSubVolt[clunum][clusterCel];    /*记录需要开启均衡电池压差和电池号*/
                    subCell[onNum] = slaCel;
                    onNum++;
                  //  printf("[BAL_ON!] Cluster: %d, Pack: %d, Cell: %d, Score: %d\r\n", clunum, slave, slaCel, gGBalanSubVolt[clunum][clusterCel]);
                }
                else
                {
                    gGPasBalanReqState[clunum][slave] &= ~(1ULL << slaCel); 
                }
            }
            /*均衡温度偏高且需开启节数超限(8节)*/
            if((((GetGCellMaxTempAPI(clunum) + 5) / 10) >= BALAN_PAS_LIM_TEMP) && (onNum > BALAN_PAS_LIM_CELL)) 
            {
                gGPasBalanReqState[clunum][slave] = 0;             /*开启电池号清零重新计算*/
                /*由大到小排序*/
                for(index0 = 0; index0 < BALAN_PAS_LIM_CELL; index0++) 
                {
                    for(index1 = index0; index1 < onNum; index1++)
                    {
                        if(subVolt[index0] < subVolt[index1])
                        {
                            /*将最大的放在最前面*/
                            temp0 = subVolt[index0];
                            subVolt[index0] = subVolt[index1];
                            subVolt[index1] = temp0;
                            /*电池号跟随调换*/
                            temp1 = subCell[index0];
                            subCell[index0] = subCell[index1];
                            subCell[index1] = temp1;
                        }
                    }
                    gGPasBalanReqState[clunum][slave] |= (1ULL << subCell[index0]);         /*设置被动放电均衡放电电路开启*/
                    
                }
            }
        }
    }
}
/*=================================================================================================
** @Name      : static u8 GetDhgBalanSubVolt(u16 cell)
** @Input     : cell:电池号
** @Output    : 开启放电均衡的压差 1mv
** @Function  : 获取开启放电均衡的压差
** @The notes :
**===============================================================================================*/
static u8 GetDhgBalanSubVolt(u8 clunum, u16 cell)
{
    u8 subVolt = 0;

    u16 avgVolt = GetGCellAvgVoltAPI(clunum);
    u16 plateauUpper = gGBmuGenPara_102[eBmuGenPara102_CellPlaUpV] - 70;   //3450-70
    u16 plateauLower = gGBmuGenPara_102[eBmuGenPara102_CellPlaDnV] + 50;   //3150+50

    if((eCURR_IDLE == GetGChgDhgStateAPI(clunum)) && (SInfy_Idletime[clunum] >= 7200) && (avgVolt < plateauLower))
    {
        subVolt = Infy_eGHardPara104_BalDhgV_2; 
    }
    else if((avgVolt > plateauUpper) || (avgVolt < plateauLower))
    {
        subVolt = Infy_eGHardPara104_BalDhgV; 
    }
    else if((avgVolt <= plateauUpper) && (avgVolt >= plateauLower))
    {
        subVolt = (u8)((u16)gGHardPara_104[eGHardPara104_BalDhgV] * 2 / 3); 
    }

    return(subVolt);
}
