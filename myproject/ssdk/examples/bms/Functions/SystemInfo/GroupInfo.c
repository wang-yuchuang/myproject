/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : GroupInfo.c
** @Founder      : ZQD808
** @Date         : 2025.01.22
** @Function     : 电池组全局信息管理模块
**===========================================================================================*/
#include "GroupInfo.h"
#include "PublicInfo.h"
#include "GroupPara.h"
#include "ParaWrite.h"
#include "ParaInterface.h"
#include <cmsis_os2.h>
#include <stdio.h>
#include <debug.h>
#include "DiagUser.h"
#include "DiagFilter.h"
#include "CharactCalc.h"
#include "CapInfoCalc.h"
#include "CellFadeCalc.h"
#include "CurrIntegral.h"
#include "CurrSample.h"
#include "EnerInfoCalc.h"
#include "GroupProtect.h"
#include "DiagCheck.h"
#include "BalanTactic.h"
#include "CLI.h"
#include "CLI_type.h"
#include "heap_ext.h"
#include <stdlib.h>
#include <string.h>
#include "GroupTactic.h"
#include "IMIU4Protocol.h"
#include "InsuCheck.h"
#include "DodRcdStatis.h"

/* N2402 模块数 */
#ifndef MODNUM_N2402
#define MODNUM_N2402  4
#endif

extern u16 gGBmuGenPara_102[];  	 /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
extern u8 gGHardPara_104[];          /*主控硬件参数配置信息*/
extern t_FilState (*gErrFilTab)[eERR_ID_FINISH];

//校时信息
u16 *gVerTime_0 = NULL;             /*校时信息*/
//设备信息
u16 *gDevInfo_2 = NULL;             /*设备信息*/
u8 *gSerialNum_3 = NULL;             /*条码信息*/
u16 *gDevSampInfo_14 = NULL;        /*板级采样信息*/

//历史故障信息
u16 *gGHisErrInfo1_10 = NULL;     /*组1历史故障信息*/
u16 *gGHisErrInfo2_11 = NULL;     /*组2历史故障信息*/
u16 *gGHisEventInfo1_12 = NULL;   /*组1历史事件信息*/
u16 *gGHisEventInfo2_13 = NULL;   /*组2历史事件信息*/
//IMIU4相关信息
s16 *gImiu4Info_67 = NULL;      /*1#IMIU4信息核间通信数组*/
s16 *gImiu4Info2_68 = NULL;     /*2#IMIU4信息核间通信数组(2026-09-14接入,与67同构24词)*/
u16 *gSysDbgCmd_71 = NULL;      /*系统调试命令*/
u16 gGDebugResult[GDBGRESULT_LEN] = {0};  /*调试结果数组: 0=结果状态 1=读数据LW 2=读数据HW*/
u16 *gSampCalCmd_72 = NULL;     /*采样校准命令*/
u16 *gSampCalFbInfo_73 = NULL;  /*采样校准反馈信息*/
//BSU相关信息
u16 *gBsudPSampInfo_74 = NULL;  /*BSU02U2板级及PACK采样信息*/




//IEM3U相关信息
u8 *gImeu3BoardInfo1 = NULL;    /*IMEU3板级信息1*/
u16 *gImeu3BoardInfo2 = NULL;   /*IMEU3板级信息2*/
u16 *gImeu3AlmInfo = NULL;      /*IMEU3告警信息*/

/* IMEU3接收数据 */
u8  *gImeu3DevSerial = NULL;    /*IMEU3单板序列号(mIndex=2, u8)*/
u16 *gImeu3SysStatus = NULL;    /*IMEU3系统状态(mIndex=50, u16)*/
u16 *gImeu3CfgPara   = NULL;    /*IMEU3常规参数(mIndex=102, u16)*/
u16 *gImeu3DevAddr   = NULL;    /*IMEU3设备信息(mIndex=1, u16) */
u8  *gImeu3CfgSet_76 = NULL;    /*IMEU3参数设置(ICC mIndex=76, u8×12)*/
u16 *gImeu3Info_77   = NULL;    /*IMEU3状态信息(ICC mIndex=77, u16×53)*/
u8  *gImeu3CfgRO_75  = NULL;    /*IMEU3参数回读(ICC mIndex=75, u8×12)*/
u8   gEU3CfgReadbackFlag = 0;   /*IMEU3参数设置下发后回读标志*/

//IMIU4相关信息
gImiu4_data *gImiu4_Info[e_IMIU4_NUM] = {NULL};           //IMIU4信息

u16 *gUserCtrlCmd_69 = NULL;     /*用户控制命令数组*/
u16 *gCtrlCmdInfo_70 = NULL;     /*控制命令反馈信息数组*/

//电芯相关信息
u16 *gGCellVoltInfo_1 = NULL;  /*组1电池单体电压信息*/
u16 *gGCellVoltInfo_2 = NULL;  /*组2电池单体电压信息*/
s16 *gGCellTempInfo_1 = NULL;  /*组1电池单体温度信息*/
s16 *gGCellTempInfo_2 = NULL;  /*组2电池单体温度信息*/
u16 *gGCellResisInfo_1 = NULL; /*组1电池单体内阻信息*/
u16 *gGCellResisInfo_2 = NULL; /*组2电池单体内阻信息*/
u16 *gGChgBalanInfo_1 = NULL;  /*组1充电均衡状态信息*/
u16 *gGChgBalanInfo_2 = NULL;  /*组2充电均衡状态信息*/
u16 *gGDhgBalanInfo_1 = NULL;  /*组1放电均衡状态信息*/
u16 *gGDhgBalanInfo_2 = NULL;  /*组2放电均衡状态信息*/
#define DHG_BALAN_STATE_RCV_TIMEOUT_MS (1000U)
static u32 sGDhgBalanStateLastRcvTick[CLUTER_MAX_NUM][MAX_BSU_NUM] = {{0}}; /*从控放电均衡状态最后有效回报时刻+1*/
u8 *gGAfeStateInfo_1 = NULL;   /*组1AFE状态信息*/
u8 *gGAfeStateInfo_2 = NULL;   /*组2AFE状态信息*/

//内阻查询相关信息
u16 *gIccInsuQuery_85 = NULL;      /*ICC mIndex=85 内阻查询指令缓冲(u16×8)*/
u32 *gIccCellResisReal = NULL;     /*ICC mIndex=28-31 内阻实部缓冲(u32×4×211)*/
u32 *gIccCellResisImag = NULL;     /*ICC mIndex=32-35 内阻虚部缓冲(u32×4×211)*/
u32 *gBsuCellResisReal = NULL;     /*BSU 内阻实部原始缓冲(u32×N)*/
u32 *gBsuCellResisImag = NULL;     /*BSU 内阻虚部原始缓冲(u32×N)*/
u16  gIccInsuFreqIdx = 0;          /*ICC85下发的内阻检测频率索引(填ICC28-35子2)*/


//整体信息
u16 *gSysStaInfo_50 = NULL;       /*系统状态信息*/
s16 *gSysMainInfo_51 = NULL;      /*系统整体信息*/
u16 *gSysCapInfo_52 = NULL;       /*系统电量信息*/
u16 *gSysLimInfo_53 = NULL;       /*系统限值信息*/
s16 *gSysCharInfo_54 = NULL;      /*系统特征信息*/
u16 *gCurrCapEner_55 = NULL;      /*当前能量统计信息*/
u32 *gHisCapEner_56 = NULL;       /*累计能量统计信息*/
u16 *gAlmFault1_57 = NULL;        /*组1告警故障信息*/
u16 *gAlmFault2_58 = NULL;        /*组2告警故障信息*/
u16 *gGStatistTime_59 = NULL;     /*时间统计信息*/
u16 *gGStatistErr_60 = NULL;      /*故障统计信息*/
u16 *gGStatistSosInfo_61 = NULL;  /*安全等级SOS统计信息*/
u16 *gGStatistDodInfo_62 = NULL;  /*放电深度统计信息*/
u16 *gGHisDodRing = NULL;         /*历史DOD记录环形缓冲(内部,协议出口mIndex15/16)*/
u16 *gGHisDodInfo1_15 = NULL;     /*组1历史DOD记录信息(最新一条,ICC mIndex15推送/查询出口)*/
u16 *gGHisDodInfo2_16 = NULL;     /*组2历史DOD记录信息(最新一条,ICC mIndex16推送/查询出口)*/
u16 *gGAuthorInfo_63 = NULL;      /*权限级别状态信息*/
u16 *gGAuthorCtrl_64 = NULL;      /*权限密码登录信息数组*/
u16 *gSysCtrlCmd_65 = NULL;       /*组1系统控制命令*/
u16 *gSysCtrlCmd_66 = NULL;       /*组2系统控制命令*/

u16 (*gGBsuGenPara_102)[SLAVE_MAX_NUM][GINDEX105_LEN] = NULL;   /* 从控参数指针数组 */
s16 (*gPackTemp_94)[MAX_BSU_NUM][GINDEX94_LEN] = NULL;          /* 从控环境信息数组 */
u16 (*gPackStat_95)[MAX_BSU_NUM][GINDEX95_LEN] = NULL;          /* 从控环境信息数组 */

u16 *gTFAirUserParaInfo_212 = NULL;  /*TF液冷空调用户参数信息*/
u16 *gTFAirStateInfo_213 = NULL;     /*TF液冷空调状态信息*/
u16 *gTFAirErrInfo_214 = NULL;       /*TF液冷空调故障信息*/

/*=============================================================================================
** @Name      : u8 GroupInfoInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===========================================================================================*/
u8 GroupInfoInit(void)
{
   static u8 init_flag = 0;
   if(init_flag == 1)
      return FALSE;

   //ssdk_printf(SSDK_WARNING, "[DIAG] BMS diagnostic logging initialized — CONFIG_DEBUG is ENABLED\r\n"); //调试横幅,已屏蔽 2026-08-31
   //校时信息
   gVerTime_0 = ExtMalloc(sizeof(u16) * GINDEX0_LEN);
   if(gVerTime_0 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gVerTime_0 fail\r\n", __func__);
       ExtFree(gVerTime_0);
       return FALSE;
   }
   else 
   {
      printf("malloc gVerTime_0 %d success\r\n",sizeof(u16) * GINDEX0_LEN);
      for(u16 i = 0;i < GINDEX0_LEN;i++)
      {
        gVerTime_0[i] = 0;
      }
      gVerTime_0[0] = 2025;       /*系统时间-年*/
      gVerTime_0[1] = 0x0101;     /*系统时间-月日*/
      gVerTime_0[2] = 0x0;        /*系统时间-时分*/
      gVerTime_0[3] = 0x0;        /*系统时间-毫秒*/
   }

   //设备信息
   gDevInfo_2 = ExtMalloc(sizeof(u16) * GINDEX2_LEN);
   if(gDevInfo_2 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gDevInfo_2 fail\r\n", __func__);
       ExtFree(gDevInfo_2);
       return FALSE;
   }
   else 
   {
      printf("malloc gDevInfo_2 %d success\r\n",sizeof(u16) * GINDEX2_LEN);
      for(u16 i = 0;i < GINDEX2_LEN;i++)
      {
        gDevInfo_2[i] = 0;
      }
      gDevInfo_2[0] = BMU_DEV_ID;
      gDevInfo_2[1] = BMU_BAS_DEVADDR;
      gDevInfo_2[2] = BMU_HW_VER;
      gDevInfo_2[3] = BMU_SW_VERL;
      gDevInfo_2[4] = BMU_SW_VERH;
      gDevInfo_2[5] = 0x0100;
      gDevInfo_2[6] = 0x0100;
      gDevInfo_2[7] = 0x0100;
   }

   //序列号信息
   gSerialNum_3 = ExtMalloc(sizeof(u8) * GINDEX3_LEN);
   if(gSerialNum_3 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gSerialNum_3 fail\r\n", __func__);
       ExtFree(gSerialNum_3);
       return FALSE;
   }
   else 
   {
      printf("malloc gSerialNum_3 %d success\r\n",sizeof(u8) * GINDEX3_LEN);
      for(u16 i = 0;i < GINDEX3_LEN;i++)
      {
        gSerialNum_3[i] = 0;
      }
   }

    //组1历史故障信息
    gGHisErrInfo1_10 = ExtMalloc(sizeof(u16) * GINDEX10_LEN);
    if(gGHisErrInfo1_10 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGHisErrInfo1_10 fail\r\n", __func__);
        ExtFree(gGHisErrInfo1_10);
        return FALSE;
    }
    else 
    {
        printf("malloc gGHisErrInfo1_10 %d success\r\n",sizeof(u8) * GINDEX10_LEN);

    }

    //组2历史故障信息
    gGHisErrInfo2_11 = ExtMalloc(sizeof(u16) * GINDEX11_LEN);
    if(gGHisErrInfo2_11 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGHisErrInfo2_11 fail\r\n", __func__);
        ExtFree(gGHisErrInfo2_11);
        return FALSE;
    }
    else 
    {
        printf("malloc gGHisErrInfo2_11 %d success\r\n",sizeof(u8) * GINDEX11_LEN);

    }

    //组1历史事件信息
    gGHisEventInfo1_12 = ExtMalloc(sizeof(u16) * GINDEX12_LEN);
    if(gGHisEventInfo1_12 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGHisEventInfo1_12 fail\r\n", __func__);
        ExtFree(gGHisEventInfo1_12);
        return FALSE;
    }
    else 
    {
        printf("malloc gGHisEventInfo1_12 %d success\r\n",sizeof(u8) * GINDEX12_LEN);

    }

    //组2历史故障信息
    gGHisEventInfo2_13 = ExtMalloc(sizeof(u16) * GINDEX13_LEN);
    if(gGHisEventInfo2_13 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGHisEventInfo2_13 fail\r\n", __func__);
        ExtFree(gGHisEventInfo2_13);
        return FALSE;
    }
    else 
    {
        printf("malloc gGHisEventInfo2_13 %d success\r\n",sizeof(u8) * GINDEX13_LEN);

    }


   
   //设备信息
   gDevSampInfo_14 = ExtMalloc(sizeof(u16) * GINDEX14_LEN);
   if(gDevSampInfo_14 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gDevSampInfo_14 fail\r\n", __func__);
       ExtFree(gDevSampInfo_14);
       return FALSE;
   }
   else 
   {
      printf("malloc gDevSampInfo_14 %d success\r\n",sizeof(u16) * GINDEX14_LEN);
      for(u16 i = 0;i < GINDEX14_LEN;i++)
      {
        gDevSampInfo_14[i] = 0;
      }
   }

 
    /* 用户控制命令数组 */
    gCtrlCmdInfo_70 = ExtMalloc(sizeof(u16) * eCtrlCmdInfo70_Len);
    
    if(gCtrlCmdInfo_70 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gCtrlCmdInfo_70 fail\r\n", __func__);
        return FALSE; /* 内存分配失败，直接返回 */
    }
    else
    {
      for(u8 i = 0;i < eCtrlCmdInfo70_Len;i++)
      {
        gCtrlCmdInfo_70[i] = 0;
      }
        printf("malloc gCtrlCmdInfo_70 %d success\r\n", sizeof(u16) * eCtrlCmdInfo70_Len);
    }

    /* 控制命令反馈信息数组 */
    gUserCtrlCmd_69 = ExtMalloc(sizeof(u16) * eUserCtrlCmd69_Len);
    
    if(gUserCtrlCmd_69 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gUserCtrlCmd_69 fail\r\n", __func__);
        return FALSE; /* 内存分配失败，直接返回 */
    }
    else
    {
      for(u8 i = 0;i < eUserCtrlCmd69_Len;i++)
      {
        gUserCtrlCmd_69[i] = 0;
      }
        printf("malloc gUserCtrlCmd_69 %d success\r\n", sizeof(u16) * eUserCtrlCmd69_Len);
    }


   //IMEU3信息
   gImeu3BoardInfo1 = ExtMalloc(sizeof(u8) * eIMEUBoardInfo1_Len);
   if(gImeu3BoardInfo1 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3BoardInfo1 fail\r\n", __func__);
       ExtFree(gImeu3BoardInfo1);
       return FALSE;
   }
   else 
   {
      printf("malloc gImeu3BoardInfo1 %d success\r\n",sizeof(u8) * eIMEUBoardInfo1_Len);
      for(u16 i = 0;i < eIMEUBoardInfo1_Len;i++)
      {
        gImeu3BoardInfo1[i] = 0;
      }
   }

   gImeu3BoardInfo2 = ExtMalloc(sizeof(u16) * eIMEUBoardInfo2_Len);
   if(gImeu3BoardInfo2 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3BoardInfo2 fail\r\n", __func__);
       ExtFree(gImeu3BoardInfo2);
       return FALSE;
   }
   else 
   {
      printf("malloc gImeu3BoardInfo2 %d success\r\n",sizeof(u16) * eIMEUBoardInfo2_Len);
      for(u16 i = 0;i < eIMEUBoardInfo2_Len;i++)
      {
        gImeu3BoardInfo2[i] = 0;
      }
   }

   gImeu3AlmInfo = ExtMalloc(sizeof(u16) * eIMEUAlmInfo_Len);
   if(gImeu3AlmInfo == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3AlmInfo fail\r\n", __func__);
       ExtFree(gImeu3AlmInfo);
       return FALSE;
   }
   else 
   {
      printf("malloc gImeu3AlmInfo %d success\r\n",sizeof(u16) * eIMEUAlmInfo_Len);
      for(u16 i = 0;i < eIMEUAlmInfo_Len;i++)
      {
        gImeu3AlmInfo[i] = 0;
       }
    }

    /* IMEU3单板序列号(u8, mIndex=2, 23项) */
    gImeu3DevSerial = ExtMalloc(sizeof(u8) * eIMEUDevSerial_Len);
    if(gImeu3DevSerial == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3DevSerial fail\r\n", __func__);
        ExtFree(gImeu3DevSerial);
        return FALSE;
    }
    else
    {
        for(u16 i = 0; i < eIMEUDevSerial_Len; i++)
            gImeu3DevSerial[i] = 0;
    }

    /* IMEU3系统状态(u16, mIndex=50, 6项) */
    gImeu3SysStatus = ExtMalloc(sizeof(u16) * eIMEU3SysStat_Len);
    if(gImeu3SysStatus == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3SysStatus fail\r\n", __func__);
        ExtFree(gImeu3SysStatus);
        return FALSE;
    }
    else
    {
        for(u16 i = 0; i < eIMEU3SysStat_Len; i++)
            gImeu3SysStatus[i] = 0;
    }

    /* IMEU3常规参数(u16, mIndex=102, 20项) */
    gImeu3CfgPara = ExtMalloc(sizeof(u16) * eIMEU3Cfg_Len);
    if(gImeu3CfgPara == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3CfgPara fail\r\n", __func__);
        ExtFree(gImeu3CfgPara);
        return FALSE;
    }
    else
    {
        for(u16 i = 0; i < eIMEU3Cfg_Len; i++)
            gImeu3CfgPara[i] = 0;
    }

    /* IMEU3设备信息(u16, mIndex=1, 10项) */
    gImeu3DevAddr = ExtMalloc(sizeof(u16) * eIMEUDevAddr_Len);
    if(gImeu3DevAddr == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3DevAddr fail\r\n", __func__);
        return FALSE;
    }
    for(u16 i = 0; i < eIMEUDevAddr_Len; i++)
        gImeu3DevAddr[i] = 0;

    /* IMEU3配置缓存(u8, ICC mIndex=76, 12项) */
    gImeu3CfgSet_76 = ExtMalloc(sizeof(u8) * GINDEX76_LEN);
    if(gImeu3CfgSet_76 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3CfgSet_76 fail\r\n", __func__);
        ExtFree(gImeu3CfgSet_76);
        return FALSE;
    }
    else
    {
        for(u16 i = 0; i < GINDEX76_LEN; i++)
            gImeu3CfgSet_76[i] = 0;
    }

    /* IMEU3采样故障汇总(u16, ICC mIndex=77, 53项) */
    gImeu3Info_77 = ExtMalloc(sizeof(u16) * GINDEX77_LEN);
    if(gImeu3Info_77 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3Info_77 fail\r\n", __func__);
        ExtFree(gImeu3Info_77);
        return FALSE;
    }
    else
    {
        for(u16 i = 0; i < GINDEX77_LEN; i++)
            gImeu3Info_77[i] = 0;
    }

    /* IMEU3回读参数缓存(u8, ICC mIndex=75, 12项) */
    gImeu3CfgRO_75 = ExtMalloc(sizeof(u8) * GINDEX75_LEN);
    if(gImeu3CfgRO_75 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gImeu3CfgRO_75 fail\r\n", __func__);
        ExtFree(gImeu3CfgRO_75);
        return FALSE;
    }
    else
    {
        for(u16 i = 0; i < GINDEX75_LEN; i++)
            gImeu3CfgRO_75[i] = 0;
    }

    //IMIU4信息
   for(int imiu4num = 0; imiu4num < e_IMIU4_NUM; imiu4num++)
   {
       gImiu4_Info[imiu4num] = (gImiu4_data*)ExtMalloc(sizeof(gImiu4_data));
        if (gImiu4_Info[imiu4num] == NULL)
        {
            ssdk_printf(SSDK_WARNING, "%s: malloc gImiu4_Info[%d] fail\r\n", __func__, imiu4num);
            ExtFree(gImiu4_Info[imiu4num]);
            return FALSE;
        }
        else
        {
            memset(gImiu4_Info[imiu4num], 0, sizeof(gImiu4_data));
            printf("malloc gImiu4_Info[%d] %d success\r\n", imiu4num, sizeof(gImiu4_data));
        }
   }

   //申请单体电压、温度、内阻、均衡状态信息空间
   gGCellVoltInfo_1 = ExtMalloc(sizeof(u16) * GINDEX92_LEN);
   if(gGCellVoltInfo_1 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGCellVoltInfo_1 fail\r\n", __func__);
       ExtFree(gGCellVoltInfo_1);
       return FALSE;
   }
   else 
   {
      printf("malloc gGCellVoltInfo_1 %d success\r\n",sizeof(u16) * GINDEX92_LEN);
      for(u16 i = 0;i < GINDEX92_LEN;i++)
      {
        gGCellVoltInfo_1[i] = 0;
      }
   }

   gGCellVoltInfo_2 = ExtMalloc(sizeof(u16) * GINDEX93_LEN);
   if(gGCellVoltInfo_2 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGCellVoltInfo_2 fail\r\n", __func__);
       ExtFree(gGCellVoltInfo_2);
       return FALSE;
   }
   else 
   {
      printf("malloc gGCellVoltInfo_2 %d success\r\n",sizeof(u16) * GINDEX93_LEN);
      for(u16 i = 0;i < GINDEX93_LEN;i++)
      {
        gGCellVoltInfo_2[i] = 0;
      }
   }
   //温度
   gGCellTempInfo_1 = ExtMalloc(sizeof(s16) * GINDEX93_LEN);
   if(gGCellTempInfo_1 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGCellTempInfo_1 fail\r\n", __func__);
       ExtFree(gGCellTempInfo_1);
       return FALSE;
   }
   else 
   {
      printf("malloc gGCellTempInfo_1 %d success\r\n",sizeof(s16) * GINDEX93_LEN);
      for(u16 i = 0;i < GINDEX93_LEN;i++)
      {
        gGCellTempInfo_1[i] = 0;
      }
   }

    gGCellTempInfo_2 = ExtMalloc(sizeof(s16) * GINDEX93_LEN);
    if(gGCellTempInfo_2 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGCellTempInfo_2 fail\r\n", __func__);
        ExtFree(gGCellTempInfo_2);
        return FALSE;
    }
    else 
    {
       printf("malloc gGCellTempInfo_2 %d success\r\n",sizeof(s16) * GINDEX93_LEN);
       for(u16 i = 0;i < GINDEX93_LEN;i++)
      {
        gGCellTempInfo_2[i] = 0;
      }
    }

   //内阻信息
   gGCellResisInfo_1 = ExtMalloc(sizeof(u16) * GINDEX92_LEN);
   if(gGCellResisInfo_1 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGCellResisInfo_1 fail\r\n", __func__);
       ExtFree(gGCellResisInfo_1);
       return FALSE;
   }
   else 
   {
      printf("malloc gGCellResisInfo_1 %d success\r\n",sizeof(u16) * GINDEX92_LEN);
      for(u16 i = 0;i < GINDEX92_LEN;i++)
      {
        gGCellResisInfo_1[i] = 0;
      }
   }

   gGCellResisInfo_2 = ExtMalloc(sizeof(u16) * GINDEX92_LEN);
   if(gGCellResisInfo_2 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGCellResisInfo_2 fail\r\n", __func__);
       ExtFree(gGCellResisInfo_2);
       return FALSE;
   }
   else 
   {
      printf("malloc gGCellResisInfo_2 %d success\r\n",sizeof(u16) * GINDEX92_LEN);
      for(u16 i = 0;i < GINDEX92_LEN;i++)
      {
        gGCellResisInfo_2[i] = 0;
      }
   }

   //被动均衡
   gGDhgBalanInfo_1 = ExtMalloc(sizeof(u16) * GINDEX91_LEN);
   if(gGDhgBalanInfo_1 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGDhgBalanInfo_1 fail\r\n", __func__);
       ExtFree(gGDhgBalanInfo_1);
       return FALSE;
   }
   else 
   {
      printf("malloc gGDhgBalanInfo_1 %d success\r\n",sizeof(u16) * GINDEX91_LEN);
      for(u16 i = 0;i < GINDEX91_LEN;i++)
      {
        gGDhgBalanInfo_1[i] = 0;
      }
   }

    gGDhgBalanInfo_2 = ExtMalloc(sizeof(u16) * GINDEX91_LEN);
    if(gGDhgBalanInfo_2 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGDhgBalanInfo_2 fail\r\n", __func__);
        ExtFree(gGDhgBalanInfo_2);
        return FALSE;
    }
   else 
   {
      printf("malloc gGDhgBalanInfo_2 %d success\r\n",sizeof(u16) * GINDEX91_LEN);
      for(u16 i = 0;i < GINDEX91_LEN;i++)
      {
        gGDhgBalanInfo_2[i] = 0;
      }
   }

   //主动均衡
   gGChgBalanInfo_1 = ExtMalloc(sizeof(u16) * GINDEX90_LEN);
   if(gGChgBalanInfo_1 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGChgBalanInfo_1 fail\r\n", __func__);
       ExtFree(gGChgBalanInfo_1);
       return FALSE;
   }
   else 
   {
      printf("malloc gGChgBalanInfo_1 %d success\r\n",sizeof(u16) * GINDEX90_LEN);
      for(u16 i = 0;i < GINDEX90_LEN;i++)
      {
        gGChgBalanInfo_1[i] = 0;
      }
   }
    //主动均衡
   gGChgBalanInfo_2 = ExtMalloc(sizeof(u16) * GINDEX90_LEN);
   if(gGChgBalanInfo_2 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGChgBalanInfo_2 fail\r\n", __func__);
       ExtFree(gGChgBalanInfo_2);
       return FALSE;
   }
   else 
   {
      printf("malloc gGChgBalanInfo_2 %d success\r\n",sizeof(u16) * GINDEX90_LEN);
      for(u16 i = 0;i < GINDEX90_LEN;i++)
      {
        gGChgBalanInfo_2[i] = 0;
      }
   }

       //AFE状态信息
    gGAfeStateInfo_1 = ExtMalloc(sizeof(u8) * GINDEX98_LEN);
    if(gGAfeStateInfo_1 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGAfeStateInfo_1 fail\r\n", __func__);
        ExtFree(gGAfeStateInfo_1);
        return FALSE;
    }
    else
    {
       memset(gGAfeStateInfo_1, 0, sizeof(u8) * GINDEX98_LEN);
       printf("malloc gGAfeStateInfo_1 %d success\r\n",sizeof(u8) * GINDEX98_LEN);
    }

    gGAfeStateInfo_2 = ExtMalloc(sizeof(u8) * GINDEX98_LEN);
    if(gGAfeStateInfo_2 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGAfeStateInfo_2 fail\r\n", __func__);
        ExtFree(gGAfeStateInfo_2);
        return FALSE;
    }
    else
    {
       memset(gGAfeStateInfo_2, 0, sizeof(u8) * GINDEX98_LEN);
       printf("malloc gGAfeStateInfo_2 %d success\r\n",sizeof(u8) * GINDEX98_LEN);
    }

    /*内阻查询信息(ICC85/ICC28-35/BSU50-53实部+90-93虚部), N=总单体数=主控高级参数VSumNum*/
    {
        u16 insuCellNum = gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum];

        gIccInsuQuery_85 = ExtMalloc(sizeof(u16) * ICC_MINDEX85_INSQUERY_LEN);
        if(gIccInsuQuery_85 == NULL)
        {
            ssdk_printf(SSDK_WARNING, "%s: malloc gIccInsuQuery_85 fail\r\n", __func__);
            ExtFree(gIccInsuQuery_85);
            return FALSE;
        }
        else
        {
            memset(gIccInsuQuery_85, 0, sizeof(u16) * ICC_MINDEX85_INSQUERY_LEN);
        }

        gBsuCellResisReal = ExtMalloc(sizeof(u32) * insuCellNum);
        if(gBsuCellResisReal == NULL)
        {
            ssdk_printf(SSDK_WARNING, "%s: malloc gBsuCellResisReal fail\r\n", __func__);
            ExtFree(gBsuCellResisReal);
            return FALSE;
        }
        else
        {
            memset(gBsuCellResisReal, 0, sizeof(u32) * insuCellNum);
        }

        gBsuCellResisImag = ExtMalloc(sizeof(u32) * insuCellNum);
        if(gBsuCellResisImag == NULL)
        {
            ssdk_printf(SSDK_WARNING, "%s: malloc gBsuCellResisImag fail\r\n", __func__);
            ExtFree(gBsuCellResisImag);
            return FALSE;
        }
        else
        {
            memset(gBsuCellResisImag, 0, sizeof(u32) * insuCellNum);
        }

        gIccCellResisReal = ExtMalloc(sizeof(u32) * (ICC_RESIS_GRP_NUM * ICC_MINDEX28_IDX_LEN));
        if(gIccCellResisReal == NULL)
        {
            ssdk_printf(SSDK_WARNING, "%s: malloc gIccCellResisReal fail\r\n", __func__);
            ExtFree(gIccCellResisReal);
            return FALSE;
        }
        else
        {
            memset(gIccCellResisReal, 0, sizeof(u32) * (ICC_RESIS_GRP_NUM * ICC_MINDEX28_IDX_LEN));
        }

        gIccCellResisImag = ExtMalloc(sizeof(u32) * (ICC_RESIS_GRP_NUM * ICC_MINDEX28_IDX_LEN));
        if(gIccCellResisImag == NULL)
        {
            ssdk_printf(SSDK_WARNING, "%s: malloc gIccCellResisImag fail\r\n", __func__);
            ExtFree(gIccCellResisImag);
            return FALSE;
        }
        else
        {
            memset(gIccCellResisImag, 0, sizeof(u32) * (ICC_RESIS_GRP_NUM * ICC_MINDEX28_IDX_LEN));
        }
    }

   gSysStaInfo_50 = ExtMalloc(sizeof(u16) * GINDEX50_LEN);
   if(gSysStaInfo_50 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gSysStaInfo_50 fail\r\n", __func__);
       ExtFree(gSysStaInfo_50);
       return FALSE;
   }
   else 
   {
      printf("malloc gSysStaInfo_50 %d success\r\n",sizeof(u16) * GINDEX50_LEN);
       for(u16 i = 0; i < GINDEX50_LEN; i++)
       {
         gSysStaInfo_50[i] = 0;
       }
   }

   //系统整体信息 
   gSysMainInfo_51 = ExtMalloc(sizeof(s16) * GINDEX51_LEN);
   if(gSysMainInfo_51 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gSysMainInfo_51 fail\r\n", __func__);
       ExtFree(gSysMainInfo_51);
       return FALSE;
   }
   else 
   {
      printf("malloc gSysMainInfo_51 %d success\r\n",sizeof(s16) * GINDEX51_LEN);
       for(u16 i = 0; i < GINDEX51_LEN; i++)
       {
         gSysMainInfo_51[i] = 0;
       }
   }

   //系统电量信息 
   gSysCapInfo_52 = ExtMalloc(sizeof(u16) * GINDEX52_LEN);
   if(gSysCapInfo_52 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gSysCapInfo_52 fail\r\n", __func__);
       ExtFree(gSysCapInfo_52);
       return FALSE;
   }
   else 
   {
      printf("malloc gSysCapInfo_52 %d success\r\n",sizeof(u16) * GINDEX52_LEN);
       for(u16 i = 0; i < GINDEX52_LEN; i++)
       {
         gSysCapInfo_52[i] = 0;
       }
   }

    //系统限值信息 
   gSysLimInfo_53 = ExtMalloc(sizeof(u16) * GINDEX53_LEN);
   if(gSysLimInfo_53 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gSysLimInfo_53 fail\r\n", __func__);
       ExtFree(gSysLimInfo_53);
       return FALSE;
   }
   else 
   {
      printf("malloc gSysLimInfo_53 %d success\r\n",sizeof(u16) * GINDEX53_LEN);
      for(u16 i = 0; i < GINDEX53_LEN; i++)
      {
         gSysLimInfo_53[i] = 0;
      }
   }

    //系统特征信息 
   gSysCharInfo_54 = ExtMalloc(sizeof(s16) * GINDEX54_LEN);
   if(gSysCharInfo_54 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gSysCharInfo_54 fail\r\n", __func__);
       ExtFree(gSysCharInfo_54);
       return FALSE;
   }
   else 
   {
      printf("malloc gSysCharInfo_54 %d success\r\n",sizeof(s16) * GINDEX54_LEN);
      for(u16 i = 0; i < GINDEX54_LEN; i++)
      {
          gSysCharInfo_54[i] = 0;
      }
   }

   //当前能量统计信息 
   gCurrCapEner_55 = ExtMalloc(sizeof(u16) * GINDEX55_LEN);
   if(gCurrCapEner_55 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gCurrCapEner_55 fail\r\n", __func__);
       ExtFree(gCurrCapEner_55);
       return FALSE;
   }
   else 
   {
      printf("malloc gCurrCapEner_55 %d success\r\n",sizeof(u16) * GINDEX55_LEN);
       for(u16 i = 0; i < GINDEX55_LEN; i++)
       {
         gCurrCapEner_55[i] = 0;
       }
   }

    //历史能量统计信息 
   gHisCapEner_56 = ExtMalloc(sizeof(u32) * GINDEX56_LEN);
   if(gHisCapEner_56 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gHisCapEner_56 fail\r\n", __func__);
       ExtFree(gHisCapEner_56);
       return FALSE;
   }
   else 
   {
      printf("malloc gHisCapEner_56 %d success\r\n",sizeof(u32) * GINDEX56_LEN);
       for(u16 i = 0; i < GINDEX56_LEN; i++)
       {
         gHisCapEner_56[i] = 0;
       }
   }

   
   //组1故障告警信息 
   gAlmFault1_57 = ExtMalloc(sizeof(u16) * GINDEX57_LEN);
   if(gAlmFault1_57 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gAlmFault1_57 fail\r\n", __func__);
       ExtFree(gAlmFault1_57);
       return FALSE;
   }
   else 
   {
      printf("malloc gAlmFault1_57 %d success\r\n",sizeof(u16) * GINDEX57_LEN);
       for(u16 i = 0; i < GINDEX57_LEN; i++)
       {
         gAlmFault1_57[i] = 0;
       }
   }

   //组2故障告警信息 
   gAlmFault2_58 = ExtMalloc(sizeof(u16) * GINDEX58_LEN);
   if(gAlmFault2_58 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gAlmFault2_58 fail\r\n", __func__);
       ExtFree(gAlmFault2_58);
       return FALSE;
   }
   else 
   {
      printf("malloc gAlmFault2_58 %d success\r\n",sizeof(u16) * GINDEX58_LEN);
       for(u16 i = 0; i < GINDEX58_LEN; i++)
       {
         gAlmFault2_58[i] = 0;
       }
   }
   
   //时间统计信息 
   gGStatistTime_59 = ExtMalloc(sizeof(u16) * GINDEX59_LEN);
   if(gGStatistTime_59 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGStatistTime_59 fail\r\n", __func__);
       ExtFree(gGStatistTime_59);
       return FALSE;
   }
   else 
   {
      printf("malloc gGStatistTime_59 %d success\r\n",sizeof(u16) * GINDEX59_LEN);
       for(u16 i = 0; i < GINDEX59_LEN; i++)
       {
         gGStatistTime_59[i] = 0;
       }
   }
   //故障统计信息 
   gGStatistErr_60 = ExtMalloc(sizeof(u16) * GINDEX60_LEN);
   if(gGStatistErr_60 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGStatistErr_60 fail\r\n", __func__);
       ExtFree(gGStatistErr_60);
       return FALSE;
   }
   else 
   {
      printf("malloc gGStatistErr_60 %d success\r\n",sizeof(u16) * GINDEX60_LEN);
       for(u16 i = 0; i < GINDEX60_LEN; i++)
       {
         gGStatistErr_60[i] = 0;
       }
   }

   //安全等级SOS统计信息 
   gGStatistSosInfo_61 = ExtMalloc(sizeof(u16) * GINDEX61_LEN);
   if(gGStatistSosInfo_61 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGStatistSosInfo_61 fail\r\n", __func__);
       ExtFree(gGStatistSosInfo_61);
       return FALSE;
   }
   else 
   {
      printf("malloc gGStatistSosInfo_61 %d success\r\n",sizeof(u16) * GINDEX61_LEN);
       for(u16 i = 0; i < GINDEX61_LEN; i++)
       {
         gGStatistSosInfo_61[i] = 0;
       }
   }

   //放电深度统计信息 
   gGStatistDodInfo_62 = ExtMalloc(sizeof(u16) * GINDEX62_LEN);
   if(gGStatistDodInfo_62 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGStatistDodInfo_62 fail\r\n", __func__);
       ExtFree(gGStatistDodInfo_62);
       return FALSE;
   }
   else 
   {
      printf("malloc gGStatistDodInfo_62 %d success\r\n",sizeof(u16) * GINDEX62_LEN);
       for(u16 i = 0; i < GINDEX62_LEN; i++)
       {
         gGStatistDodInfo_62[i] = 0;
       }
   }

    //历史DOD记录信息(环形缓冲,内部镜像,无ICC注册)
    gGHisDodRing = ExtMalloc(sizeof(u16) * HISDOD_RING_LEN * CLUTER_MAX_NUM);
    if(gGHisDodRing == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGHisDodRing fail\r\n", __func__);
        ExtFree(gGHisDodRing);
        return FALSE;
    }
    else
    {
        printf("malloc gGHisDodRing %d success\r\n",sizeof(u16) * HISDOD_RING_LEN * CLUTER_MAX_NUM);
        for(u16 i = 0; i < HISDOD_RING_LEN * CLUTER_MAX_NUM; i++)
        {
            gGHisDodRing[i] = 0;
        }
    }

    //组1历史DOD记录信息(最新一条,ICC注册)
    gGHisDodInfo1_15 = ExtMalloc(sizeof(u16) * GINDEX15_LEN);
    if(gGHisDodInfo1_15 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGHisDodInfo1_15 fail\r\n", __func__);
        ExtFree(gGHisDodInfo1_15);
        return FALSE;
    }
    else
    {
        printf("malloc gGHisDodInfo1_15 %d success\r\n",sizeof(u16) * GINDEX15_LEN);
        for(u16 i = 0; i < GINDEX15_LEN; i++)
        {
            gGHisDodInfo1_15[i] = 0;
        }
    }

    //组2历史DOD记录信息(最新一条,ICC注册)
    gGHisDodInfo2_16 = ExtMalloc(sizeof(u16) * GINDEX16_LEN);
    if(gGHisDodInfo2_16 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGHisDodInfo2_16 fail\r\n", __func__);
        ExtFree(gGHisDodInfo2_16);
        return FALSE;
    }
    else
    {
        printf("malloc gGHisDodInfo2_16 %d success\r\n",sizeof(u16) * GINDEX16_LEN);
        for(u16 i = 0; i < GINDEX16_LEN; i++)
        {
            gGHisDodInfo2_16[i] = 0;
        }
    }

    //权限级别状态信息 
   gGAuthorInfo_63 = ExtMalloc(sizeof(u16) * GINDEX63_LEN);
   if(gGAuthorInfo_63 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGAuthorInfo_63 fail\r\n", __func__);
       ExtFree(gGAuthorInfo_63);
       return FALSE;
   }
   else 
   {
      printf("malloc gGAuthorInfo_63 %d success\r\n",sizeof(u16) * GINDEX63_LEN);
       for(u16 i = 0; i < GINDEX63_LEN; i++)
       {
         gGAuthorInfo_63[i] = 0;
       }
   }

    //权限密码登录信息数组
   gGAuthorCtrl_64 = ExtMalloc(sizeof(u16) * GINDEX64_LEN);
   if(gGAuthorCtrl_64 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gGAuthorCtrl_64 fail\r\n", __func__);
       ExtFree(gGAuthorCtrl_64);
       return FALSE;
   }
   else 
   {
      printf("malloc gGAuthorCtrl_64 %d success\r\n",sizeof(u16) * GINDEX64_LEN);
       for(u16 i = 0; i < GINDEX64_LEN; i++)
       {
         gGAuthorCtrl_64[i] = 0;
       }
   }

   //组1系统控制命令
   gSysCtrlCmd_65 = ExtMalloc(sizeof(u16) * GINDEX65_LEN);
   if(gSysCtrlCmd_65 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gSysCtrlCmd_65 fail\r\n", __func__);
       ExtFree(gSysCtrlCmd_65);
       return FALSE;
   }
   else 
   {
      printf("malloc gSysCtrlCmd_65 %d success\r\n",sizeof(u16) * GINDEX65_LEN);
       for(u16 i = 0; i < GINDEX65_LEN; i++)
       {
         gSysCtrlCmd_65[i] = 0;
       }
   }

   //组2系统控制命令
   gSysCtrlCmd_66 = ExtMalloc(sizeof(u16) * GINDEX66_LEN);
   if(gSysCtrlCmd_66 == NULL)
   {
       ssdk_printf(SSDK_WARNING, "%s: malloc gSysCtrlCmd_66 fail\r\n", __func__);
       ExtFree(gSysCtrlCmd_66);
       return FALSE;
   }
   else 
   {
      printf("malloc gSysCtrlCmd_66 %d success\r\n",sizeof(u16) * GINDEX66_LEN);
       for(u16 i = 0; i < GINDEX66_LEN; i++)
       {
         gSysCtrlCmd_66[i] = 0;
       }
   }




    //IMIU4信息核间通信数组
    gImiu4Info_67 = ExtMalloc(sizeof(s16) * GINDEX67_LEN);
    if(gImiu4Info_67 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gImiu4Info_67 fail\r\n", __func__);
        ExtFree(gImiu4Info_67);
        return FALSE;
    }
    else
    {
        printf("malloc gImiu4Info_67 %d success\r\n",sizeof(s16) * GINDEX67_LEN);
        for(u16 i = 0; i < GINDEX67_LEN; i++)
        {
            gImiu4Info_67[i] = 0;
        }
        gImiu4Info_67[eImiu4Info67_PasInsu] = 0;/*初始值0,未检测*/
        gImiu4Info_67[eImiu4Info67_NegInsu] = 0;
    }

    //组二IMIU4信息核间通信数组
    gImiu4Info2_68 = ExtMalloc(sizeof(s16) * GINDEX68_LEN);
    if(gImiu4Info2_68 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gImiu4Info2_68 fail\r\n", __func__);
        ExtFree(gImiu4Info2_68);
        return FALSE;
    }
    else
    {
        printf("malloc gImiu4Info2_68 %d success\r\n",sizeof(s16) * GINDEX68_LEN);
        for(u16 i = 0; i < GINDEX68_LEN; i++)
        {
            gImiu4Info2_68[i] = 0;
        }
        gImiu4Info2_68[eImiu4Info68_PasInsu] = 0;/*初始值0,未检测*/
        gImiu4Info2_68[eImiu4Info68_NegInsu] = 0;
    }

    //系统调试命令
    gSysDbgCmd_71 = ExtMalloc(sizeof(u16) * GINDEX71_LEN);
    if(gSysDbgCmd_71 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gSysDbgCmd_71 fail\r\n", __func__);
        ExtFree(gSysDbgCmd_71);
        return FALSE;
    }
    else 
    {
        printf("malloc gSysDbgCmd_71 %d success\r\n",sizeof(u16) * GINDEX71_LEN);
        for(u16 i = 0; i < GINDEX71_LEN; i++)
        {
            gSysDbgCmd_71[i] = 0;/*清零防Affirm堆残留误触发调试命令*/
        }
    }
    //采样校准命令
    gSampCalCmd_72 = ExtMalloc(sizeof(u16) * GINDEX72_LEN);
    if(gSampCalCmd_72 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gSampCalCmd_72 fail\r\n", __func__);
        ExtFree(gSampCalCmd_72);
        return FALSE;
    }
    else 
    {
        printf("malloc gSampCalCmd_72 %d success\r\n",sizeof(u16) * GINDEX72_LEN);
        for(u16 i = 0; i < GINDEX72_LEN; i++)
        {
            gSampCalCmd_72[i] = 0;
        }
    }
    //采样校准反馈信息
    gSampCalFbInfo_73 = ExtMalloc(sizeof(u16) * GINDEX73_LEN);
    if(gSampCalFbInfo_73 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gSampCalFbInfo_73 fail\r\n", __func__);
        ExtFree(gSampCalFbInfo_73);
        return FALSE;
    }
    else
    {
        printf("malloc gSampCalFbInfo_73 %d success\r\n",sizeof(u16) * GINDEX73_LEN);
        for(u16 i = 0; i < GINDEX73_LEN; i++)
        {
            gSampCalFbInfo_73[i] = 0;/*初始值0:无校准/未校准*/
        }
    }
    //BSU02U2板级及PACK采样信息
    gBsudPSampInfo_74 = ExtMalloc(sizeof(u16) * GINDEX74_LEN);
    if(gBsudPSampInfo_74 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gBsudPSampInfo_74 fail\r\n", __func__);
        ExtFree(gBsudPSampInfo_74);
        return FALSE;
    }
    else 
    {
        printf("malloc gBsudPSampInfo_74 %d success\r\n",sizeof(u16) * GINDEX74_LEN);
        for(u16 i = 0; i < GINDEX74_LEN; i++)
        {
            gBsudPSampInfo_74[i] = 0;/*RO组清零,防堆残留随机数上报A核*/
        }
    }








    u32 size94 = sizeof(s16) * CLUTER_MAX_NUM * MAX_BSU_NUM * GINDEX94_LEN;
    gPackTemp_94 = (s16 (*)[MAX_BSU_NUM][GINDEX94_LEN])ExtMalloc(size94);
    if(gPackTemp_94 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gPackTemp_94 fail\r\n", __func__);
        return FALSE;
    }
    else
    {
        memset(gPackTemp_94, 0, size94);
        printf("malloc gPackTemp_94 %d success\r\n", size94);
    }

    /* ====================================================================
    ** 申请 从控常规配置参数 (gGBsuGenPara_102) 内存空间
    ** ==================================================================*/
    u32 sizeBsu102 = sizeof(u16) * CLUTER_MAX_NUM * SLAVE_MAX_NUM * GINDEX105_LEN;
    
    /* 强制类型转换：匹配后面的两个维度 */
    gGBsuGenPara_102 = (u16 (*)[SLAVE_MAX_NUM][GINDEX105_LEN])ExtMalloc(sizeBsu102);
    
    if(gGBsuGenPara_102 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gGBsuGenPara_102 fail\r\n", __func__);
        return FALSE; /* 内存分配失败，直接返回 */
    }
    else
    {
        memset(gGBsuGenPara_102, 0, sizeBsu102);
        printf("malloc gGBsuGenPara_102 %d success\r\n", sizeBsu102);
    }
    /* ====================================================================
    ** 申请 95 协议 (PACK状态 u16) 内存空间
    ** ==================================================================*/
    u32 size95 = sizeof(u16) * CLUTER_MAX_NUM * MAX_BSU_NUM * GINDEX95_LEN;
    gPackStat_95 = (u16 (*)[MAX_BSU_NUM][GINDEX95_LEN])ExtMalloc(size95);
    if(gPackStat_95 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gPackStat_95 fail\r\n", __func__);
        return FALSE;
    }
    else
    {
        memset(gPackStat_95, 0, size95);
        printf("malloc gPackStat_95 %d success\r\n", size95);
    }
    //申请TF液冷空调内存空间
    gTFAirUserParaInfo_212 = ExtMalloc(sizeof(u16) * GINDEX212_LEN);
    if(gTFAirUserParaInfo_212 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gTFAirUserParaInfo_212 fail\r\n", __func__);
        return FALSE;
    }
    else
    {
        /* 如果有默认值，可以在这里初始化 */
        memset(gTFAirUserParaInfo_212, 0, sizeof(u16) * GINDEX212_LEN);
        printf("malloc gTFAirUserParaInfo_212 success\r\n");
    }

    gTFAirStateInfo_213 = ExtMalloc(sizeof(u16) * GINDEX213_LEN);
    if(gTFAirStateInfo_213 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gTFAirStateInfo_213 fail\r\n", __func__);
        return FALSE;
    }
    else
    {
        memset(gTFAirStateInfo_213, 0, sizeof(u16) * GINDEX213_LEN);
    }

    gTFAirErrInfo_214 = ExtMalloc(sizeof(u16) * GINDEX214_LEN);
    if(gTFAirErrInfo_214 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gTFAirErrInfo_214 fail\r\n", __func__);
        return FALSE;
    }
    else
    {
        memset(gTFAirErrInfo_214, 0, sizeof(u16) * GINDEX214_LEN);
    }

   DiagUserInfoInit();
   CharactCalcDataInit();
   CapEnerCorrInfoInit();
   CapInfoCalcDataInit();
   CellFadeCalcDataInit();
   CurrIntegralDataInit();
   EnerInfoCalcDataInit();
   GroupProtectDataInit();
   DiagFilterInfoInit();
   BalanTacticInfoInit();
   init_flag = 1;
   return TRUE;
}

/*=================================================================================================
** @Name      : u8 GetGroupOffsetDevAddrAPI(void)
** @Input     : void
** @Output    : 电池组自身设备地址偏移量
** @Function  : 获取电池组自身偏移设备地址
** @The notes : 主控设备地址偏移量
**===============================================================================================*/
u8 GetGroupOffsetDevAddrAPI(void)
{
    u8 group = 0;

    if((gGBmuGenPara_102[eBmuGenPara102_BmuDev] >= BMU_BAS_DEVADDR)
    	&& (gGBmuGenPara_102[eBmuGenPara102_BmuDev] < SMU_BAS_DEVADDR))
    {
    	group = (u8)(gGBmuGenPara_102[eBmuGenPara102_BmuDev] - BMU_BAS_DEVADDR);
    }
	else
	{
		group = 0xff;
	}

    return(group);
}
/*=============================================================================================
** @Name      : u16 GetBsuCellVolt(u8 ecu, u8 cell)
** @Input     : ecu:从控节点号[0,31] cell:从控内电池号[0,52]
** @Output    : 从控单体当前电压值:1mV
** @Function  : 获取指定从控内部单体当前电压值
**===========================================================================================*/
u16 GetBsuCellVolt(u8 clunum, u8 ecu, u8 cell)
{
    u16 index = 0;

    if(gGCellVoltInfo_1 == NULL || gGCellVoltInfo_2 == NULL)
    {
        return (0);
    }
    if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])       /*PACK节点号错误*/
        || (cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]))/*一个PACK电池节数错误*/
    {
        return (0);
    }

    if(clunum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]) //簇数节点号错误
    {
        return (0);
    }

    /*从控内序号转换成组内序号*/
    index = ChangSlaveVidToGroupVid(ecu, cell);

    /*电池组总节数正确*/
    if(index < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
    {
        if(clunum == 0)
            return(gGCellVoltInfo_1[index]);
        else
            return(gGCellVoltInfo_2[index]);
    }
    else
    {
        return(0);
    }
}
/*=================================================================================================
** @Name      : u16 ChangSlaveVidToGroupVid(u8 slave, u8 cell)
** @Input     : slave:从控号[0,31] cell:从控内电池号[0,17]
** @Output    : 从控内电压序号对应组内电压序号[0,399],0xffff:错误序号
** @Function  : 将从控内电压序号转换成电池组内电压序号
** @The notes : 每个从控电压数目不一样的需要特殊处理
**===============================================================================================*/
u16 ChangSlaveVidToGroupVid(u8 slave, u8 cell)
{
	u16 index = 0;

	//兼容CATL液冷PACK电池52串解析(16+18+18)
	#if(4 == PRJ_PACK_CELL_INFO)
	{
		//每3个第一个只有16节,越界非法
		if((0 == (slave % 3)) && (cell >= 16))
		{
			//错误序号
			index = 0xffff;
		}
		//每3个从控首个少2节(即第2个起每3个从控(52S)前移2S)
		else if(slave > 0)
		{
			//index = (slave - 1) * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum];
			index = (slave - 1) * 18;
			index = index - (index / 54 * 2) + cell + 16;
		}
		else
		{
			//index = slave * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] + cell;
			index = cell;
		}
	}
	#else
	{
		index = slave * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] + cell;
	}
    #endif

	return(index);
}
/*=============================================================================================
** @Name      : void InputBsuCellVolt(u8 clunum, u8 ecu, u8 cell, u16 volt)
** @Input     : clunum:第几簇 ecu:从控节点号[0,31] cell:从控内电池号[0,15] volt:电压值 1mv
** @Output    : void
** @Function  : 输入接收到从控单体电压值
**===========================================================================================*/
void InputBsuCellVolt(u8 clunum, u8 ecu, u8 cell, u16 volt)
{
    u16 index = 0;
    if(gGCellVoltInfo_1 == NULL || gGCellVoltInfo_2 == NULL)
    {
        return;
    }
    if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])    /*PACK节点号错误*/
        || (cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]))/*一个PACK电池节数错误*/
    {
        return;
    }
    if(clunum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]) //簇数节点号错误
    {
        return;
    }

	//从控内序号转换成组内序号
    index = ChangSlaveVidToGroupVid(ecu, cell);

    if(index < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])      /*电池组总节数正确*/
    {
        if(clunum == 0)
        {
            gGCellVoltInfo_1[index] = volt;
        }
        else
        {
            gGCellVoltInfo_2[index] = volt;
        }
        if(volt > 0)                                                /*非电压清零*/
        {
            DiagRcvCellMissCheckAPI(clunum, 0, ecu, cell);                /*电压信息缺失检测*/
        }
    }
}
/*=============================================================================================
** @Name      : s16 GetBsuCellTemp(u8 ecu, u8 num)
** @Input     : ecu:从控节点号[0,31] num:从控内单体温度序号[0,3]
** @Output    : 从控单体当前温度值:1℃
** @Function  : 获取指定从控内部单体当前温度值
**===========================================================================================*/
s16 GetBsuCellTemp(u8 clunum, u8 ecu, u8 num)
{
    u16 index = 0;
    //指针空判断
    if(gGCellTempInfo_1 == NULL || gGCellTempInfo_2 == NULL)
    {
        return (-410);
    }


    if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])       /*PACK节点号错误*/
        || (num >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]))/*一个PACK电池节数错误*/
    {
        return (0);
    }

    if(clunum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]) //簇数节点号错误
    {
        return (0);
    }

    /*从控内序号转换成组内序号*/
    index = ChangSlaveTidToGroupTid(ecu, num);

    /*电池组总单体温度数正确*/
    if(index < gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum])
    {
        if(clunum == 0)
            return(gGCellTempInfo_1[index]);
        else
            return(gGCellTempInfo_2[index]);
    }
    else
    {
        return(-410);
    }
}
/*=============================================================================================
** @Name      : void InputBsuCellTemp(u8 clunum, u8 ecu, u8 num, s16 temp)
** @Input     : ecu:从控节点号[0,31] num:从控内单体温度序号[0,3] temp:温度值(-40~126℃)
** @Output    : void
** @Function  : 输入接收到BSU单体温度值
** @The notes : 不包括均衡板温
**===========================================================================================*/
void InputBsuCellTemp(u8 clunum, u8 ecu, u8 num, s16 temp)
{
    u16 index = 0;
    //指针空判断
    if(gGCellTempInfo_1 == NULL || gGCellTempInfo_2 == NULL)
    {
        return;
    }
    if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])    /*PACK节点号错误*/
        || (num >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]))/*一个PACK电池节数错误*/
    {
        return;
    }

/*电池温序号正确*/
    if(num < (gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]))
    {
        /*从控内序号转换成组内序号*/
        index = ChangSlaveTidToGroupTid(ecu, num);

        /*电池温度序号越界*/
        if(index >= gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum])
        {
            return;
        }
    }
    else
    {
        return;
    }

    if(temp <= -410)
    {
        temp = -410;
    }
    else
    {
        /*非单体温度清零才进行单体温度缺失检测*/
        DiagRcvCellMissCheckAPI(clunum, 1, ecu, num);
    }

    if(clunum == 0)
    {
        gGCellTempInfo_1[index] = temp;
    }
    else
    {
        gGCellTempInfo_2[index] = temp;
    }
}


/*=================================================================================================
** @Name      : u16 ChangSlaveTidToGroupTid(u8 slave, u8 cell)
** @Input     : slave:从控号[0,31] cell:从控内电池号[0,17]
** @Output    : 从控内温度序号对应组内温度序号[0,399],0xffff:错误序号
** @Function  : 将从控内温度序号转换成电池组内温度序号
** @The notes : 每个从控电压数目不一样的需要特殊处理
**===============================================================================================*/
u16 ChangSlaveTidToGroupTid(u8 slave, u8 cell)
{
	u16 index = 0;

	//兼容CATL液冷PACK电池52串解析(16+18+18)
	#if(4 == PRJ_PACK_CELL_INFO)
	{
		//每3个第一个只有16节,越界非法
		if((0 == (slave % 3)) && (cell >= 16))
		{
			//错误序号
			index = 0xffff;
		}
		//每3个从控首个少2节(即第2个起每3个从控(52S)前移2S)
		else if(slave > 0)
		{
			//index = (slave - 1) * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum];
			index = (slave - 1) * 18;
			index = index - (index / 54 * 2) + cell + 16;
		}
		else
		{
			//index = slave * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] + cell;
			index = cell;
		}
	}
	#else
	{
        index = slave * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] + cell;
	}
    #endif

	return(index);
}

/*=============================================================================================
** @Name      : void InputSlaveChgBalanState(u16 cell, u16 state)
** @Input     : cell:组内起始电池号[0,415] state:连续16节电池充电均衡开启状态值[0关闭 1:开启(均充)]
** @Output    : void
** @Function  : 输入接收到从机充电均衡状态值(支持52串跨界拼接)
** @The notes : 按连续16Bit位表示连续16节电池顺序，采用掩码，避免跨从控覆盖
**===========================================================================================*/
void InputSlaveChgBalanState(u8 clunum, u16 cell, u16 state)
{
    u8 index = 0;
    u8 cellBit = 0;
    u16 BsuCellMaxNum = 0;
    u8 validBits = 16;
    u16 mask1 = 0;
    u16 mask2 = 0;

    /*起始电池号错误*/
    if((clunum >= CLUTER_MAX_NUM) || (cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]))
    {
        return;
    }

    /*获取单从控(Pack)最大电芯节数*/
    BsuCellMaxNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];
    if (BsuCellMaxNum == 0U)
    {
        return;
    }
    /*计算当前起始电池号在当前从控内的相对位置*/
    u16 cellInPack = cell % BsuCellMaxNum;

    /*若当前报文已达本从控末尾，且剩余有效电芯不足16节(如52串时剩4节)*/
    if (BsuCellMaxNum - cellInPack < 16)
    {
        /*只截取有效位数，丢弃底层报文凑数的无效0*/
        validBits = (u8)(BsuCellMaxNum - cellInPack);
        state &= ((1 << validBits) - 1);
    }

    /*计算在全局均衡数组中的位置与位偏移*/
    index = cell / 16;
    cellBit = cell % 16;

    /*索引未越界*/
    if(index < GINDEX90_LEN)
    {
        u8 bitsInFirst = 16 - cellBit;
        if (bitsInFirst > validBits) 
        {
            bitsInFirst = validBits;
        }
        
        /*生成掩码：只清空属于当前报文的bit位，保护原有的相邻数据不被覆盖*/
        mask1 = ((1 << bitsInFirst) - 1) << cellBit;

        /* 根据簇号分发到不同的数组 */
        if (clunum == 0) {
            gGChgBalanInfo_1[index] = (gGChgBalanInfo_1[index] & ~mask1) | ((state << cellBit) & mask1);
        } else {
            gGChgBalanInfo_2[index] = (gGChgBalanInfo_2[index] & ~mask1) | ((state << cellBit) & mask1);
        }

        /*跨数组元素情况*/
        if ((validBits > bitsInFirst) && ((index + 1) < GINDEX90_LEN))
        {
            u8 bitsInSecond = validBits - bitsInFirst;
            mask2 = (1 << bitsInSecond) - 1;
            if (clunum == 0) {
                gGChgBalanInfo_1[index + 1] = (gGChgBalanInfo_1[index + 1] & ~mask2) | ((state >> bitsInFirst) & mask2);
            } else {
                gGChgBalanInfo_2[index + 1] = (gGChgBalanInfo_2[index + 1] & ~mask2) | ((state >> bitsInFirst) & mask2);
            }
        }
    }
}

/*=================================================================================================
** @Name      : u8 GetGDhgBalanCellStateAPI(u8 clunum, u16 cell)
** @Function  : 获取从控回报的单体放电均衡状态
** @The notes : 状态由InputSlaveDhgBalanState更新；无效索引或未初始化时返回关闭。
**===============================================================================================*/
u8 GetGDhgBalanCellStateAPI(u8 clunum, u16 cell)
{
    u16 index = cell / 16U;
    u8 bit = (u8)(cell % 16U);
    u16 *stateInfo = NULL;

    if((clunum >= CLUTER_MAX_NUM)
        || (cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
        || (index >= GINDEX91_LEN))
    {
        return 0U;
    }

    stateInfo = (clunum == 0U) ? gGDhgBalanInfo_1 : gGDhgBalanInfo_2;
    if(stateInfo == NULL)
    {
        return 0U;
    }

    return (u8)((stateInfo[index] >> bit) & 0x0001U);
}

/*=================================================================================================
** @Name      : u8 GetGDhgBalanStateRcvAPI(u8 clunum, u8 slave)
** @Function  : 获取从控放电均衡状态是否已接收
** @The notes : 仅当该从控在时效期内有有效的放电均衡状态回报时返回1，避免历史回报掩盖通讯中断。
**===============================================================================================*/
u8 GetGDhgBalanStateRcvAPI(u8 clunum, u8 slave)
{
    u32 lastTick = 0U;

    if((clunum >= CLUTER_MAX_NUM) || (slave >= MAX_BSU_NUM))
    {
        return 0U;
    }

    lastTick = sGDhgBalanStateLastRcvTick[clunum][slave];
    if(lastTick == 0U)
    {
        return 0U;
    }

    return ((u32)(osKernelGetTickCount() - (lastTick - 1U)) <= DHG_BALAN_STATE_RCV_TIMEOUT_MS) ? 1U : 0U;
}

/*=============================================================================================
** @Name      : void InputSlaveDhgBalanState(u8 cell, u8 state)
** @Input     : cell:组内电池号[0,399] state:连续16节电池放电均衡开启状态值[0关闭 1:开启(均放)]
** @Output    : void
** @Function  : 输入接收到从机放电均衡状态值
** @The notes : 按连续16Bit位表示连续16节电池顺序(非一个数组索引表示1个从控)
**===========================================================================================*/
void InputSlaveDhgBalanState(u8 clunum, u16 cell, u16 state)
{
    u8 index = 0;
    u8 cellBit = 0;
    u16 BsuCellMaxNum = 0;
    u8 validBits = 16;
    u16 mask1 = 0;
    u16 mask2 = 0;

    /*起始电池号错误*/
    if((clunum >= CLUTER_MAX_NUM) || (cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]))
    {
        return;
    }

    /*获取单从控(Pack)最大电芯节数*/
    BsuCellMaxNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];
    if (BsuCellMaxNum == 0U)
    {
        return;
    }

    if((cell / BsuCellMaxNum) >= MAX_BSU_NUM)
    {
        return;
    }

    /*仅放电均衡回报可刷新诊断的状态时效。保存tick+1以区分上电未接收。*/
    sGDhgBalanStateLastRcvTick[clunum][cell / BsuCellMaxNum] = osKernelGetTickCount() + 1U;

    /*计算当前起始电池号在当前从控内的相对位置*/
    u16 cellInPack = cell % BsuCellMaxNum;
    if (BsuCellMaxNum - cellInPack < 16)
    {
        validBits = (u8)(BsuCellMaxNum - cellInPack);
        state &= ((1 << validBits) - 1);
    }

    index = cell / 16;
    cellBit = cell % 16;

    if(index < GINDEX91_LEN)
    {
        u8 bitsInFirst = 16 - cellBit;
        if (bitsInFirst > validBits) bitsInFirst = validBits;
        
        mask1 = ((1 << bitsInFirst) - 1) << cellBit;
       /* 根据簇号分发到不同的数组 */
        if (clunum == 0) {
            gGDhgBalanInfo_1[index] = (gGDhgBalanInfo_1[index] & ~mask1) | ((state << cellBit) & mask1);
        } else {
            gGDhgBalanInfo_2[index] = (gGDhgBalanInfo_2[index] & ~mask1) | ((state << cellBit) & mask1);
        }

        if ((validBits > bitsInFirst) && ((index + 1) < GINDEX91_LEN))
        {
            u8 bitsInSecond = validBits - bitsInFirst;
            mask2 = (1 << bitsInSecond) - 1;
            if (clunum == 0) {
                gGDhgBalanInfo_1[index + 1] = (gGDhgBalanInfo_1[index + 1] & ~mask2) | ((state >> bitsInFirst) & mask2);
            } else {
                gGDhgBalanInfo_2[index + 1] = (gGDhgBalanInfo_2[index + 1] & ~mask2) | ((state >> bitsInFirst) & mask2);
            }
        }
    }
}

/*=================================================================================================
** @Name      : u16 GetDevSampInfoAPI(u8 index)
** @Input     : index:数组下标
** @Output    : 板级采样信息
** @Function  : 获取板级采样信息
** @The notes : eDevSampInfo14
**===============================================================================================*/
u16 GetDevSampInfoAPI(u8 index)
{
    if(gDevSampInfo_14 == NULL || index >= eDevSamp_Len)
    {
        return (0);
    }
    return(gDevSampInfo_14[index]);
}



/*=================================================================================================
** @Name      : u16 GetGProtectStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组保护状态[0,5]
** @Function  : 获取电池组当前保护状态
** @The notes : e_ProState
**===============================================================================================*/
u16 GetGProtectStateAPI(u8 clunum)
{
    if(gSysStaInfo_50 == NULL || clunum >= 2)
        return (0);
    if (clunum == 0)
    {
        return(gSysStaInfo_50[eSysStateInfo50_1SysPro]);
    }
    else
    {
        return(gSysStaInfo_50[eSysStateInfo50_2SysPro]);
    }
}
/*=================================================================================================
** @Name      : u16 GetGWorkStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组工作状态[0,6]
** @Function  : 获取电池组当前工作状态
** @The notes : e_WorkState
**===============================================================================================*/
u16 GetGWorkStateAPI(u8 clunum)
{
    if(gSysStaInfo_50 == NULL || clunum >= 2)
        return (0);
    if (clunum == 0)
    {
        return(gSysStaInfo_50[eSysStateInfo50_1SysWor]);
    }
    else
    {
        return(gSysStaInfo_50[eSysStateInfo50_2SysWor]);
    }
}
/*=================================================================================================
** @Name      : u16 GetGChgDhgStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充放电状态[0,2]
** @Function  : 获取电池组当前充放电状态
** @The notes : e_CurrState
**===============================================================================================*/
u16 GetGChgDhgStateAPI(u8 clunum)
{
    if(gSysStaInfo_50 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysStaInfo_50[eSysStateInfo50_1SysChg]); 
    }
    else
    {
        return(gSysStaInfo_50[eSysStateInfo50_2SysChg]); 
    }
}
/*=================================================================================================
** @Name      : u16 GetGWorkModeAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组工作模式[0,5]
** @Function  : 获取电池组当前工作模式
** @The notes : e_WorkMode
**===============================================================================================*/
u16 GetGWorkModeAPI(u8 clunum)
{
    if(gSysStaInfo_50 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysStaInfo_50[eSysStateInfo50_1SysMod]); 
    }
    else
    {
        return(gSysStaInfo_50[eSysStateInfo50_2SysMod]); 
    }
}
/*=================================================================================================
** @Name      : u16 GetGChgLimCurrAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 实时充电电流限值 0.1A
** @Function  : 获取电池组当前充电电流限值
** @The notes : 最大充电电流请求值
**===============================================================================================*/
u16 GetGChgLimCurrAPI(u8 clunum)
{
    if(gSysLimInfo_53 == NULL || clunum >= 2)
        return (0);   
    if(clunum == 0)
    {
        return(gSysLimInfo_53[eSysLimInfo53_1ChgC]);
    }
    else
    {
        return(gSysLimInfo_53[eSysLimInfo53_2ChgC]);
    }
}
/*=================================================================================================
** @Name      : u16 GetGDhgLimCurrAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 实时放电电流限值 0.1A
** @Function  : 获取电池组当前放电电流限值
** @The notes : 最大放电电流请求值
**===============================================================================================*/
u16 GetGDhgLimCurrAPI(u8 clunum)
{
    if(gSysLimInfo_53 == NULL || clunum >= 2)
        return (0);   
    if(clunum == 0)
    {
        return(gSysLimInfo_53[eSysLimInfo53_1DhgC]);
    }
    else
    {
        return(gSysLimInfo_53[eSysLimInfo53_2DhgC]);
    }
}
/*=============================================================================================
** @Name      : u16 GetGChgLimPowerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充电功率限值 0.1kW
** @Function  : 返回充电功率限值
** @The notes : 最大充电功率请求值
**===========================================================================================*/
u16 GetGChgLimPowerAPI(u8 clunum)
{
    if(gSysLimInfo_53 == NULL || clunum >= 2)
        return (0);  
    if(clunum == 0)
    {
        return(gSysLimInfo_53[eSysLimInfo53_1ChgP]);
    }
    else
    {
        return(gSysLimInfo_53[eSysLimInfo53_2ChgP]);
    }
}
/*=============================================================================================
** @Name      : u16 GetGDhgLimPowerAPI(void)
** @Input     : clunum:簇编号
** @Output    : 放电功率限值 0.1kW
** @Function  : 返回放电功率限值
** @The notes : 最大放电功率请求值
**===========================================================================================*/
u16 GetGDhgLimPowerAPI(u8 clunum)
{
    if(gSysLimInfo_53 == NULL || clunum >= 2)
        return (0);  
    if(clunum == 0)
    {
        return(gSysLimInfo_53[eSysLimInfo53_1DhgP]);
    }
    else
    {
        return(gSysLimInfo_53[eSysLimInfo53_2DhgP]);
    }
}
/*=============================================================================================
** @Name      : u16 GetGChgLimSumVAPI(void)
** @Input     : clunum:簇编号
** @Output    : 充电总压限值 0.1V
** @Function  : 返回充电总压限值
**===========================================================================================*/
u16 GetGChgLimSumVAPI(u8 clunum)
{
    if(gSysLimInfo_53 == NULL || clunum >= 2)
        return (0); 
    if(clunum == 0)
    {
        return(gSysLimInfo_53[eSysLimInfo53_1ChgV]);
    }
    else
    {
        return(gSysLimInfo_53[eSysLimInfo53_2ChgV]);
    }
}

/*=============================================================================================
** @Name      : u16 GetGDhgLimSumVAPI(void)
** @Input     : clunum:簇编号
** @Output    : 放电总压限值 0.1V
** @Function  : 返回放电总压限值
**===========================================================================================*/
u16 GetGDhgLimSumVAPI(u8 clunum)
{
    if(gSysLimInfo_53 == NULL || clunum >= 2)
        return (0); 
    if(clunum == 0)
    {
        return(gSysLimInfo_53[eSysLimInfo53_1DhgV]);
    }
    else
    {
        return(gSysLimInfo_53[eSysLimInfo53_2DhgV]);
    }
}
/*=============================================================================================
** @Name      : u16 GetGAlarmFaultInfoAPI(u8 clunum, u8 index)
** @Input     : clunum,簇编号 index:告警信息索引号[0,2]
** @Output    : 电池组告警信息
** @Function  : 返回电池组告警信息
** @The notes : 16位告警码
**===========================================================================================*/
u16 GetGAlarmFaultInfoAPI(u8 clunum, u8 index)
{
    if(gAlmFault1_57 == NULL || gAlmFault2_58 == NULL)
    {
        return (0);
    }
    if(clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        if(index < GINDEX57_LEN)
        {
            return(gAlmFault1_57[index]);
        }
        else
        {
            return(0);
        }
    }
    else
    {
        if(index < GINDEX58_LEN)
        {
            return(gAlmFault2_58[index]);
        }
        else
        {
            return(0);
        }
    }
}
/*=================================================================================================
** @Name      : u16 GetGSampSumVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组采样总电压 0.1V
** @Function  : 获取电池组采样总电压
** @The notes : 采样值
**===============================================================================================*/
u16 GetGSampSumVoltAPI(u8 clunum)
{
    if((gSysMainInfo_51 == NULL) || (clunum >= 2))
        return (0);
    if(clunum == 0)
    {
        return((u16)gSysMainInfo_51[eSysMainInfo51_1SumV]);
    }
    else
    {
        return((u16)gSysMainInfo_51[eSysMainInfo51_2SumV]);
    }
}

/*=================================================================================================
** @Name      : s16 GetGSampOutCurrAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组采样输出电流 0.1A
** @Function  : 获取电池组采样输出电流
** @The notes : 负号为充电
**===============================================================================================*/
s16 GetGSampOutCurrAPI(u8 clunum)
{
    if((gSysMainInfo_51 == NULL) || (clunum >= 2))
        return (0);
    if(clunum == 0)
    {
        return(gSysMainInfo_51[eSysMainInfo51_1OutC]);
    }
    else
    {
        return(gSysMainInfo_51[eSysMainInfo51_2OutC]);
    }
}

/*=================================================================================================
** @Name      : s16 GetGSampOutPowerAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组输出功率 0.1kW
** @Function  : 获取电池组输出功率值
** @The notes : 负号为充电
**===============================================================================================*/
s16 GetGSampOutPowerAPI(u8 clunum)
{
    if((gSysMainInfo_51 == NULL) || (clunum >= 2))
        return (0);
    if(clunum == 0)
    {
        return(gSysMainInfo_51[eSysMainInfo51_1OutP]);
    }
    else
    {
        return(gSysMainInfo_51[eSysMainInfo51_2OutP]);
    }
}
/*=================================================================================================
** @Name      : u16 GetGSampPreVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组外端预充电压 0.1V
** @Function  : 获取电池组外端预充电压值
** @The notes : 采样值
**===============================================================================================*/
u16 GetGSampPreVoltAPI(u8 clunum)
{
    if((gSysMainInfo_51 == NULL) || (clunum >= 2))
        return (0);
    if(clunum == 0)
    {
        return(gSysMainInfo_51[eSysMainInfo51_1PreV]);
    }
    else
    {
        return(gSysMainInfo_51[eSysMainInfo51_2PreV]);
    }
}
/*=================================================================================================
** @Name      : u16 GetGBattSocAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组SOC 0.1%
** @Function  : 获取电池组SOC值
**===============================================================================================*/
u16 GetGBattSocAPI(u8 clunum)
{
    if((gSysCapInfo_52 == NULL) || (clunum >= 2))
        return (0);
    if(clunum == 0)
    {
        return(gSysCapInfo_52[eSysCapInfo52_1SOC]);
    }
    else
    {
        return(gSysCapInfo_52[eSysCapInfo52_2SOC]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGBattSoeAPI(u8 clunum)
** @Input     : clunum：簇编号
** @Output    : 电池组SOE 0.1%
** @Function  : 获取电池组SOE值
**===============================================================================================*/
u16 GetGBattSoeAPI(u8 clunum)
{
    if((gSysCapInfo_52 == NULL) || (clunum >= 2))
        return (0);
    if(clunum == 0)
    {
        return(gSysCapInfo_52[eSysCapInfo52_1SOE]);
    }
    else
    {
        return(gSysCapInfo_52[eSysCapInfo52_2SOE]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGBattSohAPI(u8 clunum)
** @Input     : clunum：簇编号
** @Output    : 电池组SOH 0.1%
** @Function  : 获取电池组SOH值
**===============================================================================================*/
u16 GetGBattSohAPI(u8 clunum)
{
    if((gSysCapInfo_52 == NULL) || (clunum >= 2))
        return (0);
    if(clunum == 0)
    {
        return(gSysCapInfo_52[eSysCapInfo52_1SOH]);
    }
    else
    {
        return(gSysCapInfo_52[eSysCapInfo52_2SOH]);
    }
}
/*=================================================================================================
** @Name      : u16 GetGBattAllCapAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组总容量 0.1AH
** @Function  : 获取电池组总容量值
** @The notes : 实测值
**===============================================================================================*/
u16 GetGBattAllCapAPI(u8 clunum)
{
    if((gSysCapInfo_52 == NULL) || (clunum >= 2))
        return (0);
    if(clunum == 0)
    {
        return(gSysCapInfo_52[eSysCapInfo52_1AllCap]);
    }
    else
    {
        return(gSysCapInfo_52[eSysCapInfo52_2AllCap]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGCellSumVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组求和总电压 0.1V
** @Function  : 获取电池组求和总电压
** @The notes : 单体累加值
**===============================================================================================*/
u16 GetGCellSumVoltAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return((u16)gSysCharInfo_54[eSysCharnfo54_1SumV]);
    }
    else
    {
        return((u16)gSysCharInfo_54[eSysCharnfo54_2SumV]);
    }
}
/*=================================================================================================
** @Name      : u16 GetGCellAvgVoltAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组单体平均电压 1mV
** @Function  : 获取电池组单体平均电压
** @The notes : 单体累加平均值
**===============================================================================================*/
u16 GetGCellAvgVoltAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return((u16)gSysCharInfo_54[eSysCharnfo54_1AvgV]);
    }
    else
    {
        return((u16)gSysCharInfo_54[eSysCharnfo54_2AvgV]);
    }
}

/*=================================================================================================
** @Name      : s16 GetGCellAvgTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体平均温度 1℃
** @Function  : 获取电池组单体平均温度
** @The notes : 单体累加平均值
**===============================================================================================*/
s16 GetGCellAvgTempAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1AvgT]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2AvgT]);
    }
}

/*=================================================================================================
** @Name      : s16 GetGEnvirTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组环境温度 1℃
** @Function  : 获取电池组环境温度
** @The notes : 第1路外围温度
**===============================================================================================*/
s16 GetGEnvirTempAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1EnvT]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2EnvT]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGCellMaxVNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最大电压电池号[1,400]
** @Function  : 获取电池组单体最大电压电池号
**===============================================================================================*/
u16 GetGCellMaxVNumAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1MaxVNum]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2MaxVNum]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGCellMaxVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最大电压值 1mV
** @Function  : 获取电池组单体最大电压值
**===============================================================================================*/
u16 GetGCellMaxVoltAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1MaxVolt]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2MaxVolt]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGCellMinVNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最小电压电池号[1,400]
** @Function  : 获取电池组单体最小电压电池号
**===============================================================================================*/
u16 GetGCellMinVNumAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1MinVNum]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2MinVNum]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGCellMinVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最小电压值 1mV
** @Function  : 获取电池组单体最小电压值
**===============================================================================================*/
u16 GetGCellMinVoltAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1MinVolt]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2MinVolt]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGCellMaxTNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最高温度序号[1,416]
** @Function  : 获取电池组单体最高温度序号
**===============================================================================================*/
u16 GetGCellMaxTNumAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1MaxTNum]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2MaxTNum]);
    }
}

/*=================================================================================================
** @Name      : s16 GetGCellMaxTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最高温度值 0.1℃
** @Function  : 获取电池组单体最高温度值
**===============================================================================================*/
s16 GetGCellMaxTempAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1MaxTemp]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2MaxTemp]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGCellMinTNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最低温度序号[1,400]
** @Function  : 获取电池组单体最低温度序号
**===============================================================================================*/
u16 GetGCellMinTNumAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1MinTNum]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2MinTNum]);
    }
}

/*=================================================================================================
** @Name      : s16 GetGCellMinTempAPI(u8 clunum)
** @Input     : clunum：簇编号
** @Output    : 电池组单体最低温度值 1℃
** @Function  : 获取电池组单体最低温度值
**===============================================================================================*/
s16 GetGCellMinTempAPI(u8 clunum)
{
    if(gSysCharInfo_54 == NULL || clunum >= 2)
        return (0);
    if(clunum == 0)
    {
        return(gSysCharInfo_54[eSysCharnfo54_1MinTemp]);
    }
    else
    {
        return(gSysCharInfo_54[eSysCharnfo54_2MinTemp]);
    }
}
/*=================================================================================================
** @Name      : u16 GetGCanChgTimeAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组可充电时间 1min
** @Function  : 获取电池组可充电时间
**===============================================================================================*/
u16 GetGCanChgTimeAPI(u8 clunum)
{
    if(gGStatistTime_59 == NULL || clunum > 0)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistTime_59[eTime59_1CanChg]);
    }
    else
    {
        return(gGStatistTime_59[eTime59_2CanChg]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGCanDhgTimeAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组可放电时间 1min
** @Function  : 获取电池组可放电时间
**===============================================================================================*/
u16 GetGCanDhgTimeAPI(u8 clunum)
{
    if(gGStatistTime_59 == NULL || clunum > 0)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistTime_59[eTime59_1CanDhg]);
    }
    else
    {
        return(gGStatistTime_59[eTime59_2CanDhg]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGChgCycleTimesAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充放电循环次数 1次
** @Function  : 获取电池组充放电循环次数
**===============================================================================================*/
u16 GetGChgCycleTimesAPI(u8 clunum)
{
    if(gGStatistTime_59 == NULL || clunum > 0)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistTime_59[eTime59_1ChgCycle]);
    }
    else
    {
        return(gGStatistTime_59[eTime59_2ChgCycle]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGMonWarNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当月告警数目 1个
** @Function  : 获取电池组当月告警数目
**===============================================================================================*/
u16 GetGMonWarNumAPI(u8 clunum)
{
    if(gGStatistErr_60 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistErr_60[eErr60_1MonWarNum]);
    }
    else
    {
        return(gGStatistErr_60[eErr60_2MonWarNum]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGMonErrNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当月故障数目 1个
** @Function  : 获取电池组当月故障数目
**===============================================================================================*/
u16 GetGMonErrNumAPI(u8 clunum)
{
    if(gGStatistErr_60 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistErr_60[eErr60_1MonErrNum]);
    }
    else
    {
        return(gGStatistErr_60[eErr60_2MonErrNum]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGHisWarNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计告警数目 1个
** @Function  : 获取电池组累计告警数目
**===============================================================================================*/
u16 GetGHisWarNumAPI(u8 clunum)
{
    if(gGStatistErr_60 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistErr_60[eErr60_1HisWarNum]);
    }
    else
    {
        return(gGStatistErr_60[eErr60_2HisWarNum]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGHisErrNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计故障数目 1个
** @Function  : 获取电池组累计故障数目
**===============================================================================================*/
u16 GetGHisErrNumAPI(u8 clunum)
{
    if(gGStatistErr_60 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistErr_60[eErr60_1HisErrNum]);
    }
    else
    {
        return(gGStatistErr_60[eErr60_2HisErrNum]);
    }
}

/*=================================================================================================
** @Name      : u32 GetGHisErrSumNumAPI(void)
** @Input     : void
** @Output    : 电池组累计故障总数目 1个
** @Function  : 获取电池组累计故障总数目
** @The notes : 包括告警数目和故障数目
**===============================================================================================*/
u32 GetGHisErrSumNumAPI(u8 clunum)
{
    if(gGStatistErr_60 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistErr_60[eErr60_1HisErrNum] + gGStatistErr_60[eErr60_1HisWarNum]);
    }
    else
    {
        return(gGStatistErr_60[eErr60_2HisErrNum] + gGStatistErr_60[eErr60_2HisWarNum]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGRemainEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前剩余电能 0.1kWh
** @Function  : 获取电池组当前剩余电能
**===============================================================================================*/
u16 GetGRemainEnerAPI(u8 clunum)
{
    if(gCurrCapEner_55 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gCurrCapEner_55[eCurrCapEner55_1NowEner]);
    }
    else
    {
        return(gCurrCapEner_55[eCurrCapEner55_2NowEner]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGBattAllEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组电池总电能 0.1kWh
** @Function  : 获取电池组电池总电能
**===============================================================================================*/
u16 GetGBattAllEnerAPI(u8 clunum)
{
    if(gCurrCapEner_55 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gCurrCapEner_55[eCurrCapEner55_1AllEner]);
    }
    else
    {
        return(gCurrCapEner_55[eCurrCapEner55_2AllEner]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGNowChgCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前单次充电量 1Ah
** @Function  : 获取电池组当前单次充电量
**===============================================================================================*/
u16 GetGNowChgCapAPI(u8 clunum)
{
    if(gCurrCapEner_55 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gCurrCapEner_55[eCurrCapEner55_1ChgCap]);
    }
    else
    {
        return(gCurrCapEner_55[eCurrCapEner55_2ChgCap]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGNowDhgCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前单次放电量 1Ah
** @Function  : 获取电池组当前单次放电量
**===============================================================================================*/
u16 GetGNowDhgCapAPI(u8 clunum)
{
    if(gCurrCapEner_55 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gCurrCapEner_55[eCurrCapEner55_1DhgCap]);
    }
    else
    {
        return(gCurrCapEner_55[eCurrCapEner55_2DhgCap]);
    }
}

/*=================================================================================================
** @Name      : u32 GetGHisChgCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计充电电量 1Ah
** @Function  : 获取电池组累计充电电量
**===============================================================================================*/
u32 GetGHisChgCapAPI(u8 clunum)
{
    if(gHisCapEner_56 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gHisCapEner_56[eHisCapEner56_1HisChgCap]);
    }
    else
    {
        return(gHisCapEner_56[eHisCapEner56_2HisChgCap]);
    }
}

/*=================================================================================================
** @Name      : u32 GetGHisDhgCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计放电电量 1Ah
** @Function  : 获取电池组累计放电电量
**===============================================================================================*/
u32 GetGHisDhgCapAPI(u8 clunum)
{
    if(gHisCapEner_56 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gHisCapEner_56[eHisCapEner56_1HisDhgCap]);
    }
    else
    {
        return(gHisCapEner_56[eHisCapEner56_2HisDhgCap]);
    }
}

/*=================================================================================================
** @Name      : u32 GetGHisChgEnerAPI(u8 clunum)
** @Input     : clunum:簇标号
** @Output    : 电池组累计充电电能 0.1kWh
** @Function  : 获取电池组累计充电电能
**===============================================================================================*/
u32 GetGHisChgEnerAPI(u8 clunum)
{
    if(gHisCapEner_56 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gHisCapEner_56[eHisCapEner56_1HisChgEner]);
    }
    else
    {
        return(gHisCapEner_56[eHisCapEner56_2HisChgEner]);
    }
}

/*=================================================================================================
** @Name      : u32 GetGHisDhgEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计放电电能 0.1kWh
** @Function  : 获取电池组累计放电电能
**===============================================================================================*/
u32 GetGHisDhgEnerAPI(u8 clunum)
{
    if(gHisCapEner_56 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gHisCapEner_56[eHisCapEner56_1HisDhgEner]);
    }
    else
    {
        return(gHisCapEner_56[eHisCapEner56_2HisDhgEner]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGStatistNowDodAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前放电深度DOD 0.1%
** @Function  : 获取电池组当前放电深度DOD
**===============================================================================================*/
u16 GetGStatistNowDodAPI(u8 clunum)
{
    if(gGStatistDodInfo_62 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistDodInfo_62[eDodStat62_1NowDod]);
    }
    else
    {
        return(gGStatistDodInfo_62[eDodStat62_2NowDod]);
    }
}

/*=================================================================================================
** @Name      : u16 GetGStatistMaxDodAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组最大放电深度DOD 0.1%
** @Function  : 获取电池组最大放电深度DOD
**===============================================================================================*/
u16 GetGStatistMaxDodAPI(u8 clunum)
{
    if(gGStatistDodInfo_62 == NULL || clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        return(gGStatistDodInfo_62[eDodStat62_1MaxDod]);
    }
    else
    {
        return(gGStatistDodInfo_62[eDodStat62_2MaxDod]);
    }
}
/*=================================================================================================
** @Name      : u16 GetGCellVoltAPI(u8 clunum, u16 cell)
** @Input     : clunum:簇编号 cell:电池号[0,399]
** @Output    : 电池组单体电压值 1mV
** @Function  : 获取电池组单体电压值
**===============================================================================================*/
u16 GetGCellVoltAPI(u8 clunum, u16 cell)
{
    if(gGCellVoltInfo_1 == NULL || gGCellVoltInfo_2 == NULL)
    {
        return (0);
    }
    if(clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        if(cell < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
        {
            return(gGCellVoltInfo_1[cell]);
        }

        return(0);
    }
    else
    {
        if(cell < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
        {
            return(gGCellVoltInfo_2[cell]);
        }

        return(0);
    }
}

/*=================================================================================================
** @Name      : s16 GetGCellTempAPI(u8 clunum, u16 cell)
** @Input     : clunum:簇编号 cell:电池号[0,399]
** @Output    : 电池组单体温度值 1℃
** @Function  : 获取电池组单体温度值
**===============================================================================================*/
s16 GetGCellTempAPI(u8 clunum, u16 cell)
{
    if(gGCellTempInfo_1 == NULL || gGCellTempInfo_2 == NULL)
    {
        return (0);
    }
    if(clunum >= 2)
    {
        return (0);
    }
    if(clunum == 0)
    {
        if(cell < gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum])
        {
            return(gGCellTempInfo_1[cell]);
        }

        return(-410);
    }
    else
    {
        if(cell < gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum])
        {
            return(gGCellTempInfo_2[cell]);
        }

        return(-410);
    }
}
/*=================================================================================================
** @Name      : s16 GetGShuntSampTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组分流器温度采样值 0.1℃
** @Function  : 获取电池组分流器温度采样值
**===============================================================================================*/
s16 GetGShuntSampTempAPI(u8 clunum)
{
    if(gImiu4_Info[e_IMIU4_1]  == NULL || gImiu4_Info[e_IMIU4_2]  == NULL )
    {
        printf("gImiu4 *p is null\r\n");
        return (0);
    }
    if(clunum == 0)
    {
        return(gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_ShuntT]);
    }
    else
    {
        return(gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_ShuntT]);
    }
    
}

/*=================================================================================================
** @Name      : s16 GetGDcPwrSampTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 低压电源板温度采样值 1℃
** @Function  : 获取低压电源板温度采样值
** @The notes : 2026-09-15接入DCV_TH(37)判定;低压电源板温度通道待BSP/IMIU4采样补充,当前恒0℃(≤70℃回差区,恒清告警)
**===============================================================================================*/
s16 GetGDcPwrSampTempAPI(u8 clunum)
{
    return (0);
}
/*=================================================================================================
** @Name      : u8 GetImeu3BoardInfo1API(e_IMEU3BoardInfo1 index)
** @Input     : index:IMEU3采样信息1
** @Output    : IMEU3采样信息1
** @Function  : 获取IMEU3采样信息1信息值
**===============================================================================================*/
u8 GetImeu3BoardInfo1API(e_IMEU3BoardInfo1 index)
{
    if(gImeu3BoardInfo1 == NULL)
    {
        printf("gImeu3BoardInfo1 is null\r\n");
        return (0);
    }
	if(index < eIMEUBoardInfo1_Len)
	{
        return(gImeu3BoardInfo1[index]);
	}
	else
	{
		return(0);
	}
}
/*=================================================================================================
** @Name      : u16 GetImeu3BoardInfo2API(e_IMEU3BoardInfo2 index)
** @Input     : index:IMEU3采样信息2
** @Output    : IMEU3采样信息2
** @Function  : 获取IMEU3采样信息2信息值
**===============================================================================================*/
u16 GetImeu3BoardInfo2API(e_IMEU3BoardInfo2 index)
{
    if(gImeu3BoardInfo2 == NULL)
    {
        printf("gImeu3BoardInfo1 is null\r\n");
        return (0);
    }
	if(index < eIMEUBoardInfo2_Len)
	{
        return(gImeu3BoardInfo2[index]);
	}
	else
	{
		return(0);
	}
}
/*=================================================================================================
** @Name      : u8 GetGPerInsidMaxTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池柜内部最高环境温度值 1℃
** @Function  : 获取电池柜内部最高环境温度(IMEU3采集)
** @The notes : 舱1={NTC1,2,3}舱2={NTC4,5}(2026-09-14原理图更正,NTC3属舱1右部);51表℃+50偏移码,return处解码
**===============================================================================================*/
u8 GetGPerInsidMaxTempAPI(u8 clunum)
{
	u8 i = 0;
	u8 maxT = 0;
    if(clunum == 0)
    {
        maxT = gImeu3BoardInfo1[eIMEUBoardInfo1_Ntc1Tem];
    }else if(clunum == 1)
    {
        maxT = gImeu3BoardInfo1[eIMEUBoardInfo1_Ntc4Tem];
    }
	
	for(i = 1; i < 3; i++)
	{

        if(clunum == 0)
        {
            if(gImeu3BoardInfo1[eIMEUBoardInfo1_Ntc1Tem + i] > maxT)
            {
                maxT = gImeu3BoardInfo1[eIMEUBoardInfo1_Ntc1Tem + i];
            }
        }else if(clunum == 1)
        {
            if((i < 2) && (gImeu3BoardInfo1[eIMEUBoardInfo1_Ntc4Tem + i] > maxT))   /*舱2仅NTC4/5两探头*/
            {
                maxT = gImeu3BoardInfo1[eIMEUBoardInfo1_Ntc4Tem + i];
            }
        }
	}

	return((maxT >= 50) ? (maxT - 50) : 0);//移除50度偏移(负温钳0防u8下溢假高温)
}
/*=================================================================================================
** @Name      : u8 GetGPerInsidMaxHumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池柜内部最高环境湿度值 1℃
** @Function  : 获取电池柜内部最高环境湿度(IMEU3采集)
** @The notes : 多个湿度传感器取最大值,for循环
**===============================================================================================*/
u8 GetGPerInsidMaxHumAPI(u8 clunum)
{
	u8 i = 0;
	u8 maxH = 0;
    if(clunum >= 2)
    {
        return(0);
    }
	maxH = gImeu3BoardInfo1[eIMEUBoardInfo1_485Hum1];
	for(i = 1; i < 4; i++)
	{

        if(gImeu3BoardInfo1[eIMEUBoardInfo1_485Hum1 + (i * 2)] > maxH)
        {
            maxH = gImeu3BoardInfo1[eIMEUBoardInfo1_485Hum1 + (i * 2)];
        }

	}
	return(maxH);
}
/*=================================================================================================
** @Name      : u16 GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Len index)
** @Input     : index:IMEU3告警信息
** @Output    : IMEU3告警信息
** @Function  : 获取IMEU3告警信息值
**===============================================================================================*/
u16 GetImeu3AlarmInfoAPI(e_IMEU3AlmInfo index)
{
    if(gImeu3AlmInfo == NULL)
    {
        printf("gImeu3BoardInfo1 is null\r\n");
        return (0);
    }
	if(index < eIMEUAlmInfo_Len)
	{
        return(gImeu3AlmInfo[index]);
	}
	else
	{
		return(0);
	}
}
/*=================================================================================================
** @Name      : s16 GetImiu4SampInfoAPI(u8 iu4num, e_IMIUSampInfo index)
** @Input     : iu4num:imiu4序号 index:IMIU4采样信息
** @Output    : IMIU4采样信息
** @Function  : 获取IMIU4采样信息值
**===============================================================================================*/
s16 GetImiu4SampInfoAPI(u8 iu4num, e_IMIUSampInfo index)
{
    if(iu4num >= 2)
    {
        return (0);
    }
    if(gImiu4_Info[e_IMIU4_1]  == NULL)
    {
        printf("gImiu4_1SampInfo is null\r\n");
        return (0);
    }
    if(gImiu4_Info[e_IMIU4_2]  == NULL)
    {
        printf("gImiu4_2SampInfo is null\r\n");
        return (0);
    }
    if(iu4num == 0)
    {
        if(index < eIMIUSampInfo_Len)
        {
            return(gImiu4_Info[e_IMIU4_1]->SampInfo[index]);
        }
        else
        {
            return(0);
        }
    }
    else
    {
        if(index < eIMIUSampInfo_Len)
        {
            return(gImiu4_Info[e_IMIU4_2]->SampInfo[index]);
        }
        else
        {
            return(0);
        }
    }
}
/*=================================================================================================
** @Name      : u16 GetImiu4AlmInfoAPI(u8 iu4num, e_IMIUAlmInfo index)
** @Input     : iu4num:imiu4序号 index:IMIU4告警信息
** @Output    : IMIU4告警信息
** @Function  : 获取IMIU4告警信息值
**===============================================================================================*/
u16 GetImiu4AlmInfoAPI(u8 iu4num, e_IMIUAlmInfo index)
{
    if(iu4num >= 2)
    {
        return (0);
    }
    if(gImiu4_Info[e_IMIU4_1]  == NULL)
    {
        printf("gImiu4_1AlmInfo is null\r\n");
        return (0);
    }
    if(gImiu4_Info[e_IMIU4_2]  == NULL)
    {
        printf("gImiu4_2AlmInfo is null\r\n");
        return (0);
    }
    if(iu4num == 0)
    {
        if(index < eIMIUAlmInfo_Len)
        {
            return(gImiu4_Info[e_IMIU4_1]-> AlmFault[index]);
        }
        else
        {
            return(0);
        }
    }
    else
    {
        if(index < eIMIUAlmInfo_Len)
        {
            return(gImiu4_Info[e_IMIU4_2]-> AlmFault[index]);
        }
        else
        {
            return(0);
        }
    }
}

/*=================================================================================================
** @Name      : u16 GetImiu4WorkModeAPI(u8 iu4num)
** @Input     : iu4num:imiu4序号
** @Output    : IMIU4系统工作模式
** @Function  : 获取IMIU4系统工作模式(ICC67/68 sub17转发用,2026-09-14接入)
**===============================================================================================*/
u16 GetImiu4WorkModeAPI(u8 iu4num)
{
    if(iu4num >= 2)
    {
        return (0);
    }
    if(gImiu4_Info[e_IMIU4_1]  == NULL)
    {
        printf("gImiu4_1WorkMode is null\r\n");
        return (0);
    }
    if(gImiu4_Info[e_IMIU4_2]  == NULL)
    {
        printf("gImiu4_2WorkMode is null\r\n");
        return (0);
    }
    if(iu4num == 0)
    {
        return(gImiu4_Info[e_IMIU4_1]-> WorkMode);
    }
    else
    {
        return(gImiu4_Info[e_IMIU4_2]-> WorkMode);
    }
}
/*=============================================================================================
** @Name      : void InputBsuCellResis(u8 clunum, u8 ecu, u8 num, u16 resis)
** @Input     : clunum:第几簇 ecu:从控节点号[0,31] num:从控内电池号[0,15] resis:内阻值 1μΩ
** @Output    : void
** @Function  : 输入接收到从控单体内阻值
**===========================================================================================*/
void InputBsuCellResis(u8 clunum, u8 ecu, u8 num, u16 resis)
{
    u16 index = 0;
    if(gGCellResisInfo_1 == NULL || gGCellResisInfo_2 == NULL)
    {
        return;
    }
    if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])
        || (num >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]))
    {
        return;
    }
    if(clunum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || clunum >= CLUTER_MAX_NUM)
    {
        return;
    }

    index = ChangSlaveVidToGroupVid(ecu, num);

    if(index < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
    {
        if(clunum == 0)
        {
            gGCellResisInfo_1[index] = resis;
        }
        else
        {
            gGCellResisInfo_2[index] = resis;
        }
    }
}

/*=============================================================================================
** @Name      : void InputBsuAfeState(u8 clunum, u8 ecu, u8 afeIndex, u8 afeState)
** @Input     : clunum:第几簇 ecu:从控节点号[0,31] afeIndex:AFE序号[0,26] afeState:AFE状态值(4bit)
** @Output    : void
** @Function  : 输入接收到从控AFE状态值
** @The notes : 每个AFE IC占1个u8,状态值定义见e_AfeState
**===========================================================================================*/
void InputBsuAfeState(u8 clunum, u8 ecu, u8 afeIndex, u8 afeState)
{
    u16 index = 0;
    if(gGAfeStateInfo_1 == NULL || gGAfeStateInfo_2 == NULL)
    {
        return;
    }
    if(ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])
    {
        return;
    }
    if(clunum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || clunum >= CLUTER_MAX_NUM)
    {
        return;
    }

    if(gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] == 0)
    {
        return;
    }
    index = (u16)ecu * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] + afeIndex;

    if(index < GINDEX98_LEN)
    {
        if(clunum == 0)
        {
            gGAfeStateInfo_1[index] = afeState;
        }
        else
        {
            gGAfeStateInfo_2[index] = afeState;
        }
    }
}
/*=============================================================================================
** @Name      : void ClrPduInImeu3AllInfoAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 清除IMEU3信息
** @The notes : IMEU3信息接收中断时使用
**===========================================================================================*/
void ClrImeu3AllInfoAPI(void)
{
	u8 i = 0;
    if(gImeu3BoardInfo1 == NULL || gImeu3BoardInfo2 == NULL || gImeu3AlmInfo == NULL)
    {
        printf("gImeu3 *p is null\r\n");
        return;
    }
	for(i = 0; i < eIMEUBoardInfo1_Len; i++)
	{
		gImeu3BoardInfo1[i] = 0;
	}
	for(i = 0; i < eIMEUBoardInfo2_Len; i++)
	{
		gImeu3BoardInfo2[i] = 0;
	}
    for(i = 0; i < eIMEUAlmInfo_Len; i++)
	{
		gImeu3AlmInfo[i] = 0;
	}
}
/*=============================================================================================
** @Name      : void ClrImiu4AllInfoAPI(u8 iu4num)
** @Input     : iu4num:iu4序号
** @Output    : void
** @Function  : 清除IMIU4信息
** @The notes : IMIU4信息接收中断时使用
**===========================================================================================*/
void ClrImiu4AllInfoAPI(u8 iu4num)
{
    u8 i = 0;
    if(gImiu4_Info[e_IMIU4_1] == NULL || gImiu4_Info[e_IMIU4_2] == NULL )
    {
        printf("gImiu4 *p is null\r\n");
        return;
    }
    if(iu4num == 0)
    {
        for(i = 0; i < eIMIUSampInfo_Len; i++)
        {
            gImiu4_Info[e_IMIU4_1]->SampInfo[i] = 0;
        }
        for(i = 0; i < eIMIUAlmInfo_Len; i++)
        {
            gImiu4_Info[e_IMIU4_1]-> AlmFault[i] = 0;
        }
        /*清除由1#IMIU4告警帧生成的采样异常位，避免离线后残留旧故障。*/
        ClrRemoteCurrSampExpFlagAPI(0);
    }
    else if (iu4num == 1)
    {
        for(i = 0; i < eIMIUSampInfo_Len; i++)
        {
            gImiu4_Info[e_IMIU4_2]->SampInfo[i] = 0;
        }
        for(i = 0; i < eIMIUAlmInfo_Len; i++)
        {
            gImiu4_Info[e_IMIU4_2]-> AlmFault[i] = 0;
        }
        /*清除由2#IMIU4告警帧生成的采样异常位，避免离线后残留旧故障。*/
        ClrRemoteCurrSampExpFlagAPI(1);
    }
}

/*=================================================================================================
** CLI调试命令部分 - 文件末尾
**===============================================================================================*/
#if(1)

/*=================================================================================================
** @Name      : BmuDebugPrintPara
** @Input     : void
** @Output    : void
** @Function  : 打印主控参数信息（完整版）
** @The notes : 打印gGBmuGenPara_102、gGBmuHigLevPara_103、gGHardPara_104所有参数值
**===============================================================================================*/
static void BmuDebugPrintPara(void)
{
    u16 i;
    
    printf("\r\n=== BMU Parameter Info ===\r\n");
    
    /*打印主控通用参数值gGBmuGenPara_102（完整85个参数）*/
    if(gGBmuGenPara_102 != NULL)
    {
        printf("\r\n[gGBmuGenPara_102] Main Controller General Parameters (Total %d):\r\n", eBmuGenPara102_CfgFin);
        printf("  [0]  CfgFlag     = %d\r\n", gGBmuGenPara_102[eBmuGenPara102_CfgFlag]);
        printf("  [1]  BmuDev      = %d\r\n", gGBmuGenPara_102[eBmuGenPara102_BmuDev]);
        printf("  [2]  1NowSoc     = %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_1NowSoc]);
        printf("  [3]  2NowSoc     = %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_2NowSoc]);
        printf("  [4]  1NowSoe     = %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_1NowSoe]);
        printf("  [5]  2NowSoe     = %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_2NowSoe]);
        printf("  [6]  1ReaAllCap  = %d (0.1AH)\r\n", gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap]);
        printf("  [7]  2ReaAllCap  = %d (0.1AH)\r\n", gGBmuGenPara_102[eBmuGenPara102_2ReaAllCap]);
        printf("  [8]  1ReaAllEner = %d (0.1kWh)\r\n", gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner]);
        printf("  [9]  2ReaAllEner = %d (0.1kWh)\r\n", gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner]);
        printf("  [10] RateCap     = %d (0.1AH)\r\n", gGBmuGenPara_102[eBmuGenPara102_RateCap]);
        printf("  [11] DampNum     = %d\r\n", gGBmuGenPara_102[eBmuGenPara102_DampNum]);
        printf("  [12] VH2Lim      = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_VH2Lim]);
        printf("  [13] VH1Lim      = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_VH1Lim]);
        printf("  [14] VL2Lim      = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_VL2Lim]);
        printf("  [15] VL1Lim      = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_VL1Lim]);
        printf("  [16] ChgTH2Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgTH2Lim]);
        printf("  [17] ChgTH1Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgTH1Lim]);
        printf("  [18] DhgTH2Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgTH2Lim]);
        printf("  [19] DhgTH1Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgTH1Lim]);
        printf("  [20] ChgTL2Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgTL2Lim]);
        printf("  [21] ChgTL1Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgTL1Lim]);
        printf("  [22] DhgTL2Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgTL2Lim]);
        printf("  [23] DhgTL1Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgTL1Lim]);
        printf("  [24] SumVH2Lim   = %d (0.1V)\r\n", gGBmuGenPara_102[eBmuGenPara102_SumVH2Lim]);
        printf("  [25] SumVH1Lim   = %d (0.1V)\r\n", gGBmuGenPara_102[eBmuGenPara102_SumVH1Lim]);
        printf("  [26] SumVL2Lim   = %d (0.1V)\r\n", gGBmuGenPara_102[eBmuGenPara102_SumVL2Lim]);
        printf("  [27] SumVL1Lim   = %d (0.1V)\r\n", gGBmuGenPara_102[eBmuGenPara102_SumVL1Lim]);
        printf("  [28] ChgCH2Lim   = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim]);
        printf("  [29] ChgCH1Lim   = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgCH1Lim]);
        printf("  [30] DhgCH2Lim   = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgCH2Lim]);
        printf("  [31] DhgCH1Lim   = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgCH1Lim]);
        printf("  [32] ChgCMaxLim  = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgCMaxLim]);
        printf("  [33] DhgCMaxLim  = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgCMaxLim]);
        printf("  [34] ChgCFinLim  = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgCFinLim]);
        printf("  [35] DhgCFinLim  = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgCFinLim]);
        printf("  [36] ChgPMaxLim  = %d (0.1kW)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgPMaxLim]);
        printf("  [37] DhgPMaxLim  = %d (0.1kW)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgPMaxLim]);
        printf("  [38] ChgPRatLim  = %d (0.1kW)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgPRatLim]);
        printf("  [39] DhgPRatLim  = %d (0.1kW)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgPRatLim]);
        printf("  [40] Leak2Lim    = %d (1Ω/V)\r\n", gGBmuGenPara_102[eBmuGenPara102_Leak2Lim]);
        printf("  [41] Leak1Lim    = %d (1Ω/V)\r\n", gGBmuGenPara_102[eBmuGenPara102_Leak1Lim]);
        printf("  [42] Envir2Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_Envir2Lim]);
        printf("  [43] Envir1Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_Envir1Lim]);
        printf("  [44] Perip2Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_Perip2Lim]);
        printf("  [45] Perip1Lim   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_Perip1Lim]);
        printf("  [46] VDiff2SubV  = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_VDiff2SubV]);
        printf("  [47] VDiff1SubV  = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_VDiff1SubV]);
        printf("  [48] TDiff2SubT  = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_TDiff2SubT]);
        printf("  [49] TDiff1SubT  = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_TDiff1SubT]);
        printf("  [50] CapHig2Soc  = %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_CapHig2Soc]);
        printf("  [51] CapHig1Soc  = %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_CapHig1Soc]);
        printf("  [52] CapLow2Soc  = %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_CapLow2Soc]);
        printf("  [53] CapLow1Soc  = %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_CapLow1Soc]);
        printf("  [54] CapHig1Volt = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_CapHig1Volt]);
        printf("  [55] CapLow1Volt = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_CapLow1Volt]);
        printf("  [56] CVH2Clr     = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_CVH2Clr]);
        printf("  [57] CVH1Clr     = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_CVH1Clr]);
        printf("  [58] CVL2Clr     = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_CVL2Clr]);
        printf("  [59] CVL1Clr     = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_CVL1Clr]);
        printf("  [60] CTH2Clr     = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_CTH2Clr]);
        printf("  [61] CTH1Clr     = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_CTH1Clr]);
        printf("  [62] DTH2Clr     = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_DTH2Clr]);
        printf("  [63] DTH1Clr     = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_DTH1Clr]);
        printf("  [64] CTL2Clr     = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_CTL2Clr]);
        printf("  [65] CTL1Clr     = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_CTL1Clr]);
        printf("  [66] DTL2Clr     = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_DTL2Clr]);
        printf("  [67] DTL1Clr     = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_DTL1Clr]);
        printf("  [68] SVH2Clr     = %d (0.1V)\r\n", gGBmuGenPara_102[eBmuGenPara102_SVH2Clr]);
        printf("  [69] SVH1Clr     = %d (0.1V)\r\n", gGBmuGenPara_102[eBmuGenPara102_SVH1Clr]);
        printf("  [70] SVL2Clr     = %d (0.1V)\r\n", gGBmuGenPara_102[eBmuGenPara102_SVL2Clr]);
        printf("  [71] SVL1Clr     = %d (0.1V)\r\n", gGBmuGenPara_102[eBmuGenPara102_SVL1Clr]);
        printf("  [72] ChgCH2Clr   = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgCH2Clr]);
        printf("  [73] ChgCH1Clr   = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgCH1Clr]);
        printf("  [74] DhgCH2Clr   = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgCH2Clr]);
        printf("  [75] DhgCH1Clr   = %d (0.1A)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgCH1Clr]);
        printf("  [76] Envir2Clr   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_Envir2Clr]);
        printf("  [77] Envir1Clr   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_Envir1Clr]);
        printf("  [78] Perip2Clr   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_Perip2Clr]);
        printf("  [79] Perip1Clr   = %d (1C)\r\n", gGBmuGenPara_102[eBmuGenPara102_Perip1Clr]);
        printf("  [80] ChgLimSubSoc= %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgLimSubSoc]);
        printf("  [81] DhgLimSubSoc= %d (0.1%%)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgLimSubSoc]);
        printf("  [82] ChgLimClrV  = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_ChgLimClrV]);
        printf("  [83] DhgLimClrV  = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_DhgLimClrV]);
        printf("  [84] CellPlaUpV  = %d (1mV)\r\n", gGBmuGenPara_102[eBmuGenPara102_CellPlaUpV]);
    }
    else
    {
        printf("  gGBmuGenPara_102 is NULL\r\n");
    }
    
    /*打印主控高级参数值gGBmuHigLevPara_103（完整72个参数）*/
    if(gGBmuHigLevPara_103 != NULL)
    {
        printf("\r\n[gGBmuHigLevPara_103] Main Controller High Level Parameters (Total %d):\r\n", eBmuHigLevPara103_CfgFin);
        printf("  [0]  CfgFlag       = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_CfgFlag]);
        printf("  [1]  ClusterNum    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]);
        printf("  [2]  BsuNum        = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]);
        printf("  [3]  ConNum        = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]);
        printf("  [4]  BsuVNum       = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum]);
        printf("  [5]  BsuTNum       = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum]);
        printf("  [6]  BalTNum       = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_BalTNum]);
        printf("  [7]  VSumNum       = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]);
        printf("  [8]  TSumNum       = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum]);
        printf("  [9]  BTSumNum      = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_BTSumNum]);
        printf("  [10] ChgLimOffT    = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ChgLimOffT]);
        printf("  [11] DhgLimOffT    = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_DhgLimOffT]);
        printf("  [12] ChgProOffT    = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ChgProOffT]);
        printf("  [13] DhgProOffT    = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_DhgProOffT]);
        printf("  [14] ErrProOffT    = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ErrProOffT]);
        printf("  [15] CommProOffT   = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_CommProOffT]);
        printf("  [16] VL2PwrOffT    = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_VL2PwrOffT]);
        printf("  [17] VL1PwrOffT    = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_VL1PwrOffT]);
        printf("  [18] ErrPWrOffT    = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ErrPWrOffT]);
        printf("  [19] CommPwrOffT   = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_CommPwrOffT]);
        printf("  [20] FanOpenTemp   = %d (1C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_FanOpenTemp]);
        printf("  [21] FanCloseTemp  = %d (1C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_FanCloseTemp]);
        printf("  [22] HotOpenTemp   = %d (1C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_HotOpenTemp]);
        printf("  [23] HotCloseTemp  = %d (1C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_HotCloseTemp]);
        printf("  [24] ChgFinCVEn    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ChgFinCVEn]);
        printf("  [25] DhgFinCVEn    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_DhgFinCVEn]);
        printf("  [26] ChgTHLimEn    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ChgTHLimEn]);
        printf("  [27] DhgTHLimEn    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_DhgTHLimEn]);
        printf("  [28] ChgTLLimEn    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ChgTLLimEn]);
        printf("  [29] DhgTLLimEn    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_DhgTLLimEn]);
        printf("  [30] TSloVLimEn    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_TSloVLimEn]);
        printf("  [31] PwrPor       = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_PwrPor]);
        printf("  [32] IsoPort      = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_IsoPort]);
        printf("  [33] FusPort      = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_FusPort]);
        printf("  [34] SmogPort     = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_SmogPort]);
        printf("  [35] EmergPort    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_EmergPort]);
        printf("  [36] ThundPort    = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ThundPort]);
        printf("  [37] FirePort     = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_FirePort]);
        printf("  [38] GasPort      = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_GasPort]);
        printf("  [39] FanPort      = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_FanPort]);
        printf("  [40] LeakMonitType = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_LeakMonitType]);
        printf("  [41] AdhMonitType = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_AdhMonitType]);
        printf("  [42] MainSwhC     = %d (0.1A)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_MainSwhC]);
        printf("  [43] PChgSwhC     = %d (0.1A)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_PChgSwhC]);
        printf("  [44] PChgResis    = %d (1Ω)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_PChgResis]);
        printf("  [45] PChgPower    = %d (1W)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_PChgPower]);
        printf("  [46] SafeSubV     = %d (0.1V)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_SafeSubV]);
        printf("  [47] PChgFinV     = %d (0.1V)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_PChgFinV]);
        printf("  [48] PChgWaitT    = %d (0.1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_PChgWaitT]);
        printf("  [49] PChgOffT     = %d (0.1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_PChgOffT]);
        printf("  [50] SelfWaitT    = %d (0.1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_SelfWaitT]);
        printf("  [51] MSwhOffV     = %d (0.1V)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV]);
        printf("  [52] HigVOffV     = %d (0.1V)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_HigVOffV]);
        printf("  [53] SumDiffV     = %d (0.1V)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_SumDiffV]);
        printf("  [54] LedkRlyState = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_LedkRlyState]);
        printf("  [55] LeakMonitCycl= %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_LeakMonitCycl]);
        printf("  [56] LedkSampDelyT= %d (1ms)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_LedkSampDelyT]);
        printf("  [57] PgndVSampGapT= %d (1ms)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_PgndVSampGapT]);
        printf("  [58] AdhRlyState  = %d\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_AdhRlyState]);
        printf("  [59] AdhMonitCycl = %d (1s)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_AdhMonitCycl]);
        printf("  [60] Lk0SampDelyT = %d (1ms)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_Lk0SampDelyT]);
        printf("  [61] Lk1SampDelyT = %d (1ms)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_Lk1SampDelyT]);
        printf("  [62] Lk2SampDelyT = %d (1ms)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_Lk2SampDelyT]);
        printf("  [63] RlyCtrlGapT  = %d (1ms)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_RlyCtrlGapT]);
        printf("  [64] ChgHTDnFstT  = %d (1C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnFstT]);
        printf("  [65] ChgHTDnDifT  = %d (1C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnDifT]);
        printf("  [66] ChgHTDnRate  = %d (0.1%%/C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnRate]);
        printf("  [67] ChgHTDnMinC  = %d (0.1A)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_ChgHTDnMinC]);
        printf("  [68] DhgHTDnFstT  = %d (1C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnFstT]);
        printf("  [69] DhgHTDnDifT  = %d (1C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnDifT]);
        printf("  [70] DhgHTDnRate  = %d (0.1%%/C)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnRate]);
        printf("  [71] DhgHTDnMinC  = %d (0.1A)\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_DhgHTDnMinC]);
    }
    else
    {
        printf("  gGBmuHigLevPara_103 is NULL\r\n");
    }
    
    /*打印主控硬件参数配置信息gGHardPara_104*/
    if(gGHardPara_104 != NULL)
    {
        printf("\r\n[gGHardPara_104] Main Controller Hardware Parameters (Total %d):\r\n", eGHardPara104_Fin);
        for(i = 0; i < eGHardPara104_Fin; i++)
        {
            printf("  [%d] = %d\r\n", i, gGHardPara_104[i]);
        }
    }
    else
    {
        printf("  gGHardPara_104 is NULL\r\n");
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintImeu3
** @Input     : void
** @Output    : void
** @Function  : 打印IMEU3模块信息
** @The notes : 打印gImeu3BoardInfo1、gImeu3BoardInfo2、gImeu3AlmInfo数据
**===============================================================================================*/
static void BmuDebugPrintImeu3(void)
{
    u8 i;
    
    printf("\r\n=== IMEU3 Module Info ===\r\n");
    
    /*打印IMEU3板级信息1*/
    if(gImeu3BoardInfo1 != NULL)
    {
        printf("\r\n[gImeu3BoardInfo1] IMEU3 Board Info 1:\r\n");
        for(i = 0; i < eIMEUBoardInfo1_Len; i++)
        {
            printf("  [%d] = %d\r\n", i, gImeu3BoardInfo1[i]);
        }
    }
    else
    {
        printf("  gImeu3BoardInfo1 is NULL\r\n");
    }
    
    /*打印IMEU3板级信息2*/
    if(gImeu3BoardInfo2 != NULL)
    {
        printf("\r\n[gImeu3BoardInfo2] IMEU3 Board Info 2:\r\n");
        for(i = 0; i < eIMEUBoardInfo2_Len; i++)
        {
            printf("  [%d] = %d\r\n", i, gImeu3BoardInfo2[i]);
        }
    }
    else
    {
        printf("  gImeu3BoardInfo2 is NULL\r\n");
    }
    
    /*打印IMEU3告警信息*/
    if(gImeu3AlmInfo != NULL)
    {
        printf("\r\n[gImeu3AlmInfo] IMEU3 Alarm Info:\r\n");
        for(i = 0; i < eIMEUAlmInfo_Len; i++)
        {
            printf("  [%d] = %d\r\n", i, gImeu3AlmInfo[i]);
        }
    }
    else
    {
        printf("  gImeu3AlmInfo is NULL\r\n");
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintImiu4
** @Input     : void
** @Output    : void
** @Function  : 打印IMIU4模块信息
** @The notes : 打印gImiu4_1SampInfo、gImiu4_2SampInfo、gImiu4_1AlmInfo、gImiu4_2AlmInfo数据
**===============================================================================================*/
static void BmuDebugPrintImiu4(void)
{
    u8 i;
    
    printf("\r\n=== IMIU4 Module Info ===\r\n");
    
    /*打印1#IMIU4采样信息*/
    if(gImiu4_Info[e_IMIU4_1] != NULL)
    {
        printf("\r\n[gImiu4_1SampInfo] 1# IMIU4 Sample Info:\r\n");
        printf("  BatVolt     = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_BatVolt]);
        printf("  LinkVolt    = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_LinkVolt]);
        printf("  Hall5V      = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_Hall5V]);
        printf("  HallSamp    = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_HallSamp]);
        printf("  HallCurrL   = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_HallCurrL]);
        printf("  HallCurrH   = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_HallCurrH]);
        printf("  ShuntCurrL  = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_ShuntCurrL]);
        printf("  ShuntCurrH  = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_ShuntCurrH]);
        printf("  ShuntT      = %d (0.1C)\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_ShuntT]);
        printf("  PasInsu     = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_PasInsu]);
        printf("  NegInsu     = %d\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_NegInsu]);
        printf("  Ntc1T       = %d (1C)\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_Ntc1T]);
        printf("  Ntc2T       = %d (1C)\r\n", gImiu4_Info[e_IMIU4_1]->SampInfo[eIMIUSampInfo_Ntc2T]);
    }
    else
    {
        printf("  gImiu4_1SampInfo is NULL\r\n");
    }
    
    /*打印2#IMIU4采样信息*/
    if(gImiu4_Info[e_IMIU4_2] != NULL)
    {
        printf("\r\n[gImiu4_2SampInfo] 2# IMIU4 Sample Info:\r\n");
        printf("  BatVolt     = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_BatVolt]);
        printf("  LinkVolt    = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_LinkVolt]);
        printf("  Hall5V      = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_Hall5V]);
        printf("  HallSamp    = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_HallSamp]);
        printf("  HallCurrL   = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_HallCurrL]);
        printf("  HallCurrH   = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_HallCurrH]);
        printf("  ShuntCurrL  = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_ShuntCurrL]);
        printf("  ShuntCurrH  = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_ShuntCurrH]);
        printf("  ShuntT      = %d (0.1C)\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_ShuntT]);
        printf("  PasInsu     = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_PasInsu]);
        printf("  NegInsu     = %d\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_NegInsu]);
        printf("  Ntc1T       = %d (1C)\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_Ntc1T]);
        printf("  Ntc2T       = %d (1C)\r\n", gImiu4_Info[e_IMIU4_2]->SampInfo[eIMIUSampInfo_Ntc2T]);
    }
    else
    {
        printf("  gImiu4_2SampInfo is NULL\r\n");
    }
    
    /*打印1#IMIU4告警信息*/
    if(gImiu4_Info[e_IMIU4_1] != NULL)
    {
        printf("\r\n[gImiu4_1AlmInfo] 1# IMIU4 Alarm Info:\r\n");
        for(i = 0; i < eIMIUAlmInfo_Len; i++)
        {
            printf("  [%d] = 0x%04X\r\n", i, gImiu4_Info[e_IMIU4_1]-> AlmFault[i]);
        }
    }
    else
    {
        printf("  gImiu4_1AlmInfo is NULL\r\n");
    }
    
    /*打印2#IMIU4告警信息*/
    if(gImiu4_Info[e_IMIU4_2]-> AlmFault != NULL)
    {
        printf("\r\n[gImiu4_2AlmInfo] 2# IMIU4 Alarm Info:\r\n");
        for(i = 0; i < eIMIUAlmInfo_Len; i++)
        {
            printf("  [%d] = 0x%04X\r\n", i, gImiu4_Info[e_IMIU4_2]-> AlmFault[i]);
        }
    }
    else
    {
        printf("  gImiu4_2AlmInfo is NULL\r\n");
    }
}
/*=================================================================================================
** @Name      : BmuDebugPrintSysInfo
** @Input     : void
** @Output    : void
** @Function  : 打印系统整体信息
** @The notes : 打印gSysMainInfo_51(电压电流)、gSysCapInfo_52(SOC/SOH)数据
**===============================================================================================*/
static void BmuDebugPrintSysInfo(void)
{
    printf("\r\n=== System Main Info ===\r\n");
    
    /*打印系统整体信息gSysMainInfo_51*/
    if(gSysMainInfo_51 != NULL)
    {
        printf("\r\n[gSysMainInfo_51] System Main Info:\r\n");
        printf("  Cluster 1:\r\n");
        printf("    SumV  = %d (0.1V)\r\n", gSysMainInfo_51[eSysMainInfo51_1SumV]);
        printf("    OutC  = %d (0.1A)\r\n", gSysMainInfo_51[eSysMainInfo51_1OutC]);
        printf("    OutP  = %d (0.1kW)\r\n", gSysMainInfo_51[eSysMainInfo51_1OutP]);
        printf("    PreV  = %d (0.1V)\r\n", gSysMainInfo_51[eSysMainInfo51_1PreV]);
        printf("  Cluster 2:\r\n");
        printf("    SumV  = %d (0.1V)\r\n", gSysMainInfo_51[eSysMainInfo51_2SumV]);
        printf("    OutC  = %d (0.1A)\r\n", gSysMainInfo_51[eSysMainInfo51_2OutC]);
        printf("    OutP  = %d (0.1kW)\r\n", gSysMainInfo_51[eSysMainInfo51_2OutP]);
        printf("    PreV  = %d (0.1V)\r\n", gSysMainInfo_51[eSysMainInfo51_2PreV]);
    }
    else
    {
        printf("  gSysMainInfo_51 is NULL\r\n");
    }
    
    /*打印系统电量信息gSysCapInfo_52*/
    if(gSysCapInfo_52 != NULL)
    {
        printf("\r\n[gSysCapInfo_52] System Capacity Info:\r\n");
        printf("  Cluster 1:\r\n");
        printf("    SOC    = %d (0.1%%)\r\n", gSysCapInfo_52[eSysCapInfo52_1SOC]);
        printf("    SOE    = %d (0.1%%)\r\n", gSysCapInfo_52[eSysCapInfo52_1SOE]);
        printf("    SOH    = %d (0.1%%)\r\n", gSysCapInfo_52[eSysCapInfo52_1SOH]);
        printf("    AllCap = %d (0.1AH)\r\n", gSysCapInfo_52[eSysCapInfo52_1AllCap]);
        printf("  Cluster 2:\r\n");
        printf("    SOC    = %d (0.1%%)\r\n", gSysCapInfo_52[eSysCapInfo52_2SOC]);
        printf("    SOE    = %d (0.1%%)\r\n", gSysCapInfo_52[eSysCapInfo52_2SOE]);
        printf("    SOH    = %d (0.1%%)\r\n", gSysCapInfo_52[eSysCapInfo52_2SOH]);
        printf("    AllCap = %d (0.1AH)\r\n", gSysCapInfo_52[eSysCapInfo52_2AllCap]);
    }
    else
    {
        printf("  gSysCapInfo_52 is NULL\r\n");
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintCellInfo
** @Input     : void
** @Output    : void
** @Function  : 打印电芯特征信息
** @The notes : 打印gSysCharInfo_54(最大最小电压温度)数据
**===============================================================================================*/
static void BmuDebugPrintCellInfo(void)
{
    printf("\r\n=== Cell Characteristic Info ===\r\n");
    
    /*打印系统特征信息gSysCharInfo_54*/
    if(gSysCharInfo_54 != NULL)
    {
        printf("\r\n[gSysCharInfo_54] System Characteristic Info:\r\n");
        printf("  Cluster 1:\r\n");
        printf("    SumV    = %d (0.1V)\r\n", gSysCharInfo_54[eSysCharnfo54_1SumV]);
        printf("    AvgV    = %d (1mV)\r\n", gSysCharInfo_54[eSysCharnfo54_1AvgV]);
        printf("    AvgT    = %d (1C)\r\n", gSysCharInfo_54[eSysCharnfo54_1AvgT]);
        printf("    EnvT    = %d (1C)\r\n", gSysCharInfo_54[eSysCharnfo54_1EnvT]);
        printf("    MaxVNum = %d\r\n", gSysCharInfo_54[eSysCharnfo54_1MaxVNum]);
        printf("    MaxVolt = %d (1mV)\r\n", gSysCharInfo_54[eSysCharnfo54_1MaxVolt]);
        printf("    MinVNum = %d\r\n", gSysCharInfo_54[eSysCharnfo54_1MinVNum]);
        printf("    MinVolt = %d (1mV)\r\n", gSysCharInfo_54[eSysCharnfo54_1MinVolt]);
        printf("    MaxTNum = %d\r\n", gSysCharInfo_54[eSysCharnfo54_1MaxTNum]);
        printf("    MaxTemp = %d (1C)\r\n", gSysCharInfo_54[eSysCharnfo54_1MaxTemp]);
        printf("    MinTNum = %d\r\n", gSysCharInfo_54[eSysCharnfo54_1MinTNum]);
        printf("    MinTemp = %d (1C)\r\n", gSysCharInfo_54[eSysCharnfo54_1MinTemp]);
        printf("  Cluster 2:\r\n");
        printf("    SumV    = %d (0.1V)\r\n", gSysCharInfo_54[eSysCharnfo54_2SumV]);
        printf("    AvgV    = %d (1mV)\r\n", gSysCharInfo_54[eSysCharnfo54_2AvgV]);
        printf("    AvgT    = %d (1C)\r\n", gSysCharInfo_54[eSysCharnfo54_2AvgT]);
        printf("    EnvT    = %d (1C)\r\n", gSysCharInfo_54[eSysCharnfo54_2EnvT]);
        printf("    MaxVNum = %d\r\n", gSysCharInfo_54[eSysCharnfo54_2MaxVNum]);
        printf("    MaxVolt = %d (1mV)\r\n", gSysCharInfo_54[eSysCharnfo54_2MaxVolt]);
        printf("    MinVNum = %d\r\n", gSysCharInfo_54[eSysCharnfo54_2MinVNum]);
        printf("    MinVolt = %d (1mV)\r\n", gSysCharInfo_54[eSysCharnfo54_2MinVolt]);
        printf("    MaxTNum = %d\r\n", gSysCharInfo_54[eSysCharnfo54_2MaxTNum]);
        printf("    MaxTemp = %d (1C)\r\n", gSysCharInfo_54[eSysCharnfo54_2MaxTemp]);
        printf("    MinTNum = %d\r\n", gSysCharInfo_54[eSysCharnfo54_2MinTNum]);
        printf("    MinTemp = %d (1C)\r\n", gSysCharInfo_54[eSysCharnfo54_2MinTemp]);
    }
    else
    {
        printf("  gSysCharInfo_54 is NULL\r\n");
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintCellVoltAll
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印全部电芯电压值（一次性打印全部）
** @The notes : 用于查看所有电芯电压数据
**===============================================================================================*/
static void BmuDebugPrintCellVoltAll(u8 clunum)
{
    u16 i;
    u16 vSumNum;
    
    printf("\r\n=== Cell Voltage All (Cluster %d) ===\r\n", clunum);
    
    if(clunum >= 2)
    {
        printf("  Invalid cluster number\r\n");
        return;
    }
    
    if(gGCellVoltInfo_1 == NULL || gGCellVoltInfo_2 == NULL)
    {
        printf("  Cell voltage pointer is NULL\r\n");
        return;
    }
    
    if(gGBmuHigLevPara_103 == NULL)
    {
        printf("  Parameter pointer is NULL\r\n");
        return;
    }
    
    /*获取电压总数量*/
    vSumNum = gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum];
    printf("  Total Cell Num: %d\r\n", vSumNum);
    
    /*打印全部电压值*/
    for(i = 0; i < vSumNum; i++)
    {
        if(clunum == 0)
        {
            printf("  Cell[%d] = %d mV\r\n", i, gGCellVoltInfo_1[i]);
        }
        else
        {
            printf("  Cell[%d] = %d mV\r\n", i, gGCellVoltInfo_2[i]);
        }
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintCellTempAll
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印全部电芯温度值（一次性打印全部）
** @The notes : 用于查看所有电芯温度数据
**===============================================================================================*/
static void BmuDebugPrintCellTempAll(u8 clunum)
{
    u16 i;
    u16 tSumNum;
    
    printf("\r\n=== Cell Temperature All (Cluster %d) ===\r\n", clunum);
    
    if(clunum >= 2)
    {
        printf("  Invalid cluster number\r\n");
        return;
    }
    
    if(gGCellTempInfo_1 == NULL || gGCellTempInfo_2 == NULL)
    {
        printf("  Cell temperature pointer is NULL\r\n");
        return;
    }
    
    if(gGBmuHigLevPara_103 == NULL)
    {
        printf("  Parameter pointer is NULL\r\n");
        return;
    }
    
    /*获取温度总数量*/
    tSumNum = gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum];
    printf("  Total Temp Num: %d\r\n", tSumNum);
    
    /*打印全部温度值*/
    for(i = 0; i < tSumNum; i++)
    {
        if(clunum == 0)
        {
            printf("  Temp[%d] = %d C\r\n", i, gGCellTempInfo_1[i]);
        }
        else
        {
            printf("  Temp[%d] = %d C\r\n", i, gGCellTempInfo_2[i]);
        }
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintState
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印系统状态信息(主索引50) - 中文显示
** @The notes : 保护状态、工作状态、充放电状态、工作模式
**===============================================================================================*/
static void BmuDebugPrintState(u8 clunum)
{
    u16 proState, worState, chgState, worMode;
    const char* proStr, *worStr, *chgStr, *modStr;
    
    printf("\r\n=== 系统状态信息 (主索引50) - 组%d ===\r\n", clunum + 1);
    
    if(clunum >= 2)
    {
        printf("  错误: 簇编号超出范围\r\n");
        return;
    }
    
    if(gSysStaInfo_50 == NULL)
    {
        printf("  gSysStaInfo_50 未初始化\r\n");
        return;
    }
    
    if(clunum == 0)
    {
        proState = gSysStaInfo_50[eSysStateInfo50_1SysPro];
        worState = gSysStaInfo_50[eSysStateInfo50_1SysWor];
        chgState = gSysStaInfo_50[eSysStateInfo50_1SysChg];
        worMode  = gSysStaInfo_50[eSysStateInfo50_1SysMod];
    }
    else
    {
        proState = gSysStaInfo_50[eSysStateInfo50_2SysPro];
        worState = gSysStaInfo_50[eSysStateInfo50_2SysWor];
        chgState = gSysStaInfo_50[eSysStateInfo50_2SysChg];
        worMode  = gSysStaInfo_50[eSysStateInfo50_2SysMod];
    }
    
    switch(proState)
    {
        case 0: proStr = "未就绪"; break;
        case 1: proStr = "正常"; break;
        case 2: proStr = "充电截止"; break;
        case 3: proStr = "放电截止"; break;
        case 4: proStr = "充放电限制"; break;
        case 5: proStr = "充放电保护"; break;
        default: proStr = "未知"; break;
    }
    
    switch(worState)
    {
        case 0: worStr = "开机"; break;
        case 1: worStr = "待命"; break;
        case 2: worStr = "启动"; break;
        case 3: worStr = "运行"; break;
        case 4: worStr = "停机"; break;
        case 5: worStr = "故障"; break;
        case 6: worStr = "关机"; break;
        default: worStr = "未知"; break;
    }
    
    switch(chgState)
    {
        case 0: chgStr = "空闲"; break;
        case 1: chgStr = "充电"; break;
        case 2: chgStr = "放电"; break;
        default: chgStr = "未知"; break;
    }
    
    switch(worMode)
    {
        case 0: modStr = "自动"; break;
        case 1: modStr = "手动"; break;
        case 2: modStr = "紧急"; break;
        case 5: modStr = "调试"; break;
        default: modStr = "未知"; break;
    }
    
    printf("  保护状态[%d]:     %s\r\n", proState, proStr);
    printf("  工作状态[%d]:     %s\r\n", worState, worStr);
    printf("  充放电状态[%d]:   %s\r\n", chgState, chgStr);
    printf("  工作模式[%d]:     %s\r\n", worMode, modStr);
}

/*=================================================================================================
** @Name      : BmuDebugPrintLimit
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印系统限值信息(主索引53) - 中文显示
** @The notes : 充放电电流/功率/电压限值
**===============================================================================================*/
static void BmuDebugPrintLimit(u8 clunum)
{
    printf("\r\n=== 系统限值信息 (主索引53) - 组%d ===\r\n", clunum + 1);
    
    if(clunum >= 2)
    {
        printf("  错误: 簇编号超出范围\r\n");
        return;
    }
    
    if(gSysLimInfo_53 == NULL)
    {
        printf("  gSysLimInfo_53 未初始化\r\n");
        return;
    }
    
    if(clunum == 0)
    {
        printf("  允许充电电流限值: %d (0.1A) = %.1f A\r\n", gSysLimInfo_53[eSysLimInfo53_1ChgC], gSysLimInfo_53[eSysLimInfo53_1ChgC] * 0.1f);
        printf("  允许放电电流限值: %d (0.1A) = %.1f A\r\n", gSysLimInfo_53[eSysLimInfo53_1DhgC], gSysLimInfo_53[eSysLimInfo53_1DhgC] * 0.1f);
        printf("  允许充电功率限值: %d (0.1kW) = %.1f kW\r\n", gSysLimInfo_53[eSysLimInfo53_1ChgP], gSysLimInfo_53[eSysLimInfo53_1ChgP] * 0.1f);
        printf("  允许放电功率限值: %d (0.1kW) = %.1f kW\r\n", gSysLimInfo_53[eSysLimInfo53_1DhgP], gSysLimInfo_53[eSysLimInfo53_1DhgP] * 0.1f);
        printf("  允许充电电压限值: %d (0.1V) = %.1f V\r\n", gSysLimInfo_53[eSysLimInfo53_1ChgV], gSysLimInfo_53[eSysLimInfo53_1ChgV] * 0.1f);
        printf("  允许放电电压限值: %d (0.1V) = %.1f V\r\n", gSysLimInfo_53[eSysLimInfo53_1DhgV], gSysLimInfo_53[eSysLimInfo53_1DhgV] * 0.1f);
    }
    else
    {
        printf("  允许充电电流限值: %d (0.1A) = %.1f A\r\n", gSysLimInfo_53[eSysLimInfo53_2ChgC], gSysLimInfo_53[eSysLimInfo53_2ChgC] * 0.1f);
        printf("  允许放电电流限值: %d (0.1A) = %.1f A\r\n", gSysLimInfo_53[eSysLimInfo53_2DhgC], gSysLimInfo_53[eSysLimInfo53_2DhgC] * 0.1f);
        printf("  允许充电功率限值: %d (0.1kW) = %.1f kW\r\n", gSysLimInfo_53[eSysLimInfo53_2ChgP], gSysLimInfo_53[eSysLimInfo53_2ChgP] * 0.1f);
        printf("  允许放电功率限值: %d (0.1kW) = %.1f kW\r\n", gSysLimInfo_53[eSysLimInfo53_2DhgP], gSysLimInfo_53[eSysLimInfo53_2DhgP] * 0.1f);
        printf("  允许充电电压限值: %d (0.1V) = %.1f V\r\n", gSysLimInfo_53[eSysLimInfo53_2ChgV], gSysLimInfo_53[eSysLimInfo53_2ChgV] * 0.1f);
        printf("  允许放电电压限值: %d (0.1V) = %.1f V\r\n", gSysLimInfo_53[eSysLimInfo53_2DhgV], gSysLimInfo_53[eSysLimInfo53_2DhgV] * 0.1f);
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintEnergy
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印能量统计信息(主索引55/56) - 中文显示
** @The notes : 当前能量、累计能量
**===============================================================================================*/
static void BmuDebugPrintEnergy(u8 clunum)
{
    u8 offset;
    
    printf("\r\n=== 能量统计信息 (主索引55/56) - 组%d ===\r\n", clunum + 1);
    
    if(clunum >= 2)
    {
        printf("  错误: 簇编号超出范围\r\n");
        return;
    }
    
    offset = (clunum == 0) ? 0 : 4;
    
    printf("\r\n[主索引55] 当前能量统计:\r\n");
    if(gCurrCapEner_55 != NULL)
    {
        printf("  当前电能:     %d (0.1kWh) = %.1f kWh\r\n", gCurrCapEner_55[offset], gCurrCapEner_55[offset] * 0.1f);
        printf("  总电能:       %d (0.1kWh) = %.1f kWh\r\n", gCurrCapEner_55[offset+1], gCurrCapEner_55[offset+1] * 0.1f);
        printf("  单次充电电量: %d (1AH) = %d AH\r\n", gCurrCapEner_55[offset+2], gCurrCapEner_55[offset+2]);
        printf("  单次放电电量: %d (1AH) = %d AH\r\n", gCurrCapEner_55[offset+3], gCurrCapEner_55[offset+3]);
    }
    else
    {
        printf("  gCurrCapEner_55 未初始化\r\n");
    }
    
    printf("\r\n[主索引56] 累计能量统计:\r\n");
    if(gHisCapEner_56 != NULL)
    {
        printf("  累计充电电量: %lu (1AH)\r\n", gHisCapEner_56[offset]);
        printf("  累计放电电量: %lu (1AH)\r\n", gHisCapEner_56[offset+1]);
        printf("  累计充电电能: %lu (0.1kWh) = %.1f kWh\r\n", gHisCapEner_56[offset+2], gHisCapEner_56[offset+2] * 0.1f);
        printf("  累计放电电能: %lu (0.1kWh) = %.1f kWh\r\n", gHisCapEner_56[offset+3], gHisCapEner_56[offset+3] * 0.1f);
    }
    else
    {
        printf("  gHisCapEner_56 未初始化\r\n");
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintAlarm
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印故障告警信息(主索引57/58) - 中文显示
** @The notes : 告警信息、故障信息
**===============================================================================================*/
static void BmuDebugPrintAlarm(u8 clunum)
{
    u16 *almFault;
    u8 i;
    
    printf("\r\n=== 故障告警信息 (主索引%d) - 组%d ===\r\n", clunum == 0 ? 57 : 58, clunum + 1);
    
    if(clunum >= 2)
    {
        printf("  错误: 簇编号超出范围\r\n");
        return;
    }
    
    almFault = (clunum == 0) ? gAlmFault1_57 : gAlmFault2_58;
    
    if(almFault == NULL)
    {
        printf("  告警故障数组未初始化\r\n");
        return;
    }
    
    printf("\r\n[告警信息] (10组):\r\n");
    for(i = 0; i < 10; i++)
    {
        if(almFault[eAlmFault_Alm1 + i] != 0)
            printf("  告警%d: 0x%04X [有告警]\r\n", i+1, almFault[eAlmFault_Alm1 + i]);
        else
            printf("  告警%d: 0x%04X [无告警]\r\n", i+1, almFault[eAlmFault_Alm1 + i]);
    }
    
    printf("\r\n[故障信息] (8组):\r\n");
    for(i = 0; i < 8; i++)
    {
        if(almFault[eAlmFault_Fault1 + i] != 0)
            printf("  故障%d: 0x%04X [有故障]\r\n", i+1, almFault[eAlmFault_Fault1 + i]);
        else
            printf("  故障%d: 0x%04X [无故障]\r\n", i+1, almFault[eAlmFault_Fault1 + i]);
    }
    TestGetStartFailReason();
}

/*=================================================================================================
** @Name      : BmuDebugPrintTime
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印时间统计信息(主索引59) - 中文显示
** @The notes : 运行时长、充放电时间、循环次数
**===============================================================================================*/
static void BmuDebugPrintTime(u8 clunum)
{
    u8 offset;
    
    printf("\r\n=== 时间统计信息 (主索引59) - 组%d ===\r\n", clunum + 1);
    
    if(clunum >= 2)
    {
        printf("  错误: 簇编号超出范围\r\n");
        return;
    }
    
    if(gGStatistTime_59 == NULL)
    {
        printf("  gGStatistTime_59 未初始化\r\n");
        return;
    }
    
    offset = (clunum == 0) ? 0 : 4;
    
    printf("  系统运行时长:   %d 小时\r\n", gGStatistTime_59[offset]);
    printf("  可充电时间:     %d 分钟\r\n", gGStatistTime_59[offset+1]);
    printf("  可放电时间:     %d 分钟\r\n", gGStatistTime_59[offset+2]);
    printf("  充放电循环次数: %d 次\r\n", gGStatistTime_59[offset+3]);
}

/*=================================================================================================
** @Name      : BmuDebugPrintErrStat
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印故障统计信息(主索引60) - 中文显示
** @The notes : 单月/累计告警故障数目
**===============================================================================================*/
static void BmuDebugPrintErrStat(u8 clunum)
{
    u8 offset;
    
    printf("\r\n=== 故障统计信息 (主索引60) - 组%d ===\r\n", clunum + 1);
    
    if(clunum >= 2)
    {
        printf("  错误: 簇编号超出范围\r\n");
        return;
    }
    
    if(gGStatistErr_60 == NULL)
    {
        printf("  gGStatistErr_60 未初始化\r\n");
        return;
    }
    
    offset = (clunum == 0) ? 0 : 4;
    
    printf("  单月告警数目: %d 次\r\n", gGStatistErr_60[offset]);
    printf("  单月故障数目: %d 次\r\n", gGStatistErr_60[offset+1]);
    printf("  累计告警数目: %d 次\r\n", gGStatistErr_60[offset+2]);
    printf("  累计故障数目: %d 次\r\n", gGStatistErr_60[offset+3]);
}

/*=================================================================================================
** @Name      : BmuDebugPrintSos
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印安全等级SOS信息(主索引61) - 中文显示
** @The notes : 当前/预测安全等级及原因
**===============================================================================================*/
static void BmuDebugPrintSos(u8 clunum)
{
    u8 offset;
    u16 sosLevel;
    const char* sosStr;
    
    printf("\r\n=== 安全等级SOS (主索引61) - 组%d ===\r\n", clunum + 1);
    
    if(clunum >= 2)
    {
        printf("  错误: 簇编号超出范围\r\n");
        return;
    }
    
    if(gGStatistSosInfo_61 == NULL)
    {
        printf("  gGStatistSosInfo_61 未初始化\r\n");
        return;
    }
    
    offset = (clunum == 0) ? 0 : 4;
    
    sosLevel = gGStatistSosInfo_61[offset];
    switch(sosLevel)
    {
        case 0: sosStr = "安全(等级0)"; break;
        case 1: sosStr = "轻微异常(等级1)"; break;
        case 2: sosStr = "中度异常(等级2)"; break;
        case 3: sosStr = "严重异常(等级3)"; break;
        case 4: sosStr = "危险(等级4)"; break;
        default: sosStr = "未知"; break;
    }
    printf("  当前安全等级SOS: %d - %s\r\n", sosLevel, sosStr);
    printf("  SOS异常原因:      0x%04X\r\n", gGStatistSosInfo_61[offset+1]);
    
    sosLevel = gGStatistSosInfo_61[offset+2];
    switch(sosLevel)
    {
        case 0: sosStr = "安全(等级0)"; break;
        case 1: sosStr = "轻微异常(等级1)"; break;
        case 2: sosStr = "中度异常(等级2)"; break;
        case 3: sosStr = "严重异常(等级3)"; break;
        case 4: sosStr = "危险(等级4)"; break;
        default: sosStr = "未知"; break;
    }
    printf("  预测安全等级SOS: %d - %s\r\n", sosLevel, sosStr);
    printf("  预测SOS异常原因:  0x%04X\r\n", gGStatistSosInfo_61[offset+3]);
}

/*=================================================================================================
** @Name      : BmuDebugPrintDod
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印放电深度DOD信息(主索引62) - 中文显示
** @The notes : 最新/最大/平均放电深度
**===============================================================================================*/
static void BmuDebugPrintDod(u8 clunum)
{
    u8 offset;
    
    printf("\r\n=== 放电深度DOD (主索引62) - 组%d ===\r\n", clunum + 1);
    
    if(clunum >= 2)
    {
        printf("  错误: 簇编号超出范围\r\n");
        return;
    }
    
    if(gGStatistDodInfo_62 == NULL)
    {
        printf("  gGStatistDodInfo_62 未初始化\r\n");
        return;
    }
    
    offset = (clunum == 0) ? 0 : 4;
    
    printf("  最新放电深度DOD: %d (0.1%%) = %.1f %%\r\n", gGStatistDodInfo_62[offset], gGStatistDodInfo_62[offset] * 0.1f);
    printf("  最新放电起始DOD: %d (0.1%%) = %.1f %%\r\n", gGStatistDodInfo_62[offset+1], gGStatistDodInfo_62[offset+1] * 0.1f);
    printf("  最大放电深度DOD: %d (0.1%%) = %.1f %%\r\n", gGStatistDodInfo_62[offset+2], gGStatistDodInfo_62[offset+2] * 0.1f);
    printf("  平均放电深度DOD: %d (0.1%%) = %.1f %%\r\n", gGStatistDodInfo_62[offset+3], gGStatistDodInfo_62[offset+3] * 0.1f);
}

/**=================================================================================================
** @Name      : BmuDebugPrintHisDod
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印历史DOD记录环形缓冲信息 - 中文显示
** @The notes : 序号、次数、最近50条记录(含时间戳/量能差值,同协议mIndex15/16单条)
**===============================================================================================*/
static void BmuDebugPrintHisDod(u8 clunum)
{
    u8 i;
    u16 base;
    u16 id;
    u16 num;
    s16 slot;
    u16 roff;
    
    printf("\r\n=== 历史DOD记录(环形,协议推送mIndex%d) - 组%d ===\r\n", (0 == clunum) ? 15 : 16, clunum + 1);
    
    if(clunum >= 2)
    {
        printf("  错误: 簇编号超出范围\r\n");
        return;
    }
    
    if(gGHisDodRing == NULL)
    {
        printf("  gGHisDodRing 未初始化\r\n");
        return;
    }
    
    base = clunum * HISDOD_RING_LEN;
    id = gGHisDodRing[base + eHisDodRing_Id];
    num = (id < HISDOD_REC_NUM) ? id : HISDOD_REC_NUM;
    
    printf("  累计记录次数(Id): %d, 环形有效条数: %d\r\n", id, num);
    
    if(num > 0)
    {
        printf("  历史DOD记录(最新→最旧):\r\n");
        /*环形缓冲:最新记录Id=id,slot=(id-1)%N;逐条向前回溯*/
        for(i = 0; i < num; i++)
        {
            slot = (s16)(((s32)id - 1 - i) % HISDOD_REC_NUM);
            if(slot < 0)
            {
                slot += HISDOD_REC_NUM;
            }
            roff = base + eHisDodRing_Rec + (u16)slot * eHisDodRec_Len;
            printf("    [%2d] #%u %u-%02u-%02u %02u:%02u:%02u DOD=%u(%.1f%%) 放电量=%u 充电量=%u 放电能=%u 充电能=%u\r\n", i,
                   gGHisDodRing[roff + eHisDodRec_Id],
                   gGHisDodRing[roff + eHisDodRec_Y],
                   (gGHisDodRing[roff + eHisDodRec_MD] >> 8) & 0xff,
                   gGHisDodRing[roff + eHisDodRec_MD] & 0xff,
                   (gGHisDodRing[roff + eHisDodRec_HM] >> 8) & 0xff,
                   gGHisDodRing[roff + eHisDodRec_HM] & 0xff,
                   gGHisDodRing[roff + eHisDodRec_S],
                   gGHisDodRing[roff + eHisDodRec_Dod],
                   gGHisDodRing[roff + eHisDodRec_Dod] * 0.1f,
                   gGHisDodRing[roff + eHisDodRec_DhgCap],
                   gGHisDodRing[roff + eHisDodRec_ChgCap],
                   gGHisDodRing[roff + eHisDodRec_DhgEner],
                   gGHisDodRing[roff + eHisDodRec_ChgEner]);
        }
    }
    else
    {
        printf("  (无历史记录)\r\n");
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintSlaveInfo
** @Input     : clunum: cluster number (0/1)
** @Output    : void
** @Function  : Print slave PACK info for specified cluster - English display
** @The notes : Includes PACK temperature(94), status(95), and BSU parameters(102)
**===============================================================================================*/
static void BmuDebugPrintSlaveInfo(u8 clunum)
{
    u8 packId;
    u8 cfgBsuNum;

    if(clunum >= CLUTER_MAX_NUM)
    {
        printf("Invalid cluster number: %d\r\n", clunum);
        return;
    }

    if(gPackTemp_94 == NULL || gPackStat_95 == NULL || gGBsuGenPara_102 == NULL || gGBmuHigLevPara_103 == NULL)
    {
        printf("Slave info buffer is NULL!\r\n");
        return;
    }

    cfgBsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];

    printf("=== Cluster %d Slave PACK Info (PACK count: %d) ===\r\n", clunum, cfgBsuNum);

    for(packId = 0; packId < cfgBsuNum; packId++)
    {
        printf("--- PACK %d ---\r\n", packId);

        printf("  [94-PACK Temperature]\r\n");
        printf("  Pt1SelfTemp   = %d (0.1C)\r\n", gPackTemp_94[clunum][packId][ePackTemp94_Pt1SelfTemp]);
        printf("  Pt2SelfTemp   = %d (0.1C)\r\n", gPackTemp_94[clunum][packId][ePackTemp94_Pt2SelfTemp]);
        printf("  Pt1Temp       = %d (0.1C)\r\n", gPackTemp_94[clunum][packId][ePackTemp94_Pt1Temp]);
        printf("  Pt2Temp       = %d (0.1C)\r\n", gPackTemp_94[clunum][packId][ePackTemp94_Pt2Temp]);

        printf("  [95-PACK Status]\r\n");
        printf("  WaterSelfHz1  = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_WaterSelfHz1]);
        printf("  WaterSelfHz2  = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_WaterSelfHz2]);
        printf("  Pt5VPullUp    = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_Pt5VPullUp]);
        printf("  Out12VCheck   = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_Out12VCheck]);
        printf("  WaterHz1      = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_WaterHz1]);
        printf("  WaterHz2      = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_WaterHz2]);
        printf("  HvilPwmAmp    = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_HvilPwmAmp]);
        printf("  HvilPwmDuty   = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_HvilPwmDuty]);
        printf("  FgFreq        = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_FgFreq]);
        printf("  FgDuty        = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_FgDuty]);
        printf("  AeroConc      = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_AeroConc]);
        printf("  AeroType      = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_AeroType]);
        printf("  AeroState     = 0x%04X\r\n", gPackStat_95[clunum][packId][ePackStat95_AeroState]);
        printf("  AfeBotCount   = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_AfeBotCount]);
        printf("  AfeTopCount   = %d\r\n", gPackStat_95[clunum][packId][ePackStat95_AfeTopCount]);

        printf("  [102-BSU Parameters]\r\n");
        printf("  CfgFlag       = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_CfgFlag]);
        printf("  EcuId         = 0x%02X\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_EcuId]);
        printf("  SlaveNum      = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_SlaveNum]);
        printf("  DaisyType     = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_DaisyType]);
        printf("  DaisyNum      = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_DaisyNum]);
        printf("  SampVnum      = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_SampVnum]);
        printf("  SampTnum      = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_SampTnum]);
        printf("  TotalVnum     = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_TotalVnum]);
        printf("  TotalTnum     = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_TotalTnum]);
        printf("  PassiveDuty   = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_PassiveDuty]);
        printf("  ActiveDuty    = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_ActiveDuty]);
        printf("  BalanceMethod = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_BalanceMethod]);
        printf("  CANCtrlVer    = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_CANCtrlVer]);
        printf("  CANBaudRate   = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_CANBaudRate]);
        printf("  CellShieldEn  = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_CellShieldEn]);
        printf("  AFELowPowerMode = %d\r\n", gGBsuGenPara_102[clunum][packId][eGBsuPara102_AFELowPowerMode]);
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintMsdInfo
** @Input     : clunum: cluster number (0/1)
** @Output    : void
** @Function  : Print MSD alarm debug info (raw error bits + filter states)
** @The notes : Diagnostic tool for MSD alarm recovery issues
**=================================================================================================*/
static void BmuDebugPrintMsdInfo(u8 clunum)
{
    u8 packId, ecu;
    u8 cfgBsuNum;
    u32 msdBit;
    u8 msdAlarmId;
    e_ErrState nowState, filState;
    u16 timer;

    if(clunum >= CLUTER_MAX_NUM)
    {
        printf("Invalid cluster number: %d\r\n", clunum);
        return;
    }

    if(gErrFilTab == NULL)
    {
        printf("gErrFilTab is NULL!\r\n");
        return;
    }

    cfgBsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
    msdBit = DiagGetBsu02u1MsdErrorBitAPI();

    printf("=== Cluster %d MSD Alarm Debug ===\r\n", clunum);
    printf("sBsu02u1MsdErrBit = 0x%08X\r\n", msdBit);
    printf("\r\n");
    printf("PACK ecu AlarmID Bit NowState FilState Timer\r\n");
    printf("---- --- ------- --- -------- -------- -----\r\n");

    for(packId = 0; packId < cfgBsuNum; packId++)
    {
        ecu = packId + clunum * cfgBsuNum;
        msdAlarmId = eERR_ID_PACK1_MSD + packId * 7;
        nowState = DiagGetErrNowStateAPI(clunum, (e_ERR_ID)msdAlarmId);
        filState = DiagGetErrFilStateAPI(clunum, (e_ERR_ID)msdAlarmId);
        timer = gErrFilTab[clunum][msdAlarmId].timer;

        printf("  %d    %d    %3d     %d      %d         %d       %d\r\n",
               packId, ecu, msdAlarmId,
               (msdBit >> ecu) & 1,
               nowState, filState, timer);
    }
}

/*=================================================================================================
** @Name      : BmuDebugPrintGroupAll
** @Input     : clunum:簇编号(0/1)
** @Output    : void
** @Function  : 打印指定簇的所有整体信息 - 中文显示
** @The notes : 综合打印函数，包含主索引50-62的所有信息
**===============================================================================================*/
static void BmuDebugPrintGroupAll(u8 clunum)
{
    printf("\r\n");
    printf("################################################################\r\n");
    printf("# BMU02核间通信协议 - 组%d 整体信息 (主索引50-62)\r\n", clunum + 1);
    printf("################################################################\r\n");
    
    BmuDebugPrintState(clunum);
    BmuDebugPrintSysInfo();
    BmuDebugPrintCellInfo();
    BmuDebugPrintLimit(clunum);
    BmuDebugPrintEnergy(clunum);
    BmuDebugPrintAlarm(clunum);
    BmuDebugPrintTime(clunum);
    BmuDebugPrintErrStat(clunum);
    BmuDebugPrintSos(clunum);
    BmuDebugPrintDod(clunum);
    
    printf("\r\n");
    printf("################################################################\r\n");
    printf("# 整体信息打印完毕\r\n");
    printf("################################################################\r\n");
}
/*=================================================================================================
** @Name      : BmuDebugPrintHelp
** @Input     : void
** @Output    : void
** @Function  : 打印CLI命令帮助信息
** @The notes : 显示所有支持的调试子命令
**===============================================================================================*/
static void BmuDebugPrintHelp(void)
{
    printf("\r\n=== BMU整体信息调试命令 ===\r\n");
    printf("用法: bmu_info <命令> [参数]\r\n");
    printf("\r\n");
    printf("命令列表:\r\n");
    printf("  para          - 打印BMU全部参数 (102/103/104)\r\n");
    printf("  imeu3         - 打印IMEU3模块信息\r\n");
    printf("  imiu4         - 打印IMIU4模块信息\r\n");
    printf("  sys           - 打印系统整体信息(主索引51/52)\r\n");
    printf("  cell          - 打印电芯特征信息(主索引54)\r\n");
    printf("  volt <clu>    - 打印全部单体电压 (clu=0/1)\r\n");
    printf("  temp <clu>    - 打印全部单体温度 (clu=0/1)\r\n");
    printf("  state <clu>   - 打印系统状态信息(主索引50)\r\n");
    printf("  limit <clu>   - 打印系统限值信息(主索引53)\r\n");
    printf("  energy <clu>  - 打印能量统计信息(主索引55/56)\r\n");
    printf("  alarm <clu>   - 打印故障告警信息(主索引57/58)\r\n");
    printf("  time <clu>    - 打印时间统计信息(主索引59)\r\n");
    printf("  errstat <clu> - 打印故障统计信息(主索引60)\r\n");
    printf("  sos <clu>     - 打印安全等级SOS(主索引61)\r\n");
    printf("  dod <clu>     - 打印放电深度DOD(主索引62)\r\n");
    printf("  group <clu>   - 打印指定簇所有整体信息(主索引50-62)\r\n");
    printf("  slave <clu>   - 打印从控PACK信息(温度94/状态95/参数102)\r\n");
    printf("  msd <clu>     - MSD alarm debug (raw error bits + filter state)\r\n");
    printf("  all           - 打印全部信息(两组)\r\n");
    printf("  help          - 打印帮助信息\r\n");
    printf("\r\n");
    printf("说明: clu为簇编号，取值0(组1)或1(组2)\r\n");
    printf("\r\n");
}

/*=================================================================================================
** @Name      : bmu_info_main
** @Input     : argc:参数个数  argv:参数数组
** @Output    : CLI_FALSE:参数错误  0:执行成功
** @Function  : BMU信息调试CLI命令入口
** @The notes : 支持打印主控参数、IMEU3、IMIU4、系统信息、电芯信息等
**===============================================================================================*/
static int bmu_info_main(int argc, char *argv[])
{
    u8 clunum;
    
    /*检查参数有效性*/
    if(argc == 0)
    {
        BmuDebugPrintHelp();
        return CLI_FALSE;
    }
    
    /*根据参数执行对应打印*/
    if(!strcmp(argv[0], "para"))
    {
        BmuDebugPrintPara();
    }
    else if(!strcmp(argv[0], "imeu3"))
    {
        BmuDebugPrintImeu3();
    }
    else if(!strcmp(argv[0], "imiu4"))
    {
        BmuDebugPrintImiu4();
    }
    else if(!strcmp(argv[0], "sys"))
    {
        BmuDebugPrintSysInfo();
    }
    else if(!strcmp(argv[0], "cell"))
    {
        BmuDebugPrintCellInfo();
    }
    else if(!strcmp(argv[0], "state"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: state <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintState(clunum);
    }
    else if(!strcmp(argv[0], "limit"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: limit <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintLimit(clunum);
    }
    else if(!strcmp(argv[0], "energy"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: energy <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintEnergy(clunum);
    }
    else if(!strcmp(argv[0], "alarm"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: alarm <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintAlarm(clunum);
    }
    else if(!strcmp(argv[0], "time"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: time <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintTime(clunum);
    }
    else if(!strcmp(argv[0], "errstat"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: errstat <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintErrStat(clunum);
    }
    else if(!strcmp(argv[0], "sos"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: sos <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintSos(clunum);
    }
    else if(!strcmp(argv[0], "dod"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: dod <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintDod(clunum);
    }
    else if(!strcmp(argv[0], "his_dod"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: his_dod <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintHisDod(clunum);
    }
    else if(!strcmp(argv[0], "group"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: group <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintGroupAll(clunum);
    }
    else if(!strcmp(argv[0], "slave"))
    {
        if(argc < 2)
        {
            printf("错误: 需要1个参数\r\n");
            printf("用法: slave <clu>\r\n");
            printf("      clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintSlaveInfo(clunum);
    }
    else if(!strcmp(argv[0], "msd"))
    {
        u8 msdClunum = 0;
        if(argc >= 2)
        {
            msdClunum = (u8)strtoul(argv[1], NULL, 0);
        }
        BmuDebugPrintMsdInfo(msdClunum);
    }
    else if(!strcmp(argv[0], "volt"))
    {
        if(argc < 2)
        {
            printf("Error:  Need 1 parameter\r\n");
            printf("Usage: volt <clu>\r\n");
            printf("       clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintCellVoltAll(clunum);
    }
    else if(!strcmp(argv[0], "temp"))
    {
        if(argc < 2)
        {
            printf("Error: Need 1 parameter\r\n");
            printf("Usage: temp <clu>\r\n");
            printf("       clu=0/1\r\n");
            return CLI_FALSE;
        }
        clunum = (u8)strtoul(argv[1], NULL, 0);
        BmuDebugPrintCellTempAll(clunum);
    }
    else if(!strcmp(argv[0], "all"))
    {
        BmuDebugPrintPara();
        BmuDebugPrintImeu3();
        BmuDebugPrintImiu4();
        BmuDebugPrintSysInfo();
        BmuDebugPrintCellInfo();
        BmuDebugPrintGroupAll(0);
        BmuDebugPrintGroupAll(1);
        BmuDebugPrintSlaveInfo(0);
        BmuDebugPrintSlaveInfo(1);
    }
    else if(!strcmp(argv[0], "help"))
    {
        BmuDebugPrintHelp();
    }
    else
    {
        printf("Unknown command: %s\r\n", argv[0]);
        BmuDebugPrintHelp();
        return CLI_FALSE;
    }
    
    return 0;
}

static int bmu_para_main(int argc, char *argv[])
{
    u16 paraType, paraIndex, paraValue;
    u8 paraValue8;
    u16 eepAddr;
    u8 result;
    
    if(argc < 4)
    {
        printf("\r\n=== BMU参数设置命令 ===\r\n");
        printf("用法: bmu_para set <type> <index> <value>\r\n");
        printf("\r\n");
        printf("参数类型 (type):\r\n");
        printf("  102 - 主控通用参数 (u16数组, 共85项)\r\n");
        printf("  103 - 主控高级参数 (u16数组, 共72项)\r\n");
        printf("  104 - 主控硬件参数 (u8数组, 共62项)\r\n");
        printf("\r\n");
        printf("示例:\r\n");
        printf("  bmu_para set 102 1 60     - 设置设备地址=60\r\n");
        printf("  bmu_para set 102 2 850    - 设置组1当前SOC=85.0%\r\n");
        printf("  bmu_para set 103 7 200    - 设置单体电压总数=200\r\n");
        printf("  bmu_para set 104 2 1      - 设置均衡策略算法=1\r\n");
        printf("\r\n");
        printf("查询参数列表:\r\n");
        printf("  bmu_info para            - 打印所有参数值\r\n");
        printf("\r\n");
        return CLI_FALSE;
    }
    
    if(strcmp(argv[0], "set") != 0)
    {
        printf("错误: 未知命令 '%s'\r\n", argv[0]);
        printf("支持的命令: set\r\n");
        return CLI_FALSE;
    }
    
    paraType = (u16)strtoul(argv[1], NULL, 0);
    paraIndex = (u16)strtoul(argv[2], NULL, 0);
    
    if(paraType == 102)
    {
        paraValue = (u16)strtoul(argv[3], NULL, 0);
        
        if(gGBmuGenPara_102 == NULL)
        {
            printf("错误: 参数数组未初始化\r\n");
            return CLI_FALSE;
        }
        
        if(paraIndex >= eBmuGenPara102_CfgFin)
        {
            printf("错误: 参数索引越界 (最大=%d)\r\n", eBmuGenPara102_CfgFin - 1);
            return CLI_FALSE;
        }
        
        eepAddr = PARA_DEVGEN_PARA_ADDR + (2 * paraIndex);
        if(ParaCheckGGenParaEffect(eepAddr, paraValue) == FALSE)
        {
            printf("错误: 参数值 %d 不在合理范围内\r\n", paraValue);
            printf("请参考 GroupPara.h 中各参数的范围定义\r\n");
            return CLI_FALSE;
        }
        
        gGBmuGenPara_102[paraIndex] = paraValue;
        result = ParaWriteStoreGroupGenParaInfo(paraIndex, &paraValue, 1);
        if(result == TRUE)
        {
            printf("成功: 设置主控通用参数[%d] = %d, 已保存到EEPROM\r\n", paraIndex, paraValue);
        }
        else
        {
            printf("警告: 设置主控通用参数[%d] = %d, EEPROM写入失败\r\n", paraIndex, paraValue);
        }
    }
    else if(paraType == 103)
    {
        paraValue = (u16)strtoul(argv[3], NULL, 0);
        
        if(gGBmuHigLevPara_103 == NULL)
        {
            printf("错误: 参数数组未初始化\r\n");
            return CLI_FALSE;
        }
        
        if(paraIndex >= eBmuHigLevPara103_CfgFin)
        {
            printf("错误: 参数索引越界 (最大=%d)\r\n", eBmuHigLevPara103_CfgFin - 1);
            return CLI_FALSE;
        }
        
        eepAddr = PARA_DEVHIG_PARA_ADDR + (2 * paraIndex);
        if(ParaCheckGHigParaEffect(eepAddr, paraValue) == FALSE)
        {
            printf("错误: 参数值 %d 不在合理范围内\r\n", paraValue);
            printf("请参考 GroupPara.h 中各参数的范围定义\r\n");
            return CLI_FALSE;
        }
        
        gGBmuHigLevPara_103[paraIndex] = paraValue;
        result = ParaWriteStoreGroupHigParaInfo(paraIndex, &paraValue, 1);
        if(result == TRUE)
        {
            printf("成功: 设置主控高级参数[%d] = %d, 已保存到EEPROM\r\n", paraIndex, paraValue);
        }
        else
        {
            printf("警告: 设置主控高级参数[%d] = %d, EEPROM写入失败\r\n", paraIndex, paraValue);
        }
    }
    else if(paraType == 104)
    {
        paraValue8 = (u8)strtoul(argv[3], NULL, 0);
        
        if(gGHardPara_104 == NULL)
        {
            printf("错误: 参数数组未初始化\r\n");
            return CLI_FALSE;
        }
        
        if(paraIndex >= eGHardPara104_Fin)
        {
            printf("错误: 参数索引越界 (最大=%d)\r\n", eGHardPara104_Fin - 1);
            return CLI_FALSE;
        }
        
        eepAddr = PARA_DEVHW_PARA_ADDR + paraIndex;
        if(ParaCheckGHWParaEffect(eepAddr, paraValue8) == FALSE)
        {
            printf("错误: 参数值 %d 不在合理范围内\r\n", paraValue8);
            printf("请参考 GroupPara.h 中各参数的范围定义\r\n");
            return CLI_FALSE;
        }
        
        gGHardPara_104[paraIndex] = paraValue8;
        result = ParaWriteStoreGroupHWParaInfo(paraIndex, &paraValue8, 1);
        if(result == TRUE)
        {
            printf("成功: 设置主控硬件参数[%d] = %d, 已保存到EEPROM\r\n", paraIndex, paraValue8);
        }
        else
        {
            printf("警告: 设置主控硬件参数[%d] = %d, EEPROM写入失败\r\n", paraIndex, paraValue8);
        }
    }
    else
    {
        printf("错误: 不支持的参数类型 %d\r\n", paraType);
        printf("支持的类型: 102/103/104/110/111/116\r\n");
        return CLI_FALSE;
    }
    
    return 0;
}

CLI_CMD("bmu_para", "BMU参数设置命令 (支持102/103/104)", bmu_para_main);

static int bmu_para_get_main(int argc, char *argv[])
{
    u16 paraType, paraIndex;
    
    if(argc < 2)
    {
        printf("\r\n=== BMU参数查询命令 ===\r\n");
        printf("用法: bmu_para_get <type> <index>\r\n");
        printf("\r\n");
        printf("参数类型 (type):\r\n");
        printf("  102 - 主控通用参数\r\n");
        printf("  103 - 主控高级参数\r\n");
        printf("  104 - 主控硬件参数\r\n");
        printf("\r\n");
        printf("示例:\r\n");
        printf("  bmu_para_get 102 1     - 查询设备地址\r\n");
        printf("  bmu_para_get 102 2     - 查询组1当前SOC\r\n");
        printf("  bmu_para_get 103 7     - 查询单体电压总数\r\n");
        printf("  bmu_para_get 104 2     - 查询均衡策略算法\r\n");
        printf("\r\n");
        return CLI_FALSE;
    }
    
    paraType = (u16)strtoul(argv[0], NULL, 0);
    paraIndex = (u16)strtoul(argv[1], NULL, 0);
    
    if(paraType == 102)
    {
        if(gGBmuGenPara_102 == NULL)
        {
            printf("错误: 参数数组未初始化\r\n");
            return CLI_FALSE;
        }
        
        if(paraIndex >= eBmuGenPara102_CfgFin)
        {
            printf("错误: 参数索引越界 (最大=%d)\r\n", eBmuGenPara102_CfgFin - 1);
            return CLI_FALSE;
        }
        
        printf("主控通用参数[%d] = %d\r\n", paraIndex, gGBmuGenPara_102[paraIndex]);
    }
    else if(paraType == 103)
    {
        if(gGBmuHigLevPara_103 == NULL)
        {
            printf("错误: 参数数组未初始化\r\n");
            return CLI_FALSE;
        }
        
        if(paraIndex >= eBmuHigLevPara103_CfgFin)
        {
            printf("错误: 参数索引越界 (最大=%d)\r\n", eBmuHigLevPara103_CfgFin - 1);
            return CLI_FALSE;
        }
        
        printf("主控高级参数[%d] = %d\r\n", paraIndex, gGBmuHigLevPara_103[paraIndex]);
    }
    else if(paraType == 104)
    {
        if(gGHardPara_104 == NULL)
        {
            printf("错误: 参数数组未初始化\r\n");
            return CLI_FALSE;
        }
        
        if(paraIndex >= eGHardPara104_Fin)
        {
            printf("错误: 参数索引越界 (最大=%d)\r\n", eGHardPara104_Fin - 1);
            return CLI_FALSE;
        }
        
        printf("主控硬件参数[%d] = %d\r\n", paraIndex, gGHardPara_104[paraIndex]);
    }
    else
    {
        printf("错误: 不支持的参数类型 %d\r\n", paraType);
        printf("支持的类型: 102/103/104/110/111/116\r\n");
        return CLI_FALSE;
    }
    
    return 0;
}

CLI_CMD("bmu_para_get", "BMU参数查询命令 (支持102/103/104/110/111)", bmu_para_get_main);

static int bmu_para_list_main(int argc, char *argv[])
{
    u16 paraType;
    u16 i, maxLen;
    
    if(argc < 1)
    {
        printf("\r\n=== BMU参数列表命令 ===\r\n");
        printf("用法: bmu_para_list <type>\r\n");
        printf("\r\n");
        printf("参数类型 (type):\r\n");
        printf("  102 - 主控通用参数 (85项)\r\n");
        printf("  103 - 主控高级参数 (72项)\r\n");
        printf("  104 - 主控硬件参数 (62项)\r\n");
        printf("  all  - 所有参数\r\n");
        printf("\r\n");
        return CLI_FALSE;
    }
    
    paraType = (u16)strtoul(argv[0], NULL, 0);
    
    if(paraType == 102)
    {
        printf("\r\n=== 主控通用参数102列表 ===\r\n");
        if(gGBmuGenPara_102 == NULL)
        {
            printf("参数数组未初始化\r\n");
            return CLI_FALSE;
        }
        maxLen = eBmuGenPara102_CfgFin;
        for(i = 0; i < maxLen; i++)
        {
            printf("[%2d] = %d\r\n", i, gGBmuGenPara_102[i]);
        }
        printf("共 %d 项\r\n", maxLen);
    }
    else if(paraType == 103)
    {
        printf("\r\n=== 主控高级参数103列表 ===\r\n");
        if(gGBmuHigLevPara_103 == NULL)
        {
            printf("参数数组未初始化\r\n");
            return CLI_FALSE;
        }
        maxLen = eBmuHigLevPara103_CfgFin;
        for(i = 0; i < maxLen; i++)
        {
            printf("[%2d] = %d\r\n", i, gGBmuHigLevPara_103[i]);
        }
        printf("共 %d 项\r\n", maxLen);
    }
    else if(paraType == 104)
    {
        printf("\r\n=== 主控硬件参数104列表 ===\r\n");
        if(gGHardPara_104 == NULL)
        {
            printf("参数数组未初始化\r\n");
            return CLI_FALSE;
        }
        maxLen = eGHardPara104_Fin;
        for(i = 0; i < maxLen; i++)
        {
            printf("[%2d] = %d\r\n", i, gGHardPara_104[i]);
        }
        printf("共 %d 项\r\n", maxLen);
    }
    else if(strcmp(argv[0], "all") == 0)
    {
        printf("\r\n=== 主控通用参数102列表 ===\r\n");
        if(gGBmuGenPara_102 != NULL)
        {
            maxLen = eBmuGenPara102_CfgFin;
            for(i = 0; i < maxLen; i++)
            {
                printf("[%2d] = %d\r\n", i, gGBmuGenPara_102[i]);
            }
        }
        
        printf("\r\n=== 主控高级参数103列表 ===\r\n");
        if(gGBmuHigLevPara_103 != NULL)
        {
            maxLen = eBmuHigLevPara103_CfgFin;
            for(i = 0; i < maxLen; i++)
            {
                printf("[%2d] = %d\r\n", i, gGBmuHigLevPara_103[i]);
            }
        }
        
        printf("\r\n=== 主控硬件参数104列表 ===\r\n");
        if(gGHardPara_104 != NULL)
        {
            maxLen = eGHardPara104_Fin;
            for(i = 0; i < maxLen; i++)
            {
                printf("[%2d] = %d\r\n", i, gGHardPara_104[i]);
            }
        }
    }
    else
    {
        printf("错误: 不支持的参数类型 %d\r\n", paraType);
        return CLI_FALSE;
    }
    
    return 0;
}

CLI_CMD("bmu_para_list", "BMU参数列表命令 (支持102/103/104/all)", bmu_para_list_main);

/*注册CLI命令: bmu_info*/
CLI_CMD("bmu_info", "BMU info debug commands", bmu_info_main);

/*=================================================================================================
** @Name      : bmu_clr_dod_main
** @Input     : argc:参数个数  argv:参数数组(argv[1]=簇编号)
** @Output    : CLI_FALSE:参数错误  0:执行成功
** @Function  : 清除DOD统计记录CLI命令(含EEPROM持久化)
** @The notes : 用于runtime测试时触发清零,等同于外部下发eCLR_DODRCD命令
**===============================================================================================*/
static int bmu_clr_dod_main(int argc, char *argv[])
{
    u8 clunum;
    
    if(argc < 2)
    {
        printf("用法: bmu_clr_dod <clu>\r\n");
        printf("      clu=0/1  清除指定簇DOD统计\r\n");
        return CLI_FALSE;
    }
    
    clunum = (u8)strtoul(argv[1], NULL, 0);
    
    if(clunum >= 2)
    {
        printf("错误: 簇编号超出范围 (0或1)\r\n");
        return CLI_FALSE;
    }
    
    printf("清除组%d DOD统计记录...\r\n", clunum + 1);
    GroupClrDodStatisRcdAPI(clunum);
    printf("DOD统计记录已清除(含EEPROM持久化)\r\n");
    
    return 0;
}

/*注册CLI命令: bmu_clr_dod*/
CLI_CMD("bmu_clr_dod", "清除DOD统计记录 (clu=0/1)", bmu_clr_dod_main);

/*=================================================================================================
** @Name      : extfree_handler
** @Input     : argc, argv (not used)
** @Output    : CLI_FALSE / 0
** @Function  : 查询外部堆剩余可用内存大小
** @The notes : 调用ExtxPortGetFreeHeapSize获取DDR外部堆的空闲字节数并打印
**===============================================================================================*/
static int extfree_handler(int argc, char *argv[])
{
    size_t freeSize = ExtxPortGetFreeHeapSize();
    printf("\r\n=== External Heap Info ===\r\n");
    printf("Free Size: %u bytes (%u KB)\r\n", (unsigned int)freeSize, (unsigned int)(freeSize / 1024));
    return 0;
}

CLI_CMD("extfree", "Show external heap free memory size", extfree_handler);

#endif

    
    
