// #define _TRACE_PRINT_ // RPMSG-SETT
#include "ccu_sampler_main.h"
// #define TRACE  TRACED
// #define TRACEX TRACED

/*********************************************************************************
*  
*  FUNCTION NAME : CAN_StuffHead4PCS
*  CREATOR       : John
*  CREATED DATE  : 2022-7-16 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void CAN_StuffHead4PCS(BYTE	byDevNo, BYTE byCmdNo, BYTE byDestAddr, BYTE bySrcAddr, BYTE *pbyFrame)
{
	//#define	CAN_FRAME_DATA_LEN				0x08
	#define	CAN_SEND_NO_ERR_CODE			0x00
	//*pbyFrame = (BYTE)(0x80 + CAN_FRAME_DATA_LEN);

	UINT uiFrmID = 0;

	uiFrmID = ((UINT)CAN_SEND_NO_ERR_CODE << 26);
	uiFrmID += ((UINT)byDevNo << 22);
	uiFrmID += ((UINT)byCmdNo << 16);
	uiFrmID += ((UINT)byDestAddr << 8);
	uiFrmID += ((UINT)bySrcAddr);

	TRACE("--------Ready to Send Frame[%08X]----byDestAddr=[%d], bySrcAddr=[%d]\n",uiFrmID, byDestAddr, bySrcAddr);

	//uiFrmID = (uiFrmID << 3);//John 20220707 Remove this Offset

	*(pbyFrame + 0) = (BYTE)(uiFrmID >> 24);
	*(pbyFrame + 1) = (BYTE)(uiFrmID >> 16);
	*(pbyFrame + 2) = (BYTE)(uiFrmID >> 8);
	*(pbyFrame + 3) = (BYTE)(uiFrmID);	
}

static void CAN_ParseHead4PCS(A_CAN_RECT_FRAME *pFrame)
{
	UINT uiRawHead = 0;
	uiRawHead = ((UINT)pFrame->byRawData[0]) << 24;
	uiRawHead += ((UINT)pFrame->byRawData[1]) << 16;
	uiRawHead += ((UINT)pFrame->byRawData[2]) << 8;
	uiRawHead += ((UINT)pFrame->byRawData[3]);
	// uiRawHead = (uiRawHead >> 3); //John 20220707 Remove this Offset

	TRACE("----The Parsed Reply Frame Head: [%08X]----------------\n", uiRawHead);

	pFrame->bySrcAddr = (BYTE)uiRawHead;
	pFrame->byDestAddr = (BYTE)(uiRawHead >> 8);
	pFrame->byCmdNo = (BYTE)((uiRawHead >> 16) & 0x3F);
	pFrame->byDevNo = (BYTE)((uiRawHead >> 22) & 0x0F);
	pFrame->byErrCode = (BYTE)((uiRawHead >> 26) & 0x07);

	memcpy(pFrame->byData, pFrame->byRawData + 4, 8);

	return;
}

void CAN_FloatToString(float fInput, BYTE *strOutput)
{
	int i;
	VAR_VALUE val;

	val.fValue = fInput;

	for (i = 0; i < 4; i++)
	{
		strOutput[4 - i - 1] = val.abyValue[i];
	}

	return;
}

void CAN_UShortToString(USHORT usInput, BYTE *strOutput)
{
	strOutput[0] = (BYTE)(usInput >> 8);
	strOutput[1] = (BYTE)(usInput);
}

float CAN_StringToFloat(BYTE *strInput)
{
	int i;
	VAR_VALUE val;

	for (i = 0; i < 4; i++)
	{
		val.abyValue[4 - i - 1] = strInput[i];
	}
	return val.fValue;
}

UINT CAN_StringToUint(BYTE *strInput)
{
	UINT uiValue = (((UINT)(strInput[0])) << 24) + (((UINT)(strInput[1])) << 16) + (((UINT)(strInput[2])) << 8) + ((UINT)(strInput[3]));

	return uiValue;
}

int CAN_StringToInt(char *strInput)
{
	return (int)(((strInput[0] & 0xFF) << 24) | ((strInput[1] & 0xFF) << 16) | ((strInput[2] & 0xFF) << 8) | (strInput[3] & 0xFF));
}

USHORT CAN_StringToUShort(BYTE *strInput)
{
	USHORT usValue = (((USHORT)(strInput[0])) << 8) + ((USHORT)(strInput[1]));

	return usValue;
}

int CAN_StringToEnum(char *strInput)
{
	return (int)(strInput[3] & 0x0F);
}

/*********************************************************************************
*  
*  FUNCTION NAME : On_Handle_PCSSettingCmd
*  CREATOR       :  
*  CREATED DATE  : 2022-7-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL On_Handle_PCSSettingCmd(ROUGH_CCU_DATA *pCCU, int nDevId, int nParamId, VAR_VALUE *pVarVal, BOOL bSettCmd, BOOL bIsGroupCmd, int nWhichSingleModule)
{
	BOOL bRet = TRUE;
	BYTE byDevNoType;
	BYTE byDest_Addr;
	int nDevIdx = nDevId - 701;
	int nOnlineSingleNum = pCCU->strLSYSInfo.blobGrpRectData.nOnlineRectNum[nDevIdx];
	if(bIsGroupCmd == TRUE)
	{
		byDevNoType = DEV_NO;
		byDest_Addr = (BYTE)(nDevIdx + 1); 
	}
	else
	{
		byDevNoType = DEV_NO_SINGLE;
		byDest_Addr = (BYTE)((nDevIdx) * nOnlineSingleNum + (nWhichSingleModule)); 
	}

	if (nParamId == 319151 || nParamId >= 996 && nParamId <= 999) 
	return TRUE; // auth support param, must be set successfully

	if((byDest_Addr < 0) || (byDest_Addr > (MAX_SINGLE_STBATT_NUM+1)))
	{
		printf("Err:Wrong Dest Addr=[%02X] nDevId=[%d] nParamId=[%d]\n", byDest_Addr, nDevId, nParamId);
		return FALSE;
	}
	
	if(nDevId == 705) //STS
	{
		byDest_Addr = STS_ADDR;
		byDevNoType = DEV_NO_SINGLE;
	}

	SETT_PCS_PARAM pcsSettigs[] =
	{
		{SETT_PCS_ID_1101,	1101,		1000,  	PCS_TYPE_FLOAT,			0x10, 0x01}, 
		{SETT_PCS_ID_1102,	1102,		1000,  	PCS_TYPE_FLOAT,			0x10, 0x02}, //Use Group Cmd.0x1102 is Single Cmd
		{SETT_PCS_ID_1110,	1110,		1,  	PCS_TYPE_ENUM,			0x11, 0x10}, 

		{SETT_PCS_ID_1127,	1127,		1,  	PCS_TYPE_ENUM,			0x11, 0x27}, 
		{SETT_PCS_ID_1128,	1128,		1,  	PCS_TYPE_ENUM,			0x11, 0x28}, 
		{SETT_PCS_ID_1133, 	1133,		1,  	PCS_TYPE_ENUM,			0x11, 0x33},
		{SETT_PCS_ID_2003, 	2003,		1000000,PCS_TYPE_FLOAT,			0x20, 0x03},
		{SETT_PCS_ID_2101,	2101,	  	1000,  	PCS_TYPE_FLOAT,			0x21, 0x01},
		{SETT_PCS_ID_2102, 	2102,		1000, 	PCS_TYPE_FLOAT,			0x21, 0x02},
		{SETT_PCS_ID_2103, 	2103,		250, 	PCS_TYPE_FLOAT,			0x21, 0x03},
		{SETT_PCS_ID_2105,	2105,	  	1000,  	PCS_TYPE_FLOAT,			0x21, 0x05},
		{SETT_PCS_ID_2106,	2106,	  	1000,  	PCS_TYPE_UINT,			0x21, 0x06},
		{SETT_PCS_ID_2107,	2107,	  	1000,  	PCS_TYPE_UINT,			0x21, 0x07},
		{SETT_PCS_ID_2110,	2110,	  	1,  	PCS_TYPE_ENUM,			0x21, 0x10},
		{SETT_PCS_ID_2111,	2111,		1,  	PCS_TYPE_ENUM,			0x21, 0x11},
		{SETT_PCS_ID_2114,	2114,		1,  	PCS_TYPE_ENUM,			0x21, 0x14},
		{SETT_PCS_ID_2115,	2115,		1,  	PCS_TYPE_ENUM,			0x21, 0x15},
		{SETT_PCS_ID_2117,	2117,		1,  	PCS_TYPE_ENUM,			0x21, 0x17},
		{SETT_PCS_ID_2118,	2118,		1,  	PCS_TYPE_ENUM,			0x21, 0x18},
		{SETT_PCS_ID_2120,	2120,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x20},
		{SETT_PCS_ID_2121,	2121,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x21},
		{SETT_PCS_ID_2122,	2122,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x22},
		{SETT_PCS_ID_2123,	2123,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x23},
		{SETT_PCS_ID_2124,	2124,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x24},
		{SETT_PCS_ID_2125,	2125,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x25},
		{SETT_PCS_ID_2126,	2126,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x26},
		{SETT_PCS_ID_2127,	2127,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x27},
		{SETT_PCS_ID_2128,	2128,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x28},
		{SETT_PCS_ID_2129,	2129,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x29},
		{SETT_PCS_ID_21210,	21210,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x2A},	
		{SETT_PCS_ID_21211,	21211,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x2B},
		{SETT_PCS_ID_21212,	21212,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x2C},
		{SETT_PCS_ID_21213,	21213,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x2D},
		{SETT_PCS_ID_21214,	21214,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x2E},
		{SETT_PCS_ID_21215,	21215,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x2F},
		{SETT_PCS_ID_2130,	2130,		1000,  	PCS_TYPE_FLOAT,			0x21, 0x30},
		{SETT_PCS_ID_2140,	2140,		1,  	PCS_TYPE_ENUM,			0x21, 0x40},
		{SETT_PCS_ID_2141,	2141,		1,  	PCS_TYPE_ENUM,			0x21, 0x41},
		{SETT_PCS_ID_2142,	2142,		1,  	PCS_TYPE_ENUM,			0x21, 0x42},

		{SETT_PCS_ID_2143,	2143,		1,  	PCS_TYPE_ENUM,			0x21, 0x43},
		{SETT_PCS_ID_2144,	2144,		1,  	PCS_TYPE_ENUM,			0x21, 0x44},
		{SETT_PCS_ID_2145,	2145,		1,  	PCS_TYPE_ENUM,			0x21, 0x45},
		{SETT_PCS_ID_2146,	2146,		1,  	PCS_TYPE_ENUM,			0x21, 0x46},

		{SETT_PCS_ID_3111,	3111,		1,  	PCS_TYPE_ENUM,			0x31, 0x11}, 
		{SETT_PCS_ID_3112,	3112,		1,  	PCS_TYPE_ENUM,			0x31, 0x12}, 	
		{SETT_PCS_ID_3113,	3113,		1,  	PCS_TYPE_FLOAT,			0x31, 0x13}, 
		{SETT_PCS_ID_3114,	3114,		1000,  	PCS_TYPE_UINT,			0x31, 0x14}, 
		{SETT_PCS_ID_3115,	3115,		1,  	PCS_TYPE_FLOAT,			0x31, 0x15}, 
		{SETT_PCS_ID_3116,	3116,		1000,  	PCS_TYPE_UINT,			0x31, 0x16}, 
		{SETT_PCS_ID_3117,	3117,		1,  	PCS_TYPE_UINT,			0x31, 0x17}, 
		{SETT_PCS_ID_3118,	3118,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x18},
		{SETT_PCS_ID_3119,	3119,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x19},
		{SETT_PCS_ID_31181,	31181,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x18},
		{SETT_PCS_ID_31191,	31191,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x19},
		{SETT_PCS_ID_3121,	3121,		1,  	PCS_TYPE_FLOAT,			0x31, 0x21},
		{SETT_PCS_ID_3122,	3122,		1,  	PCS_TYPE_FLOAT,			0x31, 0x22},
		{SETT_PCS_ID_3123,	3123,		1000000, PCS_TYPE_FLOAT,		0x31, 0x23}, 
		{SETT_PCS_ID_3124,	3124,		1,  	PCS_TYPE_FLOAT,			0x31, 0x24}, 
		{SETT_PCS_ID_3125,	3125,		1,  	PCS_TYPE_FLOAT,			0x31, 0x25}, 
		{SETT_PCS_ID_3126,	3126,		1,  	PCS_TYPE_FLOAT,			0x31, 0x26},
		{SETT_PCS_ID_3127,	3127,		1,  	PCS_TYPE_FLOAT,			0x31, 0x27},
		{SETT_PCS_ID_3128,	3128,		1,  	PCS_TYPE_FLOAT,			0x31, 0x28},
		{SETT_PCS_ID_31281,	31281,		1,  	PCS_TYPE_FLOAT,			0x31, 0x2A},
		{SETT_PCS_ID_3129,	3129,		1,  	PCS_TYPE_FLOAT,			0x31, 0x29},
		{SETT_PCS_ID_31211,	31211,		3000,  	PCS_TYPE_FLOAT,			0x31, 0x2B},
		{SETT_PCS_ID_3131,	3131,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x31},
		{SETT_PCS_ID_3132,	3132,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x32},
		{SETT_PCS_ID_3133,	3133,		20,  	PCS_TYPE_FLOAT,			0x31, 0x33},
		{SETT_PCS_ID_3135,	3135,		1,  	PCS_TYPE_FLOAT,			0x31, 0x35},
		{SETT_PCS_ID_3136,	3136,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x36},
		{SETT_PCS_ID_3138,	3138,		20,  	PCS_TYPE_FLOAT,			0x31, 0x38},

		{SETT_PCS_ID_3150,	3150,		100,  	PCS_TYPE_FLOAT,			0x31, 0x50},
		{SETT_PCS_ID_3151,	3151,		100,  	PCS_TYPE_FLOAT,			0x31, 0x51},
		{SETT_PCS_ID_3152,	3152,		100,  	PCS_TYPE_FLOAT,			0x31, 0x52},
		{SETT_PCS_ID_3153,	3153,		100,  	PCS_TYPE_FLOAT,			0x31, 0x53},
		{SETT_PCS_ID_3154,	3154,		100,  	PCS_TYPE_FLOAT,			0x31, 0x54},
		{SETT_PCS_ID_3155,	3155,		100,  	PCS_TYPE_FLOAT,			0x31, 0x55},
		{SETT_PCS_ID_3156,	3156,		100,  	PCS_TYPE_FLOAT,			0x31, 0x56},
		{SETT_PCS_ID_3157,	3157,		100,  	PCS_TYPE_FLOAT,			0x31, 0x57},

		{SETT_PCS_ID_3160,	3160,		10,  	PCS_TYPE_FLOAT,			0x31, 0x60}, 
		{SETT_PCS_ID_3161,	3161,		1,  	PCS_TYPE_ENUM,			0x31, 0x61},
		{SETT_PCS_ID_3200,	3200,		1,  	PCS_TYPE_ENUM,			0x31, 0x61},
		{SETT_PCS_ID_3163,	3163,		3000,  	PCS_TYPE_FLOAT,			0x31, 0x63},
		{SETT_PCS_ID_3164,	3164,		100,  PCS_TYPE_FLOAT,			0x31, 0x64},
		{SETT_PCS_ID_3165,	3165,		100,  PCS_TYPE_FLOAT,			0x31, 0x65},
		{SETT_PCS_ID_3166,	3166,		100,  PCS_TYPE_FLOAT,			0x31, 0x66},
		{SETT_PCS_ID_3167,	3167,		100,  PCS_TYPE_FLOAT,			0x31, 0x67},
		{SETT_PCS_ID_3168,	3168,		100,  PCS_TYPE_FLOAT,			0x31, 0x68},
		{SETT_PCS_ID_3169,	3169,		100,  PCS_TYPE_FLOAT,			0x31, 0x69},

		{SETT_PCS_ID_31610,	31610,		100,  	PCS_TYPE_FLOAT,			0x31, 0x6A},
		{SETT_PCS_ID_31611,	31611,		100,  	PCS_TYPE_FLOAT,			0x31, 0x6B},
		{SETT_PCS_ID_31612,	31612,		10,  	PCS_TYPE_FLOAT,			0x31, 0x6C},
		{SETT_PCS_ID_31613,	31613,		10,  	PCS_TYPE_FLOAT,			0x31, 0x6D},

		{SETT_PCS_ID_31614,	31614, 		100,  	PCS_TYPE_FLOAT,			0x31, 0x6E},
		{SETT_PCS_ID_31615,	31615,		100,  	PCS_TYPE_FLOAT,			0x31, 0x6F},

		{SETT_PCS_ID_3173,	3173, 		1,  	PCS_TYPE_FLOAT,			0x31, 0x73},
		{SETT_PCS_ID_3175,	3175, 		10,  	PCS_TYPE_FLOAT,			0x31, 0x75},
		{SETT_PCS_ID_31751,	31751, 		1,  	PCS_TYPE_FLOAT,			0x31, 0x75},
		{SETT_PCS_ID_3174,	3174, 		10,  	PCS_TYPE_FLOAT,			0x31, 0x74},
		{SETT_PCS_ID_31741,	31741,		1,  	PCS_TYPE_FLOAT,			0x31, 0x74},
		{SETT_PCS_ID_3176,	3176,		1,  	PCS_TYPE_FLOAT,			0x31, 0x76},
		{SETT_PCS_ID_3177,	3177,		1,  	PCS_TYPE_FLOAT,			0x31, 0x77},
		{SETT_PCS_ID_3178,	3178,		1,  	PCS_TYPE_FLOAT,			0x31, 0x78},
		{SETT_PCS_ID_31781,	31781,		1,  	PCS_TYPE_FLOAT,			0x31, 0x78},
		
		{SETT_PCS_ID_3179,	3179,		10,  	PCS_TYPE_FLOAT,			0x31, 0x79},
		{SETT_PCS_ID_31710,	31710,		10,  	PCS_TYPE_FLOAT,			0x31, 0x7A},
		{SETT_PCS_ID_31711,	31711,		10,  	PCS_TYPE_FLOAT,			0x31, 0x7B},
		{SETT_PCS_ID_31712,	31712,		10,  	PCS_TYPE_FLOAT,			0x31, 0x7C},
		{SETT_PCS_ID_31713,	31713,		10,  	PCS_TYPE_FLOAT,			0x31, 0x7D},
		{SETT_PCS_ID_31714,	31714,		10,  	PCS_TYPE_FLOAT,			0x31, 0x7E},
		{SETT_PCS_ID_317141,317141,		10,  	PCS_TYPE_FLOAT,			0x31, 0x7E},

		{SETT_PCS_ID_31110,	31110,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x1A},
		{SETT_PCS_ID_31111,	31111,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x1B},
		{SETT_PCS_ID_311101,311101,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x1A},
		{SETT_PCS_ID_311111,311111,		1000,  	PCS_TYPE_FLOAT,			0x31, 0x1B},
		{SETT_PCS_ID_31113,	31113,		1,  	PCS_TYPE_ENUM,			0x31, 0x1D},

		{SETT_PCS_ID_31121,	31121,		1,  	PCS_TYPE_UINT,			0x31, 0xC1},
		{SETT_PCS_ID_31130,	31130,		1,  	PCS_TYPE_ENUM,			0x31, 0xD0}, 

		{SETT_PCS_ID_31131,	31131,		1,  	PCS_TYPE_ENUM,			0x31, 0xD1}, 
		{SETT_PCS_ID_31132,	31132,		1,  	PCS_TYPE_ENUM,			0x31, 0xD2},
		{SETT_PCS_ID_31133,	31133,		1,  	PCS_TYPE_ENUM,			0x31, 0xD3},
		{SETT_PCS_ID_31136,	31136,		1,  	PCS_TYPE_ENUM,			0x31, 0xD6}, 
		{SETT_PCS_ID_31138,	31138,		1,  	PCS_TYPE_ENUM,			0x31, 0xD8}, 

		{SETT_PCS_ID_31139,	31139,		1,  	PCS_TYPE_ENUM,			0x31, 0xD9}, 
		{SETT_PCS_ID_31156,	31156,		1,  	PCS_TYPE_ENUM,			0x31, 0x2E},
		{SETT_PCS_ID_311561,311561,		1,  	PCS_TYPE_ENUM,			0x31, 0xF6},
		{SETT_PCS_ID_31157,	31157,		1,  	PCS_TYPE_ENUM,			0x31, 0xF7},
		{SETT_PCS_ID_31157,	31158,		1,  	PCS_TYPE_ENUM,			0x31, 0xF8},
		{SETT_PCS_ID_31159,	31159,		10,  	PCS_TYPE_FLOAT,			0x31, 0xF9},
		{SETT_PCS_ID_311512,311512,		100,  	PCS_TYPE_FLOAT,			0x31, 0xFC},
		{SETT_PCS_ID_31310,	31310,		1,  	PCS_TYPE_FLOAT,			0x31, 0x3A},
		{SETT_PCS_ID_31315,	31315,		1,  	PCS_TYPE_FLOAT,			0x31, 0x2F},
		{SETT_PCS_ID_313151,313151,		3000,  	PCS_TYPE_FLOAT,			0x31, 0x3F},

		{SETT_PCS_ID_31914,	31914,		1,  	PCS_TYPE_FLOAT,			0x31, 0x9E},
		{SETT_PCS_ID_31915,	31915,		1,  	PCS_TYPE_FLOAT,			0x31, 0x9F},
		{SETT_PCS_ID_3111111,3111111,	1,  	PCS_TYPE_FLOAT,			0x31, 0xBB},
		{SETT_PCS_ID_31234,	31243,		1,  	PCS_TYPE_FLOAT,			0x31, 0xF3},
		{SETT_PCS_ID_31250,	31250,		1,  	PCS_TYPE_FLOAT,			0x31, 0xFA},
		{SETT_PCS_ID_31251,	31251,		1,  	PCS_TYPE_FLOAT,			0x31, 0xFB},

		{SETT_PCS_ID_311313,311313,		1,  	PCS_TYPE_ENUM,			0x31, 0xDD},
		{SETT_PCS_ID_311315,311315,		1,  	PCS_TYPE_ENUM,			0x31, 0xDF},

		{SETT_PCS_ID_5101,	5101,		1000,  	PCS_TYPE_FLOAT,			0x51, 0x01},
		{SETT_PCS_ID_5102,	5102,		1000,  	PCS_TYPE_FLOAT,			0x51, 0x02},
		{SETT_PCS_ID_5103,	5103,		1000,  	PCS_TYPE_FLOAT,			0x51, 0x03},
		{SETT_PCS_ID_5104,	5104,		1000,  	PCS_TYPE_FLOAT,			0x51, 0x04},
		{SETT_PCS_ID_5105,	5105,		1000,  	PCS_TYPE_FLOAT,			0x51, 0x05},

		{SETT_PCS_ID_7140,	7140,		1,  	PCS_TYPE_ENUM,			0x71, 0x40}, 
		{SETT_PCS_ID_71411,	71411,		1,  	PCS_TYPE_ENUM,			0x71, 0x4B}, 
		{SETT_PCS_ID_71412,	71412,		1,  	PCS_TYPE_ENUM,			0x71, 0x4C}, 
		{SETT_PCS_ID_3180,	3180,		1,  	PCS_TYPE_FLOAT,			0x31, 0x80},
		{SETT_PCS_ID_3181,	3181,		1,  	PCS_TYPE_FLOAT,			0x31, 0x81},
		{SETT_PCS_ID_3182,	3182,		1,  	PCS_TYPE_FLOAT,			0x31, 0x82},
		{SETT_PCS_ID_3183,	3183,		1,  	PCS_TYPE_FLOAT,			0x31, 0x83},
		{SETT_PCS_ID_3184,	3184,		1,  	PCS_TYPE_FLOAT,			0x31, 0x84},
		{SETT_PCS_ID_3185,	3185,		1,  	PCS_TYPE_FLOAT,			0x31, 0x85},
		{SETT_PCS_ID_3186,	3186,		1,  	PCS_TYPE_FLOAT,			0x31, 0x86},
		{SETT_PCS_ID_3187,	3187,		1,  	PCS_TYPE_FLOAT,			0x31, 0x87},
		{SETT_PCS_ID_3188,	3188,		1,  	PCS_TYPE_FLOAT,			0x31, 0x88},
		{SETT_PCS_ID_3189,	3189,		1,  	PCS_TYPE_FLOAT,			0x31, 0x89},
		{SETT_PCS_ID_31810,	31810,		1,  	PCS_TYPE_FLOAT,			0x31, 0x8A},
		{SETT_PCS_ID_31811,	31811,		1,  	PCS_TYPE_ENUM,			0x31, 0x8B},
		{SETT_PCS_ID_31812,	31812,		1,  	PCS_TYPE_ENUM,			0x31, 0x8C},
		{SETT_PCS_ID_31813,	31813,		1,  	PCS_TYPE_FLOAT,			0x31, 0x8D},//edit by kuangyue 20250913
		{SETT_PCS_ID_31814,	31814,		1,  	PCS_TYPE_FLOAT,			0x31, 0x8E},
		{SETT_PCS_ID_3190,	3190,		1,  	PCS_TYPE_FLOAT,			0x31, 0x90},
		{SETT_PCS_ID_3191,	3191,		1,  	PCS_TYPE_FLOAT,			0x31, 0x91},
		{SETT_PCS_ID_3192,	3192,		1,  	PCS_TYPE_FLOAT,			0x31, 0x92},
		{SETT_PCS_ID_3193,	3193,		1,  	PCS_TYPE_FLOAT,			0x31, 0x93},
		{SETT_PCS_ID_3194,	3194,		1,  	PCS_TYPE_FLOAT,			0x31, 0x94},
		{SETT_PCS_ID_3195,	3195,		1,  	PCS_TYPE_FLOAT,			0x31, 0x95},
		{SETT_PCS_ID_3196,	3196,		1,  	PCS_TYPE_FLOAT,			0x31, 0x96},
		{SETT_PCS_ID_3197,	3197,		1,  	PCS_TYPE_FLOAT,			0x31, 0x97},
		{SETT_PCS_ID_3198,	3198,		1,  	PCS_TYPE_FLOAT,			0x31, 0x98},
		{SETT_PCS_ID_3199,	3199,		1,  	PCS_TYPE_FLOAT,			0x31, 0x99},
		{SETT_PCS_ID_31910,	31910,		1,  	PCS_TYPE_FLOAT,			0x31, 0x9A},
		{SETT_PCS_ID_31911,	31911,		1,  	PCS_TYPE_FLOAT,			0x31, 0x9B},
		{SETT_PCS_ID_31912,	31912,		1,  	PCS_TYPE_FLOAT,			0x31, 0x9C},

		{SETT_PCS_ID_311011,311011,		1,		PCS_TYPE_ENUM,			0x31, 0xAB},

		{SETT_PCS_ID_32001,32000,		1,		PCS_TYPE_FLOAT,			0x32, 0x00},
		{SETT_PCS_ID_3201,	3201,		1,		PCS_TYPE_FLOAT,			0x32, 0x01},
		{SETT_PCS_ID_3202,	3202,		1,		PCS_TYPE_FLOAT,			0x32, 0x02},
		{SETT_PCS_ID_3203,	3203,		1,		PCS_TYPE_FLOAT,			0x32, 0x03},
		{SETT_PCS_ID_3204,	3204,		10,		PCS_TYPE_FLOAT,			0x32, 0x04},
		{SETT_PCS_ID_3205,	3205,		10,		PCS_TYPE_FLOAT,			0x32, 0x05},
		{SETT_PCS_ID_3206,	3206,		10,		PCS_TYPE_FLOAT,			0x32, 0x06},
		{SETT_PCS_ID_3207,	3207,		10,		PCS_TYPE_FLOAT,			0x32, 0x07},
		{SETT_PCS_ID_91011,	91011,		1,		PCS_TYPE_ENUM,			0x91, 0x0B},


	};

	SETT_PCS_PARAM *pElemtList = &pcsSettigs[0];

	int n = 0;
	BOOL bFoundKey = FALSE;
	for (n = 0; n < ITEM_OF(pcsSettigs) ; n++)
	{
		TRACEX("n = %d pcsIdx=[%d] pcsSendCmd=[%d] \n", n, pElemtList[n].nParamId, nParamId);
		if(pElemtList[n].nParamId == nParamId)
		{
			TRACEX(" n = %d  %d \n",n, pElemtList[n].nParamId);
			bFoundKey = TRUE;
			break;
		}
	}

	if (!bFoundKey)
	{
		TRACEX("Error:Cannot get the key[%s]\n", nParamId);
		return FALSE;
	}

	BYTE pbySendBuf[32]={'\0'};
	BYTE byPCS_SetData[10]={'\0'};
	int  nSetValue = 0;

	byPCS_SetData[0] = pElemtList[n].bySubCmd;
	byPCS_SetData[1] = pElemtList[n].byCmdIdx;

	//---John Added 2022-12-7---//
	//Only Read One PCS Sett to avoid soo much data to Parse.//
	byPCS_SetData[2] = (bSettCmd == TRUE) ?  0x00 : 0x01; 
	if(bSettCmd == TRUE)
	{
		switch (pElemtList[n].ValueType)
		{
			case PCS_TYPE_INT:
				nSetValue = (int)pVarVal->lValue*pElemtList[n].nScale;
				byPCS_SetData[4] = (BYTE)(nSetValue >> 24);
				byPCS_SetData[5] = (BYTE)(nSetValue >> 16);
				byPCS_SetData[6] = (BYTE)(nSetValue >> 8);
				byPCS_SetData[7] = (BYTE)(nSetValue);
			break;
			case PCS_TYPE_UINT:
				nSetValue = (int)pVarVal->ulValue*pElemtList[n].nScale;
				byPCS_SetData[4] = (BYTE)(nSetValue >> 24);
				byPCS_SetData[5] = (BYTE)(nSetValue >> 16);
				byPCS_SetData[6] = (BYTE)(nSetValue >> 8);
				byPCS_SetData[7] = (BYTE)(nSetValue);
			break;
			case PCS_TYPE_ENUM:
				// byPCS_SetData[7] = 0xA0 + pVarVal->emValue; // default value
				if(pVarVal->emValue == 18)//0x10 EN 50549-1/-10 type B 和0x10 EN 50549-1/-10 type A 的值相同
				{
					byPCS_SetData[7] = 0xB0;
				}
				else if(pVarVal->emValue == 19)//0x11 EN 50549-2 type B 和 0x11 EN 50549-2 type A 的值相同
				{
					byPCS_SetData[7] = 0xB1;
				}
				else
				{
					byPCS_SetData[7] = 0xA0 + pVarVal->emValue; // default value
				}
				// else if(pVarVal->emValue == 2)
				// {
				// 	byPCS_SetData[7] = 0xA2;
				// }
				// else if(pVarVal->emValue == 3)
				// {
				// 	byPCS_SetData[7] = 0xA3;
				// }
				// else if(pVarVal->emValue == 4)
				// {
				// 	byPCS_SetData[7] = 0xA4;
				// }
				// else if(pVarVal->emValue == 5)
				// {
				// 	byPCS_SetData[7] = 0xA5;
				// }
				// else if(pVarVal->emValue == 6)
				// {
				// 	byPCS_SetData[7] = 0xA6;
				// }
			break;
			case PCS_TYPE_FLOAT:
				if((nParamId == 3133) || (nParamId == 3138))
				{
					if (ABS(pVarVal->fValue) > 0.001) // LJH250407 fix
					{
						nSetValue = (int)((100.0/pVarVal->fValue)*pElemtList[n].nScale);
					}
				}
				else
				{
					nSetValue = (int)(pVarVal->fValue*pElemtList[n].nScale);
				}	

				if (
				// pElemtList[n].nParamId == 31610 || pElemtList[n].nParamId == 31611 ||
				pElemtList[n].pcsIdx >= SETT_PCS_ID_3173 && pElemtList[n].pcsIdx <= SETT_PCS_ID_3178 ||
				pElemtList[n].nParamId == 31741 || pElemtList[n].nParamId == 31751 ||
				pElemtList[n].pcsIdx >= SETT_PCS_ID_32001 && pElemtList[n].pcsIdx <= SETT_PCS_ID_3203)
				{
					float fPd;
					VAR_VALUE varV;
					pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 200, 223, &fPd, 4);
					printf("fPd = %f\n", fPd);
					fPd *= 10;
					nSetValue *= fPd;
				}

				if (pElemtList[n].nParamId = 3160)
				{
					Set_IMMU_DO(IOC_SET_DO6, 1);
					Sleep(50);
					Set_IMMU_DO(IOC_SET_DO6, 0);
				}	
				byPCS_SetData[4] = (BYTE)(nSetValue >> 24);
				byPCS_SetData[5] = (BYTE)(nSetValue >> 16);
				byPCS_SetData[6] = (BYTE)(nSetValue >> 8);
				byPCS_SetData[7] = (BYTE)(nSetValue);
			break;
			default:
				break;
		}
	}	
	else
	{
		byPCS_SetData[4] = 0;
		byPCS_SetData[5] = 0;
		byPCS_SetData[6] = 0;
		byPCS_SetData[7] = 0;
	}
	if(nDevId == 700)
	{
		BOOL bFlag = TRUE;
		byDevNoType = DEV_NO;
		int m;

		if(nParamId == 1110) //Rect On/Off, output through J8-DO5
		{
			Set_IMMU_DO(IOC_SET_DO5, 1);
			Sleep(50);
			Set_IMMU_DO(IOC_SET_DO5, 0);
		}
		
		// if (pCCU->lcCfg.emSystemType >= SYSTYPE_SCADA_INPCS)
		// {// LJH241210 add DevId:700 INPCS need addr=0
		// 	BYTE pbyGrpSendBuf[32]={'\0'};
		// 	if(pElemtList[n].bySubCmd != 0x10)
		// 	{
		// 			byDevNoType = DEV_NO_SINGLE;
		// 	}
		// 	CAN_StuffHead4PCS(byDevNoType, SETT_CMD_NO, byDest_Addr , SRC_ADDR, pbyGrpSendBuf);
		// 	memcpy(pbyGrpSendBuf + 4, byPCS_SetData, 8);
		// 	bRet = DoTCPSession(pCCU, (bSettCmd == TRUE) ? NL_MSG_PCSSET : NL_MSG_READ_PCSSET, pbyGrpSendBuf, 12, 1000);
		// 	// printf("bRet = [%d]\n", bRet);
		// 	bFlag = bFlag && bRet;
		// }
		// else
		{
			for(m = 1; m <= MAX_SINGLE_STBATT_NUM; m++)
			{
				byDest_Addr = (BYTE)(m);
				BYTE pbyGrpSendBuf[32]={'\0'};
				CAN_StuffHead4PCS(byDevNoType, SETT_CMD_NO, byDest_Addr , SRC_ADDR, pbyGrpSendBuf);

				memcpy(pbyGrpSendBuf + 4, byPCS_SetData, 8);

				// printf("\n\t111111 Send Cmd To CCU nSetValue=[%d]:", nSetValue);
				// for(n = 0; n < 12; n++)
				// {
				// 	printf("%02X-", pbyGrpSendBuf[n]);
				// }
				// printf("\n\t bRet=[%d]\n", bRet);

				bRet = DoTCPSession(pCCU, (bSettCmd == TRUE) ? NL_MSG_PCSSET : NL_MSG_READ_PCSSET, pbyGrpSendBuf, 12, 1000);
				bFlag = bFlag && bRet;
			}
		}
		return bFlag;
	}

	CAN_StuffHead4PCS(byDevNoType, SETT_CMD_NO, byDest_Addr , SRC_ADDR, pbySendBuf);

	memcpy(pbySendBuf + 4, byPCS_SetData, 8);

	// printf("\n\t Send Cmd To CCU nSetValue=[%d]:", nSetValue);
	// for(n = 0; n < 12; n++)
	// {
	// 	printf("%02X-", pbySendBuf[n]);
	// }

	bRet = DoTCPSession(pCCU, NL_MSG_PCSSET, pbySendBuf, 12, 1000);

	if(nParamId == 1110) //Rect On/Off, output through J8-DO5
	{
		Set_IMMU_DO(IOC_SET_DO5, 1);
		Sleep(50);
		Set_IMMU_DO(IOC_SET_DO5, 0);
	}

	return bRet;
}

/*===============================================================================
 *  RPMSG 设置参数映射表 + 下发函数
 *  参数 4000~4013 通过 RPMSG 下发到 R 核
 *  映射关系:
 *    4000~4002 → RP_MSG_MODULE_TYPE_CFG    (0x6E, u8, 子索引0~2)
 *    4003~4008 → RP_MSG_MODULE_NUMBER_CFG  (0x6F, u8, 子索引0~5)
 *    4009       → RP_MSG_MAUNL_AUTO_MODE    (0x72, u8, 子索引0)
 *    4010~4013 → RP_MSG_ON_OFF_GRID_CTRL   (0x72, u8, 子索引0~3)
 *    ACDC/DCDC充放电控制暂空，后续补充
 *===============================================================================*/

typedef enum tagRPMsgSettValueType
{
	RPMSG_VAL_U8,
	RPMSG_VAL_U16,
} RPMSG_SETT_VAL_TYPE;

typedef struct tagRPMsgSettParam
{
	SETT_PCS_IDXLIST	pcsIdx;		// 枚举索引
	int					nParamId;	// 参数ID (4000~4013)
	RP_MSGTYPE			emMsgType;	// RPMSG 帧类型
	int					nSubIdx;	// 子索引 (0-based)
	RPMSG_SETT_VAL_TYPE	emValType;	// 数据类型 u8/u16
} RPMSG_SETT_PARAM;

/* RPMSG 设置参数映射表 */
static const RPMSG_SETT_PARAM s_RpmsgSettCfg[] =
{
	// 0x6E 模块类型配置 (u8, 子索引0~2)
	{SETT_PCS_ID_4000,	4000,	RP_MSG_MODULE_TYPE_CFG,		0,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4001,	4001,	RP_MSG_MODULE_TYPE_CFG,		1,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4002,	4002,	RP_MSG_MODULE_TYPE_CFG,		2,	RPMSG_VAL_U8},

	// 0x6F 模块数配置 (u8, 子索引0~5)
	{SETT_PCS_ID_4003,	4003,	RP_MSG_MODULE_NUMBER_CFG,	0,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4004,	4004,	RP_MSG_MODULE_NUMBER_CFG,	1,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4005,	4005,	RP_MSG_MODULE_NUMBER_CFG,	2,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4006,	4006,	RP_MSG_MODULE_NUMBER_CFG,	3,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4007,	4007,	RP_MSG_MODULE_NUMBER_CFG,	4,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4008,	4008,	RP_MSG_MODULE_NUMBER_CFG,	5,	RPMSG_VAL_U8},

	// 0x72 自动/手动模式 (u8, 子索引0)
	{SETT_PCS_ID_4009,	4009,	RP_MSG_MAUNL_AUTO_MODE,	0,	RPMSG_VAL_U8},

	// 0x73 N2002并离网控制 (u16, 子索引0~3)
	{SETT_PCS_ID_4010,	4010,	RP_MSG_ON_OFF_GRID_CTRL,	0,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4011,	4011,	RP_MSG_ON_OFF_GRID_CTRL,	1,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4012,	4012,	RP_MSG_ON_OFF_GRID_CTRL,	2,	RPMSG_VAL_U8},
	{SETT_PCS_ID_4013,	4013,	RP_MSG_ON_OFF_GRID_CTRL,	3,	RPMSG_VAL_U8},
};

/*===============================================================================
 *  FUNCTION: On_Handle_RPMsgSettingCmd
 *  PURPOSE : RPMSG 设置参数下发
 *  FLOW    :
 *    1. 查 s_RpmsgSettCfg 映射表，找到 nParamId 对应的帧类型、子索引、数据类型
 *    2. 按 RPMSG 帧格式组装 pData:
 *       pData[0] = 0x0c(无确认) / 0x0d(有确认)
 *       pData[1] = 子索引数目(1，单参数下发)
 *       pData[2] = 起始子索引
 *       pData[3..] = 数据(u8 占1字节, u16 占2字节小端)
 *    3. 调用 SendRpMsg_CCU 下发
 *===============================================================================*/
BOOL On_Handle_RPMsgSettingCmd(ROUGH_CCU_DATA *pCCU, int nDevId, int nParamId, VAR_VALUE *pVarVal, BOOL bSettCmd)
{
	BOOL bRet = FALSE;
	TRACED("[RPMSG-TRACE] On_Handle_RPMsgSettingCmd: ENTER nDevId=[%d] nParamId=[%d] ulVal=[%lu] bSettCmd=[%d]", nDevId, nParamId, pVarVal->ulValue, bSettCmd);
	int  n = 0;
	const RPMSG_SETT_PARAM *pFound = NULL;

	/* 1. 查映射表 */
	for (n = 0; n < ITEM_OF(s_RpmsgSettCfg); n++)
	{
		if (s_RpmsgSettCfg[n].nParamId == nParamId)
		{
			pFound = &s_RpmsgSettCfg[n];
			break;
		}
	}

	if (pFound == NULL)
	{
		TRACED("On_Handle_RPMsgSettingCmd: nParamId=[%d] not found in RPMSG mapping table!\n", nParamId);
		return FALSE;
	}

	/* 2. 组装 pData */
	BYTE abyData[8] = {0};
	int  nDataLen = 0;

	abyData[0] = 0x0c;			// 无确认通信
	abyData[1] = 1;				// 子索引数目=1 (单参数下发)
	abyData[2] = (BYTE)pFound->nSubIdx;	// 起始子索引
	nDataLen = 3;

	if (pFound->emValType == RPMSG_VAL_U8)
	{
		abyData[3] = (BYTE)(pVarVal->ulValue & 0xFF);
		nDataLen = 4;
	}
	else /* RPMSG_VAL_U16 */
	{
		USHORT usVal = (USHORT)(pVarVal->ulValue & 0xFFFF);
		abyData[3] = (BYTE)(usVal & 0xFF);		// 低字节 (小端)
		abyData[4] = (BYTE)((usVal >> 8) & 0xFF);	// 高字节
		nDataLen = 5;
	}

	/* 3. 下发 */
	FUNCTION_CODE emFC = (bSettCmd == TRUE) ? FUNCODE_REQ_MODIFY : FUNCODE_REQ_QUERY;

	TRACED("On_Handle_RPMsgSettingCmd: nParamId=[%d] MsgType=[%d] SubIdx=[%d] ValType=[%d] FC=[%d]\n",
		nParamId, pFound->emMsgType, pFound->nSubIdx, pFound->emValType, emFC);
	
	SendRpMsg_CCU(pCCU, emFC, pFound->emMsgType, nDataLen, abyData);

	bRet = TRUE;

	return bRet;
}

static BOOL Send_QueryVersion(ROUGH_CCU_DATA *pCCU, BYTE Cmd_Type, UINT nPCS_NUM)
{
	BYTE pbySendBuf[32]={'\0'};
	BYTE byPCS_SetData[10]={'\0'};

	BOOL bRet = TRUE;
	
	// 判断是否是CPLD查询
	if(Cmd_Type == 0xE3)
	{
		byPCS_SetData[0] = 0x11;
		byPCS_SetData[1] = 0x2A;
		
		CAN_StuffHead4PCS(0x02, 0xE3, 0x01, SRC_ADDR, pbySendBuf);
		pbySendBuf[0] = 0x02;
		pbySendBuf[1] = 0xE3;
	}
	else
	{
		byPCS_SetData[0] = 0x03;
		byPCS_SetData[1] = Cmd_Type;
		CAN_StuffHead4PCS(0x02, 0x3F, nPCS_NUM, SRC_ADDR, pbySendBuf);
		pbySendBuf[0] = 0x02;
		pbySendBuf[1] = 0x3F;
	}
	memcpy(pbySendBuf + 4, byPCS_SetData, 8);
	bRet = DoTCPSession(pCCU, NL_MSG_PCSGET, pbySendBuf, 12, 1000);

	return bRet;
}

BOOL Refresh_PCSVersion(ROUGH_CCU_DATA *pCCU)
{
	UINT nPCS_NUM = pCCU->pNewSet.ccuSettings.byNumGrp_1 + pCCU->pNewSet.ccuSettings.byNumGrp_2 + 
					pCCU->pNewSet.ccuSettings.byNumGrp_3 + pCCU->pNewSet.ccuSettings.byNumGrp_4;

	UINT nTotal_PCSNum = nPCS_NUM * 2 + 1;
	if((pCCU->nPCSNum >= nTotal_PCSNum) || (ABS(_NOW_ - pCCU->tmLastSend_Cmd) <= MAX_QUERYVERSION_TIMES))
	{
		return FALSE;
	}

	// printf("Refresh_PCSVersion nPCSNum=[%d] \n", pCCU->nPCSNum);

	if(pCCU->nPCSNum < nPCS_NUM)
	{	
		// printf("Refresh_PCSVersion On_Handle 0x03 nPCSNum=[%d] \n", pCCU->nPCSNum);
		Send_QueryVersion(pCCU, 0x03,  pCCU->nPCSNum);
	}
	else if(pCCU->nPCSNum < nPCS_NUM * 2)
	{
		// printf("Refresh_PCSVersion On_Handle 0x02 nPCSNum=[%d] \n", pCCU->nPCSNum - nPCS_NUM);
		Send_QueryVersion(pCCU, 0x02, pCCU->nPCSNum - nPCS_NUM);
	}
	else if(pCCU->nPCSNum == nPCS_NUM * 2)
    {
        Send_QueryVersion(pCCU, 0xE3, 0x01);
    }
	pCCU->tmLastSend_Cmd = _NOW_;
	pCCU->nPCSNum++;
	return TRUE;
}


/*********************************************************************************
*  
*  FUNCTION NAME : On_Handle_PCSGettCmd
*  CREATOR       :  
*  CREATED DATE  : 2022-7-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL On_Handle_PCSGettCmd(ROUGH_CCU_DATA *pCCU, int nDevId, int nParamId, VAR_VALUE *pVarVal)
{
	BYTE byDest_Addr = (BYTE)(nDevId - 701 + 1);
	BYTE byDevNoType = DEV_NO;

	if(byDest_Addr <= 0 || (byDest_Addr == 0 && pCCU->lcCfg.emSystemType < SYSTYPE_MAX))
	{
		printf("Err:Wrong Dest Addr=[%02X] nDevId=[%d] nParamId=[%d]\n", byDest_Addr, nDevId, nParamId);
		return FALSE;
	}

	if(nDevId == 705) //STS
	{
		byDevNoType = DEV_NO_SINGLE;
		byDest_Addr = STS_ADDR;
	}

	SETT_PCS_PARAM pcsSettigs[] =
	{
		{GET_PCS_ID_1001,	1001,	1,  	PCS_TYPE_UINT,	0x10, 0x00, 0x00}, 	//0x23
		{GET_PCS_ID_1002, 	1002,	1,  	PCS_TYPE_UINT,	0x11, 0x00, 0x00}, 	//0x23
		{GET_PCS_ID_1003,	1003,	1,  	PCS_TYPE_UINT,	0x21, 0x00, 0x00},	//0x23
		{GET_PCS_ID_1004, 	1004,	1, 		PCS_TYPE_UINT,	0x31, 0x00, 0x00},	//0x23
		{GET_PCS_ID_1005,	1005,	1,  	PCS_TYPE_UINT,	0x41, 0x00, 0x00},	//0x23
		{GET_PCS_ID_1006,	1006,	1,  	PCS_TYPE_UINT,	0x11, 0x00, 0x01},	//0x24	
		{GET_PCS_ID_1007,	1007,	1,  	PCS_TYPE_UINT,	0x21, 0x00, 0x01},	//0x24
		{GET_PCS_ID_1008,	1008,	1,  	PCS_TYPE_UINT,	0x31, 0x00, 0x01},	//0x24
		{GET_PCS_ID_1009,	1009,	1,  	PCS_TYPE_UINT,	0x41, 0x00, 0x01},	//0x24
		{GET_PCS_ID_1010,	1010,	1,  	PCS_TYPE_UINT,	0x51, 0x00, 0x01},	//0x24

	};

	SETT_PCS_PARAM *pElemtList = &pcsSettigs[0];

	int n = 0;
	BOOL bFoundKey = FALSE;
	for (n = 0; n < ITEM_OF(pcsSettigs) ; n++)
	{
		TRACEX("n = %d pcsIdx=[%d] pcsSendCmd=[%d] \n", n, pElemtList[n].nParamId, nParamId);
		if(pElemtList[n].nParamId == nParamId)
		{
			TRACEX(" n = %d  %d \n",n, pElemtList[n].nParamId);
			bFoundKey = TRUE;
			break;
		}
	}

	if (!bFoundKey)
	{
		TRACEX("Error:Cannot get the key[%s]\n", nParamId);
		return FALSE;
	}

	BYTE pbySendBuf[32]={'\0'};
	BYTE byPCS_SetData[10]={'\0'};
	int  nSetValue = 0;

	byPCS_SetData[0] = pElemtList[n].bySubCmd;
	byPCS_SetData[1] = (BYTE)(pElemtList[n].byCmdIdx + pVarVal->ulValue);
	byPCS_SetData[2] = pElemtList[n].byByte2;
	// if((pCCU->lcCfg.emSystemType >= SYSTYPE_SCADA_INPCS) &&
	// 	(pElemtList[n].bySubCmd != 0x10 && pElemtList[n].bySubCmd != 0x01))
	// {
	// 	byDevNoType = DEV_NO_SINGLE;
	// }
	
	BYTE byCmd = 0x23;
	if(byPCS_SetData[2] == 0x01)
	{
		byCmd = 0x24;
	}

	CAN_StuffHead4PCS(byDevNoType, byCmd, byDest_Addr , SRC_ADDR, pbySendBuf);

	memcpy(pbySendBuf + 4, byPCS_SetData, 8);

	BOOL bRet = TRUE;
	bRet = DoTCPSession(pCCU, NL_MSG_PCSGET, pbySendBuf, 12, 1000);
 
 	printf("\n\t Send GET byCmd=[%02X] To CCU nSetValue=[%d]:", byCmd, nSetValue);
	for(n = 0; n < 12; n++)
	{
		printf("%02X-", pbySendBuf[n]);
	}
	printf("\n\t bRet=[%d]\n", bRet);

	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : 0x23 To Get PCS StatusInfo
*  CREATOR       : John
*  CREATED DATE  : 2022-7-14 11:22:53
*  DESCRIPTION   :  

	Get 1001 From CCU:02-E3-02-F0-10-01-00-00-00-00-00-00-
*  	Reply:		  00-02-E3-F0-02-10-01-00-00-00-00-03-C8-

*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_PCSGet(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	BYTE rtn = pData[0];
	TRACE("0x23:Received rep for Parse_PCS Get rtn= %d usLenID=[%d]\n", rtn, usLenID);
	
	int n = 0, nOffset = 0;
	char szTime[16];

	TimeToString(time(NULL), "%H:%M:%S", szTime, sizeof(szTime));

	memset(pCCU->szSettCmdReply, 0, 64);

	nOffset += sprintf(pCCU->szSettCmdReply + nOffset, "%s|", szTime);

	for(n = 0; n < MIN(usLenID,26); n++)
	{
		//printf("%02X-",pData[n]);
		if(n == 14)
		{
			nOffset += sprintf(pCCU->szSettCmdReply + nOffset, "|");
		}
		nOffset += sprintf(pCCU->szSettCmdReply + nOffset, "%02X",pData[n]);
	}

	if((pData[2] == 0x02) && (pData[3] == 0x3F))
	{
		
		int nIdx = (UINT)pData[5] + 1;
		VAR_VALUE varV;

		if(pData[7] == 0x03)
		{
			varV.ulValue = (UINT)(pData[10] << 8 | pData[11]);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 250 + nIdx, 1425, varV);

			varV.ulValue = (UINT)(pData[12] << 8 | pData[13]);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 250 + nIdx, 1426, varV);
		}
		else if(pData[7] == 0x02)
		{
			varV.ulValue = (UINT)(pData[10] << 8 | pData[11]);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 250 + nIdx, 1427, varV);

			varV.ulValue = (UINT)(pData[12] << 8 | pData[13]);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 250 + nIdx, 1428, varV);
		}
	} 
	else if((pData[2] == 0x02) && (pData[3] == 0xE3))
    {
		if(rtn == 0 && usLenID > 2)
		{
			BYTE responseNum = pData[1];
			int i;
			for(i = 0; i < responseNum; i++)
			{
				int frameOffset = 2 + i * 12;
				if(frameOffset + 12 <= usLenID)
				{
					A_CAN_RECT_FRAME canFrame;
					memcpy(canFrame.byRawData, &pData[frameOffset], 12);
					CAN_ParseHead4PCS(&canFrame);
					if(canFrame.byData[0] == 0x11 && canFrame.byData[1] == 0x2A)
					{
						int devAddr = canFrame.bySrcAddr + 1;
						VAR_VALUE varV;
						varV.ulValue = (UINT)(canFrame.byData[6] << 8) | canFrame.byData[7];
						Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 250 + devAddr, 1430, varV);
					}
				}
			}
		}
	}
 
	pCCU->szSettCmdReply[nOffset] ='\0';
	//printf("nOffset=[%d] pCCU->szSettCmdReply=[%s]\n", nOffset, pCCU->szSettCmdReply);

	Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 105, (void*)&pCCU->szSettCmdReply,  64);

	if(usLenID < 8 )
	{
		return -1;
	}

	if(rtn != 0)
	{
		return -1;
	}

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : 0x24 To Set PCS Parameter
*  CREATOR       : John
*  CREATED DATE  : 2022-7-14 11:22:53
*  DESCRIPTION   :
 	Note: Use PM Protocal SubCmd=0x10 Group—Cmd,PM Response also GRoup-Cmd;
		Other SubCmd,PM Use Single PM Addr Response.

	AA-7E-AA-24-00-0C-02-E4-02-F0-10-01-00-00-00-00-02-DA-02-F5-0D-

	Set 2115 To CCU:02-E4-02-F0-21-15-00-00-00-00-00-A1-

			 Rcv:00-02-E4-F0-01-21-15-00-00-00-00-00-A1-
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_PCSSet(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	BYTE rtn = pData[0];
	TRACE("0x24:Received Reps for Parse_PCS Set rtn= %d, usLenID=%d Reason:%s\n", \
		rtn, usLenID, (rtn==2)?"CmdNo Abnml":((rtn==7)?"DataArea Abnml":"OK"));	
	int n = 0, nOffset = 0, i = 0;
	char szTime[16];
	BYTE byCanMsgNum = pData[1];

	for(n = 0; n < usLenID; n++)
	{
		printf("%02X-",pData[n]);
	}

	if(usLenID < 8 )
	{
		return -1;
	}

	if(rtn != 0)
	{
		return -1;
	}
	
 	USHORT 	nReadLen = usLenID - 2;
 	BYTE   	*byRecvBuff = &pData[2];
	if(nReadLen < CAN_FRAME_DATALEN)
	{
		return -1;
	}

	for(n = 0; n < (nReadLen / CAN_FRAME_DATALEN); n++)
	{
		memcpy(pCCU->frmReply.byRawData, byRecvBuff + n * CAN_FRAME_DATALEN, CAN_FRAME_DATALEN);
		
		CAN_ParseHead4PCS(&pCCU->frmReply);

		TRACEX("byDestAddr=%02X byCmdNo =%02X byData[0]=%02X byData[1]=%02X\n",
			pCCU->frmReply.byDestAddr,
			pCCU->frmReply.byCmdNo, pCCU->frmReply.byData[0],
			pCCU->frmReply.byCmdNo, pCCU->frmReply.byData[1]);

		TRACEX("Info: byErrCode=%02X bySrcAddr=%02X  frmReply.byCmdNo=%02X\n",
				pCCU->frmReply.byErrCode,\	
				pCCU->frmReply.bySrcAddr,
				pCCU->frmReply.byCmdNo);

		if(pCCU->frmReply.byCmdNo == 0x24)
		{
			ParseCanFrame_24XXXX0001(pCCU, &pCCU->frmReply);
			printf("[%d]th single mode Info Cmd 24:%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",\
			n,
			pCCU->frmReply.byData[0],\
			pCCU->frmReply.byData[1],\
			pCCU->frmReply.byData[2],\
			pCCU->frmReply.byData[3],
			pCCU->frmReply.byData[4],
			pCCU->frmReply.byData[5],
			pCCU->frmReply.byData[6],
			pCCU->frmReply.byData[7]);
		}	
	}

	// TimeToString(time(NULL), "%H:%M:%S", szTime, sizeof(szTime));
	
	// memset(pCCU->szSettCmdReply, 0, 64);
	// nOffset += sprintf(pCCU->szSettCmdReply + nOffset, "%s|", szTime);

	// for(n = 0; n < MIN(usLenID,26); n++)
	// {
	// 	//printf("%02X-",pData[n]);
	// 	if(n == 14)
	// 	{
	// 		nOffset += sprintf(pCCU->szSettCmdReply + nOffset, "|");
	// 	}
	// 	nOffset += sprintf(pCCU->szSettCmdReply + nOffset, "%02X",pData[n]);
	// }
 
	// pCCU->szSettCmdReply[nOffset] ='\0';
	// TRACE("nOffset=[%d] pCCU->szSettCmdReply=[%s]\n", nOffset, pCCU->szSettCmdReply);

	// Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 104, (void*)&pCCU->szSettCmdReply,  64);

	// if(usLenID < 8 )
	// {
	// 	return -1;
	// }

	// if(rtn != 0)
	// {
	// 	return -1;
	// }

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : 0x24 To Set PCS Parameter
*  CREATOR       : John
*  CREATED DATE  : 2022-7-14 11:22:53
*  DESCRIPTION   :If Return Back 0x24XXXX value is Enum, Should Process To [0,1,2,3].
* 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void ParseCanFrame_24XXXX0001(IN ROUGH_CCU_DATA *pCCU, IN A_CAN_RECT_FRAME *pFrmReply)
{
	int i = 0;
	TRACE("byErrCode=%d byDevNo=%d, byCmdNo=%d, bySrcAddr=%d, byDestAddr=%d\n",
		  pFrmReply->byErrCode, pFrmReply->byDevNo, pFrmReply->byCmdNo, pFrmReply->bySrcAddr, pFrmReply->byDestAddr);
	int nWhichModule = (int)pFrmReply->bySrcAddr;
	float fPd;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 200, 223, &fPd, 4);
	fPd *= 10;
	/*
	int nIdx = pFrmReply->bySrcAddr;
	if ((nIdx < 0) && (nIdx >= pCCU->nMaxRectNumToSample))
	{
		printf("Read Sett Param :ParseCanFrame\r\n");
		TRACE("Err: Wrong DstAddr=%d\n", pFrmReply->bySrcAddr);
		return;
	}

	if ((pFrmReply->byData[0] == 0x21) && (pFrmReply->byData[1] == 0x20)) //_CAN_S_CMD_24_21_20
	{
		pCCU->PCSRereadParam[nWhichModule].fL1_overvolt_propot = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
		//printf("Rect fL1_overvolt_propot = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fL1_overvolt_propot);
	}
	else if ((pFrmReply->byData[0] == 0x21) && (pFrmReply->byData[1] == 0x21)) //_CAN_S_CMD_24_21_21
	{
		pCCU->PCSRereadParam[nWhichModule].fL1_overvolt_proTM = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001; // 2121
		//printf("Rect fL1_overvolt_proTM = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fL1_overvolt_proTM);
	}
	else if ((pFrmReply->byData[0] == 0x21) && (pFrmReply->byData[1] == 0x22)) //_CAN_S_CMD_24_21_22
	{
		pCCU->PCSRereadParam[nWhichModule].fL2_overvolt_propot = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001; // 2122
		//printf("Rect fL2_overvolt_propot = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fL2_overvolt_propot);
	}
	*/
	//---------------回读数据解析开始---------------//
	// 0x10 分支
	if(pFrmReply->byData[0]== 0x10)
	{
		switch(pFrmReply->byData[1])
		{
			case 0x01: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam1001 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam1001 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam1001);
				}
				break;
			case 0x02: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam1002 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam1002 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam1002);
				}
				break;
			default:
				{
					break;
				}
		}
	}

	// 0x11 分支
	if(pFrmReply->byData[0]== 0x11)
	{
		switch(pFrmReply->byData[1])
		{
			case 0x10: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam1110 = (int)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam1110 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam1110);
				}
				break; 
			case 0x27:
				{
					pCCU->PCSRereadParam[nWhichModule].nParam1127 = (int)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam1127 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].nParam1127);
				}
				break;
			case 0x28:
				{
					pCCU->PCSRereadParam[nWhichModule].nParam1128 = (int)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam1127 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].nParam1128);
				}
				break;
			case 0x33:
				{
					pCCU->PCSRereadParam[nWhichModule].fParam1133 = (int)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect fParam1133 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam1133);
				}
				break;
			default:
				{
					break;
				}
		}
	}

	// ========== 0x20 分支 ==========//
	if(pFrmReply->byData[0]== 0x20)
	{
		switch(pFrmReply->byData[1])
		{
			case 0x03: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2003 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.000001;
					TRACE("Rect fParam2003 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2003);
				}
				break;
			default:
				{
					break;
				}
		}
	}

	// ========== 0x21 分支 ==========//
	if(pFrmReply->byData[0]== 0x21)
	{
		switch(pFrmReply->byData[1])
		{
			case 0x01: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2101 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2101 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2101);
				}
				break;
			case 0x02: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2102 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2102 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2102);
				}
				break;
			case 0x03: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2103 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.004;
					TRACE("Rect fParam2103 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2103);
				}
				break;
			case 0x05: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2105 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2105 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2105);
				}
				break;
			case 0x06: 
				{
					pCCU->PCSRereadParam[nWhichModule].uParam2106 = (UINT)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect uParam2106 = %d\n", pCCU->PCSRereadParam[nWhichModule].uParam2106);
				}
				break;
			case 0x07: 
				{
					pCCU->PCSRereadParam[nWhichModule].uParam2107 = (UINT)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect uParam2107 = %d\n", pCCU->PCSRereadParam[nWhichModule].uParam2107);
				}
				break;
			case 0x10: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam2110 = (UINT)pFrmReply->byData[7] & 0x0f ;
					TRACE("Rect nParam2110 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam2110);
				}
				break;
			case 0x11: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam2111 = (UINT)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam2111 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam2111);
				}
				break;
			case 0x14: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam2114 = (UINT)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam2114 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam2114);
				}
				break;
			case 0x15: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam2115 = (UINT)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam2115 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam2115);
				}
				break;
			case 0x17: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam2117 = (UINT)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam2117 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam2117);
				}
				break;
			case 0x18: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam2118 = (UINT)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam2118 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam2118);
				}
				break;
			case 0x20: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2120 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2120 = %3f\n", pCCU->PCSRereadParam[nWhichModule].fParam2120);

				}
				break;
			case 0x21: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2121 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001f;
					TRACE("Rect fParam2121 = %.3f\n", pCCU->PCSRereadParam[nWhichModule].fParam2121);
				}
				break;
			case 0x22: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2122 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2122 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2122);
				}
				break;
			case 0x23: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2123 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2123 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2123);
				}
				break;
			case 0x24: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2124 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2124 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2124);
				}
				break;
			case 0x25: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2125 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2125 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2125);
				}
				break;
			case 0x26: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2126 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2126 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2126);
				}
				break;
			case 0x27: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2127 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2127 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2127);
				}
				break;
			case 0x28: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2128 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2128 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2128);
				}
				break;
			case 0x29: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2129 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2129 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2129);
				}
				break;
			case 0x2A: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam21210 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam21210 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam21210);
				}
				break;
			case 0x2B: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam21211 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam21211 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam21211);
				}
				break;
			case 0x2C: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam21212 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam21212 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam21212);
				}
				break;
			case 0x2D: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam21213 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam21213 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam21213);
				}
				break;
			case 0x2E: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam21214 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam21214 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam21214);
				}
				break;
			case 0x2F: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam21215 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam21215 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam21215);
				}
				break;
			case 0x30: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam2130 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam2130 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam2130);
				}
				break;
			case 0x40: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam2140 = pFrmReply->byData[7] & 0x0f;
					TRACE("Rect bParam2140 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam2140);
				}
				break;
			case 0x41: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam2141 = pFrmReply->byData[7] & 0x0f;
					TRACE("Rect bParam2141 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam2141);
				}
				break;
			case 0x42: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam2142 = pFrmReply->byData[7] & 0x0f;
					TRACE("Rect bParam2142 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam2142);
				}
				break;
			case 0x43: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam2143 = pFrmReply->byData[7] & 0x0f;
					TRACE("Rect bParam2143 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam2143);
				}
				break;
			case 0x44: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam2144 = pFrmReply->byData[7] & 0x0f;
					TRACE("Rect bParam2144 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam2144);
				}
				break;
			case 0x45: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam2145 = (int)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect nParam2145 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam2145);
				}
				break;
			case 0x46: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam2146 = (int)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect nParam2146 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam2146);
				}
				break;
			default:
				{
					break;
				}
		}
	}

	// ========== 0x31 分支 ==========//
	if(pFrmReply->byData[0]== 0x31)
	{
		switch(pFrmReply->byData[1])
		{
			case 0x11: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam3111 = (int)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect bParam3111 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam3111);
				}
				break;
			case 0x12: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam3112 = (int)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect bParam3112 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam3112);
				}
				break;
			case 0x13: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3113 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3113 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3113);
				}
				break;
			case 0x14: 
				{
					pCCU->PCSRereadParam[nWhichModule].uParam3114 = (int)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect nParam3114 = %d\n", pCCU->PCSRereadParam[nWhichModule].uParam3114);
				}
				break;
			case 0x15: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3115 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3115 = %f\n", pCCU->PCSRereadParam[nWhichModule].fParam3115);
				}
				break;
			case 0x16: 
				{
					pCCU->PCSRereadParam[nWhichModule].uParam3116 = (int)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect nParam3116 = %d\n", pCCU->PCSRereadParam[nWhichModule].uParam3116);
				}
				break;
			case 0x17: 
				{
					pCCU->PCSRereadParam[nWhichModule].uParam3117 = (int)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect nParam3117 = %d\n", pCCU->PCSRereadParam[nWhichModule].uParam3117);
				}
				break;
			case 0x18: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3118 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam3118 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3118);
				}
				break;
			case 0x19: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3119 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam3119 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3119);
				}
				break;
			case 0x1A: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31110 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam31110 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31110);
				}
				break;
			case 0x1B: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31111 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam31111 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31111);
				}
				break;
			case 0x1D: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31113 = (int)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect bParam31113 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31113);
				}
				break;
			case 0x21: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3121 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3121 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3121);
				}
				break;
			case 0x22: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3122 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3122 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3122);
				}
				break;
			case 0x23: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3123 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.000001;
					TRACE("Rect fParam3123 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3123);
				}
				break;
			case 0x24: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3124 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3124 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3124);
				}
				break;
			case 0x25: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3125 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3125 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3125);
				}
				break;
			case 0x26: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3126 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3126 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3126);
				}
				break;
			case 0x27: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3127 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3127 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3127);
				}
				break;
			case 0x28: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3128 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3128 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3128);
				}
				break;
			case 0x29: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3129 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3129 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3129);
				}
				break;
			case 0x2A: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31281 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31281 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31281);
				}
				break;
			case 0x2B: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31211 = (float)CAN_StringToInt(pFrmReply->byData + 4) / 3000;
					TRACE("Rect fParam31211 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31211);
				}
				break;
			case 0x2E: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31156 = (int)pFrmReply->byData[7] & 0x0f;
					TRACE("Rect bParam31156 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31156);
				}
				break;
			case 0x2F: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31315 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam31315 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31315);
				}
				break;
			case 0x31: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3131 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam3131 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3131);
				}
				break;
			case 0x32: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3132 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam3132 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3132);
				}
				break;
			case 0x33: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3133 = (float)(100.0f / (CAN_StringToInt(pFrmReply->byData + 4) / 20));
					TRACE("Rect fParam3133 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3133);
				}
				break;
			case 0x35: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3135 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3135 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3135);
				}
				break;
			case 0x36: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3136 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam3136 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3136);
				}
				break;
			case 0x38: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3138 = (float)(100.0f / (CAN_StringToInt(pFrmReply->byData + 4) / 20));
					TRACE("Rect fParam3138 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3138);
				}
				break;
			case 0x3A: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31310 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31310 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31310);
				}
				break;
			case 0x3F: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam313151 = (float)CAN_StringToInt(pFrmReply->byData + 4) / 3000;
					TRACE("Rect fParam313151 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam313151);
				}
				break;
			case 0x50: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3150 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3150 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3150);
				}
				break;
			case 0x51: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3151 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3151 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3151);
				}
				break;
			case 0x52: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3152 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3152 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3152);
				}
				break;
			case 0x53: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3153 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3153 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3153);
				}
				break;
			case 0x54: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3154 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3154 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3154);
				}
				break;
			case 0x55: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3155 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3155 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3155);
				}
				break;
			case 0x56: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3156 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3156 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3156);
				}
				break;
			case 0x57: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3157 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3157 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3157);
				}
				break;
			case 0x60: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3160 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam3160 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3160);
				}
				break;
			case 0x61: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam3161 = (int)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect nParam3161 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam3161);
				}
				break;
			case 0x63: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3163 = (float)CAN_StringToInt(pFrmReply->byData + 4) / 3000;
					TRACE("Rect fParam3163 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3163);
				}
				break;
			case 0x64: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3164 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3164 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3164);
				}
				break;
			case 0x65: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3165 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3165 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3165);
				}
				break;
			case 0x66: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3166 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3166 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3166);
				}
				break;
			case 0x67: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3167 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3167 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3167);
				}
				break;
			case 0x68: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3168 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3168 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3168);
				}
				break;
			case 0x69: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3169 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam3169 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3169);
				}
				break;
			case 0x6A: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31610 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam31610 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31610);
				}
				break;
			case 0x6B: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31611 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam31611 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31611);
				}
				break;
			case 0x6C: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31612 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam31612 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31612);
				}
				break;
			case 0x6D: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31613 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam31613 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31613);
				}
				break;
			case 0x6E: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31614 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam31614 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31614);
				}
				break;
			case 0x6F: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31615 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam31615 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31615);
				}
				break;
			case 0x73: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3173 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					pCCU->PCSRereadParam[nWhichModule].fParam3173 /= fPd;
					TRACE("Rect fParam3173 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3173);
				}
				break;
			case 0x74: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3174 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					pCCU->PCSRereadParam[nWhichModule].fParam3174 /= fPd;
					TRACE("Rect fParam3174 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3174);
				}
				break;
			case 0x75: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3175 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					pCCU->PCSRereadParam[nWhichModule].fParam3175 /= fPd;
					TRACE("Rect fParam3175 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3175);
				}
				break;
			case 0x76: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3176 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					pCCU->PCSRereadParam[nWhichModule].fParam3176 /= fPd;
					TRACE("Rect fParam3176 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3176);
				}
				break;
			case 0x77: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3177 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					pCCU->PCSRereadParam[nWhichModule].fParam3177 /= fPd;
					TRACE("Rect fParam3177 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3177);
				}
				break;
			case 0x78: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3178 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					pCCU->PCSRereadParam[nWhichModule].fParam3178 /= fPd;
					TRACE("Rect fParam3178 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3178);
				}
				break;
			case 0x79: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3179 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam3179 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3179);
				}
				break;
			case 0x7A: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31710 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam31710 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31710);
				}
				break;
			case 0x7B: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31711 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam31711 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31711);
				}
				break;
			case 0x7C: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31712 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam31712 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31712);
				}
				break;
			case 0x7D: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31713 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam31713 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31713);
				}
				break;
			case 0x7E: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31714 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam31714 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31714);
				}
				break;
			case 0x80: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3180 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3180 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3180);
				}
				break;
			case 0x81: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3181 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3181 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3181);
				}
				break;
			case 0x82: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3182 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3182 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3182);
				}
				break;
			case 0x83: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3183 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3183 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3183);
				}
				break;
			case 0x84: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3184 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3184 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3184);
				}
				break;
			case 0x85: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3185 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3185 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3185);
				}
				break;
			case 0x86: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3186 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3186 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3186);
				}
				break;
			case 0x87: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3187 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3187 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3187);
				}
				break;
			case 0x88: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3188 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3188 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3188);
				}
				break;
			case 0x89: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3189 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3189 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3189);
				}
				break;
			case 0x8A: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31810 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31810 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31810);
				}
				break;
			case 0x8B: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam31811 = pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam31811 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam31811);

				}
				break;
			case 0x8C: 
				{
					pCCU->PCSRereadParam[nWhichModule].nParam31812 =  pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam31812 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam31812);
				}
				break;
			case 0x8D: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31813 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31813 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31813);
				}
				break;
			case 0x8E: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31814 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31814 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31814);
				}
				break;
			case 0x90: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3190 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3190 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3190);
				}
				break;
			case 0x91: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3191 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3191 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3191);
				}
				break;
			case 0x92: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3192 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3192 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3192);
				}
				break;
			case 0x93: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3193 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3193 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3193);
				}
				break;
			case 0x94: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3194 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3194 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3194);
				}
				break;
			case 0x95: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3195 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3195 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3195);
				}
				break;
			case 0x96: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3196 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3196 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3196);
				}
				break;
			case 0x97: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3197 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3197 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3197);
				}
				break;
			case 0x98: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3198 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3198 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3198);
				}
				break;
			case 0x99: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3199 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3199 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3199);
				}
				break;
			case 0x9A: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31910 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31910 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31910);
				}
				break;
			case 0x9B: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31911 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31911 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31911);
				}
				break;
			case 0x9C: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31912 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31912 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31912);
				}
				break;
			case 0x9E: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31914 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31914 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31914);
				}
				break;
			case 0x9F: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31915 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31915 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31915);
				}
				break;
			case 0xAB: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam311011 = pFrmReply->byData[7] & 0x0f;
					TRACE("Rect uParam311011 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam311011);
				}
				break;
			case 0xBB: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3111111 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam3111111 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3111111);
				}
				break;
			case 0xC1: 
				{
					pCCU->PCSRereadParam[nWhichModule].uParam31121 = (int)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect uParam31121 = %d\n", pCCU->PCSRereadParam[nWhichModule].uParam31121);
				}
				break;
			case 0xD0: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31130 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam31130 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31130);
				}
				break;
			case 0xD1: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31131 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam31131 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31131);
				}
				break;
			case 0xD2: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31132 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam31132 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31132);
				}
				break;
			case 0xD3: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31133 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam31133 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31133);
				}
				break;
			case 0xD6: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31136 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam31136 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31136);
				}
				break;
			case 0xD8: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31138 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam31138 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31138);
				}
				break;
			case 0xD9: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31139 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam31139 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31139);
				}
				break;
			case 0xDD: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam311313 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam311313 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam311313);
				}
				break;
			case 0xDF: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam311315 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam311315 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam311315);
				}
				break;
			case 0xF3: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31234 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31234 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31234);
				}
				break;
			case 0xF6: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam311561 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam311561 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam311561);
				}
				break;
			case 0xF7: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31157 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam31157 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31157);
				}
				break;
			case 0xF8: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam31158 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam31158 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam31158);
				}
				break;
			case 0xF9: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31159 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam31159 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31159);
				}
				break;
			case 0xFA: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31250 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31250 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31250);
				}
				break;
			case 0xFB: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam31251 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					TRACE("Rect fParam31251 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam31251);
				}
				break;
			case 0xFC: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam311512 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.01;
					TRACE("Rect fParam311512 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam311512);
				}
				break;
			default:
				{
					break;
				}
		}
	}

	// ========== 0x51 分支 ==========//
	if(pFrmReply->byData[0]== 0x51)
	{
		switch(pFrmReply->byData[1])
		{
			case 0x01: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam5101 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam5101 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam5101);
				}
				break;
			case 0x02: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam5102 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam5102 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam5102);
				}
				break;
			case 0x03: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam5103 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam5103 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam5103);
				}
				break;
			case 0x04: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam5104 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam5104 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam5104);
				}
				break;
			case 0x05: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam5105 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.001;
					TRACE("Rect fParam5105 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam5105);
				}
				break;
			default:
				{
					break;
				}
		}
	}

	// ========== 0x71 分支 ==========//
	if(pFrmReply->byData[0]== 0x71)
	{
		switch(pFrmReply->byData[1])
		{
			case 0x40: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam7140 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam7140 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam7140);
				}
				break;
			case 0x4B: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam71411 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam71411 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam71411);
				}
				break;
			case 0x4C: 
				{
					pCCU->PCSRereadParam[nWhichModule].bParam71412 = pFrmReply->byData[7] & 0x0f; 
					TRACE("Rect uParam71412 = %d\n", pCCU->PCSRereadParam[nWhichModule].bParam71412);
				}
				break;
			default:
				{
					break;
				}
		}
	}

	// ========== 0x32 分支 ==========//
	if(pFrmReply->byData[0]== 0x32)
	{
		switch(pFrmReply->byData[1])
		{
			case 0x00: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam32001 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					pCCU->PCSRereadParam[nWhichModule].fParam32001 /= fPd;
					TRACE("Rect fParam32001 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam32001);
				}
				break;
			case 0x01: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3201 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					pCCU->PCSRereadParam[nWhichModule].fParam3201 /= fPd;
					TRACE("Rect fParam3201 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3201);
				}
				break;
			case 0x02: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3202 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					pCCU->PCSRereadParam[nWhichModule].fParam3202 /= fPd;
					TRACE("Rect fParam3202 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3202);
				}
				break;
			case 0x03: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3203 = (float)CAN_StringToInt(pFrmReply->byData + 4);
					pCCU->PCSRereadParam[nWhichModule].fParam3203 /= fPd;
					TRACE("Rect fParam3203 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3203);
				}
				break;
			case 0x04: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3204 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam3204 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3204);
				}
				break;
			case 0x05: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3205 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam3205 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3205);
				}
				break;
			case 0x06: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3206 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;				
					TRACE("Rect fParam3206 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3206);
				}
				break;
			case 0x07: 
				{
					pCCU->PCSRereadParam[nWhichModule].fParam3207 = (float)CAN_StringToInt(pFrmReply->byData + 4) * 0.1;
					TRACE("Rect fParam3207 = %.1f\n", pCCU->PCSRereadParam[nWhichModule].fParam3207);
				}
				break;
			default:
			{
				break;
			}
		}
	}
	// ========== 0x91 分支 ==========//
	if(pFrmReply->byData[0]== 0x91)
	{
		switch(pFrmReply->byData[1])
		{
			case 0x0B:
				{
					pCCU->PCSRereadParam[nWhichModule].nParam91011 = pFrmReply->byData[7] & 0x0f;
					TRACE("Rect nParam91011 = %d\n", pCCU->PCSRereadParam[nWhichModule].nParam91011);
				}
				break;
			default:
				{
					break;
				}
		}
	}

//---------------回读数据解析完成---------------//
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : 0x24 To Read PCS Settings Parameter
*  CREATOR       : John
*  CREATED DATE  : 2022-12-7 11:22:53
*  DESCRIPTION   : 
	Byte0~1:Protocal SubCmd 	
	Byte2:00-normal Cmd,01-Get PCS Sett Param.
	Byte3:0x00
	Byte4~7:Data Area;

	CMD = 0x24, DataLen = 1(RTN) +1(ResponseNum)+12* ResponseNum {ReponseData}, 
	ReponseData =CAN Head 4 Bytes+ Data Area 8 Bytes.

*  Note: Use PM Protocal SubCmd=0x10 Group—Cmd,PM Response also GRoup-Cmd;
	Other SubCmd,PM Use Single PM Addr Response.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_Read_PCSSett(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	BYTE rtn = pData[0];
	BYTE byCanMsgNum = pData[1];
	UINT n = 0;

	for(n = 0; n < usLenID; n++)
	{
		printf("%02X-",pData[n]);
	}
	printf("Parse_Read_PCSSett]\n");

	if(usLenID < 8 )
	{
		return -1;
	}

	if(rtn != 0)
	{
		return -1;
	}
	
 	USHORT 	nReadLen = usLenID - 2;
 	BYTE   	*byRecvBuff = &pData[2];
	if(nReadLen < CAN_FRAME_DATALEN)
	{
		return -1;
	}

	for(n = 0; n < (nReadLen / CAN_FRAME_DATALEN); n++)
	{
		memcpy(pCCU->frmReply.byRawData, byRecvBuff + n * CAN_FRAME_DATALEN, CAN_FRAME_DATALEN);
		
		CAN_ParseHead4PCS(&pCCU->frmReply);

		TRACE("byDestAddr=%02X byCmdNo =%02X byData[0]=%02X byData[1]=%02X\n",
			pCCU->frmReply.byDestAddr,
			pCCU->frmReply.byCmdNo, pCCU->frmReply.byData[0],
			pCCU->frmReply.byCmdNo, pCCU->frmReply.byData[1]);

		if((pCCU->frmReply.byDestAddr == R_PROTO_SELF_ADDR) &&
			/*(pCCU->frmReply.bySrcAddr == pCCU->frmSent.byDestAddr) &&*/
			(pCCU->frmReply.byCmdNo == pCCU->frmSent.byCmdNo))
		{
			if(pCCU->frmReply.bySrcAddr != pCCU->frmSent.byDestAddr)
			{
				TRACE("Alarm: bySrcAddr=%d byDestAddr=%d frmReply.byCmdNo=%d pCCU->frmSent.byCmdNo=%d\n",
					pCCU->frmReply.bySrcAddr,
					pCCU->frmSent.byDestAddr,
					pCCU->frmReply.byCmdNo,
					pCCU->frmSent.byCmdNo);
				//continue; //!!! Need Judge SendCmd SRC GroupID !!!
			}
			TRACE("Info: byErrCode=%02X bySrcAddr=%02X byDestAddr=%02X frmReply.byCmdNo=%02X pCCU->frmSent.byCmdNo=%02X\n",
				  pCCU->frmReply.byErrCode,\	
				  pCCU->frmReply.bySrcAddr,
				  pCCU->frmSent.byDestAddr,
				  pCCU->frmReply.byCmdNo,
				  pCCU->frmSent.byCmdNo);
 
			if(pCCU->frmReply.byCmdNo != 0x23)
			{
				for (n = 0; n < CAN_FRAME_DATALEN; n++)
				{
					printf("%02X-", pCCU->frmReply.byRawData[n]);
				}
				printf("\n");
			}

	  		if(pCCU->frmReply.byCmdNo == 0x24)
			{
				ParseCanFrame_24XXXX0001(pCCU, &pCCU->frmReply);
				TRACE("Info Cmd 24: Reply:byData[4]=%02X|byData[5]=%02X|byData[6]=%02X|byData[7]=%02X\
					SendData: byData[4]=%02X|byData[5]=%02X|byData[6]=%02X|byData[7]=%02X\n",
					pCCU->frmReply.byData[4],
					pCCU->frmReply.byData[5],
					pCCU->frmReply.byData[6],
					pCCU->frmReply.byData[7],
					pCCU->frmSent.byData[4],
					pCCU->frmSent.byData[5],
					pCCU->frmSent.byData[6],
					pCCU->frmSent.byData[7]);
					// printf("%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",\
					// pCCU->frmReply.byData[0],\
					// pCCU->frmReply.byData[1],\
					// pCCU->frmReply.byData[2],\
					// pCCU->frmReply.byData[3],
					// pCCU->frmSent.byData[4],\
					// pCCU->frmSent.byData[5],\
					// pCCU->frmSent.byData[6],\
					// pCCU->frmSent.byData[7]);
 					break; //Only Judge One Single PM Settings.
			}	
		}
	}
 
	return 0;
}

void Judge_IfNeedQueryPCS_Settings(ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;

	/*READ_PCS_SETTPARAM readSett[] =
	{
		{SETT_PCS_ID_1127,	1127,		1,  	PCS_TYPE_ENUM,			0x11, 0x27}, 
		{SETT_PCS_ID_1133, 	1133,		1,  	PCS_TYPE_ENUM,			0x11, 0x33},
		{SETT_PCS_ID_2101,	2101,	  	1000,  	PCS_TYPE_FLOAT,			0x21, 0x01},
		{SETT_PCS_ID_2102, 	2102,		1000, 	PCS_TYPE_FLOAT,			0x21, 0x02},
		{SETT_PCS_ID_2105,	2105,	  	1000,  	PCS_TYPE_FLOAT,			0x21, 0x05},
	}*/	 

	UINT nDevId = 701;
	UINT nParamId = 1127;
	varV.lValue = 0;

	VAR_VALUE *pVarVal = &varV;

	// On_Handle_PCSSettingCmd(pCCU, nDevId, nParamId, pVarVal, FALSE);
	


	return;
}
