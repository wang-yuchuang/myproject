#include "rs232_cr_sampler_main.h"

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	CR232_SAMPLER_DATA *pCR_Rough = (CR232_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	//if(emEvType == EVENT_TYPE_USER_AUTHORIZE)
	//{
	//	if(nEvDataSize != sizeof(EVENT_AUTHORIZE))
	//	{
	//		return EVENT_HANDED_FAILED;
	//	}
	//	else if(emMsgType != EVENT_MSG_STATUS_CHANGED)
	//	{
	//		return EVENT_HANDED_FAILED;
	//	}
	//	//now parse authorization result
	//	EVENT_AUTHORIZE* pEvAuth = (EVENT_AUTHORIZE*)pEvData;
	//	if((strcmp(pEvAuth->szIdTocken, pCR_Rough->szRawCardId) != 0) || (pCR_Rough->emAuthorizeStatus != AUTH_STATUS_ONGOING))
	//	{
	//		return EVENT_HANDED_FAILED;
	//	}
	//	//now notice the result to sampler
	//	pCR_Rough->emAuthorizeStatus = (pEvAuth->emResult == ACCEPTED) ? AUTH_STATUS_ACCEPTED : AUTH_STATUS_REJECTED;

	//	//and send out the event?
	//	EVENT_AUTHORIZE evAuth;
	//	memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
	//	memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
	//	evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
	//	evAuth.nWhereFrom = AUTH_SRC_LOCAL;
	//	evAuth.nWhereDoAuthorizing = AUTH_DO_OCPP;
	//	evAuth.emResult = (pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ? ACCEPTED : BLOCKED;

	//	EVENT_ARG eventArg;
	//	ZERO_POBJS(&eventArg, 1);
	//	eventArg.emEvType = EVENT_TYPE_USER_AUTHORIZE;
	//	eventArg.emMsgType = EVENT_MSG_END;
	//	eventArg.nEvDataSize = sizeof(EVENT_AUTHORIZE);
	//	eventArg.pEvData = (void*)&evAuth;
	//	//none block
	//	eventArg.bBlockMode = FALSE;
	//	pCR_Rough->pDPR->Set(pCR_Rough->hSampler, SETTYPE_SENDMSG,
	//		&eventArg, sizeof(EVENT_ARG));

	//}
	if(emEvType == EVENT_TYPE_CFG_CHANGED)
	{
		if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
		{
			return EVENT_HANDED_FAILED;
		}
		EVENT_PARAM_CHG *pCfgChange = (EVENT_PARAM_CHG*)pEvData;
		if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 26))
		{
			TRACEX("CR Received OCPP setting change Event\n");
			//OCPP setting has changed
			varV.blobValue = (BYTE *)&pCR_Rough->stESiOCPPConfigs;
			//copy setting to current data
			ERRCDDE_DPR err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				1, 26, &varV, sizeof(A_BLOB_ES_iOCPP_CONFIGURAION));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 19))
		{
			TRACEX("CR Received LocalAuthEnabled setting change Event\n");
			ERRCDDE_DPR err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, 1, 19, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pCR_Rough->bIsLocalAuthorizeDisabled = (varV.emValue == 1) ? TRUE : FALSE;
		}
	}

	return EVENT_HANDED_OK;
}

DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
	//闂傚倷娴囧▔鏇㈠窗閺嶎厼围婵炲樊浜滃Λ姗€鏌曢崼婵囶棞妞ゅ繐鐖煎铏规崉閵娿儲鐎鹃梺鍝勵儏椤兘鐛箛娑欏€婚柤鎭掑劜濞呫垽姊洪悷鐗堣础闁哄苯顦甸獮鎾活敂閸繄顔嗛梺缁樺灱婵倝寮查幖浣圭厸闁稿本锚閳ь剚鐗滈埀顒佽壘缂嶅﹪寮婚妸鈺傚亜闁告稑锕︽导鍕⒑瑜版帩妫戦柛蹇旓耿瀵偊骞樼拠鍙夘棟闂侀潧鐗嗗Λ妤咁敂閿燂拷/**/
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;

	pSampler->pDataHeap = (void*)NEW(CR232_SAMPLER_DATA, 1);
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}

	CR232_SAMPLER_DATA *pCR_Rough = (CR232_SAMPLER_DATA*)pSampler->pDataHeap;

	pCR_Rough->hSampler = hSamplerSelf;

	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	pCR_Rough->pDPR = pDPR;

	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},
		//{EVENT_TYPE_USER_AUTHORIZE,	EVENT_MSG_END}, 
		//{EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END},
		//{EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_BEGIN},
	};
	int n;
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pDPR->Register_EvHandler("CR_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}


	pCR_Rough->hSamplerLock = pSampler->hSamplerLock; //Init lock
	//Mutex_Destroy(pSamplerInfo->hSamplerLock); //闂傚倷娴囧▔鏇㈠窗閹版澘鍑犲┑鐘宠壘缁狀垶鏌ｉ幋锝呅撻柡鍛倐閺岋繝宕掑Ο琛″亾閺嶎偀鍋撳顒傜Ш闁哄被鍔戦幃銏ゅ川婵犲嫪绱曢梻浣哥秺椤ユ捇宕楀鈧顐﹀箻鐠囧弶顥濋梺闈涚墕濡顢旈崼鏇熲拺閻犳亽鍔岄弸鏃傜磼椤旇姤鍊愰柟顖氬€垮畷銊︾節閸屻倓绱﹂梻浣藉亹閳峰牓宕戞繝鍥ф瀬闁瑰墽绮崕妤併亜閹板墎纾垮☉鎾寸叀閺岀喐顦版惔鈥冲箣闂佽桨鐒﹂幑鍥ь嚕椤掑嫬围闁糕剝顨忔导鎾绘⒒娴ｈ姤纭堕柛锝忕畵楠炲繘鏁撻敓锟�/**/
	//pCR_Rough->nSamplerId = pSampler->nSamplerID;

	//Initialize rough data and some parameter. 
	CardReader_InitRoughData(pCR_Rough);

	//Open
	int nErrCode = 0;
	pCR_Rough->pComm = pSampler->pUsedCommPort;
	pCR_Rough->hPort = pSampler->pUsedCommPort->Open(pSampler->pUsedCommPort->info.szPortAttri, 
		pSampler->info.szSamplerSetting,
		COMM_CLIENT_MODE, 500, &nErrCode);
	if(ERR_COMM_OK != nErrCode)
	{
		TRACEX("CR232 comm-port canNOT be opened!\n");
		return FALSE;
	}

	//Now pull system type setting
	
	pCR_Rough->emESSystemType = GetESSysType(hSamplerSelf);
	return TRUE;
}

DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	return 0;
}

static BOOL IsChkValid(BYTE *pbySrcChk, BYTE* pbyRomdNum, BYTE *pbyDstChk)
{
	int n;
	for(n = 0; n < 4; n++)
	{
		if(((pbySrcChk[n] )^ (pbyRomdNum[n])) != pbyDstChk[n])
		{
			return FALSE;
		}
	}
	return TRUE;
}

static ULONG Swap4Bytes(BYTE* pByte)
{
	VAR_VALUE varV;
	varV.abyValue[0] = pByte[0];
	varV.abyValue[1] = pByte[1];
	varV.abyValue[2] = pByte[2];
	varV.abyValue[3] = pByte[3];
	return varV.ulValue;
}

static char ConvBCD(BYTE byVal)
{
	if(byVal <= 9)
	{
		return '0' + byVal;
	}
	else if((byVal >= 10) && (byVal <= 15))
	{
		return 'A' + byVal - 10;
	}
	else
	{
		return '0';
	}
}

static void ParseCardID(CR232_SAMPLER_DATA *pCR_Rough, BYTE* pbySrc)
{
	memset(pCR_Rough->szRawCardId, 0, 20);
	int n;
	// for(n = 0; n < 8; n++)
	// {
	// 	pCR_Rough->szRawCardId[n * 2] = ConvBCD(pbySrc[n] >> 4);
	// 	pCR_Rough->szRawCardId[n * 2 + 1] = ConvBCD(pbySrc[n + 1] & 0x0F);
	// }
	//Jimmy note 20190625 found the old parsing was WRONG!
	for(n = 0; n < 4; n++)
	{
		pCR_Rough->szRawCardId[0 + 4*n] = ConvBCD(pbySrc[0 + 2*n] >> 4);
		pCR_Rough->szRawCardId[1 + 4*n] = ConvBCD(pbySrc[0 + 2*n] & 0x0F);
		pCR_Rough->szRawCardId[2 + 4*n] = ConvBCD(pbySrc[1 + 2*n] >> 4);
		pCR_Rough->szRawCardId[3 + 4*n] = ConvBCD(pbySrc[1 + 2*n] & 0x0F);
	}
}

//static void Beep(int nMSec)
//{
//#define DIDO_DEV		"/dev/dido"
//#define DIDO_MAJOR		240
//#define IOCTL_BEEP_100	_IOW(DIDO_MAJOR, 18, unsigned long)
//	int fd = open(DIDO_DEV, O_RDWR);
//	if(fd > 0)
//	{
//		ioctl(fd, IOCTL_BEEP_100, nMSec / 100);
//		close(fd);
//	}
//}

static BOOL SearchCard(CR232_SAMPLER_DATA *pCR_Rough)
{
	TRACEX("Now begin searching RFID card...\n");

	BYTE bySec0ChkData[4];
	BYTE bySec5ChkRomdNum[4];
	BYTE bySec5ChkRet[4];
	//Step1闂傚倷娴囧▔鏇㈠窗閹版澘鍑犲┑鐘宠壘缁狀垶鏌ㄩ悤鍌涘 search Card
	CARDREADER_FRAME_INFO* pFrame = &pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_SERACH_CARD];
	if(ERR_SAMPLE_OK != Sample_MTFrame(pCR_Rough, pFrame))
	{
		return FALSE;//just return OK;
	}

	//Step2: validate card info
	int nSampResult = Sample_MTFrame(pCR_Rough, &pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_VAILIDATE_SEC0_KEYA]);
	if(ERR_SAMPLE_OK != nSampResult)
	{
		return FALSE;
	}
	pFrame = &pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_READ_SEC0_BLK0];
	nSampResult = Sample_MTFrame(pCR_Rough, pFrame); //Now Read sec0blk0, get check sn
	if(ERR_SAMPLE_OK != nSampResult)
	{
		return FALSE;
	}
	//婵犵數鍋涢ˇ顓㈠礉閹达箑鏋侀柟鎹愵嚙濡﹢鏌曢崼婵囶棞妞ゅ繐鐖煎铏规崉閵娿儲鐎鹃梺鍝勵儏椤兘鐛箛鏃戞Ч閹艰揪绲芥晶楣冩⒒娴ｈ姤纭堕柛鐘冲姍瀵憡绻濆顒傤唵闂佺粯鍨兼慨銈夊疾閹间焦鍋℃繛鍡樺姇闁叉粓鏌熼崙銈嗗/**/
	memcpy(bySec0ChkData, pFrame->byParsedSecBlkData, 4);

	nSampResult = Sample_MTFrame(pCR_Rough, &pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_VAILIDATE_SEC5_KEYA]);
	if(ERR_SAMPLE_OK != nSampResult)
	{
		return FALSE;
	}
	pFrame = &pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_READ_SEC5_BLK0];
	nSampResult = Sample_MTFrame(pCR_Rough, pFrame); //Now Read sec5blk0, and cmp the ret with sec0 data
	if(ERR_SAMPLE_OK != nSampResult)
	{
		return FALSE;
	}
	memcpy(bySec5ChkRomdNum, pFrame->byParsedSecBlkData + 4, 4);
	memcpy(bySec5ChkRet, pFrame->byParsedSecBlkData, 4);
	//闂傚倷娴囧▔鏇㈠窗閹版澘鍑犲┑鐘宠壘缁狀垶鏌ｉ幋锝呅撻柡鍛倐閺岋繝宕掑Ο琛″亾閺嶎偀鍋撳顒傜Ш鐎殿喕绮欓弫鎾绘偐閾忣偅顏熼梻浣告惈閸婂爼宕愰弽顐熷亾濮橆剛绉洪柡灞诲姂閹垽宕ㄦ繝鍕磿闂備礁缍婇ˉ鎾诲礂濮椻偓瀵偊骞橀懜闈涙瀭闂佸湱鍎ら崹闈涒枔閳哄懏鐓欑€瑰嫭婢樺Σ濠氭倶韫囷絽娅嶉柡灞诲姂閹垽宕ㄦ繝鍕磿闂備胶鎳撻崵鏍箯閿燂拷,闂傚倷娴囧▔鏇㈠窗閹版澘鍑犲┑鐘宠壘缁狀垳鈧懓瀚姗€鎮ラ柆宥嗏拺閻犳亽鍔岄弸鎴︽煕閵婏妇绠為柡浣哥Т閻ｆ繈宕熼浣诡仧闂備礁鎼崐鍫曞磹閺嶎偀鍋撳顒傜Ш闁哄被鍔戦幃銏ゅ川婵犲嫪绱曢梻浣哥秺椤ユ捇宕楀鈧顐﹀箻鐠囧弶顥濋梺闈涚墕濡顢旈崼鏇熲拺閻犳亽鍔岄弸鎴︽煛鐎ｎ亶鐓兼鐐茬箻閹粓鎳為妷锔筋仧闂備浇娉曢崳锕傚箯閿燂拷/**/
	if(!IsChkValid(bySec0ChkData, bySec5ChkRomdNum, bySec5ChkRet))
	{
		TRACEX("CheckSum not OK!!!!\n");
		return FALSE;
	}

	//Step3: parse user card, read left money
	nSampResult = Sample_MTFrame(pCR_Rough, &pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_VAILIDATE_SEC3_KEYA]);
	if(ERR_SAMPLE_OK != nSampResult)
	{
		return FALSE;
	}

	ParseCardID(pCR_Rough, pFrame->byParsedSecBlkData + 8);
	TRACEX("The parsed CardID:[%s]\n", pCR_Rough->szRawCardId);
	
	pFrame = &pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_READ_SEC3_BLK0];
	nSampResult = Sample_MTFrame(pCR_Rough, pFrame);
	if(ERR_SAMPLE_OK != nSampResult)
	{
		return FALSE;
	}

	pCR_Rough->fRawLeftMoney = (float)Swap4Bytes(pFrame->byParsedSecBlkData) * 0.01;
	TRACEX("Bytes Money:[%02X %02X %02X %02X]The Left Money is [%0.2f]\n",
		pFrame->byParsedSecBlkData[0], pFrame->byParsedSecBlkData[1],
		pFrame->byParsedSecBlkData[2], pFrame->byParsedSecBlkData[3],
		pCR_Rough->fRawLeftMoney);
	
	pCR_Rough->tm_LastSuccessCard = _NOW_;

	memset(pCR_Rough->szRawParentId, 0, 20);

	//Beep(300); //do not beep here, just use it in Qt

	return TRUE;
}


static void RefreshSystemStatus(CR232_SAMPLER_DATA *pCR_Rough)
{
	VAR_VALUE varV;
	pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_SP_VAR, 1, 4, &varV, 4);
	if((varV.emValue == 0) || (varV.emValue == 3))
	{
		pCR_Rough->bIsChargerAvailable = TRUE;
	}
	else
	{
		pCR_Rough->bIsChargerAvailable = FALSE;
	}

	pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ALMSTATE, 1, 4, &varV, 4);
	pCR_Rough->bServerCommOK = (varV.emValue == 0);

	pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ALMSTATE, 1, 23, &varV, 4);
	pCR_Rough->bCommWithMasterOK = (varV.emValue == 0);
	
}

static int DoLocalAuthorization(CR232_SAMPLER_DATA *pCR_Rough )
{
	time_t tmNow = _NOW_;
	int n;
	AUTHORIZATION_DATA* pAuth;

	//Jimmy note 20181306: first check cache, that will improve performance
	//Authorize Cache check
	if(pCR_Rough->stESiOCPPConfigs.bLocalAuthentEnable)
	{
		for(n = 0; n < pCR_Rough->stAuthCache.nAuthorizationDataNumber; n++)
		{
			pAuth = &pCR_Rough->stAuthCache.authorizationDataList[n];
			if(strcmp(pAuth->szIdTag, pCR_Rough->szRawCardId) == 0)
			{
				pCR_Rough->emAuthorizeStatus = AUTH_STATUS_ACCEPTED;
				memcpy(pCR_Rough->szRawParentId, pAuth->szParentIdTag, 20);
				if(pAuth->emAuthorizationStatus != AT_Accpted)
				{
					TRACEX("Rejected reason 1222222\n");
					pCR_Rough->emAuthorizeStatus = AUTH_STATUS_REJECTED;
				}
				else if(pAuth->dtExpiryDate > 0) //optional
				{
					if(tmNow > pAuth->dtExpiryDate)
					{
						TRACEX("Rejected reason 33333333\n");
						pCR_Rough->emAuthorizeStatus = AUTH_STATUS_REJECTED;
					}
				}
				TRACEX("Accepted by AuthCache!!!\n");
				return AUTH_DO_AUTHCACHE;
			}
		}
	}

	//Local List check
	if(pCR_Rough->stESiOCPPConfigs.bLocalAuthentEnable)
	{
		for(n = 0; n < pCR_Rough->stLocalList.nAuthorizationDataNumber; n++)
		{
			pAuth = &pCR_Rough->stLocalList.authorizationDataList[n];
			if(strcmp(pAuth->szIdTag, pCR_Rough->szRawCardId) == 0)
			{
				pCR_Rough->emAuthorizeStatus = AUTH_STATUS_ACCEPTED;
				memcpy(pCR_Rough->szRawParentId, pAuth->szParentIdTag, 20);
				if(pAuth->emAuthorizationStatus != AT_Accpted)
				{
					TRACEX("Rejected reason 334444444\n");
					pCR_Rough->emAuthorizeStatus = AUTH_STATUS_REJECTED;
				}
				else if(pAuth->dtExpiryDate > 0) //optional
				{
					if(tmNow > pAuth->dtExpiryDate)
					{
						TRACEX("Rejected reason 455555555\n");
						pCR_Rough->emAuthorizeStatus = AUTH_STATUS_REJECTED;
					}
				}
				return AUTH_DO_LOCALLIST;
			}
		}
	}

	return -1;
}

static UINT IsCardSame_4StopTransaction(CR232_SAMPLER_DATA *pCR_Rough)
{
	VAR_VALUE varV;
	int n;
	UINT nFlagConn = 0;
	A_BLOB_RUN_CHARGE_INFO blobConnRunInfo;
	ZERO_POBJS(&blobConnRunInfo, 1);

	for(n = 0; n < MAX_SUPPORT_GUN_NUM; n++)
	{
		pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_SP_VAR, 201+n, 
			101, &varV, 4);
		if((varV.emValue >= GUN_STATE_WAITING_START) && 
			(varV.emValue < GUN_STATE_WAITING_END))
		{
			varV.blobValue = (BYTE *)&blobConnRunInfo;
			pCR_Rough->pDPR->Get((HANDLE)pCR_Rough->hSampler, GETTYPE_SP_VAR,
				201+n, 107, &varV, sizeof(A_BLOB_RUN_CHARGE_INFO));
			if(strcmp(blobConnRunInfo.szUserID, pCR_Rough->szRawCardId) == 0)
			{
				nFlagConn |= (1 << n);
			}
		}
	}

	return nFlagConn;
}

int ProCard_MultiMain(SAMPLER *hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	CR232_SAMPLER_DATA *pCR_Rough = (CR232_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	time_t tmNow = _NOW_;
	
	if(Mutex_Lock(pCR_Rough->hSamplerLock, MAX_CR_WAIT_SAMPLE) != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}

	if(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_NONE)
	{
		TRACE("ms:Here Waiting Card Comming !!!\n");
		if(!pCR_Rough->bHasSearchCard)
		{
			Mutex_Unlock(pCR_Rough->hSamplerLock);
			return ERR_SAMPLE_OK;
		}
		pCR_Rough->bHasSearchCard = FALSE;
		printf("Master CR:bHasSearchCard =%d bServerCommOK =%d\n", \
			pCR_Rough->bHasSearchCard, pCR_Rough->bServerCommOK);

		if((!pCR_Rough->bServerCommOK) && (!pCR_Rough->stESiOCPPConfigs.bLocalAuthentEnable))
		{
			printf("pCR_Rough->bServerCommOK =%d\n",pCR_Rough->bServerCommOK);
			Mutex_Unlock(pCR_Rough->hSamplerLock);
			return ERR_SAMPLE_OK;
		}
		else// if(SearchCard(pCR_Rough))
		{
			int nLocalAuthSrc = -1;
			//if(pCR_Rough->stOCPPConfigs.bAllowOfflineTxForUnknowId_RW && (!pCR_Rough->bServerCommOK))
			//{
			//	pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_ACCEPTED;
			//	nLocalAuthSrc = AUTH_DO_TX_UNKONW;
			//}
			//else
			{
				pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_ONGOING;
				if(((pCR_Rough->stESiOCPPConfigs.bLocalAuthentEnable) && (pCR_Rough->bServerCommOK))/* ||
					((pCR_Rough->stOCPPConfigs.bLocalAuthorizeOffline_RW) && (!pCR_Rough->bServerCommOK))*/)
				{
					nLocalAuthSrc = DoLocalAuthorization(pCR_Rough);
					if((pCR_Rough->emAuthorizeStatus == AUTH_STATUS_REJECTED) && (pCR_Rough->bServerCommOK))
					{
						pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_ONGOING;
						nLocalAuthSrc = -1; //need send out Authorize.req when off line auth result is rejected
					}
				}
				if(nLocalAuthSrc == -1)
				{
					if(!pCR_Rough->bServerCommOK)
					{
						pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_REJECTED;
					}
					else //send server for authorizing
					{
						EVENT_AUTHORIZE evAuth;
						memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
						memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
						evAuth.emChargeStartMethod = pCR_Rough->emStartMethod;
						evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
						evAuth.nWhereFrom = AUTH_SRC_LOCAL;

						EVENT_ARG eventArg;
						ZERO_POBJS(&eventArg, 1);
						eventArg.emEvType = EVENT_TYPE_USER_AUTHORIZE;
						eventArg.emMsgType = EVENT_MSG_BEGIN;
						eventArg.nEvDataSize = sizeof(EVENT_AUTHORIZE);
						eventArg.pEvData = (void*)&evAuth;
						pDPR->Set(hSamplerThis, SETTYPE_SENDMSG,
							&eventArg, sizeof(EVENT_ARG));
					}
				}
			}
			//pCR_Rough->emAuthorizeStatus = AUTH_STATUS_ACCEPTED;
			if((pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ||
				(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_REJECTED))
			{
				EVENT_AUTHORIZE evAuth;
				memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
				memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
				memcpy(evAuth.szIdParent, pCR_Rough->szRawParentId, 20);
				evAuth.emChargeStartMethod = pCR_Rough->emStartMethod;
				evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
				evAuth.nWhereFrom = pCR_Rough->nWhichClientAuth;
				evAuth.nWhereDoAuthorizing = nLocalAuthSrc;
				evAuth.emResult = (pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ? ACCEPTED : BLOCKED;
				//printf("evAuth.emChargeStartMethod=%d evAuth.emResult=%d\n", evAuth.emChargeStartMethod, evAuth.emResult);
				EVENT_ARG eventArg;
				ZERO_POBJS(&eventArg, 1);
				eventArg.emEvType = EVENT_TYPE_USER_AUTHORIZE;
				eventArg.emMsgType = EVENT_MSG_END;
				eventArg.nEvDataSize = sizeof(EVENT_AUTHORIZE);
				eventArg.pEvData = (void*)&evAuth;
				pDPR->Set(hSamplerThis, SETTYPE_SENDMSG,
					&eventArg, sizeof(EVENT_ARG));
			}

			printf("Now we found a new RFID card, do the authorization now, local: %d emAuthorizeStatus=%d...\n",\
				pCR_Rough->emAuthorizeStatus, pCR_Rough->emAuthorizeStatus);
		}
	}
	else if(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ONGOING)
	{
		printf("Now waiting server authorization, wait[%d --> %d]...\n", tmNow - pCR_Rough->tm_LastSuccessCard, pCR_Rough->nAuthorizationTimeouT_Sec);
		if(ABS(tmNow - pCR_Rough->tm_LastSuccessCard) >= pCR_Rough->nAuthorizationTimeouT_Sec) //in case user forget to do the charing
		{
			pCR_Rough->emAuthorizeStatus = AUTH_STATUS_NONE; //return to normal
			EVENT_UI_NOTICE evNote;
			evNote.emNotoceType = UI_NOTICETYPE_AUTHRIZE_TIMEOUT;
			SendMsg(hSamplerThis, EVENT_TYPE_UI_NOTICE, EVENT_MSG_BEGIN,
				&evNote, sizeof(EVENT_UI_NOTICE));
		}
	}
	else //accepted/rejected, triggle msg for other modules
	{
		int nDiff = ABS(tmNow - pCR_Rough->tm_LastSuccessCard);
		printf("Now CR was %s, wait[%d --> %d]...\n", (pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ? "accepted" : "rejected",
			nDiff, pCR_Rough->nAuthorizationTimeouT_Sec);
		//this will be done in the event handler
		if(/*(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) && */
			(nDiff >= pCR_Rough->nAuthorizationTimeouT_Sec)) //the information will be kept 5 sec
		{
			pCR_Rough->emAuthorizeStatus = AUTH_STATUS_NONE; //return to normal
		}

		//now push the data to reflect on Display
		Push_TextValue(SETTYPE_RAW_SP, hSamplerThis, 301, 301, (void*)&pCR_Rough->szRawCardId, 20);
		Push_TextValue(SETTYPE_RAW_SP, hSamplerThis, 301, 302, (void*)&pCR_Rough->szRawParentId, 20);
	}

	VAR_VALUE varV;
	varV.emValue = pCR_Rough->emAuthorizeStatus;
	Push_VarValue(SETTYPE_RAW_SP, hSamplerThis, 301, 303, varV);

	Mutex_Unlock(pCR_Rough->hSamplerLock);

	return ERR_SAMPLE_OK;
}

int ProCard_Slave(SAMPLER *hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	CR232_SAMPLER_DATA *pCR_Rough = (CR232_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	time_t tmNow = _NOW_;

	if(Mutex_Lock(pCR_Rough->hSamplerLock, MAX_CR_WAIT_SAMPLE) != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}

	if(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_NONE)
	{
		if(!SearchCard(pCR_Rough)) //will always work
		{
			Mutex_Unlock(pCR_Rough->hSamplerLock);
			return ERR_SAMPLE_OK;
		}

		//Judge if stopTrans, if the same id, no need to do authorize
		UINT unFlagConn = IsCardSame_4StopTransaction(pCR_Rough);
		if(unFlagConn > 0)
		{
			pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_ACCEPTED;
			EVENT_AUTHORIZE evAuth;
			memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
			memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
			evAuth.emChargeStartMethod = pCR_Rough->emStartMethod;
			evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
			evAuth.nWhereFrom = AUTH_SRC_LOCAL;
			evAuth.nWhereDoAuthorizing = AUTH_DO_STOPTRANS_SAMEID;
			evAuth.uFlagUsed4StopConn = unFlagConn;
			evAuth.emResult = ACCEPTED;

			EVENT_ARG eventArg;
			ZERO_POBJS(&eventArg, 1);
			eventArg.emEvType = EVENT_TYPE_USER_AUTHORIZE;
			eventArg.emMsgType = EVENT_MSG_END;
			eventArg.nEvDataSize = sizeof(EVENT_AUTHORIZE);
			eventArg.pEvData = (void*)&evAuth;
			pDPR->Set(hSamplerThis, SETTYPE_SENDMSG,
				&eventArg, sizeof(EVENT_ARG));
		}
		else if(!pCR_Rough->bCommWithMasterOK)
		{
			printf("Comm With Master Failed, Do Nothing\n");
			Mutex_Unlock(pCR_Rough->hSamplerLock);
			return ERR_SAMPLE_OK;
		}
		else 
		{
			pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_ONGOING;
	
			EVENT_AUTHORIZE evAuth;
			memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
			memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
			evAuth.emChargeStartMethod = pCR_Rough->emStartMethod;
			evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
			evAuth.nWhereFrom = AUTH_SRC_LOCAL;

			EVENT_ARG eventArg;
			ZERO_POBJS(&eventArg, 1);
			eventArg.emEvType = EVENT_TYPE_USER_AUTHORIZE;
			eventArg.emMsgType = EVENT_MSG_BEGIN;
			eventArg.nEvDataSize = sizeof(EVENT_AUTHORIZE);
			eventArg.pEvData = (void*)&evAuth;
			pDPR->Set(hSamplerThis, SETTYPE_SENDMSG,
				&eventArg, sizeof(EVENT_ARG));
		
		
			printf("Now we Send new RFID card To Master ->szRawCardId=%s\n", pCR_Rough->szRawCardId);
		}
	}
	else if(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ONGOING)
	{
		printf("Now waiting server authorization, wait[%d --> %d]...\n", tmNow - pCR_Rough->tm_LastSuccessCard, pCR_Rough->nAuthorizationTimeouT_Sec);
		if(ABS(tmNow - pCR_Rough->tm_LastSuccessCard) >= pCR_Rough->nAuthorizationTimeouT_Sec) //in case user forget to do the charing
		{
			pCR_Rough->emAuthorizeStatus = AUTH_STATUS_NONE; //return to normal
			EVENT_UI_NOTICE evNote;
			evNote.emNotoceType = UI_NOTICETYPE_AUTHRIZE_TIMEOUT;
			SendMsg(hSamplerThis, EVENT_TYPE_UI_NOTICE, EVENT_MSG_BEGIN,
				&evNote, sizeof(EVENT_UI_NOTICE));
		}
	}
	else //accepted/rejected, triggle msg for other modules
	{
		int nDiff = ABS(tmNow - pCR_Rough->tm_LastSuccessCard);
		printf("Now CR was %s, wait[%d --> %d]...\n", (pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ? "accepted" : "rejected",
			nDiff, pCR_Rough->nAuthorizationTimeouT_Sec);
		//this will be done in the event handler
		if(/*(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) && */
			(nDiff >= pCR_Rough->nAuthorizationTimeouT_Sec)) //the information will be kept 5 sec
		{
			pCR_Rough->emAuthorizeStatus = AUTH_STATUS_NONE; //return to normal
		}

		//now push the data to reflect on Display
		Push_TextValue(SETTYPE_RAW_SP, hSamplerThis, 301, 301, (void*)&pCR_Rough->szRawCardId, 20);
		Push_TextValue(SETTYPE_RAW_SP, hSamplerThis, 301, 302, (void*)&pCR_Rough->szRawParentId, 20);
	}

	VAR_VALUE varV;
	varV.emValue = pCR_Rough->emAuthorizeStatus;
	Push_VarValue(SETTYPE_RAW_SP, hSamplerThis, 301, 303, varV);

	Mutex_Unlock(pCR_Rough->hSamplerLock);
}


int ProCard_StandAlone(SAMPLER *hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	CR232_SAMPLER_DATA *pCR_Rough = (CR232_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	time_t tmNow = _NOW_;

	if(Mutex_Lock(pCR_Rough->hSamplerLock, MAX_CR_WAIT_SAMPLE) != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}

	if(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_NONE)
	{
		if(!SearchCard(pCR_Rough)) //will always work
		{
			Mutex_Unlock(pCR_Rough->hSamplerLock);
			return ERR_SAMPLE_OK;
		}

		//Judge if stopTrans, if the same id, no need to do authorize
		UINT unFlagConn = IsCardSame_4StopTransaction(pCR_Rough);
		if(unFlagConn > 0)
		{
			pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_ACCEPTED;
			EVENT_AUTHORIZE evAuth;
			memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
			memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
			evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
			evAuth.nWhereFrom = AUTH_SRC_LOCAL;
			evAuth.nWhereDoAuthorizing = AUTH_DO_STOPTRANS_SAMEID;
			evAuth.uFlagUsed4StopConn = unFlagConn;
			evAuth.emResult = ACCEPTED;

			EVENT_ARG eventArg;
			ZERO_POBJS(&eventArg, 1);
			eventArg.emEvType = EVENT_TYPE_USER_AUTHORIZE;
			eventArg.emMsgType = EVENT_MSG_END;
			eventArg.nEvDataSize = sizeof(EVENT_AUTHORIZE);
			eventArg.pEvData = (void*)&evAuth;
			pDPR->Set(hSamplerThis, SETTYPE_SENDMSG,
				&eventArg, sizeof(EVENT_ARG));
		}
		else if((!pCR_Rough->bServerCommOK) && (!pCR_Rough->stESiOCPPConfigs.bLocalAuthentEnable))
		{
			Mutex_Unlock(pCR_Rough->hSamplerLock);
			return ERR_SAMPLE_OK;
		}
		else// if(SearchCard(pCR_Rough))
		{
			int nLocalAuthSrc = -1;
			// if(pCR_Rough->stOCPPConfigs.bAllowOfflineTxForUnknowId_RW && (!pCR_Rough->bServerCommOK))
			// {
			// 	pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_ACCEPTED;
			// 	nLocalAuthSrc = AUTH_DO_TX_UNKONW;
			// }
			// else
			{
				pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_ONGOING;
				if(((pCR_Rough->stESiOCPPConfigs.bLocalAuthentEnable) && (pCR_Rough->bServerCommOK))/* ||
					((pCR_Rough->stOCPPConfigs.bLocalAuthorizeOffline_RW) && (!pCR_Rough->bServerCommOK))*/)
				{
					nLocalAuthSrc = DoLocalAuthorization(pCR_Rough);
					if((pCR_Rough->emAuthorizeStatus == AUTH_STATUS_REJECTED) && (pCR_Rough->bServerCommOK))
					{
						pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_ONGOING;
						nLocalAuthSrc = -1; //need send out Authorize.req when off line auth result is rejected
					}
				}
				if(nLocalAuthSrc == -1)
				{
					if(!pCR_Rough->bServerCommOK)
					{
						pCR_Rough->emAuthorizeStatus  = AUTH_STATUS_REJECTED;
					}
					else //send server for authorizing
					{
						EVENT_AUTHORIZE evAuth;
						memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
						memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
						evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
						evAuth.nWhereFrom = AUTH_SRC_LOCAL;

						EVENT_ARG eventArg;
						ZERO_POBJS(&eventArg, 1);
						eventArg.emEvType = EVENT_TYPE_USER_AUTHORIZE;
						eventArg.emMsgType = EVENT_MSG_BEGIN;
						eventArg.nEvDataSize = sizeof(EVENT_AUTHORIZE);
						eventArg.pEvData = (void*)&evAuth;
						pDPR->Set(hSamplerThis, SETTYPE_SENDMSG,
							&eventArg, sizeof(EVENT_ARG));
					}
				}
			}
			//pCR_Rough->emAuthorizeStatus = AUTH_STATUS_REJECTED; //For Test
			if((pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ||
				(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_REJECTED))
			{
				EVENT_AUTHORIZE evAuth;
				memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
				memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
				memcpy(evAuth.szIdParent, pCR_Rough->szRawParentId, 20);
				evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
				evAuth.nWhereFrom = AUTH_SRC_LOCAL;
				evAuth.nWhereDoAuthorizing = nLocalAuthSrc;
				evAuth.emResult = (pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ? ACCEPTED : BLOCKED;

				EVENT_ARG eventArg;
				ZERO_POBJS(&eventArg, 1);
				eventArg.emEvType = EVENT_TYPE_USER_AUTHORIZE;
				eventArg.emMsgType = EVENT_MSG_END;
				eventArg.nEvDataSize = sizeof(EVENT_AUTHORIZE);
				eventArg.pEvData = (void*)&evAuth;
				pDPR->Set(hSamplerThis, SETTYPE_SENDMSG,
					&eventArg, sizeof(EVENT_ARG));
			}

			TRACEX("Now we found a new RFID card, do the authorization now, local: %d...\n", pCR_Rough->emAuthorizeStatus);
		}
	}
	else if(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ONGOING)
	{
		TRACEX("Now waiting server authorization, wait[%d --> %d]...\n", tmNow - pCR_Rough->tm_LastSuccessCard, pCR_Rough->nAuthorizationTimeouT_Sec);
		if(ABS(tmNow - pCR_Rough->tm_LastSuccessCard) >= pCR_Rough->nAuthorizationTimeouT_Sec) //in case user forget to do the charing
		{
			pCR_Rough->emAuthorizeStatus = AUTH_STATUS_NONE; //return to normal
			EVENT_UI_NOTICE evNote;
			evNote.emNotoceType = UI_NOTICETYPE_AUTHRIZE_TIMEOUT;
			SendMsg(hSamplerThis, EVENT_TYPE_UI_NOTICE, EVENT_MSG_BEGIN,
				&evNote, sizeof(EVENT_UI_NOTICE));
		}
	}
	else //accepted/rejected, triggle msg for other modules
	{
		//EVENT_AUTHORIZE evAuth;
		//memset(evAuth, 0, sizeof(EVENT_AUTHORIZE));
		//memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
		//evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
		//evAuth.nWhereFrom = 0;
		//evAuth.emResult = (pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ? ACCEPTED : BLOCKED;

		//EVENT_ARG eventArg;
		//ZERO_POBJS(&eventArg, 1);
		//eventArg.emEvType = EVENT_TYPE_USER_AUTHORIZE;
		//eventArg.emMsgType = EVENT_MSG_END;
		//eventArg.nEvDataSize = sizeof(EVENT_AUTHORIZE);
		//eventArg.pEvData = (void*)&evAuth;
		////none block
		//eventArg.bBlockMode = FALSE;
		//pDPR->Set(hSampler, SETTYPE_SENDMSG,
		//	&eventArg, sizeof(EVENT_ARG));
		//pCR_Rough->emAuthorizeStatus = AUTH_STATUS_ONGOING;
		int nDiff = ABS(tmNow - pCR_Rough->tm_LastSuccessCard);
		TRACEX("Now CR was %s, wait[%d --> %d]...\n", (pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ? "accepted" : "rejected",
			nDiff, pCR_Rough->nAuthorizationTimeouT_Sec);
		//this will be done in the event handler
		if(/*(pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) && */
			(nDiff >= pCR_Rough->nAuthorizationTimeouT_Sec)) //the information will be kept 5 sec
		{
			pCR_Rough->emAuthorizeStatus = AUTH_STATUS_NONE; //return to normal
		}

		//now push the data to reflect on Display
		Push_TextValue(SETTYPE_RAW_SP, hSamplerThis, 301, 301, (void*)&pCR_Rough->szRawCardId, 20);
		Push_TextValue(SETTYPE_RAW_SP, hSamplerThis, 301, 302, (void*)&pCR_Rough->szRawParentId, 20);
	}

	VAR_VALUE varV;
	varV.emValue = pCR_Rough->emAuthorizeStatus;
	Push_VarValue(SETTYPE_RAW_SP, hSamplerThis, 301, 303, varV);

	Mutex_Unlock(pCR_Rough->hSamplerLock);

	return ERR_SAMPLE_OK;
}

DLLExport int Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	CR232_SAMPLER_DATA *pCR_Rough = (CR232_SAMPLER_DATA*)pSampler->pDataHeap;
	int nRet;

	RefreshSystemStatus(pCR_Rough);	

	if(!pCR_Rough->bIsCardReaderInstalled)
	{
		return ERR_SAMPLE_OK;
	}

	if(pCR_Rough->bIsLocalAuthorizeDisabled)
	{
		return ERR_SAMPLE_OK;
	}
	//else if(!pCR_Rough->bIsChargerAvailable)
	//{
	//	pCR_Rough->emAuthorizeStatus = AUTH_STATUS_NONE;
	//	Mutex_Unlock(pCR_Rough->hSamplerLock);
	//	return ERR_SAMPLE_OK;
	//}

	if(pCR_Rough->emESSystemType == ESSYSTYPE_MULTIMAIN) // Master Cabinet
	{
		nRet = ProCard_MultiMain(pSampler);
	}
	else if(pCR_Rough->emESSystemType == ESSYSTYPE_SLAVE) //Slave Cabinet
	{
		nRet = ProCard_Slave(pSampler);
 	}
	else //StandAlone
	{
		nRet = ProCard_StandAlone(pSampler);
	}

	return nRet;
}

static void UpdateAuthCache(CR232_SAMPLER_DATA *pCR_Rough, AUTHORIZATION_DATA *pNew)
{
	int n;
	//find old record
	BOOL bFoundOld = FALSE;
	TRACEX("--------Now update local cache-------\n");
	for(n = 0; n < pCR_Rough->stAuthCache.nAuthorizationDataNumber; n++)
	{
		if(strcmp(pCR_Rough->stAuthCache.authorizationDataList[n].szIdTag, pNew->szIdTag) == 0)
		{
			if((pCR_Rough->stAuthCache.authorizationDataList[n].emAuthorizationStatus == pNew->emAuthorizationStatus) &&
				(pCR_Rough->stAuthCache.authorizationDataList[n].dtExpiryDate == pNew->dtExpiryDate))
			{
				TRACEX("--------UUOOOOOFDFEFEFE-------\n");
				return;
			}
			//should think about it later, maybe need remove expired records
			memcpy(&pCR_Rough->stAuthCache.authorizationDataList[n], pNew,
				sizeof(AUTHORIZATION_DATA));
			TRACEX("--------Now update local cache sssuususssss-------\n");
			bFoundOld = TRUE;
			break;
		}
	}

	if(!bFoundOld)
	{
		TRACEX("--------Now add new authCache-------\n");
		//now it is new record, should insert into the tail
		int nCurrPos = pCR_Rough->stAuthCache.nAuthorizationDataNumber;
		if(pCR_Rough->stAuthCache.nAuthorizationDataNumber < MAX_AUTHORIZE_CACHE_NUMBER)
		{
			memcpy(&pCR_Rough->stAuthCache.authorizationDataList[nCurrPos], pNew,
				sizeof(AUTHORIZATION_DATA));
			pCR_Rough->stAuthCache.nAuthorizationDataNumber++;
		}
		else
		{
			nCurrPos = MAX_AUTHORIZE_CACHE_NUMBER - 1;
			for(n = 1; n < nCurrPos; n++)
			{
				memcpy(&pCR_Rough->stAuthCache.authorizationDataList[n - 1], &pCR_Rough->stAuthCache.authorizationDataList[n],
					sizeof(AUTHORIZATION_DATA));
			}
			memcpy(&pCR_Rough->stAuthCache.authorizationDataList[nCurrPos], pNew,
				sizeof(AUTHORIZATION_DATA));
		}
	}

	Push_BlobValue(SETTYPE_RAW_ST, pCR_Rough->hSampler, 301, 7, &pCR_Rough->stAuthCache, 
		sizeof(A_BLOB_AUTHORIZECACHE), BLOB_TYPE_AUTHORIZE_CACHE);
}

static void UpdateLocalList(CR232_SAMPLER_DATA *pCR_Rough, A_BLOB_UPDATE_LOCALLIST *pUpdate)
{
	int n;
	pCR_Rough->stLocalList.nListVersion = pUpdate->nListVersion;
	if(pUpdate->emUpdateType == LL_UPDATE_Full)
	{
		for(n = 0; n < MIN(MAX_LOCALLIST_UPDATE_ITEMS, pUpdate->nAuthorizationDataNumber); n++)
		{
			memcpy(&pCR_Rough->stLocalList.authorizationDataList[n], &pUpdate->authorizationDataList[n], sizeof(AUTHORIZATION_DATA));
		}
		pCR_Rough->stLocalList.nAuthorizationDataNumber = pUpdate->nAuthorizationDataNumber;
	}
	else
	{
		int m;
		BOOL bFoundExisted = FALSE;
		int nOldLocalListTotal = pCR_Rough->stLocalList.nAuthorizationDataNumber;
		TRACEX("Now update LocalList Differential, localTotal[%d], newTotal[%d]\n", nOldLocalListTotal, pUpdate->nAuthorizationDataNumber);
		for(n = 0; n < pUpdate->nAuthorizationDataNumber; n++)
		{
			bFoundExisted = FALSE;
			for(m = 0; m < nOldLocalListTotal; m++)
			{
				if(strcmp(pCR_Rough->stLocalList.authorizationDataList[m].szIdTag, pUpdate->authorizationDataList[n].szIdTag) == 0)
				{
					//need handle expired list later??
					memcpy(&pCR_Rough->stLocalList.authorizationDataList[m],
						&pUpdate->authorizationDataList[n], sizeof(AUTHORIZATION_DATA));
					bFoundExisted = TRUE;
					break;
				}
			}
			if(!bFoundExisted)
			{				
				if(pCR_Rough->stLocalList.nAuthorizationDataNumber >= MAX_LOCALLIST_ITEMS)
				{
					break;
				}
				memcpy(&pCR_Rough->stLocalList.authorizationDataList[pCR_Rough->stLocalList.nAuthorizationDataNumber],
					&pUpdate->authorizationDataList[n], sizeof(AUTHORIZATION_DATA));
				pCR_Rough->stLocalList.nAuthorizationDataNumber++;
			}
		}
	}

	Push_BlobValue(SETTYPE_RAW_ST, pCR_Rough->hSampler, 301, 6, &pCR_Rough->stLocalList, 
		sizeof(A_BLOB_LOCALLIST), BLOB_TYPE_LOACL_LIST);
}

DLLExport int Control(HANDLE hSamplerThis, PARAM_TYPE	emParaType,
	int			nDevId,
	int			nParamId,
	VAR_VALUE	*pVarVal,
	int			nValSize,
	int			nTimeOut)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	CR232_SAMPLER_DATA *pCR_Rough = (CR232_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	time_t tmNow;

	int nRet = ERR_CONTROL_OK;

	if(Mutex_Lock(pCR_Rough->hSamplerLock, nTimeOut) != ERR_MUTEX_OK)
	{
		return ERR_CONTROL_TIMEOUT;
	}

	if(emParaType == PARAM_TYPE_CONTROL)
	{
		if(nParamId == 301) //clear auth cache
		{
			memset(&pCR_Rough->stAuthCache, 0, sizeof(A_BLOB_AUTHORIZECACHE));
			nRet = Push_BlobValue(SETTYPE_RAW_ST, hSamplerThis, 301, 7, &pCR_Rough->stAuthCache, sizeof(A_BLOB_AUTHORIZECACHE), BLOB_TYPE_AUTHORIZE_CACHE);
			if(nRet != ERR_DPR_OK)
			{
				nRet = ERR_CONTROL_FAILED;
			}
		}
		else if(nParamId == 302) //reset cardreader status
		{
			pCR_Rough->emAuthorizeStatus = AUTH_STATUS_NONE;
		}
		else if(nParamId == 303)
		{
			if(nValSize != sizeof(A_BLOB_UPDATE_AUTHSTAT_ANDCACHE))
			{
				nRet = ERR_CONTROL_FAILED;
			}
			A_BLOB_UPDATE_AUTHSTAT_ANDCACHE *pAuthUpdate = (A_BLOB_UPDATE_AUTHSTAT_ANDCACHE*)pVarVal->blobValue;
			if(pAuthUpdate->emUpdateType == AUTH_UPDATE_AUTHORIZATION_REQ)
			{
				TRACEX("Received server resp of Auth for IdTag[%s, %s], emStatus = %d, Accepted = %d\n",
					pCR_Rough->szRawCardId, pAuthUpdate->newAuthData.szIdTag, pCR_Rough->emAuthorizeStatus , 
					pAuthUpdate->newAuthData.emAuthorizationStatus);

				if((pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ONGOING) && 
					(strcmp(pCR_Rough->szRawCardId, pAuthUpdate->newAuthData.szIdTag) == 0))
				{
					memcpy(pCR_Rough->szRawParentId, pAuthUpdate->newAuthData.szParentIdTag, 20);
					pCR_Rough->emAuthorizeStatus = (pAuthUpdate->newAuthData.emAuthorizationStatus == AT_Accpted) ?
							AUTH_STATUS_ACCEPTED : AUTH_STATUS_REJECTED;

					//notice for authorization result
					EVENT_AUTHORIZE evAuth;
					memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
					memcpy(evAuth.szIdTocken, pCR_Rough->szRawCardId, 20);
					memcpy(evAuth.szIdParent, pCR_Rough->szRawParentId, 20);
					evAuth.emChargeStartMethod = pCR_Rough->emStartMethod;
					evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
					evAuth.nWhereFrom = AUTH_SRC_LOCAL;
					evAuth.nWhereDoAuthorizing = AUTH_DO_OCPP;
					evAuth.emResult = (pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ? ACCEPTED : BLOCKED;

					SendMsg(hSamplerThis, EVENT_TYPE_USER_AUTHORIZE, EVENT_MSG_END,
						&evAuth, sizeof(EVENT_AUTHORIZE));

					if(pCR_Rough->stESiOCPPConfigs.bLocalAuthentEnable)
					{
						UpdateAuthCache(pCR_Rough, &pAuthUpdate->newAuthData);
					}
				}
				else
				{
					nRet = ERR_CONTROL_FAILED;
				}
			}
			else
			{
				//Jimmy added 20190413: just to notice the idTag info change so charing thread get updated info for judging parent iD.
				if(pAuthUpdate->emUpdateType == AUTH_UPDATE_STARTTHANS_CONF)
				{
					//notice for authorization result
					EVENT_AUTHORIZE evAuth;
					memset(&evAuth, 0, sizeof(EVENT_AUTHORIZE));
					memcpy(evAuth.szIdTocken, pAuthUpdate->newAuthData.szIdTag, 20);
					memcpy(evAuth.szIdParent, pAuthUpdate->newAuthData.szParentIdTag, 20);
					//evAuth.fCardLeftMoney = pCR_Rough->fRawLeftMoney;
					evAuth.nWhereFrom = AUTH_SRC_LOCAL;
					evAuth.nWhereDoAuthorizing = AUTH_DO_OCPP;
					evAuth.emResult = (pCR_Rough->emAuthorizeStatus == AUTH_STATUS_ACCEPTED) ? ACCEPTED : BLOCKED;
					evAuth.uFlagUsed4StopConn = pAuthUpdate->emWhichConn_2Update; //for which con
					SendMsg(hSamplerThis, EVENT_TYPE_USER_AUTHORIZE, EVENT_MSG_STATUS_CHANGED,
						&evAuth, sizeof(EVENT_AUTHORIZE));
					TRACEX("Now send out the authUpdateINfo[%s,%s,%d]\n", evAuth.szIdTocken, evAuth.szIdParent, evAuth.uFlagUsed4StopConn);
				}

				//now update local cache
				if(pCR_Rough->stESiOCPPConfigs.bLocalAuthentEnable)
				{
					UpdateAuthCache(pCR_Rough, &pAuthUpdate->newAuthData);
				}
				else
				{
					nRet = ERR_CONTROL_FAILED;
				}
			}
		}
		else if(nParamId == 304)
		{
			if(nValSize != sizeof(A_BLOB_UPDATE_LOCALLIST))
			{
				nRet = ERR_CONTROL_FAILED;
			}
			A_BLOB_UPDATE_LOCALLIST* pLocalListUpdate = (A_BLOB_UPDATE_LOCALLIST*)pVarVal->blobValue;

			UpdateLocalList(pCR_Rough, pLocalListUpdate);
		}
		else if(nParamId == 305)
		{
			if(nValSize != 20)
			{
				nRet = ERR_CONTROL_FAILED;
			}
			
			memcpy(pCR_Rough->szRawCardId, pVarVal->blobValue, 20);
			nRet = (DoLocalAuthorization(pCR_Rough) == -1) ? ERR_CONTROL_FAILED : ERR_CONTROL_OK;
		}
		else if(nParamId == 306)
		{
			if(nValSize != 22)
			{
				Mutex_Unlock(pCR_Rough->hSamplerLock);
				return  ERR_CONTROL_FAILED;
			}
			tmNow = _NOW_;
			if(ABS(tmNow - pCR_Rough->tm_LastRecvCardAuth) < 5)
			{
				Mutex_Unlock(pCR_Rough->hSamplerLock);
				return  ERR_CONTROL_FAILED;
			}
			pCR_Rough->tm_LastRecvCardAuth = tmNow;

			pCR_Rough->bHasSearchCard = TRUE;
			pCR_Rough->nWhichClientAuth = (int)pVarVal->blobValue[0];
			memcpy(pCR_Rough->szRawCardId, &pVarVal->blobValue[1], 20);
			pCR_Rough->emStartMethod = pVarVal->blobValue[21];
			//printf("nWhichClientAuth =%d szRawCardId=%s tm_LastRecvCardAuth=%ld tmNow=%ld emStartMethod=%d\n",\
				pCR_Rough->nWhichClientAuth, pCR_Rough->szRawCardId,\
				 pCR_Rough->tm_LastRecvCardAuth, tmNow, pCR_Rough->emStartMethod);
		}
	}
	else //settings
	{
		//VAR_VALUE varV;
		TRACE("--------Found cmd[%d]--------\n", nParamId);
		if(nParamId == 301) //car dreader install enable
		{
			//pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ST_VAR, 301, 301, &varV, 4);
			//pCR_Rough->bIsCardReaderInstalled = (varV.emValue == 1) ? TRUE : FALSE;
			pCR_Rough->bIsCardReaderInstalled = (pVarVal->emValue == 1) ? TRUE : FALSE;
		}
		else if(nParamId == 302) //authorization timeout
		{
			//pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ST_VAR, 301, 302, &varV, 4);
			//pCR_Rough->nAuthorizationTimeouT_Sec = varV.ulValue;
			pCR_Rough->nAuthorizationTimeouT_Sec = pVarVal->ulValue;
		}
	}

	Mutex_Unlock(pCR_Rough->hSamplerLock);
	return nRet;
}

