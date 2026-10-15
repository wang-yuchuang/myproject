/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IccMsgTxCycle.c
** @Founder      : ZQD808
** @Date         : 2026.02.05
** @Function     : 核间通信通信包报文周期上传操作处理接口
** @Instructions :
**===============================================================================================*/
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "IccMsgCreate.h"
#include "IccMsgHandle.h"
#include "IccMsgInterface.h"
#include "IccMsgTxCycle.h"
#include "IccMsgUser.h"
#include "IccMsgTabOper.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "iccmsg.h"
#include <CLI.h>
#include <debug.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "BSPTypeDef.h"
#include <malloc.h>

/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
extern t_ICCCellDataDef *gIccMsgCellDataDef;          /*单体信息通道数据配置*/
extern t_ICCCycleDataDef *gIccMsgCycleDataDef;          /*周期信息通道数据配置*/
extern t_ICCTxTab* gIccTxCelTable;                /*核间通信周期发送处理表*/
extern t_ICCTxTab* gIccTxCycTable;                /*核间通信周期发送处理表*/
extern u8 *gIccBaseDef;         /*基本配置信息*/
extern u16 *gDevInfo_2;             /*设备信息*/


extern u16 *gGCellVoltInfo_1;        /*组1电池单体电压信息*/
extern u16 *gGChgBalanInfo_1;

extern iccmsg_t *iccmsg_cell;   //单体信息通道句柄
extern iccmsg_t *iccmsg_cycle;  //周期信息通道句柄
//extern iccmsg_t *iccmsg_pcs;    //模块信息通道句柄

extern u32 gIccCountTime;                                           //核间通信包总计时(1ms)

/*=================================================================================================
** @Static function declaration
**===============================================================================================*/
 
/*=================================================================================================
** @Interface function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : void ICCMSGTxCellCycleTabInit(void)
** @Input     : void
** @Output    : void
** @Function  : 核间通信包周期发送表初始化
** @The notes : 通信包初始化调用
**===============================================================================================*/
void ICCMSGTxCellCycleTabInit(void)
{
	u8 i = 0,j = 0,k = 0;
	u8 cellId = 0,cycleId = 0,pcsId = 0;
	u8 tabLen = 0;
	u8 mIndex = 0;
	u16 sIndex = 0;
	u16 sIndexNum = 0;
	u16 txTime = 0;
	u8 errcode = 0;
    const t_ICCIndexTab *pIndexTable;

    /*未定义发送表*/
    if(gIccTxCelTable == NULL)
    {
        return;
    }

    /*周期发送单体信息配置信息*/
    for(i = 0; i < gIccBaseDef[eICCRBase_TxCelNum]; i++)
    {
        /*获得用户配置的发送配置信息*/
    	mIndex = gIccMsgCellDataDef[i].mIndex;
    	sIndex = gIccMsgCellDataDef[i].sIndex;
    	sIndexNum = gIccMsgCellDataDef[i].sIndexNum;
    	txTime = (u16)gIccMsgCellDataDef[i].cycleTime;

        /*查找主索引表,获得该主索引的数据地址*/
    	pIndexTable = IccScanIndexTabInfo(gDevInfo_2[eICCDev2_Addr], mIndex, sIndex, sIndexNum, &errcode);

    	/*索引信息、数据类型和周期必须配置*/
        if((pIndexTable != NULL) && (pIndexTable->pObject != NULL) && (pIndexTable->dataType > 0) &&  (txTime >= ICC_TX_UPM_SPA_T))
        {
        	/*获得该主索引的数据地址*/
            gIccTxCelTable[cellId].pdata = pIndexTable->pObject;

            /*配置主动发送信息和启动时间*/
            gIccTxCelTable[cellId].dataType = pIndexTable->dataType;
            gIccTxCelTable[cellId].funcode = eICCFUN_UPC;
            gIccTxCelTable[cellId].index = i;
            gIccTxCelTable[cellId].nextTime = ICC_TX_UPM_SPA_T * i;
            cellId++;
            printf("the codes do not have bugs!!!cellId:%d****mIndex:%d\n",cellId,mIndex);
        }else{
            printf("the codes have bugs!!!mIndex:%d\n",mIndex);
        }
    }
    /*剩余单体发送表清零*/
    for(i = cellId; i < gIccBaseDef[eICCRBase_TxCelNum]; i++)
    {
        /*发送表清零*/
        gIccTxCelTable[i].pdata = NULL;
        gIccTxCelTable[i].dataType = 0;
        gIccTxCelTable[i].funcode = eICCFUN_VFT;
        gIccTxCelTable[i].index = 0;
        gIccTxCelTable[i].nextTime = 0;
    }


    /*未定义发送表*/
    if(gIccTxCycTable == NULL)
    {
        return;
    }

    /*周期发送循环信息配置信息*/
    for(j = 0; j < gIccBaseDef[eICCRBase_TxCylNum]; j++)
    {
        /*获得用户配置的发送配置信息*/
    	mIndex = gIccMsgCycleDataDef[j].mIndex;
    	sIndex = gIccMsgCycleDataDef[j].sIndex;
    	sIndexNum = gIccMsgCycleDataDef[j].sIndexNum;
    	txTime = (u16)gIccMsgCycleDataDef[j].cycleTime;

        /*查找主索引表,获得该主索引的数据地址*/
    	pIndexTable = IccScanIndexTabInfo(gDevInfo_2[eICCDev2_Addr], mIndex, sIndex, sIndexNum, &errcode);

    	/*索引信息、数据类型和周期必须配置*/
        if((pIndexTable != NULL) && (pIndexTable->pObject != NULL) && (pIndexTable->dataType > 0) &&  (txTime >= ICC_TX_UPM_SPA_T))
        {
        	/*获得该主索引的数据地址*/
            gIccTxCycTable[cycleId].pdata = pIndexTable->pObject;

            /*配置主动发送信息和启动时间*/
            gIccTxCycTable[cycleId].dataType = pIndexTable->dataType;
            gIccTxCycTable[cycleId].funcode = eICCFUN_UPM;
            gIccTxCycTable[cycleId].index = j;
            gIccTxCycTable[cycleId].nextTime = ICC_TX_UPM_SPA_T * j;
            cycleId++;
            printf("the codes do not have bugs!!!cycleId:%d****mIndex:%d\n",cycleId,mIndex);
        }else{
            printf("the codes have bugs!!!mIndex:%d\n",mIndex);
        }
    }


    /*剩余循环发送表清零*/
    for(j = cycleId; j < gIccBaseDef[eICCRBase_TxCylNum]; j++)
    {
        /*发送表清零*/
        gIccTxCycTable[j].pdata = NULL;
        gIccTxCycTable[j].dataType = 0;
        gIccTxCycTable[j].funcode = eICCFUN_VFT;
        gIccTxCycTable[j].index = 0;
        gIccTxCycTable[j].nextTime = 0;
    }

    // /*未定义发送表*/
    // {
    //     return;
    // }

    // /*周期发送模块信息配置信息*/
}


/*=================================================================================================
** @Name      : void ICCTxCellCycleTabHandle(void)
** @Input     : void
** @Output    : void
** @Function  : 核间通信包单体和循环发送表处理任务
** @The notes : 通信包总任务10ms调用
**===============================================================================================*/
void ICCTxCellCycleTabHandle(void)
{
    u8 i = 0;
	u8 mIndex = 0;
	u16 sIndex = 0;
	u16 sIndexNum = 0;
	u16 txTime = 0;
	u8 destAddr = 0;
	u8 mapIndex = 0;
	u8 tabLen = 0;
    /*核间通信未初始化*/
    if(iccmsg_cell == NULL || iccmsg_cycle == NULL)
    {
        return;
    }

    /*未定义发送表*/
    if(gIccTxCelTable == NULL)
    {
        return;
    }

    /*逐个检测每个通信包*/
    for(i = 0; i < gIccBaseDef[eICCRBase_TxCelNum]; i++)
    {
    	/*到达发送周期*/
        if( (gIccCountTime >= gIccTxCelTable[i].nextTime) || (gIccCountTime + 100000 < gIccTxCelTable[i].nextTime))
        {
        	/*通信发送信息有效*/
            if(gIccTxCelTable[i].pdata != NULL)
            {
                /*发送单体*/
            	if(eICCFUN_UPC == gIccTxCelTable[i].funcode)
                {
                    /*使能发送单体帧*/
                    if(1 == gIccMsgCellDataDef[gIccTxCelTable[i].index].txAble)
                    {
                        /*每组6个信息*/
                        mIndex = gIccMsgCellDataDef[gIccTxCelTable[i].index].mIndex;
                        sIndex = gIccMsgCellDataDef[gIccTxCelTable[i].index].sIndex;
                        sIndexNum = gIccMsgCellDataDef[gIccTxCelTable[i].index].sIndexNum;
                        mapIndex = gIccMsgCellDataDef[gIccTxCelTable[i].index].mapIndex;
                        txTime = gIccMsgCellDataDef[gIccTxCelTable[i].index].cycleTime;
                        destAddr = gIccMsgCellDataDef[gIccTxCelTable[i].index].daddr;

                        /*用户允许发送*/
						//if(TRUE == ICCTxCellUpUserEnState_Hook(eICCFUN_UPC, mIndex))
						{
							/*生成主动发送信息*/
            	            (void)ICCReqIdleTxRcvTab(eICCFUN_UPC, gIccTxCelTable[i].pdata, gIccTxCelTable[i].dataType, mIndex, sIndex, sIndexNum, mapIndex, destAddr, eICCCHA_CEL);
						}
                    }
                }
                /*更新下一次发送信息的时间*/
                gIccTxCelTable[i].nextTime =  gIccCountTime + txTime;
                /*超过最大时间时间*/
                if(gIccTxCelTable[i].nextTime >= ICC_MAX_COUNT_TIME)
                {
                    /*回头计时*/
                    gIccTxCelTable[i].nextTime = gIccTxCelTable[i].nextTime - ICC_MAX_COUNT_TIME;
                }
                
            }

        }
        
    }
    


    /*未定义发送表*/
    if(gIccTxCycTable == NULL) 
    {
        return;
    }

    /*逐个检测每个通信包*/
    for(i = 0; i < gIccBaseDef[eICCRBase_TxCylNum]; i++)
    {
    	/*到达发送周期*/
        if( (gIccCountTime >= gIccTxCycTable[i].nextTime) || (gIccCountTime + 100000 < gIccTxCycTable[i].nextTime))
        {
        	/*通信发送信息有效*/
            if(gIccTxCycTable[i].pdata != NULL)
            {
                /*发送循环帧*/
            	if(eICCFUN_UPM == gIccTxCycTable[i].funcode)
                {
                    /*使能发送循环帧*/
                    if(1 == gIccMsgCycleDataDef[gIccTxCycTable[i].index].txAble)
                    {
                        /*每组5个信息*/
                        mIndex = gIccMsgCycleDataDef[gIccTxCycTable[i].index].mIndex;
                        sIndex = gIccMsgCycleDataDef[gIccTxCycTable[i].index].sIndex;
                        sIndexNum = gIccMsgCycleDataDef[gIccTxCycTable[i].index].sIndexNum;
                        txTime = gIccMsgCycleDataDef[gIccTxCycTable[i].index].cycleTime;
                        destAddr = gIccMsgCycleDataDef[gIccTxCycTable[i].index].daddr;

                        /*用户允许发送*/
						//if(TRUE == ICCTxCellUpUserEnState_Hook(eICCFUN_UPC, mIndex))
						{
							/*生成主动发送信息*/
            	            (void)ICCReqIdleTxRcvTab(eICCFUN_UPM, gIccTxCycTable[i].pdata, gIccTxCycTable[i].dataType, mIndex, sIndex, sIndexNum, 0, destAddr, eICCCHA_CYC);
						}
                    }
                }
                /*更新下一次发送信息的时间*/
                gIccTxCycTable[i].nextTime =  gIccCountTime + txTime;
                /*超过最大时间时间*/
                if(gIccTxCycTable[i].nextTime >= ICC_MAX_COUNT_TIME)
                {
                    /*回头计时*/
                    gIccTxCycTable[i].nextTime = gIccTxCycTable[i].nextTime - ICC_MAX_COUNT_TIME;
                }
            }
        }
    }
}


/*=================================================================================================
** @Static function definition
**===============================================================================================*/
static int iccmsg_cycletx_test(int argc, char *argv[])
{
    if (argc == 0) {
        printf("iccmsg cycletx test para error\n");
        return CLI_FALSE;
    }else
    {
        printf("the parameters cannot be recognized\n");
        return 0;
    }
}
CLI_CMD("iccmsg_cycletx_test",
        "iccmsg_cycletx_test"
        "\r\n\t iccmsg_cycletx_test tx",
        iccmsg_cycletx_test);