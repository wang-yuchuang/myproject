#ifndef __GH_DIGITAL_ENERGY_H
#define __GH_DIGITAL_ENERGY_H

#include "std_sys.h"
#include "app_incs.h"

#define   GH_SUBTOPIC_NUM 3

#define   BEIJING_TIMEZONE_OFFSET_HOURS  8*60*60 //8 hours
#define   CMD_SEND_TIME  60

typedef enum tag_GH_mqtt_type
{
    GH_MQTT_TYPE_ARR = 0,		
    GH_MQTT_TYPE_CLU,
    GH_MQTT_TYPE_EMU,			
    GH_MQTT_TYPE_PCS,
    GH_MQTT_TYPE_FIRE,					
    GH_MQTT_TYPE_LIQCOOL,		
    GH_MQTT_TYPE_DRIER,			
    GH_MQTT_TYPE_METER,			
    GH_MQTT_TYPE_CELL,

    GH_STATUS_TYPE_ARR,		
    GH_STATUS_TYPE_CLU,
    GH_STATUS_TYPE_EMU,	
    GH_STATUS_TYPE_LIQCOOL,		
    GH_STATUS_TYPE_PCS,
    GH_STATUS_TYPE_FIRE,						
    GH_STATUS_TYPE_DRIER,			
    GH_STATUS_TYPE_METER,	

    GH_CMD_TYPE_POWERCMD,
    GH_CMD_TYPE_DISPATCHPOWER,
    GH_CMD_TYPE_POWERLIMIT,

    MAX_GH_MQTT_TYPE,	
}GH_MQTT_TYPE;

typedef enum tag_GH_MQTTMsgSrc
{
	GH_MQTT_MSG_C2S = 0, //from CS
	GH_MQTT_MSG_S2C, //from CP

}GH_MQTT_MSG_SRC;

typedef int (*MSG_GH_HANDLER)(void* pArg, int index);

typedef struct tag_GH_MQTTMsgInfo
{
	GH_MQTT_TYPE		emMsgType;
	GH_MQTT_MSG_SRC		emMsgSrc;
	char                szPubTopic[64];
	MSG_GH_HANDLER		pfnMsgHandler;
	time_t				tm_LastSend;
	time_t				tmLastArrived;
}GH_MQTT_INFO;


typedef enum tagGH_SetCmdType
{
    GH_SetCmdType_PowerCmd = 0, //power command
    GH_SetCmdType_DispatchPower, //dispatch power
    GH_SetCmdType_PowerLimit, //power limit
    MAX_GH_SETCMD,
}GH_SETCMD_TYPE;

typedef int (*MSG_GHSETCMD_HANDLER)(void* pArg, cJSON *json);

typedef struct tag_GH_SetCmd_Type
{
    GH_SETCMD_TYPE              emSetCmdType;
    char                        szSubTopic[64];
    MSG_GHSETCMD_HANDLER        pfnMsgHandler;
}GH_SETCMD_INFO;

//遥测
typedef struct tagArrArray
{
    UINT                 arr1[17];          
    float                arr2[26];
    time_t               Ts1;
}ARR_ARRAY;
typedef struct tagArrData
{
    UINT                 arrno;
    UINT                 arrStatus;
    UINT                 bmsStatus;
    UINT                 cluSums;
    UINT                 res_1;
    UINT                 cluMaxSocNum;
    UINT                 cluMinSocNum;
    UINT                 cluMaxVolNum;
    UINT                 cluMinVolNum;
    UINT                 maxCellVolCluNum;
    UINT                 maxCellVolCellNum;
    UINT                 minCellVolCluNum;
    UINT                 minCellVolCellNum;
    UINT                 maxCellTemCluNum;
    UINT                 maxCellTemCellNum;
    UINT                 minCellTemCluNum;
    UINT                 minCellTemCellNum;
    float                arrVol;
    float                arrCur;
    float                arrSOC;
    float                arrSOH;
    float                arrRechaElec;
    float                arrRedischgElec;
    float                arrLastRechaElec;
    float                arrLastRedischgElec;
    float                arrSumsChaElec;
    float                arrSumsDischgElec;
    float                res_2;
    float                maxCellVol;
    float                minCellVol;
    float                res_3;
    float                maxCellTem;
    float                minCellTem;
    float                res_4;
    float                cluMaxSoc;
    float                cluMinSoc;
    float                res_5;
    float                cluMaxVol;
    float                cluMinVol;
    float                arrMaxReChaPower;
    float                arrMaxReDischgPower;
    float                arrMaxReChaCur;
    float                arrMaxRedischgCur;
    time_t               Ts;
}ARR_DATA;

typedef union tagARR_Union
{
    ARR_ARRAY   arrArray;    
    ARR_DATA    arrData;
}ARR_UNION;


typedef struct tagCluArray
{
    UINT                 Clu1[2];
    float                Clu2[2];
    UINT                 Clu3[2];
    float                Clu4[12];
    UINT                 Clu5[4];
    float                Clu6[7];
    UINT                 Clu7[2];
    float                Clu8[7];
    time_t               Ts1;
}CLU_ARRAY;

typedef struct tagCluData
{
    UINT                     arrno;
    UINT                     cluno;
    float                    cluSoc;
    float                    cluSoh;
    UINT                     cluStatus;
    UINT                     bmsStatus;
    float                    cluVol;
    float                    cluCur;;
    float                    maxCellVol;
    float                    minCellVol;
    float                    maxCellTem;
    float                    minCellTem;
    float                    maxCellSOC;
    float                    minCellSOC;
    float                    maxCellSOH;
    float                    minCellSOH;
    float                    cluPosres;
    float                    cluNegres;
    UINT                     maxCellVolCellNum;
    UINT                     minCellVolCellNum;
    UINT                     maxCellTemCellNum;
    UINT                     minCellTemCellNum;
    float                    cellVolTotalSum;
    float                    maxDischgArrCur;
    float                    res_1;
    float                    reqDischgArrCur;
    float                    res_2;
    float                    res_3;
    float                    res_4;
    UINT                     res_5;
    UINT                     res_6;
    float                    cluRtPower;
    float                    cluSumsChaElec;
    float                    cluSumsDischgElec;
    float                    lastSumsChaElec;
    float                    lastSumsDischgElec;
    float                    cluRechaElec;
    float                    cluRedischgElec;
    time_t                   Ts;
}CLU_DATA;

typedef union tagCLU_Union
{
    CLU_ARRAY   cluArray;    
    CLU_DATA    cluData;
}CLU_UNION;

typedef struct tagEmuArray
{
    float            arr1[6];
    UINT             batteryStatus;
    float            arr2[7]; 
    UINT             sysStatus;  
    float            arr3[3];
    UINT             arr4[3]; 
    time_t           Ts1;       
}EMU_ARRAY;

typedef struct tagEmuData
{
    float                    runStatus;
    float                    pcsSumsActivePower;
    float                    pcsSumsReactivePower;
    float                    dcBusVol;
    float                    dcBusVur;
    float                    sumsSOC;
    UINT                     batteryStatus;
    float                    cellSums;
    float                    batteryAverageTem;
    float                    batteryMaxTemDif;
    float                    maxCellVol;
    float                    minCellVol;
    float                    ambientTem;
    float                    ambientHumidity;
    UINT                     sysStatus;
    float                    workMode;
    float                    maxAllowChargPower;
    float                    minAllowChargPower;
    UINT                     FaultCode;
    UINT                     ControlSrc;
    UINT                     RemoErr;
    time_t                   Ts;
}EMU_DATA;

typedef union tagEMU_Union
{
    EMU_ARRAY   emuArray;    
    EMU_DATA    emuData;
}EMU_UNION;

typedef struct tagPCSArray
{
    UINT                    PCS1[5];
    float                   PCS2[31];
    UINT                    Heart_Beat1;
    time_t                  Ts1;
}PCS_ARRAY;
typedef struct tagPCS
{
    UINT                     ArrNum;
    UINT                     CluNum;
    UINT                     Devno;
    UINT                     Run_Sta;
    UINT                     Offgrd_Sta;
    float                    Out_Uw_Vol;
    float                    Out_Vw_Vol;
    float                    Out_Wu_Vol;
    float                    Out_U_Cur;
    float                    Out_V_Cur;
    float                    Out_W_Cur;
    float                    Grd_F;
    float                    Ac_U_Pow_P;
    float                    Ac_V_Pow_P;
    float                    Ac_W_Pow_P;
    float                    Out_Pow_P;
    float                    Ac_U_Pow_Q;
    float                    Ac_V_Pow_Q;
    float                    Ac_W_Pow_Q;
    float                    Out_Pow_Q;
    float                    Ac_U_Pow_S;
    float                    Ac_V_Pow_S;
    float                    Ac_W_Pow_S;
    float                    Out_Pow_S;
    float                    Ac_U_Pow_Pf;
    float                    Ac_V_Pow_Pf;
    float                    Ac_W_Pow_Pf;
    float                    Out_Pow_Pf;
    float                    Power_Input;
    float                    Vol_Input;
    float                    Cur_Input;
    float                    Disstemp;
    float                    Ac_Acc_Chg_Cur;
    float                    Ac_Acc_Dis_Cur;
    float                    Dc_Acc_Chg_Cur;
    float                    Dc_Acc_Dis_Cur;
    UINT                     Heart_Beat;
    time_t                   Ts;
}PCS_DATA;

typedef union tagPCS_Union
{
    PCS_ARRAY   pcsArray;    
    PCS_DATA    pcsData;
}PCS_UNION;

typedef struct tagFireArray
{
    UINT                   fire[10];
    time_t                 Ts1;
}FIRE_ARRAY;

typedef struct tagFIREData
{
    UINT                    ArrNum;
    UINT                    CluNum;
    UINT                    Devno;
    UINT                    Batpacknum;
    UINT                    Sysprealarmlevel;
    UINT                    Sysfault;
    UINT                    Starttype;
    UINT                    Co_Concentration;
    UINT                    Fire_Temp;
    UINT                    Bies_Cnt;
    time_t                    Ts;
}FIRE_DATA;

typedef union tagFIRE_Union
{
    FIRE_ARRAY   fireArray;    
    FIRE_DATA    fireData;
}FIRE_UNION;

typedef struct tagLiqcoolArray
{
    UINT                 Liqcool1[9]; 
    float                Liqcool2[7];
    UINT                 Liqcool3[4];
    float                Press_Rev;
    UINT                 Board_Information;
    UINT                 Ts1;
}LIQCOOL_ARRARY;
typedef struct tagLiqcoolData
{
    UINT                ArrNum;
    UINT                CluNum;
    UINT                Devno;
    UINT                Cur_Run_State;
    UINT                Relay_Out0;
    UINT                Relay_Out2;
    UINT                Relay_Out3;
    UINT                Relay_Out5;
    UINT                Relay_Out6;
    float               Hot_Water_Tmp;
    float               Inlet_Water_Tmp;
    float               System1_Steam_In;
    float               System1_Steam_Out;
    float               System1_Condensation;
    float               Hydraulic_Pressure;
    float               Liquid_Pressure;
    UINT                Information_Mark0;
    UINT                Information_Mark1;
    UINT                Information_Mark2;
    UINT                Information_Mark3;
    float               Press_Rev;
    UINT                Board_Information;
    UINT                Ts;
}LIQCOOL_DATA;

typedef union tagLIQCOOL_Union
{
    LIQCOOL_ARRARY   liqcoolArray;    
    LIQCOOL_DATA     liqcoolData;
}LIQCOOL_UNION;

typedef struct tagDrierArray
{
    UINT            Drier1[3];
    float           Pow_Code;
    UINT            Drier2[2];
    float           Drier3[2];
    UINT            Drier4[5];
    time_t          Ts1;
}DRIER_ARRAY;
typedef struct tagDrierData
{
    UINT            ArrNum;
    UINT            CluNum;
    UINT            Devno;
    float           Pow_Code;
    UINT            Env_Tem;
    UINT            Env_Wet;
    float           Cool_Tem;
    float           Start_Wet;
    UINT            Tmpwet_Sta;
    UINT            Cool_Tem_Sta;
    UINT            Cool_Sta;
    UINT            Out_Status;
    UINT            Out_Sta;
    time_t            Ts;
}DRIRE_DATA;

typedef union tagDRIER_Union
{
    DRIER_ARRAY   drierArray;    
    DRIRE_DATA    drierData;
}DRIER_UNION;

typedef struct tagMeterArray
{
    UINT            Meter1[3];
    float           Meter2[10];
    UINT            Meter3[2];
    float           Meter4[24];
    time_t          Ts1;
}METER_ARRAY;

typedef struct tagMeterData
{
    UINT             ArrNum;
    UINT             CluNum;
    UINT             Devno;
    float            Meter_Add_Pos;
    float            Meter_Ver_Pos;
    float            Meter_Hig_Pos;
    float            Meter_Mid_Pos;
    float            Meter_Low_Pos;
    float            Meter_Add_Neg;
    float            Meter_Ver_Neg;
    float            Meter_Hig_Neg;
    float            Meter_Mid_Neg;
    float            Meter_Low_Neg;
    UINT             Pt;
    UINT             Ct;
    float            Meter_A_Vol;
    float            Meter_B_Vol;
    float            Meter_C_Vol;
    float            Meter_A_Cur;
    float            Meter_B_Cur;
    float            Meter_C_Cur;
    float            Meter_F;
    float            Meter_Tot_P;
    float            Meter_Tot_Q;
    float            Meter_Tot_Pf;
    float            Meter_All_Pow;
    float            Meter_A_Pow;
    float            Meter_B_Pow;
    float            Meter_C_Pow;
    float            Meter_All_Qre;
    float            Meter_A_Qre;
    float            Meter_B_Qre;
    float            Meter_C_Qre;
    float            Meter_Add_P;
    float            Meter_Add_Q;
    float            res_1;
    float            res_2;
    float            Meter_1Day_Pos_Eng;
    float            Meter_1Day_Neg_Eng;
    time_t           Ts;            
}METER_DATA;

typedef union tagMETER_Union
{
    METER_ARRAY   meterArray;    
    METER_DATA    meterData;
}METER_UNION;

typedef struct tagCellArray
{
    UINT            Meter1[2];
    float           Meter2[4];
    time_t          Ts1;
}CELL_ARRAY;
typedef struct tagCellData
{
    UINT             ArrNo;
    UINT             CluNo;
    float            CelVol;
    float            CelTem;
    float            CelSOC;
    float            CelSOH;
    time_t           Ts;
}CETT_DATA;


//遥信

typedef struct tagStatusArray
{
    UINT        arr1[78];
    time_t      Ts1;
}ARR_STATUS_ARRAY;

typedef struct tagStatusArr
{
    UINT        arrno;
    UINT        arrTswitchStatus;
    UINT        cluVolLowWarnL1;
    UINT        cluVolLowWarnL2;
    UINT        cluVolLowWarnL3;
    UINT        cluVolUpWarnL1;
    UINT        cluVolUpWarnL2;
    UINT        cluVolUpWarnL3;
    UINT        cluCurUpWarnL1;
    UINT        cluCurUpWarnL2;
    UINT        cluCurUpWarnL3;
    UINT        cluRegLowWarnL1;
    UINT        cluRegLowWarnL2;
    UINT        cluRegLowWarnL3;
    UINT        devModuleTemLowWarnL1;
    UINT        devModuleTemLowWarnL2;
    UINT        devModuleTemLowWarnL3;
    UINT        devModuleTemUpWarnL1;
    UINT        devModuleTemUpWarnL2;
    UINT        devModuleTemUpWarnL3;
    UINT        cellVolUpWarnL1;
    UINT        cellVolUpWarnL2;
    UINT        cellVolUpWarnL3;
    UINT        cellVolLowWarnL1;
    UINT        cellVolLowWarnL2;
    UINT        cellVolLowWarnL3;
    UINT        cellVolDifWarnL1;
    UINT        cellVolDifWarnL2;
    UINT        cellVolDifWarnL3;
    UINT        cellTemLowWarnL1;
    UINT        cellTemLowWarnL2;
    UINT        cellTemLowWarnL3;
    UINT        cellTemUpWarnL1;
    UINT        cellTemUpWarnL2;
    UINT        cellTemUpWarnL3;
    UINT        cellTemDifWarnL1;
    UINT        cellTemDifWarnL2;
    UINT        cellTemDifWarnL3;
    UINT        cluVolDifWarnL1;
    UINT        cluVolDifWarnL2;
    UINT        cluVolDifWarnL3;
    UINT        cluCurDifWarnL1;
    UINT        cluCurDifWarnL2;
    UINT        cluCurDifWarnL3;
    UINT        arrPosresWarnL1;
    UINT        arrPosresWarnL2;
    UINT        arrPosresWarnL3;
    UINT        arrNegresWarnL1;
    UINT        arrNegresWarnL2;
    UINT        arrNegresWarnL3;
    UINT        cellSocLowWarnL1;
    UINT        cellSocLowWarnL2;
    UINT        cellSocLowWarnL3;
    UINT        cellSocUpWarnL1;
    UINT        cellSocUpWarnL2;
    UINT        cellSocUpWarnL3;
    UINT        cellSocDifWarnL1;
    UINT        cellSocDifWarnL2;
    UINT        cellSocDifWarnL3;
    UINT        cellSohLowWarnL1;
    UINT        cellSohLowWarnL2;
    UINT        cellSohLowWarnL3;;
    UINT        cellSohUpWarnL1;
    UINT        cellSohUpWarnL2;
    UINT        cellSohUpWarnL3;
    UINT        bcmComWarn;
    UINT        bmuComWarn;
    UINT        pcsComWarn;
    UINT        emergencyStop;
    UINT        upsAcWarn;
    UINT        cluVolAbnDif;
    UINT        arrAbnContactorOpening;
    UINT        arrAbnContactorClosing;
    UINT        chaDisable;
    UINT        dischgDisable;
    UINT        bmsWarn;
    UINT        bmsFault;
    UINT        eclondComFault;
    time_t        Ts;
}ARR_STATUS;

typedef union tagArrStatus_Union
{
    ARR_STATUS_ARRAY   arrArray;    
    ARR_STATUS         arrStatus;
}ARR_STATUS_UNION;

typedef struct tagStatusCluArray
{
    UINT         clu1[39];
    time_t       Ts1;
}CLU_STATUS_ARRAY;

typedef struct tagStatusClu
{
    UINT         arrno;
    UINT         cluno;
    UINT         cluPosresWarnLevel;
    UINT         cluNegresWarnLevel;
    UINT         cluSelftestFault;
    UINT         cluVolAbn;
    UINT         cluCurSensorFault;
    UINT         esmuComWarn;
    UINT         bmuComWarn;
    UINT         cluSocLowWarnLevel;
    UINT         cluSohLowWarnLevel;
    UINT         cluVolUpWarnLevel;
    UINT         cluVolLowWarnLevel;
    UINT         cluChgCurUpWarnLevel;
    UINT         cluDischgCurUpWarnLevel;
    UINT         cellVolUpWarnLevel;
    UINT         cellVolLowWarnLevel;
    UINT         cellChgTemUpWarnLevel;
    UINT         cellChgTemLowWarnLevel;
    UINT         cellDischgTemUpWarnLevel;
    UINT         cellDischgTemLowWarnLevel;
    UINT         cellTemUpWarnLevel;
    UINT         cellTemLowWarnLevel;
    UINT         cellSocLowWarnLevel;
    UINT         cellSocUpWarnLevel;
    UINT         cellSohLowWarnLevel;
    UINT         cellVolDifWarnLevel;
    UINT         cellTemDifWarnLevel;
    UINT         cluPositiveRelaySt;
    UINT         cluNegativeRelaySt;
    UINT         cluPrePos;
    UINT         cluMaintenanceSwitchSt;
    UINT         cluFuseStatus;
    UINT         deviceLoking;
    UINT         deviceWarn;
    UINT         chaDisable;
    UINT         dischgDisable;
    UINT         bmsWarn;
    UINT         bmsFault;
    time_t         Ts;
}CLU_STATUS;

typedef union tagCluStatus_Union
{
    CLU_STATUS_ARRAY   cluArray;    
    CLU_STATUS  cluStatus;
}CLU_STATUS_UNION;

typedef struct tagStatusEMUArray
{
    UINT         emu1[21];
    time_t       Ts1;
}EMU_STATUS_ARRAY;

typedef struct tagStatusEMU
{
    UINT        pcsFault;
    UINT        sysFault;
    UINT        res_1;
    UINT        res_2;
    UINT        airFault;
    UINT        fireFeedBack;
    UINT        doorOpenStatus;
    UINT        spdFault;
    UINT        ydnwdtFault;
    UINT        upsFault;
    UINT        bmsqfFeedback;
    UINT        bmsqfOut;
    UINT        remoteComNoLink;
    UINT        liqcoolComFault;
    UINT        drierComFault;
    UINT        pcsComFault;
    UINT        bmsComFault;
    UINT        airComFault;
    UINT        backsetDeviceComFault;
    UINT        meterDeviceComFault;
    UINT        STOPFault;
    time_t        Ts;
}EMU_STATUS;

typedef union tagEMUStatus_Union
{
    EMU_STATUS_ARRAY   emuArray;    
    EMU_STATUS         emuStatus;
}EMU_STATUS_UNION;

typedef struct tagStatusLiqcoolArray
{
    UINT         Liqcool1[45];
    time_t       Ts1;
}LIQCOOL_STATUS_ARRAY;

typedef struct tagStatusLiqcool
{
    UINT           ArrNum;
    UINT           CluNum;
    UINT           Devno;
    UINT           Alarm_Sw_In0;
    UINT           Alarm_Sw_In2;
    UINT           Alarm_Sw_In5;
    UINT           Alarm_Sw_In6;
    UINT           Alarm_Sw_In8;
    UINT           Alarm_Sw_In9;
    UINT           Alarm_Sw_In10;
    UINT           Alarm_Sw_In12;
    UINT           Alarm_Sw_In13;
    UINT           Alarm_Sw_In14;
    UINT           Alarm1_1;
    UINT           Alarm1_2;
    UINT           Alarm1_5;
    UINT           Alarm1_6;
    UINT           Alarm1_7;
    UINT           Alarm1_8;
    UINT           Alarm1_9;
    UINT           Alarm1_10;
    UINT           Alarm1_12;
    UINT           Alarm1_14;
    UINT           Alarm2_0;
    UINT           Alarm2_1;
    UINT           Alarm2_2;
    UINT           Alarm2_4;
    UINT           Alarm2_6;
    UINT           Alarm2_8;
    UINT           Alarm2_10;
    UINT           Alarm2_12;
    UINT           Alarm3_0;
    UINT           Alarm3_1;
    UINT           Alarm3_4;
    UINT           Alarm3_6;
    UINT           Alarm3_7;
    UINT           Alarm3_12;
    UINT           Alarm3_13;
    UINT           Alarm8;
    UINT           Alarm9;
    UINT           Alarm_Mark;
    UINT           Comm_Err;
    UINT           Err1;
    UINT           Err2;
    UINT           Err3;
    time_t           Ts;
}LIQCOOL_STATUS;

typedef union tagLIQCOOLStatus_Union
{
    LIQCOOL_STATUS_ARRAY   liqcoolArray;    
    LIQCOOL_STATUS         liqcoolStatus;
}LIQCOOL_STATUS_UNION;


typedef struct tagStatusPCSArray
{
    UINT         pcs1[62];
    time_t       Ts1;
}PCS_STATUS_ARRAY; 

typedef struct tagStatusPCS
{
    UINT          ArrNum;
    UINT          CluNum;
    UINT          Devno;
    UINT          Alarm_0;
    UINT          Alarm_1;
    UINT          Alarm_2;
    UINT          Alarm_3;
    UINT          Alarm_4;
    UINT          Alarm_5;
    UINT          Alarm_6;
    UINT          Alarm_7;
    UINT          Alarm_8;
    UINT          Alarm_9;
    UINT          Alarm_10;
    UINT          Alarm_11;
    UINT          Alarm_12;
    UINT          Alarm_13;
    UINT          Alarm_14;
    UINT          Alarm_15;
    UINT          Alarm_16;
    UINT          Alarm_17;
    UINT          Alarm_18;
    UINT          Alarm_19;
    UINT          Alarm_20;
    UINT          Alarm_21;
    UINT          Alarm_22;
    UINT          Alarm_23;
    UINT          Alarm_24;
    UINT          Alarm_25;
    UINT          Alarm_26;
    UINT          Alarm_27;
    UINT          Alarm_28;
    UINT          Alarm_29;
    UINT          Alarm_30;
    UINT          Alarm_31;
    UINT          Alarm_32;
    UINT          Alarm_33;
    UINT          Alarm_34;
    UINT          Alarm_35;
    UINT          Alarm_36;
    UINT          Alarm_37;
    UINT          Alarm_38;
    UINT          Alarm_39;
    UINT          Alarm_40;
    UINT          Alarm_41;
    UINT          Alarm_42;
    UINT          Alarm_43;
    UINT          Alarm_44;
    UINT          Alarm_45;
    UINT          Alarm_46;
    UINT          Alarm_47;
    UINT          Alarm_48;
    UINT          Alarm_49;
    UINT          Alarm_50;
    UINT          Alarm_51;
    UINT          Alarm_52;
    UINT          Alarm_53;
    UINT          Alarm_54;
    UINT          Comm_Err;
    UINT          Err1;
    UINT          Err2;
    UINT          Err3;
    time_t          Ts;
}PCS_STATUS;

typedef union tagPCSStatus_Union
{
    PCS_STATUS_ARRAY     pcsArray;    
    PCS_STATUS           pcsStatus;
}PCS_STATUS_UNION;

typedef struct tagFireStatusArray
{
    UINT         fire1[19];
    time_t       Ts1;
}FIRE_STATUS_ARRAY;

typedef struct tagStatusfire
{
    UINT        ArrNum;
    UINT        CluNum;
    UINT        Devno;
    UINT        Comm_Err;
    UINT        Err1;
    UINT        Err2;
    UINT        Err3;
    UINT        res_1;
    UINT        res_2;
    UINT        res_3;
    UINT        res_4;
    UINT        res_5;
    UINT        res_6;
    UINT        res_7;
    UINT        res_8;
    UINT        res_9;
    UINT        res_10;
    UINT        res_11;
    UINT        res_12;
    time_t        Ts;
}FIRE_STATUS;

typedef union tagFireStatus_Union
{
    FIRE_STATUS_ARRAY   fireArray;    
    FIRE_STATUS         fireStatus;
}FIRE_STATUS_UNION;

typedef struct tagStatusDrierArray
{
    UINT         Drier1[7];
    time_t       Ts1;
}DRIER_STATUS_ARRAY;

typedef struct tagStatusDrier
{
    UINT          ArrNum;
    UINT          CluNum;
    UINT          Devno;
    UINT          Comm_Err;
    UINT          Err1;
    UINT          Err2;
    UINT          Err3;
    time_t          Ts;
}DRIER_STATUS;

typedef union tagDrierStatus_Union
{
    DRIER_STATUS_ARRAY   drierArray;    
    DRIER_STATUS         drierStatus;
}DRIER_STATUS_UNION;

typedef struct tagStatusMeterArray
{
    UINT         Meter1[7];
    time_t       Ts1;
}METER_STATUS_ARRAY;

typedef struct tagStatusMeter
{
    UINT          ArrNum;
    UINT          CluNum;
    UINT          Devno;
    UINT          Comm_Err;
    UINT          Err1;
    UINT          Err2;
    UINT          Err3;
    time_t          Ts; 
}METER_STATUS;

typedef union tagMeterStatus_Union
{
    METER_STATUS_ARRAY   meterArray;    
    METER_STATUS         meterStatus;
}METER_STATUS_UNION;

#endif