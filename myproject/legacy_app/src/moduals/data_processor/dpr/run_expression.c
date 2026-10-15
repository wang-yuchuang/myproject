#include <math.h>
#include <values.h>
#include "run_expression.h"
#include "dpr.h"

DEFINE_STACK(PEXP_ELEMENT);	// define the stack structure STACK_PEXP_ELEMENT

// return the pResult.
typedef PEXP_ELEMENT (*EXP_EVALUATION_PROC)(IN EXP_ELEMENT *pResult, IN int nDevId, 
							IN OUT STACK_PEXP_ELEMENT *pOpDataStack);

typedef struct _EXP_EVALUATION_PROC_LIST
{
#ifdef _DEBUG
	char					*name;
#endif
	int						nOpType;		// operator  Type
	EXP_EVALUATION_PROC		pfnCalc;		// evaluation Function
}EXP_EVALUATION_PROC_LIST;


#ifdef _DEBUG
#define DEF_EXP_EVAL_PROC_LIST(opr, proc)	\
	{ #opr, (opr), (EXP_EVALUATION_PROC)(proc)} 
// to check the stack status
#define POP_EXP_OPD(opd, pStack, pRes)		\
	do {																	\
		if (!STACK_SAFELY_POP((pStack), (opd)))								\
		{																	\
			TRACEX("Error, stack is empty when popup operand.\n");	\
			(pRes)->byElementType = EXP_ETYPE_ERROR; /*EXP_ETYPE_INVALID;*/	\
			return (pRes);													\
		}																	\
	} while(0)

#else	// release, it may not safe
#define DEF_EXP_EVAL_PROC_LIST(opr, proc)	\
		{ (opr), (EXP_EVALUATION_PROC)(proc)} 
#define POP_EXP_OPD(opd, pStack, pRes)		((opd)=STACK_POP(pStack))
#endif

static int GetVariableExpVal(int nDevId, EXP_VARIABLE* pExpVariable, EXP_CONST *pValue)
{
	PARAMETER *pParam = NULL;
	ALARM *pAlarm = NULL;
	BOOL bIsParam = FALSE;
	BOOL bIsAlarm = FALSE;
	
	if((pExpVariable->nParamID < 0) &&
		(PARAM_TYPE_STATUS == pExpVariable->nParamType))
	{
		DEVICE* pDev = GetDevce_ById(nDevId);
		if(pDev == NULL)
		{
			return EXP_ETYPE_INVALID;
		}
		if(pExpVariable->nParamID == EXP_PARAM_COMM_DEV)
		{
			*pValue = (pDev->emStatus == DEVICE_STATUS_COMM_FAIL) ? (EXP_CONST)1 : (EXP_CONST)0;
			TRACEX("Now get ExpVal for Dev:%d, Value:%.0f\n", pDev->info.nDeviceId, *pValue);
		}
		else if(pExpVariable->nParamID == EXP_PARAM_COMM_SAM)
		{
			*pValue = (pDev->info.pBelonged_Sampler->emWorkStatus == SAMPLER_IS_DISCON) ? (EXP_CONST)1 : (EXP_CONST)0;
		}
		else
		{
			return EXP_ETYPE_INVALID;
		}
		return EXP_ETYPE_CONST;
	}

	switch(pExpVariable->nParamType)
	{
	case EXP_PARAM_STATUS:
		pParam = GetParam_ByPR(PARAM_TYPE_STATUS, 
			(pExpVariable->nDeviceId == EXP_DEV_SELF) ? nDevId : pExpVariable->nDeviceId,
			pExpVariable->nParamID);
		bIsParam = TRUE;
		break;
	case EXP_PARAM_CONTRL:
		pParam = GetParam_ByPR(PARAM_TYPE_CONTROL, 
			(pExpVariable->nDeviceId == EXP_DEV_SELF) ? nDevId : pExpVariable->nDeviceId,
			pExpVariable->nParamID);
		bIsParam = TRUE;
		break;
	case EXP_PARAM_SETTING:
		pParam = GetParam_ByPR(PARAM_TYPE_SETTING, 
			(pExpVariable->nDeviceId == EXP_DEV_SELF) ? nDevId : pExpVariable->nDeviceId,
			pExpVariable->nParamID);
		bIsParam = TRUE;
		break;
	case EXP_PARAM_ALARM:
		pAlarm = GetAlarm_ByPR((pExpVariable->nDeviceId == EXP_DEV_SELF) ? nDevId : pExpVariable->nDeviceId,
			pExpVariable->nParamID);
		bIsAlarm = TRUE;
		break;
	default:
		break;
	}
	if(bIsParam)
	{
		if(pParam == NULL)
		{
			return EXP_ETYPE_INVALID;
		}
		//raw can only be status param
		VAR_VALUE* pVal = &pParam->currValue;
		switch(pParam->pParamInfo->emValueType)
		{
		case VALUE_TYPE_LONG:
			*pValue = (EXP_CONST)pVal->lValue;
			break;
		case VALUE_TYPE_ULONG:
		case VALUE_TYPE_DATETIME:
		case VALUE_TYPE_ENUM:
			*pValue = (EXP_CONST)pVal->ulValue;
			break;
		case VALUE_TYPE_FLOAT:
			*pValue = (EXP_CONST)pVal->fValue;
			break;
		default:
			return EXP_ETYPE_INVALID; //string and blob can not be used in Expression
		}
	}
	else if(bIsAlarm)
	{
		if(pAlarm == NULL)
		{
			return EXP_ETYPE_INVALID;
		}
		*pValue = (pAlarm->emStatus == ALARM_STATUS_ACTIVE) ? (EXP_CONST)1 : (EXP_CONST)0;
	}
	else
	{
		return EXP_ETYPE_INVALID;
	}

	return EXP_ETYPE_CONST;
}

static int Eval_GetElementValue(IN int nDevId, IN EXP_ELEMENT *pVal, OUT EXP_CONST *pValue)
{
	EXP_VARIABLE* pExpVariable;

	if(pVal->byElementType == EXP_ETYPE_VARIABLE)
	{
		pExpVariable = (EXP_VARIABLE *)pVal->pElement;
		return GetVariableExpVal(nDevId, pExpVariable, pValue);		
	}
	else if(pVal->byElementType == EXP_ETYPE_CONST)
	{
		*pValue = *(EXP_CONST *)pVal->pElement;
		return EXP_ETYPE_CONST;
	}

	return EXP_ETYPE_INVALID;
}

static PEXP_ELEMENT Eval_EXP_NOT(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST	val2;

	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);	//no use

	pResult->byElementType = (BYTE)Eval_GetElementValue(nDevId, opd2, &val2);

	// the value is ok
	if (pResult->byElementType == EXP_ETYPE_CONST)
	{
		*(EXP_CONST*)pResult->pElement = 
			(EXP_CONST)(FLOAT_EQUAL0((float)val2) ? 1 : 0);
	}

	return pResult;
}

static PEXP_ELEMENT Eval_EXP_MUL(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST	val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);

	if ((Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		*(EXP_CONST*)pResult->pElement =
			(EXP_CONST)((float)val1 * (float)(val2));
		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}

static PEXP_ELEMENT Eval_EXP_DIV(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST	val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);	

	// get the opd value
	if ((Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		if (FLOAT_EQUAL0((float)val2))		// divided by zero?
		{
			if (FLOAT_NOT_EQUAL0((float)val1))	// 0/0 ? = 0
			{
				*(EXP_CONST *)pResult->pElement = (EXP_CONST)0;
			}
			else
			{
				//printf("[Eval_EXP_DIV] -- %f divided by zero(%f).\n",
				//	(float)val1, (float)val2);
				*(EXP_CONST *)pResult->pElement = (EXP_CONST)MAXFLOAT;
			}
		}
		else
		{
			*(EXP_CONST *)pResult->pElement =
				(EXP_CONST)((float)val1 / (float)(val2));
		}

		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}


static PEXP_ELEMENT Eval_EXP_ADD(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST		val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);	

	if ((Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		*(EXP_CONST *)pResult->pElement =
			(EXP_CONST)((float)val1 + (float)(val2));
		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}


static PEXP_ELEMENT Eval_EXP_SUB(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST		val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);	

	if ((Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		*(EXP_CONST *)pResult->pElement =
			(EXP_CONST)((float)val1 - (float)(val2));
		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}


static PEXP_ELEMENT Eval_EXP_GREATER(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST	val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);

	if ((Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		*(EXP_CONST *)pResult->pElement = 
			(EXP_CONST)(((float)val1 > (float)(val2)) ? 1 : 0);
		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}


static PEXP_ELEMENT Eval_EXP_LESS(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST		val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);	

	if ((Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		*(EXP_CONST *)pResult->pElement =
			(EXP_CONST)(((float)val1 < (float)(val2)) ? 1 : 0);
		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}


static PEXP_ELEMENT Eval_EXP_GREATER_EQUAL(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST		val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);	

	if ((Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		if (FLOAT_EQUAL((float)val1, (float)val2))
		{
			*(EXP_CONST *)pResult->pElement = (EXP_CONST)1;
		}
		else
		{
			*(EXP_CONST *)pResult->pElement = 
				(EXP_CONST)(((float)val1 > (float)(val2)) ? 1 : 0);
		}

		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}


static PEXP_ELEMENT Eval_EXP_LESS_EQUAL(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST		val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);	

	if ((Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		if (FLOAT_EQUAL((float)val1, (float)val2))
		{
			*(EXP_CONST *)pResult->pElement = (EXP_CONST)1;
		}
		else
		{
			*(EXP_CONST *)pResult->pElement = 
				(EXP_CONST)(((float)val1 < (float)(val2)) ? 1 : 0);
		}

		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}

static PEXP_ELEMENT Eval_EXP_EQUAL(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST		val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);	

	if ((Eval_GetElementValue(nDevId,opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		//TRACEX("The EEEEEEqulExp var1.float= [%.1f],var2.float = [%.1f]. \n", (float)val1, (float)val2);
		*(EXP_CONST *)pResult->pElement = 
			(EXP_CONST)((FLOAT_EQUAL((float)val1, (float)val2)) ? 1 : 0);
		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}

static PEXP_ELEMENT Eval_EXP_AND(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST		val1, val2;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);

	if ((Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST) &&
		(Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST))
	{
		*(EXP_CONST *)pResult->pElement = 
			(EXP_CONST)(FLOAT_NOT_EQUAL0((float)val1) &&
			FLOAT_NOT_EQUAL0((float)val2));

		pResult->byElementType = EXP_ETYPE_CONST;
	}
	else
	{
		pResult->byElementType = EXP_ETYPE_INVALID;
	}

	return pResult;
}

static PEXP_ELEMENT Eval_EXP_OR(IN PEXP_ELEMENT pResult, IN int nDevId, IN OUT STACK_PEXP_ELEMENT *pOpDataStack)
{	
	PEXP_ELEMENT	opd1, opd2;
	EXP_CONST		val1, val2;
	int				nInvalidOpds = 0;
	
	POP_EXP_OPD(opd2, pOpDataStack, pResult);
	POP_EXP_OPD(opd1, pOpDataStack, pResult);	

	// get the opd value
	// for OR operator, to test the opd1 at first, if its value is TRUE, 
	// set TRUE to result, or to test the opd2.
	pResult->byElementType = EXP_ETYPE_CONST;

	if (Eval_GetElementValue(nDevId, opd1, &val1) == EXP_ETYPE_CONST)
	{
		if (FLOAT_NOT_EQUAL0((float)val1))
		{
			*(EXP_CONST *)pResult->pElement = 1;	
			return pResult;
		}
	}
	else
	{
		nInvalidOpds++;
	}

	// to check opd2
	if (Eval_GetElementValue(nDevId, opd2, &val2) == EXP_ETYPE_CONST)
	{
		if (FLOAT_NOT_EQUAL0((float)val2))
		{
			*(EXP_CONST *)pResult->pElement = 1;	
			return pResult;
		}
	}
	else
	{
		nInvalidOpds++;
	}

	// check both are invalid opds?
	if (nInvalidOpds != 2)	// not all, set 0 to result
	{
		*(EXP_CONST *)pResult->pElement = 0;		
		return pResult;
	}

	// invalid result.
	pResult->byElementType = EXP_ETYPE_INVALID;
	
	return pResult;
}


static EXP_EVALUATION_PROC_LIST	s_pExpEvalProc[] = 
{
	DEF_EXP_EVAL_PROC_LIST(EXP_NOT,				Eval_EXP_NOT			),
	DEF_EXP_EVAL_PROC_LIST(EXP_MUL,				Eval_EXP_MUL			),
	DEF_EXP_EVAL_PROC_LIST(EXP_DIV,				Eval_EXP_DIV			),
	DEF_EXP_EVAL_PROC_LIST(EXP_ADD,			Eval_EXP_ADD			),
	DEF_EXP_EVAL_PROC_LIST(EXP_SUB,				Eval_EXP_SUB			),
	DEF_EXP_EVAL_PROC_LIST(EXP_GREATER, 		Eval_EXP_GREATER		),
	DEF_EXP_EVAL_PROC_LIST(EXP_LESS,			Eval_EXP_LESS			),
	DEF_EXP_EVAL_PROC_LIST(EXP_GREATER_EQUAL,	Eval_EXP_GREATER_EQUAL),
	DEF_EXP_EVAL_PROC_LIST(EXP_LESS_EQUAL,		Eval_EXP_LESS_EQUAL	),
	DEF_EXP_EVAL_PROC_LIST(EXP_EQUAL,			Eval_EXP_EQUAL		),
	DEF_EXP_EVAL_PROC_LIST(EXP_AND,			Eval_EXP_AND			),
	DEF_EXP_EVAL_PROC_LIST(EXP_OR,				Eval_EXP_OR			),
	// end of all operator
};

#define MAX_IMPLEMENTED_OP		ITEM_OF(s_pExpEvalProc)

#define OPERATOR_IS_VALID(opr)	(((opr) < MAX_IMPLEMENTED_OP) &&		\
								(s_pExpEvalProc[(opr)].pfnCalc != NULL) &&	\
								(s_pExpEvalProc[(opr)].nOpType == (opr)))


static int Exp_StringElement(IN EXP_ELEMENT *pEle, OUT char *pszEleBuf, IN size_t nEleLen)
{
	int			nLen;
	char		*pszOperator;
	EXP_VARIABLE	*pExpVar;

	switch(pEle->byElementType)
	{
	case EXP_ETYPE_VARIABLE:
		pExpVar = (EXP_VARIABLE *)pEle->pElement;
		nLen = snprintf(pszEleBuf, nEleLen, "[%d,%d,%d]",
			pExpVar->nDeviceId, 
			pExpVar->nParamType, 
			pExpVar->nParamID);

		return nLen;

	case EXP_ETYPE_CONST:
		nLen = snprintf(pszEleBuf, nEleLen, "[C=%1.3f]",
			*((EXP_CONST *)pEle->pElement));
		return nLen;

	case EXP_ETYPE_OPERATOR:
		switch(((EXP_OPERATOR *)pEle->pElement)->byOperatorType)
		{
		case EXP_ADD:
			pszOperator = "[+]";
			break;

		case EXP_SUB:
			pszOperator = "[-]";
			break;

		case EXP_MUL:
			pszOperator = "[*]";
			break;

		case EXP_DIV:
			pszOperator = "[/]";
			break;

		case EXP_GREATER:
			pszOperator = "[>]";
			break;

		case EXP_LESS:
			pszOperator = "[<]";
			break;

		case EXP_GREATER_EQUAL:
			pszOperator = "[>=]";
			break;

		case EXP_LESS_EQUAL:
			pszOperator = "[<=]";
			break;

		case EXP_EQUAL:
			pszOperator = "[=]";
			break;

		case EXP_AND:
			pszOperator = "[&]";
			break;

		case EXP_OR:
			pszOperator = "[|]";
			break;

		case EXP_NOT:
			pszOperator = "[!]";
			break;

		default:
			pszOperator = "[E_OPERATOR]";
		}

		break;

	case EXP_ETYPE_DUMMY:
		pszOperator = "[E_DUMMY]";
		break;

	case EXP_ETYPE_INVALID:
		pszOperator = "[E_INVALID]";
		break;

	default:
		pszOperator = "[E_TYPE]";
	}


	nLen = snprintf(pszEleBuf, nEleLen, "%s", pszOperator);

	return nLen;
}


static char *Exp_StringExpression(IN EXP_ELEMENT *pEvalExp, IN int nElements, OUT char *pszExpStr, IN int nExpStrLen)
{
	int					i;
	EXP_ELEMENT			*pExp = pEvalExp;

	char					*pCurExpBuf = pszExpStr;
	int					nCurExpStrLen = nExpStrLen;
	int					nLen;

	*pCurExpBuf = 0;

	for (i = 0; (i < nElements) && (nCurExpStrLen > 0); i++, pExp++)
	{
		nLen = Exp_StringElement(pExp, pCurExpBuf, (size_t)nCurExpStrLen);

		pCurExpBuf    += nLen;
		nCurExpStrLen -= nLen;

		if (nCurExpStrLen <= 0)	// < 0 impossible.
		{
			TRACEX("The buffer is too small on stringlizing the exp.\n");
			break;
		}
	}

	return pszExpStr;
}


#ifdef _DEBUG

void Exp_DisplayExpression(IN EXP_ELEMENT *pEvalExp, IN int nElements)
{
	char	szBuf[512];

	Exp_StringExpression(pEvalExp, nElements, szBuf, sizeof(szBuf));

	//printf("Exp(%p) %d elements: %s\n", pEvalExp, nElements, szBuf);
}
#endif //_DEBUG

void Debug_PringExpression(IN CLUSTER_EXP_ELEMENT *pExpSet, char* pBuff, int nBuffSize)
{
	Exp_StringExpression(pExpSet->p, pExpSet->nTotal, pBuff, nBuffSize);
	//printf("Exp(%p) %d elements: %s\n", pExpSet->p, pExpSet->nTotal, pBuff);
}


#ifdef _DEBUG
int Exp_CalculateD(IN int nDevId, IN char *pSigName, 
					IN EXP_ELEMENT *pEvalExp, IN int nElements,
					OUT EXP_CONST *pResult,
					IN char *__function__, IN int __line__)
#else
int Exp_CalculateR(IN int nDevId, IN EXP_ELEMENT *pEvalExp, IN int nElements,
				   OUT EXP_CONST *pResult)
#endif //_DEBUG
{
	PEXP_ELEMENT			pStackBuffer[EXP_MAX_ELEMENTS];
	STACK_PEXP_ELEMENT	workStack;

	EXP_CONST			pMidResultValue[EXP_MAX_ELEMENTS];	// mid result.
	EXP_ELEMENT			pMidResultBuffer[EXP_MAX_ELEMENTS];
	PEXP_ELEMENT			pMidResult;

	int					op;
	int					i;
	EXP_ELEMENT			*pExp = pEvalExp;

#ifdef _DEBUG
	BOOL				bNoResult = FALSE;
	//TRACEX("Calculating exp=%p, n=%d...\n",
	//	pExp, nElements);
#endif

	if((pExp == NULL) || (nElements <= 0))
	{
		return EXP_ETYPE_ERROR;
	}
	else if(pExp->byElementType == EXP_ETYPE_ERROR)
	{
		return EXP_ETYPE_ERROR;
	}

	STACK_INIT(&workStack, pStackBuffer, ITEM_OF(pStackBuffer));

	for (i = 0; i < nElements; i++, pExp++)
	{
		// push the operand to stack till reach a operator
		if (pExp->byElementType != EXP_ETYPE_OPERATOR)
		{
			STACK_PUSH(&workStack, pExp);
		}
		else // to eval
		{
			// make the middle result
			pMidResult			 = &pMidResultBuffer[i];
			pMidResult->pElement = &pMidResultValue[i];

			op = ((EXP_OPERATOR*)pExp->pElement)->byOperatorType;
			//TRACEX("The op = [%d].\n", op);
			if (OPERATOR_IS_VALID(op))
			{
				// eval
				s_pExpEvalProc[op].pfnCalc(pMidResult, nDevId, &workStack);
			}
			else
			{
				pMidResult->byElementType = EXP_ETYPE_INVALID;

#ifdef _DEBUG
				TRACEX("[%s:%d %s] Meet an invalid operator %d.\n", 
					__function__, __line__, pSigName, op);
#endif
			}

			if ((pMidResult->byElementType != EXP_ETYPE_ERROR) &&
				(pMidResult->byElementType != EXP_ETYPE_INVALID))
			{
				// push the middle result to stack
				STACK_PUSH(&workStack, pMidResult);
			}
			else
			{
#ifdef _DEBUG
				TRACEX("Error on calc the No%d op=%s(%d) of exp(%p, %d ele) DevID=[%d] SigName=[%s].\n", 
					i, s_pExpEvalProc[op].name, op, 
					pEvalExp, nElements, nDevId, (pSigName==NULL)?"NULL":pSigName);
				Exp_DisplayExpression(pEvalExp, nElements);
#endif
				return EXP_ETYPE_ERROR;
			}
		}
	}

	// get the result.
	if (!STACK_IS_EMPTY(&workStack))
	{
		// popup result. copy the result. 
		pMidResult = STACK_POP(&workStack);

		// check the stack. if it now is empty, the exp is correct.
		if ((pMidResult->byElementType != EXP_ETYPE_OPERATOR) &&
			STACK_IS_EMPTY(&workStack))
		{
//#ifdef _DEBUG
//			TRACEX("The following exp(%p, %d ele) got: %1.3f.\n", 
//				pEvalExp, nElements, *(float *)(pResult->pElement));
//			Exp_DisplayExpression(pEvalExp, nElements, TRUE);
//#endif

			return Eval_GetElementValue(nDevId, pMidResult, pResult);
		}
	}

#ifdef _DEBUG
	else
	{
		bNoResult = TRUE;
	}

	// show error msg and the exp
	TRACE("[%s:%d %s] meets error on calc exp: ",
		__function__, __line__, pSigName);

	Exp_DisplayExpression(pEvalExp, nElements);
	
	if (bNoResult)
	{
		TRACE("Invalid expression, no result in stack!\n");
	}

	else if (pMidResult->byElementType == EXP_ETYPE_OPERATOR)
	{
		TRACE("Invalid expression, the result type is an"
			"operator, not const or invalid!\n");
	}

	else if(!STACK_IS_EMPTY(&workStack))
	{
		TRACE("Invalid expression, the stack is not empty after "
			"popping result!\n");
	}

	//TRACEX("The following exp(%p, %d ele) got: %1.3f.\n", 
	//	pEvalExp, nElements, *(float *)(pResult->pElement));
	Exp_DisplayExpression(pEvalExp, nElements);

#endif

	return EXP_ETYPE_ERROR;
}

static BOOL Create_ExpOperator(EXP_ELEMENT **pElement, BYTE byOp, BYTE byPrio)
{
	EXP_OPERATOR *pExpOperator;
	pExpOperator = NEW(EXP_OPERATOR, 1);
	if (pExpOperator == NULL)
	{
		return FALSE;
	}
	pExpOperator->byOperatorType = byOp;
	pExpOperator->byPriority = byPrio;
	*pElement = NEW(EXP_ELEMENT, 1);
	if(*pElement == NULL)
	{
		SAFELY_DELETE(pExpOperator);
		return FALSE;
	}
	(*pElement)->byElementType = EXP_ETYPE_OPERATOR;
	(*pElement)->pElement = pExpOperator;
	return TRUE;
}

static int Create_ExpVariable(EXP_ELEMENT **pElement, char *pVariableString, char* pEnd)
{
	EXP_VARIABLE *pExpVariable;
	char* pField;

	/* validation check, element should be 3 */
	if (3 != GetSplitStringCount(pVariableString, COMMA))
	{
		TRACEX("count is: %d\n", GetSplitStringCount(pVariableString, COMMA));
		return CFG_ERR_BADCONFIG;
	}

	pExpVariable = NEW(EXP_VARIABLE, 1);
	if(pExpVariable == NULL)
	{
		return CFG_ERR_NO_MEMORY;
	}

	/* get DevId value */
	pVariableString = SplitString(pVariableString, &pField, COMMA);
	if (pField >= pEnd)			
	{
		return CFG_ERR_BADCONFIG;					/* expression is invalid */
	}
	else if(*pField == '$')
	{
		pExpVariable->nDeviceId = EXP_DEV_SELF;
	}
	else if(*pField < '0' || *pField > '9')
	{
		return CFG_ERR_BADCONFIG;					/* expression is invalid */
	}
	else
	{
		sscanf(pField, "%d", &pExpVariable->nDeviceId);
	}
	

	/* get ParamType value */
	pVariableString = SplitString(pVariableString, &pField, ',');
	if ((pField >= pEnd) || (*pField < '0') || (*pField > '9'))
	{
		return CFG_ERR_BADCONFIG;					/* expression is invalid */
	}
	sscanf(pField, "%d", &pExpVariable->nParamType);
	if((pExpVariable->nParamType < EXP_PARAM_STATUS) ||
		(pExpVariable->nParamType > EXP_PARAM_ALARM))
	{
		return CFG_ERR_BADCONFIG;
	}
	/* get ParamId value */
	pVariableString = SplitString(pVariableString, &pField, ',');
	if ((pField >= pEnd) || (*pField < '0') || (*pField > '9'))
	{
		if(*pField != '-')
		{
			return CFG_ERR_BADCONFIG;					/* expression is invalid */
		}
		TRACEX("------------Found wired paramID --> [%s]-----------\n", pField);
	}
	sscanf(pField, "%d", &pExpVariable->nParamID);
	if((pExpVariable->nParamID < 0) && 
		(pExpVariable->nParamID != EXP_PARAM_COMM_SAM) &&
		(pExpVariable->nParamID != EXP_PARAM_COMM_DEV))
	{
		return CFG_ERR_BADCONFIG;
	}
	*pElement = NEW(EXP_ELEMENT, 1);
	if (*pElement == NULL)
	{
		SAFELY_DELETE(pExpVariable);
		return CFG_ERR_NO_MEMORY;
	}
	(*pElement)->byElementType = EXP_ETYPE_VARIABLE;
	(*pElement)->pElement = pExpVariable;
	return CFG_ERR_OK;
}

//-------------------------------------------Expression Parser---------------------------
static int Parse_ExpElement(BOOL bLastOpe, IN OUT char **szBuf, OUT EXP_ELEMENT **pElement)
{
	EXP_CONST *pConstValue;
	int nRet;
	char *pVariableString;
	char *pConstString;				/* for const element parse */
	/* for const element check, '.' can be used only once */
	BOOL bDotUsed = FALSE;

	ASSERT(szBuf);
	ASSERT(*szBuf);

	*pElement = NULL;
	*szBuf = RemoveStringWhiteSpace(*szBuf);

	if (**szBuf == '\0')	
	{
		return CFG_ERR_OK;
	}

	switch (**szBuf)
	{
	case '+':
		(*szBuf)++;
		if(!Create_ExpOperator(pElement, EXP_ADD, 3))
			return CFG_ERR_NO_MEMORY;
		break;
	case '*':
		(*szBuf)++;
		if(!Create_ExpOperator(pElement, EXP_MUL, 2)) 
			return CFG_ERR_NO_MEMORY;
		break;
	case '/':
		(*szBuf)++;
		if(!Create_ExpOperator(pElement, EXP_DIV, 2))
			return CFG_ERR_NO_MEMORY;
		break;
	case '&':
		(*szBuf)++;
		if(!Create_ExpOperator(pElement, EXP_AND, 6))
			return CFG_ERR_NO_MEMORY;
		break;
	case '|':
		(*szBuf)++;
		if(!Create_ExpOperator(pElement, EXP_OR, 7))
			return CFG_ERR_NO_MEMORY;
		break;
	case '!':
		(*szBuf)++;
		*pElement = NEW(EXP_ELEMENT, 1);
		if (*pElement == NULL)
		{
			return CFG_ERR_NO_MEMORY;
		}
		(*pElement)->byElementType = EXP_ETYPE_DUMMY;
		(*pElement)->pElement = NULL;
		break;

	case '>':
		(*szBuf)++;
		*szBuf = RemoveStringWhiteSpace(*szBuf);
		if(**szBuf == '=')
		{
			(*szBuf)++;
			if(!Create_ExpOperator(pElement, EXP_GREATER_EQUAL, 4))
				return CFG_ERR_NO_MEMORY;
		}
		else
		{
			if(!Create_ExpOperator(pElement, EXP_GREATER, 4))
				return CFG_ERR_NO_MEMORY;
		}
		break;
	case '<':
		(*szBuf)++;
		*szBuf = RemoveStringWhiteSpace(*szBuf);
		if(**szBuf == '=')
		{
			(*szBuf)++;
			if(!Create_ExpOperator(pElement, EXP_LESS_EQUAL, 4))
				return CFG_ERR_NO_MEMORY;
		}
		else
		{
			if(!Create_ExpOperator(pElement, EXP_LESS, 4))
				return CFG_ERR_NO_MEMORY;
		}
		break;
	case '=':
		(*szBuf)++;
		*szBuf = RemoveStringWhiteSpace(*szBuf);
		if (**szBuf == '=')
		{
			(*szBuf)++;
			if(!Create_ExpOperator(pElement, EXP_EQUAL, 5))
				return CFG_ERR_NO_MEMORY;
		}
		else
		{
			return CFG_ERR_BADCONFIG;
		}
		break;
	case '(':				 
		(*szBuf)++;
		if(!Create_ExpOperator(pElement, EXP_LF_BRACKET, (BYTE)-1))
			return CFG_ERR_NO_MEMORY;
		break;
	case ')':
		(*szBuf)++;
		if(!Create_ExpOperator(pElement, EXP_RT_BRACKET, (BYTE)-1))
			return CFG_ERR_NO_MEMORY;
		break;
	case '[':
		pVariableString = *szBuf;
		while (*(*szBuf) != '\0' && *(*szBuf) != ']')
		{
			(*szBuf)++;
		}
		if (*(*szBuf) != ']')			/* expression is invalid */
		{
			return CFG_ERR_BADCONFIG;
		}

		**szBuf = '\0';
		(*szBuf)++;

		pVariableString = Trim_String(pVariableString, '[');
		pVariableString = Trim_String(pVariableString, ']');
		nRet = Create_ExpVariable(pElement, pVariableString, *szBuf);
		if(nRet != CFG_ERR_OK)
			return nRet;
		break;
	case '-':
		if (!bLastOpe)  //if the last element is Operator, take it as const
		{
			(*szBuf)++;
			if(Create_ExpOperator(pElement, EXP_SUB, 3))
				return CFG_ERR_NO_MEMORY;
			break;
		}
		// if bLastOpe == TRUE, go to const process

	default:					/* should be const */
		pConstString = *szBuf;

		if (!(((**szBuf >= '0') && (**szBuf <= '9')) || (**szBuf == '-')))
		{
			return CFG_ERR_BADCONFIG;
		}
		(*szBuf)++;
		while (((**szBuf >= '0') && (**szBuf <= '9')) || 
			((!bDotUsed) && (**szBuf == '.'))) 
		{
			if (**szBuf == '.')
			{
				bDotUsed = TRUE;
			}

			(*szBuf)++;
		}
		pConstValue = NEW(EXP_CONST, 1);
		if (pConstValue == NULL)
		{
			return CFG_ERR_NO_MEMORY;
		}
		sscanf(pConstString, "%f", pConstValue);

		*pElement = NEW(EXP_ELEMENT, 1);
		if (*pElement == NULL)
		{
			SAFELY_DELETE(pConstValue);
			return CFG_ERR_NO_MEMORY;
		}
		(*pElement)->byElementType = EXP_ETYPE_CONST;
		(*pElement)->pElement = pConstValue;
		break;
	}

	return CFG_ERR_OK;
}

static int Parse_Expression(IN char *szBuf, OUT int *piExpLen, OUT EXP_ELEMENT **pExpression)
{
	int iLen,ret;				/* iLen: element count of the expression */
	EXP_ELEMENT *pEle = NULL;
	EXP_ELEMENT *pEleTmp = NULL;
	EXP_OPERATOR *pOperator = NULL;
	EXP_OPERATOR *pOperatorTmp = NULL; 
	EXP_ELEMENT *pOpEleStack[20];	/* stack of operator element */
	int iStackTop = 0;				/* top of operator element stack */

	BOOL bLastOpe = FALSE;  /* if the last element type is operator, set it to TRUE, for supporting negative const date */

	char *p = szBuf;
	iLen = 0;

	if (szBuf[0] == '\0')
	{
		*piExpLen = 0;
		return CFG_ERR_OK;
		//return CFG_ERR_BADCONFIG;
	}

	ret = Parse_ExpElement(bLastOpe, &p, &pEle);
	while (pEle != NULL)
	{
		bLastOpe = FALSE;  //reset to FALSE

		switch(pEle->byElementType)
		{
		case EXP_ETYPE_VARIABLE:		/* process by next section */
		case EXP_ETYPE_CONST:
			iLen++;
			if (iLen > EXP_MAX_ELEMENTS)
			{
				return CFG_ERR_BADCONFIG;
			}
			pExpression[iLen-1] = pEle;
			break;

		case EXP_ETYPE_DUMMY:			/* for dummy element, we should create '!' element*/
			iLen++;
			if (iLen > EXP_MAX_ELEMENTS)
			{
				/* EXP_MAX_ELEMENTS define is not big enough! */
				return CFG_ERR_BADCONFIG;
			}
			pExpression[iLen-1] = pEle;
			if(!Create_ExpOperator(pEle, EXP_NOT, 1))
			{
				return CFG_ERR_NO_MEMORY;
			}
			//Notice:Do not break, just continue!!!!!!!!
		case EXP_ETYPE_OPERATOR:
			pOperator = (EXP_OPERATOR *)pEle->pElement;
			if (pOperator->byOperatorType == EXP_LF_BRACKET)
			{
				pOpEleStack[iStackTop] = pEle;
				iStackTop++;
				break;
			}
			else if (pOperator->byOperatorType == EXP_RT_BRACKET)
			{
				SAFELY_DELETE(pOperator);
				SAFELY_DELETE(pEle);				/* free memory for ')' element */
				if (iStackTop == 0)
				{
					return CFG_ERR_BADCONFIG;
				}
				pEleTmp = pOpEleStack[--iStackTop];
				pOperatorTmp = (EXP_OPERATOR *)(pEleTmp->pElement);
				while (pOperatorTmp->byOperatorType != EXP_LF_BRACKET && 
					iStackTop >0 )
				{
					iLen++;
					if (iLen > EXP_MAX_ELEMENTS)
					{
						/* EXP_MAX_ELEMENTS define is not big enough! */
						return CFG_ERR_BADCONFIG;
					}
					pExpression[iLen-1] = pEleTmp;
					pEleTmp = pOpEleStack[--iStackTop];
					pOperatorTmp = (EXP_OPERATOR *)(pEleTmp->pElement);
				}
				if (pOperatorTmp->byOperatorType != EXP_LF_BRACKET )
				{
					return CFG_ERR_BADCONFIG;		/* should be '(' */
				}
				SAFELY_DELETE(pEleTmp->pElement);
				SAFELY_DELETE(pEleTmp);		/* free memory for '(' element */
				break;
			}
			else
			{
				bLastOpe = TRUE; 
				if (iStackTop != 0 )
				{
					pEleTmp = pOpEleStack[--iStackTop];
					pOperatorTmp = (EXP_OPERATOR *)pEleTmp->pElement;
					while ((pOperatorTmp->byPriority <= pOperator->byPriority) && 
						pOperatorTmp->byOperatorType != EXP_LF_BRACKET && 
						iStackTop > 0)
					{
						iLen++;
						if (iLen > EXP_MAX_ELEMENTS)
						{
							/* EXP_MAX_ELEMENTS define is not big enough! */
							return CFG_ERR_BADCONFIG;
						}
						pExpression[iLen-1] = pEleTmp;
						pEleTmp = pOpEleStack[--iStackTop];
						pOperatorTmp = (EXP_OPERATOR *)pEleTmp->pElement;
					}
					if (pOperatorTmp->byPriority <= pOperator->byPriority && 
						pOperatorTmp->byOperatorType != EXP_LF_BRACKET)
					{						/* stop because iStackTop == 0 */
						iLen++;
						if (iLen > EXP_MAX_ELEMENTS)
						{
							/* EXP_MAX_ELEMENTS define is not big enough! */
							return CFG_ERR_BADCONFIG;
						}
						pExpression[iLen-1] = pEleTmp;
						iStackTop--; 
					}
					pOpEleStack[++iStackTop] = pEle;
					iStackTop++;
				}
				else
				{
					pOpEleStack[iStackTop++] = pEle;
				}
			}
		}

		ret = Parse_ExpElement(bLastOpe, &p, &pEle);
	}

	if (ret != CFG_ERR_OK)
	{
		return ret;
	}

	if (iStackTop != 0)
	{
		while ( --iStackTop >= 0 )
		{
			iLen++;
			if (iLen > EXP_MAX_ELEMENTS)
			{
				/*TRACE("EXP_MAX_ELEMENTS define is not big enough!");*/
				return CFG_ERR_BADCONFIG;
			}
			pExpression[iLen-1] = pOpEleStack[iStackTop];
		}
	}

	*piExpLen = iLen;

	return CFG_ERR_OK;
}

int ExpressionParser(IN char *szBuf, OUT CLUSTER_EXP_ELEMENT *pExpSet)
{
	EXP_ELEMENT **pExpression = NEW(EXP_ELEMENT *, EXP_MAX_ELEMENTS);
	int i;
	if (pExpression == NULL)
	{
		return CFG_ERR_NO_MEMORY;
	}
	ZERO_POBJS(pExpression, EXP_MAX_ELEMENTS);

	int ret = Parse_Expression(szBuf, (int*)&pExpSet->nTotal, pExpression);
	if(ret == CFG_ERR_OK)
	{
		//Now copy out the parsed Expression
		pExpSet->p = NEW(EXP_ELEMENT, pExpSet->nTotal);
		if(pExpSet->p != NULL)
		{
			for (i = 0; i < pExpSet->nTotal; i++)
			{
				pExpSet->p[i] = *(pExpression[i]);
				DELETE(pExpression[i]);	// Do NOT release the content--pElement,
				// which is still used by pExpSet->p[i];
			}

			DELETE(pExpression);
			return CFG_ERR_OK;
		}
		else
		{
			ret = CFG_ERR_NO_MEMORY;
		}
	}

	//free the memory
	EXP_ELEMENT *pEle;
	pExpSet->nTotal = 0;
	for (i = 0; i < EXP_MAX_ELEMENTS; i++)
	{
		if (pExpression[i] != NULL)
		{
			pEle = pExpression[i];
			SAFELY_DELETE(pEle->pElement);
			SAFELY_DELETE(pEle);
		}
	}
	SAFELY_DELETE(pExpression);

	return ret;
}