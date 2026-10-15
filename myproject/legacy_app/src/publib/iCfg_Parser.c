/*==========================================================================*
 *    Copyright(c) 2016,InfyPower Co., Ltd.
 *                     ALL RIGHTS RESERVED
 * 
 *  FILENAME : cfg_loader.c
 *  CREATOR  : Jimmy Wu                   DATE: 2016-07-25
 *  VERSION  : V1.00
 *  PURPOSE  : based on Thomas Lin
 *
 *  HISTORY  :
 *==========================================================================*/

#include "stdsys.h"
#include "public.h"

//#define FILE_END_WITH_0     1



#ifndef FILE_END_WITH_0
    #define END_OF_PROF(prof)   ( (prof)->pReadPtr == (prof)->pEndFile )
#else 
    #define END_OF_PROF(prof)   ( *(prof)->pReadPtr == 0 )  	/* FILE_END_WITH_0 */
#endif 

#define CR      (0x0D)
#define LF      (0x0A)
#define SPACE   (0x20)
#define TAB     (0x09)
#define COMMENT_FLAG        "#"

#define IS_WHITE_SPACE(c)         ( ((c) == SPACE) || ((c) == TAB) )
#define END_OF_LINE(c)      ( ((c) == CR) || ((c) == LF) )

#define SECT_PREFIX     '['
#define SECT_SUFFIX     ']'

void *Cfg_ProfileOpen(IN char *szFileContents, IN int nFileSize)
{
	SProfile    *pProf = NULL;

#ifndef FILE_END_WITH_0
	if ((szFileContents != NULL) && (nFileSize > 0))
#else
	if (szFileContents != NULL)
#endif
	{
		pProf = NEW(SProfile, 1);

		if (pProf == NULL)
		{
			return NULL;
		}

        pProf->pFileBuffer = szFileContents;
#ifndef FILE_END_WITH_0
        pProf->pEndFile    = szFileContents + nFileSize;
#endif 	
        pProf->pReadPtr    = pProf->pFileBuffer;
    }

    return (void *)pProf;
}

void Cfg_ProfileRewind(IN void *pProf)
{
    ((SProfile *)pProf)->pReadPtr = ((SProfile *)pProf)->pFileBuffer;
}

long Cfg_ProfileTell(IN void *pProf)
{
	SProfile *pFile;
	long     lCurPos;

	pFile = (SProfile *)pProf;

	lCurPos = pFile->pReadPtr - pFile->pFileBuffer;
	return lCurPos;
}

void Cfg_ProfileClose(IN void *pProf)
{
	SProfile *p;

	p = (SProfile *)pProf;

    if (p != NULL)
    {
		if (p->pFileBuffer != NULL)
		{
			/* the memory is newed in Cfg_LoadConfigFile */
			DELETE(p->pFileBuffer); 
		}
        DELETE(p);
    }
}

static void Cfg_SkipWhiteSpace(IN SProfile *pProf)
{
    while (!END_OF_PROF(pProf) &&               // not end of file
			(END_OF_LINE(*pProf->pReadPtr) ||      // is CR or LF
				IS_WHITE_SPACE(*pProf->pReadPtr)))  // or is TAB or SPACE
    {
        pProf->pReadPtr++;
    }
}

int Cfg_ProfileReadLine(IN SProfile *pProf, OUT char *pszBuf, IN int nBufSize)
{
    int     i = 0;
    int     nWhitePos = -1;

    /* remove all following CR and LF */
    Cfg_SkipWhiteSpace(pProf);

    while (!END_OF_PROF(pProf))
    {
        if (END_OF_LINE(*pProf->pReadPtr))
        {
            break;
        }

        if (i < nBufSize-1)
        {
            if (IS_WHITE_SPACE(*pProf->pReadPtr))
            {
                if (nWhitePos < 0)
                    nWhitePos = i;
            }
            else
                nWhitePos = -1;

            pszBuf[i++] = *pProf->pReadPtr;
        }

        // if the buffer is full, but we do not encounter the CR or LF
        // we continue read till the CR or LF, or end of file
        pProf->pReadPtr++;
    }

    if (nWhitePos >= 0)    // remove the ending white space
    {
        i = nWhitePos;
    }

    pszBuf[i] = 0;        // now line data end with NULL

    return i;
}

char *Cfg_RemoveWhiteSpace(IN char *pszBuf)
{
    char    *p = pszBuf;
    char    *pFirstNonSpace = pszBuf, *pLastWhiteSpace = NULL;
	BOOL    bFound = FALSE;  /* first non-space character found flag */

    while (*p)
    {
        if (IS_WHITE_SPACE(*p))
        {
            if(pLastWhiteSpace == NULL)
                pLastWhiteSpace = p;
        }
        else 
        {
            if (!bFound)
			{
                pFirstNonSpace = p;
				bFound = TRUE;
			}

            pLastWhiteSpace = NULL;
        }

        p++;
    }

    if (pLastWhiteSpace != NULL)
        *pLastWhiteSpace = 0;

    return pFirstNonSpace;
}

char *Cfg_RemoveBoundChr(IN char *pszBuf, IN IS_BOUND_CHR_PROC pfnIsBoundChr)
{
    char    *p = pszBuf;
    char    *pFirstBoundChr = pszBuf, *pLastBoundChr = NULL;
	BOOL    bFound = FALSE;  /* first bound character found flag */
	
    while (*p)
    {
        if (pfnIsBoundChr((int)*p))
        {
            if(pLastBoundChr == NULL)
                pLastBoundChr = p;
        }
        else 
        {
            if (!bFound)
			{
                pFirstBoundChr = p;
				bFound = TRUE;
			}

            pLastBoundChr = NULL;
        }

        p++;
    }

    if (pLastBoundChr != NULL)
        *pLastBoundChr = 0;

    return pFirstBoundChr;
}

static int Cfg_ProfileFindSectionRealize(IN void *pProf, IN const char *pszKey)
{
	SProfile *p;
    int n, nCmp;
    char buf[MAX_LINE_SIZE];

	p = (SProfile *)pProf;

    /* find the key-section */
    nCmp = strlen(pszKey);

    do
    {
        n = Cfg_ProfileReadLine(p, buf, sizeof(buf));
    } while ((n > 0) && (strnicmp(buf, pszKey, (size_t)nCmp) != 0));

    return n;
}

int Cfg_ProfileFindSection(IN void *pProf, IN const char *pszKey)
{
    int n;
	n = Cfg_ProfileFindSectionRealize(pProf, pszKey);

	if( n == 0 )
	{
		Cfg_ProfileRewind(pProf);
		n = Cfg_ProfileFindSectionRealize(pProf, pszKey);
	}

	return n;
}

int Cfg_ProfileGetLine(IN void *pProf, IN const char *pszSectionKey, 
					   OUT char *pszBuf, IN int nBufSize)
{

    int n;
    
    n = Cfg_ProfileFindSection(pProf, pszSectionKey);

	if (n == 0 )
	{
		return -1;
	}

	else
    {
        n = Cfg_ProfileGetNextLine(pProf, pszBuf, nBufSize);
    }

}

int Cfg_ProfileGetNextLine(IN void *pProf, OUT char *pszBuf, IN int nBufSize)
{
    int     n;
    char    *p;
	char    *pBufferedReadPtr = NULL;       //buffer the ReadPtr of last line
	SProfile *pFile;

	pFile = (SProfile *)pProf;

    do
    {
        pBufferedReadPtr = pFile->pReadPtr;
		n = Cfg_ProfileReadLine(pFile, pszBuf, nBufSize);

        if (n > 0)
        {
            /* get rid of the comment line */
            p = strstr(pszBuf, COMMENT_FLAG);
            if (p != NULL)
            {
                *p = 0;
            }
        }
    } while ((n > 0) && (*pszBuf == 0));

    if (pszBuf[0] == SECT_PREFIX)// || n == 0)
	{
		pFile->pReadPtr = pBufferedReadPtr;
		n = 0;
	}

    return n;
}

char *Cfg_SplitString(IN char *pszSrc, OUT char *pszDst, IN int nBufLen, IN int cSplitter)
{
    char    *p, *q;

    /* skip the first splitter and white space */
    p = pszSrc;
    while ((*p != 0) && ((*p == (char)cSplitter) || IS_WHITE_SPACE(*p)))
    {
        p++;
    }

    /* copy string to dest */
	q = pszDst;
    for ( ; (*p != 0) && (*p != (char)cSplitter); p++ )
    {
        if (nBufLen > 1)
        {
            *q++ = *p;
            nBufLen --;
        }
    }

    *q = 0;

    Cfg_RemoveWhiteSpace(pszDst); // remove the rear space

    return p;
}

char *Cfg_SplitStringEx(IN char *pszSrc, OUT char **ppszDst, IN int cSplitter)
{
    char    *p;

    /* skip the first splitters and white space */
    p = pszSrc;
    while ((*p != 0) && ((*p == (char)cSplitter) || IS_WHITE_SPACE(*p)))
    {
        p++;
    }

    /* set dest addr ref */
    *ppszDst = p;

    for ( ; (*p != 0) && (*p != (char)cSplitter); p++ )
    {
        ;
    }

    if (*p == (char)cSplitter)
    {
        *p = 0; // set the end of string
        p++;   // goto next string
    }

    Cfg_RemoveWhiteSpace(*ppszDst); // remove the rear space

    return p;
}

int Cfg_GetSplitStringCount(IN char *p, IN int cSplitter)
{
    int  cnt = 0;

    while (*p)
    {
        // skip the first splitters and white space
        while ((*p != 0) && ((*p == (char)cSplitter) || IS_WHITE_SPACE(*p)))
        {
            p++;
        }
        
        // set dest addr ref.
        cnt++;
        
        // skip the non-white chars
        while ((*p != 0) && (*p != (char)cSplitter))
        {
             p++;
        }
    }

    return cnt;
}

int Cfg_ProfileGetInt(IN void *pCfg, IN const char *pszSectionKey, OUT int *pNum)
{
    char szBuf[32];
    int  i, ret;

	ret = Cfg_ProfileGetLine(pCfg, pszSectionKey, szBuf, sizeof(szBuf));
    if ( ret <= 0)
    {
        return ret;
    }

    // is +nnn or -nnn ?
    i = ((szBuf[0] == '-') || (szBuf[0] == '+')) ? 1 : 0;

    if ((szBuf[i] < '0') || (szBuf[i] > '9'))
        return -1;    // not num error

    *pNum = atoi(szBuf);

    return 1;
}


int Cfg_ProfileGetString(IN void *pCfg, IN const char *pszSectionKey, 
                     OUT char *pszBuf, IN int nBufLen)
{
    char szBuf[MAX_LINE_SIZE];
	int  ret;

    *pszBuf = 0;

	ret = Cfg_ProfileGetLine(pCfg, pszSectionKey, szBuf, sizeof(szBuf));
    if (ret <= 0)
    {
        return ret;
    }

    strncpy(pszBuf, szBuf, (size_t)nBufLen);
    pszBuf[nBufLen-1] = 0;

    return 1;
}

int Cfg_GetSectionLineCount(IN void *pProf, IN const char *szSectionKey)
{
	int ret, iLineCount = 0;
	char szBuf[MAX_LINE_SIZE];
	ret = Cfg_ProfileFindSection(pProf, szSectionKey);

	if (ret == 0)
	{
		return -1;
	}

	ret = Cfg_ProfileGetNextLine(pProf, szBuf, MAX_LINE_SIZE);
	while (ret != 0)
	{
		iLineCount++;
		ret = Cfg_ProfileGetNextLine(pProf, szBuf, MAX_LINE_SIZE);
	}

	return iLineCount;
}


static void MoveBackOneLine(IN void *pProf)
{
	SProfile *pFile = (SProfile *)pProf;
    char *p;

	p = pFile->pReadPtr;

	/* remove current LF or CRs */
	while (END_OF_LINE(*p))
	{
		p--;
	}
	
	/* move back one line */
	while ( p != pFile->pFileBuffer)
	{
		if (END_OF_LINE(*p))
		{
			break;
		}

		p--;
	}

	pFile->pReadPtr = p;
	return;
}


static void MoveBackCommentLines(IN void *pProf)
{
	SProfile *pFile = (SProfile *)pProf;
	char *p, *pBuf;
	int  iFoundCount = 0;

	do
	{
		pBuf = pFile->pReadPtr;
		MoveBackOneLine(pProf);
		p = pFile->pReadPtr + 1;
		iFoundCount++;
	}
	while (*p == '#');

	/* if found '#' */
	if (iFoundCount > 1)
	{
		/* remove dupplicated LF or CRs */
		while (END_OF_LINE(*pBuf))
		{
			pBuf--;
		}
		pBuf++;
	}

	pFile->pReadPtr = pBuf;
}


int Cfg_ProfileGetSectionPos(IN void *pProf,
							 IN const char *szKey, 
							 OUT size_t *pulStartPos, 
							 OUT size_t *pulEndPos)
{
	SProfile *pFile;
	char szBuf[MAX_LINE_SIZE];
	int iRet;

	pFile = (SProfile *)pProf;

	/* find the section */
	iRet = Cfg_ProfileFindSection(pProf, szKey);
    if (iRet == 0)
	{
		return 0;
	}

	/* get first line of the section */
	iRet = Cfg_ProfileGetNextLine(pProf, szBuf, MAX_LINE_SIZE);

	MoveBackOneLine(pProf);
	*pulStartPos = pFile->pReadPtr - pFile->pFileBuffer;
	
	/* empty section */
	if (iRet == 0)
	{
		*pulEndPos = *pulStartPos;

		return 1;
	}

    /* not empty section */
	do
	{
		iRet = Cfg_ProfileGetNextLine(pProf, szBuf, MAX_LINE_SIZE);
	}
	while (iRet != 0);
	
	MoveBackCommentLines(pProf);
	//MoveBackOneLine(pProf);
	*pulEndPos = pFile->pReadPtr - pFile->pFileBuffer;

	return 1;
}

//empty field judging function
BOOL Cfg_IsEmptyField(IN const char *pField)
{
	/*	if (pField == NULL)
	{
	return FALSE;
	}

	if ((pField + 1 ) == NULL)
	{
	return FALSE;
	}
	*/

	if (*pField == 'N' && *(pField + 1) == 'A' &&
		*(pField + 2) == '\0')
	{
		return TRUE;
	}
	else 
	{
		return FALSE;
	}
}


BOOL CFG_CheckBase(const char *szObject, LEGAL_CHR fnCmp)
{
	const char *p;

	if (szObject == NULL)
	{
		return FALSE;
	}

	p = szObject;
	for (; p != NULL; p++)
	{
		if (*p == '\0')
		{
			return TRUE;
		}

		if (!fnCmp(*p))
		{
			return FALSE;
		}

	}

	return FALSE;
}

static BOOL IsPNumber(char c)
{
	return  (c >= '0' && c <= '9');
}

BOOL CFG_CheckPNumber(const char *szNumber)
{
	return CFG_CheckBase(szNumber, IsPNumber);
}
	
BOOL CFG_CheckNumber(const char *szNumber)
{
	if (szNumber[0] == '-')
	{
		return CFG_CheckBase(szNumber+1, IsPNumber);
	}
	else
	{
		return CFG_CheckBase(szNumber, IsPNumber);
	}
}

static BOOL IsHexNumber(char c)
{
	return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
		(c == 'x') || (c == 'X'));
}

BOOL IsBoundChr_S(int c)
{
	return ((char)c == '[' || (char)c == ']');
}
BOOL IsBoundChr_Kuohao(int c)
{
	return ((char)c == '(' || (char)c == ')');
}
BOOL IsBoundChr_BS(int c)
{
	return ((char)c == '{' || (char)c == '}');
}

char Cfg_ReplaceIllegalChr(int c)
{
	return (((char)c == '<' || (char)c == '>' || (char)c == '\'' || 
		(char)c == '"') ? '?' : (char)c);
}


BOOL CFG_CheckHexNumber(const char *szHexNumber)
{
	return CFG_CheckBase(szHexNumber, IsHexNumber);
}

BOOL CFG_ParseINT_FromTxtBuffer(IN OUT char **szBuffer, OUT int *pOutVal)
{
	char *pField;
	*szBuffer = Cfg_SplitStringEx(*szBuffer, &pField, SPLITTER); //!!!note, this will cause szBuff move to next section
	if(Cfg_IsEmptyField(pField))
	{
		*pOutVal = -1;  //if set to 'NA', then use -1 instead.
		return TRUE;
	}
	if ((*pField < '0') || (*pField > '9'))
	{
		//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
		//	"[%s]--ParseSampleSigTableProc: ERROR: Sig Name Resource "
		//	"ID is not a number!\n", __FILE__);
		return FALSE;    /* not a num, error */
	}
	*pOutVal = atoi(pField);
	return TRUE;
}

BOOL CFG_ParsePrefixedINT_FrmTxtBuffer(IN OUT char **szBuffer, IN char chMark, OUT int *pOutVal)
{
	char *pField;
	*szBuffer = Cfg_SplitStringEx(*szBuffer, &pField, SPLITTER); //!!!note, this will cause szBuff move to next section
	//printf("\n --- The pFeild = %s", pField);
	if(Cfg_IsEmptyField(pField))
	{
		*pOutVal = -1;  //if set to 'NA', then use -1 instead.
		return TRUE;
	}
	if (*pField != chMark)
	{
		//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
		//	"[%s]--ParseSampleSigTableProc: ERROR: Sig Name Resource "
		//	"ID is not a number!\n", __FILE__);
		TRACEX("Can not find the Mark[%c], pField = %s, szBuffer = %s\n", chMark, pField,szBuffer);
		return FALSE;    /* not a num, error */
	}
	pField++;
	if ((*pField < '0') || (*pField > '9'))
	{
		//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
		//	"[%s]--ParseSampleSigTableProc: ERROR: Sig Name Resource "
		//	"ID is not a number!\n", __FILE__);
		TRACEX("Found non-number, pField = [%s]---\n", pField);
		return FALSE;    /* not a num, error */
	}
	*pOutVal = atoi(pField);
	return TRUE;
}
BOOL CFG_ParseValFormat_FrmTxtBuffer(IN OUT char **szBuffer, OUT SIG_VAL_FORMAT *pOutVal)
{
	char *pField;
	*szBuffer = Cfg_SplitStringEx(*szBuffer, &pField, SPLITTER); //!!!note, this will cause szBuff move to next section
	if ((*pField == 'F') && (*(pField+1) == '0'))
	{
		*pOutVal = VAL_FORMAT_F0;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'F') && (*(pField+1) == '1'))
	{
		*pOutVal = VAL_FORMAT_F1;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'F') && (*(pField+1) == '2'))
	{
		*pOutVal = VAL_FORMAT_F2;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'F') && (*(pField+1) == '3'))
	{
		*pOutVal = VAL_FORMAT_F3;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'F') && (*(pField+1) == '4'))
	{
		*pOutVal = VAL_FORMAT_F4;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'D') && (*(pField+1) == 'T'))
	{
		*pOutVal = VAL_FORMAT_DT;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'U') && (*(pField+1) == 'L'))
	{
		*pOutVal = VAL_FORMAT_UL;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'L') && (*(pField+1) == 'I'))
	{
		*pOutVal = VAL_FORMAT_LI;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'E') && (*(pField+1) == 'M'))
	{
		*pOutVal = VAL_FORMAT_EM;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'I') && (*(pField+1) == 'P'))
	{
		*pOutVal = VAL_FORMAT_IP;
		return TRUE;    /* not a num, error */
	}
	else if ((*pField == 'C') && (*(pField+1) == '6') && (*(pField+2) == '4'))
	{
		*pOutVal = VAL_FORMAT_C64;
		return TRUE;    /* not a num, error */
	}
	*pOutVal  = VAL_FORMAT_UNKOWN;
	return FALSE;
}
BOOL CFG_ParseAExpVariabl_FrmTxtBuffer(IN OUT char **szBuffer, OUT EXP_VARIABLE **pOutVal)
{
	char *pField;
	*szBuffer = Cfg_SplitStringEx(*szBuffer, &pField, SPLITTER); //!!!note, this will cause szBuff move to next section
	/* get rid of '[' and ']' */
	pField = Cfg_RemoveBoundChr(pField, IsBoundChr_S);
	//should be [devUID, sigType, SigID]
	int iSubFieldCount = Cfg_GetSplitStringCount(pField, ',');
	if (iSubFieldCount != 3)
	{
		//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
		//	"[%s]--CFG_ParseSigEnuemLst_FrmSetion: ERROR:  "
		//	"format for enum value is invalid(missing \'$\')!\n",
		//	__FILE__);
		*pOutVal = NULL;
		return FALSE;
	}

	char *pSubField;
	(*pOutVal) = NEW(EXP_VARIABLE, 1);
	pField = Cfg_SplitStringEx(pField, &pSubField, ',');
	(*pOutVal)->iRelevantEquipIndex = atoi(pSubField);

	pField = Cfg_SplitStringEx(pField, &pSubField, ',');
	(*pOutVal)->iSigType = atoi(pSubField);

	pField = Cfg_SplitStringEx(pField, &pSubField, ',');
	(*pOutVal)->iSigID = atoi(pSubField);

	return TRUE;
}

BOOL CFG_ParseSampDataSrc_FrmTxtBuffer(IN OUT char **szBuffer, OUT SAMP_DATA_SRC *pOutVal)
{
	char *pField;
	*szBuffer = Cfg_SplitStringEx(*szBuffer, &pField, SPLITTER); //!!!note, this will cause szBuff move to next section
	if ((*pField == 'S') && (*(pField+1) == 'A'))
	{
		*pOutVal = SAMP_DATA_SRC_SA;
		return TRUE;    /* not a num, error */
	}
	else 	if ((*pField == 'C') && (*(pField+1) == 'A'))
	{
		*pOutVal = SAMP_DATA_SRC_CA;
		return TRUE;    /* not a num, error */
	}
	else 	if ((*pField == 'L') && (*(pField+1) == 'C'))
	{
		*pOutVal = SAMP_DATA_SRC_LC;
		return TRUE;    /* not a num, error */
	}
	*pOutVal  = SAMP_DATA_SRC_UNKOWN;
	return FALSE;
}
BOOL CFG_ParseCtrlMethod_FrmTxtBuffer(IN OUT char **szBuffer, OUT CTRL_SIG_CTRL_METHOD *pOutVal)
{
	char *pField;
	*szBuffer = Cfg_SplitStringEx(*szBuffer, &pField, SPLITTER); //!!!note, this will cause szBuff move to next section
	if ((*pField == 'C') && (*(pField+1) == 'P'))
	{
		*pOutVal = CTRL_METHOD_CP;
		return TRUE;    /* not a num, error */
	}
	else 	if ((*pField == 'C') && (*(pField+1) == 'B'))
	{
		*pOutVal = CTRL_METHOD_CB;
		return TRUE;    /* not a num, error */
	}
	else 	if ((*pField == 'C') && (*(pField+1) == 'A'))
	{
		*pOutVal = CTRL_METHOD_CA;
		return TRUE;    /* not a num, error */
	}
	else 	if ((*pField == 'C') && (*(pField+1) == 'F'))
	{
		*pOutVal = CTRL_METHOD_CF;
		return TRUE;    /* not a num, error */
	}
	*pOutVal  = CTRL_METHOD_UNKOWN;
	return FALSE;
}

BOOL CFG_ParseSigEnuemLst_FrmSetion(IN char *szSection, OUT int *pEnumNum, OUT ENUM_ITEM **ppEnumLst, OUT float *pfMaxValidVal, OUT float *pfMinValidVal)
{
	char *pField;
	char *pSubField;
	int nEnumTotal = Cfg_GetSplitStringCount(szSection, ','); //members are separated by ',' mark.
	(*pEnumNum) = nEnumTotal;
	//printf("\n----The TotalEnum = %d, pField = %s\n",nEnumTotal, szSection);
	if(nEnumTotal <= 0)
	{
		return FALSE;
	}
	
	*ppEnumLst = NEW(ENUM_ITEM, nEnumTotal);
	ENUM_ITEM *pEnumMov = *ppEnumLst ;

	int iCurIndex = 0;


	BYTE byMax = 0;
	BYTE byMin = 255;
	//BYTE iCurrEnumVal = 0;
	do
	{
		szSection = Cfg_SplitStringEx(szSection, &pField, ',');	
		/* get rid of '(' and ')' */
		pField = Cfg_RemoveBoundChr(pField, IsBoundChr_Kuohao);
		int iSubFieldCount = Cfg_GetSplitStringCount(pField, '$');
		if (iSubFieldCount != 2)
		{
			//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
			//	"[%s]--CFG_ParseSigEnuemLst_FrmSetion: ERROR:  "
			//	"format for enum value is invalid(missing \'$\')!\n",
			//	__FILE__);
			return FALSE;
		}

		/* get enum value */
		pField = Cfg_SplitStringEx(pField, &pSubField, '$');
		pSubField = Cfg_RemoveWhiteSpace(pSubField);
		if (*pSubField < '0' || *pSubField > '9')
		{
			//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
			//	"[%s]--ParseSampleSigTableProc: ERROR: Value "
			//	"Range format for enum value is invalid(resource id "
			//	"is not a number)!\n", __FILE__);
			return FALSE;
		}
		pEnumMov->byEnumValue = atoi(pSubField);
		/* now get enum resID */
		Cfg_SplitStringEx(pField, &pSubField, '$');
		pSubField = Cfg_RemoveWhiteSpace(pSubField);
		if (*pSubField < '0' || *pSubField > '9')
		{
			//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
			//	"[%s]--ParseSampleSigTableProc: ERROR: Value "
			//	"Range format for enum value is invalid(resource id "
			//	"is not a number)!\n", __FILE__);
			return FALSE;
		}
		pEnumMov->nEnumResID = atoi(pSubField);

		if(pEnumMov->byEnumValue > byMax)
		{
			byMax = pEnumMov->byEnumValue;
		}

		if(pEnumMov->byEnumValue < byMin)
		{
			byMin = pEnumMov->byEnumValue;
		}

		pEnumMov++;

	}while (++iCurIndex < nEnumTotal);

	*pfMaxValidVal = (float)(byMax);
	*pfMinValidVal = (float)(byMin);

	//if(byMax > 1)
	//{
	//	TRACEX("Found min = %d, max = %d, minmax f= %f~%f",
	//		byMin, byMax, *pfMinValidVal, *pfMaxValidVal);
	//}
	return TRUE;
}
BOOL CFG_ParseFloatValRange_FrmSetion(IN char *szSection, OUT float *pfMaxValidVal, OUT float *pfMinValidVal)
{
	char *pSubField;
	int iSubFieldCount = Cfg_GetSplitStringCount(szSection, '/');
	if (iSubFieldCount != 2)
	{
		//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
		//	"[%s]--ParseSampleSigTableProc: ERROR: Value "
		//	"Range field is invalid (for non-enum value, should be "
		//	"minValue/maxValue format)!\n", __FILE__);
		return FALSE;
	}

	/* get minValue of the range */
	szSection = Cfg_SplitStringEx(szSection, &pSubField, '/');
	if ((*pSubField < '0' || *pSubField > '9') && *pSubField != '-')
	{
		//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
		//	"[%s]--ParseSampleSigTableProc: ERROR: minValue "
		//	"of Value Range field is not a number!\n", __FILE__);
		return FALSE;
	}
	*pfMinValidVal = atof(pSubField);

	/* get maxValue of the range */
	Cfg_SplitStringEx(szSection, &pSubField, '/');
	if ((*pSubField < '0' || *pSubField > '9') && *pSubField != '-')
	{
		//AppLogOut(CFG_LOAD_EQUIP, APP_LOG_ERROR, 
		//	"[%s]--ParseSampleSigTableProc: ERROR: maxValue "
		//	"of Value Range field is not a number!\n", __FILE__);
		return FALSE;
	}
	*pfMaxValidVal = atof(pSubField);

	return TRUE;
}
char *Cfg_GetFullConfigPath(IN char *pszCfgFile, OUT char *szFullPath, IN int iFullPathLen)
{
	//	static char szFullPath[256];
	////#warning "This function Cfg_GetFullPath is not Multi-Thread safe!"
	//#define ENV_ACU_ROOT_DIR	"ACU_ROOT_DIR"

	char *pszConfigEnv = getenv("IMAU_RUN_PATH");	

	if (pszConfigEnv == NULL)
	{
		printf("Please set the environment variable 'IMAU_RUN_PATH' of ACU root dir!\n");
		pszConfigEnv = "/app/config";
	}
	
	
	memset(szFullPath, 0, iFullPathLen);
	snprintf(szFullPath, iFullPathLen,
		"%s/%s/%s",
		pszConfigEnv, "config",
		pszCfgFile);

	//printf("\n The cfg is [%s], envPaht is [%s],copyiiiing size is %d\n",pszCfgFile,pszConfigEnv,iFullPathLen);

	//snprintf(szFullPath, (size_t)iFullPathLen,
	//	"%s/%s",
	//	g_szACUConfigDir,
	//	pszPartPath);


	return szFullPath;
}


