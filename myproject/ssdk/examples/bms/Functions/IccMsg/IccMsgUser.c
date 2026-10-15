/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IccMsgUser.c
** @Founder      : WYC834
** @Date         : 2026.03.10
** @Function     : 核间通信包用户接口
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "IccMsgUser.h"
#include "IccMsgInterface.h"
#include "IccMsgHandle.h"
#include "GroupInfo.h"
#include "PublicPara.h"
#include "GroupPara.h"
#include "iccmsg_dpr.h"
#include "IccMsgCreate.h"
#include <string.h>
/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
//核间通信初始化全局变量标志
u8 iccMsgInitFlag = 0; //0:未初始化核减通信包，1：已初始化核减通信包
//各核间通信通道数据配置表
t_ICCCellDataDef *gIccMsgCellDataDef = NULL;           /*单体信息通道数据配置*/
t_ICCCycleDataDef *gIccMsgCycleDataDef = NULL;         /*周期信息通道数据配置*/
t_ICCFaqDataDef *gIccMsgFaqDataDef = NULL;             /*问答信息通道数据配置*/
//u8 *gIccMsgUpdataDataDef = NULL;        /*升级信息通道数据配置*/

//核间通信相关处理表
t_ICCIndexTab *Filed_BMUTab = NULL;                    /*核间通信信息主索引表*/
t_ICCDevTab* gIccDevTable = NULL;                      /*核间通信设备表*/
t_ICCTxRcvTab* gIccTxRcvTable = NULL;                  /*核间通信收发处理表*/
t_ICCTxTab* gIccTxCelTable = NULL;                  /*核间通信单体发送处理表*/
t_ICCTxTab* gIccTxCycTable = NULL;                  /*核间通信周期发送处理表*/

//核间通信配置信息
u8 *gIccBaseDef = NULL;                 /*基本配置信息*/

//核间通信发送缓冲区
u8 *txData = NULL;
//核间通信发送表读取缓冲区
u8 *readData = NULL;
//设备信息
extern u16 *gVerTime_0;              /*校时信息*/
extern u16 *gDevInfo_2;              /*设备信息*/
extern u8 *gSerialNum_3; 

//历史故障信息
extern u16 *gGHisErrInfo1_10;     /*组1历史故障信息*/
extern u16 *gGHisErrInfo2_11;     /*组2历史故障信息*/
extern u16 *gGHisEventInfo1_12;   /*组1历史事件信息*/
extern u16 *gGHisEventInfo2_13;   /*组2历史事件信息*/
extern u16 *gDevSampInfo_14;      /*板级采样信息*/
extern u16 *gGHisDodInfo1_15;   /*组1历史DOD记录信息(最新一条)*/
extern u16 *gGHisDodInfo2_16;   /*组2历史DOD记录信息(最新一条)*/
//电芯相关信息
extern u16 *gGCellVoltInfo_1;        /*组1电池单体电压信息*/
extern u16 *gGCellVoltInfo_2;        /*组2电池单体电压信息*/
extern s16 *gGCellTempInfo_1;        /*组1电池单体温度信息*/
extern s16 *gGCellTempInfo_2;        /*组2电池单体温度信息*/
extern u16 *gGCellResisInfo_1;       /*组1电池单体内阻信息*/
extern u16 *gGCellResisInfo_2;       /*组2电池单体内阻信息*/
extern u16 *gGChgBalanInfo_1;        /*组1充电均衡状态信息*/
extern u16 *gGChgBalanInfo_2;        /*组2充电均衡状态信息*/
extern u16 *gGDhgBalanInfo_1;        /*组1放电均衡状态信息*/
extern u16 *gGDhgBalanInfo_2;        /*组2放电均衡状态信息*/



extern u16 *gSysStaInfo_50;       /*系统状态信息*/
extern s16 *gSysMainInfo_51;      /*系统整体信息*/
extern u16 *gSysCapInfo_52;       /*系统电量信息*/
extern u16 *gSysLimInfo_53;       /*系统限值信息*/
extern s16 *gSysCharInfo_54;      /*系统特征信息*/
extern u16 *gCurrCapEner_55;      /*当前能量统计信息*/
extern u32 *gHisCapEner_56;       /*累计能量统计信息*/
extern u16 *gAlmFault1_57;        /*组1告警故障信息*/
extern u16 *gAlmFault2_58;        /*组2告警故障信息*/
extern u16 *gGStatistTime_59;     /*时间统计信息*/
extern u16 *gGStatistErr_60;      /*故障统计信息*/
extern u16 *gGStatistSosInfo_61;  /*安全等级SOS统计信息*/
extern u16 *gGStatistDodInfo_62;  /*放电深度统计信息*/
extern u16 *gGAuthorInfo_63;      /*权限级别状态信息*/
extern u16 *gGAuthorCtrl_64;      /*权限密码登录信息数组*/
extern u16 *gSysCtrlCmd_65;       /*组1系统控制命令*/
extern u16 *gSysCtrlCmd_66;       /*组2系统控制命令*/

//IMIU4相关信息
extern s16 *gImiu4Info_67;      /*1#IMIU4信息核间通信数组*/
extern s16 *gImiu4Info2_68;      /*2#IMIU4信息核间通信数组*/
extern u16 *gUserCtrlCmd_69;    /*用户控制命令*/
extern u16 *gCtrlCmdInfo_70;      /*控制命令反馈*/
extern u16 *gSysDbgCmd_71;      /*系统调试命令*/
extern u16 *gSampCalCmd_72;     /*采样校准命令*/
extern u16 *gSampCalFbInfo_73;     /*采样校准反馈信息*/
//BSU相关信息
extern u16 *gBsudPSampInfo_74;  /*BSU02U2板级及PACK采样信息*/
//IMEU3相关信息
extern u8  *gImeu3CfgSet_76;    /*IMEU3硬件参数配置值(ICC mIndex=76, u8×12)*/
extern u16 *gImeu3Info_77;      /*IMEU3采样和故障信息(ICC mIndex=77, u16×53)*/
extern u8  *gImeu3CfgRO_75;     /*IMEU3硬件参数当前值(ICC mIndex=75, u8×12)*/

extern u16 gSysGenParaRO_100[];                 /*系统通用参数设置*/
extern u16 gSysHigParaRO_101[];                 /*系统硬件参数设置*/
extern u16 gGBmuGenPara_102[];                  /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];               /*主控高级参数值*/
extern u8 gGHardPara_104[];     	            /*主控硬件参数值*/
extern u16 gSysGenParaRW_105[];                 /*系统通用设置*/
extern u16 gSysHigParaRW_106[];                 /*系统高级设置*/
extern u16 gGBmuGenParaWR_107[];                /*主控通用参数值*/
extern u16 gGBmuHigLevParaWR_108[];             /*主控高级参数值*/
extern u8 gGHardParaWR_109[];  	                /*主控硬件参数值*/
/*=======================================校时信息=================================================*/



/*=================================================================================================
** @Static function declaration
**===============================================================================================*/
 
/*=================================================================================================
** @Interface function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : u8 ICCDeviceTableInit(void)
** @Input     : u8
** @Output    : void
** @Function  : 核间通信设备表初始化
** @The notes : 核间通信包总初始化调用
**===============================================================================================*/
u8 ICCDeviceTableInit(void)
{
    u32 cellIndex = 0;
    u32 cycleIndex = 0;
    u32 faqIndex = 0;
    u32 bmuTabIndex = 0;

/*=======================================通信信息数据主索引表(BMU)====================================*/
    Filed_BMUTab = ExtMalloc(sizeof(t_ICCIndexTab) * BMU_INDEX_NUM);
    if(Filed_BMUTab == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc Filed_BMUTab fail\r\n", __func__);
        ExtFree(Filed_BMUTab);
        return FALSE;
    }
    else 
    {
        printf("malloc Filed_BMUTab %d success\r\n",sizeof(t_ICCIndexTab) * BMU_INDEX_NUM);  
        /*清零初始化:ExtMalloc不清零,未注册的表尾条目若为垃圾值,二分查找可能误命中返回野指针*/
        memset(Filed_BMUTab, 0, sizeof(t_ICCIndexTab) * BMU_INDEX_NUM);
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 0;/*主索引*/
        Filed_BMUTab[bmuTabIndex].pObject = gVerTime_0;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX0_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 2;/*主索引*/
        Filed_BMUTab[bmuTabIndex].pObject = gDevInfo_2;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX2_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 3;/*主索引*/
        Filed_BMUTab[bmuTabIndex].pObject = gSerialNum_3;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U8;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX3_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 10;/*主索引*/
        Filed_BMUTab[bmuTabIndex].pObject = gGHisErrInfo1_10;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX10_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 11;/*主索引*/
        Filed_BMUTab[bmuTabIndex].pObject = gGHisErrInfo2_11;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX11_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 12;
        Filed_BMUTab[bmuTabIndex].pObject = gGHisEventInfo1_12;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX12_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 13;
        Filed_BMUTab[bmuTabIndex].pObject = gGHisEventInfo2_13;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX13_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 14;
        Filed_BMUTab[bmuTabIndex].pObject = gDevSampInfo_14;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX14_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 15;
        Filed_BMUTab[bmuTabIndex].pObject = gGHisDodInfo1_15;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX15_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 16;
        Filed_BMUTab[bmuTabIndex].pObject = gGHisDodInfo2_16;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX16_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 20;
        Filed_BMUTab[bmuTabIndex].pObject = gGChgBalanInfo_1;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX20_21_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 21;
        Filed_BMUTab[bmuTabIndex].pObject = gGChgBalanInfo_2;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX20_21_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 22;
        Filed_BMUTab[bmuTabIndex].pObject = gGDhgBalanInfo_1;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX22_23_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 23;
        Filed_BMUTab[bmuTabIndex].pObject = gGDhgBalanInfo_2;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX91_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 24;
        Filed_BMUTab[bmuTabIndex].pObject = gGCellVoltInfo_1;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX92_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 25;
        Filed_BMUTab[bmuTabIndex].pObject = gGCellVoltInfo_2;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX24_25_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 26;
        Filed_BMUTab[bmuTabIndex].pObject = gGCellTempInfo_1;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_S16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX26_27_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 27;
        Filed_BMUTab[bmuTabIndex].pObject = gGCellTempInfo_2;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_S16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX26_27_LEN;
        bmuTabIndex++;
        //内阻实部注册(28-31): 每主索引3头+208颗=211子索引
        for(u8 grp = 0; grp < ICC_RESIS_GRP_NUM; grp++)
        {
            Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
            Filed_BMUTab[bmuTabIndex].mIndex = 28 + grp;
            Filed_BMUTab[bmuTabIndex].pObject = gIccCellResisReal + grp * ICC_MINDEX28_IDX_LEN;
            Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
            Filed_BMUTab[bmuTabIndex].sIndexNum = ICC_MINDEX28_IDX_LEN;
            bmuTabIndex++;
        }
        //内阻虚部注册(32-35): 每主索引3头+208颗=211子索引
        for(u8 grp = 0; grp < ICC_RESIS_GRP_NUM; grp++)
        {
            Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
            Filed_BMUTab[bmuTabIndex].mIndex = 32 + grp;
            Filed_BMUTab[bmuTabIndex].pObject = gIccCellResisImag + grp * ICC_MINDEX28_IDX_LEN;
            Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
            Filed_BMUTab[bmuTabIndex].sIndexNum = ICC_MINDEX28_IDX_LEN;
            bmuTabIndex++;
        }
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 50;
        Filed_BMUTab[bmuTabIndex].pObject = gSysStaInfo_50;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX50_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 51;
        Filed_BMUTab[bmuTabIndex].pObject = gSysMainInfo_51;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_S16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX51_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 52;
        Filed_BMUTab[bmuTabIndex].pObject = gSysCapInfo_52;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX52_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 53;
        Filed_BMUTab[bmuTabIndex].pObject = gSysLimInfo_53;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX53_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 54;
        Filed_BMUTab[bmuTabIndex].pObject = gSysCharInfo_54;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_S16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX54_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 55;
        Filed_BMUTab[bmuTabIndex].pObject = gCurrCapEner_55;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX55_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 56;
        Filed_BMUTab[bmuTabIndex].pObject = gHisCapEner_56;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX56_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 57;
        Filed_BMUTab[bmuTabIndex].pObject = gAlmFault1_57;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX57_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 58;
        Filed_BMUTab[bmuTabIndex].pObject = gAlmFault2_58;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX58_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 59;
        Filed_BMUTab[bmuTabIndex].pObject = gGStatistTime_59;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX59_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 60;
        Filed_BMUTab[bmuTabIndex].pObject = gGStatistErr_60;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX60_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 61;
        Filed_BMUTab[bmuTabIndex].pObject = gGStatistSosInfo_61;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX61_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 62;
        Filed_BMUTab[bmuTabIndex].pObject = gGStatistDodInfo_62;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX62_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 63;
        Filed_BMUTab[bmuTabIndex].pObject = gGAuthorInfo_63;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX63_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 64;
        Filed_BMUTab[bmuTabIndex].pObject = gGAuthorCtrl_64;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX64_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 65;
        Filed_BMUTab[bmuTabIndex].pObject = gSysCtrlCmd_65;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX65_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 66;
        Filed_BMUTab[bmuTabIndex].pObject = gSysCtrlCmd_66;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX66_LEN;
        bmuTabIndex++; 
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 67;
        Filed_BMUTab[bmuTabIndex].pObject = gImiu4Info_67;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_S16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX67_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 68;
        Filed_BMUTab[bmuTabIndex].pObject = gImiu4Info2_68;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_S16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX68_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 69;
        Filed_BMUTab[bmuTabIndex].pObject = gUserCtrlCmd_69;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX69_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 70;
        Filed_BMUTab[bmuTabIndex].pObject = gCtrlCmdInfo_70;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX70_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 71;
        Filed_BMUTab[bmuTabIndex].pObject = gSysDbgCmd_71;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX71_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 72;
        Filed_BMUTab[bmuTabIndex].pObject = gSampCalCmd_72;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX72_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 73;
        Filed_BMUTab[bmuTabIndex].pObject = gSampCalFbInfo_73;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX73_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 74;
        Filed_BMUTab[bmuTabIndex].pObject = gBsudPSampInfo_74;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX74_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 75;
        Filed_BMUTab[bmuTabIndex].pObject = gImeu3CfgRO_75;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U8;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX75_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 76;
        Filed_BMUTab[bmuTabIndex].pObject = gImeu3CfgSet_76;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U8;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX76_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 77;
        Filed_BMUTab[bmuTabIndex].pObject = gImeu3Info_77;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX77_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 81;
        Filed_BMUTab[bmuTabIndex].pObject = gIccVerSrl_81;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U8;
        Filed_BMUTab[bmuTabIndex].sIndexNum = ICC_MINDEX81_VER_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 82;
        Filed_BMUTab[bmuTabIndex].pObject = gIccDevSerial_82;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U8;
        Filed_BMUTab[bmuTabIndex].sIndexNum = ICC_MINDEX82_SERIAL_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 83;
        Filed_BMUTab[bmuTabIndex].pObject = gIccPackSerial_83;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U8;
        Filed_BMUTab[bmuTabIndex].sIndexNum = ICC_MINDEX83_PACKSERIAL_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 84;
        Filed_BMUTab[bmuTabIndex].pObject = gIccCellSerial_84;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U8;
        Filed_BMUTab[bmuTabIndex].sIndexNum = ICC_MINDEX84_CELLSERIAL_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 85;
        Filed_BMUTab[bmuTabIndex].pObject = gIccInsuQuery_85;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = ICC_MINDEX85_INSQUERY_LEN;
        bmuTabIndex++;
        // //数据注册
        // Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        // Filed_BMUTab[bmuTabIndex].mIndex = 80;
        // Filed_BMUTab[bmuTabIndex].pObject = gSysOverallInfo_80;
        // Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        // Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX80_LEN;
        // bmuTabIndex++;
        // //数据注册
        // Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        // Filed_BMUTab[bmuTabIndex].mIndex = 81;
        // Filed_BMUTab[bmuTabIndex].pObject = gPCSACDCCtrlGrp_81;
        // Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
        // Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX81_LEN;
        // bmuTabIndex++;
        // //数据注册
        // Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        // Filed_BMUTab[bmuTabIndex].mIndex = 82;
        // Filed_BMUTab[bmuTabIndex].pObject = gPCSACDCCtrlGrp_82;
        // Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
        // Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX82_LEN;
        // bmuTabIndex++;
        // //数据注册
        // Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        // Filed_BMUTab[bmuTabIndex].mIndex = 83;
        // Filed_BMUTab[bmuTabIndex].pObject = gDCDCGrpInfo_83;
        // Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
        // Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX83_LEN;
        // bmuTabIndex++;
        // //数据注册
        // Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        // Filed_BMUTab[bmuTabIndex].mIndex = 84;
        // Filed_BMUTab[bmuTabIndex].pObject = gDCDCGrpInfo_84;
        // Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
        // Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX84_LEN;
        // bmuTabIndex++;    

        // //数据注册
        // Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        // Filed_BMUTab[bmuTabIndex].mIndex = 85;
        // Filed_BMUTab[bmuTabIndex].pObject = gPCSACDCModInfo_85;
        // Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
        // Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX85_LEN;
        // bmuTabIndex++;
        // //数据注册
        // Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        // Filed_BMUTab[bmuTabIndex].mIndex = 86;
        // Filed_BMUTab[bmuTabIndex].pObject = gPCSACDCModInfo_86;
        // Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
        // Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX86_LEN;
        // bmuTabIndex++;
        //数据注册
        // Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        // Filed_BMUTab[bmuTabIndex].mIndex = 87;
        // Filed_BMUTab[bmuTabIndex].pObject = gDCDCModInfo_87;
        // Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
        // Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX87_LEN;
        // bmuTabIndex++;
        // //数据注册
        // Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        // Filed_BMUTab[bmuTabIndex].mIndex = 88;
        // Filed_BMUTab[bmuTabIndex].pObject = gDCDCModInfo_88;
        // Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U32;
        // Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX88_LEN;
        // bmuTabIndex++;

        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 100;
        Filed_BMUTab[bmuTabIndex].pObject = gSysGenParaRO_100;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = SINDEX100_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 101;/*主索引*/
        Filed_BMUTab[bmuTabIndex].pObject = gSysHigParaRO_101;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = SINDEX101_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 102;/*主索引*/
        Filed_BMUTab[bmuTabIndex].pObject = gGBmuGenPara_102;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX102_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 103;/*主索引*/
        Filed_BMUTab[bmuTabIndex].pObject = gGBmuHigLevPara_103;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX103_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READONLY;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 104;
        Filed_BMUTab[bmuTabIndex].pObject = gGHardPara_104;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U8;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX104_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 105;
        Filed_BMUTab[bmuTabIndex].pObject = gSysGenParaRW_105;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = SINDEX105_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 106;
        Filed_BMUTab[bmuTabIndex].pObject = gSysHigParaRW_106;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = SINDEX106_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 107;
        Filed_BMUTab[bmuTabIndex].pObject = gGBmuGenParaWR_107;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX107_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 108;
        Filed_BMUTab[bmuTabIndex].pObject = gGBmuHigLevParaWR_108;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U16;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX108_LEN;
        bmuTabIndex++;
        //数据注册
        Filed_BMUTab[bmuTabIndex].acceType = ICC_READWRITE;/*读写属性*/
        Filed_BMUTab[bmuTabIndex].mIndex = 109;
        Filed_BMUTab[bmuTabIndex].pObject = gGHardParaWR_109;
        Filed_BMUTab[bmuTabIndex].dataType = ICC_DT_U8;
        Filed_BMUTab[bmuTabIndex].sIndexNum = GINDEX109_LEN;
        bmuTabIndex++;









        if(bmuTabIndex <= BMU_INDEX_NUM){
            printf("bmuTabIndex array index does not exceed the boundary\r\n");
        }
        else{
            ssdk_printf(SSDK_WARNING, "%s: bmuTabIndex array index out of boundary\r\n", __func__);
        }
        
    }



/*=======================================单体信息CELL通道数据配置====================================*/
    gIccMsgCellDataDef = ExtMalloc(sizeof(t_ICCCellDataDef) * ICCMSGCELLDEF_LEN);
    if(gIccMsgCellDataDef == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gIccMsgCellDataDef fail\r\n", __func__);
        ExtFree(gIccMsgCellDataDef);
        return FALSE;
    }
    else 
    {
        printf("malloc gIccMsgCellDataDef %d success\r\n",sizeof(t_ICCCellDataDef) * ICCMSGCELLDEF_LEN);
        memset(gIccMsgCellDataDef, 0, sizeof(t_ICCCellDataDef) * ICCMSGCELLDEF_LEN);

        /*第1组:主索引 20(单体均充1),cell通道100ms周期 */
        gIccMsgCellDataDef[cellIndex].mIndex = 20;          /*主动发送单体主索引号*/
        gIccMsgCellDataDef[cellIndex].sIndex = 0;           /*主动发送单体起始子索引号*/
        gIccMsgCellDataDef[cellIndex].sIndexNum = GINDEX20_21_LEN;       /*主动发送单体子索引长度*/
        gIccMsgCellDataDef[cellIndex].mapIndex = 20;        /*主动发送单体映射起始主索引号(直发:等于mIndex)*/
        gIccMsgCellDataDef[cellIndex].cycleTime = 0x64;     /*主动发送单体发送周期(1ms)*/
        gIccMsgCellDataDef[cellIndex].daddr = 0x5A;         /*主动发送单体发送目标地址*/
        gIccMsgCellDataDef[cellIndex].txAble = 1;           /*主动发送单体发送使能*/
        cellIndex++;
        /*第2组:主索引 21(单体均充2),cell通道100ms周期 */
        gIccMsgCellDataDef[cellIndex].mIndex = 21;          /*主动发送单体主索引号*/
        gIccMsgCellDataDef[cellIndex].sIndex = 0;           /*主动发送单体起始子索引号*/
        gIccMsgCellDataDef[cellIndex].sIndexNum = GINDEX20_21_LEN;       /*主动发送单体子索引长度*/
        gIccMsgCellDataDef[cellIndex].mapIndex = 21;        /*主动发送单体映射起始主索引号(直发:等于mIndex)*/
        gIccMsgCellDataDef[cellIndex].cycleTime = 0x64;     /*主动发送单体发送周期(1ms)*/
        gIccMsgCellDataDef[cellIndex].daddr = 0x5A;         /*主动发送单体发送目标地址*/
        gIccMsgCellDataDef[cellIndex].txAble = 1;           /*主动发送单体发送使能*/
        cellIndex++;
        /*第3组:主索引 22(单体均放1),cell通道100ms周期 */
        gIccMsgCellDataDef[cellIndex].mIndex = 22;          /*主动发送单体主索引号*/
        gIccMsgCellDataDef[cellIndex].sIndex = 0;           /*主动发送单体起始子索引号*/
        gIccMsgCellDataDef[cellIndex].sIndexNum = GINDEX22_23_LEN;       /*主动发送单体子索引长度*/
        gIccMsgCellDataDef[cellIndex].mapIndex = 22;        /*主动发送单体映射起始主索引号(直发:等于mIndex)*/
        gIccMsgCellDataDef[cellIndex].cycleTime = 0x64;     /*主动发送单体发送周期(1ms)*/
        gIccMsgCellDataDef[cellIndex].daddr = 0x5A;         /*主动发送单体发送目标地址*/
        gIccMsgCellDataDef[cellIndex].txAble = 1;           /*主动发送单体发送使能*/
        cellIndex++;
        /*第4组:主索引 23(单体均放2),cell通道100ms周期 */
        gIccMsgCellDataDef[cellIndex].mIndex = 23;          /*主动发送单体主索引号*/
        gIccMsgCellDataDef[cellIndex].sIndex = 0;           /*主动发送单体起始子索引号*/
        gIccMsgCellDataDef[cellIndex].sIndexNum = GINDEX91_LEN;       /*主动发送单体子索引长度*/
        gIccMsgCellDataDef[cellIndex].mapIndex = 23;        /*主动发送单体映射起始主索引号(直发:等于mIndex)*/
        gIccMsgCellDataDef[cellIndex].cycleTime = 0x64;     /*主动发送单体发送周期(1ms)*/
        gIccMsgCellDataDef[cellIndex].daddr = 0x5A;         /*主动发送单体发送目标地址*/
        gIccMsgCellDataDef[cellIndex].txAble = 1;           /*主动发送单体发送使能*/
        cellIndex++;
        /*第5组:主索引 24(单体电压1),cell通道100ms周期 */
        gIccMsgCellDataDef[cellIndex].mIndex = 24;          /*主动发送单体主索引号*/
        gIccMsgCellDataDef[cellIndex].sIndex = 0;           /*主动发送单体起始子索引号*/
        gIccMsgCellDataDef[cellIndex].sIndexNum = GINDEX92_LEN;       /*主动发送单体子索引长度*/
        gIccMsgCellDataDef[cellIndex].mapIndex = 24;        /*主动发送单体映射起始主索引号(直发:等于mIndex)*/
        gIccMsgCellDataDef[cellIndex].cycleTime = 0x64;     /*主动发送单体发送周期(1ms)*/
        gIccMsgCellDataDef[cellIndex].daddr = 0x5A;         /*主动发送单体发送目标地址*/
        gIccMsgCellDataDef[cellIndex].txAble = 1;           /*主动发送单体发送使能*/
        cellIndex++;
        /*第6组:主索引 25(单体电压2),cell通道100ms周期 */
        gIccMsgCellDataDef[cellIndex].mIndex = 25;          /*主动发送单体主索引号*/
        gIccMsgCellDataDef[cellIndex].sIndex = 0;           /*主动发送单体起始子索引号*/
        gIccMsgCellDataDef[cellIndex].sIndexNum = GINDEX24_25_LEN;       /*主动发送单体子索引长度*/
        gIccMsgCellDataDef[cellIndex].mapIndex = 25;        /*主动发送单体映射起始主索引号(直发:等于mIndex)*/
        gIccMsgCellDataDef[cellIndex].cycleTime = 0x64;     /*主动发送单体发送周期(1ms)*/
        gIccMsgCellDataDef[cellIndex].daddr = 0x5A;         /*主动发送单体发送目标地址*/
        gIccMsgCellDataDef[cellIndex].txAble = 1;           /*主动发送单体发送使能*/
        cellIndex++;
        /*第7组:主索引 26(单体温度1),cell通道100ms周期 */
        gIccMsgCellDataDef[cellIndex].mIndex = 26;          /*主动发送单体主索引号*/
        gIccMsgCellDataDef[cellIndex].sIndex = 0;           /*主动发送单体起始子索引号*/
        gIccMsgCellDataDef[cellIndex].sIndexNum = GINDEX26_27_LEN;       /*主动发送单体子索引长度*/
        gIccMsgCellDataDef[cellIndex].mapIndex = 26;        /*主动发送单体映射起始主索引号(直发:等于mIndex)*/
        gIccMsgCellDataDef[cellIndex].cycleTime = 0x64;     /*主动发送单体发送周期(1ms)*/
        gIccMsgCellDataDef[cellIndex].daddr = 0x5A;         /*主动发送单体发送目标地址*/
        gIccMsgCellDataDef[cellIndex].txAble = 1;           /*主动发送单体发送使能*/
        cellIndex++;
        /*第8组:主索引 27(单体温度2),cell通道100ms周期 */
        gIccMsgCellDataDef[cellIndex].mIndex = 27;          /*主动发送单体主索引号*/
        gIccMsgCellDataDef[cellIndex].sIndex = 0;           /*主动发送单体起始子索引号*/
        gIccMsgCellDataDef[cellIndex].sIndexNum = GINDEX26_27_LEN;       /*主动发送单体子索引长度*/
        gIccMsgCellDataDef[cellIndex].mapIndex = 27;        /*主动发送单体映射起始主索引号(直发:等于mIndex)*/
        gIccMsgCellDataDef[cellIndex].cycleTime = 0x64;     /*主动发送单体发送周期(1ms)*/
        gIccMsgCellDataDef[cellIndex].daddr = 0x5A;         /*主动发送单体发送目标地址*/
        gIccMsgCellDataDef[cellIndex].txAble = 1;           /*主动发送单体发送使能*/
        cellIndex++;

        if(cellIndex <= ICCMSGCELLDEF_LEN){
            printf("cellIndex array index does not exceed the boundary\r\n");
        }
        else{
            ssdk_printf(SSDK_WARNING, "%s: cellIndex array index out of boundary\r\n", __func__);
        }
        
    }

/*=======================================循环信息CYCLE通道数据配置====================================*/
    gIccMsgCycleDataDef = ExtMalloc(sizeof(t_ICCCycleDataDef) * ICCMSGCYCLEDEF_LEN);
    if(gIccMsgCycleDataDef == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gIccMsgCycleDataDef fail\r\n", __func__);
        ExtFree(gIccMsgCycleDataDef);
        return FALSE;
    }
    else 
    {
        printf("malloc gIccMsgCycleDataDef %d success\r\n",sizeof(t_ICCCycleDataDef) * ICCMSGCYCLEDEF_LEN);
        memset(gIccMsgCycleDataDef, 0, sizeof(t_ICCCycleDataDef) * ICCMSGCYCLEDEF_LEN);
        /*第1组:主索引 50(系统状态信息),cycle通道100ms周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 50;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX50_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x64;                /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第2组:主索引 51(系统整体信息),cycle通道500ms周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 51;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX51_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x1F4;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第3组:主索引 52(系统电量信息),cycle通道500ms周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 52;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX52_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x1F4;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第4组:主索引 53(限值信息),cycle通道500ms周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 53;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX53_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x1F4;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第5组:主索引 54(特征信息),cycle通道500ms周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 54;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX54_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x1F4;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第6组:主索引 55(当前能量统计信息),cycle通道3s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 55;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX55_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0xBB8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第7组:主索引 56(累计电量(能)统计信息),cycle通道3s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 56;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX56_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0xBB8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第8组:主索引 57(组1故障告警信息),cycle通道1s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 57;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX57_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x3E8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第9组:主索引 58(组2故障告警信息),cycle通道1s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 58;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX58_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x3E8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第10组:主索引 59(时间统计信息),cycle通道3s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 59;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX59_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0xBB8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第11组:主索引 60(故障统计信息),cycle通道3s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 60;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX60_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0xBB8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第12组:主索引 61(电池安全等级统计信息),cycle通道1s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 61;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX61_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x3E8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第13组:主索引 62(放电深度统计信息),cycle通道1s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 62;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX62_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x3E8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第14组:主索引 63(权限级别状态),cycle通道10s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 63;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX63_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x2710;              /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第15组:主索引 67(IMIU4信息),cycle通道100ms周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 67;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX67_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x64;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第16组:主索引 68(组二IMIU4信息),cycle通道100ms周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 68;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX68_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x64;                /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第17组:主索引 70(IMIU4控制命令反馈信息),cycle通道1s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 70;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX70_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x3E8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第18组:主索引 73(采样校准反馈信息),cycle通道1s周期(校准期间调试模式主动更新) */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 73;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX73_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x3E8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第19组:主索引 74(BSU02U2板级及PACK采样信息),cycle通道1s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 74;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX74_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x3E8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;
        /*第20组:主索引 77(IMEU3采样和故障信息),cycle通道1s周期 */
        gIccMsgCycleDataDef[cycleIndex].mIndex = 77;                     /*主动发送循环主索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndex = 0;                      /*主动发送循环起始子索引号*/
        gIccMsgCycleDataDef[cycleIndex].sIndexNum = GINDEX77_LEN;        /*主动发送循环子索引长度*/
        gIccMsgCycleDataDef[cycleIndex].cycleTime = 0x3E8;               /*主动发送循环发送周期(1ms)*/
        gIccMsgCycleDataDef[cycleIndex].daddr = 0x5A;                    /*主动发送循环发送目标地址*/
        gIccMsgCycleDataDef[cycleIndex].txAble = 1;                      /*主动发送循环发送使能*/
        cycleIndex++;



        if(cycleIndex <= ICCMSGCYCLEDEF_LEN){
            printf("cycleIndex array index does not exceed the boundary\r\n");
        }
        else{
            ssdk_printf(SSDK_WARNING, "%s: cycleIndex array index out of boundary\r\n", __func__);
        }
        
    }

/*=======================================问答信息FAQ通道数据配置====================================*/
    gIccMsgFaqDataDef = ExtMalloc(sizeof(t_ICCFaqDataDef) * ICCMSGFAQDEF_LEN);
    if(gIccMsgFaqDataDef == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gIccMsgFaqDataDef fail\r\n", __func__);
        ExtFree(gIccMsgFaqDataDef);
        return FALSE;
    }
    else 
    {
        printf("malloc gIccMsgFaqDataDef %d success\r\n",sizeof(t_ICCFaqDataDef) * ICCMSGFAQDEF_LEN);
        /*清零初始化:FAQ实际注册数小于表长,发送表初始化会扫描全部表项,垃圾值可能误过检查*/
        memset(gIccMsgFaqDataDef, 0, sizeof(t_ICCFaqDataDef) * ICCMSGFAQDEF_LEN);
        /*第1组:主索引 0(校时信息),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 0;                     /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX0_LEN;        /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第2组:主索引 2(设备信息),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 2;                     /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX2_LEN;        /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第3组:主索引 3(设备序列号),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 3;                     /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX3_LEN;        /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第4组:主索引 10(组1历史故障信息),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 10;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX10_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第5组:主索引 11(组2历史故障信息),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 11;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX11_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第6组:主索引 12(组1历史事件信息),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 12;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX12_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第7组:主索引 13(组2历史事件信息),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 13;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX13_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第8组:主索引 14(板级采样信息),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 14;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX14_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*组1历史DOD记录信息(ICC mIndex=15),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 15;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX15_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*组2历史DOD记录信息(ICC mIndex=16),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 16;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX16_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第9组:主索引 64(权限切换命令),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 64;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX64_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第10组:主索引 65(组1系统控制命令),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 65;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX65_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第11组:主索引 66(组2系统控制命令),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 66;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX66_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第12组:主索引 69(用户控制命令),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 69;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX69_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第13组:主索引 71(系统调试命令),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 71;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX71_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第14组:主索引 72(采样校准命令(对IMIU4)),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 72;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX72_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*IMEU3硬件参数回读(ICC mIndex=75),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 75;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX75_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*IMEU3硬件参数配置(ICC mIndex=76),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 76;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX76_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第15组:主索引 100(系统通用设置结果),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 100;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = SINDEX100_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第16组:主索引 101(系统高级设置结果),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 101;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = SINDEX101_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第17组:主索引 102(主控通用参数值),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 102;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX102_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第18组:主索引 103(主控高级参数值),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 103;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX103_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第19组:主索引 104(主控硬件参数值),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 104;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX104_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第20组:主索引 105(系统通用设置),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 105;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = SINDEX105_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第21组:主索引 106(系统高级设置),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 106;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = SINDEX106_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第23组:主索引 107(主控通用参数设置),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 107;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX107_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第24组:主索引 108(主控高级参数设置),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 108;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX108_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;
        /*第25组:主索引 109(主控硬件参数设置),faq通道查询回复 */
        gIccMsgFaqDataDef[faqIndex].mIndex = 109;                    /*问答主索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndex = 0;                     /*问答起始子索引号*/
        gIccMsgFaqDataDef[faqIndex].sIndexNum = GINDEX109_LEN;       /*问答子索引长度*/
        gIccMsgFaqDataDef[faqIndex].daddr = 0x5A;                   /*问答发送目标地址*/
        faqIndex++;




        
        if(faqIndex <= ICCMSGFAQDEF_LEN){
            printf("faqIndex array index does not exceed the boundary\r\n");
        }
        else{
            ssdk_printf(SSDK_WARNING, "%s: faqIndex array index out of boundary\r\n", __func__);
        }
        
    }



/*=============================================================================================*/


    //基本配置信息
    gIccBaseDef = ExtMalloc(sizeof(u8) * ICCBaseDef_LEN);
    if(gIccBaseDef == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gIccBaseDef fail\r\n", __func__);
        ExtFree(gIccBaseDef);
        return FALSE;
    }
    else 
    {
        printf("malloc gIccBaseDef %d success\r\n",sizeof(u8) * ICCBaseDef_LEN);
        gIccBaseDef[eICCRBase_DevNum] = ICC_DEVTAB_NUM;
        gIccBaseDef[eICCRBase_IndexNum] = BMU_INDEX_NUM;
        gIccBaseDef[eICCRBase_TRTabNum] = ICC_TRTAB_NUM;
        gIccBaseDef[eICCRBase_TxVerNum] = CAN_TX_VER_NUM;
        gIccBaseDef[eICCRBase_TxBreNum] = CAN_TX_BRE_NUM;
        gIccBaseDef[eICCRBase_TxCelNum] = CAN_TX_CEL_NUM;
        gIccBaseDef[eICCRBase_TxCylNum] = CAN_TX_CYL_NUM;
        gIccBaseDef[eICCRBase_TxFaqNum] = CAN_TX_FAQ_NUM;
    }

    //核间通信设备表
    gIccDevTable = ExtMalloc(sizeof(t_ICCDevTab) * ICC_DEVTAB_NUM);
    if(gIccDevTable == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gIccDevTable fail\r\n", __func__);
        ExtFree(gIccDevTable);
        return FALSE;
    }
    else 
    {
        printf("malloc gIccDevTable %d success\r\n",sizeof(t_ICCDevTab) * ICC_DEVTAB_NUM);
    }

    //核间通信收发处理表
    gIccTxRcvTable = ExtMalloc(sizeof(t_ICCTxRcvTab) * ICC_TRTAB_NUM);
    if(gIccTxRcvTable == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gIccTxRcvTable fail\r\n", __func__);
        ExtFree(gIccTxRcvTable);
        return FALSE;
    }
    else 
    {
        printf("malloc gIccTxRcvTable %d success\r\n",sizeof(t_ICCTxRcvTab) * ICC_TRTAB_NUM);
    }

    //核间通信单体发送处理表
    gIccTxCelTable = ExtMalloc(sizeof(t_ICCTxTab) * CAN_TX_CEL_NUM);
    if(gIccTxCelTable == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gIccTxCelTable fail\r\n", __func__);
        ExtFree(gIccTxCelTable);
        return FALSE;
    }
    else 
    {
        printf("malloc gIccTxCelTable %d success\r\n",sizeof(t_ICCTxTab) * CAN_TX_CEL_NUM);
    }

    //核间通信周期发送处理表
    gIccTxCycTable = ExtMalloc(sizeof(t_ICCTxTab) * CAN_TX_CYL_NUM);
    if(gIccTxCycTable == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc gIccTxCycTable fail\r\n", __func__);
        ExtFree(gIccTxCycTable);
        return FALSE;
    }
    else 
    {
        printf("malloc gIccTxCycTable %d success\r\n",sizeof(t_ICCTxTab) * CAN_TX_CYL_NUM);
    }
    // //核间模块信息周期发送处理表
    // {
    //     return FALSE;
    // }
    // else 
    // {
    // }


    //核间通信发送缓冲区内存申请
    txData = ExtMalloc(sizeof(u8) * 1024);
    if(txData == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc txData\r\n", __func__);
        ExtFree(txData);
        return FALSE;
    }
    else 
    {
        printf("malloc txData %d success\r\n",sizeof(u8) * 1024);
    }
    //核间通信发送表读取缓冲区内存申请
    readData = ExtMalloc(sizeof(u8) * 1024);
    if(readData == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: malloc readData\r\n", __func__);
        ExtFree(readData);
        return FALSE;
    }
    else 
    {
        printf("malloc readData %d success\r\n",sizeof(u8) * 1024);
    } 
    /*将通信主索引表创建到设备表*/
    ICCDevTabEntryInit_API((u8)gDevInfo_2[eICCDev2_Addr], BMU_INDEX_NUM, Filed_BMUTab, 0);
    //创建核减通信通道
    IccMsgInit();

    return TRUE;
}
/*=================================================================================================
** @Static function definition
**===============================================================================================*/