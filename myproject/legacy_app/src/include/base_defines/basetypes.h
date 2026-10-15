#ifndef __BASE_TYPES_H_2018_03_28
#define __BASE_TYPES_H_2018_03_28 

//#include <endian.h>

//#if __BYTE_ORDER == __LITTLE_ENDIAN		//x86
//	#undef	_CPU_BIG_ENDIAN
//	#define _CPU_LITTLE_ENDIAN	1
//#elif __BYTE_ORDER == __BIG_ENDIAN		// ppc
//	#undef  _CPU_LITTLE_ENDIAN
//	#define _CPU_BIG_ENDIAN		1
//#endif

// 编辑代码时使能
// #define IMMU2
// #define IMMU3

#define	__INLINE			inline
#define	LOCAL			static		// static

typedef	 char				CHAR;
typedef	 unsigned char		UCHAR;
typedef	 unsigned char		BYTE;      /* by : variable prefix */
typedef	   signed short		SHORT;
typedef	 unsigned short		USHORT;
typedef	 unsigned short		WORD;      /* n  : variable prefix */
typedef	 unsigned long		DWORD;     /* dw : variable prefix */
typedef	 unsigned long		ULONG;     /* ul : variable prefix */
typedef	 DWORD			*LPDWORD;
typedef	 WORD			*LPWORD;
typedef	 int				BOOL;      /* b  : variable prefix */
typedef	 long				LONG;      /* l  : variable prefix */
typedef	 void				*HANDLE;
#define	 CALLBACK
typedef	 void				VOID;
typedef	 void				*PVOID;
typedef	 VOID			*LPVOID;

typedef	unsigned int		UINT;
typedef	unsigned long		_DATE_TIME;
typedef	unsigned long		_ENUM;


#ifndef _UTIME_T_DEFINED
typedef unsigned long utime_t;
#define _UTIME_T_DEFINED
#endif

#ifndef XBOOL   // unix boolean
#define XBOOL   int
#define OK      0
#define SUCCESS OK
#define FAIL    -1
#endif

#ifndef BOOL    
    #define BOOL    int
#endif
#ifndef TRUE
    #define TRUE    1
#endif
#ifndef FALSE
    #define FALSE   0
#endif

#define UNUSED(x)	(void)(x)

#ifdef _TRACE_PRINT_
	#define TRACED(fmt, ...) do {trace_debug(fmt,##__VA_ARGS__);} while(0)
	#define TRACEI(fmt, ...) do {trace_info(fmt, ##__VA_ARGS__);} while(0)
	#define TRACEW(fmt, ...) do {trace_warn(fmt, ##__VA_ARGS__);} while(0)
	#define TRACEE(fmt, ...) do {trace_error(__FILE__, __LINE__, fmt, ##__VA_ARGS__);} while(0)
#else
	#define TRACED(fmt, ...) do {} while(0)
	#define TRACEI(fmt, ...) do {} while(0)
	#define TRACEW(fmt, ...) do {} while(0)
	#define TRACEE(fmt, ...) do {} while(0)
#endif
// will write to DB Data_AppLog 慎用
#define TRACEF(fmt, ...) do {trace_fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__);} while(0)

#ifdef _DEBUG
	#define TRACE		printf
	// trace with function and line number
	#define TRACEX		printf("[%s-->%s:%d] [%ld]-- ", __FILE__, __FUNCTION__, __LINE__, (long)time(NULL)), printf
	#define  _TRACE_PRINT_
#else
	#define TRACE   1 ? (void)0 : (void)printf
	#define TRACEX	1 ? (void)0 : (void)printf
#endif

#ifndef ASSERT		// Protocol defined in assert.h
	#define ASSERT		assert
#endif

#ifndef IN
	#define IN
	#define OUT
#endif

#ifndef MIN
	#define MIN(x,y)    ( ((x)<=(y)) ? (x) : (y) )
#endif

#ifndef MAX
	#define MAX(x,y)    ( ((x)>=(y)) ? (x) : (y) )
#endif

#ifndef SWAP
	#define SWAP(T, a, b)   do { T t = (a); (a) = (b); (b) = (t); } while(0)
#endif

#ifndef ABS
	#define ABS(a)	(((a) < 0) ? (-(a)) : (a))
#endif

#define EPSILON					0.00001
#define FLOAT_EQUAL(f1, f2)		(ABS(((f1)-(f2)))<=EPSILON)
#define FLOAT_EQUAL0(f)			(FLOAT_EQUAL((f), 0))
#define FLOAT_NOT_EQUAL(f1, f2)		(ABS(((f1)-(f2)))>EPSILON)
#define FLOAT_NOT_EQUAL0(f)			(FLOAT_NOT_EQUAL((f), 0))

#define _NOW_		(time(NULL))
#define STRING		char*



#ifndef DWORD_PTR
#define DWORD_PTR DWORD
#endif //DWORD_PTR


#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#endif //MAKEWORD

#ifndef MAKEWORDX
#define MAKEWORDX(a, b)      ((WORD)(((WORD)((DWORD_PTR)(a) & 0xfff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xf))) << 12))
#endif //MAKEWORD


#ifndef MAKELONG
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#endif //MAKELONG


#ifndef LOWORD
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#endif //LOWORD


#ifndef HIWORD
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#endif //HIWORD


#ifndef LOBYTE
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#endif //LOBYTE


#ifndef HIBYTE
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))
#endif //HIBYTE



// some basic const value for time
#define MS_PER_SEC		1000
#define US_PER_SEC		1000000
#define NS_PER_MS		1000000		//
#define NS_PER_SEC		1000000000

#define SEC_PER_HOUR	3600
#define SEC_PER_DAY		86400

#define MONTHLY_INTERVAL_TIME 					30*24*60*60
#define WEEKLY_INTERVAL_TIME  					7*24*60*60
#define DAILY_INTERVAL_TIME 					1*24*60*60

#define MS_TO_NS(ms)	((ms)*NS_PER_MS)
#define MS_TO_SEC(ms)	((ms)/MS_PER_SEC)
// Get the item of an array, the array must be defined as:
// ITEM_TYPE	array[] = { item, item2, ...};
// This macro can NOT be used to get the item of a pointer, ITEM_TYPE *pArray!
#define ITEM_OF( array )		(int)(sizeof(array) / sizeof(array[0]))
#ifndef MINLONG
	#define MINLONG     0x80000000
	#define MAXLONG     0x7fffffff  
#endif
#define MAX_ULONG	((unsigned long)-1)
#define MIN_FLOAT	-1000000

typedef union tagValue	
{
	long			lValue;
	float			fValue;
	unsigned long	ulValue;
	_DATE_TIME	dtValue;
	_ENUM		emValue;
	BYTE			abyValue[8];

	BYTE			*blobValue; //to store the binary raw data
}VAR_VALUE;


typedef enum tagVALUE_TYPE
{
	VALUE_TYPE_LONG,
	VALUE_TYPE_ULONG,
	VALUE_TYPE_DATETIME,
	VALUE_TYPE_FLOAT,
	VALUE_TYPE_ENUM,
	VALUE_TYPE_STRING,
	VALUE_TYPE_BLOB
}_VALUE_TYPE;

typedef enum tagParam_Type
{
	PARAM_TYPE_STATUS,
	PARAM_TYPE_CONTROL,
	PARAM_TYPE_SETTING
}PARAM_TYPE;

typedef struct tagObjPTR //for param/alarm that do not need para_type
{
	int			nId;
	int			nSubId;
}OBJ_PTR;

typedef OBJ_PTR STATPARAM_PTR;
typedef OBJ_PTR CTRLPARAM_PTR;
typedef OBJ_PTR SETTPARAM_PTR;
typedef OBJ_PTR ALARM_PTR;

typedef enum tagModuleType
{
	MODULE_TYPE_SAMPLER,
	MODULE_TYPE_REPORTER,
	MODULE_TYPE_DATAPROC
}MODULE_TYPE;

typedef struct tagModuleInfo
{
	MODULE_TYPE	emModuleType; //shall put in first place
	int			nModualId;
}MODULE_INFO;

//calculator related
struct tagExpOperator			
{
	BYTE	byOperatorType;	
	BYTE	byPriority;	
};
typedef struct tagExpOperator EXP_OPERATOR;

/* const element */
typedef float	EXP_CONST;


/* General Expression Element discription */
struct tagExpElement	
{
	BYTE 	byElementType;	/* defined by TypesUsedByExp enum const */
	void	* pElement;	
};
typedef struct tagExpElement EXP_ELEMENT, *PEXP_ELEMENT;

struct tagExpVariable			
{
#define EXP_DEV_SELF		(-1)
	int	nDeviceId;	 //-1 means itself
#define EXP_PARAM_STATUS	0
#define EXP_PARAM_CONTRL	1
#define EXP_PARAM_SETTING	2
#define EXP_PARAM_ALARM	3
	int	nParamType; //0--raw status value, 1--status, 2--ctrl, 3--sett, 4--alarm
#define EXP_PARAM_COMM_DEV	(-1)
#define EXP_PARAM_COMM_SAM	(-2)
	int	nParamID;
};
typedef struct tagExpVariable EXP_VARIABLE;

#define EXP_MAX_ELEMENTS	30

#define DEFINE_CLUSTER(type)         \
	typedef struct tagCluster_##type  \
	{                           \
		type		*p;  \
		UINT		nTotal;    \
	}CLUSTER_##type

DEFINE_CLUSTER(EXP_ELEMENT);

#define DEFINE_LINKTABLE(type)	\
	typedef struct tagLinkTable_##type	\
	{		\
		type						it;	\
		type						*next;	\
	}LINKTABLE_##type

#define DLLExport 

#define	SAMP_INVALID_VALUE			(-999999)

typedef enum tagGUNState
{
	GUN_STATE_IDLE = 0,
	GUN_STATE_NOT_INSTALL,
	GUN_STATE_FAULTED, //new state: this gun is not usable! 
	GUN_STATE_LOCKED,
	GUN_STATE_RESERVE_PENDING, //reserve pending 
	GUN_STATE_WAITING_START,
	GUN_STATE_HANDSHAKE, //Before CHM/CRM, and self-check
	GUN_STATE_PREPARE_4CHARGE, //Before Charging
	GUN_STATE_CHARGING,
	GUN_STATE_CHARGE_PAUSED,
	GUN_STATE_WAITING_END,
	GUN_STATE_CHARGE_FINISHED
}GUN_STATE;

#define MAX_SUPPORT_GUN_NUM		6 	//stBatt system max support 6-gun

//Jimmy 20200902: re-placed it here for GUI_Component
typedef enum tagLangTy
{
	LANG_TYPE_CHINESE = 0,
	LANG_TYPE_ENGLISH,
    LANG_TYPE_ITALIAN,
    LANG_TYPE_CHINESE_TRADITIONAL,
    LANG_TYPE_GERMAN,
    LANG_TYPE_SPNANISH,
    LANG_TYPE_PORTUGUESE,
	LANG_TYPE_FRENCH,
	LANG_TYPE_AUSTRIAN_CERTIFICATION,
	LANG_TYPE_MAX
}LANG_TYPE;

// LJH241101 MU3
typedef enum tagSystem_Type
{
	SYSTYPE_ESS_AIR = 0,       //  风冷   电池单体温度精度为1，
	SYSTYPE_ESS_LIQUIDED,  //  液冷   精度0.1，EU3和BSU升级发送数据需要 -2，
	SYSTYPE_EMS_STATION,   // 站级EMS
	// SYSTYPE_SCADA_LOCALCTRL, // 本地控制器
	// SYSTYPE_SCADA_INPCS,     // PCS监控
	// SYSTYPE_SCADA_INPCS235KW,      // 235kW PCS 监控
	SYSTYPE_MAX,
 } SYSTEM_TYPE;

 #define MASK_NUM_LC_2nd    (100000)
 #define MASK_NUM_LC_4cab   (1000)
 typedef enum tagLC_Type
{
	LC_TYPE_1st,
	LC_TYPE_2nd,
	LC_TYPE_3rd,
	LC_TYPE_4cab
} LC_TYPE;

#define MAX_SINGLE_RECT_NUM     (10)

#endif // ifndef __BASE_TYPES_H
