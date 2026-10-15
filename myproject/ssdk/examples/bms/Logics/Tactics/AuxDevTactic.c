/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : AuxDevTactic.c
** @Founder      : ZQD808
** @Date         : 2026.03.19 / 2026.08.10
** @Function     : 辅助设备控制策略模块
** @Instructions : A2509项目正常模式EU3外设自动控制
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "AuxDevTactic.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "BSPUserAPI.h"
#include "DiagCheck.h"
#include "TacInterface.h"
#include "IMEU3Interface.h"
#include "AuxDevMsg.h"
#include "TacUser.h"
#include "FireTactic.h"
#include "GroupMode.h"
#include "CLI.h"
#include <stdio.h>
#include "DiagFilter.h"          /*DiagSetErrNowStateAPI(2026-09-08 SPD/气体告警接入)*/
#include <string.h>

extern u16 *gTFAirUserParaInfo_212;          /*TF液冷空调用户参数信息*/
extern u16 *gTFAirStateInfo_213;             /*TF液冷空调状态信息*/
extern u16 *gTFAirErrInfo_214;               /*TF液冷空调故障信息*/
extern u8  gGHardPara_104[];                 /*主控硬件参数配置信息*/
extern u16 *gSysCtrlCmd_65;                  /*组1系统控制命令*/
extern u16 *gImeu3AlmInfo;                   /*IMEU3告警信息*/
extern u8  *gImeu3BoardInfo1;                /*IMEU3板级信息1*/
extern u16 *gImeu3BoardInfo2;                /*IMEU3板级信息2*/
extern u16 *gSampCalFbInfo_73;               /*采样校准反馈信息*/
extern u16 *gImeu3Info_77;                   /*采样+故障汇总(ICC mIndex=77)*/
extern u16 *gImeu3CfgPara;                   /*常规参数(CANFD mIndex=102)*/
extern u8  *gImeu3CfgRO_75;                  /*回读参数缓存(ICC mIndex=75)*/
extern u8   gEU3CfgReadbackFlag;             /*下发后回读标志*/
extern gImiu4_data *gImiu4_Info[e_IMIU4_NUM]; 
static s16 sAvgCurr1min = 0;                 /*1分钟平均电流 0.1A*/

/*=============================================================================================
** @Data type definition
**===========================================================================================*/

/*=============================================================================================
** @Static function declaration
**===========================================================================================*/


/*=============================================================================================
** @Name      : static void TacExplosionFanCtrlTask(void)
** @Input     : void
** @Output    : void
** @Function  : 泄爆阀排气扇4步状态机控制(Step0决策/Step1发命令/Step2回检/Step3维护PWM)
** @The notes : 参考S32K146 TacShutFanCtrlTask; 在TacAuxDevManagTask中调用
**=============================================================================================*/
static void TacExplosionFanCtrlTask(void);
/*=============================================================================================
** @Name      : static void TacCabinetFanCtrlTask(void)
** @Input     : void
** @Output    : void
** @Function  : 交流柜PDU散热风扇PID滞回控制(读取NTC6温度,10点滑动平均,驱动PWM2)
** @The notes : 参考S32K146 TacPduPcsFanCtrlTask; 在TacAuxDevManagTask中调用
**=============================================================================================*/
static void TacCabinetFanCtrlTask(void);
/*=============================================================================================
** @Name      : static void TacDehumidifierTask(void)
** @Input     : void
** @Output    : void
** @Function  : 除湿机反馈对比控制(读取IMEU3当前值与EEPROM配置对比,不一致时下发更新)
** @The notes : 参考S32K146 TacHumManageTask; 在TacAuxDevManagTask中调用
**=============================================================================================*/
static void TacDehumidifierTask(void);
/*=============================================================================================
** @Name      : static u8 GetAuxFireLevel(void)
** @Input     : void
** @Output    : 火警等级[0,4] 0:正常 1:一级 2:二级 3:三级 4:四级
** @Function  : 获取当前最高火警等级
** @The notes : 遍历簇取TacGetFireLevelAPI最大值; S32K146用GetFireLevelInfoAPI
**=============================================================================================*/
static u8 GetAuxFireLevel(void);
/*=============================================================================================
** @Name      : static void TacImeu3InfoSync(void)
** @Input     : void
** @Output    : void
** @Function  : IMEU3采样/告警直拷到ICC mIndex=77汇总数组
** @The notes : sub0-27=采样一(u8→u16), sub28-49=采样二, sub50-52=告警
**=============================================================================================*/
static void TacImeu3InfoSync(void);
/*=============================================================================================
** @Name      : static void TacImeu3CfgSync(void)
** @Input     : void
** @Output    : void
** @Function  : IMEU3常规参数映射到ICC mIndex=75回读缓存
** @The notes : gImeu3CfgPara(u16×13, CANFD102) → gImeu3CfgRO_75(u8×12, ICC75), 子索引+1偏移丢弃配置标志
**=============================================================================================*/
static void TacImeu3CfgSync(void);

/*=============================================================================================
** @Local variable definition
**===========================================================================================*/

/*=============================================================================================
** @Name      : gAuxShutUserCmd / gExplosionFanDuty / gAuxZHSAForceCmd
** @Input     : 由A核通过ICC或调试模式写入
** @Output    : 泄爆阀手动命令 / 排气扇PWM占空比 / 声光报警强制命令
** @Function  : A2509项目EU3外设用户控制变量
**===========================================================================================*/
static u8  gAuxShutUserCmd     = 0;          /*泄爆阀手动命令 0:关闭 1:打开*/
static u8  gExplosionFanDuty   = 50;         /*泄爆阀排气扇PWM占空比[0,100]1%, 默认50%*/
static u8  sExpValveStuck   = 0;          /*泄爆阀卡死标志 bit0:开到位回检失败 bit1:关到位回检失败(5次重试置位,回检通过清除)*/

/*=============================================================================================
** @Local variable definition
**===========================================================================================*/
#define CABINET_FAN_SAMPLE_NUM   (10)        /*PDU温度滑动采样窗口点数*/

/*=============================================================================================
** @Name      : static u8 GetAuxFireLevel(void)
** @Input     : void
** @Output    : 火警等级[0,4] 0:正常 1:一级 2:二级 3:三级 4:四级
** @Function  : 获取当前最高火警等级
** @The notes : 对接FireTactic.c TacGetFireLevelAPI(FireReportDiagTask同源状态)
**===========================================================================================*/
static u8 GetAuxFireLevel(void)
{
    u8 clunum;
    u8 level;
    u8 maxLevel = (u8)eFireLevel_None;

    for(clunum = 0; clunum < CLUTER_MAX_NUM; clunum++)
    {
        level = TacGetFireLevelAPI(clunum);
        if(level > maxLevel)
        {
            maxLevel = level;
        }
    }
    return(maxLevel);
}

/*=============================================================================================
** @Name      : static void TacImeu3InfoSync(void)
** @Input     : void
** @Output    : void
** @Function  : IMEU3采样/告警直拷到ICC mIndex=77汇总数组
** @The notes : ICC77布局: sub0-27=采样一(u8零扩展u16), sub28-49=采样二(前22项),
**              sub50-52=告警; 挂TacAuxDevManagTask 500ms周期调用
**=============================================================================================*/
static void TacImeu3InfoSync(void)
{
    u8 i;
    if(gImeu3Info_77 == NULL)
    {
        return;
    }
    for(i = 0; i < eIMEUBoardInfo1_Len; i++)
    {
        gImeu3Info_77[i] = (u16)gImeu3BoardInfo1[i];
    }
    for(i = 0; i < 22; i++)                                   /* 采样二有效22项(sub0-21), 跳过软件版本/预留 */
    {
        gImeu3Info_77[28 + i] = gImeu3BoardInfo2[i];          /* 28 = ICC77采样二起始子索引 */
    }
    for(i = 0; i < eIMEUAlmInfo_Len; i++)
    {
        gImeu3Info_77[50 + i] = gImeu3AlmInfo[i];             /* 50 = ICC77告警起始子索引 */
    }
}

/*=============================================================================================
** @Name      : static void TacImeu3CfgSync(void)
** @Input     : void
** @Output    : void
** @Function  : IMEU3常规参数映射到ICC mIndex=75回读缓存
** @The notes : gImeu3CfgPara(u16×13, CANFD102) → gImeu3CfgRO_75(u8×12, ICC75),
**              子索引+1偏移(丢弃sub0配置标志); 配置值≤63无截断损失; 500ms周期调用
**=============================================================================================*/
static void TacImeu3CfgSync(void)
{
    u8 i;
    if(gImeu3CfgRO_75 == NULL || gImeu3CfgPara == NULL)
    {
        return;
    }
    for(i = 0; i < GINDEX75_LEN; i++)
    {
        gImeu3CfgRO_75[i] = (u8)gImeu3CfgPara[i + 1];         /* +1偏移: 跳过CANFD102 sub0配置标志 */
    }
}

extern u16 gGBmuHigLevPara_103[];                           /*主控高级参数配置信息(簇数,2026-09-08 SPD/气体告警接入)*/

/*=============================================================================================
** @Name      : static void TacImeu3FaultJudgeTask(void)
** @Input     : void
** @Output    : void
** @Function : IMEU3外设告警判定: 防雷器DI/可燃气体/除湿机失联/泄爆阀卡死(1s周期,TacAuxDevManagTask调用)
** @The notes : 2026-09-08接入(此前SPD_EXP/CGAS_EXP无setter恒0);
**   防雷器: 主回路DI2(Spd1)/支路DI6(Spd2),常闭开关,断开(高电平)告警;极性如与整机相反,反转spdErr即可;
**   可燃气体: H2/CO原码(ppm),口径2500ppm,与消防L3共用FIRE_L3_*_TH宏(2026-09-08统一口径);
**   IMEU3通信中断时数据无效,清告警(参考100.37离线清告警模式);
**   去抖由gErrFilPara[42]/[163]滤波完成(SPD 3s/气体3s确认)。
**   除湿机失联: IMEU3告警字1 bit3→DE_CON(50),滤波[50]3s/5s(2026-09-08接入);
**   泄爆阀卡死: 开/关到位回检5次重试失败→EXP_EXP(55),滤波[55]3s/5s,回检通过自清(2026-09-08接入);
**   外设失联三连(2026-09-14接入,09-15修正): 告警字1 bit0可燃气体→COM_CON(51)/bit1温湿度→TH_CON(49)/
**   bit0|bit1聚合→THC_OFF(41,掩码0x0003;bit2多一体气体实属LU1单元,836下恒0不聚合);
**   灯带IMLU_CON(47)曾误接bit5,硬件追溯灯带=LU1(IMLU2)/A核控制灯带,已回退(见枚举注释);
**   1#IMEU3通信中断→EU31_CON(44,2026-09-15接入): Imeu3En bit0门控+断线状态直取,滤波[44]3s/5s;
**   水浸异常→FLOOD_ERR(162,2026-09-15接入): 频率(0,300)Hz置位/≥400Hz复位(100Hz滞回),INIT期不检,滤波[162]3s/10s;
**   NTC温度告警(2026-09-14接入): 51表u8=℃+50偏移码,53表Alm2/3短路/断路位+0/255值哨兵双保险屏蔽;
**   PDU_TH2/1←NTC6(交流柜PDU环境),CAB_TH2/1←NTC8(交流功率线缆),一级越限级联二级;
**   COMP_TH1←舱1max(NTC1/2/3),COMP_TH2←舱2max(NTC4/5),共用CompTH,探头故障剔除/全故障恒清;
**   PCS_TH(65/66)836无出风口通道定性废弃,COMP_TH3(69)仅两舱保留不置位(见枚举注释);
**===========================================================================================*/
/*=============================================================================================
** @Name      : static u8 TacNtcChValid(u16 almShort, u16 almOpen, u8 bitPos, u8 rawVal)
** @Input     : almShort/almOpen: 53表告警字2/3; bitPos: 通道故障位; rawVal: 51表原始字节
** @Output    : 1=通道有效 0=故障
** @Function  : NTC通道有效性判定(告警位+值哨兵双保险)
** @The notes : CANFD 53表bit(n-1)连续布局(第8路=bit7,0912协议定稿);0/255值哨兵(51表明文)为
**              采样值域安全兑底而非经典协议兼容层,IMEU3须以CANFD 51/52/53表上报接入本BMU(2026-09-14)
**===========================================================================================*/
static u8 TacNtcChValid(u16 almShort, u16 almOpen, u8 bitPos, u8 rawVal)
{
    if((0 != BitGet(almShort, bitPos)) || (0 != BitGet(almOpen, bitPos)))
    {
        return 0;
    }
    if((0 == rawVal) || (255 == rawVal))
    {
        return 0;
    }
    return 1;
}

static void TacImeu3FaultJudgeTask(void)
{
    u16 diState;
    u16 h2Raw;
    u16 coRaw;
    u16 floodHz;
    u8 spdErr = 0;
    u8 gasErr = 0;
    u8  deErr  = 0;
    u8  expErr = 0;
    u8  thcErr = 0;
    u8  comErr = 0;
    u8  thErr  = 0;
    u8  eu3Err = 0;
    u8  fireErr = 0;                            /*消防(气溶胶)状态*/
    u8  fanErr  = 0;                            /*交流舱风机聚合状态*/
    static u8 sFloodErr = 0;                          /*水浸滞回状态(300~400Hz保持区)*/
    u16 ntcAlm2;                              /*53表告警字2:bit(n-1)=第n路NTC短路*/
    u16 ntcAlm3;                              /*53表告警字3:bit(n-1)=第n路NTC断路*/
    s16 ntcT;                                 /*NTC解码暂存(℃+50偏移域→℃)*/
    s16 pduEnvT = -50;                        /*NTC6交流柜PDU环境温度℃(探头故障保持-50不越限)*/
    s16 acCabT  = -50;                        /*NTC8交流功率线缆温度℃*/
    s16 cab1T   = -50;                        /*舱1(NTC1/2/3)最高温℃*/
    s16 cab2T   = -50;                        /*舱2(NTC4/5)最高温℃*/
    u8  pduTh1Err = 0;
    u8  pduTh2Err = 0;
    u8  cabTh1Err = 0;
    u8  cabTh2Err = 0;
    u8  comp1Err   = 0;
    u8  comp2Err   = 0;
    u8 clunum;

    /*上电3s等待IMEU3首帧数据*/
    if(BSPGetOSMillSecTimeAPI() < 3000)
    {
        return;
    }

    /*1#IMEU3通信中断: Imeu3En bit0门控+断线状态直取(不使能/在线恒清;DiagCheck超时置位,上电3s双保护)*/
    if(((gGHardPara_104[eGHardPara104_Imeu3En] & 0x01) != 0) && (0 != DiagGetInImeu3OffCommuStateAPI()))
    {
        eu3Err = 1;
    }

    /*Imeu3En bit0未使能: 不评估在线数据,17项告警全部按清除处理(含水浸锁存)——
      显式禁用=设备不存在,恒清;与"断线保持水浸"的安全语义区分(2026-09-18审查修复)*/
    if((gGHardPara_104[eGHardPara104_Imeu3En] & 0x01) == 0)
    {
        sFloodErr = 0;
    }

    if((0 == DiagGetInImeu3OffCommuStateAPI())
       && (0 != (gGHardPara_104[eGHardPara104_Imeu3En] & 0x01)))
    {
        diState = GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI1_16);
        h2Raw   = GetImeu3BoardInfo2API(eIMEUBoardInfo2_H2Raw);
        coRaw   = GetImeu3BoardInfo2API(eIMEUBoardInfo2_CORaw);

        /*防雷器常闭: 断开(高电平)告警,主回路/支路任一*/
        if((0 != BitGet(diState, eIMEUBoardInfoDI_Spd1)) || (0 != BitGet(diState, eIMEUBoardInfoDI_Spd2)))
        {
            spdErr = 1;
        }

        /*可燃气体: H2/CO任一达到阈值(2500ppm,与消防L3同源)*/
        if((h2Raw >= FIRE_L3_H2_TH) || (coRaw >= FIRE_L3_CO_TH))
        {
            gasErr = 1;
        }

        /*除湿机失联: IMEU3告警字1 bit3(除湿机挂IMEU3的RS485,失联由IMEU3自检上报)*/
        if(0 != BitGet(GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm1), 3))
        {
            deErr = 1;
        }

        /*泄爆阀卡死: 开/关到位回检5次重试均失败(仅IMEU3在线置位防误报,回检通过自动清除)*/
        if(sExpValveStuck != 0)
        {
            expErr = 1;
        }

        /*配电板外设通信中断: 告警字1 bit0可燃气体|bit1温湿度聚合(枚举注释同源)*/
        /*bit2多一体气体实属LU1(IMLU2)单元归A核,IMEU3在836下恒0,不参与聚合(2026-09-15收窄)*/
        if(0 != (GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm1) & 0x0003))
        {
            thcErr = 1;
        }

        /*可燃气体传感器失联: IMEU3告警字1 bit0(传感器挂IMEU3,失联由IMEU3自检上报)*/
        if(0 != BitGet(GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm1), 0))
        {
            comErr = 1;
        }

        /*温湿度传感器失联: IMEU3告警字1 bit1(485温湿度回路,失联由IMEU3自检上报)*/
        if(0 != BitGet(GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm1), 1))
        {
            thErr = 1;
        }

        floodHz = GetImeu3BoardInfo2API(eIMEUBoardInfo2_WaterFreq);

        /*水浸异常(滞回): 进水频率跌落(0,300)Hz置位,≥ 400Hz复位,300~400保持上周期防抖;
        WP1电气舱/WP2电池舱双开关由IMEU3合并为单频上报(52表);INIT期频率不可信不置位(取1簇状态);断线时保持*/
        if(((floodHz > 0) && (floodHz < BMU_EU_FLOOD_HZ)) && (eWORK_INIT != GetGWorkStateAPI(0)))
        {
            sFloodErr = 1;
        }
        else if(floodHz >= (BMU_EU_FLOOD_HZ + 100))
        {
            sFloodErr = 0;
        }

        /* TODO: DI极性图纸未标明,待上板实测,如极性反转则更改判定条件 */

        /*消防系统告警: FFD1/FFD2气溶胶常闭接点入EU1 DI5(DI1-7常闭组,同防雷器约定,断开高电平报警),
        IMEU3 DI1_16 bit4;信号仅持续30s(滤波[195]用7s置位快速配合)*/
        if(0 != BitGet(diState, eIMEUBoardInfoDI_Ffd))
        {
            fireErr = 1;
        }

        /*交流舱风机回检告警: 1FAN1/1FAN2/1FAN3三台RDb转速回检聚合入EU1 DI21, IMEU3 DI17_22 bit4*/
        if(0 != BitGet(GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI17_22), 4))
        {
            fanErr = 1;
        }

        /*NTC温度告警(2026-09-14接入,原理图NTC通道映射+CANFD协议51/53表): 上传u8=℃+50偏移码,
        Alm2/3故障位+0/255值哨兵双保险屏蔽;NTC7未焊,NTC9=BMU自温无告警id*/
        ntcAlm2 = GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm2);
        ntcAlm3 = GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm3);

        /*舱1最高温: max(NTC1顶,NTC2左,NTC3右),探头故障剔除,全故障保持-50恒清*/
        if(0 != TacNtcChValid(ntcAlm2, ntcAlm3, 0, GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc1Tem)))
        {
            cab1T = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc1Tem) - 50;
        }
        if(0 != TacNtcChValid(ntcAlm2, ntcAlm3, 1, GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc2Tem)))
        {
            ntcT = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc2Tem) - 50;
            if(ntcT > cab1T)
            {
                cab1T = ntcT;
            }
        }
        if(0 != TacNtcChValid(ntcAlm2, ntcAlm3, 2, GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc3Tem)))
        {
            ntcT = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc3Tem) - 50;
            if(ntcT > cab1T)
            {
                cab1T = ntcT;
            }
        }

        /*舱2最高温: max(NTC4右,NTC5顶)*/
        if(0 != TacNtcChValid(ntcAlm2, ntcAlm3, 3, GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc4Tem)))
        {
            cab2T = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc4Tem) - 50;
        }
        if(0 != TacNtcChValid(ntcAlm2, ntcAlm3, 4, GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc5Tem)))
        {
            ntcT = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc5Tem) - 50;
            if(ntcT > cab2T)
            {
                cab2T = ntcT;
            }
        }

        /*PDU环境温度: NTC6(bit5),一级越限级联二级(老平台语义)*/
        if(0 != TacNtcChValid(ntcAlm2, ntcAlm3, 5, GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc6Tem)))
        {
            pduEnvT = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc6Tem) - 50;
            if(pduEnvT > (s8)gGHardPara_104[eGHardPara104_PduEnvTH1])
            {
                pduTh1Err = 1;
                pduTh2Err = 1;
            }
            else if(pduEnvT > (s8)gGHardPara_104[eGHardPara104_PduEnvTH2])
            {
                pduTh2Err = 1;
            }
        }

        /*交流功率线缆温度: NTC8(bit7,0912协议定稿;0/255值哨兵为采样值域安全兑底)*/
        if(0 != TacNtcChValid(ntcAlm2, ntcAlm3, 7, GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc8Tem)))
        {
            acCabT = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc8Tem) - 50;
            if(acCabT > (s8)gGHardPara_104[eGHardPara104_PduCabTH1])
            {
                cabTh1Err = 1;
                cabTh2Err = 1;
            }
            else if(acCabT > (s8)gGHardPara_104[eGHardPara104_PduCabTH2])
            {
                cabTh2Err = 1;
            }
        }

        if(cab1T > (s8)gGHardPara_104[eGHardPara104_CompTH])
        {
            comp1Err = 1;
        }
        if(cab2T > (s8)gGHardPara_104[eGHardPara104_CompTH])
        {
            comp2Err = 1;
        }

    }


    for(clunum = 0; clunum < (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        DiagSetErrNowStateAPI(clunum, eERR_ID_SPD_EXP, (1 == spdErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_CGAS_EXP, (1 == gasErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_DE_CON, (1 == deErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_EXP_EXP, (1 == expErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_THC_OFF, (1 == thcErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_COM_CON, (1 == comErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_TH_CON, (1 == thErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_EU31_CON, (1 == eu3Err) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_FLOOD_ERR, (1 == sFloodErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_FIRE_EXP, (1 == fireErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_FAN3_WAR, (1 == fanErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_PDU_TH2, (1 == pduTh2Err) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_PDU_TH1, (1 == pduTh1Err) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_CAB_TH2, (1 == cabTh2Err) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_CAB_TH1, (1 == cabTh1Err) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_COMP_TH1, (1 == comp1Err) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_COMP_TH2, (1 == comp2Err) ? eErr_B : eErr_A);
    }
}


/*=============================================================================================
** @Name      : void TacImiu4InfoSync(void)
** @Input     : void
** @Output    : void
** @Function  : IMIU4采样/告警直拷到ICC mIndex=67/68汇总数组(1#/2#IMIU4)
** @The notes : ICC67/68布局(协议03表24项): sub0-16=采样信息17词(电池总压..板子5V),
**              sub17=系统工作模式, sub18-20=告警信息1/2/3, sub21-23=故障信息1/2/3;
**              IMIU4离线时DiagCheck调ClrImiu4AllInfoAPI清零,转发随之为0;
**              挂OSUserHook 500ms任务(不受Imeu3En门控);2026-09-14接入(此前零填充断链)
**===========================================================================================*/
void TacImiu4InfoSync(void)
{
    u8 i;
    u8 iu4num;
    s16 *iccTab[(u8)e_IMIU4_NUM];
    u8 imiu4Index;

    iccTab[0] = gImiu4Info_67;
    iccTab[1] = gImiu4Info2_68;

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }

        if(clunum == 0)
        {
            imiu4Index = e_IMIU4_1;          /* 0 */
        }
        else if(clunum == 1)
        {
            imiu4Index = e_IMIU4_2;          /* 1 */
        }

        if(iccTab[imiu4Index] == NULL)
        {
            continue;
        }

        for(i = 0; i < 17; i++)                                /* 采样信息17词(sub0-16), 枚举尾部Link分流器/PEGND不在ICC表 */
        {
            iccTab[imiu4Index][i] = GetImiu4SampInfoAPI(imiu4Index, (e_IMIUSampInfo)i);
        }

        iccTab[imiu4Index][17] = (s16)GetImiu4WorkModeAPI(imiu4Index); /* sub17=系统工作模式 */

        for(i = 0; i < (u8)eIMIUAlmInfo_Len; i++)              /* 告警1-3/故障1-3(sub18-23) */
        {
            iccTab[imiu4Index][18 + i] = (s16)GetImiu4AlmInfoAPI(imiu4Index, (e_IMIUAlmInfo)i);
        }

        if(gSampCalFbInfo_73 != NULL)
        {
            for(u8 i = 0; i < 3; i++)//一个IU4传3个
            {
                gSampCalFbInfo_73[i + (imiu4Index * 3)] = gImiu4_Info[imiu4Index]->SampCalInfo[i];
            }
        }
    }
}

/*=============================================================================================
** @Name      : void TacAuxDevManagTask(void)
** @Input     : void
** @Output    : void
** @Function  : 辅助板管理任务, 1s周期调度EU3外设自动控制
** @The notes : A2509项目分支, M2501预留; 每个子任务内部自行判断调试模式
**===========================================================================================*/
void TacAuxDevManagTask(void)
{
    static u8 sDehumCount = 0; /*除湿机周期计数器*/

    /*告警判定/清除不受Imeu3En门控: 未使能时17项IMEU3告警恒清,
      防运行时关使能后活动告警锁死到重启(2026-09-18审查修复)*/
    TacImeu3FaultJudgeTask();

    /*IMEU3未使能: 控制类任务(查询/DO/泄爆阀/风扇/除湿)不执行*/
    if((gGHardPara_104[eGHardPara104_Imeu3En]) == 0)
    {
        return;
    }

    /* 上电后首次查询IMEU3设备序列号和常规参数 */
    {
        static u8 sEU3QueryDone = 0;
        if(!sEU3QueryDone && BSPGetOSMillSecTimeAPI() >= 3000)
        {
            if(gGHardPara_104[eGHardPara104_ProjectCode] == 0)
            {
                /*查询EU3设备信息*/
                IMEU3_SendDeviceInfoQuery(IMEU3_DEV_ADDR);
                /*查询EU3单板序列号*/
                IMEU3_SendDevSerialQuery(IMEU3_DEV_ADDR);
                /*查询EU3参数配置*/
                IMEU3_SendCfgParaQuery(IMEU3_DEV_ADDR);
                sEU3QueryDone = 1;
            }
        }
    }

    /*正常态DO5=1(远程急停合闸,EPO回路闭合)+DO4=1(QFP5合闸保持直流辅源供电,预留方案2);
    **消防L4自杀后闭锁停驱动两路,DO4/DO5维持FireTactic置的0防回拉*/
    {
        static u8 sQfp5CutLatch = 0;                            /*L4自杀闭锁(一次性,复位仅随重启);已知差距:RAM锁存重启即清零,BMU复位后保活恢复合闸*/
        if(GetAuxFireLevel() >= eFireLevel_4)
        {
            sQfp5CutLatch = 1;
        }
        if((0 == sQfp5CutLatch) && (eMODE_DEBUG != GetGWorkModeAPI(0)))
        {
            if(gGHardPara_104[eGHardPara104_ProjectCode] == 0)
            {
                SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, FIRE_DO_EPO, 1);     /*DO5=1 远程急停合闸(EPO回路闭合,正常)*/
                SendCanImeu3DoCtrlMsg(IMEU3_DEV_ADDR, 1, FIRE_DO_QFP5, 1);    /*DO4=1 QFP5合闸(辅源供电,预留方案2)*/
            }
        }
    }
	
    /*泄爆阀控制*/
    TacExplosionFanCtrlTask();
    /*PDU风扇控制*/
    TacCabinetFanCtrlTask();

    sDehumCount++;
    if(sDehumCount >= 2)
    {
        sDehumCount = 0;
        /*除湿机控制*/
        TacDehumidifierTask();
    }

    /*ICC mIndex=77 采样故障汇总同步(500ms)*/
    /*告警判定TacImeu3FaultJudgeTask已移至Imeu3En门控之前*/

    TacImeu3InfoSync();

    /*参数回读(下发后查一次CANFD 102 + 映射到ICC 75)*/
    if(gEU3CfgReadbackFlag != 0)
    {
        gEU3CfgReadbackFlag = 0;
        IMEU3_SendCfgParaQuery(IMEU3_DEV_ADDR);
    }
    TacImeu3CfgSync();

    /*系统状态查询(500ms周期)*/
    IMEU3_SendSystemStatusQuery(IMEU3_DEV_ADDR);
}

/*=============================================================================================
** @Name      : static void TacExplosionFanCtrlTask(void)
** @Input     : void
** @Output    : void
** @Function  : 泄爆阀排气扇4步状态机控制
** @The notes : 参考S32K146 TacShutFanCtrlTask;
**   Step0: 决策(手动/火警/关), Step1: 发送MotorCtrl命令,
**   Step2: DI17/DI18回检确认, Step3: 排气扇PWM+维持10s后重新决策
**===========================================================================================*/
static void TacExplosionFanCtrlTask(void)
{
    static u8  sCtrlStep    = 0;             /*泄爆阀状态机步骤[0,3]*/
    static u8  sFanState    = 0;             /*泄爆阀当前状态 0:关闭 1:打开*/
    static u8  sCtrlTimes   = 0;             /*泄爆阀步骤内循环计数器*/

    /*上电3秒内跳过*/
    if(BSPGetOSMillSecTimeAPI() < 3000)
    {
        return;
    }

    /*调试模式跳过自动控制, 由A核通过ICC mIndex=71直接控制*/
    if(eMODE_DEBUG == GetGWorkModeAPI(0))
    {
        return;
    }

    /* A2509项目: 泄爆阀排气扇控制 */
    if(gGHardPara_104[eGHardPara104_ProjectCode] == 0)
    {
        switch(sCtrlStep)
        {
            /*===========================================================
            ** Step0: 判断是否需要开/关泄爆阀(手动命令 || 火警≥3)
            **===========================================================*/
            case 0:
            {
                u8 newState = 0;

                if(gAuxShutUserCmd == 1 || GetAuxFireLevel() >= 3)
                {
                    newState = 1;
                }

                if(newState != sFanState)
                {
                    sFanState = newState;
                }
                sCtrlTimes = 0;               /*每次决策重置计数器*/
                sCtrlStep  = 1;
                break;
            }

            /*===========================================================
            ** Step1: 发送泄爆阀开关命令(Motor subIdx=7/8)
            **===========================================================*/
            case 1:
                SendCanImeu3MotorCtrlMsg(IMEU3_DEV_ADDR, 7, sFanState);
                SendCanImeu3MotorCtrlMsg(IMEU3_DEV_ADDR, 8, sFanState);
                sCtrlStep = 2;
                break;

            /*===========================================================
            ** Step2: 读取DI17/DI18回检
            ** DI=1打开到位, DI=0关闭到位; 异常→Step1重试(最多5次), 正常/超时→Step3
            **===========================================================*/
            case 2:
            {
                u16 diState  = GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI17_22);
                u8  di17     = BitGet(diState, 0);
                u8  di18     = BitGet(diState, 1);
                u8  mismatch = 0;

                if((sFanState == 1 && (di17 == 0 || di18 == 0))
                   || (sFanState == 0 && (di17 == 1 || di18 == 1)))
                {
                    mismatch = 1;
                }

                if(mismatch)
                {
                    if(sCtrlTimes >= 5)
                    {
                        /*5次重试仍失配: 置卡死标志(IMEU3在线才置位,防离线数据清零误报)*/
                        if(DiagGetInImeu3OffCommuStateAPI() == 0)
                        {
                            if(sFanState == 1)
                            {
                                BitSet(sExpValveStuck, 0);   /*开到位回检失败*/
                            }
                            else
                            {
                                BitSet(sExpValveStuck, 1);   /*关到位回检失败*/
                            }
                        }
                        sCtrlTimes = 0;
                        sCtrlStep  = 3;    /*5次超时, 仍然开排气扇排风*/
                    }
                    else
                    {
                        sCtrlTimes++;
                        sCtrlStep  = 1;    /*回Step1重发命令*/
                    }
                }
                else
                {
                    /*回检通过清卡死标志: 须IMEU3在线(离线时BoardInfo2被DiagCheck清零,
                      di17/18全0会假匹配——防真实卡死锁存被误清,2026-09-18审查修复)*/
                    if(DiagGetInImeu3OffCommuStateAPI() == 0)
                    {
                        BitClr(sExpValveStuck, 0);    /*回检通过: 清卡死标志(参考100.37正确即清)*/
                        BitClr(sExpValveStuck, 1);
                    }
                    sCtrlTimes = 0;
                    sCtrlStep  = 3;
                }
                break;
            }

            /*===========================================================
            ** Step3: 阀开→发排气扇PWM, 阀关→停风扇
            ** 维持5轮(10s)后回Step0重新决策
            **===========================================================*/
            case 3:
                if(sFanState == 1)
                {
                    SendCanImeu3PwmCtrlMsg(IMEU3_DEV_ADDR, 0, gExplosionFanDuty);
                    SendCanImeu3PwmCtrlMsg(IMEU3_DEV_ADDR, 3, gExplosionFanDuty);
                }
                else
                {
                    SendCanImeu3PwmCtrlMsg(IMEU3_DEV_ADDR, 0, 0);
                    SendCanImeu3PwmCtrlMsg(IMEU3_DEV_ADDR, 3, 0);
                }

                sCtrlTimes++;
                if(sCtrlTimes >= 5)
                {
                    sCtrlTimes = 0;
                    sCtrlStep  = 0;
                }
                break;

            default:
                sCtrlStep = 0;
                break;
        }
    }
}

/*=============================================================================================
** @Name      : static void TacCabinetFanCtrlTask(void)
** @Input     : void
** @Output    : void
** @Function  : 交流柜PDU散热风扇PID滞回控制
** @The notes : 参考S32K146 TacPduPcsFanCtrlTask;
**   读取NTC6(交流柜PDU内部环境温度 raw-50=℃), 10点滑动平均, PID滞回驱动PWM2
**===========================================================================================*/
static void TacCabinetFanCtrlTask(void)
{
    static s16 sTempBuf[CABINET_FAN_SAMPLE_NUM] = {0}; /*PDU温度滑动窗口缓冲区, 实际℃*/
    static u8  sTempIdx     = 0;              /*滑动窗口当前写入索引[0,9]*/
    static u8  sTempFull    = 0;              /*滑动窗口是否已填满*/
    static u8  sCtrlStep    = 0;              /*风扇状态机步骤[0,2]*/
    static u8  sFanDuty     = 0;              /*风扇PWM占空比[0,100]1%*/
    static u8  sFanStarted  = 0;              /*风扇已启动标志, 滞回判断用*/
    static u8  sStartDelay  = 0;              /*风扇启动延时计数[0,5]*/
    static u8  sStopDelay   = 0;              /*风扇停止延时计数[0,5]*/

    if(BSPGetOSMillSecTimeAPI() < 3000)
    {
        return;
    }

    /*调试模式跳过自动控制, 由A核通过ICC mIndex=71直接控制*/
    if(eMODE_DEBUG == GetGWorkModeAPI(0))
    {
        return;
    }

    /* A2509项目: PDU散热风扇PID控制 */
    if(gGHardPara_104[eGHardPara104_ProjectCode] == 0)
    {
        switch(sCtrlStep)
        {
            /*===========================================================
            ** Step0: 上电后先全速运行一轮, 验证风扇硬件通路正常
            **===========================================================*/
            case 0:
                sFanDuty  = 100;
                sCtrlStep = 2;
                break;

            /*===========================================================
            ** Step1: NTC6温度采集+PID滞回计算(10点滑动平均)
            **===========================================================*/
            case 1:
            {
                s16 temp   = (s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc6Tem) - 50;
                /*u8无符号扩展再减50,防raw≥128负温回绕*/

                sTempBuf[sTempIdx] = temp;
                sTempIdx++;
                if(sTempIdx >= CABINET_FAN_SAMPLE_NUM)
                {
                    sTempIdx  = 0;
                    sTempFull = 1;
                }

                if(sTempFull == 0)
                {
                    sCtrlStep = 2;
                    break;
                }

                s16 avgTemp = 0;
                for(u8 i = 0; i < CABINET_FAN_SAMPLE_NUM; i++)
                {
                    avgTemp += sTempBuf[i];
                }
                avgTemp /= CABINET_FAN_SAMPLE_NUM;

                s8 openT = (s8)gGHardPara_104[eGHardPara104_PduFanOpenT];
                s8 fullT = (s8)gGHardPara_104[eGHardPara104_PduFanFullT];
                s8 diff  = fullT - openT;
                if(diff <= 0) diff = 10;

                /* zone1: avgTemp < openT-7  → 延时停止风扇 */
                if(avgTemp < (openT - 7))
                {
                    sStartDelay = 0;
                    if(sStopDelay < 5)
                    {
                        sStopDelay++;
                    }
                    else
                    {
                        sFanDuty    = 0;
                        sFanStarted = 0;
                    }
                }
                /* zone2: avgTemp >= openT(40℃) → 延时启动+PID, 30%-100%线性 */
                else if(avgTemp >= openT)
                {
                    sStopDelay = 0;
                    if(sStartDelay < 5)
                    {
                        sStartDelay++;
                    }
                    else
                    {
                        s16 duty;
                        duty = 30 + (s16)(70L * (avgTemp - openT) / diff);
                        if(duty > 100) duty = 100;
                        if(duty < 30)  duty = 30;
                        sFanDuty    = (u8)duty;
                        sFanStarted = 1;
                    }
                }
                /* zone3: 滞回区(openT-7~openT) → 已启动维持30%, 未启动停 */
                else
                {
                    sFanDuty = sFanStarted ? 30 : 0;
                }
                sCtrlStep = 2;
                break;
            }

            /*===========================================================
            ** Step2: 发送PWM2占空比到IMEU3, 回到Step1
            **===========================================================*/
            case 2:
                SendCanImeu3PwmCtrlMsg(IMEU3_DEV_ADDR, 1, sFanDuty);
                sCtrlStep = 1;
                break;

            default:
                sCtrlStep = 0;
                break;
        }
        }
}

/*=============================================================================================
** @Name      : static void TacDehumidifierTask(void)
** @Input     : void
** @Output    : void
** @Function  : 除湿机反馈对比控制
** @The notes : 参考S32K146 TacHumManageTask;
**   读取IMEU3当前除湿参数与EEPROM配置值对比, 不一致或手动模式时下发更新;
**   通讯中断或除湿机告警时跳过(Alm1 bit3=除湿机通讯中断)
**===========================================================================================*/
static void TacDehumidifierTask(void)
{
    if(BSPGetOSMillSecTimeAPI() < 3000)
    {
        return;
    }

    if(eMODE_DEBUG == GetGWorkModeAPI(0))
    {
        return;
    }

    /*IMEU3通信中断: 跳过, 避免读到旧反馈值误判*/
    if(DiagGetInImeu3OffCommuStateAPI() != 0)
    {
        return;
    }

    /*除湿机通讯中断(Alm1 bit3): 跳过*/
    if(gImeu3AlmInfo != NULL && BitGet(gImeu3AlmInfo[eIMEUAlmInfo_Alm1], 3))
    {
        return;
    }

    u8 curOpen  = GetImeu3BoardInfo1API(eIMEUBoardInfo1_DeOpenHum);
    u8 curClose = GetImeu3BoardInfo1API(eIMEUBoardInfo1_DeCloseHum);
    u8 curState = GetImeu3BoardInfo1API(eIMEUBoardInfo1_DeState);

    u8 cfgOpen  = (u8)gGHardPara_104[eGHardPara104_AirDeHumOn_H];
    u8 cfgClose = (u8)gGHardPara_104[eGHardPara104_AirDeHumOff_H];

    if(curOpen != cfgOpen || curClose != cfgClose || ((curState & 0x08) != 0))
    {
        SendCanImeu3DeCtrlMsg(IMEU3_DEV_ADDR, cfgOpen, cfgClose, 0xFF);
    }
}


/* 结束 */