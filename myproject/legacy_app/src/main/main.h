#ifndef __MAIN_H_
#define __MAIN_H_
#include "std_sys.h"
#include "app_incs.h"

// watch dog.
BOOL WatchDog_Open(void);
void WatchDog_Feed(void);
void WatchDog_Close(BOOL bDisableDog);
int	Main_ProcessCmdLine(IN int argc, IN char *argv[]);

BOOL Install_SigHandlers(void* pArgs);
void Main_Exit(int nExitCode);

#endif

