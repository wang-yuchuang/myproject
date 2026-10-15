
#ifndef __PUBFUNC_H__
#define __PUBFUNC_H__

/* 
 * Functions for string operating	
 */
#include "basetypes.h"
#include <stdio.h>
#include "std_sys.h"
#include "3rd_party/sqlite/sqlite3.h"

//define METER deviced
typedef enum tagMeterTypes
{
    METER_TYPES_CAB = 0,
    METER_TYPES_GRID ,
    METER_TYPES_PV ,
    METER_TYPES_AUX ,

    METER_TYPES_MAX ,
}METER_TYPES;

#define METER_TYPE_NUM	4
#define CAB_FIRST (METER_TYPES_CAB * METER_TYPE_NUM + 0)
#define GRID_FIRST (METER_TYPES_GRID * METER_TYPE_NUM + 0)

extern UINT All_TypesMetersID[METER_TYPES_MAX][METER_TYPE_NUM];

typedef char (*REPLACE_ILLEGAL_CHR_PROC)(int c); 

char *strncpyz(OUT char *pDst, IN const char *pSrc, IN int nDstLen);
char *strncpyz_f(OUT char *pDst, IN const char *pSrc, IN int nDstLen, 
				REPLACE_ILLEGAL_CHR_PROC pfn);
char * NEW_strdup_f(const char *szSrc,REPLACE_ILLEGAL_CHR_PROC pfn);

char * NEW_strdup(IN const char *szSrc);
char * NEW_straddz(IN const char *szSrc1, IN const char *szSrc2);
char * NEW_strcpy(OUT char **szDst, IN const char *szSrc);

BOOL CheckInt_AndParse(IN char* szSrc, OUT int* pnOut);

#ifdef unix
#define stricmp		strcasecmp
#define strnicmp	strncasecmp
char * strupr(char * string);
#endif

// init mem to 0. p: the ptr of struct objects, n: the num of struct items
#define ZERO_POBJS(p, n)	memset((p), 0, (n)*sizeof(*(p)))
#define CR		(0x0D)
#define LF			(0x0A)
#define SPACE		(0x20)
#define TAB		(0x09)
#define DOT		('.')
#define COMMA		(',')
#define SLASH		('/')
#define COLON		(':')

/*
 * Functions for FILE operating
 */

long GetFileLength( IN FILE *fp );

void Sleep( IN DWORD dwMilliseconds );
void Sleep_Ex( IN DWORD dwMilliseconds); //added by Jimmy

#define WAIT_DATA_READY        1
#define WAIT_DATA_TIMEOUT     0
#define WAIT_DATA_ERROR       -1
#define WAIT_DATA_EXCEPT      -2

int WaitFiledWritable(int fd, int nmsTimeOut/* milliseconds*/);

int WaitFiledReadable(int fd, int nmsTimeOut/* milliseconds*/);

int Dec2Bcd(int num);
int Bcd2Dec(int num);
// LJH241202 SEMS add Bcd datetime+ms
#pragma pack(1)
typedef struct tagBcdTimeStmp
{
	USHORT   msecond;
	BYTE     second;
	BYTE     minute;
	BYTE     hour;
	BYTE     day;
	BYTE     month;
	BYTE     yearL;
	BYTE     yearH;
} BcdTimeStmp;
#pragma pack()

char* printableBcdTimeStmp(const BcdTimeStmp* pBcdTime);
BcdTimeStmp GetCurrentBcdTimeStmp(void);
time_t BcdTimeStmp2time_t(const BcdTimeStmp* pBcdTime);
double BcdTimeStmp2DoubleTime(const BcdTimeStmp* pBcdTime);
double GetCurrentTime(void); // return the current time in seconds. 
UINT ElapseS(time_t timeS);
UINT ElapseMS(double dTimeMS);
UINT ElapseMSBcdTimeStmp(const BcdTimeStmp* pBcdTime);

BOOL HasEMSFeature(SYSTEM_TYPE emSystemType);

HANDLE LoadDynamicLibrary(IN const char *pszLibName,
					   IN int			nSym,
					   IN const char	*ppszSymName[],
					   OUT HANDLE		*pfnProc[],
					   IN BOOL			bNeedAllSym);

void UnloadDynamicLibrary(IN HANDLE hLib);

#define MAX_FILE_PATH			256
char *MakeFullFileName(OUT char *pFullName,
							  IN const char *pszRootDir,
							  IN const char *pszSubDir,
							  IN const char *pszFileName);

int FindIntItemIndex(IN int nItemToFind, IN int *pIntArray, IN int nItemNum);

#define TIME_CHN_FMT		"%Y-%m-%d %H:%M:%S"	// YYYYMMDD hhmmss
#define SQLITE_DATE_FMT		"%Y-%m-%d" //YYYY-MM-DD
#define TIME_TRIM_FMT		"%Y%m%d%H%M%S"

char *TimeToString(IN time_t tmTime, const char *fmt, 
				   OUT char *strTime, IN int nLenStrTime);

time_t StringToTime(IN char* strTime, IN const char *fmt);

time_t ConvDateTime_2DateOnly(time_t tt);

char *AcurTimeToString(IN double fTime, const char *fmt, 
	OUT char *strTime, IN int nLenStrTime);

char *RemoveStringWhiteSpace(IN char *pszBuf);
char *Trim_String(IN char *pszBuf, char cTrim);
int GetSplitStringCount(IN char *p, IN int cSplitter);
char *SplitString(IN char *pszSrc, OUT char **ppszDst, IN int cSplitter);
time_t Parse_ISO8601TimeStr(char *pTimeStr);
int Parse_ISO8601TimeZone(char *pTimeStr);
BOOL Parse_ISO8601TimeStrEx(char *pTimeStr, OUT time_t* pOutTime);
int _SYSTEM(char* pszCmd);
void Do_Proc_HeartBeat(void);
int _SYSTEM_EX(char* pszCmd); //updated version, for returnning a value
/*
 void PostSignal_4Qt(void);
 void Do_Ctrl(int ulWhichDO, BOOL bOn);
 BOOL Get_DI(int nWhichDI);
 void Wait_QtCmd_andExecute(int nMaxWait_mSec);
 
 
 #define DIDO_NAME 		"/dev/dido"
 #define DIDO_MAJOR			240
 // ################## IMMU2 DO1~DO6  #############//
 #define IOC_SET_DO1  	 _IOW(DIDO_MAJOR, 1, unsigned long)
 #define IOC_SET_DO2  	 _IOW(DIDO_MAJOR, 2, unsigned long)
 #define IOC_SET_DO3  	 _IOW(DIDO_MAJOR, 3, unsigned long)
 #define IOC_SET_DO4  	 _IOW(DIDO_MAJOR, 4, unsigned long)
 #define IOC_SET_DO5  	 _IOW(DIDO_MAJOR, 5, unsigned long)
 #define IOC_SET_DO6  	 _IOW(DIDO_MAJOR, 6, unsigned long)
 
 //#define IOCTL_BLIGHT_PWM  _IOW(LCD_MAJOR, 6, unsigned long)
 #define IOCTL_BEEP_100	  _IOW(DIDO_MAJOR, 18, unsigned long)
 #define IOCTL_BEEP_ONOFF  _IOW(DIDO_MAJOR, 17, unsigned long)
 
 // ################## IMMU2 DI1~DO4  #############//
 #define IMMU2_DI1		0
 #define IMMU2_DI2		1
 #define IMMU2_DI3		2
 #define IMMU2_DI4		3
 */
 
char* Percent_EncodeUrl(char* szSrcUrl);
char* Percent_DecodeUrl(char* szSrcUrl); //de-code for percent-encoded-url
#define TIME_CHN_FMT		"%Y-%m-%d %H:%M:%S"	// YYYYMMDD hhmmss
#define TIME_COMPACT_FMT	"%y%m%d %H%M%S"		// YYMMDDhhmmss
#define TIME_CHN_FMT_EX		"%Y%m%d%H%M%S"	// YYYYMMDDhhmmss, no split

#define ISO8601_FMT_1	"%Y%m%dT%H%M%S%z"
#define ISO8601_FMT_2	"%Y%m%dT%H%M%S"

#define ISO8601_FMT_3	"%Y-%m-%dT%H:%M:%S%z"
#define ISO8601_FMT_4	"%Y-%m-%dT%H:%M:%S"

#define MAX_TIME_LEN 50

#define COMMA_SKIP		'.'
//Define my comp struct so as to identify if data was compressed before
typedef struct myCompStream
{
#define MYCOMP_SOI		0xAA
#define MYCOMP_HEAD_LEN	13
	BYTE	bySOI;
	UINT	nLenOfSrc; //length of source data(2be compressed)
	UINT	nChkSumOf1K; //the chkSum of the first 1K Data
	UINT	nLenOfDest; //length of destination data(already compressed)
	BYTE*	pData;
}MY_COMPRESSED_STREAM;
//Jimmy added 20200522 for comressing of sqliteDB
int sqlite3_bind_blob_compress(sqlite3_stmt* pStmt, int idx, const void* pDataSrc, int nSize, void** ppDataOut, void(*pfnDispose)(void*));
void sqlite3_finalize_compressed(sqlite3_stmt* pStmt, void* pDataDest); //do not forget to call this instead after insert/update done
BOOL sqlite3_parse_blob_uncompressed(sqlite3_stmt* pStmt, int idx, OUT void* pUncompDest, int nUnCompDestSize);
size_t sqlite3_sizeof_blob_uncompressed(sqlite3_stmt* pStmt, int idx);
char* GetIMMU2_HWVersion(void); //added by Jimmy 20210304
BOOL FindMatchFile_FromDir(char* szFindPath, char* szMatchName, char* szFoundFileName);
BOOL JudgeIfRunInNew_Kernel_4(void); //added by Jimmy 20210413 for new kernel
char* FindFilePath_In_USB_OrSD_MountedDir(BOOL bFindUSB, char* szFile2Find, char* szPath, int nPathLen);
//base64 encode/decode using openssl lib
BOOL Base64_Encode(const char * content, 
				  int          length, 
				  char       * output, 
				  int        * output_size);
BOOL Base64_Decode(const char * content, 
				  int          length, 
				  char       * output, 
				  int        * output_size);
BOOL MD5_Encode(const unsigned char * content, 
				  size_t     		length, 
				  unsigned char      * output,
				  int				nOutlen);

//upper/lower case conv
char*  ConvertString_2UpperCase(char* src);
char*  ConvertString_2LowerCase(char* src);
LC_TYPE GetLCType(UINT idxClientLC);
UINT GetIdx0_LC2nd(UINT idxClientLC);
UINT GetIdx0_LC4cab(UINT idxClientLC);

char*  printableBytes(const BYTE* data, int len);
void trace_debug(const char *format, ...);
void trace_info(const char *format, ...);
void trace_warn(const char *format, ...);
void trace_error(const char *file, int line, const char *format, ...);
void trace_fatal(const char *file, int line, const char *format, ...);
BYTE* reverseBytes(BYTE* pBytes, int len);
BOOL Save_CloudToDb(time_t t_SaveTime, void* blobDataStorage, UINT nDataStorageLen);

#endif /*__PUBFUNC_H__*/
