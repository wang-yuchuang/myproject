#ifndef __SHARE_DATA_DEF_H__
#define __SHARE_DATA_DEF_H__

#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/wait.h>

#define _STP_SEM_CMDWAIT_PATH "/_semStartupWait"
#define _STP_SEM_ANSWER_PATH "/_semStartupAnsw"
#define _STP_KEY_T_SYSHAND  (ftok("/home",25)) //syshand进程的ID
typedef enum tagShmType
{
	//From startup request
	SHM_STP_REQ_STATUS,
	SHM_STP_PASSWD_DONE,
	SHM_STP_SET_GUN_SEL,
	//From APP reply
	SHM_APP_REPLY_STAT,
	SHM_APP_NOTIFY_EXIT //app通知退出/**/
}SHM_DATA_TYPE;
#define  MAX_EXTRA_SHM_DATA	64 //最大64个字符
typedef struct _sShmData
{
	SHM_DATA_TYPE	emType;
	char				szExtraData[MAX_EXTRA_SHM_DATA]; //附加内容/**/
}SHM_DATA;

#define START_SETTING_FILE	"/home/startup/settings"

#endif
