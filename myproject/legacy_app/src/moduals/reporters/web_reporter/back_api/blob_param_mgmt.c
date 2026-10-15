#include "blob_param_mgmt.h"

#define DEF_SUBBLOB_ITEM(p, _nSubBlobIdx, _pData, _nNameRes, _nUnitRes, _nMin, _nMax, _bSetAble, _nAuthLevel) \
            do{ \
                (p)->nSubParam_UniqId = _nSubBlobIdx; \
                (p)->pDataVal = _pData; \
                (p)->nName_ResId = _nNameRes;\
                (p)->nUnit_ResId = _nUnitRes;\
                (p)->fMinRange = (float)_nMin;\
                (p)->fMaxRange = (float)_nMax;\
                (p)->bSetAble = _bSetAble;\
                (p)->uMaskAuthorityLevel = _nAuthLevel;\
            }while(0)

static void Make_UINT_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, int nUnitRes, BOOL bSetAble, UINT nMin, UINT nMax, UINT nAuthLevel)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_UINT;
    pBlobEle->nValueSize = 4;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, nUnitRes, nMin, nMax, bSetAble, nAuthLevel);
}

static void Make_INT_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, int nUnitRes, BOOL bSetAble, int nMin, int nMax, UINT nAuthLevel)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_INT;
    pBlobEle->nValueSize = 4;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, nUnitRes, nMin, nMax, bSetAble, nAuthLevel);
}

static void Make_BYTE_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, int nUnitRes, BOOL bSetAble, int nMin, int nMax, UINT nAuthLevel)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_BYTE;
    pBlobEle->nValueSize = 1;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, nUnitRes, nMin, nMax, bSetAble, nAuthLevel);
}

static void Make_USHORT_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, int nUnitRes, BOOL bSetAble, int nMin, int nMax, UINT nAuthLevel)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_USHORT;
    pBlobEle->nValueSize = 2;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, nUnitRes, nMin, nMax, bSetAble, nAuthLevel);
}

static void Make_FLOAT_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, int nUnitRes, BOOL bSetAble, float nMin, float nMax, UINT nAuthLevel, int nPrecision)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_FLOAT;
    pBlobEle->nValueSize = 4;
    pBlobEle->nPrecision = nPrecision;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, nUnitRes, nMin, nMax, bSetAble, nAuthLevel);
}

static void Make_DATETIME_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, BOOL bSetAble, UINT nAuthLevel)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_DATETIME;
    pBlobEle->nValueSize = 4;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, 0, 0, 0, bSetAble, nAuthLevel);
}

static void Make_IP_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, BOOL bSetAble, UINT nAuthLevel)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_IP;
    pBlobEle->nValueSize = 4;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, 0, 0, 0, bSetAble, nAuthLevel);
}

static void Make_STRING_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, BOOL bSetAble, UINT nAuthLevel, UINT nValSize)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_STRING;
    pBlobEle->nValueSize = nValSize;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, 0, 0, 0, bSetAble, nAuthLevel);
}

static void Make_ENUM_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, int nUnitRes, BOOL bSetAble, UINT nAuthLevel, int nEnumTotal, ...)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_ENUM;
    pBlobEle->nValueSize = 4;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, nUnitRes, 0, 99999, bSetAble, nAuthLevel);
    //now create the enum list
    va_list		enumList;
    va_start(enumList, nEnumTotal);
    int n;
    nEnumTotal = MIN(nEnumTotal, MAX_SUPPORTED_SUBBLOB_WEB_ENUMITEM);
    pBlobEle->nEnumTotal = nEnumTotal;
    for(n = 0; n < nEnumTotal; n++)
    {
        pBlobEle->enumList[n].emVal = (BYTE)va_arg(enumList, int);
        pBlobEle->enumList[n].nEnumName_ResId = (USHORT)va_arg(enumList, int);
    }    
    va_end(enumList);      
}

static void Make_BIT_SubBlobParam(WEB_SUBBLOB_ITEM* pBlobEle, int nSubBlobIdx, void* pData, int nNameRes, UINT nBits, UINT nBitOffset, BOOL bSetAble, UINT nAuthLevel, int nEnumTotal, ...)
{
    ZERO_POBJS(pBlobEle, 1);
    pBlobEle->emSubBlobValType = SUB_BLOB_VALTYPE_BIT;
    pBlobEle->nValueSize = 4;
    pBlobEle->nBits = nBits;
    pBlobEle->nBitOffset = nBitOffset;
    DEF_SUBBLOB_ITEM(pBlobEle, nSubBlobIdx, pData, nNameRes, 0, 0, 99999, bSetAble, nAuthLevel);
    //now create the enum list
    va_list		enumList;
    va_start(enumList, nEnumTotal);
    int n;
    nEnumTotal = MIN(nEnumTotal, MAX_SUPPORTED_SUBBLOB_WEB_ENUMITEM);
    pBlobEle->nEnumTotal = nEnumTotal;
    for(n = 0; n < nEnumTotal; n++)
    {
        pBlobEle->enumList[n].emVal = (BYTE)va_arg(enumList, int);
        pBlobEle->enumList[n].nEnumName_ResId = (USHORT)va_arg(enumList, int);
    }    
    va_end(enumList);      
}

static void SetSubBlob_TrueReadOnlyList(WEB_SUBBLOB_ITEM* pBlobEle, int nTotal, ...)
{
    pBlobEle->nTrueReadOnlyTotal = nTotal;
    pBlobEle->pnTrueReadOnlyList = NEW(int, nTotal);
    va_list		list;
    va_start(list, nTotal);
    int n;
    for(n = 0; n < nTotal; n++)
    {
        pBlobEle->pnTrueReadOnlyList[n] = va_arg(list, int);
    }
    va_end(list);
}

BOOL Create_AWebBlob_Param(A_WEBBLOB_PARAM* pBlob2Create, WEB_PARAM_VALUE* pParamVal, void* pBlobVal_Raw)
{
    if(!pParamVal)
    {
        return FALSE;
    }
    //TRACEX("===Now create Blob[%d]===\n", pParamVal->pParamInfo->nBlobId);
    pBlob2Create->pParamVal = pParamVal;
    BOOL bNeedCreate = FALSE;
    if(!pBlobVal_Raw)
    {
        bNeedCreate = TRUE;
        //now need create the blob here
        //Cerate_Ablob_ValMemory();
        //will implement it later
        switch(pParamVal->pParamInfo->nBlobId)
		{
		
		case BLOB_TYPE_NETWORKPARAMS:
			pBlobVal_Raw = NEW(A_BLOB_NETWORKINFO, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_NETWORKPARAMS;
			break;
		// case BLOB_TYPE_CHARGE_START_CMD:
		// 	pBlobVal_Raw = NEW(A_BLOB_CHARGE_START_CMD, 1);
		// 	break;
		//case BLOB_TYPE_RUN_CHARGE_INFO:
		//	pBlobVal_Raw = NEW(A_BLOB_RUN_CHARGE_INFO, 1);
        //    //need copy the original value back so blob-pram only change those modified
        //    memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
        //    *(int*)pBlobVal_Raw = BLOB_TYPE_RUN_CHARGE_INFO;
		//	break;
		//case BLOB_TYPE_RUN_BMS_INFO:
		//	pBlobVal_Raw = NEW(A_BLOB_RUN_BMSINFO, 1);
        //    //need copy the original value back so blob-pram only change those modified
        //    memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
        //    *(int*)pBlobVal_Raw = BLOB_TYPE_RUN_BMS_INFO;
		//	break;
		// case BLOB_TYPE_RESERVE_CMD:
		// 	pInfo->nValueSize = sizeof(A_BLOB_RESREVE_CMD);
		// 	break;
		// case BLOB_TYPE_CHARGE_FEE_SET:
		// 	pInfo->nValueSize = sizeof(A_BLOB_CHARGE_FEESET);
		// 	break;
		// case BLOB_TYPE_CHARGE_STOP_CMD:
		// 	pInfo->nValueSize = sizeof(A_BLOB_CHARGE_STOP_CMD);
		// 	break;
		// case BLOB_TYPE_UPDATE_LOCALLIST:
		// 	pInfo->nValueSize = sizeof(A_BLOB_UPDATE_LOCALLIST);
		// 	break;
		// case BLOB_TYPE_UPDATE_AUTHSTATUS_ANDCACHE:
		// 	pInfo->nValueSize = sizeof(A_BLOB_UPDATE_AUTHSTAT_ANDCACHE);
		// 	break;
		//case BLOB_TYPE_OCPP_CONFIGURATIONS:
		//	pBlobVal_Raw = NEW(A_BLOB_OCPP_CONFIGS, 1);
        //    //need copy the original value back so blob-pram only change those modified
        //    memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
        //    *(int*)pBlobVal_Raw = BLOB_TYPE_OCPP_CONFIGURATIONS;
		//	break;
		// case BLOB_TYPE_SET_CHARGEPROFILE:
		// 	pInfo->nValueSize = sizeof(A_BLOB_SET_CHARGINGPROFILE);
		// 	break;
		// case BLOB_TYPE_METERVALUES:
		// 	pInfo->nValueSize = sizeof(A_BLOB_METER_VALUES);
		// 	break;
		// case BLOB_TYPE_UPDATE_CHARG_STAT_METER:
		// 	pBlobVal_Raw = NEW(A_BLOB_UPDATE_CHARGEMETER_STAT, 1);
		// 	break;
        //case BLOB_TYPE_BATT_MGMT_PLAN:
        //    pBlobVal_Raw = NEW(A_BLOB_BATT_MGMT_PLAN, 1);
        //    //need copy the original value back so blob-pram only change those modified
        //    memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
        //    *(int*)pBlobVal_Raw = BLOB_TYPE_BATT_MGMT_PLAN;
		//	break;
        case BLOB_TYPE_ES_BACKEND_CFG:
        	pBlobVal_Raw = NEW(A_BLOB_ES_BACKEND_CFG, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_ES_BACKEND_CFG;
			break;
		case BLOB_TYPE_CHARGER_BOOTINFO:
			pBlobVal_Raw = NEW(A_BLOB_CHARGER_BOOTINFO, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_CHARGER_BOOTINFO;
			break;
		// case BLOB_TYPE_UPDATE_FIRMWARE_CMD:
		// 	pInfo->nValueSize = sizeof(A_BLOB_UPDATEFIRMWARE_CMD);
		// 	break;
		// case BLOB_TYPE_CHAGER_STATUSINFO:
		// 	pInfo->nValueSize = sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO);
		// 	break;
		case BLOB_TYPE_CHARGER_SPECIFIC_SETTING:
			pBlobVal_Raw = NEW(A_BLOB_CHARGERSPECIFIC_SETTINGS, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_CHARGER_SPECIFIC_SETTING;
			break;
		// case BLOB_TYPE_UPLOAD_DIAGNOSTIC:
		// 	pInfo->nValueSize = sizeof(A_BLOB_UPLOADDIAGN_CMD);
		// 	break;
        case BLOB_TYPE_MANUALCTRL_RECT: case BLOB_TYPE_MANUALCTRL_RECT_SCADA:
			pBlobVal_Raw = NEW(A_BLOB_MANUALCTRL_RECT_CMD, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);
            if (pParamVal->pParamInfo->nBlobId == BLOB_TYPE_MANUALCTRL_RECT)
            {
                *(int*)pBlobVal_Raw = BLOB_TYPE_MANUALCTRL_RECT;
            }
            else
            {
                *(int*)pBlobVal_Raw = BLOB_TYPE_MANUALCTRL_RECT_SCADA;
            }
			break;
        case BLOB_TYPE_SINGLE_STBATT_RUNINFO:
        	pBlobVal_Raw = NEW(A_BLOB_SINGLE_STBATT_RUNINFO, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
            break;
        case BLOB_TYPE_GRP_PM_DATAINFO:
             pBlobVal_Raw = NEW(A_BLOB_RECT_GRP_DATA, 1);
             //need copy the original value back so blob-pram only change those modified
             memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
             *(int*)pBlobVal_Raw = BLOB_TYPE_GRP_PM_DATAINFO;
             break;
        case BLOB_TYPE_BMS_TESTMODE_SET:
            pBlobVal_Raw = NEW(A_BLOB_BMS_TESTMODE_SET, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_BMS_TESTMODE_SET;
            break;
        case BLOB_TYPE_EMS_CONFIG_PLAN:
            pBlobVal_Raw = NEW(A_BLOB_EMS_CONFIG_PLAN, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_EMS_CONFIG_PLAN;

            TRACE("nBlobSize=%d A_BLOB_EMS_CONFIG_PLAN=%d\n", pParamVal->nBlobSize, sizeof(A_BLOB_EMS_CONFIG_PLAN));
            break;
        case BLOB_TYPE_CHARGE_FEE_SET:
            pBlobVal_Raw = NEW(A_BLOB_CHARGE_FEESET, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_CHARGE_FEE_SET;
            break;
		// case BLOB_TYPE_JIG_TEST_RET:
		// 	pBlobVal_Raw = NEW(A_BLOB_JIG_TEST_INFO);
		// 	break;
		// case BLOB_TYPE_CLEAR_CHARGINGPROFILE:
		// 	pInfo->nValueSize = sizeof(A_BLOB_CLEAR_CHARGINGPROFILE);
		// 	break;
		// case BLOB_TYPE_CTRL_RECT_UPDATE:
		// 	pInfo->nValueSize = sizeof(A_BLOB_RECT_UPDT_INFO_4CTRL);
		// 	break;
        case BLOB_TYPE_BRANCH_NODE_CALC_DATA:
            pBlobVal_Raw = NEW(A_BLOB_BRANCH_NODE_CALC_DATA, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_BRANCH_NODE_CALC_DATA;
            break;
        case BLOB_TYPE_RECTGRP_AUTOCTRLCMD: // LJH241111 PcsSCADA
            pBlobVal_Raw = NEW(A_BLOB_GRP_RECT_AUTOCTRL_CMD, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_RECTGRP_AUTOCTRLCMD;
            break;
        
        case BLOB_TYPE_BRANCH_GRP_RECT_DATA:
        {
            pBlobVal_Raw = NEW(A_BLOB_BRANCH_GRP_RECTDATA, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_BRANCH_GRP_RECT_DATA;
            break;
        }
        case BLOB_TYPE_BATTCAB_IMEU2_V20:
        {
            pBlobVal_Raw = NEW(A_BLOB_BATTCAB_IMEU2_V20, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_BATTCAB_IMEU2_V20;
            break;
        }
        case BLOB_TYPE_ES_RUNDATA:
        {
            pBlobVal_Raw = NEW(A_BLOB_ES_RUN_DATA, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_ES_RUNDATA;
            break;
        }
        case BLOB_TYPE_SEMS_CLIENTNFO:
        {// LJH241202 SEMS ClientInfo
            pBlobVal_Raw = NEW(A_BLOB_SEMS_CLIENTINFO, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_SEMS_CLIENTNFO;
            break;
        }
        case BLOB_TYPE_SEMS_CLIENTNFO_SUM:
        {// LJH250313 ClientSum
            pBlobVal_Raw = NEW(A_BLOB_SEMS_CLIENTINFO_SUM, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_SEMS_CLIENTNFO_SUM;
            break;
        }
        case BLOB_TYPE_PCS_ONGRID_CTRL:
            pBlobVal_Raw = NEW(A_BLOB_CTRL_PCS_ONGRID_CMD, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_PCS_ONGRID_CTRL;
            break;
        case BLOB_TYPE_PCS_OFFGRID_CTRL:
            pBlobVal_Raw = NEW(A_BLOB_CTRL_PCS_OFFGRID_CMD, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_PCS_OFFGRID_CTRL;
            break;
        case BLOB_TYPE_KEEPALIVEDINFO:
            pBlobVal_Raw = NEW(A_BLOB_KEEPALIVEDINFO, 1);
            //need copy the original value back so blob-pram only change those modified
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_KEEPALIVEDINFO;
            break;
        case BLOB_TYPE_CMD155:
        {// LJH250428 cmd155
            pBlobVal_Raw = NEW(A_BLOB_CMD155, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);		
            *(int*)pBlobVal_Raw = BLOB_TYPE_CMD155;
            break;
        }
        case BLOB_TYPE_CMD157:
        {// LJH250520 cmd157
            pBlobVal_Raw = NEW(A_BLOB_CMD157, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);
            *(int*)pBlobVal_Raw = BLOB_TYPE_CMD157;
            break;
        }
        case BLOB_TYPE_DEVINSTALLCONFIG:
        {
            pBlobVal_Raw = NEW(A_BLOB_DEVINSTALLCONFIG, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);
            *(int*)pBlobVal_Raw = BLOB_TYPE_DEVINSTALLCONFIG;
            break;
        }
        case BLOB_TYPE_PV_INVERTER_DATA:
        {
            pBlobVal_Raw = NEW(A_BLOB_PV_INVERTER_DATA, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);
            *(int*)pBlobVal_Raw = BLOB_TYPE_PV_INVERTER_DATA;
            break;
        }
        case BLOB_TYPE_PV_INVERTER_DATA_TOTAL:
        {
            pBlobVal_Raw = NEW(A_BLOB_PV_INVERTER_DATA_TOTAL, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);
            *(int*)pBlobVal_Raw = BLOB_TYPE_PV_INVERTER_DATA_TOTAL;
            break;
        }
        case BLOB_TYPE_PV_SRTTING_BY_AEMS:
        {
            pBlobVal_Raw = NEW(A_BLOB_PV_CTRL_BY_AEMS, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);
            *(int*)pBlobVal_Raw = BLOB_TYPE_PV_SRTTING_BY_AEMS;
            break;
        }
        case BLOB_TYPE_ENERGY_STAT:
        {
            pBlobVal_Raw = NEW(A_BLOB_ENERGY_STAT, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);
            *(int*)pBlobVal_Raw = BLOB_TYPE_ENERGY_STAT;
            break;
        }
        case BLOB_TYPE_STATUS_STAT:
        {
            pBlobVal_Raw = NEW(A_BLOB_STATUS_STAT, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);
            *(int*)pBlobVal_Raw = BLOB_TYPE_STATUS_STAT;
            break;
        }
        case BLOB_TYPE_DIAG_STAT:
        {
            pBlobVal_Raw = NEW(A_BLOB_DIAG_STAT, 1);
            memcpy(pBlobVal_Raw, pParamVal->currValue.blobValue, pParamVal->nBlobSize);
            *(int*)pBlobVal_Raw = BLOB_TYPE_DIAG_STAT;
            break;
        }

        default:
			return FALSE;
		}        
    }
    pBlob2Create->pBlobVal_Raw = pBlobVal_Raw;   
    BOOL bCreateOK = FALSE;

    if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_ES_RUNDATA)
    {
        A_BLOB_ES_RUN_DATA* pESRunData = (A_BLOB_ES_RUN_DATA*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_ESRUN_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_ESRUN_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_PHA_INVOLT,  &pESRunData->fSTS_InputVolt[0], 1670,  161, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_PHA_INVOLT,  &pESRunData->fSTS_InputVolt[1], 1671,  161, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_PHA_INVOLT,  &pESRunData->fSTS_InputVolt[2], 1672,  161, TRUE, 0.0,   99999999.0, 2, 2);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_PHA_INVOLT,  &pESRunData->fSTS_InputCurt[0], 1673,  162, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_PHA_INVOLT,  &pESRunData->fSTS_InputCurt[1], 1674,  162, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_PHA_INVOLT,  &pESRunData->fSTS_InputCurt[2], 1675,  162, TRUE, 0.0,   99999999.0, 2, 2);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_PHA_INVOLT,  &pESRunData->fSTS_OutputVolt[0], 1676,  161, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_PHA_INVOLT,  &pESRunData->fSTS_OutputVolt[1], 1677,  161, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_PHA_INVOLT,  &pESRunData->fSTS_OutputVolt[2], 1678,  161, TRUE, 0.0,   99999999.0, 2, 2);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_AC_Freq,     &pESRunData->fAC_Freq,           143,  144, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_Ambit_Temp,  &pESRunData->fAmbit_Temp,        610,  127, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_Pactive,     &pESRunData->fPtotalActive,      159,  165, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_Preactive,   &pESRunData->fPtotalReactive,    160,  166, TRUE, 0.0,   99999999.0, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ESRUN_Pfactor,     &pESRunData->fPtotalFactor,      142,  0,   TRUE, 0.0,   99999999.0, 2, 2);

        //do not forget this
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_NETWORKPARAMS)
    {
        A_BLOB_NETWORKINFO* pNetInfo = (A_BLOB_NETWORKINFO*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_NETWORK_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_NETWORK_MAXID;
        //wired network #1
        Make_ENUM_SubBlobParam(pBlob2Create->pSubBlobList + 0, SUBIDX_NETWORK_STATUS_WIRED_1, &pNetInfo->netInfo_eht0.emNetPort_Status, 442, 0, FALSE, 0, 3, 
                0, 267, 1, 33, 2, 340);
        Make_ENUM_SubBlobParam(pBlob2Create->pSubBlobList + 1, SUBIDX_NETWORK_DHCP_WIRED_1, &pNetInfo->bDHCP_On_eth0, 89, 0, TRUE, 0, 2, 
                0, 91, 1, 90);
        SetSubBlob_TrueReadOnlyList(pBlob2Create->pSubBlobList + 1, 3, SUBIDX_NETWORK_IP_WIRED_1, SUBIDX_NETWORK_NETMASK_WIRED_1, SUBIDX_NETWORK_GATEWAY_WIRED_1);
        Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 2, SUBIDX_NETWORK_IP_WIRED_1, &pNetInfo->netInfo_eht0.ulIpV4_IP, 86, TRUE, 0);
        Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 3, SUBIDX_NETWORK_NETMASK_WIRED_1, &pNetInfo->netInfo_eht0.ulIpV4_Mask, 87, TRUE, 0);
        Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 4, SUBIDX_NETWORK_GATEWAY_WIRED_1, &pNetInfo->netInfo_eht0.ulIpV4_Gateway, 88, TRUE, 0);
		Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 5, SUBIDX_NETWORK_DNSSERVER_WIRED_1, &pNetInfo->netInfo_eht0.ulIpV4_DNSServer, 1116, TRUE, 0);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 6, SUBIDX_NETWORK_MAC_ADDR_1, &pNetInfo->netInfo_eht0.szMacAddr, 119, FALSE, 0, 18);
        //wired network #2
        Make_ENUM_SubBlobParam(pBlob2Create->pSubBlobList + 7, SUBIDX_NETWORK_STATUS_WIRED_2, &pNetInfo->netInfo_eht1.emNetPort_Status, 443, 0, FALSE, 0, 3, 
                0, 267, 1, 33, 2, 340);
        Make_ENUM_SubBlobParam(pBlob2Create->pSubBlobList + 8, SUBIDX_NETWORK_DHCP_WIRED_2, &pNetInfo->bDHCP_On_eth1, 89, 0, TRUE, 0, 2, 
                0, 91, 1, 90);
        SetSubBlob_TrueReadOnlyList(pBlob2Create->pSubBlobList + 8, 3, SUBIDX_NETWORK_IP_WIRED_2, SUBIDX_NETWORK_NETMASK_WIRED_2, SUBIDX_NETWORK_GATEWAY_WIRED_2);
        Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 9, SUBIDX_NETWORK_IP_WIRED_2, &pNetInfo->netInfo_eht1.ulIpV4_IP, 86, TRUE, 0);
        Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 10, SUBIDX_NETWORK_NETMASK_WIRED_2, &pNetInfo->netInfo_eht1.ulIpV4_Mask, 87, TRUE, 0);
        Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 11, SUBIDX_NETWORK_GATEWAY_WIRED_2, &pNetInfo->netInfo_eht1.ulIpV4_Gateway, 88, TRUE, 0);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 12, SUBIDX_NETWORK_MAC_ADDR_2, &pNetInfo->netInfo_eht1.szMacAddr, 119, FALSE, 0, 18);
        //wireless
        Make_ENUM_SubBlobParam(pBlob2Create->pSubBlobList + 13, SUBIDX_NETWORK_ENABLE_LTE_MODEM, &pNetInfo->bIsEnable_LTEModem, 38, 0, TRUE, 0, 6, 
                0, 231, 1, 1114, 2, 1115 ,3, 2720, 4, 2721, 5, 4020);
        Make_ENUM_SubBlobParam(pBlob2Create->pSubBlobList + 14, SUBIDX_NETWORK_STATUS_WIRELESS, &pNetInfo->netInfo_usb0.emNetPort_Status, 444, 0, FALSE, 0, 3, 
                0, 267, 1, 33, 2, 340);
        Make_ENUM_SubBlobParam(pBlob2Create->pSubBlobList + 15, SUBIDX_NETWORK_APN_SEL_WIRELESS, &pNetInfo->lteModemINfo.bIsManulSet_APN, 474, 0, TRUE, 0, 2, 
                0, 7, 1, 23);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 16, SUBIDX_NETWORK_APN_NAME_WIRELESS, &pNetInfo->lteModemINfo.szAPN, 473, TRUE, 0, 32);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 17, SUBIDX_NETWORK_APN_USER_WIRELESS, &pNetInfo->lteModemINfo.szAPN_User, 36, TRUE, 0, 32);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 18, SUBIDX_NETWORK_APN_PASSWD_WIRELESS, &pNetInfo->lteModemINfo.szAPN_Passwd, 37, TRUE, 0, 32);
        Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 19, SUBIDX_NETWORK_IP_WIRELESS, &pNetInfo->netInfo_usb0.ulIpV4_IP, 86, FALSE, 0);
        Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 20, SUBIDX_NETWORK_NETMASK_WIRELESS, &pNetInfo->netInfo_usb0.ulIpV4_Mask, 87, FALSE, 0);
        Make_IP_SubBlobParam(pBlob2Create->pSubBlobList + 21, SUBIDX_NETWORK_GATEWAY_WIRELESS, &pNetInfo->netInfo_usb0.ulIpV4_Gateway, 88, FALSE, 0);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 22, SUBIDX_NETWORK_MAC_ADDR_WIRELESS, &pNetInfo->netInfo_usb0.szMacAddr, 119, FALSE, 0, 18);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 23, SUBIDX_NETWORK_IMSI_WIRELESS, &pNetInfo->lteModemINfo.szIMSI, 447, FALSE, 0, 20);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 24, SUBIDX_NETWORK_ICCID_WIRELESS, &pNetInfo->lteModemINfo.szICCID, 448, FALSE, 0, 32);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 25, SUBIDX_NETWORK_NETTYPE_WIRELESS, &pNetInfo->lteModemINfo.szNetwork_Type, 449, FALSE, 0, 32);
        Make_INT_SubBlobParam(pBlob2Create->pSubBlobList + 26, SUBIDX_NETWORK_SIGLEVEL_WIRELESS, &pNetInfo->lteModemINfo.nSignal_dBm, 450, 451, FALSE, -99999, 99999, 0);
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 27, SUBIDX_NETWORK_SN_WIRELESS, &pNetInfo->lteModemINfo.szSN, 343, FALSE, 0, 64);
        
        //do not forget this
        bCreateOK = TRUE;
    }   
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_CHARGER_BOOTINFO)
    {
        A_BLOB_CHARGER_BOOTINFO* pBootSet = (A_BLOB_CHARGER_BOOTINFO*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_BOOTSET_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_BOOTSET_MAXID;
        //model
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 0, SUBIDX_BOOTSET_CHARERMODEL, &pBootSet->szChargePointModel, 342, TRUE, 10, 20);
        //SN
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 1, SUBIDX_BOOTSET_CHARERSN, &pBootSet->szChargePointSerualNuml, 343, TRUE, 10, 32);
        //vendor
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 2, SUBIDX_BOOTSET_CHARERVENDOR, &pBootSet->szChargePointVendor, 344, TRUE, 10, 20);
        //app version
        Make_STRING_SubBlobParam(pBlob2Create->pSubBlobList + 3, SUBIDX_BOOTSET_APPVERSION, &pBootSet->szFirmwareVer, 125, FALSE, 0, 50);
        //do not forget this
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_CHARGE_FEE_SET)
    {
        A_BLOB_CHARGE_FEESET* pFeeRate = (A_BLOB_CHARGE_FEESET*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_FEERATE_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_FEERATE_MAXID; 
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_INT_SubBlobParam(p++, SUBIDX_FEE_LMTSECTORS, &pFeeRate->nTotal_FeeRate_Sectors, 
            649, 0, TRUE,  0, 96, 2);
        
        //Sector1
        Make_DATETIME_SubBlobParam(p++, SUBIDX_FEE_SEC1_TMSTART, &pFeeRate->feeRateSectors[0].tmStart, 651,  TRUE, 2);
        Make_INT_SubBlobParam(p++, SUBIDX_FEE_SEC1_INTERVAL, &pFeeRate->feeRateSectors[0].nInterval, 652, 163, TRUE,  0, 86400, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_FEE_SEC1_FEERATE, &pFeeRate->feeRateSectors[0].emFeeRate, 276, 0, TRUE, 2, 4, 
            0, 866, 1, 278, 2, 279, 3, 280);
        
        //Sector2
        Make_DATETIME_SubBlobParam(p++, SUBIDX_FEE_SEC2_TMSTART, &pFeeRate->feeRateSectors[1].tmStart, 651,  TRUE, 2);
        Make_INT_SubBlobParam(p++, SUBIDX_FEE_SEC2_INTERVAL, &pFeeRate->feeRateSectors[1].nInterval, 652, 163, TRUE,  0, 86400, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_FEE_SEC2_FEERATE, &pFeeRate->feeRateSectors[1].emFeeRate, 276, 0, TRUE, 2, 4, 
            0, 866, 1, 278, 2, 279, 3, 280);
        
        //Sector3
        Make_DATETIME_SubBlobParam(p++, SUBIDX_FEE_SEC3_TMSTART, &pFeeRate->feeRateSectors[2].tmStart, 651,  TRUE, 2);
        Make_INT_SubBlobParam(p++, SUBIDX_FEE_SEC3_INTERVAL, &pFeeRate->feeRateSectors[2].nInterval, 652, 163, TRUE,  0, 86400, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_FEE_SEC3_FEERATE, &pFeeRate->feeRateSectors[2].emFeeRate, 276, 0, TRUE, 2, 4, 
            0, 866, 1, 278, 2, 279, 3, 280);

        //Sector4
        Make_DATETIME_SubBlobParam(p++, SUBIDX_FEE_SEC4_TMSTART, &pFeeRate->feeRateSectors[3].tmStart, 651,  TRUE, 2);
        Make_INT_SubBlobParam(p++, SUBIDX_FEE_SEC4_INTERVAL, &pFeeRate->feeRateSectors[3].nInterval, 652, 163, TRUE,  0, 86400, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_FEE_SEC4_FEERATE, &pFeeRate->feeRateSectors[3].emFeeRate, 276, 0, TRUE, 2, 4, 
            0, 866, 1, 278, 2, 279, 3, 280);

        //Sector5
        Make_DATETIME_SubBlobParam(p++, SUBIDX_FEE_SEC5_TMSTART, &pFeeRate->feeRateSectors[4].tmStart, 651,  TRUE, 2);
        Make_INT_SubBlobParam(p++, SUBIDX_FEE_SEC5_INTERVAL, &pFeeRate->feeRateSectors[4].nInterval, 652, 163, TRUE,  0, 86400, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_FEE_SEC5_FEERATE, &pFeeRate->feeRateSectors[4].emFeeRate, 276, 0, TRUE, 2, 4, 
            0, 866, 1, 278, 2, 279, 3, 280);

        //Sector6
        Make_DATETIME_SubBlobParam(p++, SUBIDX_FEE_SEC6_TMSTART, &pFeeRate->feeRateSectors[5].tmStart, 651,  TRUE, 2);
        Make_INT_SubBlobParam(p++, SUBIDX_FEE_SEC6_INTERVAL, &pFeeRate->feeRateSectors[5].nInterval, 652, 163, TRUE,  0, 86400, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_FEE_SEC6_FEERATE, &pFeeRate->feeRateSectors[5].emFeeRate, 276, 0, TRUE, 2, 4, 
            0, 866, 1, 278, 2, 279, 3, 280);

        //Sector7
        Make_DATETIME_SubBlobParam(p++, SUBIDX_FEE_SEC7_TMSTART, &pFeeRate->feeRateSectors[6].tmStart, 651,  TRUE, 2);
        Make_INT_SubBlobParam(p++, SUBIDX_FEE_SEC7_INTERVAL, &pFeeRate->feeRateSectors[6].nInterval, 652, 163, TRUE,  0, 86400, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_FEE_SEC7_FEERATE, &pFeeRate->feeRateSectors[6].emFeeRate, 276, 0, TRUE, 2, 4, 
            0, 866, 1, 278, 2, 279, 3, 280);

        //Sector8
        Make_DATETIME_SubBlobParam(p++, SUBIDX_FEE_SEC8_TMSTART, &pFeeRate->feeRateSectors[7].tmStart, 651,  TRUE, 2);
        Make_INT_SubBlobParam(p++, SUBIDX_FEE_SEC8_INTERVAL, &pFeeRate->feeRateSectors[7].nInterval, 652, 163, TRUE,  0, 86400, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_FEE_SEC8_FEERATE, &pFeeRate->feeRateSectors[7].emFeeRate, 276, 0, TRUE, 2, 4, 
            0, 866, 1, 278, 2, 279, 3, 280);

        //Sector9
        Make_DATETIME_SubBlobParam(p++, SUBIDX_FEE_SEC9_TMSTART, &pFeeRate->feeRateSectors[8].tmStart, 651,  TRUE, 2);
        Make_INT_SubBlobParam(p++, SUBIDX_FEE_SEC9_INTERVAL, &pFeeRate->feeRateSectors[8].nInterval, 652, 163, TRUE,  0, 86400, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_FEE_SEC9_FEERATE, &pFeeRate->feeRateSectors[8].emFeeRate, 276, 0, TRUE, 2, 4, 
            0, 866, 1, 278, 2, 279, 3, 280);
        
        bCreateOK = TRUE;

    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_CHARGER_SPECIFIC_SETTING)
    {
        A_BLOB_CHARGERSPECIFIC_SETTINGS* pCCUSet = (A_BLOB_CHARGERSPECIFIC_SETTINGS*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_CCU_BACK_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_CCU_BACK_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        //----  Bit Map1  ----//
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_EPO_VOLTLEVEL,        &pCCUSet->ccuSettings.sysAtributesMap1, 322,    1, 0,   TRUE, 10, 2, 0, 320, 1, 321); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_DOOROPEN_ENABLE,      &pCCUSet->ccuSettings.sysAtributesMap1, 325,    1, 1,   TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_SPD_ENABLE,           &pCCUSet->ccuSettings.sysAtributesMap1, 205,    1, 2,   TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_ACGridInputMCB,       &pCCUSet->ccuSettings.sysAtributesMap1, 1152,   1, 3,   TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_ACLoadOutputMCB,      &pCCUSet->ccuSettings.sysAtributesMap1, 1153,   1, 4,   TRUE, 10, 2, 0, 324, 1, 323); 
    //     //Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_AuxACMCB,             &pCCUSet->ccuSettings.sysAtributesMap1, 696,  1, 5,   TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_MainACInputContactor, &pCCUSet->ccuSettings.sysAtributesMap1, 1154,   1, 6,   TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_N_LineContactorDI,    &pCCUSet->ccuSettings.sysAtributesMap1, 1165,   1, 10,  TRUE, 10, 2, 0, 320, 1, 321); 

    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_LeakageCurr,          &pCCUSet->ccuSettings.sysAtributesMap1, 65,     1, 11,   TRUE, 10, 2, 0, 324, 1, 323); 

    //    Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_Smoke_Sensor,          &pCCUSet->ccuSettings.sysAtributesMap1, 1161,     1, 12, TRUE, 10, 2, 0, 324, 1, 323); 
    //    Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_Flooding_DI,           &pCCUSet->ccuSettings.sysAtributesMap1, 1162,     1, 13, TRUE, 10, 2, 0, 324, 1, 323); 
    //    Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_Topple_DI,             &pCCUSet->ccuSettings.sysAtributesMap1, 1163,     1, 14, TRUE, 10, 2, 0, 324, 1, 323); 
       
    //    Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_LowVoltMCBBreaker,     &pCCUSet->ccuSettings.sysAtributesMap1, 1164,    1, 15,   TRUE, 10, 2, 0, 320, 1, 321); 

    //     //----  Bit Map2  ----//
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGRP1_INSTSTAT,     &pCCUSet->ccuSettings.sysAtributesMap2, 346, 1, 0, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGRP2_INSTSTAT,     &pCCUSet->ccuSettings.sysAtributesMap2, 347, 1, 1, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGRP3_INSTSTAT,     &pCCUSet->ccuSettings.sysAtributesMap2, 348, 1, 2, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGRP4_INSTSTAT,     &pCCUSet->ccuSettings.sysAtributesMap2, 349, 1, 3, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_STS1_INSTSTAT,        &pCCUSet->ccuSettings.sysAtributesMap2, 809, 1, 4, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_STS2_INSTSTAT,        &pCCUSet->ccuSettings.sysAtributesMap2, 810, 1, 5, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_STS3_INSTSTAT,        &pCCUSet->ccuSettings.sysAtributesMap2, 811, 1, 6, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_STS4_INSTSTAT,        &pCCUSet->ccuSettings.sysAtributesMap2, 812, 1, 7, TRUE, 10, 2, 0, 324, 1, 323); 

    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_IMEU2_1_INSTSTAT,     &pCCUSet->ccuSettings.sysAtributesMap2, 328, 1, 8, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_IMEU2_2_INSTSTAT,     &pCCUSet->ccuSettings.sysAtributesMap2, 329, 1, 9, TRUE, 10, 2, 0, 324, 1, 323); 

    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGRP1_CAN_INSTSTAT, &pCCUSet->ccuSettings.sysAtributesMap2, 407, 1, 10, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGRP2_CAN_INSTSTAT, &pCCUSet->ccuSettings.sysAtributesMap2, 408, 1, 11, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGRP3_CAN_INSTSTAT, &pCCUSet->ccuSettings.sysAtributesMap2, 409, 1, 12, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGRP4_CAN_INSTSTAT, &pCCUSet->ccuSettings.sysAtributesMap2, 410, 1, 13, TRUE, 10, 2, 0, 324, 1, 323); 

    //     //----  Bit Map3  ----//
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp1_ForbidRun,       &pCCUSet->ccuSettings.sysAtributesMap3, 1204, 1, 0, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp2_ForbidRun,       &pCCUSet->ccuSettings.sysAtributesMap3, 1205, 1, 1, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp3_ForbidRun,       &pCCUSet->ccuSettings.sysAtributesMap3, 1206, 1, 2, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp4_ForbidRun,       &pCCUSet->ccuSettings.sysAtributesMap3, 1207, 1, 3, TRUE, 10, 2, 0, 324, 1, 323); 
       
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp1_InputContactor,  &pCCUSet->ccuSettings.sysAtributesMap3, 1208, 1, 4, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp2_InputContactor,  &pCCUSet->ccuSettings.sysAtributesMap3, 1209, 1, 5, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp3_InputContactor,  &pCCUSet->ccuSettings.sysAtributesMap3, 1210, 1, 6, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp4_InputContactor,  &pCCUSet->ccuSettings.sysAtributesMap3, 1211, 1, 7, TRUE, 10, 2, 0, 324, 1, 323); 
        
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp1_IsolationSwitch, &pCCUSet->ccuSettings.sysAtributesMap3, 1212, 1, 8, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp2_IsolationSwitch, &pCCUSet->ccuSettings.sysAtributesMap3, 1213, 1, 9, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp3_IsolationSwitch, &pCCUSet->ccuSettings.sysAtributesMap3, 1214, 1, 10, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_BATGrp4_IsolationSwitch, &pCCUSet->ccuSettings.sysAtributesMap3, 1215, 1, 11, TRUE, 10, 2, 0, 324, 1, 323); 
       
    //     //----  Bit Map4  ----//
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp1Out_Contactor,      &pCCUSet->ccuSettings.sysAtributesMap4, 1236, 1, 0, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp2Out_Contactor,      &pCCUSet->ccuSettings.sysAtributesMap4, 1237, 1, 1, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp3Out_Contactor,      &pCCUSet->ccuSettings.sysAtributesMap4, 1238, 1, 2, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp4Out_Contactor,      &pCCUSet->ccuSettings.sysAtributesMap4, 1239, 1, 3, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp1SPD_Detection,      &pCCUSet->ccuSettings.sysAtributesMap4, 1240, 1, 4, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp2SPD_Detection,      &pCCUSet->ccuSettings.sysAtributesMap4, 1241, 1, 5, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp3SPD_Detection,      &pCCUSet->ccuSettings.sysAtributesMap4, 1242, 1, 6, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp4SPD_Detection,      &pCCUSet->ccuSettings.sysAtributesMap4, 1243, 1, 7, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp1IN_IsolationSwitch, &pCCUSet->ccuSettings.sysAtributesMap4, 1244, 1, 8, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp2IN_IsolationSwitch, &pCCUSet->ccuSettings.sysAtributesMap4, 1245, 1, 9, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp3IN_IsolationSwitch, &pCCUSet->ccuSettings.sysAtributesMap4, 1246, 1, 10, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_PVGrp4IN_IsolationSwitch, &pCCUSet->ccuSettings.sysAtributesMap4, 1247, 1, 11, TRUE, 10, 2, 0, 324, 1, 323); 
       

    //     //----  Bit Map6  ----//
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_IMIU1_INSTSTAT,       &pCCUSet->ccuSettings.sysAtributesMap6, 1172, 1, 0, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_IMIU2_INSTSTAT,       &pCCUSet->ccuSettings.sysAtributesMap6, 1174, 1, 1, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_IMIU3_INSTSTAT,       &pCCUSet->ccuSettings.sysAtributesMap6, 1176, 1, 2, TRUE, 10, 2, 0, 324, 1, 323); 
    //     Make_BIT_SubBlobParam(p++, SUBIDX_CCUSETT_IMIU4_INSTSTAT,       &pCCUSet->ccuSettings.sysAtributesMap6, 1178, 1, 3, TRUE, 10, 2, 0, 324, 1, 323); 
       
        //----  PM Num Config  ----//
        Make_BYTE_SubBlobParam(p++, SUBIDX_PMGRP1_CFGNUM,       &pCCUSet->ccuSettings.byNumGrp_1, 721, 0, TRUE, 0, 8, 10);
        Make_BYTE_SubBlobParam(p++, SUBIDX_PMGRP2_CFGNUM,       &pCCUSet->ccuSettings.byNumGrp_2, 722, 0, TRUE, 0, 8, 10);
        Make_BYTE_SubBlobParam(p++, SUBIDX_PMGRP3_CFGNUM,       &pCCUSet->ccuSettings.byNumGrp_3, 723, 0, TRUE, 0, 8, 10);
        Make_BYTE_SubBlobParam(p++, SUBIDX_PMGRP4_CFGNUM,       &pCCUSet->ccuSettings.byNumGrp_4, 724, 0, TRUE, 0, 8, 10);
        // Make_BYTE_SubBlobParam(p++, SUBIDX_CCUSETT_PMGRP5_CFGNUM,       &pCCUSet->ccuSettings.byNumGrp_5, 741, 0, TRUE, 0, 8, 10);
        // Make_BYTE_SubBlobParam(p++, SUBIDX_CCUSETT_PMGRP6_CFGNUM,       &pCCUSet->ccuSettings.byNumGrp_6, 742, 0, TRUE, 0, 8, 10);
        // Make_BYTE_SubBlobParam(p++, SUBIDX_CCUSETT_PMGRP7_CFGNUM,       &pCCUSet->ccuSettings.byNumGrp_7, 743, 0, TRUE, 0, 8, 10);
        // Make_BYTE_SubBlobParam(p++, SUBIDX_CCUSETT_PMGRP8_CFGNUM,       &pCCUSet->ccuSettings.byNumGrp_8, 744, 0, TRUE, 0, 8, 10);

    //     //----  Bit Map5  ----//
    //     Make_BIT_SubBlobParam(p++,  SUBIDX_CCUSETT_ACFanOrDCFan,        &pCCUSet->ccuSettings.sysAtributesMap5, 2386, 1, 8, TRUE, 10, 2, 0, 2387, 1, 2388); 
    //     Make_BIT_SubBlobParam(p++,  SUBIDX_CCUSETT_DCFan_DI_AlmLevel,   &pCCUSet->ccuSettings.sysAtributesMap5, 2389, 1, 9, TRUE, 10, 2, 0, 2, 1, 1); 
        
    //     Make_INT_SubBlobParam(p++, SUBIDX_CCUSETT_SYSFAN3_OVERTEMP,     &pCCUSet->ccuSettings.nFan3_OverTemp4Cool,  930, 127, TRUE,  0, 150, 2);
    //     Make_INT_SubBlobParam(p++, SUBIDX_CCUSETT_SYSFAN3_ULTRATEMP,    &pCCUSet->ccuSettings.nFan3_UltraTemp4Cool, 931, 127, TRUE,  0, 150, 2);

        Make_ENUM_SubBlobParam(p++, SUBIDX_PCSTYPE, &pCCUSet->ccuSettings.byPCSType, 66, 0, TRUE, 2, 4, 
            0, 67, 1, 68, 2, 2722, 3, 2690);

        Make_ENUM_SubBlobParam(p++, SUBIDX_ADCMODE, &pCCUSet->ccuSettings.byDCorAC_AdjMode, 465, 0, TRUE, 2, 2, 0, 1864, 1, 1865);
        Make_ENUM_SubBlobParam(p++, SUBIDX_STARTMODE, &pCCUSet->ccuSettings.nStartupMode, 1012, 0, TRUE, 2, 2, 0, 1850, 1, 1851);
        Make_ENUM_SubBlobParam(p++, SUBIDX_WOEKMODE, &pCCUSet->ccuSettings.nworkMode, 1920, 0, TRUE, 2, 3, 0, 2687, 1, 2688, 2, 702);
        Make_ENUM_SubBlobParam(p++, SUBIDX_ESSMODE, &pCCUSet->ccuSettings.nessMode, 2582, 0, TRUE, 2, 2, 0, 324, 1, 323);
            
        
        bCreateOK = TRUE;
    }
        else if (pParamVal->pParamInfo->nBlobId == BLOB_TYPE_PCS_ONGRID_CTRL)
    {
        A_BLOB_CTRL_PCS_ONGRID_CMD* pPcsCtrl = (A_BLOB_CTRL_PCS_ONGRID_CMD*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_PCSCTRL_ONGRID_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_PCSCTRL_ONGRID_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_ENUM_SubBlobParam(p++, SUBIDX_PCSCTRL_CTRLTYPE, &pPcsCtrl->emCtrlType, 2723, 0, TRUE, 2, 3,
            0, 222, 1, 2687, 2, 2688);        // Make_INT_SubBlobParam(p++, SUBIDX_PCSCTRL_INDEX, &pPcsCtrl->nPMIndex, 2689, 0, TRUE, 0, 65535, 2);
        Make_ENUM_SubBlobParam(p++, SUBIDX_PCSCTRL_ONGRID_ENRGY, &pPcsCtrl->emEnergySchMode, 2727, 0, TRUE, 2, 2,
            0, 1864, 1, 1865);
        Make_ENUM_SubBlobParam(p++, SUBIDX_PCSCTRL_ONGRID_CODM, &pPcsCtrl->emChaOrDischMode, 2728, 0, TRUE, 2, 2,
            0, 1866, 1, 1867);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL_ONGRID_CURR, &pPcsCtrl->fGrpCurrLmt, 454, 162, TRUE, 0, 65535, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL_ONGRID_POWR, &pPcsCtrl->fGrpPwrLmt, 1968, 165, TRUE, 0, 65535, 2, 1);
        bCreateOK = TRUE;
    }
    else if (pParamVal->pParamInfo->nBlobId == BLOB_TYPE_PCS_OFFGRID_CTRL)
    {
        A_BLOB_CTRL_PCS_OFFGRID_CMD* pPcsCtrl = (A_BLOB_CTRL_PCS_OFFGRID_CMD*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_PCSCTRL_OFFGRID_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_PCSCTRL_OFFGRID_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_ENUM_SubBlobParam(p++, SUBIDX_PCSCTRL_OFFGRID_CTRLTYPE, &pPcsCtrl->emCtrlType, 2724, 0, TRUE, 2, 2,
            0, 222, 2, 2684);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL_OFFGRID_VOLT, &pPcsCtrl->fGrpVoltLmt, 453, 161, TRUE, 360, 440, 2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL_OFFGRID_FREQ, &pPcsCtrl->fGrpFreqLmt, 2685, 144, TRUE, 50, 60, 2, 3);
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_MANUALCTRL_RECT || pParamVal->pParamInfo->nBlobId == BLOB_TYPE_MANUALCTRL_RECT_SCADA)
    {
        TRACED("Create_AWebBlob_Param nBlobId:%d", pParamVal->pParamInfo->nBlobId);
        A_BLOB_MANUALCTRL_RECT_CMD* pRectCtrl = (A_BLOB_MANUALCTRL_RECT_CMD*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_RECTCTRL_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_RECTCTRL_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;
        
        Make_ENUM_SubBlobParam(p++, SUBIDX_RECTCTRL_WORK_MODE, &pRectCtrl->emState, 465, 0, TRUE, 2, 5, 
            0, 222, 1, 717, 2, 718, 3, 719, 4, 720);
        Make_ENUM_SubBlobParam(p++, SUBIDX_RECTCTRL_GRP_NUMBER, &pRectCtrl->nGrp, 455, 0, TRUE, 2, 9, 
            0, 456, 1, 1126, 2, 1127, 3, 1128, 4, 1129, 5, 1130, 6, 1131, 7, 1132, 8, 1133);    

        Make_FLOAT_SubBlobParam(p++, SUBIDX_RECTCTRL_ADJ_VOLT, &pRectCtrl->fGrpVolt, 453, 161, TRUE, 0, 65535, 2, 0);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_RECTCTRL_ADJ_CURR, &pRectCtrl->fGrpCurr, 454, 162, TRUE, 0, 65535, 2, 0);
        if (pParamVal->pParamInfo->nBlobId == BLOB_TYPE_MANUALCTRL_RECT)
        {
            Make_UINT_SubBlobParam(p++, SUBIDX_RECTCTRL_STOP_SOC,  &pRectCtrl->nStopSOC, 655, 16, TRUE,  0, 100, 2);
        }
        else
        {
            pBlob2Create->nSubBlobTotal -= 1;
        }
        Make_ENUM_SubBlobParam(p++, SUBIDX_RECTCTRL_DCACADJ_MODE, &pRectCtrl->nAdjMode, 1961, 0, TRUE, 2, 2, 
            0, 1864, 1, 1865);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_RECTCTRL_ADJ_ACTVPWR, &pRectCtrl->fActv_Pwr, 159, 165, TRUE, -65535, 65535, 2, 0);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_RECTCTRL_ADJ_REACTVPWR, &pRectCtrl->fReactv_Pwr, 160, 16, TRUE, -101, 101, 2, 0);
     

        bCreateOK = TRUE;
        
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_ES_BACKEND_CFG)
    {
        A_BLOB_ES_BACKEND_CFG* pESBackend = (A_BLOB_ES_BACKEND_CFG*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_BACKENDCFG_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_BACKENDCFG_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_FLOAT_SubBlobParam(p++, SUBIDX_SysACInput_PwrLmt,      &pESBackend->fSysACInput_PwrLmt,        815,  165, TRUE, 0.0,   99999999.0,     2, 2);
        // LJH240705 AitiOverLoad HVMeter
        Make_FLOAT_SubBlobParam(p++, SUBIDX_SysACInput_PwrLmtHV,    &pESBackend->fSysACInput_PwrLmtHV,      2515, 165, TRUE, 0.0,   99999999.0,     2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Enter_UrgetDischrgPwr,  &pESBackend->fEnter_UrgetDischrgPwr,    1082, 165, TRUE, 0.0,   99999999.0,     2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Exit_UrgetDischrgPwr,   &pESBackend->fExit_UrgetDischrgPwr,     1083, 165, TRUE, 0.0,   99999999.0,     2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ACLoadPwrProtcPoint,    &pESBackend->fACLoadPwrProtcPoint,      1081, 165, TRUE, 0.0,   99999999.0,     2, 2);
        
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Min_ACLoad_StartDischg,  &pESBackend->fMin_ACLoadPwr_Start_Dischg,  360,  165, TRUE, 0.0,   99999999.0, 2, 2);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_Max_Output2AC_PwrLmt,   &pESBackend->fMax_Output2AC_PwrLmt,     837,  165, TRUE, 0.0,   99999999.0,     2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Min_Output2AC_PwrLmt,   &pESBackend->fMin_Output2AC_PwrLmt,     840,  165, TRUE, 0.0,   99999999.0,     2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_PreProt4RefluxAndOverLoad, &pESBackend->fPreProt4RefluxAndOverLoad,899,  165, TRUE, 0.0,   99999999.0,     2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Output2AC_PwrRate,      &pESBackend->fOutput2AC_PwrRate,        838,  0,   TRUE, 0.0,   1.0,            2, 2);
        // LJH250703 分开充放电保护/跟随比(充电)
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fPreProt_Chrg,          &pESBackend->fPreProt_Chrg,             2827,  165, TRUE, 0.0,   99999999.0,     2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_PowerRate_Chrg,         &pESBackend->PowerRate_Chrg,            2828,  0,   TRUE, 0.0,   1.0,            2, 2);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_Max_Q_LmtSett,          &pESBackend->fQmaxOutput2ACLmt,         1971, 166, TRUE, 0.0,   1000,           2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Pfactory_SettDemand,    &pESBackend->fPfactory_SettDemand,      168,  165, TRUE, 0.0,   99999999.0,     2, 2);
        // LJH240911 AvgPwrFirst
        Make_ENUM_SubBlobParam(p++, SUBIDX_PowCtrlMode,             &pESBackend->emPowCtrlMode,             2516, 0, TRUE, 2, 2, 0, 2615, 1, 2616);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_PwrLmtMin_AvgPwrMode,   &pESBackend->fPwrLmtMin_AvgPwrMode,     2517, 165, TRUE, 0.0,   99999999.0,     2, 2);
        // LJH241007 AntiPhase
        Make_FLOAT_SubBlobParam(p++, SUBIDX_PreProt4Reflux_Phase,   &pESBackend->fPreProt4Reflux_Phase,     2652, 165, TRUE, -1000.0, 99999999.0,   2, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_MaxChrgCurLmt_Phase,    &pESBackend->fMaxChrgCurLmt_Phase,      2653, 162, TRUE, 0.0, 99999999.0,       2, 2);

        // Jeffery241204 Dynamic Grid Input Limit
        Make_ENUM_SubBlobParam(p++, SUBIDX_DynamicGridInputLimit_Mode, &pESBackend->emDynamicGridInputLimit_Mode, 2601, 0, TRUE, 2, 3, 0, 324, 1, 2603, 2, 2604);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Fixed_GridInput_Limit,     &pESBackend->fFixed_GridInput_Limit,       2602, 165, TRUE, 0.0, 99999999.0, 2, 2);

        // Jeffery250305 PCS Hot Stand-by Mode
        Make_ENUM_SubBlobParam(p++, SUBIDX_PCS_HotStandby_Mode,      &pESBackend->emPCS_HotStandby_Mode, 2398, 0, TRUE, 2, 2, 0, 324, 1, 323);
        Make_ENUM_SubBlobParam(p++, SUBIDX_backend_reserved,         &pESBackend->backend_reserved,      2112, 0, TRUE, 2, 2, 0, 2112, 1, 2112);

        Make_ENUM_SubBlobParam(p++, SUBIDX_QuickResponseChannel,     &pESBackend->bEnableQuickResponseChannel,      4031, 0, TRUE, 2, 2, 0, 324, 1, 323);
        // Make_UINT_SubBlobParam(p++,  SUBIDX_FilterPointNum,          &pESBackend->nFilterPointNum,                  4032,  0, TRUE, 0, 30, 2);

        Make_ENUM_SubBlobParam(p++, SUBIDX_isACDC_Grp1_Exist,        &pESBackend->isACDC_Grp_Exist[0], 1126, 0, TRUE, 2, 2, 0, 780, 1, 779);
        Make_ENUM_SubBlobParam(p++, SUBIDX_isACDC_Grp2_Exist,        &pESBackend->isACDC_Grp_Exist[1], 1127, 0, TRUE, 2, 2, 0, 780, 1, 779);
        Make_ENUM_SubBlobParam(p++, SUBIDX_isACDC_Grp3_Exist,        &pESBackend->isACDC_Grp_Exist[2], 1128, 0, TRUE, 2, 2, 0, 780, 1, 779);
        Make_ENUM_SubBlobParam(p++, SUBIDX_isACDC_Grp4_Exist,        &pESBackend->isACDC_Grp_Exist[3], 1129, 0, TRUE, 2, 2, 0, 780, 1, 779);
        Make_ENUM_SubBlobParam(p++, SUBIDX_isMPPT_Grp1_Exist,        &pESBackend->isMPPT_Grp_Exist[0], 1130, 0, TRUE, 2, 2, 0, 780, 1, 779);
        Make_ENUM_SubBlobParam(p++, SUBIDX_isMPPT_Grp2_Exist,        &pESBackend->isMPPT_Grp_Exist[1], 1131, 0, TRUE, 2, 2, 0, 780, 1, 779);
        Make_ENUM_SubBlobParam(p++, SUBIDX_isMPPT_Grp3_Exist,        &pESBackend->isMPPT_Grp_Exist[2], 1132, 0, TRUE, 2, 2, 0, 780, 1, 779);
        Make_ENUM_SubBlobParam(p++, SUBIDX_isMPPT_Grp4_Exist,        &pESBackend->isMPPT_Grp_Exist[3], 1133, 0, TRUE, 2, 2, 0, 780, 1, 779);

        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_101_Interval,      &pESBackend->nCmd_101_Interval,         1411,  163, TRUE, 0,     60,      2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_102_Interval,      &pESBackend->nCmd_102_Interval,         1412,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_103_Interval,      &pESBackend->nCmd_103_Interval,         1413,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_104_Interval,      &pESBackend->nCmd_104_Interval,         1414,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_105_Interval,      &pESBackend->nCmd_105_Interval,         1415,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_155_Interval,      &pESBackend->nCmd_155_Interval,         7052,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_107_Interval,      &pESBackend->nCmd_107_Interval,         1417,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_157_Interval,      &pESBackend->nCmd_157_Interval,         7053,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_106_Interval,      &pESBackend->nCmd_106_Interval,         1416,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_108_Interval,      &pESBackend->nCmd_108_Interval,         1418,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_109_Interval,      &pESBackend->nCmd_109_Interval,         1419,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_110_Interval,      &pESBackend->nCmd_110_Interval,         1420,  163, TRUE, 0,     6000,    2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_114_Interval,      &pESBackend->nCmd_114_Interval,         6989,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_115_Interval,      &pESBackend->nCmd_115_Interval,         6990,  163, TRUE, 0,     600,     2);   
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_112_Interval,      &pESBackend->nCmd_112_Interval,         1421,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_200_Interval,      &pESBackend->nCmd_200_Interval,         7059,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_202_Interval,      &pESBackend->nCmd_202_Interval,         1422,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_204_Interval,      &pESBackend->nCmd_204_Interval,         1424,  163, TRUE, 0,     600,     2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_nCmd_206_Interval,      &pESBackend->nCmd_206_Interval,         1423,  163, TRUE, 0,     600,     2);
 
        bCreateOK = TRUE;

        TRACEX("Info:A_BLOB_ES_BACKEND_CFG Create Success!\n");
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_GRP_PM_DATAINFO)
     {
         TRACE("Info:A_BLOB_RECT_GRP_DATA Create Success!\n");
 
         A_BLOB_RECT_GRP_DATA* pPMCtrl = (A_BLOB_RECT_GRP_DATA*)pBlobVal_Raw;
         pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_PCSCTRL2CCU_MAXID);
         pBlob2Create->nSubBlobTotal = SUBIDX_PCSCTRL2CCU_MAXID;
         WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;
         int nSubParam = 0;
 
         //PCS Group#1
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP1_RatePwr,   &pPMCtrl->fRatedPower[0],       123,  591, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP1_RateCurt,  &pPMCtrl->fRatedCurr[0],        122,  162, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP1_RTMaxCurt, &pPMCtrl->fRealTime_MaxCurr[0], 466,  162, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_ENUM_SubBlobParam(p++,  SUBIDX_PCSCTRL2CCU_GRP1_WorkMode,  &pPMCtrl->emWorkMode[0],         465,    0, FALSE, 2, 2, 0, 709, 1, 710);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP1_CtrlVolt,  &pPMCtrl->fCtrlVolt[0],         572,  161, FALSE, 0.0,      999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP1_CtrlCurt,  &pPMCtrl->fCtrlCurr[0],         573,  162, FALSE, -9999.9,  999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP1_RTVolt,    &pPMCtrl->fRealTimeVolt[0],     453,  161, FALSE, 0.0,      999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP1_RTCurt,    &pPMCtrl->fRealTimeCurr[0],     454,  162, FALSE, -9999.9,  999999.0, 2, 2);
         nSubParam++;
 
         Make_INT_SubBlobParam(p++,   SUBIDX_PCSCTRL2CCU_GRP1_OnlineNum, &pPMCtrl->nOnlineRectNum[0],    316,    0, FALSE, 0, 100, 2);   
         nSubParam++;
 
         //PCS Group#2
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP2_RatePwr,   &pPMCtrl->fRatedPower[1],       123,  592, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP2_RateCurt,  &pPMCtrl->fRatedCurr[1],        122,  162, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP2_RTMaxCurt, &pPMCtrl->fRealTime_MaxCurr[1], 466,  162, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_ENUM_SubBlobParam(p++,  SUBIDX_PCSCTRL2CCU_GRP2_WorkMode,  &pPMCtrl->emWorkMode[1],         465,    0, FALSE, 2, 2, 0, 709, 1, 710);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP2_CtrlVolt,  &pPMCtrl->fCtrlVolt[1],         572,  161, FALSE, 0.0,      999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP2_CtrlCurt,  &pPMCtrl->fCtrlCurr[1],         573,  162, FALSE, -9999.9,  999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP2_RTVolt,    &pPMCtrl->fRealTimeVolt[1],     453,  161, FALSE, 0.0,      999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP2_RTCurt,    &pPMCtrl->fRealTimeCurr[1],     454,  162, FALSE, -9999.9,  999999.0, 2, 2);
         nSubParam++;
 
         Make_INT_SubBlobParam(p++,   SUBIDX_PCSCTRL2CCU_GRP2_OnlineNum, &pPMCtrl->nOnlineRectNum[1],    316,    0, FALSE, 0, 100, 2);   
         nSubParam++;
 
         //PCS Group#3
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP3_RatePwr,   &pPMCtrl->fRatedPower[2],       315,  593, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP3_RateCurt,  &pPMCtrl->fRatedCurr[2],        2805,  162, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP3_RTMaxCurt, &pPMCtrl->fRealTime_MaxCurr[2], 454,  162, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_ENUM_SubBlobParam(p++,  SUBIDX_PCSCTRL2CCU_GRP3_WorkMode,  &pPMCtrl->emWorkMode[2],         121,    0, FALSE, 2, 2, 0, 709, 1, 710);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP3_CtrlVolt,  &pPMCtrl->fCtrlVolt[2],         1445,  161, FALSE, 0.0,      999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP3_CtrlCurt,  &pPMCtrl->fCtrlCurr[2],         109,  162, FALSE, -9999.9,  999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP3_RTVolt,    &pPMCtrl->fRealTimeVolt[2],     1701,  161, FALSE, 0.0,      999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP3_RTCurt,    &pPMCtrl->fRealTimeCurr[2],     1702,  162, FALSE, -9999.9,  999999.0, 2, 2);
         nSubParam++;
 
         Make_INT_SubBlobParam(p++,   SUBIDX_PCSCTRL2CCU_GRP3_OnlineNum, &pPMCtrl->nOnlineRectNum[2],    2385,    0, FALSE, 0, 100, 2);   
         nSubParam++;
         
         //PCS Group#4
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP4_RatePwr,   &pPMCtrl->fRatedPower[3],       784,  594, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP4_RateCurt,  &pPMCtrl->fRatedCurr[3],        785,  162, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP4_RTMaxCurt, &pPMCtrl->fRealTime_MaxCurr[3], 786,  162, FALSE, 0.0,   99999999.0, 2, 2);
         nSubParam++;
 
         Make_ENUM_SubBlobParam(p++,  SUBIDX_PCSCTRL2CCU_GRP4_WorkMode,  &pPMCtrl->emWorkMode[3],         783,    0, FALSE, 2, 2, 0, 709, 1, 710);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP4_CtrlVolt,  &pPMCtrl->fCtrlVolt[3],         791,  161, FALSE, 0.0,      999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP4_CtrlCurt,  &pPMCtrl->fCtrlCurr[3],         2381,  162, FALSE, -9999.9,  999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP4_RTVolt,    &pPMCtrl->fRealTimeVolt[3],     2382,  161, FALSE, 0.0,      999999.0, 2, 2);
         nSubParam++;
 
         Make_FLOAT_SubBlobParam(p++, SUBIDX_PCSCTRL2CCU_GRP4_RTCurt,    &pPMCtrl->fRealTimeCurr[3],     2383,  162, FALSE, -9999.9,  999999.0, 2, 2);
         nSubParam++;
 
         Make_INT_SubBlobParam(p++,   SUBIDX_PCSCTRL2CCU_GRP4_OnlineNum, &pPMCtrl->nOnlineRectNum[3],    2384,    0, FALSE, 0, 100, 2);   
         nSubParam++;
 
         pBlob2Create->nSubBlobTotal = nSubParam;
 
         bCreateOK = TRUE;
     }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_SINGLE_STBATT_RUNINFO)
    {
        TRACE("Info:A_BLOB_SINGLE_STBATT_RUNINFO Create Success!\n");

        A_BLOB_SINGLE_STBATT_RUNINFO* pSgBatt = (A_BLOB_SINGLE_STBATT_RUNINFO*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_STBATT_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_STBATT_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;
 
        Make_UINT_SubBlobParam(p++, SUBIDX_BATT_RUNSTAT,            &pSgBatt->emBattStatus_Run,                 533, 0, FALSE, 0, 100, 2);   
        Make_UINT_SubBlobParam(p++, SUBIDX_BATT_SOC,                &pSgBatt->nSOC ,                            115, 16, FALSE,  0, 100, 2);
        Make_UINT_SubBlobParam(p++, SUBIDX_BATT_SOH,                &pSgBatt->nSOH ,                            26,  16, FALSE,  0, 255, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_PACKVOLT,          &pSgBatt->fPackTotalVolt ,                  496, 161, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_PACKCURR,          &pSgBatt->fPackTotalCurr ,                  497, 162, FALSE, -1000, 1000, 2, 1);  

        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_LMTCHRGVOLT,       &pSgBatt->fAllow_LmtChargeVolt ,            521, 161, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_LMTDISCHRGVOLT,    &pSgBatt->fAllow_LmtDisChargeVolt ,         522, 161, FALSE, 0, 1000, 2, 1);  
        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_MAXCHRGCURR,       &pSgBatt->fAllow_MaxChargeCurr ,            523, 162, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_MAXDISCHRGCURR,    &pSgBatt->fAllow_MaxDisChargeCurr ,         524, 162, FALSE, 0, 1000, 2, 1);  

        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_MAXCELLVOLT,       &pSgBatt->fMax_CellVolt ,                  525, 161, FALSE, 0, 1000, 2, 3);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_MINCELLVOLT,       &pSgBatt->fMin_CellVolt ,                  526, 161, FALSE, 0, 1000, 2, 3);  
        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_MAXCELLTEMP,       &pSgBatt->fMax_CellTemp ,                   529, 127, FALSE, -127, 65535, 2, 3);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_BATT_MINCELLTEMP,       &pSgBatt->fMin_CellTemp ,                   530, 127, FALSE, -127, 65535, 2, 3);  

        Make_USHORT_SubBlobParam(p++, SUBIDX_BATT_UCELL_MAX_CSC,      &pSgBatt->uiBAT_Ucell_Max_CSC,              668, 0, FALSE, 0, 65535, 1);
        Make_USHORT_SubBlobParam(p++, SUBIDX_BATT_UCELL_MAX_CELL,     &pSgBatt->uiBAT_Ucell_Max_CELL,             669, 0, FALSE, 0, 65535, 1);
        Make_USHORT_SubBlobParam(p++, SUBIDX_BATT_UCELL_MIN_CSC,      &pSgBatt->uiBAT_Ucell_Min_CSC,              670, 0, FALSE, 0, 65535, 1);
        Make_USHORT_SubBlobParam(p++, SUBIDX_BATT_UCELL_MIN_CELL,     &pSgBatt->uiBAT_Ucell_Min_CELL,             671, 0, FALSE, 0, 65535, 1);
        
        Make_USHORT_SubBlobParam(p++, SUBIDX_STBATT_CYCLETIMES,      &pSgBatt->nLifeCycleTimes,                  536, 0, FALSE, 0, 65535, 1);
        Make_UINT_SubBlobParam(p++, SUBIDX_STBATT_CHRGFIRBIDDEN,     &pSgBatt->bIsBatt_ChargeForbidden,          537, 0, FALSE, 0, 65535, 1);
        Make_UINT_SubBlobParam(p++, SUBIDX_STBATT_DISCHRGFORBIDDEN,  &pSgBatt->bIsBatt_DisChargeForbidden,       538, 0, FALSE, 0, 65535, 1);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_STBATT_VERSION,         &pSgBatt->fVersion ,                        125, 0, FALSE, 0, 1000, 2, 3);  
        Make_ENUM_SubBlobParam(p++, SUBIDX_STBATT_PERMISSION,       &pSgBatt->uiPermissionLevel,                1029, 0, FALSE, 10, 4, 0, 2050, 1, 2051, 2, 2052, 3, 2053);
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_EMS_CONFIG_PLAN)
    {
        int nSubParam = 0;
        A_BLOB_EMS_CONFIG_PLAN* pEmsCfg = (A_BLOB_EMS_CONFIG_PLAN*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, EMS_SEC_NUM*EMS_MAX_SECTORS + 2);
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        //printf("Info : CreateA_BLOB_EMS_CONFIG_PLAN nTotal_EmsLmt_Sectors=%d!\n", pEmsCfg->nTotal_EmsLmt_Sectors);
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_VER, &pEmsCfg->nEMSMgmt_Ver, 1061, 0, TRUE,  0, 99999999, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_LMTSECTORS, &pEmsCfg->nTotal_EmsLmt_Sectors, 
            649, 0, TRUE,  0, 96, 2);
        nSubParam++;
        
        //Sector1
        int n = 0;
        for(n = 0; n < EMS_MAX_SECTORS; ++n)
        {
            Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC_TMSTART + EMS_SEC_NUM*n, &pEmsCfg->emsLmtSectors[n].tmStart, 651,  TRUE, 2);
            nSubParam++;
            Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC_INTERVAL + EMS_SEC_NUM*n, &pEmsCfg->emsLmtSectors[n].nInterval, 652, 163, TRUE,  0, 86400, 2);
            nSubParam++;
            //Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC1_FEERATE, &pEmsCfg->emsLmtSectors[0].emFeeRate, 276, 0, TRUE, 2, 4, 
            //    0, 866, 1, 278, 2, 279, 3, 280);
            //nSubParam++;
            Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC_PRIMODE + EMS_SEC_NUM*n, &pEmsCfg->emsLmtSectors[n].emBatt_WorkMode, 806, 0, TRUE, 2, 6, 
                //              2:只放电 3:只充电  4:待机   5:关机
                0, 807, 1, 808, 2, 2831, 3, 2832, 4, 6710, 5, 6702);
            nSubParam++;
            Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC_STOPSOC + EMS_SEC_NUM*n, &pEmsCfg->emsLmtSectors[n].nStopSOC, 655, 16, TRUE,  0, 100, 2);
            nSubParam++;
            Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC_BATTGRPPWRLMT + EMS_SEC_NUM*n, &pEmsCfg->emsLmtSectors[n].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
            nSubParam++;
            Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC_ACTVPWRLMT + EMS_SEC_NUM*n, &pEmsCfg->emsLmtSectors[n].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
            nSubParam++;
            Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC_REACTVPWRLMT + EMS_SEC_NUM*n, &pEmsCfg->emsLmtSectors[n].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
            nSubParam++;
            Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC_GRIDINPUTPWRLMT + EMS_SEC_NUM*n, &pEmsCfg->emsLmtSectors_ext[n].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
            nSubParam++;            
        }
/*         Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC1_TMSTART, &pEmsCfg->emsLmtSectors[0].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC1_INTERVAL, &pEmsCfg->emsLmtSectors[0].nInterval, 652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
        //Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC1_FEERATE, &pEmsCfg->emsLmtSectors[0].emFeeRate, 276, 0, TRUE, 2, 4, 
        //    0, 866, 1, 278, 2, 279, 3, 280);
        //nSubParam++;
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC1_PRIMODE, &pEmsCfg->emsLmtSectors[0].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC1_STOPSOC, &pEmsCfg->emsLmtSectors[0].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC1_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[0].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC1_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[0].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC1_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[0].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC1_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[0].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //Sector2
         Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC2_TMSTART, &pEmsCfg->emsLmtSectors[1].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC2_INTERVAL, &pEmsCfg->emsLmtSectors[1].nInterval, 652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC2_PRIMODE, &pEmsCfg->emsLmtSectors[1].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC2_STOPSOC, &pEmsCfg->emsLmtSectors[1].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC2_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[1].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC2_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[1].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC2_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[1].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC2_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[1].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //Sector3
        Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC3_TMSTART, &pEmsCfg->emsLmtSectors[2].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC3_INTERVAL, &pEmsCfg->emsLmtSectors[2].nInterval, 652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC3_PRIMODE, &pEmsCfg->emsLmtSectors[2].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC3_STOPSOC, &pEmsCfg->emsLmtSectors[2].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC3_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[2].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC3_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[2].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC3_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[2].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC3_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[2].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //Sector4
        Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC4_TMSTART, &pEmsCfg->emsLmtSectors[3].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC4_INTERVAL, &pEmsCfg->emsLmtSectors[3].nInterval, 652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC4_PRIMODE, &pEmsCfg->emsLmtSectors[3].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC4_STOPSOC, &pEmsCfg->emsLmtSectors[3].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC4_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[3].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC4_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[3].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC4_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[3].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC4_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[3].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //Sector5
        Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC5_TMSTART, &pEmsCfg->emsLmtSectors[4].tmStart, 
            651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC5_INTERVAL, &pEmsCfg->emsLmtSectors[4].nInterval, 
            652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC5_PRIMODE, &pEmsCfg->emsLmtSectors[4].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC5_STOPSOC, &pEmsCfg->emsLmtSectors[4].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC5_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[4].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC5_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[4].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC5_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[4].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC5_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[4].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //Sector6
        Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC6_TMSTART, &pEmsCfg->emsLmtSectors[5].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC6_INTERVAL, &pEmsCfg->emsLmtSectors[5].nInterval, 652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC6_PRIMODE, &pEmsCfg->emsLmtSectors[5].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC6_STOPSOC, &pEmsCfg->emsLmtSectors[5].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC6_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[5].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC6_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[5].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC6_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[5].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC6_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[5].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //Sector7
        Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC7_TMSTART, &pEmsCfg->emsLmtSectors[6].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC7_INTERVAL, &pEmsCfg->emsLmtSectors[6].nInterval, 652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC7_PRIMODE, &pEmsCfg->emsLmtSectors[6].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC7_STOPSOC, &pEmsCfg->emsLmtSectors[6].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC7_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[6].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC7_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[6].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC7_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[6].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC7_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[6].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //Sector8
        Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC8_TMSTART, &pEmsCfg->emsLmtSectors[7].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC8_INTERVAL, &pEmsCfg->emsLmtSectors[7].nInterval, 652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC8_PRIMODE, &pEmsCfg->emsLmtSectors[7].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC8_STOPSOC, &pEmsCfg->emsLmtSectors[7].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC8_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[7].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC8_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[7].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC8_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[7].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC8_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[7].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //Sector9
        Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SEC9_TMSTART, &pEmsCfg->emsLmtSectors[8].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SEC9_INTERVAL, &pEmsCfg->emsLmtSectors[8].nInterval, 
            652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
        //Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC9_FEERATE, &pEmsCfg->emsLmtSectors[8].emFeeRate, 276, 0, TRUE, 2, 4, 
        //    0, 866, 1, 278, 2, 279, 3, 280);
        //nSubParam++;
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SEC9_PRIMODE, &pEmsCfg->emsLmtSectors[8].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC9_STOPSOC, &pEmsCfg->emsLmtSectors[8].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC9_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[8].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC9_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[8].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC9_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[8].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SEC9_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[8].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //sector 10
        Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SECA_TMSTART, &pEmsCfg->emsLmtSectors[9].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SECA_INTERVAL, &pEmsCfg->emsLmtSectors[9].nInterval, 
            652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
 
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SECA_PRIMODE, &pEmsCfg->emsLmtSectors[9].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECA_STOPSOC, &pEmsCfg->emsLmtSectors[9].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECA_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[9].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECA_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[9].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECA_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[9].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECA_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[9].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        //sector 11
        Make_DATETIME_SubBlobParam(p++, SUBIDX_EMS_SECB_TMSTART, &pEmsCfg->emsLmtSectors[10].tmStart, 651,  TRUE, 2);
        nSubParam++;
        Make_INT_SubBlobParam(p++, SUBIDX_EMS_SECB_INTERVAL, &pEmsCfg->emsLmtSectors[10].nInterval, 
            652, 163, TRUE,  0, 86400, 2);
        nSubParam++;
 
        Make_ENUM_SubBlobParam(p++, SUBIDX_EMS_SECB_PRIMODE, &pEmsCfg->emsLmtSectors[10].emBatt_WorkMode, 806, 0, TRUE, 2, 2, 
            0, 807, 1, 808);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECB_STOPSOC, &pEmsCfg->emsLmtSectors[10].nStopSOC, 655, 16, TRUE,  0, 100, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECB_BATTGRPPWRLMT, &pEmsCfg->emsLmtSectors[10].nBattGrpPwrLmt, 1109, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECB_ACTVPWRLMT, &pEmsCfg->emsLmtSectors[10].nActvPwrLmt, 658, 165, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECB_REACTVPWRLMT, &pEmsCfg->emsLmtSectors[10].nReActvPwrLmt, 1971, 166, TRUE,  0, 999999, 2);
        nSubParam++;
        Make_UINT_SubBlobParam(p++, SUBIDX_EMS_SECB_GRIDINPUTPWRLMT, &pEmsCfg->emsLmtSectors_ext[10].nGridInputPwrLmt, 2605, 165, TRUE,  0, 999999, 2);
        nSubParam++;  */
        pBlob2Create->nSubBlobTotal = nSubParam;
        //printf("pBlob2Create->nSubBlobTotal=%d\n", pBlob2Create->nSubBlobTotal);
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_BRANCH_GRP_RECT_DATA)
    {
        A_BLOB_BRANCH_GRP_RECTDATA* pGrpRect = (A_BLOB_BRANCH_GRP_RECTDATA*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_PM_GRP_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_PM_GRP_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_UINT_SubBlobParam(p++, SUBIDX_OnlineACDCNum,                   &pGrpRect->nOnlineACDCNum,                705, 0, FALSE, 0, 100, 2);
        Make_UINT_SubBlobParam(p++, SUBIDX_OnlineDCDCNum,                   &pGrpRect->nOnlineDCDCNum,                706, 0, FALSE, 0, 100, 2);
        Make_UINT_SubBlobParam(p++, SUBIDX_OnlineMPPTNum,                   &pGrpRect->nOnlineMPPTNum,                707, 0, FALSE, 0, 100, 2);
     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ACDC_RealTime_Volt,             &pGrpRect->fACDC_RealTime_Volt ,          1380, 161, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ACDC_RealTime_Curr,             &pGrpRect->fACDC_RealTime_Curr ,          1381, 162, FALSE, -1000, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ACDC_Max_OutputPwr_Ability,     &pGrpRect->fACDC_Max_Output_Pwr_Ability , 470,  165, FALSE, 0, 1000, 2, 1);     
        Make_UINT_SubBlobParam(p++,  SUBIDX_ACDC_WorkStatu,                 &pGrpRect->emACDC_WorkStatu ,             1382,  0,  FALSE, 0, 100, 2);     

        Make_FLOAT_SubBlobParam(p++, SUBIDX_MPPT_RealTime_Volt,             &pGrpRect->fMPPT_RealTime_Volt ,           1383, 161, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_MPPT_RealTime_Curr,             &pGrpRect->fMPPT_RealTime_Curr ,           1384, 162, FALSE, -1000, 1000, 2, 1);     
        Make_UINT_SubBlobParam(p++,  SUBIDX_MPPT_WorkStatu,                 &pGrpRect->emMPPT_WorkStatu ,              1385,  0,  FALSE, 0, 100, 2);     

        Make_FLOAT_SubBlobParam(p++, SUBIDX_DCDC_RealTime_Volt,             &pGrpRect->fDCDC_RealTime_Volt ,           1386, 161, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_DCDC_RealTime_Curr,             &pGrpRect->fDCDC_RealTime_Curr ,           1387, 162, FALSE, -1000, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_DCDC_Max_OutputPwr_Ability,     &pGrpRect->fDCDC_Max_Output_Pwr_Ability ,  123,  165, FALSE, 0, 1000, 2, 1);     
        Make_UINT_SubBlobParam(p++,  SUBIDX_DCDC_WorkStatu,                 &pGrpRect->emDCDC_WorkStatu ,              1388,  0,  FALSE, 0, 100, 2);     

        Make_FLOAT_SubBlobParam(p++, SUBIDX_Batt_RealTime_Volt,             &pGrpRect->fBatt_RealTime_Volt ,            569,  161, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Batt_RealTime_Curr,             &pGrpRect->fBatt_RealTime_Curr ,            570,  162, FALSE, -1000, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Batt_Max_DisChrgPwr,            &pGrpRect->fBatt_Max_DisChrg_Pwr ,          1389, 165, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Batt_Max_ChrgPwr_Req,           &pGrpRect->fBatt_Max_Chrg_Pwr_Req ,         1390, 165, FALSE, 0, 1000, 2, 1);     
        Make_UINT_SubBlobParam(p++,  SUBIDX_Batt_SOC,                       &pGrpRect->nSOC ,                           571,  16,  FALSE, 0, 100, 2);     

        Make_FLOAT_SubBlobParam(p++, SUBIDX_MPPT_CTRL_Q,                    &pGrpRect->fCtrl_RealTime_Q ,               1971,  166, FALSE, -1000, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ACDCOutRelayVolt,               &pGrpRect->fACDCOutRelayVolt ,               3,    161, FALSE, 0, 1000, 2, 1);     

        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_BRANCH_NODE_CALC_DATA)
    {
        A_BLOB_BRANCH_NODE_CALC_DATA* pNodeCalc = (A_BLOB_BRANCH_NODE_CALC_DATA*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_NODECALC_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_NODECALC_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        //printf("Info : Create A_BLOB_BRANCH_NODE_CALC_DATA!\n");
     
        Make_FLOAT_SubBlobParam(p++,   SUBIDX_BATT_DISTRI4ACDC_DISCHRGPWR,    &pNodeCalc->fBatt_Distri_4_ACDC_Pwr ,       789, 165, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++,   SUBIDX_ACDC_MAX_POWER,                 &pNodeCalc->fACDC_Max_Ability_Pwr ,         123, 165, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++,   SUBIDX_ACDC_DISTRI4BATT_CHRGPWR,       &pNodeCalc->fACDC_Distri_4_BattChrg_Pwr ,   792, 165, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++,   SUBIDX_REMAIN_BATT_DISCHRGPWR,         &pNodeCalc->fRemain_Batt_DisChrg_Pwr ,      794, 165, FALSE, 0, 1000, 2, 1);     
 
        Make_FLOAT_SubBlobParam(p++,   SUBIDX_REMAIN_CHRG_PWR_REQ,            &pNodeCalc->fRemain_Batt_Chrg_Pwr_Req ,     796, 165, FALSE, 0, 1000, 2, 1);     
        Make_UINT_SubBlobParam(p++,    SUBIDX_BATT_PRI_WORKMODE,              &pNodeCalc->emBatt_WorkMode,                806, 0,   FALSE,  0, 96,  2);
        Make_FLOAT_SubBlobParam(p++,   SUBIDX_AVG_CHRGLMTPWR,                 &pNodeCalc->fAvg_ChrgLmtPower ,             792, 165, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++,   SUBIDX_AVG_DISCHRGINGLMTPWR,           &pNodeCalc->fAvg_DisChrgingLmtPower ,       789, 165, FALSE, 0, 1000, 2, 1);     
 
        //printf("pBlob2Create->nSubBlobTotal=%d\n", pBlob2Create->nSubBlobTotal);
        bCreateOK = TRUE;
     }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_RECTGRP_AUTOCTRLCMD)
    {// LJH241111 PcsSCADA
        A_BLOB_GRP_RECT_AUTOCTRL_CMD* pRectCtrl = (A_BLOB_GRP_RECT_AUTOCTRL_CMD*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_RECT_AUTOCTRL_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_RECT_AUTOCTRL_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_ENUM_SubBlobParam(p++, SUBIDX_RECT_AUTOCTRL_CTRL_TYPE, &pRectCtrl->emCtrl_Type, 465, 0, TRUE, 2, 5,
            //0, 222, 1, 717, 2, 718, 3, 719, 4, 720);
            // 0:stop 1:电池充电_启动 2:电池充电_调节 3:电池放电_启动 4:电池放电_调节
            0, 222, 1, 717, 2, 718, 3, 719, 4, 720);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_RECT_AUTOCTRL_ADJ_VOLT, &pRectCtrl->fGrpVoltLmt, 453, 161, TRUE, 0, 65535, 2, 0);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_RECT_AUTOCTRL_ADJ_CURR, &pRectCtrl->fGrpCurrLmt, 454, 162, TRUE, 0, 65535, 2, 0);
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_BATTCAB_IMEU2_V20)
    {
        A_BLOB_BATTCAB_IMEU2_V20* pIMEU2 = (A_BLOB_BATTCAB_IMEU2_V20*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_IMEU2V20_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_IMEU2V20_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_INT_SubBlobParam(p++,   SUBIDX_IMEU2BMSCommState , &pIMEU2->emIMEU2BMSCommState,    1370, 0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_DI1_Fire ,          &pIMEU2->emDI1_Fire,             604,    0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_DI2_Smoke,          &pIMEU2->emDI2_Smoke,            603, 0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_DI3_DoorOpen ,      &pIMEU2->emDI3_DoorOpen,         203, 0,   FALSE,  0, 100, 2);

        Make_INT_SubBlobParam(p++,   SUBIDX_DI4_FlammableGas,           &pIMEU2->emDI4_FlammableGas,        1320, 0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_DI6_ShutterClosedOver,      &pIMEU2->emDI6_ShutterClosedOver,   1368,    0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_ShutterOpenedOver,          &pIMEU2->emShutterOpenedOver,       1369, 0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_Flood,                      &pIMEU2->emFlood,                   602, 0,   FALSE,  0, 100, 2);
 
        Make_INT_SubBlobParam(p++,   SUBIDX_ExhaustFanSpeed,          &pIMEU2->nExhaustFanSpeed,        1312, 0,   FALSE,  0, 999999, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_OnOrOffd,                 &pIMEU2->emOnOrOffd,              1313, 0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_StartRefrigMode,          &pIMEU2->emStartRefrigMode,       1314, 0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_StartSupplyAirMode,       &pIMEU2->emStartSupplyAirMode,    1315, 0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_StartRefrigMode,          &pIMEU2->emStartStandbyMode,      1316, 0,   FALSE,  0, 100, 2);
        Make_INT_SubBlobParam(p++,   SUBIDX_StartSupplyAirMode,       &pIMEU2->emStartHeatMode,         1317, 0,   FALSE,  0, 100, 2);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_CoolSetPoint,           &pIMEU2->fCoolSetPoint,         1321, 127, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_CoolDeviation,          &pIMEU2->fCoolDeviation,        1322, 127, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_HighTempAlmValue,       &pIMEU2->fHighTempAlmValue,     1323, 127, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_LowTempAlmValue,        &pIMEU2->fLowTempAlmValue,      1324, 127, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_HeatSetPoint,           &pIMEU2->fHeatSetPoint,         1325, 127, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_HeatDeviation,          &pIMEU2->fHeatDeviation,        1326, 127, FALSE, 0, 1000, 2, 1);     
    
        Make_FLOAT_SubBlobParam(p++, SUBIDX_RealTimeCabTemp,        &pIMEU2->fRealTimeCabTemp,     1343, 127, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_RealTimeCabHumi,        &pIMEU2->fRealTimeCabHumi,     1344, 16, FALSE, 0, 1000, 2, 1);     
    
        Make_FLOAT_SubBlobParam(p++, SUBIDX_DehumiStartTemp,        &pIMEU2->fDehumiStartTemp,     1327, 127, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_DehumiStopTemp,         &pIMEU2->fDehumiStopTemp,      1328, 127, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_DehumiStartHumi,        &pIMEU2->fDehumiStartHumi,     1329, 16, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_DehumiStopHumi,         &pIMEU2->fDehumiStopHumi,      1330, 16, FALSE, 0, 1000, 2, 1);     
    
        Make_FLOAT_SubBlobParam(p++, SUBIDX_CombGasPotency_Grp1,    &pIMEU2->fCombGasPotency[0],     1309, 16, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_CombGasPotency_Grp2,    &pIMEU2->fCombGasPotency[1],      1309, 16, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Humidity_Grp1,          &pIMEU2->fHumidity[0],            688, 16, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Temperature_Grp1,       &pIMEU2->fTemperature[0],         689, 127, FALSE, -127, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Humidity_Grp2,          &pIMEU2->fHumidity[1],            690, 16, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Temperature_Grp2,       &pIMEU2->fTemperature[1],         691, 127, FALSE, -127, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Humidity_Grp3,          &pIMEU2->fHumidity[2],            611, 16, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Temperature_Grp3,       &pIMEU2->fTemperature[2],         610, 127, FALSE, -127, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Humidity_Grp4,          &pIMEU2->fHumidity[3],            611, 16, FALSE, 0, 1000, 2, 1);     
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Temperature_Grp4,       &pIMEU2->fTemperature[3],         610, 127, FALSE, -127, 1000, 2, 1);     
        Make_STRING_SubBlobParam(p++,SUBIDX_CABIMEU2_VER,           &pIMEU2->szSoftwareVer,           125, FALSE, 0, 7);
 
         //printf("pBlob2Create->nSubBlobTotal=%d\n", pBlob2Create->nSubBlobTotal);
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_KEEPALIVEDINFO)
    {
        A_BLOB_KEEPALIVEDINFO* pKeepalivedCfg = (A_BLOB_KEEPALIVEDINFO*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_KEEPALIVED_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_KEEPALIVED_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_ENUM_SubBlobParam(p++,     SUBIDX_KEEPALIVED_ENABLE,                &pKeepalivedCfg->bEnable,              323, 0,   TRUE, 0, 2, 0, 324, 1, 323);
        Make_STRING_SubBlobParam(p++,   SUBIDX_KEEPALIVED_ROUTER_ID,             &pKeepalivedCfg->szRouter_Id,          2811, TRUE, 0, 64);
        Make_ENUM_SubBlobParam(p++,     SUBIDX_KEEPALIVED_STATE,                 &pKeepalivedCfg->bState,               2812, 0,   TRUE, 0, 2, 0, 588, 1, 2820);
        Make_ENUM_SubBlobParam(p++,     SUBIDX_KEEPALIVED_INTERFACE,             &pKeepalivedCfg->nInterface,           2813, 0,   TRUE, 0, 2, 0, 2821, 1, 2822);
        Make_UINT_SubBlobParam(p++,     SUBIDX_KEEPALIVED_VIRTUAL_ROUTER_ID,     &pKeepalivedCfg->nVirtual_Router_Id,   2814, 0,   TRUE,  0, 100,  0);
        Make_UINT_SubBlobParam(p++,     SUBIDX_KEEPALIVED_PRIORITY,              &pKeepalivedCfg->nPriority,            2815, 0,   TRUE,  0, 100,  0);
        Make_UINT_SubBlobParam(p++,     SUBIDX_KEEPALIVED_ADVERT_INT,            &pKeepalivedCfg->nAdvert_Int,          2816, 0,   TRUE,  0, 600,  0);
        Make_ENUM_SubBlobParam(p++,     SUBIDX_KEEPALIVED_AUTH_TYPE,             &pKeepalivedCfg->bAuth_Type,           2817, 0,   TRUE, 0, 2, 0, 2823, 1, 2824);
        Make_STRING_SubBlobParam(p++,   SUBIDX_KEEPALIVED_AUTH_PASS,             &pKeepalivedCfg->szAuth_Pass,          2818, TRUE, 0, 64);
        Make_STRING_SubBlobParam(p++,   SUBIDX_KEEPALIVED_VIRTUAL_IPADDRESS,     &pKeepalivedCfg->szVirtual_Ipaddress,  2819, TRUE, 0, 64);

        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_SEMS_CLIENTNFO_SUM)
    {
        A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = (A_BLOB_SEMS_CLIENTINFO_SUM*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_SEMS_CLIENTINFO_SUM_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_SEMS_CLIENTINFO_SUM_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;
        // REMS控制模式
        Make_ENUM_SubBlobParam(p++,  SUBIDX_CtrlMode_REMS,   &pClientSum->emEMSCtrlMode_REMS, 3992, 0, FALSE, 0, 2, 
            0, 3993, 1, 3994);
        Make_ENUM_SubBlobParam(p++, SUBIDX_ACDC_WorkStatu_REMS, &pClientSum->emACDC_WorkStatu_REMS, 1382, 0, FALSE, 0, 4, 
            0, 91, 1, 709, 2, 710, 3, 6710);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ActvPwrReq_REMS, &pClientSum->fActvPwrReq_REMS, 3990, 165, FALSE, -99999, 99999, 0, 1);
        // SEMS
        Make_BYTE_SubBlobParam(p++,  SUBIDX_Standby_BLL,    &pClientSum->bStandby_BLL, 3703,  0,  FALSE, 0, 100, 0);
        Make_BYTE_SubBlobParam(p++,  SUBIDX_WorkAsap_SEMS,   &pClientSum->byWorkAsap_SEMS, 3704,  0,  FALSE, 0, 100, 0);
        // AEMS
        Make_BYTE_SubBlobParam(p++,  SUBIDX_Standby_AEMS,    &pClientSum->bStandby_AEMS, 3702,  0,  FALSE, 0, 100, 0);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Pacload4DischgSum, &pClientSum->fPacload4DischgSum, 3924, 165, FALSE, -99999, 99999, 0, 1);
        
        // 系统状态
        // Make_ENUM_SubBlobParam(p++,  SUBIDX_CCUStateSum,       &pClientSum->emCCUStateSum, 129, 0, FALSE, 0, 3, 
        //     0, 34, 1, 199, 2, 198);
        Make_BYTE_SubBlobParam(p++,  SUBIDX_CCUStateSum,    &pClientSum->emCCUStateSum, 129,  0,  FALSE, 0, 100, 0);
        // Make_ENUM_SubBlobParam(p++,  SUBIDX_PMCtrlModeSum,     &pClientSum->emPMCtrlModeSum, 465, 0, FALSE, 0, 5, 
        //     0, 460, 1, 461, 2, 462, 3, 5053, 4, 5051);
        Make_BYTE_SubBlobParam(p++,  SUBIDX_PMCtrlModeSum, &pClientSum->emPMCtrlModeSum, 465,  0,  FALSE, 0, 100, 0);
        // Make_ENUM_SubBlobParam(p++,  SUBIDX_ACDC_WorkStatuSum, &pClientSum->emACDC_WorkStatuSum, 1382, 0, FALSE, 0, 4, 
        //     0, 213, 1, 709, 2, 710, 3, 6710);
        Make_BYTE_SubBlobParam(p++,  SUBIDX_ACDC_WorkStatuSum, &pClientSum->emACDC_WorkStatuSum, 1382,  0,  FALSE, 0, 100, 0);
        Make_BYTE_SubBlobParam(p++,  SUBIDX_Standby_SetSum,    &pClientSum->bStandby_SetSum, 2398,  0,  FALSE, 0, 100, 0);
        Make_BYTE_SubBlobParam(p++,  SUBIDX_Standby_SEMSSum,   &pClientSum->bStandby_BLLSum, 6710,  0,  FALSE, 0, 100, 0);
        Make_BYTE_SubBlobParam(p++,  SUBIDX_WorkAsapSum,       &pClientSum->byWorkAsapSum, 3705,  0,  FALSE, 0, 100, 0);

        Make_BYTE_SubBlobParam(p++,  SUBIDX_SOCMin,                    &pClientSum->nSOCMinSum,                 145,  16,  FALSE, 0, 100, 0);
        Make_BYTE_SubBlobParam(p++,  SUBIDX_SOCMax,                    &pClientSum->nSOCMaxSum,                 146,  16,  FALSE, 0, 100, 0);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_SOCSum,                    &pClientSum->fSOCSum,                    115,  16,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ActvPwr_PCSGrpSum,         &pClientSum->fActvPwr_PCSGrpSum,         2802, 165, FALSE, -99999, 99999, 0, 1);
        
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Max_Chrg_PwrAbilitySum0,    &pClientSum->fMax_Chrg_PwrAbilitySum0,    1390, 165, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Max_DisChrg_PwrAbilitySum0, &pClientSum->fMax_DisChrg_PwrAbilitySum0, 1389, 165, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Max_Chrg_PwrAbilitySum,    &pClientSum->fMax_Chrg_PwrAbilitySum,    2803, 165, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_Max_DisChrg_PwrAbilitySum, &pClientSum->fMax_DisChrg_PwrAbilitySum, 2804, 165, FALSE, -99999, 99999, 0, 1);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_ChrgedEnergySum,           &pClientSum->fChrgedEnergySum,           507,  14,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fRealTime_MaxCurrSum,      &pClientSum->fRealTime_MaxCurrSum,       122,  162, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ACDC_RealTime_VoltSum,     &pClientSum->fACDC_RealTime_VoltSum,     453,  161, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ACDC_RealTime_CurrSum,     &pClientSum->fACDC_RealTime_CurrSum,     454,  162, FALSE, -99999, 99999, 0, 1);

        Make_FLOAT_SubBlobParam(p++, SUBIDX_PgridMeterSum,     &pClientSum->f_PgridMeterSum,     3925, 165, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_PcabMeterSum,      &pClientSum->f_PcabMeterSum,      2801, 165, FALSE, -99999, 99999, 0, 1);
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_DEVINSTALLCONFIG)
    {
        A_BLOB_DEVINSTALLCONFIG* pDevInstallConfig = (A_BLOB_DEVINSTALLCONFIG*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_Dev_SUM_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_Dev_SUM_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_ENUM_SubBlobParam(p++, SUBIDX_AIR1,               &pDevInstallConfig->AIR[0],           5508, 0, TRUE, 0, 5, 
        0, 267, 1, 6097, 2, 1318, 3, 6345, 4, 6346); 
        Make_ENUM_SubBlobParam(p++, SUBIDX_AIR2,               &pDevInstallConfig->AIR[1],           5566, 0, TRUE, 0, 5, 
        0, 267, 1, 6097, 2, 1318, 3, 6345, 4, 6346); 
        Make_ENUM_SubBlobParam(p++, SUBIDX_PACK1,              &pDevInstallConfig->PACK[0],           6000, 0, TRUE, 0, 2, 
        0, 267, 1, 28); 
        Make_ENUM_SubBlobParam(p++, SUBIDX_PACK2,              &pDevInstallConfig->PACK[1],           6001, 0, TRUE, 0, 2, 
        0, 267, 1, 28); 
        Make_ENUM_SubBlobParam(p++, SUBIDX_Bat_EU1,            &pDevInstallConfig->Bat_IMEU[0],        599, 0, TRUE, 0, 3, 
        0, 267, 1, 5800, 2, 5670); 
        Make_ENUM_SubBlobParam(p++, SUBIDX_Bat_EU2,            &pDevInstallConfig->Bat_IMEU[1],         816, 0, TRUE, 0, 3, 
        0, 267, 1, 5800, 2, 5670); 
        Make_ENUM_SubBlobParam(p++, SUBIDX_Bat_EU3,            &pDevInstallConfig->Bat_IMEU[2],         581, 0, TRUE, 0, 3, 
        0, 267, 1, 5800, 2, 5670);  
        Make_ENUM_SubBlobParam(p++, SUBIDX_Bat_EU4,            &pDevInstallConfig->Bat_IMEU[3],          582, 0, TRUE, 0, 3, 
        0, 267, 1, 5800, 2, 5670); 
        Make_ENUM_SubBlobParam(p++, SUBIDX_PowerEU1,           &pDevInstallConfig->Power_EU3[0],             6065, 0, TRUE, 0, 3, 
        0, 267, 1, 5800, 2, 5670);  
        Make_ENUM_SubBlobParam(p++, SUBIDX_PowerEU2,           &pDevInstallConfig->Power_EU3[1],               5805, 0, TRUE, 0, 4, 
        0, 267, 1, 5800, 2, 5670, 3, 6343);       
        Make_ENUM_SubBlobParam(p++, SUBIDX_UartServer,         &pDevInstallConfig->UartServer,             2494, 0, TRUE, 0, 2, 
        0, 267, 1, 28);
        Make_ENUM_SubBlobParam(p++, SUBIDX_TempControllType,   &pDevInstallConfig->TempControllType,        2495, 0, TRUE, 0, 3, 
        0, 267, 1, 2510, 2, 2511); 
        Make_ENUM_SubBlobParam(p++, SUBIDX_PowerEU3,           &pDevInstallConfig->MulCube_EU3[0],               6349, 0, TRUE, 0, 3, 
        0, 267, 1, 5800, 2, 5670);
        Make_ENUM_SubBlobParam(p++, SUBIDX_WaterChiller,       &pDevInstallConfig->WaterChiller,              7202, 0, TRUE, 0, 2, 
        0, 267, 1, 28);
        // Make_ENUM_SubBlobParam(p++, SUBIDX_PVEnable,            &pDevInstallConfig->PVEnable,                8616, 0, TRUE, 0, 2, 
        // 0, 324, 1, 323); 

        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_PV_INVERTER_DATA)
    {
        A_BLOB_PV_INVERTER_DATA* pBlobPVInvData = (A_BLOB_PV_INVERTER_DATA*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_PV_INVERTER_DATA_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_PV_INVERTER_DATA_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;
        PV_INVERTER_DATA* pData = &(pBlobPVInvData->PVInverterData);
        Make_ENUM_SubBlobParam(p++, SUBIDX_iOnOffStatus, &pData->nOnOffStatus, 6875, 0, FALSE, 0, 2, 0, 6702, 1, 6703);
        Make_STRING_SubBlobParam(p++, SUBIDX_sn, pData->sn, 343, FALSE, 0, 32);
        Make_STRING_SubBlobParam(p++, SUBIDX_szFwVer, pData->szFwVer, 8002, FALSE, 0, 32);
        Make_STRING_SubBlobParam(p++, SUBIDX_szCtrlVer, pData->szCtrlVer, 8003, FALSE, 0, 32);
        Make_STRING_SubBlobParam(p++, SUBIDX_szModbusVer, pData->szModbusVer, 8004, FALSE, 0, 32);
        Make_UINT_SubBlobParam(p++, SUBIDX_iRatedPower, &pData->iRatedPower, 2381, 211, FALSE,  0, 10000000,  0);
        Make_ENUM_SubBlobParam(p++, SUBIDX_iWorkMode, &pData->iWorkMode, 8005, 0, FALSE, 0, 4, 0, 267, 1, 6710, 2, 920, 3, 198);
        Make_UINT_SubBlobParam(p++, SUBIDX_uErrCode, &pData->uErrCode, 6721, 0, FALSE,  0, -1,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_uWarnCode, &pData->uWarnCode, 6886, 0, FALSE,  0, -1,  0);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fLineVoltAB, &pData->fLineVoltAB, 8006,  161,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fLineVoltBC, &pData->fLineVoltBC, 8007,  161,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fLineVoltCA, &pData->fLineVoltCA, 8008,  161,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fPhaseVoltA, &pData->fPhaseVoltA, 152,  161,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fPhaseVoltB, &pData->fPhaseVoltB, 154,  161,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fPhaseVoltC, &pData->fPhaseVoltC, 156,  161,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fPhaseCurrA, &pData->fPhaseCurrA, 153,  162,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fPhaseCurrB, &pData->fPhaseCurrB, 155,  162,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fPhaseCurrC, &pData->fPhaseCurrC, 157,  162,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fFreq, &pData->fFreq, 143,  144,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fMaxRealTimeOutputPower, &pData->fRealTime_MaxActivePower, 123,  211,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fOutputActivePower, &pData->fOutputActivePower, 6876,  211,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_iOutputPowerFactor, &pData->iOutputPowerFactor, 8009,  0,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fPVInputTotalPower, &pData->fPVInputTotalPower, 8010,  211,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fInnerTemperature, &pData->fInnerTemperature, 8011,  127,  FALSE, -99999, 99999, 0, 1);
        
        int i = 0;
        for (i = 0; i < MAX_PV_NUM; i++) 
        {
            Make_FLOAT_SubBlobParam(p++, SUBIDX_fPVGroupVolt1 + 2*i,  &pData->fPVGroupVolt[i],    
                           8017 + 2*i, 161, FALSE, -99999, 99999, 0, 1);
            Make_FLOAT_SubBlobParam(p++, SUBIDX_fPVGroupCurr1 + 2*i,  &pData->fPVGroupCurr[i],   
                           8018 + 2*i, 162, FALSE, -99999, 99999, 0, 1);
        }

        static char szPVTime[128];
        time_t sysTime = (time_t)time(NULL); 
        struct tm* timeInfo = localtime(&sysTime);
        memset(szPVTime, 0, sizeof(szPVTime));
        snprintf(szPVTime, 128, "%04d-%02d-%02d %02d:%02d:%02d", timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday, 
		timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
        Make_STRING_SubBlobParam(p++, SUBIDX_szLocalTime, szPVTime, 274, FALSE, 0, 128);
        // TRACED("----szPVTime:%s------\n", szPVTime);
        Make_ENUM_SubBlobParam(p++, SUBIDX_MultiAntiFluxEnable, &pData->nParallelAntiBackflowEnable, 8634, 0, FALSE, 0, 2, 0, 324, 1, 323);
        Make_UINT_SubBlobParam(p++, SUBIDX_AntiFluxFailResponseTime, &pData->nAntiBackflowFailureResponseTime, 8635, 163, FALSE,  1, 5000,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_MultiAntiFluxPowerLimit, &pData->nParallelAntiBackflowPowerLimit, 8636, 16, FALSE,  0, 100,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_iActivePowerRate, &pData->iActivePowerRate, 8012, 16, FALSE,  0, 10000000,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_iRealPowerPercent, &pData->iRealPowerPercent, 8013, 16, FALSE,  0, 10000000,  0);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fDailyPowerGeneration, &pData->fDailyPowerGeneration, 8014,  14,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_fTotalPowerGeneration, &pData->fTotalPowerGeneration, 8015,  14,  FALSE, -99999, 99999, 0, 1);

        /*
        for (i = 0; i < MAX_PV_NUM; i++) 
        {
            Make_FLOAT_SubBlobParam(p++, SUBIDX_fPVGrpDailyPowerGeneration1 + 2*i,  &pData->fPvGrpDailyPowerGeneration[i],    
                           8099 + 2*i, 14, FALSE, -99999, 99999, 0, 1);
            Make_FLOAT_SubBlobParam(p++, SUBIDX_fPVGrpTotalPowerGeneration1 + 2*i,  &pData->fPvGrpTotalPowerGeneration[i],   
                           8100 + 2*i, 14, FALSE, -99999, 99999, 0, 1);
        }
*/

        Make_FLOAT_SubBlobParam(p++, SUBIDX_fPVDCTotalPowerGeneration, &pData->fPVDCTotalPowerGeneration, 8016,  14,  FALSE, -99999, 99999, 0, 1);

        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_PV_INVERTER_DATA_TOTAL)
    {
        A_BLOB_PV_INVERTER_DATA_TOTAL* pBlobPVInvDataTotal = (A_BLOB_PV_INVERTER_DATA_TOTAL*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_PV_INVERTER_DATA_TOTAL_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_PV_INVERTER_DATA_TOTAL_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_ENUM_SubBlobParam(p++,  SUBIDX_TOTAL_PVCtrlMode,                &pBlobPVInvDataTotal->RecvCtrlMode,                     465,  0,  FALSE, 0, 2, 0, 8619, 1, 8620);
        Make_ENUM_SubBlobParam(p++,  SUBIDX_TOTAL_DOSwitchStatus,            &pBlobPVInvDataTotal->emDIstatus,                        8630, 0,  FALSE, 0, 2, 0, 8631, 1, 8632);
        Make_ENUM_SubBlobParam(p++,  SUBIDX_TOTAL_PVManualEnable,            &pBlobPVInvDataTotal->bEnableManualMode,                8629, 0,  FALSE, 0, 2, 0, 324, 1, 323);
        Make_ENUM_SubBlobParam(p++, SUBIDX_TOTAL_PVStatus,                   &pBlobPVInvDataTotal->nRecvPVStatus,                    8618, 0,  FALSE, 0, 2, 0, 6702, 1, 6703);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_TOTAL_PVActivePower,             &pBlobPVInvDataTotal->fRecvActivePower,                 8615, 165, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_TOTAL_PVReActivePower,           &pBlobPVInvDataTotal->fRecvReActivePower,               8617, 165, FALSE, -99999, 99999, 0, 1);
        Make_UINT_SubBlobParam(p++, SUBIDX_TOTAL_PVAvailablePVNums,           &pBlobPVInvDataTotal->nAvailablePVNums,                8622, 0,  FALSE,  0, 10,  0);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_TOTAL_PVPower,                     &pBlobPVInvDataTotal->fTotalPVPowerAbility,           123, 165, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_TOTAL_PVPInput,                    &pBlobPVInvDataTotal->fTotalPVInput,                  8010, 165, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_TOTAL_PVOutputActPower,            &pBlobPVInvDataTotal->fTotalPVOutputActPower,         6876, 165, FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_TOTAL_PVDailyGenerationSum,        &pBlobPVInvDataTotal->fDailyPowerGenerationSum,       8014, 14,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_TOTAL_PVTotalGenerationSum,        &pBlobPVInvDataTotal->fTotalPowerGenerationSum,       8015, 14,  FALSE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_TOTAL_PVDCTotalGenerationSum,      &pBlobPVInvDataTotal->fPVDCTotalPowerGenerationSum,   8016, 14,  FALSE, -99999, 99999, 0, 1);
        Make_UINT_SubBlobParam(p++, SUBIDX_TOTAL_PVTotalRatedPower,           &pBlobPVInvDataTotal->iTotalRatedPower,               2381, 165, FALSE,  0, 10000000,  0);
        Make_ENUM_SubBlobParam(p++, SUBIDX_TOTAL_PVTotalStatus,              &pBlobPVInvDataTotal->emPvRunStatus,                    8005, 0, FALSE, 0, 4, 0, 267, 1, 6710, 2, 920, 3, 198);
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_PV_SRTTING_BY_AEMS)
    {
        A_BLOB_PV_CTRL_BY_AEMS* pBlobPVCtrlByAEMS = (A_BLOB_PV_CTRL_BY_AEMS*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_AEMS_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_AEMS_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        Make_ENUM_SubBlobParam(p++,  SUBIDX_AEMS_PVCtrlMode,                &pBlobPVCtrlByAEMS->CtrlMode,            465,  0,  TRUE, 0, 2, 0, 8619, 1, 8620); 
        Make_ENUM_SubBlobParam(p++, SUBIDX_AEMS_PVStatus,                   &pBlobPVCtrlByAEMS->nPVStatus,           8618, 0,  TRUE, 0, 2, 0, 6702, 1, 6703);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_AEMS_PVActivePower,             &pBlobPVCtrlByAEMS->fActivePower,        8615, 165, TRUE, -99999, 99999, 0, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_AEMS_PVReActivePower,           &pBlobPVCtrlByAEMS->fReActivePower,      8617, 165, TRUE, -99999, 99999, 0, 1);

        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_BMS_TESTMODE_SET)
    {
        A_BLOB_BMS_TESTMODE_SET* pDytestSet = (A_BLOB_BMS_TESTMODE_SET*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_DYTEST_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_DYTEST_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        //----  Bit Map1  ----//
        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_SOC1,      &pDytestSet->autoTestParm[0].nStopSOCStep1,  7020, 0,   TRUE,  0, 100,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_SOC3,      &pDytestSet->autoTestParm[0].nStopSOCStep3,  7021, 0,   TRUE,  0, 100,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_SOC5,      &pDytestSet->autoTestParm[0].nStopSOCStep5,  7022, 0,   TRUE,  0, 100,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_SOC7,      &pDytestSet->autoTestParm[0].nStopSOCStep7,  7023, 0,   TRUE,  0, 100,  0);

        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_PWR1,      &pDytestSet->autoTestParm[0].nPwrLmtStep1,   7024, 0,   TRUE,  0, 300,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_PWR3,      &pDytestSet->autoTestParm[0].nPwrLmtStep3,   7025, 0,   TRUE,  0, 300,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_PWR5,      &pDytestSet->autoTestParm[0].nPwrLmtStep5,   7026, 0,   TRUE,  0, 300,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_PWR7,      &pDytestSet->autoTestParm[0].nPwrLmtStep7,   7027, 0,   TRUE,  0, 300,  0);

        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_WAIT2,     &pDytestSet->autoTestParm[0].nWaitTimeStep2, 7028, 0,   TRUE,  0, 100,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_WAIT4,     &pDytestSet->autoTestParm[0].nWaitTimeStep4, 7029, 0,   TRUE,  0, 100,  0);
        Make_UINT_SubBlobParam(p++, SUBIDX_DYTEST_WAIT6,     &pDytestSet->autoTestParm[0].nWaitTimeStep6, 7030, 0,   TRUE,  0, 100,  0);
        
        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_ENERGY_STAT)
    {
        A_BLOB_ENERGY_STAT* pEnergyStat = (A_BLOB_ENERGY_STAT*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_ENERGY_STAT_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_ENERGY_STAT_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        // ========== 组0 ==========
        // Power Info [0]
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_POWER_SOC,            &pEnergyStat->power_info[0].soc,              7604, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_POWER_SOE,            &pEnergyStat->power_info[0].soe,              7605, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_POWER_SOH,            &pEnergyStat->power_info[0].soh,              7606, 16, TRUE, 0, 65535, 2);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_ENERGY_GRP0_POWER_TOTAL_CAPACITY, &pEnergyStat->power_info[0].total_capacity,   7607, 7873, TRUE, 0, 999999, 2, 1);

        // Energy Curr Stat [0]
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_CURR_REMAINING_ENERGY,      &pEnergyStat->energy_curr_stat[0].remaining_energy,       7612, 14, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_CURR_TOTAL_ENERGY,          &pEnergyStat->energy_curr_stat[0].total_energy,           7613, 14, TRUE, 0, 999999, 2, 1);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_CURR_SINGLE_CHARGE_ENERGY, &pEnergyStat->energy_curr_stat[0].single_charge_energy,   7614, 7873, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_CURR_SINGLE_DISCHARGE_ENERGY, &pEnergyStat->energy_curr_stat[0].single_discharge_energy, 7615, 7873, TRUE, 0, 65535, 2);

        // Energy Total Stat [0]
        Make_UINT_SubBlobParam(p++,  SUBIDX_ENERGY_GRP0_TOTAL_CHARGE_ENERGY,    &pEnergyStat->energy_total_stat[0].total_charge_energy,    7620, 7873, TRUE, 0, 4294967295, 2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_ENERGY_GRP0_TOTAL_DISCHARGE_ENERGY, &pEnergyStat->energy_total_stat[0].total_discharge_energy, 7621, 7873, TRUE, 0, 4294967295, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_TOTAL_CHARGE_POWER,     &pEnergyStat->energy_total_stat[0].total_charge_power,     7622, 14, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_TOTAL_DISCHARGE_POWER,  &pEnergyStat->energy_total_stat[0].total_discharge_power,  7623, 14, TRUE, 0, 999999, 2, 1);

        // DOD Stat [0]
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_DOD_CURRENT_DOD,    &pEnergyStat->dod_stat[0].current_dod,    7628, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_DOD_START_DOD,      &pEnergyStat->dod_stat[0].start_dod,      7629, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_DOD_HISTORY_MAX_DOD,&pEnergyStat->dod_stat[0].history_max_dod,7630, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP0_DOD_HISTORY_AVG_DOD,&pEnergyStat->dod_stat[0].history_avg_dod,7631, 16, TRUE, 0, 65535, 2);

        // ========== 组1 ==========
        // Power Info [1]
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_POWER_SOC,            &pEnergyStat->power_info[1].soc,              7608, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_POWER_SOE,            &pEnergyStat->power_info[1].soe,              7609, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_POWER_SOH,            &pEnergyStat->power_info[1].soh,              7610, 16, TRUE, 0, 65535, 2);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_ENERGY_GRP1_POWER_TOTAL_CAPACITY, &pEnergyStat->power_info[1].total_capacity,   7611, 7873, TRUE, 0, 999999, 2, 1);

        // Energy Curr Stat [1]
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_CURR_REMAINING_ENERGY,      &pEnergyStat->energy_curr_stat[1].remaining_energy,       7616, 14, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_CURR_TOTAL_ENERGY,          &pEnergyStat->energy_curr_stat[1].total_energy,           7617, 14, TRUE, 0, 999999, 2, 1);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_CURR_SINGLE_CHARGE_ENERGY, &pEnergyStat->energy_curr_stat[1].single_charge_energy,   7618, 7873, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_CURR_SINGLE_DISCHARGE_ENERGY, &pEnergyStat->energy_curr_stat[1].single_discharge_energy, 7619, 7873, TRUE, 0, 65535, 2);

        // Energy Total Stat [1]
        Make_UINT_SubBlobParam(p++,  SUBIDX_ENERGY_GRP1_TOTAL_CHARGE_ENERGY,    &pEnergyStat->energy_total_stat[1].total_charge_energy,    7624, 7873, TRUE, 0, 4294967295, 2);
        Make_UINT_SubBlobParam(p++,  SUBIDX_ENERGY_GRP1_TOTAL_DISCHARGE_ENERGY, &pEnergyStat->energy_total_stat[1].total_discharge_energy, 7625, 7873, TRUE, 0, 4294967295, 2);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_TOTAL_CHARGE_POWER,     &pEnergyStat->energy_total_stat[1].total_charge_power,     7626, 14, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_TOTAL_DISCHARGE_POWER,  &pEnergyStat->energy_total_stat[1].total_discharge_power,  7627, 14, TRUE, 0, 999999, 2, 1);

        // DOD Stat [1]
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_DOD_CURRENT_DOD,    &pEnergyStat->dod_stat[1].current_dod,    7632, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_DOD_START_DOD,      &pEnergyStat->dod_stat[1].start_dod,      7633, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_DOD_HISTORY_MAX_DOD,&pEnergyStat->dod_stat[1].history_max_dod,7634, 16, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_ENERGY_GRP1_DOD_HISTORY_AVG_DOD,&pEnergyStat->dod_stat[1].history_avg_dod,7635, 16, TRUE, 0, 65535, 2);

        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_STATUS_STAT)
    {
        A_BLOB_STATUS_STAT* pStatusStat = (A_BLOB_STATUS_STAT*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_STATUS_STAT_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_STATUS_STAT_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        // ========== 组0 ==========
        // Status Info [0]
        Make_ENUM_SubBlobParam(p++,  SUBIDX_STATUS_GRP0_INFO_PROTECT_STATUS,    &pStatusStat->status_info[0].protect_status,    7636, 0, TRUE, 2, 6, 0, 7880, 1, 34, 2, 7881, 3, 7882, 4, 7883, 5, 7884);
        Make_ENUM_SubBlobParam(p++,  SUBIDX_STATUS_GRP0_INFO_WORK_STATUS,       &pStatusStat->status_info[0].work_status,    7637, 0, TRUE, 2, 7, 0, 6703, 1, 6710, 2, 229, 3, 920, 4, 222, 5, 198, 6, 6702);
        // Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_INFO_PROTECT_STATUS,        &pStatusStat->status_info[0].protect_status,        7636, 0, TRUE, 0, 65535, 2);
        // Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_INFO_WORK_STATUS,           &pStatusStat->status_info[0].work_status,           7637, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_INFO_CHARGE_DISCHARGE_STATUS,&pStatusStat->status_info[0].charge_discharge_status,7638, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_INFO_WORK_MODE,             &pStatusStat->status_info[0].work_mode,             7639, 0, TRUE, 0, 65535, 2);

        // Global Info [0]
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_GLOBAL_TOTAL_VOLTAGE,     &pStatusStat->global_info[0].total_voltage,     7644, 161, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_GLOBAL_TOTAL_CURRENT,     &pStatusStat->global_info[0].total_current,     7645, 162, TRUE, -999999, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_GLOBAL_OUTPUT_POWER,      &pStatusStat->global_info[0].output_power,      7646, 165, TRUE, -999999, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_GLOBAL_PRECHARGE_VOLTAGE, &pStatusStat->global_info[0].precharge_voltage, 7647, 161, TRUE, 0, 999999, 2, 1);

        // Limit Info [0]
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_LIMIT_MAX_CHARGE_CURRENT,   &pStatusStat->limit_info[0].max_charge_current,   7652, 162, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_LIMIT_MAX_DISCHARGE_CURRENT,&pStatusStat->limit_info[0].max_discharge_current,7653, 162, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_LIMIT_MAX_CHARGE_POWER,     &pStatusStat->limit_info[0].max_charge_power,     7654, 165, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_LIMIT_MAX_DISCHARGE_POWER,  &pStatusStat->limit_info[0].max_discharge_power,  7655, 165, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_LIMIT_MAX_CHARGE_VOLTAGE,   &pStatusStat->limit_info[0].max_charge_voltage,   7656, 161, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_LIMIT_MAX_DISCHARGE_VOLTAGE,&pStatusStat->limit_info[0].max_discharge_voltage,7657, 161, TRUE, 0, 999999, 2, 1);

        // Feature Info [0]
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP0_FEATURE_CELL_TOTAL_VOLTAGE,  &pStatusStat->feature_info[0].cell_total_voltage,  7664,161, TRUE, 0, 999999, 2, 1);
        Make_INT_SubBlobParam(p++,    SUBIDX_STATUS_GRP0_FEATURE_CELL_AVG_VOLTAGE,    &pStatusStat->feature_info[0].cell_avg_voltage,    7665, 2200, TRUE, -32768, 32767, 2);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP0_FEATURE_CELL_AVG_TEMPERATURE,&pStatusStat->feature_info[0].cell_avg_temperature,7666, 127, TRUE, -999, 999, 2, 1);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP0_FEATURE_HVBOX_TEMPERATURE,   &pStatusStat->feature_info[0].hvbox_temperature,   7667, 127, TRUE, -999, 999, 2, 1);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_FEATURE_MAX_VOLTAGE_INDEX,   &pStatusStat->feature_info[0].max_voltage_index,   7668, 0, TRUE, 0, 65535, 2);
        Make_INT_SubBlobParam(p++,    SUBIDX_STATUS_GRP0_FEATURE_MAX_VOLTAGE_VALUE,   &pStatusStat->feature_info[0].max_voltage_value,   7669, 2200, TRUE, -32768, 32767, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_FEATURE_MIN_VOLTAGE_INDEX,   &pStatusStat->feature_info[0].min_voltage_index,   7670, 0, TRUE, 0, 65535, 2);
        Make_INT_SubBlobParam(p++,    SUBIDX_STATUS_GRP0_FEATURE_MIN_VOLTAGE_VALUE,   &pStatusStat->feature_info[0].min_voltage_value,   7671, 2200, TRUE, -32768, 32767, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_FEATURE_MAX_TEMPERATURE_INDEX,&pStatusStat->feature_info[0].max_temperature_index,7672, 0, TRUE, 0, 65535, 2);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP0_FEATURE_MAX_TEMPERATURE_VALUE,&pStatusStat->feature_info[0].max_temperature_value,7673, 127, TRUE, -999, 999, 2, 1);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP0_FEATURE_MIN_TEMPERATURE_INDEX,&pStatusStat->feature_info[0].min_temperature_index,7674, 0, TRUE, 0, 65535, 2);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP0_FEATURE_MIN_TEMPERATURE_VALUE,&pStatusStat->feature_info[0].min_temperature_value,7675, 127, TRUE, -999, 999, 2, 1);

        // ========== 组1 ==========
        // Status Info [1]
        Make_ENUM_SubBlobParam(p++,  SUBIDX_STATUS_GRP1_INFO_PROTECT_STATUS,       &pStatusStat->status_info[1].protect_status,    7640, 0, TRUE, 2, 6, 0, 7880, 1, 34, 2, 7881, 3, 7882, 4, 7883, 5, 7884);
        // Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_INFO_PROTECT_STATUS,        &pStatusStat->status_info[1].protect_status,        7640, 0, TRUE, 0, 65535, 2);
        Make_ENUM_SubBlobParam(p++,  SUBIDX_STATUS_GRP1_INFO_WORK_STATUS,       &pStatusStat->status_info[1].work_status,    7641, 0, TRUE, 2, 7, 0, 6703, 1, 6710, 2, 229, 3, 920, 4, 222, 5, 198, 6, 6702);
        // Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_INFO_PROTECT_STATUS,        &pStatusStat->status_info[1].protect_status,        7640, 0, TRUE, 0, 65535, 2);
        // Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_INFO_WORK_STATUS,           &pStatusStat->status_info[1].work_status,           7641, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_INFO_CHARGE_DISCHARGE_STATUS,&pStatusStat->status_info[1].charge_discharge_status,7642, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_INFO_WORK_MODE,             &pStatusStat->status_info[1].work_mode,             7643, 0, TRUE, 0, 65535, 2);

        // Global Info [1]
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_GLOBAL_TOTAL_VOLTAGE,     &pStatusStat->global_info[1].total_voltage,     7648, 161, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_GLOBAL_TOTAL_CURRENT,     &pStatusStat->global_info[1].total_current,     7649, 162, TRUE, -999999, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_GLOBAL_OUTPUT_POWER,      &pStatusStat->global_info[1].output_power,      7650, 165, TRUE, -999999, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_GLOBAL_PRECHARGE_VOLTAGE, &pStatusStat->global_info[1].precharge_voltage, 7651, 161, TRUE, 0, 999999, 2, 1);

        // Limit Info [1]
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_LIMIT_MAX_CHARGE_CURRENT,   &pStatusStat->limit_info[1].max_charge_current,   7658, 162, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_LIMIT_MAX_DISCHARGE_CURRENT,&pStatusStat->limit_info[1].max_discharge_current,7659, 162, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_LIMIT_MAX_CHARGE_POWER,     &pStatusStat->limit_info[1].max_charge_power,     7660, 165, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_LIMIT_MAX_DISCHARGE_POWER,  &pStatusStat->limit_info[1].max_discharge_power,  7661, 165, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_LIMIT_MAX_CHARGE_VOLTAGE,   &pStatusStat->limit_info[1].max_charge_voltage,   7662, 161, TRUE, 0, 999999, 2, 1);
        Make_FLOAT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_LIMIT_MAX_DISCHARGE_VOLTAGE,&pStatusStat->limit_info[1].max_discharge_voltage,7663, 161, TRUE, 0, 999999, 2, 1);

        // Feature Info [1]
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP1_FEATURE_CELL_TOTAL_VOLTAGE,  &pStatusStat->feature_info[1].cell_total_voltage,  7676,161, TRUE, 0, 999999, 2, 1);
        Make_INT_SubBlobParam(p++,    SUBIDX_STATUS_GRP1_FEATURE_CELL_AVG_VOLTAGE,    &pStatusStat->feature_info[1].cell_avg_voltage,    7677, 2200, TRUE, -32768, 32767, 2);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP1_FEATURE_CELL_AVG_TEMPERATURE,&pStatusStat->feature_info[1].cell_avg_temperature,7678, 127, TRUE, -999, 999, 2, 1);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP1_FEATURE_HVBOX_TEMPERATURE,   &pStatusStat->feature_info[1].hvbox_temperature,   7679, 127, TRUE, -999, 999, 2, 1);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_FEATURE_MAX_VOLTAGE_INDEX,   &pStatusStat->feature_info[1].max_voltage_index,   7680, 0, TRUE, 0, 65535, 2);
        Make_INT_SubBlobParam(p++,    SUBIDX_STATUS_GRP1_FEATURE_MAX_VOLTAGE_VALUE,   &pStatusStat->feature_info[1].max_voltage_value,   7681, 2200, TRUE, -32768, 32767, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_FEATURE_MIN_VOLTAGE_INDEX,   &pStatusStat->feature_info[1].min_voltage_index,   7682, 0, TRUE, 0, 65535, 2);
        Make_INT_SubBlobParam(p++,    SUBIDX_STATUS_GRP1_FEATURE_MIN_VOLTAGE_VALUE,   &pStatusStat->feature_info[1].min_voltage_value,   7683, 2200, TRUE, -32768, 32767, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_FEATURE_MAX_TEMPERATURE_INDEX,&pStatusStat->feature_info[1].max_temperature_index,7684, 0, TRUE, 0, 65535, 2);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP1_FEATURE_MAX_TEMPERATURE_VALUE,&pStatusStat->feature_info[1].max_temperature_value,7685, 127, TRUE, -999, 999, 2, 1);
        Make_USHORT_SubBlobParam(p++, SUBIDX_STATUS_GRP1_FEATURE_MIN_TEMPERATURE_INDEX,&pStatusStat->feature_info[1].min_temperature_index,7686, 0, TRUE, 0, 65535, 2);
        Make_FLOAT_SubBlobParam(p++,  SUBIDX_STATUS_GRP1_FEATURE_MIN_TEMPERATURE_VALUE,&pStatusStat->feature_info[1].min_temperature_value,7687, 127, TRUE, -999, 999, 2, 1);

        bCreateOK = TRUE;
    }
    else if(pParamVal->pParamInfo->nBlobId == BLOB_TYPE_DIAG_STAT)
    {
        A_BLOB_DIAG_STAT* pDiagStat = (A_BLOB_DIAG_STAT*)pBlobVal_Raw;
        pBlob2Create->pSubBlobList = NEW(WEB_SUBBLOB_ITEM, SUBIDX_DIAG_STAT_MAXID);
        pBlob2Create->nSubBlobTotal = SUBIDX_DIAG_STAT_MAXID;
        WEB_SUBBLOB_ITEM* p = pBlob2Create->pSubBlobList;

        // SOS Stat [0]
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_SOS_CURRENT_SOS_VALUE,  &pDiagStat->sos_stat[0].current_sos_value,  7818, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_SOS_CURRENT_SOS_REASON, &pDiagStat->sos_stat[0].current_sos_reason, 7819, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_SOS_PREDICT_SOS_VALUE,  &pDiagStat->sos_stat[0].predict_sos_value,  7820, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_SOS_PREDICT_SOS_REASON, &pDiagStat->sos_stat[0].predict_sos_reason, 7821, 0, TRUE, 0, 65535, 2);
        // Time Stat [0]
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_TIME_SYSTEM_RUN_TIME,       &pDiagStat->time_stat[0].system_run_time,       7802, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_TIME_CHARGE_AVAILABLE_TIME, &pDiagStat->time_stat[0].charge_available_time, 7803, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_TIME_DISCHARGE_AVAILABLE_TIME,&pDiagStat->time_stat[0].discharge_available_time,7804, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_TIME_CYCLE_COUNT,           &pDiagStat->time_stat[0].cycle_count,           7805, 0, TRUE, 0, 65535, 2);
        // Fault Stat [0]
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_FAULT_MONTHLY_ALARM_COUNT, &pDiagStat->fault_stat[0].monthly_alarm_count, 7810, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_FAULT_MONTHLY_FAULT_COUNT, &pDiagStat->fault_stat[0].monthly_fault_count, 7811, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_FAULT_TOTAL_ALARM_COUNT,   &pDiagStat->fault_stat[0].total_alarm_count,   7812, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP0_FAULT_TOTAL_FAULT_COUNT,   &pDiagStat->fault_stat[0].total_fault_count,   7813, 0, TRUE, 0, 65535, 2);

        // SOS Stat [1]
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_SOS_CURRENT_SOS_VALUE,  &pDiagStat->sos_stat[1].current_sos_value,  7822, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_SOS_CURRENT_SOS_REASON, &pDiagStat->sos_stat[1].current_sos_reason, 7823, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_SOS_PREDICT_SOS_VALUE,  &pDiagStat->sos_stat[1].predict_sos_value,  7824, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_SOS_PREDICT_SOS_REASON, &pDiagStat->sos_stat[1].predict_sos_reason, 7825, 0, TRUE, 0, 65535, 2);
        // Time Stat [1]
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_TIME_SYSTEM_RUN_TIME,       &pDiagStat->time_stat[1].system_run_time,       7806, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_TIME_CHARGE_AVAILABLE_TIME, &pDiagStat->time_stat[1].charge_available_time, 7807, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_TIME_DISCHARGE_AVAILABLE_TIME,&pDiagStat->time_stat[1].discharge_available_time,7808, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_TIME_CYCLE_COUNT,           &pDiagStat->time_stat[1].cycle_count,           7809, 0, TRUE, 0, 65535, 2);
        // Fault Stat [1]
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_FAULT_MONTHLY_ALARM_COUNT, &pDiagStat->fault_stat[1].monthly_alarm_count, 7814, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_FAULT_MONTHLY_FAULT_COUNT, &pDiagStat->fault_stat[1].monthly_fault_count, 7815, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_FAULT_TOTAL_ALARM_COUNT,   &pDiagStat->fault_stat[1].total_alarm_count,   7816, 0, TRUE, 0, 65535, 2);
        Make_USHORT_SubBlobParam(p++, SUBIDX_DIAG_GRP1_FAULT_TOTAL_FAULT_COUNT,   &pDiagStat->fault_stat[1].total_fault_count,   7817, 0, TRUE, 0, 65535, 2);

        bCreateOK = TRUE;
    }

    //if not create OK, we need delete the memory
    if((!bCreateOK) && (bNeedCreate))
    {
        SAFELY_DELETE(pBlobVal_Raw);
    }
    return bCreateOK;
}

void Delete_AWebBlob_Param(A_WEBBLOB_PARAM* pBlob2Delete)
{
    if(pBlob2Delete == NULL)
    {
        return;
    }

    DELETE(pBlob2Delete->pBlobVal_Raw);
    DELETE(pBlob2Delete->pSubBlobList);
    //pBlob2Delete = NULL;
}

WEB_SUBBLOB_ITEM* GetSubBlobItem_FromSubBlobIdx(A_WEBBLOB_PARAM* pBlob2Find, int nSubBlobIdx_2Set)
{
    int n;
    WEB_SUBBLOB_ITEM* pItem = pBlob2Find->pSubBlobList;
    for(n = 0; n < pBlob2Find->nSubBlobTotal; n++, pItem++)
    {
        if(pItem->nSubParam_UniqId == nSubBlobIdx_2Set)
        {
            //("----------getBlobItem[%d] OK--------\n", nSubBlobIdx_2Set);
            return pItem;
        }
    }
    //("----------getBlobItem[%d] FFFAILED--------\n", nSubBlobIdx_2Set);
    return NULL;
}

WEB_VALUE_TYPE GetSubBlobParam_ValType(WEB_SUBBLOB_ITEM* pSubBlobVal)
{
    WEB_VALUE_TYPE tp = WEB_VALTYPE_INT;
    switch (pSubBlobVal->emSubBlobValType)
    {
    case SUB_BLOB_VALTYPE_INT:
    case SUB_BLOB_VALTYPE_UINT:    
    case SUB_BLOB_VALTYPE_BYTE:
    case SUB_BLOB_VALTYPE_USHORT:
        tp = WEB_VALTYPE_INT;
        break;
    case SUB_BLOB_VALTYPE_STRING:
        tp = WEB_VALTYPE_STRING;
        break;
    case SUB_BLOB_VALTYPE_FLOAT:
        if(pSubBlobVal->nPrecision == 1)
        {
            tp = WEB_VALTYPE_FLOAT_1;
        }
        else if(pSubBlobVal->nPrecision == 2)
        {
            tp = WEB_VALTYPE_FLOAT_2;
        }
        else if(pSubBlobVal->nPrecision == 3)
        {
            tp = WEB_VALTYPE_FLOAT_3;
        }
        else if(pSubBlobVal->nPrecision == 4)
        {
            tp = WEB_VALTYPE_FLOAT_4;
        }
        else 
        {
            tp = WEB_VALTYPE_FLOAT_0;
        }
        break;
    case SUB_BLOB_VALTYPE_IP:
        tp = WEB_VALTYPE_IP;
        break;
    case SUB_BLOB_VALTYPE_ENUM:   
    case SUB_BLOB_VALTYPE_BIT:
        tp = WEB_VALTYPE_ENUM;
        break; 
    case SUB_BLOB_VALTYPE_DATETIME:
        tp = WEB_VALTYPE_DATETIME;  
        break;
    default:
        break;
    } 

    return tp;
}

BOOL SetStr_2WebSubBlob_ParamVal(A_WEBBLOB_PARAM* pBlob2Set, int nSubBlobIdx_2Set, char* szVal_2Set, int nValLen)
{
    WEB_SUBBLOB_ITEM* pSubBlob = GetSubBlobItem_FromSubBlobIdx(pBlob2Set, nSubBlobIdx_2Set);
    if(pSubBlob == NULL)
    {
        return FALSE;
    }

    WEB_VALUE_TYPE tp = GetSubBlobParam_ValType(pSubBlob);
    if(SUB_BLOB_VALTYPE_BIT == pSubBlob->emSubBlobValType)
    {
        if(pSubBlob->nBits == 2)
        {
            USHORT usVal = (USHORT)atoi(szVal_2Set);
            usVal &= 0x0003;
            USHORT ulCurrVal = *(USHORT*)pSubBlob->pDataVal;
            ulCurrVal &= ((0x0003 << pSubBlob->nBitOffset) ^ 0xFFFF);
            *(USHORT*)pSubBlob->pDataVal = ulCurrVal | (usVal << pSubBlob->nBitOffset);
        }
        else if(pSubBlob->nBits == 1)
        {
            if(pSubBlob->nBitOffset < 8)
            {
                BYTE byVal = (BYTE)atoi(szVal_2Set);
                byVal &= 0x01;
                BYTE byCurrVal = *(BYTE*)pSubBlob->pDataVal;
                byCurrVal &= ((0x01 << pSubBlob->nBitOffset) ^ 0xFF);
                *(BYTE*)pSubBlob->pDataVal = byCurrVal | (byVal << pSubBlob->nBitOffset);
            }
            else
            {
                USHORT usVal = (USHORT)atoi(szVal_2Set);
                usVal &= 0x0001;
                USHORT ulCurrVal = *(USHORT*)pSubBlob->pDataVal;
                ulCurrVal &= ((0x0001 << pSubBlob->nBitOffset) ^ 0xFFFF);
                *(USHORT*)pSubBlob->pDataVal = ulCurrVal | (usVal << pSubBlob->nBitOffset);
                //printf("ulCurrVal=[%u] usVal=[%u] nBitOffset=[%d]\n", ulCurrVal, usVal, pSubBlob->nBitOffset);
            }
        }
        else
        {
            return FALSE;
        }        
    }
    else if(SUB_BLOB_VALTYPE_BYTE == pSubBlob->emSubBlobValType)
    {
        *(BYTE*)pSubBlob->pDataVal = (BYTE)atoi(szVal_2Set);
    }
    else if(SUB_BLOB_VALTYPE_USHORT == pSubBlob->emSubBlobValType)
    {
        *(USHORT*)pSubBlob->pDataVal = (USHORT)atoi(szVal_2Set);
    }
    else if(WEB_VALTYPE_IP == tp)
    {
        *(ULONG*)pSubBlob->pDataVal  = inet_addr(szVal_2Set);
    }
    else if(WEB_VALTYPE_INT == tp)
    {
        *(UINT*)pSubBlob->pDataVal = (UINT)atoi(szVal_2Set);
    }
    else if(WEB_VALTYPE_ENUM == tp)
    {
        *(int*)pSubBlob->pDataVal = atoi(szVal_2Set);
    }
    else if(WEB_VALTYPE_STRING == tp)
    {
        if(nValLen > pSubBlob->nValueSize)
        {
            return FALSE;
        }
        memset(pSubBlob->pDataVal, 0, pSubBlob->nValueSize);
        memcpy(pSubBlob->pDataVal, szVal_2Set, nValLen);
    }
    else if(SUB_BLOB_VALTYPE_FLOAT == pSubBlob->emSubBlobValType)
    {
        *(float*)pSubBlob->pDataVal = (float)atof(szVal_2Set);
    }
    else if(SUB_BLOB_VALTYPE_DATETIME == pSubBlob->emSubBlobValType)
    {
        *(time_t*)pSubBlob->pDataVal = (time_t)StringToTime((char*)szVal_2Set, TIME_CHN_FMT);
    }
    else 
    {
        TRACEX("------other--dererererer----\n");
    }
    return TRUE;
}


char* GetStrVal_FromWebSubBlobVal(WEB_SUBBLOB_ITEM* pSubBlobVal, char* pszOut, int nValLen)
{
    memset(pszOut, 0, nValLen);
    WEB_SUBBLOB_ITEM* pItem = pSubBlobVal;
    if(pItem == NULL)
    {
        TRACEX("---now we got a NULL subBlobItemdddddd---\n");
        return pszOut;
    }
    int nEnumVal, n;
    struct in_addr ip;
    USHORT usVal;
    BYTE byVal;
    switch (pItem->emSubBlobValType)
    {
    case SUB_BLOB_VALTYPE_INT:
        snprintf(pszOut, nValLen, "%d", *(int*)pItem->pDataVal);
        break;
    case SUB_BLOB_VALTYPE_UINT:
        snprintf(pszOut, nValLen, "%u", *(UINT*)pItem->pDataVal);
        break;
    case SUB_BLOB_VALTYPE_USHORT:
        snprintf(pszOut, nValLen, "%u", *(USHORT*)pItem->pDataVal);
        break;
    case SUB_BLOB_VALTYPE_BYTE:
        snprintf(pszOut, nValLen, "%u", *(BYTE*)pItem->pDataVal);
        break;
    case SUB_BLOB_VALTYPE_STRING:
        snprintf(pszOut, nValLen, "%s", (char*)pItem->pDataVal);
        break;
    case SUB_BLOB_VALTYPE_FLOAT:
        switch (pItem->nPrecision)
        {
        case 0:
            snprintf(pszOut, nValLen, "%.0f", *(float*)pItem->pDataVal);
            break;
        case 1:
            snprintf(pszOut, nValLen, "%.1f", *(float*)pItem->pDataVal);
            break;
        case 2:
            snprintf(pszOut, nValLen, "%.2f", *(float*)pItem->pDataVal);
            break;      
        default:
            snprintf(pszOut, nValLen, "%.3f", *(float*)pItem->pDataVal);
            break;
        }
        break;
    case SUB_BLOB_VALTYPE_DATETIME:
        {
            time_t cTime = *(time_t*)pItem->pDataVal;
            char szTime[32]={0};
            TimeToString(cTime/*Conv_UTC_2LocalTime(cTime)*/, TIME_CHN_FMT, szTime, 32);
            snprintf(pszOut, nValLen, "%s", szTime);
            TRACE("szTime =%s nValLen=%d\n", szTime, nValLen);
            break;
        }
    case SUB_BLOB_VALTYPE_IP:
        ip.s_addr = *(UINT*)pItem->pDataVal;
        snprintf(pszOut, nValLen, "%s", inet_ntoa(ip));
        break;
    case SUB_BLOB_VALTYPE_ENUM:    
        nEnumVal = *(int*)pItem->pDataVal;
        // for(n = 0; n < pItem->nEnumTotal; n++)
        // {
        //     if(pItem->enumList[n].emVal == nEnumVal)
        //     {
        //         //snprintf(pszOut, nValLen, "%s", LoadLangText_ById(pItem->enumList[n].nEnumName_ResId));
        //         //snprintf(pszOut, nValLen, "%d", nEnumVal);
        //         break;
        //     }
        // }
        snprintf(pszOut, nValLen, "%d", nEnumVal);
        break;
    case SUB_BLOB_VALTYPE_BIT:
        if(pItem->nBits == 2)
        {
            usVal = *(USHORT*)pItem->pDataVal;
            usVal = (usVal >> pItem->nBitOffset);
            usVal &= 0x03;
            snprintf(pszOut, nValLen, "%u", usVal);
            TRACEX("---the RawVal->%u, parsedVal->%u---\n", *(USHORT*)pItem->pDataVal, usVal);
        }
        else if(pItem->nBits == 1)
        {
            if(pItem->nBitOffset < 8)
            {
                byVal = *(BYTE*)pItem->pDataVal;
                byVal = (byVal >> pItem->nBitOffset);
                
                byVal &= 0x01;
                snprintf(pszOut, nValLen, "%u", byVal);
                TRACEX("---the RawVal->%u, parsedVal->%u- nBitOffset=[%d]--\n", *(BYTE*)pItem->pDataVal, byVal, pItem->nBitOffset);
            }
            else
            {
                usVal = *(USHORT*)pItem->pDataVal;
                usVal = (usVal >> pItem->nBitOffset);
                usVal &= 0x0001;
                snprintf(pszOut, nValLen, "%u", usVal);
                TRACEX("---the RawVal->%u, usVal->%u- nBitOffset=[%d]--\n", *(USHORT*)pItem->pDataVal, usVal, pItem->nBitOffset);
            }
        }
        break;
    default:
        break;
    }
    return pszOut;
}

char* GetStrVal_FromWebBlob(A_WEBBLOB_PARAM* pBlob2Set, int nSubBlobIdx_2Set, char* pszOut, int nValLen)
{
    memset(pszOut, 0, nValLen);
    WEB_SUBBLOB_ITEM* pItem = GetSubBlobItem_FromSubBlobIdx(pBlob2Set, nSubBlobIdx_2Set);
    if(pItem == NULL)
    {
        TRACEX("---now we got a NULL subBlobItem---\n");
        return pszOut;
    }

    return GetStrVal_FromWebSubBlobVal(pItem, pszOut, nValLen);
}


BOOL Is_SubBlobParam_SetCtrlAble(WEB_SUBBLOB_ITEM* pSubBlobVal, int nUserLevel)
{
    if(!pSubBlobVal->bSetAble)
    {
        return FALSE;
    }

    if(nUserLevel == USER_LEVEL_ROOT)
    {
        return TRUE;
    }
    else if((nUserLevel == USER_LEVEL_ENGINEER) &&
        ((pSubBlobVal->uMaskAuthorityLevel & 0x02) != 0x02))
    {
        return TRUE;
    }
    else if((nUserLevel == USER_LEVEL_ADMIN) &&
        (((pSubBlobVal->uMaskAuthorityLevel >> 2) & 0x02) != 0x02))
    {
        return TRUE;
    }
    return FALSE;
}