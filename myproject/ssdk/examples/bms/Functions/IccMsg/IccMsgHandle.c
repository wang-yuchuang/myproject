/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : IccMsgHandle.c
** @Founder      : ZQD808
** @Date         : 2026.01.29
** @Function     : 核间通信接收数据处理模块
** @Instructions : 处理A核下发数据处理
**===========================================================================================*/
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "IccMsgCreate.h"
#include "IccMsgHandle.h"
#include "TaskMonitor.h"
#include "IccMsgTxCycle.h"
#include "IccMsgUser.h"
#include "IccMsgInterface.h"
#include "iccmsg.h"
#include "iccmsg_dpr.h"
#include <CLI.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "BSPTypeDef.h"
#include <malloc.h>
#include "GroupInfo.h"
#include "icc_upgrade_handler.h"
#include "IccMsgTabOper.h"

/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
u32 gIccCountTime = 0;                                              //核间通信包总计时(1ms)




extern iccmsg_t *iccmsg_cell;   //单体信息通道句柄
extern iccmsg_t *iccmsg_cycle;  //周期信息通道句柄
extern iccmsg_t *iccmsg_faq;    //问答信息通道句柄
extern iccmsg_t *iccmsg_update; //升级信息通道句柄
// extern iccmsg_t *iccmsg_pcs;    //模块周期信息通道句柄
// extern iccmsg_t *iccmsg_pcscfg; //模块配置信息通道句柄

extern u16 *gVerTime_0;
extern u16 *gDevInfo_2;
extern u8 *gSerialNum_3; 
extern u8 *gIccBaseDef;
extern u8 *readData;
extern u8 *txData;
extern t_ICCDevTab* gIccDevTable;

extern t_ICCTxRcvTab* gIccTxRcvTable;             /*核间通信收发处理表*/
extern t_ICCTxTab* gIccTxCelTable;                /*核间通信周期发送处理表*/
extern t_ICCTxTab* gIccTxCycTable;                /*核间通信周期发送处理表*/
extern u16 gSysHigParaRW_106[];                 /*系统高级设置*/





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
extern u16 *gSampCalFbInfo_73; /*采样校准反馈信息*/
//BSU相关信息
extern u16 *gBsudPSampInfo_74;  /*BSU02U2板级及PACK采样信息*/
/*=================================================================================================
** @Static function declaration
**===============================================================================================*/

/*=================================================================================================
** @Name      : static u8 ICCRcvVerTimeMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:核间通信帧 iccNum:核间通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 校时帧接收处理
** @The notes : 更新校时数据和调用用户HOOK
**===============================================================================================*/
static u8 ICCRcvVerTimeMsgDeal(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : static u8 ICCRcvBreathMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 心跳帧接收处理
** @The notes : 调用用户HOOK
**===============================================================================================*/
static u8 ICCRcvBreathMsgDeal(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : static u8 ICCRcvReqUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:核间通信帧结构 canNum:核间通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 请求修改帧接收处理
** @The notes : 更新修改域数据和调用用户HOOK
**===============================================================================================*/
static u8 ICCRcvReqUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : static u8 ICCRcvAnsUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 修改回复帧接收处理
** @The notes : 对比数据是否修改成功和确认修改
**===============================================================================================*/
static u8 ICCRcvAnsUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : static u8 ICCRcvConUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 修改确认帧接收处理
** @The notes : 调用用户HOOK
**===============================================================================================*/
static u8 ICCRcvConUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : static u8 ICCRcvReqReadMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : canFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 请求查询帧接收处理
** @The notes : 调用用户HOOK更新数据域和回复
**===============================================================================================*/
static u8 ICCRcvReqReadMsgDeal(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : static u8 ICCRcvAnsReadMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 查询回复帧接收处理
** @The notes : 更新查询得到的数据和确认查询
**===============================================================================================*/
static u8 ICCRcvAnsReadMsgDeal(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : static u8 ICCRcvConReadMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 查询确认帧接收处理
** @The notes : 确认是否正确接收和发送下一帧查询回复
**===============================================================================================*/
static u8 ICCRcvConReadMsgDeal(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : static u8 ICCRcvCycUpMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 周期主动更新帧接收处理
** @The notes : 更新接收得到的数据到索引表
**===============================================================================================*/
static u8 ICCRcvCycUpMsgDeal(t_ICCFram *iccFram, u8 iccNum);
/*=================================================================================================
** @Interface function definition
**===============================================================================================*/
/*=============================================================================================
** @Name      : void BSPCAN1RcvTask(void *argument)
** @Input     : *p:任务函数格式需要空指针
** @Output    : void
** @Function  : 核间通信数据接收处理
** @The notes : A核只下发问答信息指令和升级信息指令
**===========================================================================================*/
void IccMsgRcvTask(void *argument)
{
    int ret;
    iccmsg_rx_msg_t *rx_cell_msg = NULL;
    iccmsg_rx_msg_t *rx_cycle_msg = NULL;
    iccmsg_rx_msg_t *rx_faq_msg = NULL;
    iccmsg_rx_msg_t *rx_update_msg = NULL;
    iccmsg_rx_msg_t *rx_pcs_msg = NULL;
    iccmsg_rx_msg_t *rx_pcscfg_msg = NULL;
    //iccmsg_test_msg_t tmp;
    while(1)
    {
        //判断通道是否为空,A核暂时只通过FAQ和UPDATE通道下发数据，暂时不处理CELL和CYCLE的数据
        if(iccmsg_cell != NULL)
        {
            ret = osMessageQueueGet(iccmsg_cell->rx_queue, &rx_cell_msg, NULL, 0);
            if(ret == osOK && NULL != rx_cell_msg)
            {
                for(int i = 0; i< rx_cell_msg->len; i++)
                {
                   printf("rx_cell_msg->date[%d] = %d\r\n",i, rx_cell_msg->data[i]);
                }
                ExtFree(rx_cell_msg->data);
                rx_cell_msg->data = NULL;
                ExtFree(rx_cell_msg);
                rx_cell_msg = NULL;
            }
        }
        //判断通道是否为空
        if(iccmsg_cycle != NULL)
        {
            ret = osMessageQueueGet(iccmsg_cycle->rx_queue, &rx_cycle_msg, NULL, 0);
            if(ret == osOK && NULL != rx_cycle_msg)
            {
                for(int i = 0; i< rx_cycle_msg->len; i++)
                {
                   printf("rx_cycle_msg->date[%d] = %d\r\n",i,rx_cycle_msg->data[i]);
                }
                ExtFree(rx_cycle_msg->data);
                rx_cycle_msg->data = NULL;
                ExtFree(rx_cycle_msg);
                rx_cycle_msg = NULL;
            }
        }
        //判断通道是否为空
        if(iccmsg_faq != NULL)
        {
            ret = osMessageQueueGet(iccmsg_faq->rx_queue, &rx_faq_msg, NULL, 0);
            if(ret == osOK && NULL != rx_faq_msg)
            {
                for(int i = 0; i< rx_faq_msg->len; i++)
                {
                   printf("rx_faq_msg->date[%d] = %d\r\n",i,rx_faq_msg->data[i]);
                }
                ICCLibRcvMsgHandle_API(rx_faq_msg->data, rx_faq_msg->len, eICCCHA_FAQ);

                ExtFree(rx_faq_msg->data);
                rx_faq_msg->data = NULL;
                ExtFree(rx_faq_msg);
                rx_faq_msg = NULL;
            }
        }
        //判断通道是否为空
        if(iccmsg_update != NULL)
        {
            ret = osMessageQueueGet(iccmsg_update->rx_queue, &rx_update_msg, NULL, 0);
            if(ret == osOK && NULL != rx_update_msg)
            {
                icc_upgrade_handle_msg(rx_update_msg->data, rx_update_msg->len);
                ExtFree(rx_update_msg->data);
                rx_update_msg->data = NULL;
                ExtFree(rx_update_msg);
                rx_update_msg = NULL;
            }
        }
        // //判断通道是否为空
        // if(iccmsg_pcscfg != NULL)
        // {
        //     ret = osMessageQueueGet(iccmsg_pcscfg->rx_queue, &rx_pcscfg_msg, NULL, 0);
        //     if(ret == osOK && NULL != rx_pcscfg_msg)
        //     {
        //         for(int i = 0; i< rx_pcscfg_msg->len; i++)
        //         {
        //            printf("rx_pcscfg_msg->date[%d] = %d\r\n",i,rx_pcscfg_msg->data[i]);
        //         }
        //         ICCLibRcvMsgHandle_API(rx_pcscfg_msg->data, rx_pcscfg_msg->len, eICCCHA_PCF);
        //         ExtFree(rx_pcscfg_msg->data);
        //         rx_pcscfg_msg->data = NULL;
        //         ExtFree(rx_pcscfg_msg);
        //         rx_pcscfg_msg = NULL;
        //     }
        // }
        osDelay(1);
        MONITOR_STACK_UPDATE(eTaskMon_IccMsgRcvTask);    }
}


/*=================================================================================================
** @Name      : void ICCDevTabEntryInit_API(u8 devAddr, u8 mIndexNum const s_ICCIndexTab  *pIndexTab, u8 devId)
** @Input     : devAdd:设备地址 mIndexNum:主索引数目 pIndexTab:设备主动引表 devId:创建设备表序号
** @Output    : void
** @Function  : 核间通信创建设备表初始化
** @The notes : 用户配置核间通信设备表信息
**===============================================================================================*/
void ICCDevTabEntryInit_API(u8 devAddr, u8 mIndexNum, const t_ICCIndexTab *pIndexTab, u8 devId)
{
    /*创建设备表*/
    if(devId < gIccBaseDef[eICCRBase_DevNum])
    {
        gIccDevTable[devId].devAdrr = devAddr;
        gIccDevTable[devId].mIndexNum = mIndexNum;
        gIccDevTable[devId].pIndexTable = pIndexTab;
    }
}

/*=================================================================================================
** @Name      : void ICCBuildMsgID_API(u16 dataNum, e_ICCFunCode funCode, u8 mIndex, u8 destAddr, u8* idData)
** @Input     : dataNum:数据域长度 funCode:功能码 mIndex:主索引号 destAddr:目标地址  idData:帧头id域地址
** @Output    : void
** @Function  : 组建8字节ICC通信帧ID值
** @The notes :
**===============================================================================================*/
void ICCBuildMsgID_API(u16 dataNum, e_ICCFunCode funCode, u8 mIndex, u8 destAddr, u8* idData)
{
    /*数据长度4byte|功能码1byte|信息类型1byte|目标地址1byte|源地址1byte*/
  
    *(idData+0) = dataNum&0xff;
    *(idData+1) = (dataNum >> 8)&0xff;
    *(idData+2) = 0;
    *(idData+3) = 0;
    *(idData+4) = funCode;
    *(idData+5) = mIndex;
    *(idData+6) = destAddr;
    *(idData+7) = gDevInfo_2[eICCDev2_Addr];
}

/*=================================================================================================
** @Name      : void ICCBuildMsgDataMark_API(u8 frammark, u8 sindexnum, u8 sindex, u8 *pdata)
** @Input     : frammark:帧标识 sindexnum:子索引数目 pdata:起始子索引 *pdata:读到的数据
** @Output    : void
** @Function  : 组建3字节ICC通信帧数据域标识
** @The notes : 
**===============================================================================================*/
void ICCBuildMsgDataMark_API(u8 frammark, u8 sindexnum, u8 sindex, u8 *pdata)
{
    /*帧标识1byte|子索引数目1byte|起始子索引1byte*/
  
    *(pdata+0) = frammark;
    *(pdata+1) = sindexnum;
    *(pdata+2) = sindex;
}

/*=================================================================================================
** @Name      : void ICCLibTxRcvHandleTask_API(void)
** @Input     : void
** @Output    : void
** @Function  : 核间通信报文收发处理任务(收发通信表处理)
** @The notes : 用户必须1ms周期调用
**===============================================================================================*/
void ICCLibTxRcvHandleTask_API(void)
{
    u8 i = 0;

    /*未创建通信收发表*/
    if(gIccTxRcvTable == NULL)
    {
        return;
    }

    /*通信包总计时*/
    gIccCountTime++;
    if(gIccCountTime >= ICC_MAX_COUNT_TIME)
    {
        gIccCountTime = gIccCountTime - ICC_MAX_COUNT_TIME;
    }

    /*检测各个通信收发表状态*/
    for(i = 0; i < gIccBaseDef[eICCRBase_TRTabNum]; i++)
    {
        /*准备就绪,发送信息*/
        if(eICCTAB_READYOK == gIccTxRcvTable[i].ready)
        {
            /*发送通信包信息*/
            //(void)ICCSendTxRcvTabMsgPack(i);

            /*等待下一周期*/
            gIccTxRcvTable[i].ready = eICCTAB_WAIT;
        }
        /*等待处理,计时*/
        else if(eICCTAB_WAIT == gIccTxRcvTable[i].ready)
        {
            /*发送周期计时等待*/
            //ICCTxRcvTabWaitHandle(i);
        }
    }
}


/*=================================================================================================
** @Name      : u8 ICCLibRcvMsgHandle_API(u8 *pdata, u8 DLC, u8 iccNum)
** @Input     : *pdata:核间通信ID域和数据域  DLC:ID域和数据域长度 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : ICC通信报文接收处理任务
** @The notes : 用户放在ICC接收处理HOOK接口调用
**===============================================================================================*/
u8 ICCLibRcvMsgHandle_API(u8 *pdata, u8 DLC, u8 iccNum)
{

    u8 result = 0;
    t_ICCFram *iccFram = NULL;
    iccFram = ExtMalloc(sizeof(t_ICCFram));
    /*解析核间通信*/
    iccFram->framLen = (pdata[1]<<8)|pdata[0];
    iccFram->funCode = pdata[4];
    iccFram->mIndex = pdata[5];
    iccFram->dAddr = pdata[6];
    iccFram->sAddr = pdata[7];
    iccFram->pData = &pdata[8];
    iccFram->pId = &pdata[0];
    printf("*******************iccmsg_analyze*******************\r\n");
    printf("iccmsg->framLen: %d\r\n",iccFram->framLen);
    printf("iccmsg->funCode: %d\r\n",iccFram->funCode);
    printf("iccmsg->mIndex: %d\r\n",iccFram->mIndex);
    printf("iccmsg->dAddr: %d\r\n",iccFram->dAddr);
    printf("iccmsg->sAddr: %d\r\n",iccFram->sAddr);


    /*非自身报文*/
    if(gDevInfo_2[eICCDev2_Addr] != iccFram->dAddr)
    {
        /*非广播报文*/
        if(0xff != iccFram->dAddr)
        {
             return(eICCERR_NOSUCH_DEV);
        }
    }

    /*根据功能码处理报文*/
    switch(iccFram->funCode)
    {
        /*校时信息报文*/
        case eICCFUN_VFT:
            result = ICCRcvVerTimeMsgDeal(iccFram, iccNum);
            break;

        /*心跳信息报文*/
        case eICCFUN_BRE:
            result = ICCRcvBreathMsgDeal(iccFram, iccNum);
            break;

        /*请求修改命令报文*/
        case eICCFUN_RUP:
            result = ICCRcvReqUpdateMsgDeal(iccFram, iccNum);
            break;

        /*请求修改回复报文*/
        case eICCFUN_AUP:
            result = ICCRcvAnsUpdateMsgDeal(iccFram,iccNum);
            break;

        /*请求修改确认报文*/
        case eICCFUN_CUP:
            result = ICCRcvConUpdateMsgDeal(iccFram,iccNum);
            break;

        /*请求查询命令报文*/
        case eICCFUN_REQ:
            result = ICCRcvReqReadMsgDeal(iccFram,iccNum);
            break;

        /*请求查询回复报文*/
        case eICCFUN_ARQ:
            result = ICCRcvAnsReadMsgDeal(iccFram,iccNum);
            break;

        /*查询回复确认报文*/
        case eICCFUN_CRQ:
            result = ICCRcvConReadMsgDeal(iccFram,iccNum);
            break;

        /*主动更新(长帧)报文*/
        case eICCFUN_UPM:
        /*主动发送(单帧)报文*/
        case eICCFUN_UPS:
        /*主动发送(单体)报文*/
        case eICCFUN_UPC:
        /*主动发送(参数)报文*/
        case eICCFUN_UPP:
            result = ICCRcvCycUpMsgDeal(iccFram,iccNum);
            break;

        /*在线升级报文
        case eICCFUN_LOR:
        case eICCFUN_LOA:
        case eICCFUN_LOW:
            result = eICCERR_KEEP_FUN;
            break;*/

        default:
            result = eICCERR_KEEP_FUN;
            break;
    }
    //释放内存
    ExtFree(iccFram);
    return(result);
}

/*=================================================================================================
** @Name      : u8 ICCBuildSendAupMsg_API(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:接收到的ICC通信帧 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送修改回复报文帧
** @The notes : 用户自行发送修改回复报文帧
**===============================================================================================*/
u8 ICCBuildSendAupMsg_API(t_ICCFram *iccFram, u8 iccNum)
{
    /*取已写入的主索引数组数值发送*/
    const t_ICCIndexTab *pindextable;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引

    /*解析数据域*/
    framMark = iccFram->pData[0];
    sIndexNum = iccFram->pData[1];
    sIndex = iccFram->pData[2];

    /*CAN通道最大支持4路*/
    if(iccNum >= ICC_MAX_ICC_NUM)
    {
        return(eICCERR_NOSUCH_ICC);
    }

    /*查找目标设备主索引是否存在且判断子索引是否越界*/
    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);
    if(pindextable == NULL)
    {
        return(errcode);
    }

    /*重新组合ID域(接收报文源地址变目标地址,下行变上行)*/
    ICCBuildMsgID_API(iccFram->framLen, eICCFUN_AUP, iccFram->mIndex, iccFram->sAddr, iccFram->pId);

    /*重新读取当前实际值*/
    ICCReadIccIndexData(pindextable, sIndex, sIndexNum, &iccFram->pData[3]);

    /*发送回复报文*/
    ICCSendMsgUser_Hook(iccFram->pId, iccFram->framLen, iccNum);
printf("*******************iccmsg_response_sucess5*******************\r\n");
    return(errcode);
}

/*=================================================================================================
** @Name      : u8 ICCBuildSendErrMsg_API(u8 errReas, t_ICCFram *iccFram, u8 iccNum)
** @Input     : errReas:错误原因 *iccFram:接收到的核间通信帧 iccNum:核间通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送错误应答报文帧
** @The notes : 请求查询和请求修改命令错误时应答使用
**===============================================================================================*/
u8 ICCBuildSendErrMsg_API(u8 errReas, t_ICCFram *iccFram, u8 iccNum)
{
    /*将收到的报文修改成错误应答功能码发送*/

    /*核间通道最大支持4路*/
    if(iccNum >= ICC_MAX_ICC_NUM)
    {
        return(eICCERR_NOSUCH_ICC);
    }

    /*重新组合ID域(接收报文源地址变目标地址)*/
    ICCBuildMsgID_API(iccFram->framLen, eICCFUN_ERR, iccFram->mIndex, iccFram->sAddr, iccFram->pId);

    /*记录错误码*/
    iccFram->pData[0] = (iccFram->pData[0] & 0x0f) | ((errReas & 0x0f) << 4);

    /*发送回复报文*/
    ICCSendMsgUser_Hook(iccFram->pId, iccFram->framLen, iccNum);

    /*索引错误*/
    if(eICCERRREAS_INDEXERR == errReas)
    {
        return(eICCERR_NOSUCH_MID);
    }
    /*CRC错误*/
    else if(eICCERRREAS_CRCERR == errReas)
    {
        return(eICCERR_MSGCRC_ERR);
    }
    /*权限非法*/
    else if(eICCERRREAS_AUTHERR == errReas)
    {
        return(eICCERR_ICCPW_DIS);
    }
    /*状态非法*/
    /*参数非法*/
    else
    {
        return(eICCERR_RUPW_FAIL);
    }
}


/*=================================================================================================
** @Name      : u8 ICCBuildSendUpmMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : destAddr:目标地址 mIndex:主索引号 sIndex:起始子索引号 sIndexNum:子索引数目  iccNum:ICC通道
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送主动更新报文帧
** @The notes : 用户自行发送主动更新报文帧
**===============================================================================================*/
u8 ICCBuildSendUpmMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
{
    /*取相应设备主索引数组数值发送*/
    const t_ICCIndexTab *pindextable;
    u8 errcode = eICCERR_SUCCESSFUL;

    if(iccNum >= ICC_MAX_ICC_NUM)
    {
        return(eICCERR_NOSUCH_ICC);
    }
    /*查找目标设备主索引是否存在且判断子索引是否越界*/
    pindextable = IccScanIndexTabInfo(gDevInfo_2[eICCDev2_Addr], mIndex, sIndex, sIndexNum, &errcode);
    if(pindextable == NULL)
    {
        return(errcode);
    }

    /*申请空闲收发表进行发送*/
    return(ICCReqIdleTxRcvTab(eICCFUN_UPM, pindextable->pObject, pindextable->dataType, mIndex, sIndex, sIndexNum, 0, destAddr, iccNum));
}   












/*=================================================================================================
** @Static function definition
**===============================================================================================*/

/*=================================================================================================
** @Name      : static u8 ICCRcvVerTimeMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:核间通信帧 iccNum:核间通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 校时帧接收处理
** @The notes : 更新校时数据和调用用户HOOK
**===============================================================================================*/
static u8 ICCRcvVerTimeMsgDeal(t_ICCFram *iccFram, u8 iccNum)
{
    /*非校时报文(只能通过校时功能码修改)*/
    if((eICCFUN_VFT != iccFram->funCode) || (0 != iccFram->mIndex))
    {
        return(eICCERR_NOSUCH_MID);
    }

    /*数据域有效*/
    if(iccFram->framLen >= 16)
    {
        /*解析接收到的时间*/
        gVerTime_0[eICCTim0_Y] = ((u16)iccFram->pData[1] << 8) | (u16)iccFram->pData[0];
        gVerTime_0[eICCTim0_MD] = ((u16)iccFram->pData[3] << 8) | (u16)iccFram->pData[2];
        gVerTime_0[eICCTim0_HM] = ((u16)iccFram->pData[5] << 8) | (u16)iccFram->pData[4];
        gVerTime_0[eICCTim0_MS] = ((u16)iccFram->pData[7] << 8) | (u16)iccFram->pData[6];

        /*校时报文用户处理*/
        ICCRcvVerUserDeal_Hook(iccFram->sAddr, iccNum);

        return(eICCERR_SUCCESSFUL);
    }
    else
    {
        return(eICCERR_RCVMSG_ERR);
    }
}

/*=================================================================================================
** @Name      : static u8 ICCRcvBreathMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 心跳帧接收处理
** @The notes : 调用用户HOOK
**===============================================================================================*/
static u8 ICCRcvBreathMsgDeal(t_ICCFram *iccFram, u8 iccNum)
{


    /*非心跳报文*/
    if((eICCFUN_BRE != iccFram->funCode) || (1 != iccFram->mIndex))
    {
        return(eICCERR_NOSUCH_MID);
    }

    /*数据域有效*/
    if(iccFram->framLen >= 8)
    {
        /*心跳报文用户处理*/
        ICCRcvBreUserDeal_Hook(iccFram->sAddr, iccNum);

        return(eICCERR_SUCCESSFUL);
    }
    else
    {
        return(eICCERR_RCVMSG_ERR);
    }
}


/*=================================================================================================
** @Name      : static u8 ICCRcvReqUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:核间通信帧结构 canNum:核间通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 请求修改帧接收处理
** @The notes : 更新修改域数据和调用用户HOOK
**===============================================================================================*/
static u8 ICCRcvReqUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum)
{
    const t_ICCIndexTab *pindextable;
    u8 wrResult = eICCERRREAS_NORMAL;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引

    /*解析数据域*/
    framMark = iccFram->pData[0];
    sIndexNum = iccFram->pData[1];
    sIndex = iccFram->pData[2];
    printf("iccmsg->framMark: %d\r\n",framMark);
    printf("iccmsg->sIndexNum: %d\r\n",sIndexNum);
    printf("iccmsg->sIndex: %d\r\n",sIndex);

    /*非请求修改报文*/
    if(eICCFUN_RUP != iccFram->funCode)
    {
        return(eICCERR_KEEP_FUN);
    }

    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);

    /*主索引表信息存在*/
    if(pindextable != NULL)
    {

        /*核间通信报文长度不能小于索引长度*/
        if(iccFram->framLen < (11 + (sIndexNum * pindextable->dataType)))
        {
            /*子索引越界*/
            return(eICCERR_SIDLEN_OVE);
        }

        /*写子索引数据,更新通信索引表*/
        (void)ICCWriteCanIndexData(pindextable, sIndex, sIndexNum, &iccFram->pData[3], iccNum);
        /*不需要修改确认*/
        if(0x0c == framMark)
        {
            /*请求修改后处理函数*/
            wrResult = ICCRcvRupUserDeal_Hook(iccFram->sAddr, iccFram->mIndex, sIndex, sIndexNum, iccNum);
        }

        /*延迟回复:HOOK已自行发送回复,不再自动回复*/
        if(eICCERRREAS_PENDING == wrResult)
        {
            return(eICCERR_SUCCESSFUL);
        }

        /*请求修改正常*/
        if(eICCERRREAS_NORMAL == wrResult)
        {
            /*生成修改回复信息*/
            return(ICCBuildSendAupMsg_API(iccFram, iccNum));
        }
        else
        {
            /*发送错误应答报文(权限或参数错误)*/
            return(ICCBuildSendErrMsg_API(wrResult, iccFram, iccNum));
        }
    }
    /*主索引表信息不存在*/
    else
    {
        /*发送错误应答报文(索引错误)*/
        (void)ICCBuildSendErrMsg_API(eICCERRREAS_INDEXERR, iccFram, iccNum);

        return(errcode);
    }
}

/*=================================================================================================
** @Name      : static u8 ICCRcvAnsUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 修改回复帧接收处理
** @The notes : 对比数据是否修改成功和确认修改
**===============================================================================================*/
static u8 ICCRcvAnsUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum)
{
    const t_ICCIndexTab *pindextable;
    u8 wrResult = eICCERRREAS_NORMAL;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 matIndex = 0;
    u32 i;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引

    /*解析数据域*/
    framMark = iccFram->pData[0];
    sIndexNum = iccFram->pData[1];
    sIndex = iccFram->pData[2];

    /*非修改回复报文*/
    if(eICCFUN_AUP != iccFram->funCode)
    {
        return(eICCERR_KEEP_FUN);
    }

    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);

    /*主索引表信息存在*/
    if(pindextable != NULL)
    {

        /*核间通信报文长度不能小于索引长度*/
        if(iccFram->framLen < (11 + (sIndexNum * pindextable->dataType)))
        {
            /*子索引越界*/
            return(eICCERR_SIDLEN_OVE);
        }

        /*查找CAN收发通信表是否正在处理该报文(发送请求修改命令)*/
        matIndex = ICCFindTxRcvTabMatching(iccFram->sAddr, eICCFUN_RUP, iccFram->mIndex, sIndex);
        if(matIndex == 0xff)
        {
            /*无该通信收发表*/
            return(eICCERR_NOSUCH_TAB);
        }

        /*读取发送修改的数据与接收到的修改回复数据进行比较*/
        (void)ICCReadTxRcvTabData(matIndex, sIndexNum, readData);

        for(i = 0; i < (sIndexNum * pindextable->dataType); i++)
        {
            if(readData[i] != iccFram->pData[3 + i])
            {
                break;
            }
        }

        /*修改成功*/
        if(i >= (sIndexNum * pindextable->dataType))
        {
            /*正确接收，清零计时器*/
            ICCClrTxRcvTabTimer(matIndex);

            /*发送信息*/
            ICCSendTxRcvTabMsgPack(matIndex);

            /*置准备就绪标识*/
            gIccTxRcvTable[matIndex].ready = eICCTAB_WAIT;
            
            /*需要修改确认*/
            if(gIccTxRcvTable[matIndex].framMark == 0x0d)
            {
                /*发送修改确认命令*/
                (void)ICCBuildSendCupMsg_API(gIccTxRcvTable[matIndex].daddr, gIccTxRcvTable[matIndex].mIndex, gIccTxRcvTable[matIndex].sIndex, gIccTxRcvTable[matIndex].sIndexNum, iccNum);
            }

            /*清除该收发表*/
            //ICCClrTxRcvTabInfo(matIndex);
            

            errcode = eICCERR_SUCCESSFUL;
        }
        else
        {
            errcode = eICCERR_RUPW_FAIL;
        }

        return(errcode);
    }
    /*主索引表信息不存在*/
    else
    {
        return(errcode);
    }
}

/*=================================================================================================
** @Name      : static u8 ICCRcvConUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 修改确认帧接收处理
** @The notes : 调用用户HOOK
**===============================================================================================*/
static u8 ICCRcvConUpdateMsgDeal(t_ICCFram *iccFram, u8 iccNum)
{
    const t_ICCIndexTab *pindextable;
    u8 wrResult = eICCERRREAS_NORMAL;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 matIndex = 0;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引

    /*解析数据域*/
    framMark = iccFram->pData[0];
    sIndexNum = iccFram->pData[1];
    sIndex = iccFram->pData[2];

    /*非修改确认报文*/
    if(eICCFUN_CUP != iccFram->funCode)
    {
        return(eICCERR_KEEP_FUN);
    }


    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);

    /*主索引表信息存在*/
    if(pindextable != NULL)
    {
        /*报文长度不能小于索引长度*/
        if(iccFram->framLen < 11)
        {
            /*子索引越界*/
            return(eICCERR_SIDLEN_OVE);
        }

        /*修改确认*/
        if(framMark == 0X0d)
        {
            /*请求修改后处理函数*/
            wrResult = ICCRcvRupUserDeal_Hook(iccFram->sAddr, iccFram->mIndex, sIndex, sIndexNum, iccNum);

            /*请求修改正常*/
            if(eICCERRREAS_NORMAL == wrResult)
            {
                errcode = eICCERR_SUCCESSFUL;
            }
            else
            {
                /*发送错误应答报文(权限或参数错误)*/
                errcode = ICCBuildSendErrMsg_API(wrResult, iccFram, iccNum);
            }
        }
        else
        {
            errcode = eICCERR_RCVMSG_ERR;
        }

        return(errcode);
    }
    /*主索引表信息不存在*/
    else
    {
        /*发送错误应答报文(索引错误)*/
        (void)ICCBuildSendErrMsg_API(eICCERRREAS_INDEXERR, iccFram, iccNum);

        return(errcode);
    }
}

/*=================================================================================================
** @Name      : static u8 ICCRcvReqReadMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : canFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 请求查询帧接收处理
** @The notes : 调用用户HOOK更新数据域和回复
**===============================================================================================*/
static u8 ICCRcvReqReadMsgDeal(t_ICCFram *iccFram, u8 iccNum)
{
    const t_ICCIndexTab *pindextable;
    u8 wrResult = eICCERRREAS_NORMAL;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引

    /*解析数据域*/
    framMark = iccFram->pData[0];
    sIndexNum = iccFram->pData[1];
    sIndex = iccFram->pData[2];

    printf("iccmsg->framMark: %d\r\n",framMark);
    printf("iccmsg->sIndexNum: %d\r\n",sIndexNum);
    printf("iccmsg->sIndex: %d\r\n",sIndex);

    /*非请求查询报文*/
    if(eICCFUN_REQ != iccFram->funCode)
    {
        return(eICCERR_KEEP_FUN);
    }
    
    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);

    /*主索引表信息存在*/
    if(pindextable != NULL)
    {
        /*报文长度不能小于索引长度*/
        if(iccFram->framLen < 11)
        {
            //子索引越界
            return(eICCERR_SIDLEN_OVE);
        }

        /*查询长度为0表示查询剩余长度*/
        if(0 == sIndexNum)
        {
            //修改查询长度为剩余长度
            sIndexNum = pindextable->sIndexNum - sIndex;
        }

        /*请求查询回复前处理函数*/
        ICCRcvReqUserDeal_Hook(iccFram->sAddr, iccFram->mIndex, sIndex, sIndexNum, iccNum);
        /*生成查询回复信息*/
        return(ICCBuildSendArqMsg_API(iccFram, iccNum));
    }
    /*主索引表信息不存在*/
    else
    {
        /*发送错误应答报文(索引错误)*/
        (void)ICCBuildSendErrMsg_API(eICCERRREAS_INDEXERR, iccFram, iccNum);

        return(errcode);
    }
}

/*=================================================================================================
** @Name      : static u8 ICCRcvAnsReadMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 查询回复帧接收处理
** @The notes : 更新查询得到的数据和确认查询
**===============================================================================================*/
static u8 ICCRcvAnsReadMsgDeal(t_ICCFram *iccFram, u8 iccNum)
{
    u8 matIndex = 0;
    const t_ICCIndexTab *pindextable;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引

    /*解析数据域*/
    framMark = iccFram->pData[0];
    sIndexNum = iccFram->pData[1];
    sIndex = iccFram->pData[2];

    //非查询回复报文
    if(eICCFUN_ARQ != iccFram->funCode)
    {
        return(eICCERR_KEEP_FUN);
    }

    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);

    /*主索引表信息存在*/
    if(pindextable != NULL)
    {
        /*核间通信报文长度不能小于索引长度*/
        if(iccFram->framLen < (11 + (sIndexNum * pindextable->dataType)))
        {
            /*子索引越界*/
            return(eICCERR_SIDLEN_OVE);
        }

        /*查找CAN收发通信表是否正在处理该报文(发送请求查询命令)*/
        matIndex = ICCFindTxRcvTabMatching(iccFram->sAddr, eICCFUN_REQ, iccFram->mIndex, sIndex);
        if(matIndex == 0xff)
        {
            /*无该通信收发表*/
            return(eICCERR_NOSUCH_TAB);
        }

        /*更新收发通信表中等待回复的索引信息*/
        gIccTxRcvTable[matIndex].sIndex = sIndex;

        /*正确接收,清零计时器*/
        ICCClrTxRcvTabTimer(matIndex);

        /*将接收到的数据写入主索引表中*/
        errcode = ICCWriteCanIndexData(pindextable, sIndex, sIndexNum, &iccFram->pData[3], iccNum);

        /*写入主索引更新成功*/
        if(eICCERR_SUCCESSFUL == errcode)
        {
            /*如果该信息需要确认,则发送查询确认信息*/
            if(framMark == 0X0d)
            {

                ICCClrTxRcvTabInfo(matIndex);
                /*生成发送查询确认报文*/
                return(ICCBuildSendCrqMsg_API(iccFram, iccNum));
            }

            errcode = eICCERR_SUCCESSFUL;
        }
        /*查询回复写入主索引失败*/
        else
        {
            errcode = eICCERR_REQW_FAIL;
        }

        return(errcode);
    }
    /*主索引表信息不存在*/
    else
    {
        return(errcode);
    }
}

/*=================================================================================================
** @Name      : static u8 ICCRcvConReadMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 查询确认帧接收处理
** @The notes : 确认是否正确接收和发送下一帧查询回复
**===============================================================================================*/
static u8 ICCRcvConReadMsgDeal(t_ICCFram *iccFram, u8 iccNum)
{
    u8 i = {0};
    u8 matIndex = 0;
    u8 data[6] = {0};
    const t_ICCIndexTab *pindextable;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引

    /*解析数据域*/
    framMark = iccFram->pData[0];
    sIndexNum = iccFram->pData[1];
    sIndex = iccFram->pData[2];

    /*非查询确认报文*/
    if(eICCFUN_CRQ != iccFram->funCode)
    {
        return(eICCERR_KEEP_FUN);
    }

    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);

    /*主索引表信息存在*/
    if(pindextable != NULL)
    {
        /*核间通信报文长度不能小于索引长度*/
        if(iccFram->framLen < (11 + (sIndexNum * pindextable->dataType)))
        {
            /*子索引越界*/
            return(eICCERR_SIDLEN_OVE);
        }

        /*查找CAN收发通信表是否正在处理该报文(发送请求查询命令)*/
        matIndex = ICCFindTxRcvTabMatching(iccFram->sAddr, eICCFUN_ARQ, iccFram->mIndex, sIndex);
        if(matIndex == 0xff)
        {
            /*无该通信收发表*/
            return(eICCERR_NOSUCH_TAB);
        }

        /*读取回复的数据与接收到确认的数据进行比较*/
        (void)ICCReadTxRcvTabData(matIndex, sIndexNum, readData);

        for(i = 0; i < (sIndexNum * pindextable->dataType); i++)
        {
            if(readData[i] != iccFram->pData[3 + i])
            {
                break;
            }
        }

        /*确认查询回复接收成功*/
        if(i >= (sIndexNum * pindextable->dataType))
        {
            /*正确接收,清零计时器*/
            ICCClrTxRcvTabTimer(matIndex);

            /*有下一帧*/
            //if(gCanTxRcvTable[matIndex].remframe > 0)
            {
                /*发送信息*/
                ICCSendTxRcvTabMsgPack(matIndex);

                /*置准备就绪标识*/
                gIccTxRcvTable[matIndex].ready = eICCTAB_WAIT;
            }
            /*发送完全部查询回复*/
            //else
            {
                /*确认最后一帧查询回复*/
                //if()
                {
                    /*清除该收发表*/
                    ICCClrTxRcvTabInfo(matIndex);
                }
            }

            errcode = eICCERR_SUCCESSFUL;
        }
        /*未正确接收写入查询回复*/
        else
        {
            errcode = eICCERR_REQW_FAIL;
        }

        return(errcode);
    }
    /*主索引表信息不存在*/
    else
    {
        return(errcode);
    }
}

/*=================================================================================================
** @Name      : static u8 ICCRcvCycUpMsgDeal(t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : 周期主动更新帧接收处理
** @The notes : 更新接收得到的数据到索引表
**===============================================================================================*/
static u8 ICCRcvCycUpMsgDeal(t_ICCFram *iccFram, u8 iccNum)
{
    u8 *pData = 0;
    u8 mIndex = 0;
    u8 matIndex = 0;
    const t_ICCIndexTab *pindextable;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引
    t_ICCMapTrans iccMapInfo = {0};

    //周期主动更新报文
    if(eICCFUN_UPM == iccFram->funCode)
    {
        /*解析数据域*/
        framMark = iccFram->pData[0];
        sIndexNum = iccFram->pData[1];
        sIndex = iccFram->pData[2];
        pData = &iccFram->pData[3];
    }
    //周期主动更新单体或参数报文
    else if((eICCFUN_UPC == iccFram->funCode) || (eICCFUN_UPP == iccFram->funCode))
    {
        //获取索引信息
        pData = &iccFram->pData[0];

        /*通过映射求实际索引信息*/
        iccMapInfo = ICCIndexMapTransUser_Hook(iccFram->funCode,iccFram->mIndex, iccFram->framLen);
        mIndex = iccMapInfo.realMIndex;
        sIndex = iccMapInfo.realSIndex;
        sIndexNum = iccMapInfo.realSIndexNum;
    }
    //非周期主动更新帧报文
    else
    {
        return(eICCERR_KEEP_FUN);
    }

    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);

    /*主索引表信息存在*/
    if(pindextable != NULL)
    {
        //周期主动更新长帧报文
        if(eICCFUN_UPM == iccFram->funCode)
        {
            /*核间通信报文长度不能小于索引长度*/
            if(iccFram->framLen < (11 + (sIndexNum * pindextable->dataType)))
            {
                /*子索引越界*/
                return(eICCERR_SIDLEN_OVE);
            }
        }
        /*周期主动发送单帧信息*/
        else if(eICCFUN_UPC == iccFram->funCode)
        {
            /*求子索引数目*/
            sIndexNum = (iccFram->framLen - 8) / pindextable->dataType;

            /*不够一个子索引长度*/
            if(0 == sIndexNum)
            {
                /*报文错误*/
                return(eICCERR_RCVMSG_ERR);
            }
            /*写入超过子索引长度*/
            else if((sIndex + sIndexNum) > pindextable->sIndexNum)
            {
                /*子索引越界*/
                return(eICCERR_SIDLEN_OVE);
            }
        }

        /*将接收到的数据写入主索引表中*/
        errcode = ICCWriteCanIndexData(pindextable, sIndex, sIndexNum, pData, iccNum);

        /*写入主索引更新成功*/
        if(eICCERR_SUCCESSFUL == errcode)
        {
            /*接收周期更新报文后处理函数*/
            ICCRcvCycUpUserDeal_Hook (iccFram, iccNum);

            errcode = eICCERR_SUCCESSFUL;
        }
        /*主动更新写入主索引失败*/
        else
        {
            errcode = eICCERR_CYCW_FAIL;
        }

        return(errcode);
    }
    /*主索引表信息不存在*/
    else
    {
        return(errcode);
    }
}


/*=================================================================================================
** @Name      : u8 ICCBuildSendCupMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : destAddr:目标设备地址 mIndex:主索引号 sIndex:起始子索引号 sIndexNum:子索引数目 conFlag:是否需要确认 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送修改确认报文帧
** @The notes : 用户自行发送修改确认报文帧
**===============================================================================================*/
u8 ICCBuildSendCupMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
{
    /*取相应设备主索引数组数值发送*/
    const t_ICCIndexTab *pindextable;
    u8 errcode = eICCERR_SUCCESSFUL;

    /*CAN通道最大支持4路*/
    if(iccNum >= ICC_MAX_ICC_NUM)
    {
        return(eICCERR_NOSUCH_ICC);
    }

    /*查找目标设备主索引是否存在且判断子索引是否越界*/
    pindextable = IccScanIndexTabInfo(destAddr, mIndex, sIndex, sIndexNum, &errcode);
    if(pindextable == NULL)
    {
        return(errcode);
    }
    
    /*重新组合确认ID域(接收报文源地址变目标地址,下行变上行)*/
    ICCBuildMsgID_API(11, eICCFUN_CUP, mIndex, destAddr, txData);

    /*生成帧数据标识*/
    ICCBuildMsgDataMark_API(0x0d, sIndexNum, sIndex, &txData[8]);

    /*发送确认报文*/
    ICCSendMsgUser_Hook(txData, (3+8), iccNum);

    return(errcode);
}

/*=================================================================================================
** @Name      : u8 ICCBuildSendCrqMsg_API(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:接收到的ICC通信帧 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送查询确认报文帧
** @The notes : 用户自行发送查询确认报文帧
**===============================================================================================*/
u8 ICCBuildSendCrqMsg_API(t_ICCFram *iccFram, u8 iccNum)
{
    const t_ICCIndexTab *pindextable;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引

    /*解析数据域*/
    framMark = iccFram->pData[0];
    sIndexNum = iccFram->pData[1];
    sIndex = iccFram->pData[2];

    /*查找目标设备主索引是否存在且判断子索引是否越界*/
    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);
    if(pindextable == NULL)
    {
        return(errcode);
    }

    /*重新组合ID(接收报文源地址变目标地址)*/
    ICCBuildMsgID_API(iccFram->framLen, eICCFUN_CRQ, iccFram->mIndex, iccFram->sAddr, iccFram->pId);
    /*重新读取当前实际值*/
    ICCReadIccIndexData(pindextable, sIndex, sIndexNum, &iccFram->pData[3]);
    /*发送回复报文*/
    ICCSendMsgUser_Hook(iccFram->pId, ((sIndexNum * pindextable->dataType) + 11), iccNum);

    return(errcode);
}

/*=================================================================================================
** @Name      : u8 ICCBuildSendArqMsg_API(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:接收到的ICC通信帧 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送查询回复报文帧
** @The notes : 用户自行发送查询回复报文帧
**===============================================================================================*/
u8 ICCBuildSendArqMsg_API(t_ICCFram *iccFram, u8 iccNum)
{
    /*取相应设备主索引数组数值发送*/
    const t_ICCIndexTab *pindextable;
    u8 errcode = eICCERR_SUCCESSFUL;
    u8 framMark,sIndexNum,sIndex;   //帧标识、子索引数目、起始子索引

    /*CAN通道最大支持4路*/
    if(iccNum >= ICC_MAX_ICC_NUM)
    {
        return(eICCERR_NOSUCH_ICC);
    }
    /*解析数据域*/
    framMark = iccFram->pData[0];
    sIndexNum = iccFram->pData[1];
    sIndex = iccFram->pData[2];

    /*查找目标设备主索引是否存在且判断子索引是否越界*/
    pindextable = IccScanIndexTabInfo(iccFram->dAddr, iccFram->mIndex, sIndex, sIndexNum, &errcode);
    if(pindextable == NULL)
    {
        return(errcode);
    }
    /*申请空闲收发表进行发送*/
    return(ICCReqIdleTxRcvTab(eICCFUN_ARQ, pindextable->pObject, pindextable->dataType, iccFram->mIndex, sIndex, sIndexNum, framMark, iccFram->sAddr, iccNum));
}

static int iccmsg_c_test(int argc, char *argv[])
{
    int i = 0;
    if (argc == 0) {
        printf("iccmsg create test para error\n");

        return CLI_FALSE;
    }
    else if (!strcmp(argv[0], "test"))
    {
        u8 *data = NULL;
        data = ExtMalloc(sizeof(u8)*11);
        data[0] = 0X0B;
        data[1] = 00;
        data[2] = 00;
        data[3] = 00;
        data[4] = 0X05;
        data[5] = 0X64;
        data[6] = 0X32;
        data[7] = 0X5A;
        data[8] = 0x0C;
        data[9] = 0X08;
        data[10] = 0;

        // data[11] = 1;
        // data[12] = 2;
        // data[13] = 1;
        // data[14] = 2;
        // data[15] = 1;
        // data[16] = 2;
        // data[17] = 1;
        // data[18] = 2;
        // data[19] = 1;

        // data[20] = 2;
        // data[21] = 1;
        // data[22] = 2;
        // data[23] = 1;
        // data[24] = 2;
        // data[25] = 1;
        // data[26] = 2;
        // data[27] = 1;
        // data[28] = 2;
        
        // for(int i  = 0;i < 9 ;i++)
        // {
        //     printf("gSysHigParaRW_106[%d]:%d\r\n",i,gSysHigParaRW_106[i]);

        // }

        // iccFram->framLen = 19;
        // iccFram->funCode = eICCFUN_RUP;
        // iccFram->mIndex = 3;
        // iccFram->dAddr = 50;
        // iccFram->sAddr = 0xff;
        // iccFram->pData = &data[8];
        // iccFram->pId = &data[0];
        ICCLibRcvMsgHandle_API(data, 11, eICCCHA_FAQ);
        ExtFree(data);
        //ICCRcvReqUpdateMsgDeal(iccFram, eICCCHA_FAQ);
        return 0;
    }
    else if(!strcmp(argv[0], "show"))
    {
        for(i  = 0;i < 9 ;i++)
        {
            printf("gSysHigParaRW_106[%d]:%d\r\n",i,gSysHigParaRW_106[i]);

        }


        for(i  = 0;i < GINDEX50_LEN ;i++)
        {
            printf("gSysStaInfo_50[%d]:%d\r\n",i,gSysStaInfo_50[i]);

        }

        for(i  = 0;i < GINDEX51_LEN ;i++)
        {
            printf("gSysMainInfo_51[%d]:%d\r\n",i,gSysMainInfo_51[i]);

        }

        for(i  = 0;i < GINDEX52_LEN ;i++)
        {
            printf("gSysCapInfo_52[%d]:%d\r\n",i,gSysCapInfo_52[i]);

        }
        for(i  = 0;i < GINDEX53_LEN ;i++)
        {
            printf("gSysLimInfo_53[%d]:%d\r\n",i,gSysLimInfo_53[i]);

        }
        for(i  = 0;i < GINDEX54_LEN ;i++)
        {
            printf("gSysCharInfo_54[%d]:%d\r\n",i,gSysCharInfo_54[i]);

        }


        for(i  = 0;i < GINDEX55_LEN ;i++)
        {
            printf("gCurrCapEner_55[%d]:%d\r\n",i,gCurrCapEner_55[i]);

        }
        for(i  = 0;i < GINDEX56_LEN ;i++)
        {
            printf("gHisCapEner_56[%d]:%d\r\n",i,gHisCapEner_56[i]);

        }

        for(i  = 0;i < GINDEX57_LEN ;i++)
        {
            printf("gAlmFault1_57[%d]:%d\r\n",i,gAlmFault1_57[i]);

        }

        for(i  = 0;i < GINDEX58_LEN ;i++)
        {
            printf("gAlmFault2_58[%d]:%d\r\n",i,gAlmFault2_58[i]);
        }


        for(i  = 0;i < GINDEX59_LEN ;i++)
        {
            printf("gGStatistTime_59[%d]:%d\r\n",i,gGStatistTime_59[i]);
        }
        for(i  = 0;i < GINDEX60_LEN ;i++)
        {
            printf("gGStatistErr_60[%d]:%d\r\n",i,gGStatistErr_60[i]);
        }       
        
        
        for(i  = 0;i < GINDEX61_LEN ;i++)
        {
            printf("gGStatistSosInfo_61[%d]:%d\r\n",i,gGStatistSosInfo_61[i]);
        }
        for(i  = 0;i < GINDEX62_LEN ;i++)
        {
            printf("gGStatistDodInfo_62[%d]:%d\r\n",i,gGStatistDodInfo_62[i]);
        }  
        for(i  = 0;i < GINDEX63_LEN ;i++)
        {
            printf("gGAuthorInfo_63[%d]:%d\r\n",i,gGAuthorInfo_63[i]);
        }
        for(i  = 0;i < GINDEX64_LEN ;i++)
        {
            printf("gGAuthorCtrl_64[%d]:%d\r\n",i,gGAuthorCtrl_64[i]);
        }
        for(i  = 0;i < GINDEX65_LEN ;i++)
        {
            printf("gSysCtrlCmd_65[%d]:%d\r\n",i,gSysCtrlCmd_65[i]);
        }    
        
        


        return 0;
    }
    else
    {

        return 0;
    }
}

CLI_CMD("iccmsg_c_test",
        "iccmsg_c_test"
        "\r\n\t iccmsg_c_test test/show",
        iccmsg_c_test);

