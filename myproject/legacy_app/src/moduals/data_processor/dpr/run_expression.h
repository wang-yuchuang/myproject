#ifndef _RUN_EXPRESSION__
#define _RUN_EXPRESSION__
#include "std_sys.h"
#include "app_incs.h"

#ifdef _DEBUG
	int Exp_CalculateD(IN int nDevId, IN char *pSigName, 
		IN EXP_ELEMENT *pEvalExp, IN int nElements,
		OUT EXP_CONST *pResult,
		IN char *__function__, IN int __line__);
	#define Exp_Calculate(nDevUnitId, pSigName, pExp, nElements, pResult)	\
		Exp_CalculateD((nDevUnitId), (pSigName), (pExp), (nElements), (pResult), __FUNCTION__, __LINE__)

	void Exp_DisplayExpression(IN EXP_ELEMENT *pEvalExp, IN int nElements);
#else	// release version
	int Exp_CalculateR(IN int nDevId, IN EXP_ELEMENT *pEvalExp, IN int nElements, OUT EXP_CONST *pResult);
	#define Exp_Calculate(nDevUnitId, pSigName, pExp, nElements, pResult)	\
		Exp_CalculateR((nDevUnitId), (pExp), (nElements), (pResult))
#endif //_DEBUG

	void Debug_PringExpression(IN CLUSTER_EXP_ELEMENT *pExpSet, char* pBuff, int nBuffSize);

	enum TypesUsedByExp
	{
		EXP_NOT = 0,
		EXP_MUL,
		EXP_DIV,
		EXP_ADD,
		EXP_SUB,
		EXP_GREATER,
		EXP_LESS,
		EXP_GREATER_EQUAL,
		EXP_LESS_EQUAL,
		EXP_EQUAL,
		EXP_AND,
		EXP_OR,
		EXP_MAX_OPERATOR	,	// end of all operator

		// middle op used in parsing expression
		EXP_LF_BRACKET		,			/* '(' */
		EXP_RT_BRACKET		,			/* ')' */

		/* element type definition */
		EXP_ETYPE_VARIABLE	,
		EXP_ETYPE_CONST		,
		EXP_ETYPE_OPERATOR	,
		EXP_ETYPE_DUMMY		,

		// run time type.
		EXP_ETYPE_INVALID	,		// the EXP value is invalid. used in calculation. 
		EXP_ETYPE_ERROR		,	// the EXP expression is error when calculating. used in calculation
	};


	// macro to get the value from the value of EXP_ELEMENT which type is EXP_CONST 
#define EXP_SET_CONST_VALUE(pExpEle, v)		((*(EXP_CONST *)(pExpEle)->pElement) = (EXP_CONST)(v))
#define EXP_GET_CONST_VALUE(pExpEle)			((*(EXP_CONST *)(pExpEle)->pElement))
#define EXP_GET_CONST_INT(pExpEle)			((int)EXP_GET_CONST_VALUE(pExpEle))
#define EXP_GET_CONST_LONG(pExpEle)			((long)EXP_GET_CONST_VALUE(pExpEle))
#define EXP_GET_CONST_FLOAT(pExpEle)			((float)EXP_GET_CONST_VALUE(pExpEle))

#define EXP_IS_VAR(pVar)	(((pVar)->byElementType == EXP_ETYPE_VARIABLE) ||	\
	((pVar)->byElementType == EXP_ETYPE_VARIABLE_RAW))


int ExpressionParser(IN char *szBuf, OUT CLUSTER_EXP_ELEMENT *pExpSet);

#endif
