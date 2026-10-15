#ifndef	TASKN2402_H
#define	TASKN2402_H

#define MODNUM_N2402        4       //N2402模块数
#define N2402_MAX_GRP_NUM   2		//N2402模块最大组数
#define IS_N2402_GRP(i)   (((i) <= 2) && ((i) >= 1))	//是否属于N2402模块组号

#define R_PROTO_SELF_ADDR		0xF0	//本机地址

#define MODTYPE_ACDC_NONE    0
#define MODTYPE_ACDC_N2101   1
#define MODTYPE_ACDC_N2002   2
#define MODTYPE_ACDC_N2402   3

#define MODTYPE_DCDC_NONE    0
#define MODTYPE_DCDC_N2302   1

#define MODTYPE_MPPT_NONE    0
#define MODTYPE_MPPT_30KW    1


#define R_PRPTO_DEVNO_0A	    0x0A            /*设备号：0x0A-单模块*/
#define R_PRPTO_DEVNO_0B	    0x0B            /*设备号：0x0B-模块组*/
#define R_PRPTO_DEVNO_RAM       0x09            /*设备号：0x09-模块RAM*/

#define W_VALUE_A0              0xA0            /*设置值：0xA0*/
#define W_VALUE_A1              0xA1            /*设置值：0xA1*/
#define W_VALUE_A2              0xA2            /*设置值：0xA2*/

#define ABU_Addr                0xFA            /*ABU地址*/
#define MPPT_MODULE_BASE_ADDR   0x80            /*MPPT单模块地址：0x80-0xDF*/

#define	RECT_STAT_CODE_0_ALM_BITs	0x2F	
#define	RECT_STAT_CODE_1_ALM_BITs	0xBE
#define	RECT_STAT_CODE_2_ALM_BITs	0x7F

#define	RECT_STAT_CODE_0_ALM_BITs_PCS	0x00	
#define	RECT_STAT_CODE_1_ALM_BITs_PCS	0x0F
#define	RECT_STAT_CODE_2_ALM_BITs_PCS	0xFF
#define	RECT_STAT_CODE_3_ALM_BITs_PCS	0xFF

#define mVERHIGH           			5      //软件版本号高 //test
#define mVERLOW            			11      //软件版本号低

#define BATT_GRP_MAX_NUM			(4)			//电池柜最大数量
#define ENABLE_OTA_RECT_SILLENCE	(1)			//静默模式开关

#define RECT_ON						0			//手动控制模式-开机
#define RECT_OFF					1			//手动控制模式-关机

#define	mRECTNUM					(6)		//模块最大数量

#define	GRP_WORKMODE_SET_FORWORD	0		//并网整流/对电池侧充电
#define	GRP_WORKMODE_SET_REVERSE	1		//并网逆变/对电池侧放电
#define	GRP_WORKMODE_SET_REVERSE_OffGrid	2
#define	GRP_WORKMODE_SET_MPPT				3

#define RECT_ADDRESS_SCAN_DELAY				2		//模块排地址后采集延后次数
#define SINGLE_RECT_SCAN_LOOP_NUM			2		//模块排地址后采集单模块命令延后次数

/*与时间计算相关的年-月-日-时-分-秒*/
#define MIN_SECONDS         (60l)
#define HOUR_SECONDS    	(3600l)
#define DAY_SECONDS     	(24l * HOUR_SECONDS)
#define MON28_SECONDS       (28l * DAY_SECONDS)
#define MON29_SECONDS       (29l * DAY_SECONDS)
#define MON_SECONDS         (30l * DAY_SECONDS)
#define MON31_SECONDS       (31l * DAY_SECONDS)
#define YEAR_SECONDS        (365 * DAY_SECONDS)
#define LEAPYEAR_SECONDS    (366 * DAY_SECONDS)

/*与时间计算相关的时区之类的*/
extern float  GetLocalTimeZone(void);
#define GMT_TIME_MAX    (2145916800L - 1)
#define TIME_MAX        ULONG_SUB_SLONG(2145916800L - 1,\
  -(long)(GetLocalTimeZone() * MIN_SECONDS)) /*0x7fffffff*/
                                                     //2038.01.01 00:00:00 Thu
#define GMT_TIME_MIN    946684800L
#define TIME_MIN        ULONG_SUB_SLONG(946684800L, \
  -(long)(GetLocalTimeZone() * MIN_SECONDS)) /*2000.01.01 00:00:00*/
                                                     //0=1970.01.01
/*与时间计算相关的年份*/
#define YEAR0                   2000
#define EPOCH_YR                1970
#define SECS_DAY                (24L * 60L * 60L)
#define LEAPYEAR(year)          (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year)          (LEAPYEAR(year) ? 366 : 365)


#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/* signed long add to unsigned long */
#define ULONG_ADD_SLONG( unsignedLong, signedLong )          \
	(((signedLong) > 0) ? ((unsignedLong) + (signedLong))    \
	: ((unsignedLong) - (-signedLong)) )
/* unsigned long sub signed long */
#define ULONG_SUB_SLONG( unsignedLong, signedLong )          \
	(((signedLong) > 0) ? ((unsignedLong) - (signedLong))    \
	: ((unsignedLong) + (-signedLong)) )

/*与MPU通信交互相关*/
#define	ENJI_RCV_OVERTIME_MAX		10			//秒 接收中断间隔不能超过这个时间
#define	UART6_ENJI_SEND_MAX_NUM		400			//串口6单次发送最大数据量
#define	UART6_ENJI_RECV_MAX_NUM		400			//串口6单次接收最大数据量

#define ENJI_M_S_COMM_RCV_BUF_NUM	3			//接收MPU信息缓存区大小

#define CAN_MSG_MAX_NUM				15			//一次性转发给MPU的CAN报文最大数目

//MPPT模块借用组号GRP_D_ES，但实际通信时采用0组地址
#define	MPPT_GRP_NO				GRP_3_MPPT
#define	MPPT_GRP_NO_2			GRP_4_MPPT

#define	MPPT_GRP_NO_MIN			GRP_3_MPPT		
#define	MPPT_GRP_NO_MAX			GRP_4_MPPT

#define	GRP_NO_MIN				GRP_1_ACDC
#define	GRP_NO_MAX				GRP_6_DCDC	
#define	ACDC_GRP_NO_MIN			GRP_1_ACDC	
#define	ACDC_GRP_NO_MAX			GRP_2_ACDC
#define	DCDC_GRP_NO_MIN			GRP_5_DCDC	
#define	DCDC_GRP_NO_MAX			GRP_6_DCDC	

#define IS_ACDC_GRP(i) 		((i) <= ACDC_GRP_NO_MAX)	//ACDC模块（如果ACDC组变成了从大于0开始，这里需要加上下限）
#define IS_DCDC_GRP(i) 		(((i) >= DCDC_GRP_NO_MIN) && ((i) <= DCDC_GRP_NO_MAX))	//DCDC模块
#define IS_MPPT_GRP(i) 		(((i) >= MPPT_GRP_NO_MIN) && ((i) <= MPPT_GRP_NO_MAX))	//MPPT模块

#define	mMPPTNUM_A	2	//MPPT模块组A的模块数
#define	mMPPTNUM_B	2	//MPPT模块组B的模块数
#define	mMPPTNUM_C	2	//MPPT模块组C的模块数
#define	mMPPTNUM_D	2	//MPPT模块组D的模块数
#define	mMPPTNUM	(mMPPTNUM_A)		//MPPT模块单模块总数量

//20220122 lk ACDC模块如果是BEG模块，则有些采集处理会不同
#define	ACDC_IS_BEG_TYPE	0		    //22.5kw PCS  BEG模块：用于V2G的ACDC模块
#define ACDC_IS_PCS_TYPE	1		    //62.5KW PCS
#define ACDC_IS_235_PCS_TYPE	2		//235KW PCS

#define YDN23_VER_PST	1

typedef enum tag_RECT_GRP_Idx
{
	GRP_1_ACDC = 0,  //ACDC模块
	GRP_2_ACDC,

	GRP_3_MPPT,     //MPPT模块
	GRP_4_MPPT,

	GRP_5_DCDC,     //DCDC模块
	GRP_6_DCDC,

	_MAX_GRP_NUM
}RECT_GRP_IDX;


/*=================================================================================================
** @Data type definition
**===============================================================================================*/
//235KW_PCS中读取模块信息周期任务
enum RECT_CMD_NO_Def
{
	READ_INFO_CMD_2S_NO_0 = 0,	//2S周期任务,500ms进一次
	READ_INFO_CMD_2S_NO_1,
	READ_INFO_CMD_2S_NO_2,	
	READ_INFO_CMD_2S_NO_3,
	READ_INFO_CMD_2S_NO_MAX,

	READ_INFO_CMD_8S_NO_0 = 0,	//8S周期任务,4S进一次
	READ_INFO_CMD_8S_NO_1,
	READ_INFO_CMD_8S_NO_MAX,

	READ_INFO_CMD_12S_NO_0 = 0,	//12S周期任务,500ms进一次
	READ_INFO_CMD_12S_NO_1,
	READ_INFO_CMD_12S_NO_2,
	READ_INFO_CMD_12S_NO_3,
	READ_INFO_CMD_12S_NO_4,
	READ_INFO_CMD_12S_NO_5,
	READ_INFO_CMD_12S_NO_6,
	READ_INFO_CMD_12S_NO_7,
	READ_INFO_CMD_12S_NO_8,
	READ_INFO_CMD_12S_NO_9,
	READ_INFO_CMD_12S_NO_10,
	READ_INFO_CMD_12S_NO_11,
	READ_INFO_CMD_12S_NO_12,
	READ_INFO_CMD_12S_NO_13,
	READ_INFO_CMD_12S_NO_MAX,

	READ_INFO_CMD_18S_NO_0 = 0,	//18S周期任务,2S进一次
	READ_INFO_CMD_18S_NO_1,
	READ_INFO_CMD_18S_NO_2,
	READ_INFO_CMD_18S_NO_3,
	READ_INFO_CMD_18S_NO_4,
	READ_INFO_CMD_18S_NO_5,
	READ_INFO_CMD_18S_NO_6,
	READ_INFO_CMD_18S_NO_7,
	READ_INFO_CMD_18S_NO_8,
	READ_INFO_CMD_18S_NO_MAX,

	READ_INFO_CMD_FAST_NO_0 = 0,	/*快速周期任务(自旋转2个slot) DC V/I每圈都发*/
	READ_INFO_CMD_FAST_NO_1,
	READ_INFO_CMD_FAST_NO_2,
	READ_INFO_CMD_FAST_NO_3,
	READ_INFO_CMD_FAST_NO_4,
	READ_INFO_CMD_FAST_NO_MAX,

	READ_INFO_CMD_SLOW_NO_0 = 0,	//慢速周期任务(自旋转2个slot)
	READ_INFO_CMD_SLOW_NO_1,
	READ_INFO_CMD_SLOW_NO_MAX,
};

//235KW_PCS的充放电设置状态
typedef enum ChargDischarg235KWPCSSetStateDef
{
	SetState235KWPCS_Error = 0,			//设置错误
	SetState235KWPCS_Start,				//开始设置
	SetState235KWPCS_OnGridSucc,		//并网模式设置成功
	SetState235KWPCS_OffGridSucc,		//离网模式设置成功
	SetState235KWPCS_StorModeSucc,		//储能模式使能设置成功
	SetState235KWPCS_DisStorModeSucc,	//储能模式不使能设置成功
	SetState235KWPCS_ACDispSucc,		//交流调度设置成功
	SetState235KWPCS_DCDispSucc,		//直流调度设置成功
	SetState235KWPCS_SetPowerSucc,		//有功功率设置成功
	SetState235KWPCS_SetCurrSucc,		//电流设置成功
	SetState235KWPCS_SetVoltSucc,		//电压设置成功
	SetState235KWPCS_SetFreqSucc,		//频率设置成功

	SetState235KWPCS_Successful,		//模式设置成功

}SEt_STATE_235KWPCS;
//235kw_PCS控制充放电的几种模式
enum IMMU2ManualCtrl235kwCmdDef
{
	IMMU2ManualCtrl235kwCmd_Stop = 0,	//停止
	IMMU2ManualCtrl235kwCmd_Chging,		//并网整流-充电
	IMMU2ManualCtrl235kwCmd_Dischging,	//并网逆变-放电
	IMMU2ManualCtrl235kwCmd_OffGrid,	//离网模式
};


/* N2402逆变状态表0x12 */
union _Convert_Stat_N2402_12_def
{
    BYTE	BYTE_FIELD[4];
    struct
    {
        unsigned char bRunState         : 3;    /* Bit2~0: 0-关机 1-运行 2-待机 3-故障 4-告警 */
        unsigned char bOffGridSta       : 1;    /* Bit3: 1-离网 0-并网 */
        unsigned char bPCSWorkMode      : 2;    /* Bit5~4: 1-放电 2-充电 */
        unsigned char bPCSWorkStatus    : 2;    /* Bit7~6: 0-关机 1-开机 */

        unsigned char bBusUvp           : 1;    /* Bit0: 电网电压高于母线电压 */
        unsigned char bHorLVRTErr       : 1;    /* Bit1: 高低穿故障 */
        unsigned char bBatDOD           : 1;    /* Bit2: 电池DOD告警 */
        unsigned char bBatEOD           : 1;    /* Bit3: 电池EOD告警 */
        unsigned char bUserSetErr       : 1;    /* Bit4: 参数设置错误 */
        unsigned char bRlyBad           : 1;    /* Bit5: 继电器错误 */
        unsigned char bSoftFail         : 1;    /* Bit6: 软启故障 */
        unsigned char bAuxPowerFail     : 1;    /* Bit7: 辅源故障 */

        unsigned char bEPOErr           : 1;    /* Bit0: EPO故障 */
        unsigned char bIDDup            : 1;    /* Bit1: ID重复 */
        unsigned char bOutUnBal         : 1;    /* Bit2: 模块严重不均流 */
        unsigned char bFanErr           : 1;    /* Bit3: 风扇故障 */
        unsigned char bSysOtp           : 1;    /* Bit4: 过温故障 */
        unsigned char bPllFault         : 1;    /* Bit5: 锁相故障 */
        unsigned char bIslandPro        : 1;    /* Bit6: 孤岛保护 */
        unsigned char bComFault         : 1;    /* Bit7: 通讯故障 */

        unsigned char bOPDerate         : 1;    /* Bit0: 输出降载 */
        unsigned char bOPInc            : 1;    /* Bit1: 输出过载 */
        unsigned char bOutShort         : 1;    /* Bit2: 输出短路 */
        unsigned char bOcpInter         : 1;    /* Bit3: 内部过流 */
        unsigned char bBusErr           : 1;    /* Bit4: 母线故障 */
        unsigned char bDcFault          : 1;    /* Bit5: 直流故障 */
        unsigned char bAcFault          : 1;    /* Bit6: 交流故障 */
        unsigned char bSysFault         : 1;    /* Bit7: 系统故障 */

    }BIT_FIELD;
};
typedef union _Convert_Stat_N2402_12_def	Convert_Stat_N2402_12;

/* N2402逆变状态表0x13 */
union _Convert_Stat_N2402_13_def
{
    BYTE   BYTE_FIELD[4];
    struct
    {
        unsigned char   bSCIComFault            : 1;    /* Bit0: SCI通讯故障 */
        unsigned char   bSPIComFault            : 1;    /* Bit1: SPI通讯故障 */
        unsigned char   bACOverVoltage          : 1;    /* Bit2: 1：交流过压 */
        unsigned char   bACUnderVoltage         : 1;    /* Bit3: 1：交流欠压 */
        unsigned char   bACOverFreq             : 1;    /* Bit4: 1：交流过频 */
        unsigned char   bACUnderFreq            : 1;    /* Bit5: 1：交流欠频 */
        unsigned char   bBusAbnormal            : 1;    /* Bit6: 1：母线异常 */
        unsigned char   bBusOverVoltage         : 1;    /* Bit7: 1：母线过压 */

        unsigned char   bBusUnderVoltage        : 1;    /* Bit0: 1：母线欠压 */
        unsigned char   bInputNoPower           : 1;    /* Bit1: 1：输入没电 */
        unsigned char   bInputPhaseLoss         : 1;    /* Bit2: 1：输入缺相 */
        unsigned char   bInputFlicker           : 1;    /* Bit3: 1：输入闪断 */
        unsigned char   bSoftwareOverCurrent    : 1;    /* Bit4: 1：软件采样过流 */
        unsigned char   bACTZOverCurrent        : 1;    /* Bit5: 1：ACTZ过流 */
        unsigned char   bInputVolUnbalance      : 1;    /* Bit6: 1：输入电压不平衡 */
        unsigned char   bPhaseSeqError          : 1;    /* Bit7: 1：相序错误 */

        unsigned char   bPLLError_Bit7          : 1;    /* Bit0: 1：锁相错误 */
        unsigned char   bPLLError_Bit6          : 1;    /* Bit1: 1：锁相错误 */
        unsigned char   bACRelayFault           : 1;    /* Bit2: 1：AC继电器故障 */
        unsigned char   bACShortCircuit         : 1;    /* Bit3: 1：AC 短路 */
        unsigned char   bIslandProtection       : 1;    /* Bit4: 1：孤岛保护 */
        unsigned char   bESTOP                  : 1;    /* Bit5: 1：ESTOP */
        unsigned char   bBatteryReverse         : 1;    /* Bit6: 1：电池反接 */
        unsigned char   bOutputOverVoltage      : 1;    /* Bit7: 1：输出过压 */

        unsigned char   bOutputUnderVolProt     : 1;    /* Bit0: 1：输出欠压保护 */
        unsigned char   bOutputShortWarn        : 1;    /* Bit1: 1：输出短路告警 */
        unsigned char   bGridRelayOpen          : 1;    /* Bit2: 1：网侧继电器开路 */
        unsigned char   bGridRelayShort         : 1;    /* Bit3: 1：网侧继电器短路 */
        unsigned char   bInvRelayOpen           : 1;    /* Bit4: 1：逆变侧继电器开路 */
        unsigned char   bInvRelayShort          : 1;    /* Bit5: 1：逆变继电器短路 */
        unsigned char   bDCRelayFault           : 1;    /* Bit6: 1：直流继电器故障 */
        unsigned char   bSynchroAbnormal        : 1;    /* Bit7: 1：工频同步异常 */
    } BIT_FIELD;
};
typedef union _Convert_Stat_N2402_13_def	Convert_Stat_N2402_13;

/* N2402逆变状态表0x14 */
union _Convert_Stat_N2402_14_def
{
    BYTE   BYTE_FIELD[4];
    struct
    {
        unsigned char   bParallelComFault       : 1;    /* Bit0: 1：并机通讯故障 */
        unsigned char   bDCRelaySoftStartFault  : 1;    /* Bit1: 1：直流软启继电器故障 */
        unsigned char   bBusSoftStartFault      : 1;    /* Bit2: 1：Bus软启动故障 */
        unsigned char   bInvSoftStartFault      : 1;    /* Bit3: 1：逆变软启故障 */
        unsigned char   bOutputCurrentImbalance : 1;    /* Bit4: 1：输出电流不均流检测 */
        unsigned char   reserved_upper          : 3;    /* Bit7~5: 保留1 */

        unsigned char   reserved_byte1          : 8;    /* Bit7~0: 保留2 */
        unsigned char   reserved_byte2          : 8;    /* Bit7~0: 保留3 */
        unsigned char   reserved_byte3          : 8;    /* Bit7~0: 保留4 */

    } BIT_FIELD;
};
typedef union _Convert_Stat_N2402_14_def	Convert_Stat_N2402_14;

/* N2402逆变状态表0x15 */
union _Convert_Stat_N2402_15_def
{
    BYTE   BYTE_FIELD[4];
    struct
    {
        unsigned char   bCANComFault            : 1;    /* Bit0: 1：CAN通讯故障 */
        unsigned char   bInletOverTemp          : 1;    /* Bit1: 1：进水口过温 */
        unsigned char   bOutletOverTemp         : 1;    /* Bit2: 1：出水口过温 */
        unsigned char   bIPMA_OverTemp          : 1;    /* Bit3: 1：IPMA过温 */
        unsigned char   bIPMB_OverTemp          : 1;    /* Bit4: 1：IPMB过温 */
        unsigned char   bIPMC_OverTemp          : 1;    /* Bit5: 1：IPMC过温 */
        unsigned char   bIPMD_OverTemp          : 1;    /* Bit6: 1：IPMD过温 */
        unsigned char   bAmbientOverTemp        : 1;    /* Bit7: 1：环温过温 */

        unsigned char   bAuxPowerFault          : 1;    /* Bit0: 1：辅源故障 */
        unsigned char   bInsulationDetectFault  : 1;    /* Bit1: 1：绝缘检测故障 */
        unsigned char   bLeakageCurrentFault    : 1;    /* Bit2: 1：漏电流故障 */
        unsigned char   bFanFault               : 1;    /* Bit3: 1：风扇故障 */
        unsigned char   bVoSettingOverProt      : 1;    /* Bit4: 1：Vo设置超保护值 */
        unsigned char   bVoSettingLow           : 1;    /* Bit5: 1：Vo设置偏低 */
        unsigned char   bIDDuplication          : 1;    /* Bit6: 1：ID重复 */
        unsigned char   bSoftStartResOverTemp   : 1;    /* Bit7: 1：软起电阻过温 */

        unsigned char   reserved_byte2          : 8;    /* Bit7~0: 保留2 */
        unsigned char   reserved_byte3          : 8;    /* Bit7~0: 保留3 */
    } BIT_FIELD;
};
typedef union _Convert_Stat_N2402_15_def	Convert_Stat_N2402_15;

/*所有单模块信息*/
typedef struct tagARectRawN2402_Data
{
    int     u32ModDCVolt;                   //模块直流侧电压 mV 1101
    int     u32ModBattCurr;                 //模块直流侧侧电流 mA 1102
    int     u32ModACLineVolt_AB;            //模块交流AB线电压 mV 1103
    int     u32ModACLineVolt_BC;            //模块交流BC线电压 mV 1104
    int     u32ModACLineVolt_CA;            //模块交流CA线电压 mV 1105
    int     u32EnvTemp;                     //模块环温 m℃ 1106



    float   fAvailableCurr;			        //可用电流 单位：A    CMD=0x0C
    float   fMax_OutCurr;			        //最大输出电流  单位：A	   CMD=0x0A
    float   fRatedPower;			        //额定功率 单位：kw  0x1133
    float   fRatedAddPower;			        //用于计算组额定的单模块额定功率中间值 单位：kw
    float   fAddAvailablePower;		        //用于计算组可用功率的单模块可用功率中间值 单位：kw

    /*版本等固定信息*/
    UINT     u32DC_Ver;			        //DC软件版本号
    UINT     u32AC_Ver;			        //AC软件版本号
    float   fHW_Ver;				        //模块硬件版本号


    USHORT     u16BelongedGrp;                 //模块组号 0x1120
    BYTE      bIsExisted;                     //模块 存在标志
    BYTE      bCommFail;                      //是否通信中断 1-通信中断 0-正常通信
    BYTE      u8CommFailTimes;                //通信失败次数
    BYTE      u8IsFault;                //模块失效 1-故障（失效） 0-正常
    BYTE      bStartupModeSucc;               //启机方式设置成功标志
    BYTE      bGridModeSucc;                  //并离网模式设置成功标志
    BYTE      bStorModeSucc;                  //储能模式设置成功标志
    BYTE      bDispatchModeSucc;              //调度模式设置成功标志

    BYTE      bBarcode_24[4];			        //模块条码信息0 0x1124
    BYTE      bBarcode_25[4];			        //模块条码信息1 0x1125
    BYTE      bBarcode_26[4];			        //模块条码信息2 0x1126
    BYTE      bBarcode_27[4];			        //模块条码信息3 0x1127
    BYTE      bBarcode_28[4];			        //模块条码信息4 0x1128
    BYTE      bBarcode_29[4];			        //模块条码信息5 0x1129

    UINT     u32ModACPhaseVolt_A;            //交流A相相电压 MV 2101
    UINT     u32ModACPhaseVolt_B;            //交流B相相电压 MV 2102
    UINT     u32ModACPhaseVolt_C;            //交流C相相电压 MV 2103
    UINT     u32ModACPhaseCurr_A;            //交流A相电流 MA 2104
    UINT     u32ModACPhaseCurr_B;            //交流B相电流 MA 2105
    UINT     u32ModACPhaseCurr_C;            //交流C相电流 MA 2106
    UINT     u32ModACFreq;                   //交流频率 mHz 2107
    UINT     u32ModActivePower_Total;        //总有功功率 mW 2108
    UINT     u32ModActivePower_A;            //A相有功功率 mW 2109
    UINT     u32ModActivePower_B;            //B相有功功率 mW 210A
    UINT     u32ModActivePower_C;            //C相有功功率 mW 210B
    UINT     u32ModReActivePower_Total;      //总无功功率 mW 210C
    UINT     u32ModReActivePower_A;          //A相无功功率 mW 210D
    UINT     u32ModReActivePower_B;          //B相无功功率 mW 210E
    UINT     u32ModReActivePower_C;          //C相无功功率 mW 210F
    UINT     u32ModApparentPower_Total;      //总视在功率 mVA 2110
    UINT     u32ModApparentPower_A;          //A相视在功率 mVA 2111
    UINT     u32ModApparentPower_B;          //B相视在功率 mVA 2112
    UINT     u32ModApparentPower_C;          //C相视在功率 mVA 2113
    UINT     u32ModPowerFactor_Total;        //总功率因素 0.001 2114
    UINT     u32ModPowerFactor_A;            //A相功率因素 0.001 2115
    UINT     u32ModPowerFactor_B;            //B相功率因素 0.001 2116
    UINT     u32ModPowerFactor_C;            //C相功率因素 0.001 2117


    Convert_Stat_N2402_12    StatCode_12;       //模块状态表 0x12
    Convert_Stat_N2402_13    StatCode_13;       //模块状态表 0x13
    Convert_Stat_N2402_14    StatCode_14;       //模块状态表 0x14
    Convert_Stat_N2402_15    StatCode_15;       //模块状态表 0x15

	// _Module_RAM_BitMaps_1	RAMInfo;            //模块RAM信息
} __attribute__((aligned(4))) RECT_RAW_N2402_DATA;

/*模块整体信息*/
typedef struct tagRectN2402_Data
{ 
    USHORT         u16DelayCount_5S;               //5秒循环计数器
    USHORT         u16DelayCountInterval_5S;       //5秒循环周期
    BYTE          u8ScanedRectNum;                //模块总数
	BYTE          u8AddressScanDelay;			    //排地址延时, 扫描两圈后再判断模块故障
	BYTE          bySelfAddress;                  //本监控软件与整流模块通信时采用的地址 0xF0	//test 这里后面改成可配置
	BYTE          u8SingleRectScanCmdCnt;         //单模块查询命令的执行次数
	BYTE          u8IsAllCommFail;                //所有模块通信中断

    BYTE          u8GetTotalInfoDelay;            //查询模块信息总任务延时
    BYTE          u8GetTotalInfoInterval;         //查询模块信息总任务周期    100ms
    BYTE          u8Set235KWPCSWrokModeDelay;     //设置模块工作模式延时
    BYTE          u8Set235KWPCSWrokModeInterval;   //设置模块工作模式周期      1s
    BYTE          u8Set235KWPCSTimeDelay;         //235模块时间校准延时
    BYTE          u8Set235KWPCSTimeInterval;      //235模块时间校准周期
    BYTE          u8Read235KWPCSInfo2SDelay;      //2S周期任务延时 500ms周期--
    BYTE          u8Read235KWPCSInfo2SInterval;   //2S周期任务周期
    BYTE          u8Read235KWPCSInfo8SDelay;      //8S周期任务延时 4S周期--
    BYTE          u8Read235KWPCSInfo8SInterval;   //8S周期任务周期
    BYTE          u8Read235KWPCSInfo12SDelay;      //12S周期任务延时 500ms周期--
    BYTE          u8Read235KWPCSInfo12SInterval;   //12S周期任务周期
    BYTE          u8Read235KWPCSInfo18SDelay;      //18S周期任务延时 2S周期--
    BYTE          u8Read235KWPCSInfo18SInterval;   //18S周期任务周期


    BYTE          u8SetAdjVoltCurrDelay;          //设置电压电流延时
    BYTE          u8SetAdjVoltCurrInterval;       //设置电压电流周期
    BYTE          u8SetOnOffDelay;                //模块开关机延时
    BYTE          u8SetOnOffInterval;             //模块开关机周期
    BYTE          u8GetModeCalcTimeDelay;         //更新模块计算数据延时
	BYTE          u8GetModeCalcTimeInterval;	//更新模块计算数据周期

    BYTE          SetCount_235KWMODE; 

    BYTE          u8TaskCtrl235KwPCSCMD[N2402_MAX_GRP_NUM];
    BYTE          u8OnlinePowerModuleTypeNum;                             //在线模块类型数
    BYTE          u8OnlinePowerModuleType[N2402_MAX_GRP_NUM+1][5];        //在线模块类型信息记录
    char        cOnlinePowerModuleTypeName_U1[N2402_MAX_GRP_NUM+1][15]; //U1软件名
    char        cOnlinePowerModuleTypeName_U2[N2402_MAX_GRP_NUM+1][15]; //U2软件名
    
} __attribute__((aligned(4))) RECT_N2402_DATA;

/*系统整体信息*/
typedef struct tagGlobalSystemInfo_Data
{
    BYTE          u8TEST_MODE_FLAG;               //0:自动模式 1:手动模式
    BYTE          u8MPUCommAlm;                   //与EMS软件通信状态（//test 这里后续和BMS用同一个参数） 0:通信正常 1:通信中断
    BYTE          u8CfgModeType;                  //配置的模块类型
    BYTE          u8RectCommPeriodResetDelay;     //CAN通信中断延时
    BYTE		  byGrpIdx[32];                       //组号  后面改成可配置的
    BYTE          AddrInfo[32];                       //单模块地址（模块自己排址所得地址）
} __attribute__((aligned(4))) GLOBAL_SYS_DATA;
                    //N2402模块整体信息

/*================================================================================================================
** @Data type definition
**==============================================================================================================*/
//手动控制充放电的几种模式
enum IMMU2ManualCtrlGrpCmdDef
{
    IMMU2ManualCtrlGrpCmd_Init = 0,		//手动控制-初始化
    IMMU2ManualCtrlGrpCmd_Chging,		//手动控制-充电
    IMMU2ManualCtrlGrpCmd_Dischging,	//手动控制-放电
    IMMU2ManualCtrlGrpCmd_Stop,			//手动控制-停止
};

//读取数据序号
enum READINFO_NO_Def
{
	READINFO_NO_0 = 0,
	READINFO_NO_1,
	READINFO_NO_2,	
	READINFO_NO_3,
	READINFO_NO_4,
	READINFO_NO_5,
	READINFO_NO_6,
	READINFO_NO_7,	
	READINFO_NO_8,
	READINFO_NO_9,

	READINFO_NO_10,
	READINFO_NO_11,
	READINFO_NO_12,
	READINFO_NO_13,
	READINFO_NO_14,
	READINFO_NO_15,
	READINFO_NO_16,
	READINFO_NO_17,
	READINFO_NO_18,
	READINFO_NO_19,
	READINFO_NO_20,
	READINFO_NO_21,


	READINFO_NO_MAX,
};

//模块回复的错误码定义
typedef enum tagRECT_RET_ERR_Code
{
	RECT_RET_ERR_Code_NO_ERR = 0x00,        	//正常
	RECT_RET_ERR_Code_WRONG_ = 0x01,
	RECT_RET_ERR_Code_WRONG_CMD = 0x02,     	//命令号异常
	RECT_RET_ERR_Code_WRONG_DATA = 0x03,		//数据信息异常
	RECT_RET_ERR_Code_IN_STARTING = 0x07,   	//地址重排未完成
}RECT_RET_ERR_Code;

#define MODTYPE_ACDC_NONE    0
#define MODTYPE_ACDC_N2101   1
#define MODTYPE_ACDC_N2002   2
#define MODTYPE_ACDC_N2402   3

#define MODTYPE_DCDC_NONE    0
#define MODTYPE_DCDC_N2302   1

#define MODTYPE_MPPT_NONE    0
#define MODTYPE_MPPT_30KW    1

//交流侧三相
enum
{
	eType_Total = 0,
	eType_AB,
	eType_BC,
	eType_CA,
};

//模块读取类信息周期任务,5ms进一次
enum STEP_READINFO_NO_Def
{
	STEP_AC_VOLT_1 = 0,			//读相电压
	STEP_AC_CURR_2,				//读相电流
	STEP_ACT_POWER_3,			//读有功功率
	STEP_REACT_POWER_4,			//读无功功率
	STEP_BARCODE_5,				//读条码信息
	STEP_SER_NO_6,				//读软件版本号
	STEP_POWER_FA_7,			//读功率因素
	STEP_APPARENT_P_8,			//读视在功率
	STEP_LINE_VOLT_9,			//读线电压

	STEP_MAX,
};

//ABU开关机指令 开机-0xA0  关机-0xA1
enum
{
	eType_On = 0xA0,
	eType_Off = 0xA1,
};

typedef union TDWORD
{
	unsigned long			v;
	long					l;
	unsigned short int		w[2];
	signed int				s[2];
	unsigned char			b[4];
	char					c[4];
	float					f;
}
co_dword;

//can报文结构
typedef struct
{
    UINT Id;
    BYTE Data[8];
    BYTE Len;
} canmsg_rx;

//模块信息帧ID结构
typedef struct
{
    BYTE u8RcvDev;    		//源地址-8bits
    BYTE u8AimDev;    		//目标地址-8bits 
    BYTE u8Cmd;       		//命令号-6bits  0x23-读命令 0x24-写命令
    BYTE u8DevNo;				//设备号-4bits  0x0A-单模块命令 0x0B-模块组命令
    BYTE u8ErrorNum;  		//错误码-3bits  0-正常 2-命令号异常 3-数据信息异常 7-启动过程中
}Module_CanID;

union _XXXUnionValue
{
	UINT u32Value;
	float	ftemp;
	unsigned long Ltemp;
	BYTE	chtemp[4];
};
typedef union _XXXUnionValue	__XXX_UNION_VALUE;


typedef enum 
{
    DO_OFF = 0,
    DO_ON = 1,
}DoStatus_TypeDef;

union _16BitsUnionValue
{
    signed short i16Temp;
    unsigned short u16Temp;
    BYTE	chtemp[2];
};
typedef union _16BitsUnionValue	__16BITS_UNION_VALUE;

typedef enum e_ChargeDischargeMode
{
    eCDMode_Stop = 0,                                    /*停止模式*/
    eCDMode_Charge = 1,                                  /*充电模式*/
    eCDMode_Discharge = 2,                                /*放电模式*/
} e_ChargeDischargeMode;

/*=================================================================================================
** 自动/手动模式枚举定义
**===============================================================================================*/
typedef enum e_AutoManualMode
{
    eAutoMode = 0,                                        /*自动模式*/
    eManualMode = 1,                                      /*手动模式*/
} e_AutoManualMode;

/*=================================================================================================
** ACDC模块充放电控制数据结构
**===============================================================================================*/
typedef struct t_ACDCControlData
{
    int u32CtrlVolt;                                      /*控制电压[mV]*/
    UINT u32CtrlCurr;                                      /*控制电流[mA]*/
    UINT u32CtrlPower;                                     /*控制功率[W]*/
    e_ChargeDischargeMode eWorkMode;                      /*工作模式[0:停止 1:充电 2:放电]*/
} __attribute__((aligned(4))) t_ACDCControlData;

/*=================================================================================================
** 自动/手动模式数据结构
**===============================================================================================*/
typedef struct t_AutoManualMode
{
    e_AutoManualMode eMode;                               /*模式[0:自动 1:手动]*/
} __attribute__((aligned(4))) t_AutoManualMode;

/*=================================================================================================
** 全局变量声明
**===============================================================================================*/
extern t_ACDCControlData *gACDCControlData;              /*ACDC控制数据指针[ACDC_GRP_NUM组]*/
extern t_AutoManualMode *gAutoManualMode;                /*自动/手动模式指针*/

/*=================================================================================================
** ACDC模块组控制数据 (ICC 81/82)
**===============================================================================================*/
typedef struct tagPCSACDCGrpCtrlData
{
    int u32CtrlDCVolt;        /* 控制直流电压 [mV] */
    int u32CtrlDCCurr;        /* 控制直流电流 [mA] */
    int u32CtrlPower;         /* 控制有功功率 [mW] */
    int u32OnlineModNum;      /* 在线模块数 */
    int u32RatedPower;        /* 额定功率 [mW] */
    int u32RatedCurr;         /* 额定电流 [mA] */
    int u32MaxCurr;           /* 最大电流 [mA] */
    int u32CurrDCVolt;        /* 当前直流电压 [mV] */
    int u32CurrDCCurr;        /* 当前直流电流 [mA] */
    int u32CurrSysPower;      /* 当前系统有功功率 [mW] */
    int u32WorkState;         /* 工作状态 */
    int u32CtrlState;         /* 控制状态 */
} PCSACDCGrpCtrlData_t;

/*=================================================================================================
** ACDC单模块信息 (ICC 85/86)
**===============================================================================================*/
typedef struct tagPCSACDCModInfo
{
    int u32ModDCVolt;         /* 模块直流电压 [mV] */
    int u32ModDCCurr;         /* 模块直流电流 [mA] */
    int u32ActivePowerTotal;  /* 模块有功功率 [mW] */
    UINT u32ModNo;             /* 模块数量 */
} PCSACDCModInfo_t;

/*=================================================================================================
** 系统整体信息 (ICC 80)
**===============================================================================================*/
typedef struct tagSysOverallInfo
{
    UINT u32ModuleGrpOnline;   /* 模块组在线状态 */
} SysOverallInfo_t;

/*=================================================================================================
** ACDC数据总包
**===============================================================================================*/
typedef struct tagACDCDataInfo
{
    GLOBAL_SYS_DATA       gs_ModSystemInfo;
    RECT_N2402_DATA       gs_SysN2402Info;

    PCSACDCGrpCtrlData_t  gPCSACDCCtrlGrp[N2402_MAX_GRP_NUM]; //模块组信息
    RECT_RAW_N2402_DATA   gs_N2402RectInfo[MODNUM_N2402]; //每一个模块的详细详细信息

    PCSACDCModInfo_t      gPCSACDCModInfo[N2402_MAX_GRP_NUM][MODNUM_N2402];  // 单模块信息 

    SysOverallInfo_t      gSysOverallInfo;   //
} ACDC_DATA_INFO;

/*=================================================================================================
** @Name      : void InitN2402Info(void)
** @Input     : void
** @Output    : void
** @Function  : 初始化N2402相关数据
** @The notes : 
**===============================================================================================*/
void InitN2402Info(void);

/*=============================================================================================
** @Name      : static BYTE Set235kWPCS_WrokMode(void)
** @Input     : void
** @Output    : void
** @Function  : 235kw模块充放电设置
** @The notes : 所有向235KW模块发送的控制充放电相关报文都在这个函数内。
**===========================================================================================*/
void Set235kWPCS_WrokMode(ROUGH_CCU_DATA* pCCU, UINT SGNo, t_ACDCControlData ctrlData);
void N2402_SetCtrlDCVolt(BYTE grpIdx, UINT volt);
void N2402_SetCtrlDCCurr(BYTE grpIdx, UINT curr);
void N2402_SetCtrlACPower(BYTE grpIdx, UINT power);
UINT N2402_GetCtrlDCVolt(BYTE grpIdx);
UINT N2402_GetCtrlDCCurr(BYTE grpIdx);
UINT N2402_GetCtrlACPower(BYTE grpIdx);

/*=================================================================================================
** @Name      : void TaskN2402_CommFailHandle(void)
** @Input     : void
** @Output    : void
** @Function  : N2402通信故障处理
** @The notes : 
**===============================================================================================*/
void TaskN2402_CommFailHandle(void);

/*=================================================================================================
** @Name      : void ModCalcInfo_N2402(void)
** @Input     : void
** @Output    : void
** @Function  : N2402模块计算信息更新
** @The notes : 
**===============================================================================================*/
void ModCalcInfo_N2402(void);

/*=================================================================================================
** @Name      : BYTE NoSampDeal_N2402(void)
** @Input     : void
** @Output    : 采样状态
** @Function  : N2402无采样处理
** @The notes : 
**===============================================================================================*/
BYTE NoSampDeal_N2402(void);
BYTE ModFaultJudge_N2402(void);

/*=================================================================================================
** @Name      : void InitN2402EepromParam(void)
** @Input     : void
** @Output    : void
** @Function  : 初始化N2402 EEPROM参数
** @The notes : 
**===============================================================================================*/

void CANRcvMsgHandle_N2402(ROUGH_CCU_DATA* pCCU, BYTE u8RcvDev, BYTE u8Cmd, BYTE u8DevNo, BYTE u8ErrorNum, canmsg_rx *msg);
void CanRcvHandleMsg_ModuleAddr_N2402(ROUGH_CCU_DATA* pCCU, BYTE u8RcvSrcAddr, BYTE u8Cmd, canmsg_rx *msg);

#endif