/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IccMsgTabOper.c
** @Founder      : WYC834
** @Date         : 2026.03.12
** @Function     : 核间通信包表格读写操作接口
** @Instructions :
**===============================================================================================*/
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "IccMsgTabOper.h"
#include "IccMsgUser.h"
#include "IccMsgHandle.h"
#include "GroupInfo.h"
#include "iccmsg_dpr.h"
#include <CLI.h>
#include "iccmsg.h"


/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
extern u8 *gIccBaseDef;
extern t_ICCDevTab* gIccDevTable;
extern u16 *gDevInfo_2;         /*设备信息*/
extern t_ICCTxRcvTab* gIccTxRcvTable;
extern u8 *txData;              //发送缓冲区


extern iccmsg_t *iccmsg_cell;   //单体信息通道句柄
extern iccmsg_t *iccmsg_cycle;  //周期信息通道句柄
extern iccmsg_t *iccmsg_faq;    //问答信息通道句柄
extern iccmsg_t *iccmsg_update; //升级信息通道句柄


extern u16 *gGCellVoltInfo_1;        /*组1电池单体电压信息*/
extern u16 *gGChgBalanInfo_1;
/*=================================================================================================
** @Static function declaration
**===============================================================================================*/
 
/*=================================================================================================
** @Name      : static u8 ICCFindDevTabMIndexDim(u8 findIndex, u8 mindexNum, const t_ICCIndexTab *pindextable)
** @Input     : findIndex:查找的主索引号  mindexNum:主索引数目 *pindextable:所要查找的主索引表
** @Output    : 对应主索引所在位置
** @Function  : 查找主索引所在设备表中偏移位置(第几个主索引)
** @The notes : 二分查找法
**===============================================================================================*/
static u8 ICCFindDevTabMIndexDim(u8 findIndex, u8 mindexNum, const t_ICCIndexTab *pindextable);




/*=================================================================================================
** @Interface function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : const t_ICCIndexTab *IccScanIndexTabInfo(u8 devaddr, u8 mindex, u16 sIndex, u16 sIndexNum, u8 *errorCode)
** @Input     : devaddr:索引表设备地址 mindex:主索引号  sindex:子索引号  sIndexNum:子索引数目 *errorCode:存储错误码地址
** @Output    : 对应主索引信息
** @Function  : 查找主索引表信息
** @The notes : 并且判断子索引是否越界
**===============================================================================================*/
const t_ICCIndexTab *IccScanIndexTabInfo(u8 devaddr, u8 mindex, u16 sIndex, u16 sIndexNum, u8 *errorCode)
{
    u8 i = 0;
    u8 offset = 0;
    t_ICCDevTab *pDeviceTable;
    const t_ICCIndexTab *pIndexTable;

    *errorCode = eICCERR_SUCCESSFUL;

    for(i = 0; i < gIccBaseDef[eICCRBase_DevNum]; i++)
    {
        //获得设备表
        pDeviceTable = &gIccDevTable[i];
        if(pDeviceTable == NULL)
        {
            //该设备不存在
            *errorCode = eICCERR_NOSUCH_DEV;

            return(NULL);
        }

        //广播报文当作自身报文
        if(0xff == devaddr)
        {
        	devaddr = gDevInfo_2[eICCDev2_Addr];
        }
        //找到当前设备表
        if(pDeviceTable->devAdrr == devaddr)
        {
            //二分查找主索引表
            offset = ICCFindDevTabMIndexDim(mindex, pDeviceTable->mIndexNum, pDeviceTable->pIndexTable);
            /*未找到对应主索引*/
            if(offset == 0xff)
            {
                *errorCode = eICCERR_NOSUCH_MID;
            }
            else
            {
                //对应主索引所在位置
                pIndexTable = pDeviceTable->pIndexTable + offset;
                //判断查找的子索引是否越界
                if((pIndexTable->sIndexNum >= (sIndex + sIndexNum)) && (pIndexTable->sIndexNum > sIndex))
                {
                    //找到主索引表
                    return(pIndexTable);
                }
                else
                {
                    /*子索引越界*/
                    *errorCode = eICCERR_SIDLEN_OVE;
                }
            }
            break;
        }
    }

    if(i >= gIccBaseDef[eICCRBase_DevNum])
    {
        //该设备不存在
        *errorCode = eICCERR_NOSUCH_DEV;
    }

    //未找到主索引表
    return(NULL);
}

/*=================================================================================================
** @Name      : void ICCClrTxRcvTabInfo(u8 id)
** @Input     : id:核间通信收发通信表序号
** @Output    : void
** @Function  : 清除核间通信收发通信表中的记录
** @The notes :
**===============================================================================================*/
void ICCClrTxRcvTabInfo(u8 id)
{
    /*通信收发表序号越界*/
    if(id >= gIccBaseDef[eICCRBase_TRTabNum])
    {
        return;
    }

    gIccTxRcvTable[id].ready = eICCTAB_LEISUREL;

	gIccTxRcvTable[id].pdata = NULL;

	gIccTxRcvTable[id].funcode = eICCFUN_VFT;
	gIccTxRcvTable[id].sIndex = 0;
	gIccTxRcvTable[id].sIndexNum = 0;
	gIccTxRcvTable[id].daddr = 0;
	gIccTxRcvTable[id].framMark = 0;
	gIccTxRcvTable[id].mIndex = 0;
	gIccTxRcvTable[id].dataType = 0;
	gIccTxRcvTable[id].iccnum = 0;
	gIccTxRcvTable[id].timer.timeResend = 0;
	gIccTxRcvTable[id].timer.timeElasped = 0;
}



/*=================================================================================================
** @Name      : u8 ICCReqIdleTxRcvTab(e_ICCFunCode funcode, void *pdata, u8 dataType, u8 mIndex, u16 sIndex, u16 sIndexNum, u8 conflag, u8 daddr, e_ICCChannelDef iccnum)
** @Input     : funcode:功能码 pdata:数据首地址 dataType:数据类型 mIndex:主索引  sindex:子索引 sindexnum:子索引数目 conflag:确认标志[修改/查询]或映射起始主索引[单体/参数]  daddr:目的地址, iccnum:核间通信通道
** @Output    : 申请结果(0:成功 其它:错误码)
** @Function  : 请求空闲的收发表记录发送信息
** @The notes : 设置定时发送
**===============================================================================================*/
u8 ICCReqIdleTxRcvTab(e_ICCFunCode funcode, void *pdata, u8 dataType, u8 mIndex, u16 sIndex, u16 sIndexNum, u8 conflag, u8 daddr, e_ICCChannelDef iccnum)
{
    u8 i = 0;
    u8 errcode = eICCERR_NOIDLE_TAB;

    /*ICC框架未初始化时缓冲区为 NULL,跳过发送避免写空指针崩溃*/
    if((gIccBaseDef == NULL) || (gIccTxRcvTable == NULL) || (txData == NULL))
    {
        return(eICCERR_FAIL);
    }
    /*查找空闲发送区*/
    for(i = 0; i < gIccBaseDef[eICCRBase_TRTabNum]; i++)
    {
        /*空闲监听状态*/
        if(gIccTxRcvTable[i].ready == eICCTAB_LEISUREL)
        {
            gIccTxRcvTable[i].funcode = funcode;
            gIccTxRcvTable[i].pdata = pdata;
            gIccTxRcvTable[i].dataType = dataType;

            /*单体信息功能码和参数信息功能码命令字当做映射起始主索引使用*/
            if((eICCFUN_UPC == funcode) || (eICCFUN_UPP == funcode))
            {
            	gIccTxRcvTable[i].mIndex = conflag;
                gIccTxRcvTable[i].framMark = 0x0c;
            }
            /*只有请求修改和请求查询有确认标准*/
            else
            {
            	gIccTxRcvTable[i].mIndex = mIndex;
                gIccTxRcvTable[i].framMark = 0x0d;
            }
            gIccTxRcvTable[i].sIndex = sIndex;
            gIccTxRcvTable[i].sIndexNum = sIndexNum;
            gIccTxRcvTable[i].daddr = daddr;
            gIccTxRcvTable[i].iccnum = iccnum;
            gIccTxRcvTable[i].ready = eICCTAB_WAIT;
            //发送信息数据包
            errcode = ICCSendTxRcvTabMsgPack(i);
            break;
        }
        else
        {

        }
    }

    return(errcode);
}

/*=================================================================================================
** @Name      : u8 ICCSendTxRcvTabMsgPack(u8 id)
** @Input     : id:ICC收发通信表序号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 发送ICC收发通信表中需要发送的数据包
** @The notes : ICC收发通信表有报文需要发送时调用
**===============================================================================================*/
u8 ICCSendTxRcvTabMsgPack(u8 id)
{
    u16 msgLen = 0;
    u8 errcode = eICCERR_SUCCESSFUL;
    /*通信收发表序号越界*/
    if(id >= gIccBaseDef[eICCRBase_TRTabNum])
    {
        return(eICCERR_NOSUCH_TAB);
    }

    /*ICC框架未初始化时 txData 为 NULL,跳过发送避免写空指针崩溃*/
    if(txData == NULL)
    {
        return(eICCERR_FAIL);
    }
    /*发送单体信息*/
    if(gIccTxRcvTable[id].iccnum == eICCCHA_CEL)
    {
        if(gIccTxRcvTable[id].funcode == eICCFUN_UPC)
        {
            /*计算需要发送的数据域字节数*/
            msgLen = (u16)gIccTxRcvTable[id].sIndexNum * gIccTxRcvTable[id].dataType;

            /*需要发送的字节数不能超过1帧1024个字节*/
            if((msgLen + 8) <= 1024)
            {
                /*生成帧头ID数据*/
                ICCBuildMsgID_API((msgLen+8),gIccTxRcvTable[id].funcode, gIccTxRcvTable[id].mIndex, gIccTxRcvTable[id].daddr, txData);
                /*读通信收发表中子索引数据*/
                ICCReadTxRcvTabData(id, gIccTxRcvTable[id].sIndexNum, &txData[8]);
                /*发送报文*/
                ICCSendMsgUser_Hook(txData, (msgLen+8), gIccTxRcvTable[id].iccnum);

            }
            /*需要发送的字节数超过1帧1024个字节*/
            else
            {
                /*子索引越界*/
                errcode = eICCERR_SIDLEN_OVE;
            }

            /*单帧则发送完成直接清零收发通信表*/
            ICCClrTxRcvTabInfo(id);

        }

    }
    /*发送循环信息*/
    else if(gIccTxRcvTable[id].iccnum == eICCCHA_CYC)
    {
        if(gIccTxRcvTable[id].funcode == eICCFUN_UPM)
        {
            /*计算需要发送的数据域字节数*/
            msgLen = (u16)gIccTxRcvTable[id].sIndexNum * gIccTxRcvTable[id].dataType + 3;//需要3字节标识数据域信息

            /*需要发送的字节数不能超过1帧1024个字节*/
            if((msgLen + 8) <= 1024)
            {
                /*生成帧头ID数据*/
                ICCBuildMsgID_API((msgLen+8),gIccTxRcvTable[id].funcode, gIccTxRcvTable[id].mIndex, gIccTxRcvTable[id].daddr, txData);
                /*生成帧数据标识*/
                ICCBuildMsgDataMark_API(0x0c, gIccTxRcvTable[id].sIndexNum, 0, &txData[8]);
                /*读通信收发表中子索引数据*/
                ICCReadTxRcvTabData(id, gIccTxRcvTable[id].sIndexNum, &txData[11]);
                /*发送报文*/
                ICCSendMsgUser_Hook(txData, (msgLen+8), gIccTxRcvTable[id].iccnum);

            }
            /*需要发送的字节数超过1帧1024个字节*/
            else
            {
                /*子索引越界*/
                errcode = eICCERR_SIDLEN_OVE;
            }

            /*循环帧则发送完成直接清零收发通信表*/
            ICCClrTxRcvTabInfo(id);

        }

    }
    // /*发送模块循环信息*/
    // else if(gIccTxRcvTable[id].iccnum == eICCCHA_PCS)
    // {
    //     if(gIccTxRcvTable[id].funcode == eICCFUN_UPM)
    //     {
    //         /*计算需要发送的数据域字节数*/
    //         msgLen = (u16)gIccTxRcvTable[id].sIndexNum * gIccTxRcvTable[id].dataType + 3;//需要3字节标识数据域信息

    //         /*需要发送的字节数不能超过1帧1024个字节*/
    //         if((msgLen + 8) <= 1024)
    //         {
    //             /*生成帧头ID数据*/
    //             ICCBuildMsgID_API((msgLen+8),gIccTxRcvTable[id].funcode, gIccTxRcvTable[id].mIndex, gIccTxRcvTable[id].daddr, txData);
    //             /*生成帧数据标识*/
    //             ICCBuildMsgDataMark_API(0x0c, gIccTxRcvTable[id].sIndexNum, 0, &txData[8]);
    //             /*读通信收发表中子索引数据*/
    //             ICCReadTxRcvTabData(id, gIccTxRcvTable[id].sIndexNum, &txData[11]);
    //             /*发送报文*/
    //             ICCSendMsgUser_Hook(txData, (msgLen+8), gIccTxRcvTable[id].iccnum);

    //         }
    //         /*需要发送的字节数超过1帧1024个字节*/
    //         else
    //         {
    //             /*子索引越界*/
    //             errcode = eICCERR_SIDLEN_OVE;
    //         }

    //         /*循环帧则发送完成直接清零收发通信表*/
    //         ICCClrTxRcvTabInfo(id);

    //     }

    // }
    /*发送查询信息*/
    else if(gIccTxRcvTable[id].iccnum == eICCCHA_FAQ || gIccTxRcvTable[id].iccnum == eICCCHA_PCF)
    {
        if(gIccTxRcvTable[id].funcode == eICCFUN_ARQ || gIccTxRcvTable[id].funcode == eICCFUN_AUP || gIccTxRcvTable[id].funcode == eICCFUN_UPM)
        {
            /*计算需要发送的数据域字节数*/
            msgLen = (u16)gIccTxRcvTable[id].sIndexNum * gIccTxRcvTable[id].dataType + 3;//需要3字节标识数据域信息

            /*需要发送的字节数不能超过1帧1024个字节*/
            if((msgLen + 8) <= 1024)
            {
                /*生成帧头ID数据*/
                ICCBuildMsgID_API((msgLen+8),gIccTxRcvTable[id].funcode, gIccTxRcvTable[id].mIndex, gIccTxRcvTable[id].daddr, txData);
                /*生成帧数据标识*/
                ICCBuildMsgDataMark_API(0x0c, gIccTxRcvTable[id].sIndexNum, 0, &txData[8]);
                /*读通信收发表中子索引数据*/
                ICCReadTxRcvTabData(id, gIccTxRcvTable[id].sIndexNum, &txData[11]);
                /*发送报文*/
                ICCSendMsgUser_Hook(txData, (msgLen+8), gIccTxRcvTable[id].iccnum);

            }
            /*需要发送的字节数超过1帧1024个字节*/
            else
            {
                /*子索引越界*/
                errcode = eICCERR_SIDLEN_OVE;
            }

            /*发送完成直接清零收发通信表*/
            ICCClrTxRcvTabInfo(id);

        }

    }
    /*其它不支持的功能码*/
    else
    {
    	/*无此功能码,直接清除该收发通信表*/
        ICCClrTxRcvTabInfo(id);
    	errcode = eICCERR_NOSUCH_FUN;
    }

    return(errcode);
}

/*=================================================================================================
** @Name      : void ICCReadTxRcvTabData(u8 id, u16 readlen, u8 *pdata)
** @Input     : id:ICC收发通信表序号 readlen:读数据长度 *pdata:读到的数据
** @Output    : void
** @Function  : 读取ICC收发通信表中缓存区的数据
** @The notes : 确保读取长度必须有效
**===============================================================================================*/
void ICCReadTxRcvTabData(u8 id, u16 readlen, u8 *pdata)
{
    u16 i = 0;
    /*通信收发表序号越界*/
    if(id >= gIccBaseDef[eICCRBase_TRTabNum])
    {
        return;
    }

    //读通讯缓存中数据,前8字节为已经赋值的ID域
    if(gIccTxRcvTable[id].pdata != NULL)
    {
        switch(gIccTxRcvTable[id].dataType)
        {
            case 1:
                for(i = 0; i < readlen; i++)
                {
                    *(pdata + i) = *((u8*)gIccTxRcvTable[id].pdata + gIccTxRcvTable[id].sIndex + i);
                }
                break;

            case 2:
                for(i = 0; i < readlen; i++)
                {
                    u8 *pSrc = (u8*)gIccTxRcvTable[id].pdata + (gIccTxRcvTable[id].sIndex + i) * 2;
                    *(pdata + 2 * i) = pSrc[0];
                    *(pdata + 2 * i + 1) = pSrc[1];
                }
                break;

            case 4:
                for(i = 0; i < readlen; i++)
                {
                    u8 *pSrc = (u8*)gIccTxRcvTable[id].pdata + (gIccTxRcvTable[id].sIndex + i) * 4;
                    *(pdata + 4 * i) = pSrc[0];
                    *(pdata + 4 * i + 1) = pSrc[1];
                    *(pdata + 4 * i + 2) = pSrc[2];
                    *(pdata + 4 * i + 3) = pSrc[3];
                }
                break;

            default:
                break;
        }
    }
}

/*=================================================================================================
** @Name      : void ICCReadIccIndexData(const t_ICCIndexTab *ptrTable, u8 subIndex, u8 sIndexNum, u8 *Data)
** @Input     : *ptrTable:主索引信息  sindex:子索引号  sIndexNum:子索引数目 *Data:读出的数据(多字节则低位在前)
** @Output    : void
** @Function  : 读索引表中对应子索引当前数据值
** @The notes : 必须确保主索引存在且子索引长度合法
**===============================================================================================*/
void ICCReadIccIndexData(const t_ICCIndexTab *ptrTable, u8 subIndex, u8 sIndexNum, u8 *Data)
{
    u8 i = 0;
    u8 dataType = 0;
    u16 Datatemp = 0;
    u32 Datatemp1 = 0;
    void *pObject = NULL;

    //主索引数据地址
    pObject = ptrTable->pObject;

    //主索引数据类型
    dataType = ptrTable->dataType;

    /*主索引有效*/
    if(pObject != NULL)
    {
        /*逐个读取各个子索引*/
        for(i = 0; i < sIndexNum; i++)
        {
            /*8位*/
            u8 *pObjBytes = (u8*)ptrTable->pObject;
            if(dataType == 1)
            {
                pObject = (u8*)(ptrTable->pObject) + (subIndex + i);
                *(Data + i) = *((u8*)pObject);
            }
            else if(dataType == 2)
            {
                u16 byteIdx = (subIndex + i) * 2;
                *(Data + 2 * i) = pObjBytes[byteIdx];
                *(Data + 2 * i + 1) = pObjBytes[byteIdx + 1];
            }
            else if(dataType == 4)
            {
                u16 byteIdx = (subIndex + i) * 4;
                *(Data + 4 * i) = pObjBytes[byteIdx];
                *(Data + 4 * i + 1) = pObjBytes[byteIdx + 1];
                *(Data + 4 * i + 2) = pObjBytes[byteIdx + 2];
                *(Data + 4 * i + 3) = pObjBytes[byteIdx + 3];
            }
        	// /*16位*/
            // else if(dataType == 2)
            // {
            //     pObject = (u16*)(ptrTable->pObject) + (subIndex + i);
            //     Datatemp = *((u16*)pObject);;
            //     *(Data + 2 * i) = (u8)(Datatemp & 0x00FF);
            //     *(Data + 2 * i + 1) = (u8)((Datatemp & 0xFF00) >> 8);
            // }
        	// /*32位*/
        	// else if(dataType == 4)
            // {
            //     pObject = (u32*)(ptrTable->pObject) + (subIndex + i);
            //     Datatemp1 = *((u32*)pObject);
            //     *(Data + 4 * i) = (u8)(Datatemp1 & 0x000000FF);
            //     *(Data + 4 * i + 1) = (u8)((Datatemp1 & 0x0000FF00) >> 8);
            //     *(Data + 4 * i + 2) = (u8)((Datatemp1 & 0x00FF0000) >> 16);
            //     *(Data + 4 * i + 3) = (u8)((Datatemp1 & 0xFF000000) >> 24);
            // }
        }
    }
}



/*=================================================================================================
** @Name      : u8 ICCWriteCanIndexData(const t_ICCIndexTab *ptrTable, u8 subIndex, u8 sIndexNum, u8 *Data, u8 iccnum)
** @Input     : *ptrTable:主索引信息  sindex:子索引号  sIndexNum:子索引数目 *Data:需写入的数据(多字节则低位在前) iccnum:核间通道号(暂时没用到)
** @Output    : 写结果(0:成功 其它:错误码)
** @Function  : 向索引表中对应子索引写入数据值
** @The notes : 必须确保主索引存在且子索引长度合法
**===============================================================================================*/
/*=================================================================================================
** @Name      : static u8 IccCheckParaCfgFlagProtect(const t_ICCIndexTab *ptrTable, u8 subIndex)
** @Input     : ptrTable:主索引表项 subIndex:起始子索引
** @Output    : 错误码(0:允许写入 其它:拒绝)
** @Function  : 参数设置组(105-109)子索引0为EEPROM配置标志CfgFlag(0x5a5a),外部写入会废掉配置有效性,导致重启回默认值
** @The notes : 拒绝覆盖CfgFlag的整帧写入;正常按约定(+1偏移)写入起始子索引>=1不受影响
**===============================================================================================*/
static u8 IccCheckParaCfgFlagProtect(const t_ICCIndexTab *ptrTable, u8 subIndex)
{
    u8 mIndex = ptrTable->mIndex;

    if((mIndex >= 105) && (mIndex <= 109))
    {
        if(0 == subIndex)
        {
            return(eICCERR_RUPW_FAIL);
        }
    }
    return(eICCERR_SUCCESSFUL);
}

u8 ICCWriteCanIndexData(const t_ICCIndexTab *ptrTable, u8 subIndex, u8 sIndexNum, u8 *Data, u8 iccnum)
{
    u8 i = 0;
    u8 dataType = 0;
    u16 Datatemp = 0;
    u32 Datatemp1 =0;
    //void *pObject = NULL;

    //主索引数据地址
    //pObject = ptrTable->pObject;
    //逐个修改各个子索引
    u8 *pObjBytes = (u8*)ptrTable->pObject;
    //主索引数据类型
    dataType = ptrTable->dataType;

    if(pObjBytes != NULL)
    {
        u8 protRc = IccCheckParaCfgFlagProtect(ptrTable, subIndex);
        if(protRc != eICCERR_SUCCESSFUL)
        {
            return(protRc);
        }

        for(i = 0; i < sIndexNum; i++)
        {
            if(dataType == 1)
            {
                pObjBytes[subIndex + i] = *(Data + i);
                //printf("data[%d]: %d\r\n",(subIndex + i),pObjBytes[subIndex + i]); //调试刷屏,已屏蔽 2026-08-31
            }
            else if(dataType == 2)
            {
                u16 byteIdx = (subIndex + i) * 2;
                pObjBytes[byteIdx] = *(Data + 2 * i);
                pObjBytes[byteIdx + 1] = *(Data + 2 * i + 1);
                //printf("data[%d]: %d\r\n",(subIndex + i),(pObjBytes[byteIdx + 1] << 8) | pObjBytes[byteIdx]); //调试刷屏,已屏蔽 2026-08-31
            }
            else if(dataType == 4)
            {
                u16 byteIdx = (subIndex + i) * 4;
                pObjBytes[byteIdx] = *(Data + 4 * i);
                pObjBytes[byteIdx + 1] = *(Data + 4 * i + 1);
                pObjBytes[byteIdx + 2] = *(Data + 4 * i + 2);
                pObjBytes[byteIdx + 3] = *(Data + 4 * i + 3);
                //printf("data[%d]: %d\r\n",(subIndex + i),(pObjBytes[byteIdx + 2] << 24)|(pObjBytes[byteIdx + 2] << 16)|(pObjBytes[byteIdx + 1] << 8) | pObjBytes[byteIdx]); //调试刷屏,已屏蔽 2026-08-31
            }
        }
    }

    return(eICCERR_SUCCESSFUL);
}

/*=================================================================================================
** @Name      : u8 ICCFindTxRcvTabMatching(u8 destAddr, e_ICCFunCode funcode, u8 mindex, u8 sindex)
** @Input     : destAddr:源地址 funcode:功能码 mindex:主索引 sindex:子索引
** @Output    : void
** @Function  : 查找核间通信收发通信表中与输入参数匹配的序号
** @The notes : 查找是否正处于收发状态
**===============================================================================================*/
u8 ICCFindTxRcvTabMatching(u8 destAddr, e_ICCFunCode funcode, u8 mindex, u8 sindex)
{
    u8 i = 0;

    /*轮询所有CAN收发通信表*/
    for(i = 0; i < gIccBaseDef[eICCRBase_TRTabNum]; i++)
    {
        /*要查找的参数与该CAN收发通信表匹配*/
        if((gIccTxRcvTable[i].daddr == destAddr) && (gIccTxRcvTable[i].funcode == funcode) && (gIccTxRcvTable[i].mIndex == mindex)
            && ((gIccTxRcvTable[i].sIndex == sindex)))
        {
            /*返回该CAN收发通信表序号*/
            return(i);
        }
    }

    return(0xff);
}

/*=================================================================================================
** @Name      : void ICCClrTxRcvTabTimer(u8 id)
** @Input     : id:ICC收发通信表序号
** @Output    : void
** @Function  : 清除ICC收发通信表中的计时
** @The notes : 两帧交互正常计时结束
**===============================================================================================*/
void ICCClrTxRcvTabTimer(u8 id)
{
    /*通信收发表序号越界*/
    if(id >= gIccBaseDef[eICCRBase_TRTabNum])
    {
        return;
    }

	gIccTxRcvTable[id].timer.timeResend = 0;
	gIccTxRcvTable[id].timer.timeElasped = 0;
}
/*=================================================================================================
** @Static function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : static u8 ICCFindDevTabMIndexDim(u8 findIndex, u8 mindexNum, const t_ICCIndexTab *pindextable)
** @Input     : findIndex:查找的主索引号  mindexNum:主索引数目 *pindextable:所要查找的主索引表
** @Output    : 对应主索引所在位置
** @Function  : 查找主索引所在设备表中偏移位置(第几个主索引)
** @The notes : 二分查找法
**===============================================================================================*/
static u8 ICCFindDevTabMIndexDim(u8 findIndex, u8 mindexNum, const t_ICCIndexTab *pindextable)
{
    u8 low = 0;
    u8 high = mindexNum - 1;
    u8 mid = 0;
    u8 i = 0;

    for(i = 0; i < ((mindexNum / 2) + 1); i++)
    {
        if(low <= high)
        {
            mid = (u8)((low + high) / 2);
            if((pindextable + mid)->mIndex == findIndex)
            {
                return(mid);
            }
            else
            {
                if((pindextable + mid)->mIndex < findIndex)
                {
                    low = mid + 1;
                }
                else
                {
                    if((pindextable + mid)->mIndex > findIndex)
                    {
                        if(mid == 0)
                        {
                            return(0xff);
                        }

                        high = mid - 1;
                    }
                }
            }
        }
        else
        {
            return(0xff);
        }
    }

    return(0xff);
}


static int iccmsg_tx_test(int argc, char *argv[])
{
    if (argc == 0) {
        printf("iccmsg tx test para error\n");
        return CLI_FALSE;
    }
    if (!strcmp(argv[0], "tx"))
    {
        for(u32 j = 0; j < 20000; j++)
        {
        (void)ICCReqIdleTxRcvTab(eICCFUN_UPC, gGCellVoltInfo_1, ICC_DT_U16, 20, 0, GINDEX92_LEN, 0, 0xff, eICCCHA_CEL);
        (void)ICCReqIdleTxRcvTab(eICCFUN_UPC, gGChgBalanInfo_1, ICC_DT_U16, 20, 0, GINDEX90_LEN, 0, 0xff, eICCCHA_CEL);
        }
        return 0;
    }
    else
    {
        printf("the parameters cannot be recognized\n");
        return 0;
    }
}
CLI_CMD("iccmsg_tx_test",
        "iccmsg_tx_test"
        "\r\n\t iccmsg_tx_test tx",
        iccmsg_tx_test);