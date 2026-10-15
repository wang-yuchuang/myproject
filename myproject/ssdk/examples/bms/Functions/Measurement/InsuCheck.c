/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : InsuCheck.c
** @Founder      : 
** @Date         : 2026.06.12
** @Function     : 
** @Instructions : 
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**=============================================================================================*/
#include "BSPTypeDef.h"
#include "IMIU4Protocol.h"
#include "IMIU4Interface.h"
#include "IMIU4Config.h"
#include "DiagInterface.h"          
#include "GroupInfo.h"              
#include "GroupPara.h"              
#include "InsuCheck.h"
#include "TacUser.h"
#include "PublicPara.h"
#include "DiagFilter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CLI.h"
#include "CLI_type.h"
/*=============================================================================================
** @External variable declaration
**=============================================================================================*/
extern u16 gGBmuGenPara_102[];       /* 主控通用参数值（含绝缘阈值） */
extern u16 gGBmuHigLevPara_103[];    /* 主控高级参数值（含检测使能） */
extern gImiu4_data *gImiu4_Info[];   /* IMIU4设备信息数组 */
extern u16 gSysGenParaRO_100[];         /*系统通用设置*/
extern u16 *gUserCtrlCmd_69;
extern u16 *gCtrlCmdInfo_70;
/*=============================================================================================
** @Static variable definition
**   （参考2415BMU InsuCheck.c 的静态变量，简化仅保留绝缘相关）
**=============================================================================================*/
/* 每簇绝缘检测状态（两簇各自独立） */
static e_InsuState sInsuState[INSU_CLUNUM] = {eInsuState_Idle, eInsuState_Idle};

/* 每簇绝缘故障状态 */
static e_InsuErrState sInsuErrState[INSU_CLUNUM] = {eInsuErr_Norm, eInsuErr_Norm};

/* 每簇绝缘检测完成标志 */
static u8 sInsuCheckFinFlag[INSU_CLUNUM] = {0, 0};  // 0未完成，1完成（一轮）

/* 每簇绝缘检测计时器（用于超时判断） */
static u32 sInsuStartTime[INSU_CLUNUM] = {0, 0};

/* 每簇上次检测时间（用于周期判断，参考2415BMU的sHisInsuTime） */
static u32 sLastInsuTime[INSU_CLUNUM] = {0, 0};

/* 每簇正极绝缘值（KΩ） */
static u16 sIsoRp[INSU_CLUNUM] = {INSU_INVAL_VALUE, INSU_INVAL_VALUE};

/* 每簇负极绝缘值（KΩ） */
static u16 sIsoRn[INSU_CLUNUM] = {INSU_INVAL_VALUE, INSU_INVAL_VALUE};

/* 每簇：记录上次判断时的系统工作状态（用于检测状态变更） */
static u16 sSysNeedPreState[INSU_CLUNUM] = {0, 0};

/* 每簇：上次完成检测的时间（用于周期判断） */
static u32 sSysNeedTime[INSU_CLUNUM] = {0, 0};

/* 每簇：检测重试/完成计数器（>0 表示正在检测中） */
static u8  sSysNeedChkTime[INSU_CLUNUM] = {0, 0};

/* 每簇：外部请求检测的时间（用于 30s 超时判断） */
static u32 sSysNeedReqTime[INSU_CLUNUM] = {0, 0};


/*=============================================================================================
** @Name      : static u8 GetImiu4Index(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : IMIU4枚举索引
** @Function  : 将簇编号映射到IMIU4数据结构索引
**=============================================================================================*/
static u8 GetImiu4Index(u8 clunum)
{
    if(clunum == 0)
    {
        return e_IMIU4_1;          /* 0 */
    }
    else if(clunum == 1)
    {
        return e_IMIU4_2;          /* 1 */
    }
    return e_IMIU4_1; 
}


/*=============================================================================================
** @Name      : static u8 SysNeedInsuCheckState(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : TRUE:需要检测  FALSE:不需要检测
** @Function  : 判断系统是否需要启动绝缘检测
** @The notes : 参考2415BMU的SysNeedInsuCheckState()，适配BMU05参数结构
**=============================================================================================*/
  static u8 SysNeedInsuCheckState(u8 clunum)
  {
      u8 imiu4Index;
      u16 almFault1;
      if(gCtrlCmdInfo_70== NULL) return FALSE;
      if(clunum >= INSU_CLUNUM) return FALSE;
      imiu4Index = GetImiu4Index(clunum);

      /* ① 总开关:LeakMonitType==0 禁止 */
      if(0 == gGBmuHigLevPara_103[eBmuHigLevPara103_LeakMonitType])
      {
          /* 清状态、标记完成 */
          sSysNeedChkTime[clunum] = 0;
          sInsuErrState[clunum] = eInsuErr_Norm;
          sInsuCheckFinFlag[clunum] = 1;
          sSysNeedPreState[clunum] = GetGWorkStateAPI(clunum);
          return FALSE;
      }
      /* 注意外部请求时间记录:非请求时刷新 sSysNeedReqTime */
      if(1 != gCtrlCmdInfo_70[clunum])
      {
          sSysNeedReqTime[clunum] = BSPGetOSMillSecTimeAPI();
      }

    /*==============================================================*/
    /* ④ 上电 5 秒内强制首次检测（2415BMU line 1386-1393）         */
    /*    仅单模块时执行此逻辑                                       */
    /*==============================================================*/
    if((0 == sSysNeedTime[clunum])
        && (BSPGetOSMillSecTimeAPI() < INSU_BOOT_FORCE_MS))
        //&& (gSysGenParaRO_100[eSGenPara100_AllGNum] <= 1))
    {
        sSysNeedChkTime[clunum] = 1;
        sSysNeedPreState[clunum]= GetGWorkStateAPI(clunum);
        sSysNeedTime[clunum]    = BSPGetOSMillSecTimeAPI();
        
        printf("frist insucheck 5s, clummn = %d\r\n",clunum);
        return TRUE;
    }


      /* ⑥ 高压采样异常或总压过低 → 不检测(安全保护) */
      if(gImiu4_Info[imiu4Index] != NULL)
      {
          almFault1 = gImiu4_Info[imiu4Index]->AlmFault[e_IMIUAlmInfo_Alm1];
      }
      else
      {
          almFault1 = 0xFFFF;
      }
      if(((almFault1 & (IMIU4_ALM_HIGSAMP_INIT_BIT | IMIU4_ALM_HIGSAMP_COMM_BIT)) != 0)
          || (GetGSampSumVoltAPI(clunum) < (u16)(INSU_MIN_BAT_V * 10)))
      {
          sSysNeedChkTime[clunum] = 0;
          sInsuErrState[clunum] = eInsuErr_Norm;
          sInsuCheckFinFlag[clunum] = 1;
          sSysNeedPreState[clunum] = GetGWorkStateAPI(clunum);
          
         //  printf("vot error, clummn = %d\r\n",clunum);
           return FALSE;
      }

      /* ⑧ 外部请求 65s 超时强制完成 */
      if((1 == gCtrlCmdInfo_70[clunum])
          && ((BSPGetOSMillSecTimeAPI() - sSysNeedReqTime[clunum]) >= INSU_REQ_TIMEOUT_MS))
      {
          /* 清故障标志(若已消除) */
          if((sInsuErrState[clunum] > eInsuErr_Norm)
              && (eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_LEAK2))
              && (eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_LEAK1)))
          {
              sInsuErrState[clunum] = eInsuErr_Norm;
          }
          sSysNeedChkTime[clunum] = 0;
          sInsuCheckFinFlag[clunum] = 1;
          sSysNeedPreState[clunum] = GetGWorkStateAPI(clunum);
          
        //   printf("seed overtime, clummn = %d\r\n",clunum);
           return FALSE;
      }


      /* ⑨ 检测进行中 or 等待执行(核心) */
      if(0 == sInsuCheckFinFlag[clunum])
       {
          printf("external request insucheck, clummn = %d,sInsuCheckFinFlag = %d,sSysNeedTime = %d,gCtrlCmdInfo_70 = %d\r\n",clunum,sInsuCheckFinFlag[clunum],sSysNeedTime[clunum],gCtrlCmdInfo_70[clunum]);
          sSysNeedChkTime[clunum] = 1;
          sSysNeedPreState[clunum] = GetGWorkStateAPI(clunum);
          sSysNeedTime[clunum] = BSPGetOSMillSecTimeAPI();

          return TRUE;
      }

      /* ⑬ 故障态周期重触发(60s) */
      if((sInsuErrState[clunum] > eInsuErr_Norm)
        && ((BSPGetOSMillSecTimeAPI() - sSysNeedTime[clunum]) >= ISO_ERR_DIAGTIME_MS))
    {
         printf("insu alarm fault request insucheck, clummn = %d,sInsuErrState = %d,sSysNeedTime = %d\r\n",clunum,sInsuErrState[clunum],sSysNeedTime[clunum]);
        sSysNeedChkTime[clunum] = 1;
        sSysNeedPreState[clunum] = GetGWorkStateAPI(clunum);
        sSysNeedTime[clunum]    = BSPGetOSMillSecTimeAPI();
          return TRUE;
      }

    /*==============================================================*/
    /* ⑫ 工作状态变更触发（2415BMU line 1624-1633）                */
    /*    状态变为 IDLE 或 ERR（且接触器全断）时触发检测            */
    /*==============================================================*/
    if((sSysNeedPreState[clunum] != GetGWorkStateAPI(clunum))
        && ((eWORK_IDLE == GetGWorkStateAPI(clunum))
        || ((eWORK_ERR == GetGWorkStateAPI(clunum))
        && (1 == TacGetGroupRunSwhAllOffStateAPI(clunum))))
        &&((BSPGetOSMillSecTimeAPI() - sSysNeedTime[clunum]) >= ISO_ERR_DIAGTIME_MS))
    {
        printf("state change request insucheck, clummn = %d,state = %d,sSysNeedPreState = %d,sSysNeedTime = %d\r\n",clunum,GetGWorkStateAPI(clunum),sSysNeedPreState[clunum],sSysNeedTime[clunum]);       
        sSysNeedChkTime[clunum] = 1;
        sSysNeedPreState[clunum]= GetGWorkStateAPI(clunum);
        sSysNeedTime[clunum]    = BSPGetOSMillSecTimeAPI();
        return TRUE;
        
    }

      /* 默认:不检测 */
      sSysNeedChkTime[clunum] = 0;
      sSysNeedPreState[clunum] = GetGWorkStateAPI(clunum);
      return FALSE;
  }

/*=============================================================================================
** @Name      : static u8 GetImiu4Addr(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : IMIU4地址
** @Function  : 将簇编号映射到IMIU4设备地址
** @The notes : 簇0→IMIU4_1(20), 簇1→IMIU4_2(21)
**=============================================================================================*/
static u8 GetImiu4Addr(u8 clunum)
{
    if(clunum == 0)
    {
        return IMIU4_1_DEV_ADDR;   /* 20 */
    }
    else if(clunum == 1)
    {
        return IMIU4_2_DEV_ADDR;   /* 21 */
    }
    return IMIU4_1_DEV_ADDR;
}


/*=============================================================================================
** @Name      : static void InsuErrStateCheck(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : void
** @Function  : 绝缘故障等级判断
** @The notes : 参考2415BMU的InsuErrStateCheck()逻辑
**              BMU05简化：不自行计算阻值，直接使用IMIU4上报的绝缘值
**=============================================================================================*/
static void InsuErrStateCheck(u8 clunum)
{
    u16 isoRp = sIsoRp[clunum];
    u16 isoRn = sIsoRn[clunum];

    /* 总压低于最小检测电压时不告警（参考2415BMU） */
    if(GetGSampSumVoltAPI(clunum) < (u16)(INSU_MIN_BAT_V * 10))
    {
        sInsuErrState[clunum] = eInsuErr_Norm;
        return;
    }

    /* 绝缘值为无效值（0xFFFF表示检测异常，参考2415BMU: 0xffff==isoRp） */
    if((0xFFFF == isoRp) || (0xFFFF == isoRn))
    {
        sInsuErrState[clunum] = eInsuErr_Exp;
    }
    /* 绝缘值低于1级阈值（参考2415BMU: isoRp*1000 < Leak1Lim * SampVbat） */
    else if((((u32)isoRp * 1000) < ((u32)gGBmuGenPara_102[eBmuGenPara102_Leak1Lim] * GetGSampSumVoltAPI(clunum) / 10))
         || (((u32)isoRn * 1000) < ((u32)gGBmuGenPara_102[eBmuGenPara102_Leak1Lim] * GetGSampSumVoltAPI(clunum) / 10)))
    {
        sInsuErrState[clunum] = eInsuErr_Lev1;
    }
    /* 绝缘值低于2级阈值（参考2415BMU: isoRp*1000 < Leak2Lim * SampVbat） */
    else if((((u32)isoRp * 1000) < ((u32)gGBmuGenPara_102[eBmuGenPara102_Leak2Lim] * GetGSampSumVoltAPI(clunum) / 10))
         || (((u32)isoRn * 1000) < ((u32)gGBmuGenPara_102[eBmuGenPara102_Leak2Lim] * GetGSampSumVoltAPI(clunum) / 10)))
    {
        sInsuErrState[clunum] = eInsuErr_Lev2;
    }
    else
    {
        /* 正常 */
        sInsuErrState[clunum] = eInsuErr_Norm;
    }
}

/*=============================================================================================
** @Name      : void InsuCheckInit(void)
** @Input     : void
** @Output    : void
** @Function  : 绝缘检测模块初始化
** @The notes : 参考2415BMU的InsuAdhesMeasurInit()
**              BMU05不需要控制GPIO继电器，仅初始化状态变量
**=============================================================================================*/
void InsuCheckInit(void)
{
    u8 i;
    for(i = 0; i < INSU_CLUNUM; i++)
    {
        sInsuState[i]       = eInsuState_Idle;
        sInsuErrState[i]    = eInsuErr_Norm;
        sInsuCheckFinFlag[i]= 0;
        sInsuStartTime[i]   = 0;
        sLastInsuTime[i]    = 0;
        sSysNeedPreState[i] = 0;
        sSysNeedTime[i]     = 0;
        sSysNeedChkTime[i]  = 0;
        sSysNeedReqTime[i]  = 0;
        sIsoRp[i]           = INSU_INVAL_VALUE;
        sIsoRn[i]           = INSU_INVAL_VALUE;
    }
}

/*=============================================================================================
** @Name      : void InsuCheckTask(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : void
** @Function  : 绝缘检测主任务（每100ms调用一次，各自处理一簇）
** @The notes : 参考2415BMU的InsuAdhesMeasurTask()状态机逻辑
**              简化：仅绝缘检测，无粘连检测
**              状态机: IDLE→START→WAIT→READ→IDLE
**=============================================================================================*/
void InsuCheckTask(void)
{
    u8 imiu4Index;
    u8 imiu4Addr;
    u8 result;
    u16 isoPasValue;
    u16 isoNegValue;
    u8 clunum;
    for(clunum = 0;clunum < INSU_CLUNUM; clunum++)
    {
        imiu4Index = GetImiu4Index(clunum);
        imiu4Addr  = GetImiu4Addr(clunum);

        if(gImiu4_Info[imiu4Index] == NULL)
        {
            return;
        }
        gCtrlCmdInfo_70[clunum] = gImiu4_Info[imiu4Index]->CtrlInfo;

        switch(sInsuState[clunum])
        { 
            /* =========================================== */
            /* IDLE: 空闲，调用 SysNeedInsuCheckState 判断  */
            /* =========================================== */
            case eInsuState_Idle:
            {
                if(TRUE == SysNeedInsuCheckState(clunum))
                {
                    sInsuState[clunum] = eInsuState_Start;
                }
                break;
            }

            /* =========================================== */
            /* START: 下发 IMIU4 启动检测命令               */
            /* =========================================== */
            case eInsuState_Start:
            {
                result = IMIU4_SendInsuCtrl(imiu4Addr, IMIU4_INSU_START);
               // printf("insu send,result = %d,imiu4addr= %d\r\n",result,imiu4Addr);

                if(result != 0)
                {
                    /* 命令发送成功，记录开始时间，进入等待状态 */
                    sInsuStartTime[clunum] = BSPGetOSMillSecTimeAPI();
                    sInsuState[clunum] = eInsuState_Wait;
                }
                else
                {
                    /* 命令发送失败，标记异常，回到 Idle */
                    //sInsuErrState[clunum]    = eInsuErr_Norm;
                    sInsuCheckFinFlag[clunum] = 1;
                   // sLastInsuTime[clunum]     = BSPGetOSMillSecTimeAPI();
                    sInsuState[clunum]        = eInsuState_Idle;
                }
                break;
            }

            /* =========================================== */
            /* WAIT: 等待 IMIU4 检测完成                    */
            /* =========================================== */
            case eInsuState_Wait:
            {
                
            if((BSPGetOSMillSecTimeAPI() - sInsuStartTime[clunum]) >= INSU_WAIT_STABLE_MS) //添加3s延时，IMIU4保持3检测完成不变
            {
                /* CtrlInfo == 3: 检测完成 → 进入 Read 状态 */
                if(3 == gCtrlCmdInfo_70[clunum])
                {
                    sInsuState[clunum] = eInsuState_Read;
                    sSysNeedChkTime[clunum] = 0;
                 //    printf("insu success,gCtrlCmdInfo_70 = %d,clunum= %d\r\n",gCtrlCmdInfo_70[clunum],clunum);
                }
                /* CtrlInfo == 4(超时) or 5(失败): 异常 */
                else if((4 == gCtrlCmdInfo_70[clunum]) || (5 == gCtrlCmdInfo_70[clunum]))
                {
                    sInsuErrState[clunum]    = eInsuErr_Exp;
                    sSysNeedChkTime[clunum] = 0;
                    sInsuCheckFinFlag[clunum] = 1;
                    sLastInsuTime[clunum]     = BSPGetOSMillSecTimeAPI();
                    sInsuState[clunum]        = eInsuState_Idle;
                   // printf("insu fail,gCtrlCmdInfo_70 = %d,clunum= %d\r\n",gCtrlCmdInfo_70[clunum],clunum);
                }
                /* CtrlInfo == 1(检测中) or 0(自动) or 2(停止): 继续等待 */
                else
                {
                    /* 超时保护 */
                    if((BSPGetOSMillSecTimeAPI() - sInsuStartTime[clunum]) >= INSU_CHECK_TIMEOUT_MS)
                    {
                        sSysNeedChkTime[clunum] = 0;
                        sInsuErrState[clunum]    = eInsuErr_Exp;
                        sInsuCheckFinFlag[clunum] = 1;
                        sLastInsuTime[clunum]     = BSPGetOSMillSecTimeAPI();
                        sInsuState[clunum]        = eInsuState_Idle;
                     //   printf("insu overtime,gCtrlCmdInfo_70 = %d,clunum= %d,sInsuStartTime= %d\r\n",gCtrlCmdInfo_70[clunum],clunum,sInsuStartTime[clunum]);
                    }
                }
            }
                
                break;
            }

            /* =========================================== */
            /* READ: 读取绝缘值，判断告警 → 回到 Idle       */
            /* =========================================== */
            case eInsuState_Read:
            {
                /* 读取 IMIU4 上报的正/负极绝缘值 */
                isoPasValue = (u16)gImiu4_Info[imiu4Index]->SampInfo[eIMIUSampInfo_PasInsu];
                isoNegValue = (u16)gImiu4_Info[imiu4Index]->SampInfo[eIMIUSampInfo_NegInsu];

                /* 存储绝缘值 */
                sIsoRp[clunum] = isoPasValue;
                sIsoRn[clunum] = isoNegValue;

                /* 告警等级判断 */
                InsuErrStateCheck(clunum);

                /* 标记检测完成 */
                sInsuCheckFinFlag[clunum] = 1;
                sLastInsuTime[clunum]     = BSPGetOSMillSecTimeAPI();
                sInsuState[clunum]        = eInsuState_Idle;
              //  printf("insu fin,isoPasValue = %d,isoNegValue= %d,clunum= %d\r\n",isoPasValue,isoNegValue,clunum);
                break;
            }

            default:
                break;
        }
    }

}

/*=============================================================================================
** @Name      : u8 InsuGetErrState(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : 绝缘故障状态（0:正常 1:1级 2:2级 3:异常）
** @Function  : 获取绝缘故障状态（供DiagMaker调用，参考2415BMU的DiagGetDcSideLeakFlagAPI）
**=============================================================================================*/
u8 InsuGetErrState(u8 clunum)
{
    if(clunum >= 2)
    {
        return eInsuErr_Exp;
    }
    return sInsuErrState[clunum];
}

/*=============================================================================================
** @Name      : u16 InsuGetPasValue(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : 正极绝缘阻值（KΩ）
**=============================================================================================*/
u16 InsuGetPasValue(u8 clunum)
{
    if(clunum >= 2)
    {
        return INSU_INVAL_VALUE;
    }
    return sIsoRp[clunum];
}

/*=============================================================================================
** @Name      : u16 InsuGetNegValue(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : 负极绝缘阻值（KΩ）
**=============================================================================================*/
u16 InsuGetNegValue(u8 clunum)
{
    if(clunum >= 2)
    {
        return INSU_INVAL_VALUE;
    }
    return sIsoRn[clunum];
}

/*=============================================================================================
** @Name      : u8 InsuGetCheckFinFlag(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : 0:检测中  1:已完成
** @Function  : 获取绝缘检测完成标志
**===========================================================================================*/
u8 InsuGetCheckFinFlag(u8 clunum)
{
    if(clunum >= INSU_CLUNUM)
    {
        return 1;
    }
    return sInsuCheckFinFlag[clunum];
}

/*=============================================================================================
** @Name      : u8 InsuGetDiagCount(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : 当前检测重试计数
** @Function  : 获取绝缘检测重试计数器（>0 表示正在检测中）
**===========================================================================================*/
u8 InsuGetDiagCount(u8 clunum)
{
    if(clunum >= INSU_CLUNUM)
    {
        return 0;
    }
    return sSysNeedChkTime[clunum];
}

/*=============================================================================================
** @Name      : void UserReqInsuCheckAPI(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : void
** @Function  : 用户/外部请求开始绝缘检测
**              （对照 2415BMU InsuCheck.c:917-921 UserReqInsuCheckAPI）
** @The notes : 将完成标志清零，强制 SysNeedInsuCheckState 下一轮返回 TRUE
**              调用方：TacUser.c:525（用户远程控制）
**                     TacInterface.c:903（单组系统自动触发，当前注释待取消）
**===========================================================================================*/
void UserReqInsuCheckAPI(u8 clunum)
{
    if(clunum >= INSU_CLUNUM)
    {
        return;
    }
    /* 清除绝缘检测完成标志，强制下一轮检测 */
    sInsuCheckFinFlag[clunum] = 0;
}

/*=============================================================================================
** @Name      : void ClrInsuAdhesErrFlagAPI(u8 clunum)
** @Input     : clunum: 簇编号(0/1)
** @Output    : void
** @Function  : 清除绝缘故障标志
**              （对照 2415BMU InsuCheck.c:980-984 ClrInsuAdhesErrFlagAPI）
** @The notes : 2415BMU 含粘连故障清除，BMU05 仅清除绝缘故障状态
**              调用方：TacUser.c:234/358/372/496（清除故障命令处理）
**                     TacInterface.c:937（进入特殊模式清除故障）
**===========================================================================================*/
void ClrInsuAdhesErrFlagAPI(u8 clunum)
{
    if(clunum >= INSU_CLUNUM)
    {
        return;
    }
    sInsuErrState[clunum] = eInsuErr_Norm;
}

#if(1)
//如下是为了验证GPIO功能，使用CLI命令验证，调试时可以打开。
static int devinsu_test(int argc, char *argv[])
{
        if (argc == 0) 
        {
            printf("devginsu test para error\n");
            return CLI_FALSE;
        }
        if(!strcmp(argv[0], "insutest0")) 
        {
           // UserReqInsuCheckAPI(0);

            printf(" sInsuCheckFinFlag[0] = %d\r\n", sInsuCheckFinFlag[0]);
            printf(" sInsuState[0] = %d\r\n", sInsuState[0]);
            printf(" sInsuErrState[0] = %d\r\n", sInsuErrState[0]);
            printf(" sInsuStartTime[0] = %d\r\n", sInsuStartTime[0]);
            printf(" sLastInsuTime[0] = %d\r\n", sLastInsuTime[0]);
            printf(" sIsoRp[0] = %d\r\n", sIsoRp[0]);
            printf(" sIsoRn[0] = %d\r\n", sIsoRn[0]);
            printf(" sSysNeedPreState[0] = %d\r\n", sSysNeedPreState[0]);
            printf(" sSysNeedTime[0] = %d\r\n", sSysNeedTime[0]);
            printf(" sSysNeedChkTime[0] = %d\r\n", sSysNeedChkTime[0]);
            printf(" sSysNeedReqTime[0] = %d\r\n", sSysNeedReqTime[0]);
            printf(" gUserCtrlCmd_69[0] = %d\r\n", gUserCtrlCmd_69[0]);
            printf(" gCtrlCmdInfo_70[0] = %d\r\n", gCtrlCmdInfo_70[0]);
        }
        else if(!strcmp(argv[0], "insutest1")) 
        {
           // UserReqInsuCheckAPI(1);

            printf(" sInsuCheckFinFlag[1] = %d\r\n", sInsuCheckFinFlag[1]);
            printf(" sInsuState[1] = %d\r\n", sInsuState[1]);
            printf(" sInsuErrState[1] = %d\r\n", sInsuErrState[1]);
            printf(" sInsuStartTime[1] = %d\r\n", sInsuStartTime[1]);
            printf(" sLastInsuTime[1] = %d\r\n", sLastInsuTime[1]);
            printf(" sIsoRp[1] = %d\r\n", sIsoRp[1]);
            printf(" sIsoRn[1] = %d\r\n", sIsoRn[1]);
            printf(" sSysNeedPreState[1] = %d\r\n", sSysNeedPreState[1]);
            printf(" sSysNeedTime[1] = %d\r\n", sSysNeedTime[1]);
            printf(" sSysNeedChkTime[1] = %d\r\n", sSysNeedChkTime[1]);
            printf(" sSysNeedReqTime[1] = %d\r\n", sSysNeedReqTime[1]);
            printf(" gUserCtrlCmd_69[1] = %d\r\n", gUserCtrlCmd_69[1]);
            printf(" gCtrlCmdInfo_70[1] = %d\r\n", gCtrlCmdInfo_70[1]);
        }
        if(!strcmp(argv[0], "insutest2")) 
        {
            UserReqInsuCheckAPI(0);

            printf(" sInsuCheckFinFlag[0] = %d\r\n", sInsuCheckFinFlag[0]);
             printf(" sInsuState[0] = %d\r\n", sInsuState[0]);
            printf(" sInsuErrState[0] = %d\r\n", sInsuErrState[0]);
            printf(" sInsuStartTime[0] = %d\r\n", sInsuStartTime[0]);
            printf(" sLastInsuTime[0] = %d\r\n", sLastInsuTime[0]);
            printf(" sIsoRp[0] = %d\r\n", sIsoRp[0]);
            printf(" sIsoRn[0] = %d\r\n", sIsoRn[0]);
            printf(" sSysNeedPreState[0] = %d\r\n", sSysNeedPreState[0]);
            printf(" sSysNeedTime[0] = %d\r\n", sSysNeedTime[0]);
            printf(" sSysNeedChkTime[0] = %d\r\n", sSysNeedChkTime[0]);
            printf(" sSysNeedReqTime[0] = %d\r\n", sSysNeedReqTime[0]);
            printf(" gUserCtrlCmd_69[0] = %d\r\n", gUserCtrlCmd_69[0]);
            printf(" gCtrlCmdInfo_70[0] = %d\r\n", gCtrlCmdInfo_70[0]);
        }
        else if(!strcmp(argv[0], "insutest3")) 
        {
            UserReqInsuCheckAPI(1);

            printf(" sInsuCheckFinFlag[1] = %d\r\n", sInsuCheckFinFlag[1]);
            printf(" sInsuState[1] = %d\r\n", sInsuState[1]);
            printf(" sInsuErrState[1] = %d\r\n", sInsuErrState[1]);
            printf(" sInsuStartTime[1] = %d\r\n", sInsuStartTime[1]);
            printf(" sLastInsuTime[1] = %d\r\n", sLastInsuTime[1]);
            printf(" sIsoRp[1] = %d\r\n", sIsoRp[1]);
            printf(" sIsoRn[1] = %d\r\n", sIsoRn[1]);
            printf(" sSysNeedPreState[1] = %d\r\n", sSysNeedPreState[1]);
            printf(" sSysNeedTime[1] = %d\r\n", sSysNeedTime[1]);
            printf(" sSysNeedChkTime[1] = %d\r\n", sSysNeedChkTime[1]);
            printf(" sSysNeedReqTime[1] = %d\r\n", sSysNeedReqTime[1]);
            printf(" gUserCtrlCmd_69[1] = %d\r\n", gUserCtrlCmd_69[1]);
            printf(" gCtrlCmdInfo_70[1] = %d\r\n", gCtrlCmdInfo_70[1]);
        }


        return CLI_FALSE;


}
CLI_CMD("devinsu_test", "devinsu_test", devinsu_test);
#endif