#include <sys/types.h>
#include <stdarg.h>

#include "std_sys.h"
#include "app_incs.h"


int main(int argc, char *argv[])
{
	char szVarFile[128] = {0};
	char cmd[256] = {0};
	int nLogRecs;

	if(argc == 1)
	{
		MakeFullFileName(szVarFile,
			getenv(ENV_VARIABLE_VAR_PATH), NULL, "log.1");

		nLogRecs = LogCat(szVarFile, FALSE);
		printf("[Total found log records:%d]\n", nLogRecs);

		if(nLogRecs > 0)
		{
			sprintf(cmd, "cat %s", szVarFile);
			system(cmd);

			ZERO_POBJS(cmd, 256);
			sprintf(cmd, "rm -f %s", szVarFile);
			system(cmd);
		}
		return 1;
	}
	else if((argc == 2) && (strcmp(argv[1], "-delete") == 0))
	{
		Log_Delete(FALSE, LOG_DELTE_SYSLOG);
		return 1;
	}
	else if((argc == 3) && (strcmp(argv[1], "-delete") == 0) && (strcmp(argv[2], "-dump") == 0))
	{
		Log_Delete(FALSE, LOG_DELTE_DUMPLOG);
		return 1;
	}
	else if((argc == 3) && (strcmp(argv[1], "-delete") == 0) && (strcmp(argv[2], "-mbe") == 0))
	{
		Log_Delete(FALSE, LOG_DELTE_MBELOG);
		return 1;
	}
	else if((argc == 2) && (strcmp(argv[1], "-dump") == 0))
	{
		MakeFullFileName(szVarFile,
			getenv(ENV_VARIABLE_VAR_PATH), NULL, "log.1");

		nLogRecs = LogDump(szVarFile, FALSE);
		printf("[Total found Dumplog records:%d]\n", nLogRecs);

		if(nLogRecs > 0)
		{
			sprintf(cmd, "cat %s", szVarFile);
			system(cmd);

			ZERO_POBJS(cmd, 256);
			sprintf(cmd, "rm -f %s", szVarFile);
			system(cmd);
		}
		return 1;
	}
	else if((argc == 2) && (strcmp(argv[1], "-mbe") == 0))
	{
		MakeFullFileName(szVarFile,
			getenv(ENV_VARIABLE_VAR_PATH), NULL, "log.1");

		nLogRecs = Log_Dump_MBE(szVarFile, FALSE);
		printf("[Total found MBELog records:%d]\n", nLogRecs);

		if(nLogRecs > 0)
		{
			sprintf(cmd, "cat %s", szVarFile);
			system(cmd);

			ZERO_POBJS(cmd, 256);
			sprintf(cmd, "rm -f %s", szVarFile);
			system(cmd);
		}
		return 1;
	}
	printf("log_cat: invalid option, please check!\n");

	return 0;	
}

