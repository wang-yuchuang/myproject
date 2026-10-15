/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IccMsgInterface.h
** @Founder      : ZQD808
** @Date         : 2026.02.05
** @Function     : CAN通信包用户接口处理
** @Instructions :
**===============================================================================================*/
#ifndef _ICCMSGINTERFACE_H_
#define _ICCMSGINTERFACE_H_
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"

/*=================================================================================================
** @Macro definition
**===============================================================================================*/
/*=======================================设备索引表声明宏定义=====================================*/
#define DeclareIndexTableEntry(acesstype, mindex, pdata, size, sIndexNum) \
{(u8)acesstype, (u8)mindex, (void*)pdata, (u8)size, (u16)sIndexNum}

/*=================================================================================================
** @Data type definition
**===============================================================================================*/
/*核间通信通道定义*/
typedef enum
{
    eICCCHA_CEL = 0,                 //CELL通道
    eICCCHA_CYC,                     //CYCLE通道
    eICCCHA_FAQ,                     //FAQ通道
    eICCCHA_UPD,                     //UPDATA通道
    eICCCHA_PCS,                     //PCS通道
    eICCCHA_PCF,                     //PSCFG通道

}e_ICCChannelDef;
/*核间通信功能码定义*/
typedef enum
{
    eICCFUN_VFT = 0,                 //对时帧
    eICCFUN_BRE,                    //心跳帧
    eICCFUN_RUP,                    //请求修改
    eICCFUN_AUP,                    //回复修改
    eICCFUN_CUP,                    //确认修改
    eICCFUN_REQ,                    //请求查询
    eICCFUN_ARQ,                    //回复查询
    eICCFUN_CRQ,                    //确认查询
    eICCFUN_UPM,                    //主动更新
    eICCFUN_ERR,                    //读写错误应答
    eICCFUN_UPS,                   //预留
    eICCFUN_UPC,                   //单体上传(单帧)
    eICCFUN_UPP,                   //参数上传
    eICCFUN_UPD,                   //升级信息
}e_ICCFunCode;

/*核间通信错误代码定义*/
typedef enum _ICC_ERRCODE_
{
    eICCERR_SUCCESSFUL = 0,             /*正常*/
    eICCERR_NOSUCH_ICC = 1,             /*无该核间通道*/
    eICCERR_NOSUCH_FUN = 2,             /*无该功能码*/
    eICCERR_NOSUCH_DEV = 3,             /*无该设备*/
    eICCERR_NOSUCH_MID = 4,             /*无该主索引*/
    eICCERR_SIDLEN_OVE = 5,             /*子索引超长*/
    eICCERR_NOSUCH_TAB = 6,             /*无该CAN收发处理通信表*/
    eICCERR_NOIDLE_TAB = 7,             /*无空闲CAN收发处理通信表*/
    eICCERR_ICCPW_DIS = 8,              /*该CAN口禁止写入*/
    eICCERR_RUPW_FAIL = 9,              /*请求修改写入失败*/
    eICCERR_REQW_FAIL = 10,             /*查询回复写入失败*/
    eICCERR_CYCW_FAIL = 11,             /*周期更新写入失败*/
    eICCERR_RCVMSG_ERR = 12,            /*接收报文异常*/
    eICCERR_MSGCRC_ERR = 13,            /*报文CRC校验码错误*/
    eICCERR_CYCUP_DIS = 14,             /*禁止周期发送*/
    eICCERR_KEEP_FUN = 15,              /*非法使用保留功能*/
    eICCERR_FAIL= 16                    /*分配内存异常*/
}e_CANErrCode;

/*核间通信错误原因定义*/
typedef enum _ICC_ERRREAS_
{
    eICCERRREAS_NORMAL = 0,             /*正常无原因*/
    eICCERRREAS_INDEXERR = 1,           /*读写索引非法(包括长度)*/
    eICCERRREAS_CRCERR = 2,             /*CRC校验错误*/
    eICCERRREAS_AUTHERR = 3,            /*写入权限非法*/
    eICCERRREAS_STATEERR = 4,           /*写入状态非法*/
    eICCERRREAS_RANGEERR = 5,           /*写入参数范围非法*/
    eICCERRREAS_PENDING = 6,            /*延迟回复:HOOK已自行发送回复,框架不再自动回复*/
}e_ICCErrReas;

typedef enum
{
    eICCRBase_DevNum = 0,               /*通信包设备表数目*/
	eICCRBase_IndexNum = 1,             /*主控主索引数目*/
	eICCRBase_TRTabNum = 2,             /*收发通信表数目*/
	eICCRBase_TxVerNum = 3,             /*主动发送校时帧信息配置组数*/
	eICCRBase_TxBreNum = 4,             /*主动上传心跳帧信息配置组数*/
	eICCRBase_TxCelNum = 5,             /*主动上传单体信息通道配置组数*/
	eICCRBase_TxCylNum = 6,             /*主动上传循环信息通道配置组数*/
    eICCRBase_TxFaqNum = 7,             /*询问回复问答信息通道配置组数*/
    eICCRBase_TxTabLen = 10
}e_ICCBaseInfo;

/*核间通信包计时结构体*/
typedef struct _ICC_TIMER_
{
    u16 timeResend;                     //当前剩余时间
    u16 timeElasped;                    //已消耗的时间
}t_ICCTimer;

/*核间通信包主索引表*/
typedef struct _ICCINDEX_TAB_
{
    u8 acceType;                        //读写属性 0：只读 ，0x01: 读写 0x02：只写
    u8 mIndex;                          //主索引
    void *pObject;                      //数据地址
    u8 dataType;                        //数据类型
    u16 sIndexNum;                       //子索引数目
}t_ICCIndexTab;

/*核间通信协议规约定义*/
typedef struct _ICC_Fram_
{
    u8 funCode; //功能码
    u8 mIndex;  //主索引
    u8 dAddr;   //目的地址
    u8 sAddr;   //源地址
    u8 *pData;   //数据域指针地址
    u8 *pId;    //ID域指针地址
    u16 framLen;    //DATA域和ID域总长度
}t_ICCFram;

/*核间通信结构体设备表*/
typedef struct _ICCDEV_TAB_
{
    u8 devAdrr;                         //设备地址
    u8 mIndexNum;                       //主索引数目
    const t_ICCIndexTab *pIndexTable;   //主索引表指针
}t_ICCDevTab;

/*核间通信收发交互结构体*/
typedef struct _ICCTXRCV_TAB_
{
    u8 ready;                           //就绪标志 0初始化，1发送就绪 2等待确认
    e_ICCFunCode funcode;               //功能码
    void *pdata;                        //数据首地址
    u8 dataType;                        //数据类型
    u8 mIndex;                          //主索引
    u16 sIndex;                         //当前起始帧子索引
    u16 sIndexNum;                      //子索引数目
    u8 daddr;                           //目的地址
    u8 iccnum;                          //CNA通道
    u8 framMark;                        //帧标识
    t_ICCTimer timer;                   //定时器
}t_ICCTxRcvTab;


/*核间通信包数据定时上传发送表定义*/
typedef struct _ICCTX_TAB_
{
    void *pdata;                        //发送数据地址
    u8 dataType;                        //数据类型
    u8 funcode;                         //功能码
    u8 index;                           //所在配置表索引
    u32 nextTime;                       //下次发送时间
}t_ICCTxTab;

/*核间通信收发状态定义*/
typedef enum _ICC_TABSTATE_
{
    eICCTAB_LEISUREL = 0,               /*空闲监听*/
    eICCTAB_READYOK = 1,                /*发送就绪*/
    eICCTAB_WAIT = 2,                   /*忙等待*/
}e_ICCTRTabState;

/*ICC通信包主索引映射转换定义*/
typedef struct _ICC_MAPTRANS_
{
    u8 realMIndex;                       /*映射主索引转换成真实主索引值*/
    u8 realSIndex;                       /*映射主索引转换成真实子索引值*/
    u8 realSIndexNum;                    /*映射主索引转换成真实子索引数目*/
}t_ICCMapTrans;

/*CAN通信包主索引映射参数定义*/
typedef struct _CAN_INDEXMAP_
{
    u8 map90Index;                      /*主索引90映射起始主索引(单体充电均衡状态)*/
    u8 map91Index;                      /*主索引91映射起始主索引(单体放电均衡状态)*/
    u8 map92Index;                      /*主索引92映射起始主索引(单体电压信息1)*/
    u8 map93Index;                      /*主索引93映射起始主索引(单体电压信息2)*/
    u8 map94Index;                      /*主索引94映射起始主索引(单体温度信息1)*/
    u8 map95Index;                      /*主索引94映射起始主索引(单体温度信息2)*/

    u8 map100Index;                     /*主索引100映射起始主索引(系统通用参数)*/
    u8 map101Index;                     /*主索引101映射起始主索引(系统硬件参数)*/
    u8 map102Index;                     /*主索引102映射起始主索引(设备通用参数)*/
    u8 map103Index;                     /*主索引103映射起始主索引(设备高级参数)*/
    u8 map104Index;                     /*主索引104映射起始主索引(设备硬件参数)*/

    u8 index90Type;                     /*主索引90数据类型(单体充电均衡状态)*/
    u8 index91Type;                     /*主索引91数据类型(单体放电均衡状态)*/
    u8 index92Type;                     /*主索引92数据类型(单体电压信息1)*/
    u8 index93Type;                     /*主索引93数据类型(单体电压信息2)*/
    u8 index94Type;                     /*主索引94数据类型(单体温度信息1)*/
    u8 index95Type;                     /*主索引95数据类型(单体温度信息2)*/

    u8 index100Type;                    /*主索引100数据类型(系统通用参数)*/
    u8 index101Type;                    /*主索引101数据类型(系统硬件参数)*/
    u8 index102Type;                    /*主索引102数据类型(设备通用参数)*/
    u8 index103Type;                    /*主索引103数据类型(设备高级参数)*/
    u8 index104Type;                    /*主索引104数据类型(设备硬件参数)*/
}t_CANLibIndexMap;

/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/
/*=================================================================================================
** @Name      : void ICCMSGLibAllInit_API(void)
** @Input     : void
** @Output    : void
** @Function  : 核间通信包所有初始化
** @The notes : 用户必须初始化调用
**===============================================================================================*/
void ICCMSGLibAllInit_API(void);


/*=================================================================================================
** @Name      : void BmsIccPostHandshakeStart(void)
** @Input     : void
** @Output    : void
** @Function  : 方案A——握手完成后异步启动核间通信初始化（独立线程等待 A 核就绪再 AllInit）
** @The notes : 在 main() 的 sys_monitor_init() 之后调用；仅起线程，不阻塞调用者
**===============================================================================================*/
void BmsIccPostHandshakeStart(void);


/*=================================================================================================
** @Name      : void ICCSendMsgUser_Hook (u8 *pdata, u16 DLC, e_ICCChannelDef iccNum)
** @Input     : *pdata:id域和数据域 DLC:数据长度 iccNum:ICC通道号
** @Output    : void
** @Function  : 核间通信包发送报文处理
** @The notes : 用户必须补充完善
**===============================================================================================*/
void ICCSendMsgUser_Hook (u8 *pdata, u16 DLC, e_ICCChannelDef iccNum);

/*=================================================================================================
** @Name      : void ICCLibTxRcvHandleTask_API(void)
** @Input     : void
** @Output    : void
** @Function  : 核间通信报文收发处理任务(收发通信表处理)
** @The notes : 用户必须1ms周期调用
**===============================================================================================*/
void ICCLibTxRcvHandleTask_API(void);

/*=================================================================================================
** @Name      : u8 ICCRcvRupUserDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : saddr:源地址  mIndex:主索引号 sIndex:起始子索引号 sIndexNum:子索引数目 iccNum:ICC通道号
** @Output    : 执行结果:0:正常 1:索引非法 2:CRC非法 3:权限非法 4:状态非法 5:参数非法
** @Function  : 接收到请求修改报文后用户处理
** @The notes : 用户需要时补充完善,对相应索引值进行处理
**===============================================================================================*/
u8 ICCRcvRupUserDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum);

/*=================================================================================================
** @Name      : void ICCRcvReqUserDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : saddr:源地址  mIndex:主索引号 sIndex:起始子索引号 sIndexNum:子索引数目 iccNum:ICC通道号
** @Output    : void
** @Function  : 接收到请求查询报文后(查询回复前)用户处理
** @The notes : 用户需要时补充完善,更新相应索引值(不自动跟新的值)
**===============================================================================================*/
void ICCRcvReqUserDeal_Hook(u8 saddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum);

/*=================================================================================================
** @Name      : void ICCRcvVerUserDeal_Hook(u8 saddr, u8 iccNum)
** @Input     : saddr:源地址  iccNum:ICC通道号
** @Output    : void
** @Function  : 接收到校时报文后用户处理
** @The notes : 用户需要时补充完善
**===============================================================================================*/
void ICCRcvVerUserDeal_Hook(u8 saddr, u8 canNum);

/*=================================================================================================
** @Name      : void ICCRcvBreUserDeal_Hook(u8 saddr, u8 iccNum)
** @Input     : saddr:源地址  iccNum:ICC通道号
** @Output    : void
** @Function  : 接收到心跳报文后用户处理
** @The notes : 刷新心跳时间戳, 供发送门控判定
**===============================================================================================*/
void ICCRcvBreUserDeal_Hook(u8 saddr, u8 iccNum);

/*=================================================================================================
** @Name      : t_ICCMapTrans ICCIndexMapTransUser_Hook(e_ICCFunCode funCode, u8 mIndex, u8 DLC)
** @Input     : funCode:功能码  mIndex:映射主索引号 DLC:报文长度
** @Output    : void
** @Function  : ICC通信包将映射报文信息转换成真实信息
** @The notes : 用户必须补充完善,单体信息和参数信息报文的映射
**===============================================================================================*/
t_ICCMapTrans ICCIndexMapTransUser_Hook(e_ICCFunCode funCode, u8 mIndex, u8 DLC);

/*=================================================================================================
** @Name      : void ICCRcvCycUpUserDeal_Hook (t_ICCFram *iccFram, u8 iccNum)
** @Input     : iccFram:ICC通信帧 iccNum:ICC通道号
** @Output    : void
** @Function  : 接收到周期更新报文后用户处理
** @The notes : 用户需要时补充完善,处理接收周期发送的长帧、单帧、单体和参数信息
**===============================================================================================*/
void ICCRcvCycUpUserDeal_Hook (t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : u8 SendIccGroupHisErrInfoMsg(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : TRUE成功 FALSE失败
** @Function  : 用户自行发送历史故障信息报文
** @The notes : 用户需要发送时调用(信息变化时)
**===============================================================================================*/
u8 SendIccGroupHisErrInfoMsg(u8 clunum);

/*=================================================================================================
** @Name      : u8 SendIccGroupHisEventInfoMsg(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : TRUE成功 FALSE失败
** @Function  : 用户自行发送历史事件信息报文
** @The notes : 用户需要发送时调用(信息变化时)
**===============================================================================================*/
u8 SendIccGroupHisEventInfoMsg(u8 clunum);

/*=================================================================================================
** @Name      : u8 ICCLibRcvMsgHandle_API(u8 *pdata, u8 DLC, u8 iccNum)
** @Input     : *pdata:核间通信ID域和数据域  DLC:ID域和数据域长度 iccNum:ICC通道号
** @Output    : 处理结果(0:正常,其它:错误码)
** @Function  : ICC通信报文接收处理任务
** @The notes : 用户放在ICC接收处理HOOK接口调用
**===============================================================================================*/
extern u8 ICCLibRcvMsgHandle_API(u8 *pdata, u8 DLC, u8 iccNum);

/*=================================================================================================
** @Name      : u8 ICCBuildSendAupMsg_API(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:接收到的ICC通信帧 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送修改回复报文帧
** @The notes : 用户自行发送修改回复报文帧
**===============================================================================================*/
extern u8 ICCBuildSendAupMsg_API(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : u8 ICCBuildSendCupMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum)
** @Input     : destAddr:目标设备地址 mIndex:主索引号 sIndex:起始子索引号 sIndexNum:子索引数目 conFlag:是否需要确认 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送修改确认报文帧
** @The notes : 用户自行发送修改确认报文帧
**===============================================================================================*/
extern u8 ICCBuildSendCupMsg_API(u8 destAddr, u8 mIndex, u8 sIndex, u8 sIndexNum, u8 iccNum);

/*=================================================================================================
** @Name      : u8 ICCBuildSendArqMsg_API(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:接收到的ICC通信帧 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送查询回复报文帧
** @The notes : 用户自行发送查询回复报文帧
**===============================================================================================*/
extern u8 ICCBuildSendArqMsg_API(t_ICCFram *iccFram, u8 iccNum);

/*=================================================================================================
** @Name      : u8 ICCBuildSendCrqMsg_API(t_ICCFram *iccFram, u8 iccNum)
** @Input     : *iccFram:接收到的ICC通信帧 iccNum:ICC通道号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 组建和发送查询确认报文帧
** @The notes : 用户自行发送查询确认报文帧
**===============================================================================================*/
extern u8 ICCBuildSendCrqMsg_API(t_ICCFram *iccFram, u8 iccNum);


#endif