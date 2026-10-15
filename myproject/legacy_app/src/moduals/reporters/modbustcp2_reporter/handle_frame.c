#include "modbus.h"

/*==========================================================================*
 * FUNCTION : Modbus_ParseFrame
 * PURPOSE  : 
 * CALLS    :  
 *			  
 * CALLED BY: 
 * ARGUMENTS:  
 * RETURN   :  MODBUS_FRAME_TYPE 
 * COMMENTS : 
 * CREATOR  :					2023-07-12 9:25:10
 *==========================================================================*/
MODBUS_FRAME_TYPE Modbus_ParseFrame(IN REPORTER *pReporter,
	IN char *pFrame, IN int iLen,
	OUT unsigned char *pCmdData,
	OUT unsigned char *pMBAPhead)
 {
	UNUSED(pReporter);
 	int		i =0, iLengthfromTCPIP = 0;

	if (iLen < 1)
	{
		TRACE("111 iLen =%d \n",iLen);
		return MODBUS_FRAME_ERR;
	}

	iLengthfromTCPIP =(int)(MAKEWORD(*(pFrame+5),*(pFrame+4)));
	if((iLengthfromTCPIP+MODBUS_MBAP_LEN-MODBUS_MBAP_IDENTIFIER_LEN) > iLen)
	{
		return MODBUS_FRAME_ERR;
	}
	for(i=0;i<MODBUS_MBAP_LEN ;i++)
	{
		*(pMBAPhead+i) = *(pFrame+i);
	}

	for(i=0;i<(iLengthfromTCPIP-MODBUS_MBAP_IDENTIFIER_LEN) && i<MAX_MODBUSFRAME_LEN ;i++)
	{
		*(pCmdData+i) = *(pFrame+MODBUS_MBAP_LEN+1+i);
	}

	if(i<MAX_MODBUSFRAME_LEN)
	{
		*(pCmdData+i) = '\0';
	}	 
	TRACE("Analysis Success Standard Order !!!\n");
	return MODBUS_NOR;
}


/*==========================================================================*
 * FUNCTION : Modbus_03CmdParseAndHandle
 * PURPOSE  : 
 * CALLS    :  
 *			  
 * CALLED BY: 
 * ARGUMENTS:  unsigned char *pCmdData
 * RETURN   :  MODBUS_FRAME_TYPE 
 * COMMENTS : 
 * CREATOR  :					2023-07-12 9:25:10
 *==========================================================================*/
MODBUS_FRAME_TYPE Modbus_03CmdParseAndHandle(unsigned char *pCmdData)
{
	int							iRegStart, iRegCount;

	TRACE("Modbus_03DecodeandPerfrom Start\n");

	iRegStart =(int)(MAKEWORD(*(pCmdData+1),*pCmdData));
	iRegCount = (int)(MAKEWORD(*(pCmdData+3),*(pCmdData+2)));

	TRACE("iRegStart=%d, iRegCount=%d\n", iRegStart, iRegCount);

	if((iRegStart < 0) 
		|| (iRegStart > (MAX_MODBUS_SIGNAL_NUM - 1)))
	{
		TRACE("Err iRegStart =%d \n",iRegStart);
		return  MODBUS_REGERR;
	}      

	if((iRegCount < 1 )
		||(iRegCount > 125)
		||(iRegStart + iRegCount) > MAX_MODBUS_SIGNAL_NUM)
	{
		TRACE("Err iRegCount =%d \n",iRegCount);
		return  MODBUS_REGERR;
	}

	TRACE("Modbus_03DecodeandPerfrom End\n");

	return  MODBUS_NOR;
}

/*==========================================================================*
 * FUNCTION : Modbus_FindRegInfoForConfig
 * PURPOSE  : 
 * CALLS    :  
 *			  
 * CALLED BY: 
 * ARGUMENTS:  
 * RETURN   :  MODBUS_FRAME_TYPE 
 * COMMENTS : 
 * CREATOR  :					2023-07-12 9:25:10
 *==========================================================================*/
MODBUS_FRAME_TYPE Modbus_FindRegInfoForConfig(REPORTER *pReporter, int iRegAddr,int *piEquipID,MD_DATA_ITEM **ppMdDataItem)
{
	int							i;
	MD_DATA_ITEM				*pCurMdDataItem;
	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;

	pCurMdDataItem = pModbus->pmdDataItems;

	for(i = 0; i < pModbus->nDataItemNum; i++, pCurMdDataItem++)
	{
		if((iRegAddr >= pCurMdDataItem->nRegAdrStart) && (iRegAddr <= pCurMdDataItem->nRegAdrEnd))
		{
			*ppMdDataItem = pCurMdDataItem;
			*piEquipID = pCurMdDataItem->nDevUnitId;
			TRACE(" MODBUS_NOR *piEquipID =%d emDataType=%d nFlag1 =%d nRegAdrStart=%d iRegAddr=%d nSigId =%d\n",\
				*piEquipID,pCurMdDataItem->nSigType,pCurMdDataItem->nFlag1,pCurMdDataItem->nRegAdrStart,iRegAddr,pCurMdDataItem->nSigId);
			return MODBUS_NOR;
		}
	}
	TRACE("MODBUS_REGERR *piEquipID =%d \n",*piEquipID);
	return  MODBUS_REGERR;
}
 
 
BOOL SetModbusSigValues(REPORTER *pReporter, MD_DATA_ITEM *pMdDataItem, int iData)
{
    BOOL bRet = TRUE;
	ERRCODE_DAI err;
	PARAM_VAL_GET valGet;
	VAR_VALUE varV;
	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;

	float fData;
	int TmpData = (iData - pMdDataItem->nFlag0) / pMdDataItem->nFlag1;
    memcpy(&fData, &TmpData, sizeof(float));
	// float fData = (float)(iData - pMdDataItem->nFlag0) / pMdDataItem->nFlag1;
	// printf("iData==%d, fData==%f\n", iData, fData);

    if((pMdDataItem->nSigType == DAITYPE_CTRL_PARAMVALUE_RW)||(pMdDataItem->nSigType == DAITYPE_SETT_PARAMVALUE_RW))
	{
		switch(pMdDataItem->emSigType)
		{
			case MD_SIG_Ctrl_Lock :	
			case MD_SIG_Ctrl_Reboot :	
 			{
				varV.emValue = (_ENUM)iData;
				err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
					pMdDataItem->nDevUnitId, pMdDataItem->nSigId, &varV, 4, 2000);
				//TRACE("err =%d varV.emValue=%d ->nSigType=%d ->nDevUnitId=%d ->nSigId=%d\n",\
				//	err,varV.emValue,pMdDataItem->nSigType,pMdDataItem->nDevUnitId,pMdDataItem->nSigId);
			}
			break;
			case MD_SIG_Ctrl_CtrlMode : // 1001, 3902,  1001,	DAITYPE_SETT_PARAMVALUE_RW,3902,
			{
				varV.emValue = (_ENUM)iData;
				pModbus->strCtrl.emCtrlMode = varV.emValue;
				err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,
					pMdDataItem->nDevUnitId, pMdDataItem->nSigId, &varV, 4, 100);
				// printf("CtrlMode = [%d]\n",varV.emValue);
			}
			break;
			case MD_SIG_Ctrl_RunMode : // 1001, 3907,  1001,	DAITYPE_SETT_PARAMVALUE_RW,3907,
			{
				pModbus->strCtrl.emRunMode = (_ENUM)iData;
				// printf("emRunMode = [%d]\n",pModbus->strCtrl.emRunMode);
				varV.emValue = (_ENUM)iData;
				// LJH251101 new
				err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				pMdDataItem->nDevUnitId, pMdDataItem->nSigId, &varV, 4, 100);
			}
			break;
			case MD_SIG_Ctrl_ActivePower : // 1001, 3901,  1001,	DAITYPE_SETT_PARAMVALUE_RW,3901, 
			{
				// varV.emValue = 1;
				// err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,
				// 	1001, 3902, &varV, 4, 100);
				pModbus->strCtrl.fActivePower = fData;
				// printf("fActivePower = [%.1f]\n",pModbus->strCtrl.fActivePower);

				// LJH251101 old 修改modbus tcp 临时给陕西雁塔 众树使用
				// if (pModbus->strCtrl.emRunMode == 0)//关机
				// {
				// 	varV.fValue = 0.0;
				// }
				// else if (pModbus->strCtrl.emRunMode == 2) //放电
				// {
				// 	varV.fValue = (-1) * pModbus->strCtrl.fActivePower;
				// }
				// else //充电
				// {
				// 	varV.fValue = pModbus->strCtrl.fActivePower;
				// }
				// LJH251101 new
				varV.fValue = fData;
				err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
					pMdDataItem->nDevUnitId, pMdDataItem->nSigId, &varV, 4, 100);

			}
			break;
			case MD_SIG_Src_Cod :	
 			{
				pModbus->strGHSN.fSrcCod = fData;
				
				varV.emValue = 1;
				err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,
					pMdDataItem->nDevUnitId, pMdDataItem->nSigId, &varV, 4, 100);
				// printf("SrcCod:::err =%d fData=%f ->nSigType=%d ->nDevUnitId=%d ->nSigId=%d\n",\
				// 	err,fData,pMdDataItem->nSigType,pMdDataItem->nDevUnitId,pMdDataItem->nSigId);
			}
			break;
			case MD_SIG_TotP_Cod :	
 			{
				varV.emValue = 1;
				err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,
					1001, 3902, &varV, 4, 100);


				pModbus->strGHSN.fTotPCod = fData;
				
				varV.fValue = (-1)*pModbus->strGHSN.fTotPCod;
				err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
					pMdDataItem->nDevUnitId, pMdDataItem->nSigId, &varV, 4, 100);

				// printf("TotP:::err =%d fData=%f ->nSigType=%d ->nDevUnitId=%d ->nSigId=%d\n",\
				// 	err,fData,pMdDataItem->nSigType,pMdDataItem->nDevUnitId,pMdDataItem->nSigId);
			}
			break;
			case MD_SIG_TotQ_Cod :	
 			{
				pModbus->strGHSN.fTotQCod = fData;
				// printf("err =%d fData=%d ->nSigType=%d ->nDevUnitId=%d ->nSigId=%d\n",\
				// 	err,fData,pMdDataItem->nSigType,pMdDataItem->nDevUnitId,pMdDataItem->nSigId);
			}
			break;
			case MD_SIG_Run_Cod :	
 			{
				pModbus->strGHSN.fRunCod = fData;

				// varV.emValue = (fData==0.0) ? 1 : 0;
				// err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				// 	pMdDataItem->nDevUnitId, pMdDataItem->nSigId, &varV, 4, 100);

				// printf("Run:::err =%d fData=%f ->nSigType=%d ->nDevUnitId=%d ->nSigId=%d\n",\
				// 	err,fData,pMdDataItem->nSigType,pMdDataItem->nDevUnitId,pMdDataItem->nSigId);
			}
			break;
			case MD_SIG_OffGrd_Cod :
 			{
				pModbus->strGHSN.fOffGrdCod = fData;
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
					200, 218, &valGet, sizeof(PARAM_VAL_GET));
				// printf("valGet.val.emValue == %d\n", valGet.val.emValue);

				if(valGet.val.emValue == 1) // 并离网：手动控制模式
				{
				// printf("1111111\n");
					// int reTryTimes=20, tryOkFlag=0;
					// while(reTryTimes--)
					// {
					// 	if(err == ERRCODE_DAI_OK)
					// 	{
					// 		break;
					// 		tryOkFlag = 1;
					// 	}
					// 	printf("1111111---err = %d\n", err);
					// 	Sleep_Ex(1000);
					// }
					// if(tryOkFlag)
					// {
						varV.emValue = (fData==0.0) ? 0 : 1; // 0:OnGrid 1:OffGrid
						if(varV.emValue == 1)// 手动离网
						{
							// printf("ModTcp to OffGrid 1\n");
							if((pModbus->esRunData.fV_PCS3Phase[0][0] < 20) || (pModbus->esRunData.fV_PCS3Phase[0][1] < 20) || (pModbus->esRunData.fV_PCS3Phase[0][2] < 20)) //PCS三相无电压 才可下发离网命令到PCS
							{
								// printf("ModTcp to OffGrid 2\n");
								err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
									pMdDataItem->nDevUnitId, pMdDataItem->nSigId, &varV, 4, 3000);
								
								// varV.emValue = 1;
								// err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,
								// 	1001, 3902, &varV, 4, 100);
								// printf("2222222---err = %d\n", err);
								
								// varV.fValue = -5;// 给个功率，让模块跑起来，自动给负载放电(验证不需要)
								// err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
								// 	1001, 3901, &varV, 4, 100);
								// printf("3333333---err = %d\n", err);
							}
							
						}
						else // 并网
						{
							// printf("ModTcp to OnGrid\n");
							err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
								pMdDataItem->nDevUnitId, pMdDataItem->nSigId, &varV, 4, 3000);


							// varV.emValue = 1;
							// err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,
							// 	1001, 3902, &varV, 4, 100);
							// printf("2222222---err = %d\n", err);
	
							// varV.fValue = pModbus->strGHSN.fTotQCod;// 并网，按照之前指令下发有功功率
							// err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							// 	1001, 3901, &varV, 4, 100);
							// printf("3333333---err = %d\n", err);
						}
					// }					
				}
			}
			break;

			/*case MD_IDTAG_SET_CHARGER_POWER:
			{
	 
			}
			break;
			case MD_SIG_ADJ_CHARGER_INPUT_POWER:
			{
 
			}
			break;
			case MD_IDTAG_SWITCH_CHARGER_ONOROFF: //Jimmy added 20210721 for Enable/Disable the system
			{
 
			}
		 	break;*/
			default:break;
		}
 	}
   return bRet;
}

static BOOL Compare_IfNeedUpdate_EMSParam(REPORTER *pReporter)
{
	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;

    BOOL bRet = FALSE;
    int n = 0;

    pModbus->strC2S_Ems.nEmsSectorNum = MIN(MAX_EMS_SECTOR, pModbus->strC2S_Ems.nEmsSectorNum);
    if(pModbus->strLocal_Ems.nEmsSectorNum != pModbus->strC2S_Ems.nEmsSectorNum)
    {
        TRACE("strLocal_Ems.nEmsSectorNum=[%d] strC2S_Ems.nEmsSectorNum=[%d]\n", pModbus->strLocal_Ems.nEmsSectorNum , pModbus->strC2S_Ems.nEmsSectorNum);
        return TRUE;
    }

    for(n = 0; n < pModbus->strC2S_Ems.nEmsSectorNum; n++)
    {
        if(pModbus->strC2S_Ems.arryEmsParam[n].tmStart != pModbus->strLocal_Ems.arryEmsParam[n].tmStart)
        {
            TRACE("strLocal_Ems.tmStart=[%ld] strC2S_Ems.tmStart=[%ld]\n", \
                pModbus->strC2S_Ems.arryEmsParam[n].tmStart , pModbus->strLocal_Ems.arryEmsParam[n].tmStart);

            bRet = TRUE;
            break;
        }

        if(pModbus->strC2S_Ems.arryEmsParam[n].nStopSOC != pModbus->strLocal_Ems.arryEmsParam[n].nStopSOC)
        {
            TRACE("strLocal_Ems.nStopSOC=[%d] strC2S_Ems.nStopSOC=[%d]\n", \
                pModbus->strC2S_Ems.arryEmsParam[n].nStopSOC , pModbus->strLocal_Ems.arryEmsParam[n].nStopSOC);

            bRet = TRUE;
            break;
        }

        if(pModbus->strC2S_Ems.arryEmsParam[n].nBattGrpPwrLmt != pModbus->strLocal_Ems.arryEmsParam[n].nBattGrpPwrLmt)
        {
            TRACE("strLocal_Ems.nBattGrpPwrLmt=[%d] strC2S_Ems.nBattGrpPwrLmt=[%d]\n", \
                pModbus->strC2S_Ems.arryEmsParam[n].nBattGrpPwrLmt , pModbus->strLocal_Ems.arryEmsParam[n].nBattGrpPwrLmt);

            bRet = TRUE;
            break;
        }

        if(pModbus->strC2S_Ems.arryEmsParam[n].nActvPwrLmt != pModbus->strLocal_Ems.arryEmsParam[n].nActvPwrLmt)
        {
            bRet = TRUE;
            break;
        }

        if(pModbus->strC2S_Ems.arryEmsParam[n].nReActvPwrLmt != pModbus->strLocal_Ems.arryEmsParam[n].nReActvPwrLmt)
        {
            bRet = TRUE;
            break;
        } 

    }

    return bRet;
}


static void Update_EMSParam2Local(REPORTER *pReporter)
{
	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	ERRCODE_DAI err = -1;
	PARAM_VAL_GET valGet;
    int n = 0;
    int nInterval_Sec = 0;
    time_t tm_Now = _NOW_;
    static time_t tm_LastTM ;

    if((tm_Now - tm_LastTM) < 10)
    {
        return;
    }
    tm_LastTM = tm_Now;


	struct tm gmTime_Sec_Start;
 
	gmtime_r(&tm_Now, &gmTime_Sec_Start);
	gmTime_Sec_Start.tm_sec = 0;
	gmTime_Sec_Start.tm_min = 0;
	gmTime_Sec_Start.tm_hour = 0;
	
	TRACE("tm_year=[%d] tm_mon=[%d] tm_mday=[%d]\n",\
		gmTime_Sec_Start.tm_year ,
		gmTime_Sec_Start.tm_mon ,
		gmTime_Sec_Start.tm_mday);

	time_t tmNewStart = mktime(&gmTime_Sec_Start);		

	TRACE("tm_Now =[%ld] tmNewStart=[%ld]\n", tm_Now, tmNewStart);

 
    int nSectorNum = MIN(MAX_EMS_SECTOR,pModbus->strC2S_Ems.nEmsSectorNum);
	for(n = 0; n < nSectorNum; n++)
	{	
		pModbus->emsPlan.nTotal_EmsLmt_Sectors = nSectorNum; //Total Sector Number

		nInterval_Sec = pModbus->strC2S_Ems.arryEmsParam[n].tmStart*60;

		pModbus->emsPlan.emsLmtSectors[n].tmStart = tmNewStart + nInterval_Sec;

		if(nSectorNum <= 1)
		{
			pModbus->emsPlan.emsLmtSectors[n].nInterval = 86400;
		}
		else
		{
			if((n+1) < nSectorNum)
			{
				pModbus->emsPlan.emsLmtSectors[n].nInterval = 
					(pModbus->strC2S_Ems.arryEmsParam[n+1].tmStart - pModbus->strC2S_Ems.arryEmsParam[n].tmStart)*60;
			}
			else
			{
				pModbus->emsPlan.emsLmtSectors[n].nInterval = 86400 - (pModbus->strC2S_Ems.arryEmsParam[n].tmStart*60);
			}
		}

        pModbus->emsPlan.emsLmtSectors[n].nStopSOC = pModbus->strC2S_Ems.arryEmsParam[n].nStopSOC;

        if(pModbus->strC2S_Ems.arryEmsParam[n].nActvPwrLmt < 0)
        {
		    pModbus->emsPlan.emsLmtSectors[n].emBatt_WorkMode = BATT_WORKMODE_FIRST_DISCHRG;

        } else
        {
            pModbus->emsPlan.emsLmtSectors[n].emBatt_WorkMode = BATT_WORKMODE_FIRST_CHRGING; 
        }

		pModbus->emsPlan.emsLmtSectors[n].nBattGrpPwrLmt =  pModbus->strC2S_Ems.arryEmsParam[n].nBattGrpPwrLmt;

		pModbus->emsPlan.emsLmtSectors[n].nActvPwrLmt =  ABS(pModbus->strC2S_Ems.arryEmsParam[n].nActvPwrLmt); 

		pModbus->emsPlan.emsLmtSectors[n].nReActvPwrLmt = pModbus->strC2S_Ems.arryEmsParam[n].nReActvPwrLmt;  
    }

    VAR_VALUE varV;
	pModbus->emsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	varV.blobValue = &pModbus->emsPlan;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
			200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN), 1000);
	if (err == ERRCODE_DAI_OK)
	{
		TRACE("Info: EMS Config Plan Set Sucessfully\n");
        memcpy(&pModbus->strLocal_Ems, &pModbus->strC2S_Ems, sizeof(STR_PARAM_EMS));
	}
	else
	{
		TRACEX("Error: EMS Config Plan Update Failure!\n");
		//Revert EMS Config Settings
		valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
		valGet.val.blobValue = (BYTE*)&pModbus->emsPlan;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 201, &valGet, sizeof(PARAM_VAL_GET));
	}

    return;
}

static BOOL SpecialProEMSMgmtPlan(REPORTER *pReporter, MD_DATA_ITEM *pMdDataItem, BYTE *pCmdData , int nRegCount)
{
	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	BOOL 			bRet = TRUE;
	ERRCODE_DAI 	err;
 	VAR_VALUE 		varV;
	int				n;
	//UNION_COMPLEXTYPE	unionRecv;
	UNION_SHORT			unionShort;


	TRACE("Sett EMS nRegCount=[%d] nSigType=[%d]\n", nRegCount, pMdDataItem->nSigType);

	if(nRegCount < 6) // 1:SectorNum, 5-Mgmt Plan Section[N]
	{
		return FALSE;
	}

 	//if(pMdDataItem->nSigType != DAITYPE_SETT_PARAMVALUE_RW)
	//{
	//	//return FALSE;
	//}
 
	WORD nSectorNum = MAKEWORD(*(pCmdData+6),*(pCmdData+5));
	pModbus->strC2S_Ems.nEmsSectorNum = MIN(MAX_EMS_SECTOR, nSectorNum);

 	
	for(n = 0; n < pModbus->strC2S_Ems.nEmsSectorNum; n++)
	{
		TRACE("nSectorNum=[%d] nRegCount=[%d]\n", pModbus->strC2S_Ems.nEmsSectorNum, nRegCount);
		if((pModbus->strC2S_Ems.nEmsSectorNum*5) > nRegCount) //Every Section contain 5 Register
		{
			break;
		}

		//unionRecv.byValue[0] = *(pCmdData+10+n*10);
		//unionRecv.byValue[1] = *(pCmdData+9+n*10);
		//unionRecv.byValue[2] = *(pCmdData+8+n*10);
		//unionRecv.byValue[3] = *(pCmdData+7+n*10);

		pModbus->strC2S_Ems.arryEmsParam[n].tmStart 		= MAKEWORD(*(pCmdData+8+n*10),*(pCmdData+7+n*10));
		pModbus->strC2S_Ems.arryEmsParam[n].nStopSOC 		= MAKEWORD(*(pCmdData+10+n*10),*(pCmdData+9+n*10));
		pModbus->strC2S_Ems.arryEmsParam[n].nBattGrpPwrLmt 	= MAKEWORD(*(pCmdData+12+n*10),*(pCmdData+11+n*10));
		
		unionShort.byValue[0] = *(pCmdData+14+n*10);
		unionShort.byValue[1] = *(pCmdData+13+n*10);
		pModbus->strC2S_Ems.arryEmsParam[n].nActvPwrLmt 	= unionShort.sVal; 

		unionShort.byValue[0] = *(pCmdData+16+n*10);
		unionShort.byValue[1] = *(pCmdData+15+n*10);
		pModbus->strC2S_Ems.arryEmsParam[n].nReActvPwrLmt 	= unionShort.sVal; 

		TRACE("Idx=[%d] tm=[%ld] nStopSOC=[%d] nBattGrpPwrLmt=[%d] nActvPwrLmt=[%d] nReActvPwrLmt=[%d]\n",\
			n, 
			pModbus->strC2S_Ems.arryEmsParam[n].tmStart,\
			pModbus->strC2S_Ems.arryEmsParam[n].nStopSOC,\
			pModbus->strC2S_Ems.arryEmsParam[n].nBattGrpPwrLmt,\
			pModbus->strC2S_Ems.arryEmsParam[n].nActvPwrLmt,\
			pModbus->strC2S_Ems.arryEmsParam[n].nReActvPwrLmt);

	}
 	
	BOOL bNeedUpdate = Compare_IfNeedUpdate_EMSParam(pReporter); 
    if(bNeedUpdate == TRUE)
    {
        Update_EMSParam2Local(pReporter);
    }
    else
    {
        TRACE("!!! Warning:Same EMS Plan, NoNeed Update!!!\n");
    }

	 TRACEX("Special Process EMS Mgmt Plan : err =%d varV.emValue=%d ->nSigType=%d ->nDevUnitId=%d ->nSigId=%d\n",\
		err,varV.emValue,pMdDataItem->nSigType,pMdDataItem->nDevUnitId,pMdDataItem->nSigId);
   
   	return bRet;
}
