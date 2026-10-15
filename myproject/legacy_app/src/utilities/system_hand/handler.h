
#ifndef _SYSHANDLER_HH
#define _SYSHANDLER_HH
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "app_incs.h"

#define  MAX_CMD_CHARS		512 //最大512个字符
#define CMD_MEM_SIZE		1024 // (sizeof(CMD)) //命令区内存大小/**/
#ifdef _HAS_FLASH_MEM
	#define _SYSHAND_PID_FILE	 "/opt/appdata/data/syshand.pid" //这个文件用于app启动时判断该进程是否启动/**/
#else
	#define _SYSHAND_PID_FILE	 "./syshand.pid" //x86 platform, just for test purpose
#endif

#define _EXTENSION_INST_FILE "/app/extensions/install.sh"

//#define ANSWER_MWM_SIZE 8 //应答区内存大小/**/
struct _sCmdItem
{
	int iCmdType; //命令类型/**/
	char szCmd[MAX_CMD_CHARS]; //命令内容/**/
	int iCmdRet; //命令应答/**/
};
typedef struct _sCmdItem CMD;
enum _eErrRet
{
	ERR_OK = 0,
	ERR_NO_MEMORY,
	ERR_WRONG_CMD
};

int Main_Loop(void);


#endif

