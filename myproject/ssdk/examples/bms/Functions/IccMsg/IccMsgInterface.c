/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IccMsgInterface.c
** @Founder      : ZQD808
** @Date         : 2026.02.05
** @Function     : CAN通信包用户接口处理
**===============================================================================================*/
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "IccMsgInterface.h"
#include "IccMsgCreate.h"
#include "IccMsgTabOper.h"
#include "IccMsgTxCycle.h"
#include "IccMsgHandle.h"
#include "iccmsg.h"
#include "iccmsg_dpr.h"
#include "sysmgr.h"
#include <CLI.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "BSPTypeDef.h"
#include <malloc.h>
#include "IccMsgUser.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "PublicPara.h"
#include "iccmsg.h"
#include "GroupDebug.h"        /*调试模式处理接口*/
#include "IMEU3Interface.h"     /*参数配置下发接口*/
#include "BSPUserAPI.h"         /*BSPGetOSMillSecTimeAPI 毫秒计时*/
#include "IccMsgVerSrl.h"       /*81/82/83/84版本号序列号查询修改*/
#include "SlaveMsg.h"           /*BSU内阻扫描控制发送接口*/
#include "TacUser.h"
/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
//t_CANLibIndexMap* gCanIndexMap = NULL;                                /*核间通信包主索引映射信息*/
static osMutexId_t icc_send_mutex = NULL;        //核间发送互斥锁



extern u16 *gDevInfo_2;
extern u16 *gSysDbgCmd_71;              /*系统调试命令数组 主索引71*/
extern u8  *gImeu3CfgSet_76;            /*A核配置缓存(ICC mIndex=76)*/
extern u8   gEU3CfgReadbackFlag;         /*下发后回读标志*/
extern volatile u8 gEU3CfgReplyFlag;     /*EU3配置回复标志(IMEU3RcvHandle.c定义)*/
extern u16 gGBmuGenPara_102[];  	 /*主控通用参数值*/
extern u8 *gIccBaseDef;

extern u16 gGBmuGenPara_102[];      /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];   /*主控高级参数值*/
extern u8 gGHardPara_104[];         /*主控硬件参数值*/

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
extern u16 *gAlmFault1_57;        /*组1告警故障信息*/



extern iccmsg_t *iccmsg_cell;   //单体信息通道句柄
extern iccmsg_t *iccmsg_cycle;  //周期信息通道句柄
extern iccmsg_t *iccmsg_faq;    //问答信息通道句柄
extern iccmsg_t *iccmsg_update; //升级信息通道句柄
//extern iccmsg_t *iccmsg_pcs;    //模块周期信息通道句柄
//extern iccmsg_t *iccmsg_pcscfg; //模块配置信息通道句柄

extern u8 *gIccMsgCellDataDef;          /*单体信息通道数据配置*/
extern t_ICCTxRcvTab* gIccTxRcvTable;
/*=================================================================================================
** @Static function declaration
**===============================================================================================*/
 






/*=================================================================================================
** @Interface function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : void ICCMSGLibAllInit_API(void)
** @Input     : void
** @Output    : void
** @Function  : 核间通信包所有初始化
** @The notes : 用户必须初始化调用
**===============================================================================================*/
void ICCMSGLibAllInit_API(void)
{
    u8 i = 0;
    icc_send_mutex = osMutexNew(NULL);
    if(icc_send_mutex == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: icc_send_mutex create fail!!!\r\n", __func__);
        osMessageQueueDelete(icc_send_mutex);
        icc_send_mutex = NULL;
    }
    /*通信包配置参数初始化*/
    gDevInfo_2[eICCDev2_Addr] = gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    //gDevInfo_2[eICCDev2_HW] = (BMU_HW_VER & 0xff);


    /*通信包设备表初始化*/
    ICCDeviceTableInit();

    /*通信包收发通信表初始化*/
    if(gIccTxRcvTable != NULL)
    {
    	for(i = 0; i < gIccBaseDef[eICCRBase_TRTabNum]; i++)
    	{
    		/*清空所有收发通信表*/
    		ICCClrTxRcvTabInfo(i);
    	}
    }

    /*通信包周期发送表初始化*/
    ICCMSGTxCellCycleTabInit();
}

/*=================================================================================================
** @Name      : static void BmsIccInitTask(void *arg)
** @Input     : void *arg
** @Output    : void
** @Function  : 等待 A 核 BOOT_READY 握手完成后执行核间通信初始化
** @The notes : 独立线程执行(栈8K), 不占用 sysmon_init 线程; 完成后 osThreadExit
**===============================================================================================*/
#define BMS_ICC_WAIT_SLICE_SEC   1u

static void BmsIccInitTask(void *arg)
{
    (void)arg;
    bool warned = false;
    bool ept_warned = false;

    /*必须等 A 核握手完成才放行: 超时不初始化, 持续等待
     *(未启用 sys_monitor 时弱定义立即返回 0, 即无握手概念直接通过)*/
    while(sysmgr_wait_ap_ready(osKernelGetTickFreq() * BMS_ICC_WAIT_SLICE_SEC) != 0)
    {
        if(!warned)
        {
            warned = true;
            ssdk_printf(SSDK_WARNING, "%s: AP not ready, waiting handshake...\r\n", __func__);
        }
    }
    printf("[bms] AP handshake done, init ICC msg\r\n");

    /*必须等 sys_monitor 占好 sysmgr@500/ota@502 再分配 BMS 通道, 否则
     *并发取号会让 cell 抢到 500/502(A 核按 504 找 cell 将失配)。
     *未启用 sys_monitor 时弱定义立即返回 0, 即直接通过。*/
    while(sysmgr_wait_ept_ready(osKernelGetTickFreq() * BMS_ICC_WAIT_SLICE_SEC) != 0)
    {
        if(!ept_warned)
        {
            ept_warned = true;
            ssdk_printf(SSDK_WARNING, "%s: sysmgr ept not ready, waiting...\r\n", __func__);
        }
    }
    printf("[bms] sysmgr ept ready\r\n");

    /*建核间通信通道 + 通信表(内部含 iccmsg_add)*/
    ICCMSGLibAllInit_API();
    printf("[bms] ICC msg init done\r\n");

    osThreadExit();
}

/*=================================================================================================
** @Name      : void BmsIccPostHandshakeStart(void)
** @Input     : void
** @Output    : void
** @Function  : 方案A 入口——起线程等待握手完成后 AllInit, 仅触发一次
** @The notes : 在 main() 的 sys_monitor_init() 之后调用
**===============================================================================================*/
void BmsIccPostHandshakeStart(void)
{
    static bool started = false;

    if(started)
    {
        return;
    }
    started = true;

    osThreadAttr_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.name       = "BmsIccInit";
    /*优先级须低于 sysmon_init(25): 事件同时唤醒后让 sysmon_init 先 add,
     *保证 sysmgr@500/ota@502 先占号, BMS 通道(cell/cycle/faq/update)从 504 起*/
    attr.priority   = 24;
    attr.stack_size = 8192;

    if(osThreadNew(BmsIccInitTask, NULL, &attr) == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: create BmsIccInit thread fail\r\n", __func__);
    }
}

/*=================================================================================================
** @Name      : void ICCSendMsgUser_Hook (u8 *pdata, u16 DLC, e_ICCChannelDef iccNum)
** @Input     : *pdata:id域和数据域 DLC:数据长度 iccNum:ICC通道号
** @Output    : void
** @Function  : 核间通信包发送报文处理
** @The notes : 用户必须补充完善
**===============================================================================================*/
/* =====================================================================
 * 心跳感知发送门控（2026-09-21）
 * 背景：A 核消费端不在时，业务帧灌满 tx_queue，iccmsg 拥堵连坐 sysmgr
 * 心跳接收 -> sys_monitor 30s AP heartbeat timeout -> kick AP1 复位。
 * 机制：仅当 5s 内收到过 eICCFUN_BRE 心跳帧时才允许业务发送；超时未收
 * 到心跳则暂停发送。心跳到达由 ICCRcvBreUserDeal_Hook() 刷新时间戳。
 * ===================================================================== */
#define TXGATE_TIMEOUT_MS   5000
static bool s_bre_seen        = false;
static u32  s_bre_last_tick   = 0;
static bool s_txgate_open     = true;   /* 仅用于状态变化时打日志 */

static bool bms_txgate_allow(void)
{
    u32  now;
    bool allow;

    if (!s_bre_seen) {
        allow = false;                          /* 从未收到心跳: 禁止发送 */
    } else {
        now   = osKernelGetTickCount();
        allow = ((now - s_bre_last_tick) < TXGATE_TIMEOUT_MS);
    }

    if (allow != s_txgate_open) {
        s_txgate_open = allow;
        if (allow) {
            printf("[txgate] heartbeat ok, resume tx\r\n");
        } else {
            printf("[txgate] no heartbeat within %ums, pause tx\r\n",
                   (unsigned)TXGATE_TIMEOUT_MS);
        }
    }
    return allow;
}

void ICCSendMsgUser_Hook (u8 *pdata, u16 DLC, e_ICCChannelDef iccNum)
{
	if(DLC > 1024)
	{
		return;
	}
    if(osMutexAcquire(icc_send_mutex, 100) != osOK)
    {
        return;
    }
    if (!bms_txgate_allow())             /* B5: 门控移入互斥区, 多任务调用状态竞态安全 */
    {
        osMutexRelease(icc_send_mutex);
        return;
    }
    /*发送核间通信报文*/
    switch(iccNum)
    {
        case eICCCHA_CEL:
            iccmsg_send(iccmsg_cell, 0, 0, pdata, DLC, ICCMSG_ACK_NO_NEED);
            //printf("*******************iccmsgcell_data_send*******************\r\n");
            break;
        case eICCCHA_CYC:
            iccmsg_send(iccmsg_cycle, 0, 0, pdata, DLC, ICCMSG_ACK_NO_NEED);
            //printf("*******************iccmsgcycle_data_send********:%d***********\r\n",DLC);
            // if((void*)gAlmFault1_57 == (void*)pdata)
            // {
            //     for(int i  = 0; i < DLC; i++)
            //     {
            //         printf("data[%d]: %d\r\n",i,pdata[i]);
            //     }
            // }

            break;
        case eICCCHA_FAQ:
            iccmsg_send(iccmsg_faq, 0, 0, pdata, DLC, ICCMSG_ACK_NO_NEED);
            //for(int i  = 0; i < DLC; i++)
            //{
            //    printf("data[%d]: %d\r\n",i,pdata[i]); //调试刷屏,已屏蔽 2026-08-31
            //}
            break;

        // case eICCCHA_PCS:
        //     iccmsg_send(iccmsg_pcs, 0, 0, pdata, DLC, ICCMSG_ACK_NO_NEED);
        //     break;
        // case eICCCHA_PCF:
        //     iccmsg_send(iccmsg_pcscfg, 0, 0, pdata, DLC, ICCMSG_ACK_NO_NEED);
        //     break;            
        default:
            break;
    }
    osMutexRelease(icc_send_mutex);
}

/*=================================================================================================
** @Name      : u8 ICCRcvRupUserDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : saddr:源地址  mIndex:主索引号 sIndex:起始子索引号 sIndexNum:子索引数目 iccNum:ICC通道号
** @Output    : 执行结果:0:正常 1:索引非法 2:CRC非法 3:权限非法 4:状态非法 5:参数非法
** @Function  : 接收到请求修改报文后用户处理
** @The notes : 用户需要时补充完善,对相应索引值进行处理
**===============================================================================================*/
u8 ICCRcvRupUserDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
{
    /*非上层设备控制不执行*/
	if(saddr <= gDevInfo_2[eICCDev2_Addr])
	{
		/*权限非法*/
		return(eICCERRREAS_AUTHERR);
	}

    /*执行请求修改命令*/
    if(2 == mIndex)                     /*写设备序列号*/
    {
    	/*执行写设备序列号处理*/
    	//return(RcvIccDeviceSerialCfgDeal(sIndex, sIndexNum, iccNum));
    }
    /*执行请求修改命令*/
    else if(22 == mIndex)               /*写参数批量配置值*/
    {
    	/*执行写参数批量配置写数据处理*/
    	//return(RcvIccDnloadParaWRDeal(sIndex, sIndexNum, iccNum));
    }
    else if(69 == mIndex)               /*写参数批量配置值*/
    {
        RcvGroupReqLeakCheckCmdAPI(0);
        RcvGroupReqLeakCheckCmdAPI(1);  //绝缘处理

    }
    /*系统调试命令 两段式确认: A核先写sub0-8缓存,再写sub9=1/2触发*/
    else if(71 == mIndex)
    {
    	u8 affirmEnd = sIndex + sIndexNum - 1;                     /*本次写入的末尾子索引*/
    	if(affirmEnd == eDbgCmd71_Affirm)                          /*末尾正好是确认命令sub9,确保本次写入了sub9而非读旧值*/
    	{
    		u8 affirm = gSysDbgCmd_71[eDbgCmd71_Affirm];            /*确认命令 1=操作 2=写EEPROM*/
    		if(1 == affirm || 2 == affirm)
    		{
    			TacSetGroupDebugAffirmCmdAPI(affirm);
    		}
    	}
    }
    /*A核下发EU3参数配置, 转发到EU3(CANFD mIndex=110)*/
    else if(76 == mIndex)
    {
        u16 values[13];                 /* 最多13项: sub0配置标志 + sub1-12配置值 */
        u8 i;
        if((0 == sIndex) && (12 == sIndexNum))       /* 一键全量: A核一次写满12项 */
        {
            values[0] = 23132;          /* sub0配置标志(23132=0x5A5C, 提交生效) */
            for(i = 0; i < 12; i++)
            {
                values[i + 1] = (u16)gImeu3CfgSet_76[i];
            }
            IMEU3_SendModifyPara(IMEU3_DEV_ADDR, 0, 13, values);
        }
        else                             /* 单项/部分修改 */
        {
            for(i = 0; i < sIndexNum; i++)
            {
                values[i] = (u16)gImeu3CfgSet_76[sIndex + i];
            }
            IMEU3_SendModifyPara(IMEU3_DEV_ADDR, (u8)(sIndex + 1), sIndexNum, values);   /* ICC76子索引+1=CANFD110子索引 */
        }
        gEU3CfgReadbackFlag = 1;         /* 下发后触发一次CANFD 102回读查询 */
    }
    /*下级设备版本号/序列号查询修改(81/82/83/84)*/
    else if((81 == mIndex) || (82 == mIndex) || (83 == mIndex) || (84 == mIndex))
    {
        return(ICCVerSrlDeal_Hook(saddr, mIndex, sIndex, sIndexNum, iccNum));
    }
    /*A核下发内阻查询指令(85): 保存频率索引并广播触发BSU内阻扫描*/
    else if(85 == mIndex)
    {
        u16 freqIdx;
        u16 mode;

        if(gIccInsuQuery_85 == NULL)
        {
            return(eICCERRREAS_NORMAL);
        }
        freqIdx = gIccInsuQuery_85[5];       /*子索引5=内阻检测频率索引[0,228]*/
        mode = gIccInsuQuery_85[6];          /*子索引6=检测方式(0全部 1奇数 2偶数)*/
        gIccInsuFreqIdx = freqIdx;           /*存频点, BSU测完上传后填入ICC28-35头部子索引2*/
        BSU_SendInsuScanCtrl(0xff, freqIdx, mode);   /*广播触发16个BSU内阻扫描*/
        return(eICCERRREAS_NORMAL);
    }
    return(eICCERRREAS_NORMAL);
}

/*=================================================================================================
** @Name      : void ICCRcvReqUserDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : saddr:源地址  mIndex:主索引号 sIndex:起始子索引号 sIndexNum:子索引数目 iccNum:ICC通道号
** @Output    : void
** @Function  : 接收到请求查询报文后(查询回复前)用户处理
** @The notes : 用户需要时补充完善,更新相应索引值(不自动跟新的值)
**===============================================================================================*/
void ICCRcvReqUserDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
{
    /*查询系统时钟当前时间*/
	if(0 == mIndex)
	{
	    /*更新系统时钟显示*/
		//SysClockDisplayAPI();//未实现
	}
    /*查询主控设备当前条码序列号*/
	else if(2 == mIndex)
	{
		if(0 == sIndex)
		{
			/*读设备序列号到显示区域*/
			//SysDevSerialDispayAPI();//未实现
		}
	}
    /*A核查询75时, 同步回读EU3参数(CANFD 102)后再应答, 避免返回旧值*/
    else if(75 == mIndex)
    {
        u32 readbackStart;
        u8  i;
        gEU3CfgReplyFlag = 0;
        IMEU3_SendCfgParaQuery(IMEU3_DEV_ADDR);
        readbackStart = BSPGetOSMillSecTimeAPI();
        while((BSPGetOSMillSecTimeAPI() - readbackStart) < 300)
        {
            if(gEU3CfgReplyFlag)
            {
                break;
            }
            osDelay(1);
        }
        if(gEU3CfgReplyFlag && (gImeu3CfgRO_75 != NULL) && (gImeu3CfgPara != NULL))
        {
            for(i = 0; i < GINDEX75_LEN; i++)
            {
                gImeu3CfgRO_75[i] = (u8)gImeu3CfgPara[i + 1];
            }
        }
    }
}

/*=================================================================================================
** @Name      : t_ICCMapTrans ICCIndexMapTransUser_Hook(e_ICCFunCode funCode, u8 mIndex, u8 DLC)
** @Input     : funCode:功能码  mIndex:映射主索引号 DLC:报文长度
** @Output    : void
** @Function  : ICC通信包将映射报文信息转换成真实信息
** @The notes : 用户必须补充完善,单体信息和参数信息报文的映射
**===============================================================================================*/
t_ICCMapTrans ICCIndexMapTransUser_Hook(e_ICCFunCode funCode, u8 mIndex, u8 DLC)
{
    /*根据报文映射信息[功能码/主索引/报文长度]转换成真实索引信息*/
	t_ICCMapTrans iccMapInfo = {0};

    //周期主动更新单体报文,需要根据协议补充相关信息
    if(eICCFUN_UPC == funCode)
    {
    	// /*单体充电均衡状态信息*/
    	// if(mIndex < gIccIndexMap.map91Index)
    	// {
    	// 	iccMapInfo.realMIndex = 90;
    	// 	iccMapInfo.realSIndex = mIndex * 4;
    	// 	iccMapInfo.realSIndexNum = DLC / gIccIndexMap.index90Type;
    	// }
    	// /*单体放电均衡状态信息*/
    	// else if(mIndex < gIccIndexMap.map92Index)
    	// {
    	// 	iccMapInfo.realMIndex = 91;
    	// 	iccMapInfo.realSIndex = (mIndex - gIccIndexMap.map91Index) * 4;
    	// 	iccMapInfo.realSIndexNum = DLC / gIccIndexMap.index91Type;
    	// }

    }
    //周期主动更新参数报文
    else if(eICCFUN_UPP == funCode)
    {
    	// /*系统通用参数*/
    	// if(mIndex < gIccIndexMap.map101Index)
    	// {
    	// 	iccMapInfo.realMIndex = 100;
    	// 	iccMapInfo.realSIndex = mIndex * 4;
    	// 	iccMapInfo.realSIndexNum = DLC / gIccIndexMap.index100Type;
    	// }
    	// /*系统硬件参数*/
    	// else if(mIndex < gIccIndexMap.map102Index)
    	// {
    	// 	iccMapInfo.realMIndex = 101;
    	// 	iccMapInfo.realSIndex = (mIndex - gIccIndexMap.map101Index) * 4;
    	// 	iccMapInfo.realSIndexNum = DLC / gIccIndexMap.index101Type;
    	// }
    }
    //非周期主动更新帧报文
    else
    {
		iccMapInfo.realMIndex = 0xff;
		iccMapInfo.realSIndex = 0xff;
		iccMapInfo.realSIndexNum = 0xff;
    }

    return(iccMapInfo);
}

/*=================================================================================================
** @Name      : void ICCRcvCycUpUserDeal_Hook (t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : void
** @Function  : 接收到周期更新报文后用户处理
** @The notes : 用户需要时补充完善,处理接收周期发送的长帧、单帧、单体和参数信息
**===============================================================================================*/
void ICCRcvCycUpUserDeal_Hook (t_ICCFram *iccFram, u8 iccNum)
{

    

}

/*=================================================================================================
** @Name      : void ICCRcvVerUserDeal_Hook(u8 saddr, u8 iccNum)
** @Input     : saddr:源地址  iccNum:ICC通道号
** @Output    : void
** @Function  : 接收到校时报文后用户处理
** @The notes : 用户需要时补充完善
**===============================================================================================*/
void ICCRcvVerUserDeal_Hook(u8 saddr, u8 canNum)
{
    /*将时间信息更新到系统时钟*/
	SysClockExtVerifyAPI();
}

/*=================================================================================================
** @Name      : void ICCRcvBreUserDeal_Hook(u8 saddr, u8 iccNum)
** @Input     : saddr:源地址  iccNum:ICC通道号
** @Output    : void
** @Function  : 接收到心跳报文后用户处理
** @The notes : 刷新心跳时间戳, 供发送门控 bms_txgate_allow() 判定
**===============================================================================================*/
void ICCRcvBreUserDeal_Hook(u8 saddr, u8 iccNum)
{
    s_bre_last_tick = osKernelGetTickCount();
    s_bre_seen      = true;
}







/*=================================================================================================
** @Name      : u8 SendIccGroupHisErrInfoMsg(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : TRUE成功 FALSE失败
** @Function  : 用户自行发送历史故障信息报文
** @The notes : 用户需要发送时调用(信息变化时)
**===============================================================================================*/
u8 SendIccGroupHisErrInfoMsg(u8 clunum)
{
    if(clunum == 0)
    {
        return(ICCBuildSendUpmMsg_API(0x5a, 10, 0, GINDEX10_LEN, eICCCHA_FAQ));
    }else if(clunum == 1)
    {
        return(ICCBuildSendUpmMsg_API(0x5a, 11, 0, GINDEX11_LEN, eICCCHA_FAQ));
    }else
    {
        return(FALSE);
    }

}

/*=================================================================================================
** @Name      : u8 SendIccGroupHisEventInfoMsg(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : TRUE成功 FALSE失败
** @Function  : 用户自行发送历史事件信息报文
** @The notes : 用户需要发送时调用(信息变化时)
**===============================================================================================*/
u8 SendIccGroupHisEventInfoMsg(u8 clunum)
{
    if(clunum == 0)
    {
        return(ICCBuildSendUpmMsg_API(0x5a, 12, 0, GINDEX12_LEN, eICCCHA_FAQ));
    }else if(clunum == 1)
    {
        return(ICCBuildSendUpmMsg_API(0x5a, 13, 0, GINDEX13_LEN, eICCCHA_FAQ));
    }else
    {
        return(FALSE);
    }

}






/*=================================================================================================
** @Static function definition
**===============================================================================================*/
static int iccmsg_table_test(int argc, char *argv[])
{
    if (argc == 0) {
        printf("iccmsg table create test para error\n");
        return CLI_FALSE;
    }
    if (!strcmp(argv[0], "init"))
    {
        ICCMSGLibAllInit_API();
        return 0;
    }else
    {
        printf("the parameters cannot be recognized\n");
        return 0;
    }
}
CLI_CMD("iccmsg_table_test",
        "iccmsg_table_test"
        "\r\n\t iccmsg_table_test init",
        iccmsg_table_test);

/* 模拟一帧接收路径上的 eICCFUN_BRE 心跳帧, 用于验证发送门控 bms_txgate_allow()。
 * 帧头 8 字节: [0..1]=framLen(LE) [4]=funCode [5]=mIndex(心跳固定1) [6]=dAddr(0xff广播绕自身校验) [7]=sAddr [8..]=data */
static int iccmsg_bre_test(int argc, char *argv[])
{
    u32 age;

    if (argc > 0 && !strcmp(argv[0], "show"))
    {
        age = (u32)(osKernelGetTickCount() - s_bre_last_tick);
        printf("[bre_test] seen=%d age=%ums allow=%d\r\n",
               (int)s_bre_seen, (unsigned)age,
               (s_bre_seen && (age < TXGATE_TIMEOUT_MS)) ? 1 : 0);
        return 0;
    }

    {
        u8 frame[8];
        memset(frame, 0, sizeof(frame));
        frame[0] = (u8)sizeof(frame);
        frame[4] = (u8)eICCFUN_BRE;
        frame[5] = 1;
        frame[6] = 0xff;
        frame[7] = 0x01;
        ICCLibRcvMsgHandle_API(frame, sizeof(frame), eICCCHA_FAQ);
    }

    age = (u32)(osKernelGetTickCount() - s_bre_last_tick);
    printf("[bre_test] injected BRE; seen=%d age=%ums allow=%d\r\n",
           (int)s_bre_seen, (unsigned)age,
           (s_bre_seen && (age < TXGATE_TIMEOUT_MS)) ? 1 : 0);
    return 0;
}
CLI_CMD("iccmsg_bre_test",
        "iccmsg_bre_test [show]"
        "\r\n\t iccmsg_bre_test       inject one eICCFUN_BRE heartbeat frame"
        "\r\n\t iccmsg_bre_test show  print current txgate state",
        iccmsg_bre_test);


