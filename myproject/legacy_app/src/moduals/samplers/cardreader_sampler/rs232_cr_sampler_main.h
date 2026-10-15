#ifndef __CR_232_SAMPLER_MAIN_H
#define __CR_232_SAMPLER_MAIN_H

#include "../sampler_base.h"

#define MAX_CR_WAIT_SAMPLE		200

//typedef enum tagCARD_TYPE
//{
//	S50_CARD,
//	S70_CARD,
//}CARD_TYPE;

//MT means Ming Te
typedef struct tagMTFrame
{
	BYTE		STX;
	USHORT	usDataLen;
	BYTE		byCmd;
	BYTE		byCmdArg;
	BYTE		pbyData[19]; //Max 18: 1 secNo + 1 blkNo + statusP, 16bytes data 
	BYTE		ETX;
	BYTE		BCC;
}MT_FRAME;

typedef enum tagCardReader_FRAME_TYPE
{
	CARDREADER_FRAME_TYPE_SERACH_CARD = 0,
	CARDREADER_FRAME_TYPE_VAILIDATE_SEC0_KEYA,
	CARDREADER_FRAME_TYPE_READ_SEC0_BLK0,
	CARDREADER_FRAME_TYPE_VAILIDATE_SEC3_KEYA,
	CARDREADER_FRAME_TYPE_READ_SEC3_BLK0,
	CARDREADER_FRAME_TYPE_VAILIDATE_SEC5_KEYA,
	CARDREADER_FRAME_TYPE_READ_SEC5_BLK0,
	CARDREADER_FRAME_TYPE_WRITE_MONEY,
	_MAX_CARDREADER_FRAME_TYPE
}CARDREADER_FARAME_TYPE;

#define MT_FRAME_ANSWERLEN_MIN	7
typedef struct tagACARDREADERFrameInfo
{
	CARDREADER_FARAME_TYPE	frameType;
	MT_FRAME					sendFrame;
	USHORT					usFrameLen; //this is the real len used by app
	USHORT					usExpectAnswerLen;

	BYTE						byParsedSecBlkData[16];//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸嵎杈炬嫹閿熺蛋閿熸枻鎷烽敓鏂ゆ嫹y閿熸枻鎷烽敓锟�16閿熻鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹/**/
}CARDREADER_FRAME_INFO;

typedef enum tagCRAuthrizeStatus
{
	AUTH_STATUS_NONE,	//not in authorization process
	AUTH_STATUS_ONGOING,
	AUTH_STATUS_ACCEPTED,
	AUTH_STATUS_REJECTED
}AUTHO_STATUS;

struct stCR232_ROUGH
{
	DATA_PROCESSOR* pDPR;
	COMMPORT*	pComm;
	HANDLE		hSampler; //pointer to sampler
	HANDLE		hPort; //from RS_232_Open
	HANDLE		hSamplerLock;
	//int			nSamplerId;//From sampler info
	BYTE			abySendBuf[RS485_SEND_BUF_LEN];
	BYTE			abyRcvBuf[RS485_RECV_BUF_LEN];
	int			nReadBuffBytes; //Bytes that have been readout

	BOOL			bIsCardReaderInstalled; //can be set to NO via factory

	BOOL			bIsLocalAuthorizeDisabled;//if Local Authorize is diabled, then do not serach card

	CARDREADER_FRAME_INFO	frm_cardReaderFrames[_MAX_CARDREADER_FRAME_TYPE];

	time_t			tm_LastSuccessCard; //last time we succeeded in sweeping card

	// raw data from new coming card
	CHARGE_START_MOTHOD	emStartMethod;
	char				szRawCardId[64];
	char				szRawParentId[64];
	float				fRawLeftMoney;

	BOOL				bIsChargerAvailable; //locked/faulted means unavailable

	AUTHO_STATUS		emAuthorizeStatus;

	int				nAuthorizationTimeouT_Sec; //this also include when authorizing done, wait max time for none-operation

	//put all this into Reporter to manage?
	A_BLOB_LOCALLIST			stLocalList;
	A_BLOB_AUTHORIZECACHE	stAuthCache;

	A_BLOB_ES_iOCPP_CONFIGURAION	stESiOCPPConfigs;

	ESSYS_TYPE			emESSystemType;
	int					nWhichClientAuth;
	BOOL				bHasSearchCard;
	time_t				tm_LastRecvCardAuth;
	BOOL				bCommWithMasterOK;

	BOOL				bServerCommOK;
};
typedef struct stCR232_ROUGH		CR232_SAMPLER_DATA;

int Sample_MTFrame(CR232_SAMPLER_DATA *pCR_Rough, CARDREADER_FRAME_INFO* pFrame);
void CardReader_InitRoughData(CR232_SAMPLER_DATA *pCR_Rough);

#endif


