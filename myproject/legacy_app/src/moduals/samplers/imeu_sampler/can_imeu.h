#ifndef __CAN_INFY_BATT_H
#define __CAN_INFY_BATT_H

#include "imeu_sampler_main.h"

#define CELL_BALANCE_SUB_IDX_START 	0x0A
#define CELL_VOLT_SUB_IDX_START 	0x32
#define CELL_TEMP_SUB_IDX_START 	0x96
#define CELL_AFE_SUB_IDX_START 		0x14
#define CELL_SAFE_SUB_IDX_START 	0x14

#ifdef IMMU3
	#define CELL_BALANCE_SUB_IDX_END 	0x13
	#define CELL_VOLT_SUB_IDX_END 		0x72  
	#define CELL_TEMP_SUB_IDX_END 		0xD6 // 2524项目 增加到 52*5 = 260个电芯
	#define CELL_AFE_SUB_IDX_END 		0x54
	#define CELL_SAFE_SUB_IDX_END  		0x1D
#else
	#define CELL_BALANCE_SUB_IDX_END 	0x13
	#define CELL_VOLT_SUB_IDX_END 		0x6D
	#define CELL_TEMP_SUB_IDX_END 		0xD1 // update
	#define CELL_AFE_SUB_IDX_END 		0x4F
	#define CELL_SAFE_SUB_IDX_END  		0x1D
#endif

#define	STBATT_YFY_POWEROFF_CMD     0
#define	STBATT_YFY_POWERON_CMD     	2

typedef union tagIFYBattLigHTStat
{
	BYTE	szByLigHTState[2];
	struct
	{
		BYTE	Chg_Stat 				: 1;
		BYTE	Dischg_Stat 			: 1;
		BYTE	SysFault 				: 1;
		BYTE	IdleStat 				: 1;
		BYTE	OffGridStat 			: 1;
        BYTE	MpptStat 				: 1;
	}Stat;

}IFY_BATT_LIGHT_STAT;


typedef enum tagstYFYBattCmd_Idx
{ 
	//S2C: S,Server-BMS  C:Client-Monitor
	_CAN_R_CMD_S2C_STATUS = 0,
	_CAN_R_CMD_S2C_LIMIT1, 
	_CAN_R_CMD_S2C_LIMIT2, 
	_CAN_R_CMD_S2C_ALARM, 
	_CAN_R_CMD_S2C_FAULT, 
	_CAN_R_CMD_S2C_GPIO,
	_CAN_R_CMD_S2C_SUMDATA1,
	_CAN_R_CMD_S2C_SUMDATA2,
	_CAN_R_CMD_S2C_SUMDATA3,
	_CAN_R_CMD_S2C_SUMDATA4,
	_CAN_R_CMD_S2C_SUMDATA5,
	_CAN_R_CMD_S2C_SUMDATA7,
	_CAN_R_CMD_S2C_SUMDATA8,
	_CAN_R_CMD_S2C_STATISTICS1,
	_CAN_R_CMD_S2C_STATISTICS2,
	_CAN_R_CMD_S2C_STATISTICS3,
	_CAN_R_CMD_S2C_STATISTICS4,
	_CAN_R_CMD_S2C_STATISTICS5,
	_CAN_R_CMD_S2C_STATISTICS6,
	_CAN_R_CMD_S2C_DIAGNOSIS,
	_CAN_R_CMD_S2C_CELLDATA1,
	_CAN_R_CMD_S2C_CELLDATA8,
	_CAN_R_CMD_S2C_CELLDATA2,
	_CAN_R_CMD_S2C_CELLDATA3,
	_CAN_R_CMD_S2C_CELL_BARCODE_QUERY,
	_CAN_R_CMD_S2C_PACK_BARCODE_QUERY,
	_CAN_R_CMD_S2C_BSU_BARCODE_QUERY,
	_CAN_R_CMD_S2C_Permission_BARCODE,
	_CAN_R_CMD_S2C_CELL_BARCODE_WRITE,
	_CAN_R_CMD_S2C_PACK_BARCODE_WRITE,
	_CAN_R_CMD_S2C_BSU_BARCODE_WRITE,
	_CAN_R_CMD_S2C_CELLAFEWORKMODE,
	_CAN_R_CMD_S2C_CELLDATA6,//XSY250417
	_CAN_R_CMD_S2C_DEVINFO,
	_CAN_R_CMD_S2C_CFGINFO,
	_CAN_R_CMD_S2C_ISOLATRES,
	_CAN_R_CMD_S2C_AUTH,
	
	_CAN_R_CMD_S2C_RECORDNUM,

	_CAN_R_CMD_S2C_FAULTREC,
	_CAN_R_CMD_S2C_LOGREC,
	_CAN_R_CMD_S2C_WAVEREC,
	_CAN_R_CMD_S2C_DODREC,
	_CAN_R_CMD_S2C_INNERRES_1,
	_CAN_R_CMD_S2C_INNERRES_2,
	_CAN_R_CMD_S2C_ZR_INNERRES,
	_CAN_R_CMD_S2C_ZI_INNERRES,

	_CAN_R_CMD_C2S_UPGRADESTATU,

	
	_CAN_R_CMD_S2C_AIRINFO1,  //add by lpl 240805
	_CAN_R_CMD_S2C_AIRINFO2,
	_CAN_R_CMD_S2C_AIRINFO3,

	_CAN_R_CMD_Parse_S2C_PACKINFO1,
	_CAN_R_CMD_Parse_S2C_PACKINFO2,
	_CAN_R_CMD_Parse_S2C_PACKINFO3,

	_CAN_R_CMD_Parse_S2C_TFAIRPARAINFO,
	_CAN_R_CMD_Parse_S2C_TFAIRSTAINFO,
	_CAN_R_CMD_Parse_S2C_S2C_TFAIRERRINFO,

	_CAN_R_CMD_Parse_S2C_LEAKAGEDATA,//xsy

	_CAN_R_CMD_Parse_S2C_LONGALARM,//edit by Kuangyue 20241202

	_CAN_R_CMD_Parse_S2C_IMEU3AIRCONTROL,//edit by kuangyue 20250811

	_CAN_R_CMD_Parse_S2C_IMEU3AIR,//edit by kuangyue 20250217
	
	_CAN_R_CMD_Parse_S2C_XINMIAOAIR1,

	_CAN_R_CMD_Parse_S2C_XINMIAOAIR2,
	
	_CAN_R_CMD_Parse_S2C_AFEINFO,

	// LJH240715 aircond
	_CAN_R_CMD_S2C_AIRCOND_RD,
	_CAN_R_CMD_S2C_AIRCOND_SET,

	_MAX_YFY_CAN_CMD,
}ID_CMD_YFY_BATT;

typedef struct tagA_YFY_BattFrame
{
	BYTE 			byRawData[CAN_FRAME_LEN];
	
	/*CAN Protocal Head Info,Total 29 bits,extend frame*/ 
	//BYTE			byErrCode;
 	//BYTE			byResv1;
	//BYTE			byCmdType;

	ID_CMD_YFY_BATT emMsgType;
 	BYTE  			byFuncCode;
	BYTE  			bySubIndex;
    BYTE    		byBMSGrpAddr;   //0x32~
	UINT    		nBMSGrpMapIdx;  //0x0~
	BYTE 			byEMSAddr;

	UINT 			uiFrmID; 	//Send CAN ID
	UINT 			uiDataLen; 	//Send DLC
	BYTE   	 		byData[CAN_DATA_AREA_LEN+1];      /* Data Area */

	BOOL    		bIsCtrlCmd;     /* if true, need send urgently */
	// LJH240715 aircond to push data
	IMEU_SAMPLER_ROUGH *pIMEU_Rough;
}A_CAN_YFY_BATT_FRAME;

typedef BOOL (*CAN_YFY_IMEU_Sample_PROC)(A_CAN_YFY_BATT_FRAME *pFrmReply);

typedef struct tagYFYBattPro
{
	ID_CMD_YFY_BATT emMsgType;
	int  			nFuncCode;
	int  			nSubIndex;

	CAN_YFY_IMEU_Sample_PROC pfnCmdHandler;

}TAG_YFY_BATT_PRO;

void YFY_ReadAndParseCanData(IMEU_SAMPLER_ROUGH *pIMEU_Rough);
BYTE getIMEU3Addr(int iDevIdx);


extern void Cal_Crc32Init(uint32_t *curCrc);
extern void Cal_ComputeCrc32(uint32_t *curCrc, const uint8_t *buf, const uint32_t size);
extern void Cal_CrcFinalize(uint32_t *curCrc);

extern uint8_t Crc_ComputeCRC8(const uint16_t Msg_Id, const uint8_t* Crc_DataPtr, uint16_t Crc_Length);
#endif



