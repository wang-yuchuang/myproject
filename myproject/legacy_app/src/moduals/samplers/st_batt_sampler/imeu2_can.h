#ifndef __IMEU2_CAN_H__
#define __IMEU2_CAN_H__

#include "../sampler_base.h"


typedef enum tagIMEU2_CAN_CMD_ID
{ 
	//S2C: S,Server-IMEU2  C:Client-Monitor
	_IMEU2_CAN_R_CMD_S2C_MSG1 = 0,
	_IMEU2_CAN_R_CMD_S2C_MSG2,
	_IMEU2_CAN_R_CMD_S2C_MSG3,
	_IMEU2_CAN_R_CMD_S2C_MSG4,
	_IMEU2_CAN_R_CMD_S2C_MSG6,
	_IMEU2_CAN_R_CMD_S2C_MSG11,
	_IMEU2_CAN_R_CMD_S2C_MSG12,
	_IMEU2_CAN_R_CMD_S2C_MSG13,
	

	_MAX_IMEU2_CAN_CMD

}IMEU2_CAN_CMD_ID;

typedef struct tagIMEU2RunData
{
	BOOL						bFoundReply;

}IMEU2RunData;


typedef struct tagIMEU2_CAN_FRAME
{
	BYTE 			byRawData[CAN_FRAME_LEN];
	
	IMEU2_CAN_CMD_ID    emMsgType;
	UINT 			    uiFrmID; 	//Send CAN ID
	UINT 			    uiDataLen; 	//Send DLC
 	BYTE  			    byFuncCode;
	BYTE  			    bySubIndex;
	BYTE 			    byDestAddr;
    BYTE    		    bySourceAddr; 
	BYTE				bySourceIdx;
	BYTE   	 		    byData[CAN_DATA_AREA_LEN+1];      /* Data Area */
					

	BOOL    		    bIsCtrlCmd;     /* if true, need send urgently */

}IMEU2_CAN_FRAME;



typedef struct tagIMEU2CANRoughData
{
	HANDLE						hSampler; //pointer to sampler
	int							nSamplerId; //copy from sampler
	HANDLE 						hSamplerLock;

	int                         nInterrupt_Times;
	BOOL                        bFoundReply;
	BOOL						bIsCommFail;
	
	A_BLOB_BATTCAB_IMEU2_V20	blobIMEU2_V20;

	UINT						nPT1000Num;
	
	

}IMEU2_CAN_ROUGH_DATA;


typedef BOOL (*CAN_IMEU2_SAMPLE_PROC)(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrmReply);

typedef struct tagIMEU2Pro
{
	IMEU2_CAN_CMD_ID emMsgType;
	int  			nFuncCode;
	int  			nSubIndex;
	BOOL			bFoundReply;
	CAN_IMEU2_SAMPLE_PROC pfnCmdHandler;

}TAG_IMEU2_PRO;



void initIMEU2CAN(IMEU2_CAN_ROUGH_DATA IMEU2CANRoughData[], int iNum);
BOOL judgeAndParseIMEU2MsgID(IMEU2_CAN_FRAME* pFrameReply);
void processIMEU2CANData(IMEU2_CAN_ROUGH_DATA *pIMEU2CANRoughData, IMEU2_CAN_FRAME* pFrameReply);
void IMEU2CANRawData2Rough(IMEU2_CAN_ROUGH_DATA IMEU2CANRoughData[], int iNum);







BOOL Parse_S2C_MSG1(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_MSG2(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_MSG3(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_MSG4(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_MSG6(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_MSG11(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_MSG12(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_MSG13(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply);


void pushIMEU2CANData(IMEU2_CAN_ROUGH_DATA IMEU2CANRoughData[], int iNum);




















#endif // !__IMEU2_CAN_H__