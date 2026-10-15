#ifndef __PV_INVERTER_MAIN_H__
#define __PV_INVERTER_MAIN_H__

#include "../sampler_base.h"

#define ALL_PV_COMM_FAIL_REOPEN_CNT   10
#define MAX_PHY_PORT_NUM              6  // 总共6个物理端口

typedef enum tagENU_INVERTER_TYPE
{
    INVERTER_TYPE_NOT_INSTALL = 0,
    INVERTER_TYPE_Growatt,
    INVERTER_TYPE_Huawei,
    INVERTER_TYPE_Ginlong,
    INVERTER_TYPE_Sungrow,

    MAX_INVERTER_TYPE_NUM

}ENU_INVERTER_TYPE;

typedef enum tagENU_INVERTER_BAUD_RATE
{
    BAUD_RATE_AUTO = 0,
    BAUD_RATE_4800 = 2,
    BAUD_RATE_9600,
    BAUD_RATE_115200,


    MAX_BAUD_RATE
}ENU_INVERTER_BAUD_RATE;

typedef enum tagENU_PV_CMD
{
    PV_CMD_PowerOff = 0,
    PV_CMD_PowerOn,
    PV_CMD_MaxActivePowerRate,
    PV_CMD_AntiFlux_Enable,
    PV_CMD_ActiveOutputPower,
    PV_CMD_ActiveOutputPower_NoLimit,
    PV_CMD_ReActiveOutputPower,
    PV_CMD_ReActiveOutputPower_NoLimit,
    PV_CMD_TIMESYSCTR,
    PV_CMD_ActivePowerSlope,
    PV_CMD_PoweroffSave,

    MAX_PV_CMD
}ENU_PV_CMD;

typedef enum tagENU_PV_PHY_INTERFACE
{
    PHY_RS485_1 = 0,
	PHY_RS485_2,
	PHY_UART_SERVER,
	PHY_UART_SERVER_2,
	PHY_UART_SERVER_3,
	PHY_UART_SERVER_4,

    MAX_PHY
}ENU_PV_PHY;

typedef struct __PV_PARAM
{
    // void*                   pInCOMPort;
    // void*                   hInPort;
    ENU_INVERTER_TYPE       eInvertType;
    ENU_INVERTER_BAUD_RATE  eBaudRate;
    int                     iAddr;
    int                     iRatedPower;
    UINT                    nGroupNum;
    PHY_INTER_TYPE          ePhyInterface;  // 该逆变器使用的物理接口

}PV_PARAM;


typedef struct tagPvRunData
{
    BOOL                        bIsCommFail;
    A_BLOB_PV_INVERTER_DATA     blobSinglePVdata;
    
}PV_RUNNING_DATA;

typedef enum tagPV_DATA_ACQUISITION_TYPES
{
    PV_DATA_ACQUISITION_NOT_INSTALLED = 0,
    PV_DATA_ACQUISITION_DO_SWITCH,
    PV_DATA_ACQUISITION_GROWATT,
    PV_DATA_ACQUISITION_HUAWEI,
    PV_DATA_ACQUISITION_GINLANG,
    PV_DATA_ACQUISITION_SUNGROW,

    PV_DATA_ACQUISITION_MAX
}PV_SATA_ACQUISITON_TYPES;

// 前置声明
struct stPvRoughData;

// 端口线程信息结构体
typedef struct tagPortThreadInfo
{
    PHY_INTER_TYPE          ePortType;          // 端口类型
    BOOL                    bIsOpened;          // 是否已打开
    HANDLE                  hPort;              // 端口句柄
    COMMPORT*               pCommPort;          // 通信端口对象
    char                    szPortConfig[64];   // 端口配置字符串
    
    pthread_t               threadId;           // 线程ID
    BOOL                    bThreadRunning;     // 线程运行标志
    BOOL                    bThreadShouldExit;  // 线程退出标志
    pthread_mutex_t         dataMutex;          // 数据互斥锁
    
    struct stPvRoughData*   pPvRoughData;       // 指向主数据结构
    
    UINT                    uSampleCount;       // 采样计数
    UINT                    uCommFailCount;     // 通信失败计数
    
} PORT_THREAD_INFO;

// 端口信息结构体
typedef struct tagPortInfo
{
    PHY_INTER_TYPE      ePortType;          // 端口类型
    BOOL                bIsOpened;          // 是否已打开
    HANDLE              hPort;              // 端口句柄
    COMMPORT*           pCommPort;          // 通信端口对象
    char                szPortConfig[64];   // 端口配置字符串
    pthread_t           threadId;           // 线程ID
    BOOL                bThreadRunning;     // 线程运行标志
    
} PORT_INFO;

// 串口服务器配置
typedef struct tagUartServerConfig
{
    char    szIP[64];           // IP地址
    UINT    iStartPort;         // 起始端口号
    
} UART_SERVER_CONFIG;

typedef struct stPvRoughData
{
	HANDLE		                    hSampler; //pointer to sampler
	HANDLE	                        hSamplerLock;  
    // HANDLE	                        hPort; //opened port handle

    // 多端口线程管理
    PORT_THREAD_INFO                portThreadInfo[MAX_PHY_PORT_NUM];   // 6个物理端口线程信息
    UART_SERVER_CONFIG              uartServerCfg;                      // 串口服务器配置
    pthread_mutex_t                 globalDataMutex;                    // 全局数据互斥锁

    DATA_PROCESSOR*			        pDPR;
	time_t					        tmSysBoot; //system boot time
    BOOL                            bCommStatusOk;
    PV_SATA_ACQUISITON_TYPES        emPvDataAcquisitonTypes;
    BOOL                            bEnableManual;
    BOOL                            bDOSwitchStatus;
    BOOL                            bSystemExit;                         // 系统退出标志
    
    PV_PARAM                        stPVSettingParam[MAX_PVInverter_NUM];
    PV_RUNNING_DATA                 stPVRunData[MAX_PVInverter_NUM];
    A_BLOB_PV_INVERTER_DATA_TOTAL   blobPVTotalinfo;
    // A_BLOB_PV_CTRL_BY_AEMS          blobRecvAEMSCtrl;       

}ST_PV_ROUGHDATA;

typedef struct
{

    ST_PV_ROUGHDATA		*pPVInverter;


}PV_INVERTER_RUNROUGH;



int samplePVInverter(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich);
int samplePVInverterGrowatt(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich);
int samplePVInverterHuawei(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich);
int samplePVInverterGinlong(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich);
int samplePVInverterSungrow(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich);

BOOL setPVCmd(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich);
BOOL setPVCmdGrowatt(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich);
BOOL setPVCmdHuawei(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich);
BOOL setPVCmdGinlong(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich);
BOOL setPVCmdSungrow(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich);

// 端口管理函数
BOOL Open_Single_Port(ST_PV_ROUGHDATA *pPV_Rough, PHY_INTER_TYPE ePortType);
BOOL Close_Single_Port(ST_PV_ROUGHDATA *pPV_Rough, PHY_INTER_TYPE ePortType);
BOOL Open_All_Comm_Ports(ST_PV_ROUGHDATA *pPV_Rough);
BOOL Close_All_Comm_Ports(ST_PV_ROUGHDATA *pPV_Rough);
PORT_THREAD_INFO* Get_Port_By_Type(ST_PV_ROUGHDATA *pPV_Rough, PHY_INTER_TYPE ePortType);

// 线程管理函数
BOOL Start_All_Port_Threads(ST_PV_ROUGHDATA *pPV_Rough);
BOOL Stop_All_Port_Threads(ST_PV_ROUGHDATA *pPV_Rough);
void* Port_Sample_Thread(void* pParam);

#endif