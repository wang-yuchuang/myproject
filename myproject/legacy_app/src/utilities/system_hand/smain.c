#include "handler.h"
#include "smain.h"
int main(void)
{
	//建立这个文件的目的是为了app端可以通过脚本判断本进程已经启动起来了，避免重复启动
	FILE	*fp = fopen(_SYSHAND_PID_FILE,"w+t");
	if (fp == NULL)
	{
		printf("\n------Error: SystemHandler already run, do not start it TWICE!!!--------\n");
		return 1;
	}
	fprintf(fp, "%d\n", getpid());
	fclose(fp);
	chmod(_SYSHAND_PID_FILE,0777);
	//fprintf(fp, "%d\n", getpid());
	//fclose(fp);
	//close(fd);
	//------------Jimmy 20200306: the following is just for upgrading back-forward compitible script----------
//#ifndef _DEBUG
	fp = fopen(_EXTENSION_INST_FILE,"r");
	if (fp != NULL)
	{
		fclose(fp);
		printf("\n[sys_hand]--Now run ext_install script for BSP update...\n");
		//system("touch /var/.ext_mark");
		//system("source "_EXTENSION_INST_FILE); //execute the 
		//sleep(3);
		//Jimmy, just extcute it without reboot in script file itself		
		FILE* pf = popen(_EXTENSION_INST_FILE" -no_reboot", "r");		
		int bNeedReboot = 0;
		if(pf != NULL)
		{
			char tmp[1024]; //read each line text
			while (fgets(tmp, sizeof(tmp), pf) != NULL) 
			{
				//printf("\n--Line:[%s]\n", tmp);
				if (strstr(tmp, "reboot=1") != 0) 
				{
					//printf("\n--FOUNDDDFREFn");
					bNeedReboot = 1;
					//Jimmy removed break here 20200910,should continue run to end
					//break;
				}
			}
			pclose(pf);
		}

		if(bNeedReboot)		
		{
			unlink(_SYSHAND_PID_FILE);	
			char *p = getenv("EXTENS_MARK_FILE");
			if(p)
			{
				printf("\n[SystemHand]:now we create install_done mark\n");
				char szCreateExtMark[64];
				sprintf(szCreateExtMark, "touch %s", p);
				system(szCreateExtMark);	
			}
			printf("\n[SystemHand]:Now run install.sh OK, need reboot system\n");								
			system("reboot");
			return 1;
		}
	}	
	//------------------------end----------------------
//#endif	

	printf("\n------SystemHand[%s] Started OK!!!------ \n", _SYSHAND_PID_FILE);

	Main_Loop();

	unlink(_SYSHAND_PID_FILE);

	return 0;
}

