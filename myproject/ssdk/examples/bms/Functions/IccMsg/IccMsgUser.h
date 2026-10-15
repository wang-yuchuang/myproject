/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IccMsgUser.h
** @Founder      : WYC834
** @Date         : 2026.03.10
** @Function     : 核间通信包用户接口
** @Instructions :
**===============================================================================================*/
#ifndef _ICCMSGUSER_H
#define _ICCMSGUSER_H
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"

/*=================================================================================================
** @Macro definition
**===============================================================================================*/
/*=======================================主索引映射配置===========================================*/
#define CANMAP90_INDEX              0                       /*CAN通信主索引90映射起始主索引(单体充电均衡状态)*/
#define CANMAP91_INDEX              10                      /*CAN通信主索引91映射起始主索引(单体放电均衡状态)*/
#define CANMAP92_INDEX              50                      /*CAN通信主索引92映射起始主索引(单体电压信息1)*/
#define CANMAP93_INDEX              110                     /*CAN通信主索引93映射起始主索引(单体电压信息2)*/
#define CANMAP94_INDEX              150                     /*CAN通信主索引94映射起始主索引(单体温度信息1)*/
#define CANMAP95_INDEX              210                     /*CAN通信主索引94映射起始主索引(单体温度信息2)*/

#define CANMAP100_INDEX             0                       /*CAN通信主索引100映射起始主索引(系统通用参数)*/
#define CANMAP101_INDEX             25                      /*CAN通信主索引101映射起始主索引(系统硬件参数)*/
#define CANMAP102_INDEX             50                      /*CAN通信主索引102映射起始主索引(设备通用参数)*/
#define CANMAP103_INDEX             75                      /*CAN通信主索引103映射起始主索引(设备高级参数)*/
#define CANMAP104_INDEX             100                     /*CAN通信主索引104映射起始主索引(设备硬件参数)*/
/*=======================================通信数据类型定义=========================================*/
#define ICC_DT_U0                   0                       /*空类型*/
#define ICC_DT_U8                   1                       /*U8类型*/
#define ICC_DT_U16                  2                       /*U16类型*/
#define ICC_DT_U32                  4                       /*U32类型*/
#define ICC_DT_S8                   1                       /*S8类型*/
#define ICC_DT_S16                  2                       /*S16类型*/
#define ICC_DT_S32                  4                       /*S32类型*/
/*=======================================读写方式定义=======================================*/
#define ICC_READONLY                0x00                       /*只读*/
#define ICC_READWRITE               0x01                       /*读写*/
#define ICC_WRITEONLY               0x02                       /*只写*/
/*=======================================通信包配置===============================================*/

/*=======================================子索引长度配置===========================================*/
#define ICCMSG_CELL_NUM 8      /*单体通道信息配置组数*/
#define ICCMSG_CYCLE_NUM 20
#define ICCMSG_FAQ_NUM 28      /*含mIndex15/16历史DOD记录;BMU_INDEX_NUM=56,BMU表实际注册56条(含68/73/75/76),严格相等无零尾表项*/
#define ICCMSG_UPDATE_NUM 10


#define ICCMSGINDEX0_LEN               4                       /*核间通信主索引0数组长度(校时信息)*/
#define ICCBaseDef_LEN  eICCRBase_TxTabLen


#define ICCMSGCELLDEF_LEN              (ICCMSG_CELL_NUM)        /*核间通信CELL通道配置信息长度*/
#define ICCMSGCYCLEDEF_LEN             (ICCMSG_CYCLE_NUM)       /*核间通信CYCLE通道配置信息长度*/
#define ICCMSGFAQDEF_LEN               (ICCMSG_FAQ_NUM)         /*核间通信FAQ通道配置信息长度*/
#define ICCMSGUPDATEDEF_LEN            (ICCMSG_UPDATE_NUM)      /*核间通信UPDATE通道配置信息长度*/





/*周期发送信息长度配置*/
#define CAN_TX_VER_NUM              0                       /*主动发送校时帧信息配置组数*/
#define CAN_TX_BRE_NUM              3                       /*主动上传心跳帧信息配置组数*/
/*核间通信信息长度配置*/
#define CAN_TX_CEL_NUM              (ICCMSG_CELL_NUM)                      /*主动上传单体信息配置组数*/
#define CAN_TX_CYL_NUM              (ICCMSG_CYCLE_NUM)                     /*主动上传循环信息配置组数*/
#define CAN_TX_FAQ_NUM              (ICCMSG_FAQ_NUM)                       /*查询回复问答信息配置组数*/

/*通信包基本配置*/
#define ICC_DEVTAB_NUM              1
#define BMU_INDEX_NUM               (ICCMSG_CELL_NUM + ICCMSG_CYCLE_NUM + ICCMSG_FAQ_NUM + 13)                    /*BMU主索引注册数量(含81/82/83/84版本号序列号 + 85内阻查询 + 28-35内阻实部/虚部)*/
#define ICC_TRTAB_NUM               BMU_INDEX_NUM           /*收发处理表数量*/

/*=======================================版本号/序列号主索引子索引长度配置===================================*/
#define ICC_MINDEX81_VER_LEN        (8)                     /*81 版本号 8子索引*/
#define ICC_MINDEX82_SERIAL_LEN     (26)                    /*82 设备序列号 3+23子索引*/
#define ICC_MINDEX83_PACKSERIAL_LEN (23)                    /*83 PACK序列号 3+20子索引*/
#define ICC_MINDEX84_CELLSERIAL_LEN (4 + 24 * 42)           /*84 电芯序列号 4+24*42子索引*/

/*版本号/序列号主索引数据缓冲(定义于IccMsgVerSrl.c)*/
extern u8 gIccVerSrl_81[ICC_MINDEX81_VER_LEN];
extern u8 gIccDevSerial_82[ICC_MINDEX82_SERIAL_LEN];
extern u8 gIccPackSerial_83[ICC_MINDEX83_PACKSERIAL_LEN];
extern u8 gIccCellSerial_84[ICC_MINDEX84_CELLSERIAL_LEN];

/*内阻查询(85)/内阻实部(28-31)/内阻虚部(32-35)数据缓冲(定义于GroupInfo.c, 长度宏定义于GroupInfo.h)*/
extern u16 *gIccInsuQuery_85;
extern u32 *gIccCellResisReal;
extern u32 *gIccCellResisImag;

/*=======================================PCS模块组信息主索引定义====================================*/
#define ICC_MINDEX_ACDC_GRP_INFO   81                      /*ACDC模块组信息主索引*/
#define ICC_MINDEX_ACDC_GRP2_INFO  82                      /*ACDC模块组2信息主索引*/
#define ICC_MINDEX_SYS_OVERALL    80                /*模块整体信息主索引*/
#define ICC_MINDEX_DCDC_GRP_INFO   83                      /*DCDC模块组信息主索引*/
#define ICC_MINDEX_DCDC_GRP2_INFO  84                      /*DCDC模块组2信息主索引*/

/*=======================================PCS单模块信息主索引定义====================================*/
#define ICC_MINDEX_ACDC_MOD_INFO   85                      /*ACDC单模块信息主索引*/
#define ICC_MINDEX_ACDC_MOD2_INFO  86                      /*ACDC单模块2信息主索引*/
#define ICC_MINDEX_DCDC_MOD_INFO   87                      /*DCDC单模块信息主索引*/
#define ICC_MINDEX_DCDC_MOD2_INFO  88                      /*DCDC单模块2信息主索引*/

/*=======================================PCS模块配置及控制主索引定义================================*/
#define ICC_MINDEX_MODULE_TYPE      110                     /*模块类型配置主索引*/
#define ICC_SINDEX_MODULE_COUNT     6                       /*模块数配置子索引数目*/
#define ICC_MINDEX_MODULE_COUNT     111                     /*模块数配置主索引*/
#define ICC_SINDEX_MODULE_TYPE      6                       /*模块类型配置子索引数目*/
#define ICC_MINDEX_ACDC_CTRL        112                     /*ACDC控制主索引*/
#define ICC_MINDEX_DCDC_CTRL        113                     /*DCDC控制主索引*/
#define ICC_MINDEX_AUTO_MANUAL      114                     /*自动/手动模式主索引*/
#define ICC_MINDEX_GRID_CTRL        115                     /*并离网控制主索引*/
/*周期发送信息配置总组数*/
//#define ICC_TX_ALL_NUM              (CAN_TX_CEL_NUM+CAN_TX_CYL_NUM)
/*=================================================================================================
** @Data type definition
**===============================================================================================*/
/*核间通信包基本配置信息索引定义*/


/*CELL通道数据配置结构体*/
typedef struct _ICCCELLDATA_EEF_
{
    u8 mIndex;                          //主索引
    u16 sIndex;                      	//起始子索引
	u16 sIndexNum;                       //子索引数目
	u8 mapIndex;						//映射起始主索引号
	u16 cycleTime;                      //发送周期
    u8 daddr;                           //发送目的地址
    u8 txAble;                          //发送使能 1：主动发送 0：不发送
}t_ICCCellDataDef;

/*CYCLE通道数据配置结构体*/
typedef struct _ICCCYCLEDATA_EEF_
{
    u8 mIndex;                          //主索引
    u16 sIndex;                      	//起始子索引
	u16 sIndexNum;                       //子索引数目
	u16 cycleTime;                      //发送周期
    u8 daddr;                           //发送目的地址
    u8 txAble;                          //发送使能 1：主动发送 0：不发送
}t_ICCCycleDataDef;

/*FAQ通道数据配置结构体*/
typedef struct _ICCFAQDATA_EEF_
{
    u8 mIndex;                          //主索引
    u16 sIndex;                      	//起始子索引
	u16 sIndexNum;                      //子索引数目
    u8 daddr;                           //发送目的地址
}t_ICCFaqDataDef;

/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/
/*=================================================================================================
** @Name      : u8 ICCDeviceTableInit(void)
** @Input     : u8
** @Output    : void
** @Function  : 核间通信设备表初始化
** @The notes : 核间通信包总初始化调用
**===============================================================================================*/
u8 ICCDeviceTableInit(void);

#endif