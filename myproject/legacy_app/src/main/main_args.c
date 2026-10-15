#include "main.h"
#include "3rd_party/sqlite/sqlite3.h"

#ifdef _HAS_FLASH_MEM
#define _APP_PID_FILE	 "/opt/appdata/data/immu2.pid" //这个文件用于app启动时判断该进程是否启动/**/
#else
#define _APP_PID_FILE	 "./immu2.pid" //x86 platform, just for test purpose
#endif

typedef int	(*CMD_OPTION_PROC)(char *pszOption);

struct _CMD_LINE_PARSER
{
	char				*pszOptName;
	CMD_OPTION_PROC		pfnParser;
	char				*pszOptHelp;
};

typedef struct _CMD_LINE_PARSER		CMD_LINE_PARSER;

#define _DEF_CMD_LINE_PARSER(op, parser, hlp)	\
	{ (op), (CMD_OPTION_PROC)(parser), (hlp)}

static char *s_pszMainPidFile = NULL;

static int Main_CreatePidFile(IN char *pszPidFile)
{
	FILE	*fp = fopen(pszPidFile,"w+t");

	if (fp == NULL)
	{
		return 1; // continue.
	}

	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	chmod(pszPidFile, 0777);

	s_pszMainPidFile = pszPidFile;

	return 0;
}

static int Main_ShowVersion(IN char *pszContFlag)
{
	//TRACE("\nAPP_MAIN_VER=%s\n", s_pChargeThis->info.szSWVer);
	//printf("APP_APPLICATION_BUILT_DATE=%s %s\n", __DATE__, __TIME__);
	char	szDBFile[256] = {0};
	sqlite3 *cfg_db;
	int n;
	MakeFullFileName(szDBFile, \
		getenv(ENV_VARIABLE_APP_PATH), "config", CHARGER_CONFIG_DB);
	int rc = sqlite3_open_v2( szDBFile, &cfg_db, SQLITE_OPEN_READONLY, NULL);
	if(rc != SQLITE_OK)
	{
		TRACE("Show version ERROR: can not create cfg database!");
		return FALSE;
	}

	sqlite3_stmt * stat;
	char szTime[32];

	TRACE("Idx \t Main_AppVer \t Sub_Ver \t ReleaseDate \t CfgAuthor \t Description \n");
	sqlite3_prepare_v2(cfg_db, "SELECT * FROM Revisions", -1, &stat, NULL);	
	while(sqlite3_step(stat) == SQLITE_ROW)
	{
		char* szDate = (char*)sqlite3_column_text(stat, 3);
		//TRACE("----szDate = %s---\n", szDate);
		time_t tm = StringToTime(szDate, SQLITE_DATE_FMT);
		TRACE("%d \t %s \t %s \t %s \t %s \t %s \n",
			sqlite3_column_int(stat, 0), sqlite3_column_text(stat, 1), sqlite3_column_text(stat, 2),
			TimeToString(tm, TIME_CHN_FMT, szTime, 32),
			sqlite3_column_text(stat, 4), sqlite3_column_text(stat, 5));
	}

	sqlite3_close(cfg_db);
	if (strcmp(pszContFlag, "=continue") == 0)
	{
		return 0;	// OK, continue run;
	}

	return -1;	// do NOT continue.
}


int	Main_ProcessCmdLine(IN int argc, IN char *argv[])
{
	int					i, n, iLenOpt;
	CMD_LINE_PARSER		cmdLine[] =
	{
		_DEF_CMD_LINE_PARSER("-pid=", Main_CreatePidFile, 
			"Set pid file used by stopacu. Format: -pid=pid_file_name"),
		_DEF_CMD_LINE_PARSER("-version", Main_ShowVersion, 
			"Show version of application. Format: -version[=continue]"),
	};

	if(argc == 1) //无参数，直接用默认值/**/, added by Jimmy
	{
		Main_CreatePidFile(_APP_PID_FILE);
		return 0;
	}
	// help
	if (((argc > 1) && (strcmp(argv[1], "-?") == 0)) /*|| (argc == 1)*/)
	{
		printf("\nThe supported cmds line options of NCCU are:\n");

		for (n = 0; n < ITEM_OF(cmdLine); n++)
		{
			printf("\t%d: %s.\n", (n+1), cmdLine[n].pszOptHelp);
		}

		printf("\tNote: the args in [] are optional.\n");

		return -1;
	}

	// parse options
	for (i = 1; i < argc; i++)
	{
		for (n = 0; n < ITEM_OF(cmdLine); n++)
		{
			iLenOpt = strlen(cmdLine[n].pszOptName);
			ASSERT(iLenOpt > 0);

			if (strncmp(argv[i], cmdLine[n].pszOptName, (size_t)iLenOpt) ==0)
			{
				int nRet = cmdLine[n].pfnParser(&argv[i][iLenOpt]);

				if (nRet > 0)
				{
					printf("Error on processing command arg: %s.", argv[i]);
				}
				else if (nRet == -1)
				{
					return -1;	// do NOT continue;
				}

				break;
			}
		}

		if (n == ITEM_OF(cmdLine))
		{
			printf("Unknown command arg: %s.", argv[i]);
			printf("\nThe supported cmd line options of IMAU are:\n");

			for (n = 0; n < ITEM_OF(cmdLine); n++)
			{
				printf("\t%d: %s.\n", (n+1), cmdLine[n].pszOptHelp);
			}

			printf("\tNote: the args in [] are optional.\n");

			return -1;
		}
	}

	return 0;
}

