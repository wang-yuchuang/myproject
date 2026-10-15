/*==========================================================================*
 *    Copyright(c) 2016,InfyPower Co., Ltd.
 *                     ALL RIGHTS RESERVED
 * 
 *  FILENAME : 
 *  CREATOR  : Jimmy Wu                   DATE: 2017-10-16
 *  VERSION  : V1.00
 *  PURPOSE  : 
 *
 *  HISTORY  :
 *==========================================================================*/

#include "data_def.h"
#include <sys/types.h>

STARTUP_DATA_STRU* g_pStartupDataStru;
#define FB_DEV					"/dev/fb0"
#define TS_EVENT_DEV			"/dev/input/event0"
#define TOUCH_SCREEN_NAME		"/dev/imx_ts"
#define LED_DEV				"/dev/led"
//Touch screen
#define TOUCH_DRIVE_MAJOR		236
#define IOCTL_TOUCH_ENABLE		_IOW(TOUCH_DRIVE_MAJOR, 1, unsigned long)
#define IOCTL_TOUCH_DISABLE		_IOW(TOUCH_DRIVE_MAJOR, 2, unsigned long)

//Buzz
#define LED_MAJOR				251
#define IOCTL_BEEP_100			_IOW(LED_MAJOR, 5, unsigned long)
//#define LCD_ON_BRIGHTNESS		4
#define LCD_ON_BRIGHTNESS		3
#define IOCTL_BLIGHT_PWM		_IOW(LED_MAJOR, 6, unsigned long)

static void printFixedInfo(struct fb_fix_screeninfo finfo)
{
    printf("Fixed screen info:\n"
	   "\tid:          %s\n"
	   "\tsmem_start:  0x%lx\n"
	   "\tsmem_len:    %d\n"
	   "\ttype:        %d\n"
	   "\ttype_aux:    %d\n"
	   "\tvisual:      %d\n"
	   "\txpanstep:    %d\n"
	   "\typanstep:    %d\n"
	   "\tywrapstep:   %d\n"
	   "\tline_length: %d\n"
	   "\tmmio_start:  0x%lx\n"
	   "\tmmio_len:    %d\n"
	   "\taccel:       %d\n"
	   "\n",
	   finfo.id, finfo.smem_start, finfo.smem_len, finfo.type,
	   finfo.type_aux, finfo.visual, finfo.xpanstep, finfo.ypanstep,
	   finfo.ywrapstep, finfo.line_length, finfo.mmio_start,
	   finfo.mmio_len, finfo.accel);
}

static void printVariableInfo(struct fb_var_screeninfo vinfo)
{
    printf("Variable screen info:\n"
	   "\txres:           %d\n"
	   "\tyres:           %d\n"
	   "\txres_virtual:   %d\n"
	   "\tyres_virtual:   %d\n"
	   "\tyoffset:        %d\n"
	   "\txoffset:        %d\n"
	   "\tbits_per_pixel: %d\n"
	   "\tgrayscale: %d\n"
	   "\tred:    offset: %2d, length: %2d, msb_right: %2d\n"
	   "\tgreen:  offset: %2d, length: %2d, msb_right: %2d\n"
	   "\tblue:   offset: %2d, length: %2d, msb_right: %2d\n"
	   "\ttransp: offset: %2d, length: %2d, msb_right: %2d\n"
	   "\tnonstd:       %d\n"
	   "\tactivate:     %d\n"
	   "\theight:       %d\n"
	   "\twidth:        %d\n"
	   "\taccel_flags:  0x%x\n"
	   "\tpixclock:     %d\n"
	   "\tleft_margin:  %d\n"
	   "\tright_margin: %d\n"
	   "\tupper_margin: %d\n"
	   "\tlower_margin: %d\n"
	   "\thsync_len:    %d\n"
	   "\tvsync_len:    %d\n"
	   "\tsync:         %d\n"
	   "\tvmode:        %d\n"
	   "\n",
	   vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual,
	   vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel, vinfo.grayscale,
	   vinfo.red.offset, vinfo.red.length, vinfo.red.msb_right,
	   vinfo.green.offset, vinfo.green.length, vinfo.green.msb_right,
	   vinfo.blue.offset, vinfo.blue.length, vinfo.blue.msb_right,
	   vinfo.transp.offset, vinfo.transp.length, vinfo.transp.msb_right,
	   vinfo.nonstd, vinfo.activate, vinfo.height, vinfo.width,
	   vinfo.accel_flags, vinfo.pixclock, vinfo.left_margin,
	   vinfo.right_margin, vinfo.upper_margin, vinfo.lower_margin,
	   vinfo.hsync_len, vinfo.vsync_len, vinfo.sync, vinfo.vmode);
}

static sem_t* psemWaitCmd = NULL; //�ȴ�����
static sem_t* psemAnswer = NULL; //����Ӧ�����
static int shmid_cmd = -1; //cmd������id,����app���� (��Ϊ����syshand����)/**/

static BOOL InitShm(void)
{
	//���¿����źŵ�/**/
	psemWaitCmd =  sem_open(_STP_SEM_CMDWAIT_PATH, O_CREAT,0666, 0);//�ź�����ʼֵΪ0
	if(psemWaitCmd == SEM_FAILED || psemWaitCmd == NULL)
	{
		printf( "[startup_fbShow]:sem_open waitCmd Failed!\n" );
		return FALSE;
	}
	psemAnswer =  sem_open(_STP_SEM_ANSWER_PATH, O_CREAT,0666, 0);//�ź�����ʼֵΪ0
	if(psemAnswer == SEM_FAILED || psemAnswer == NULL)
	{
		printf( "[startup_fbShow]:sem_open semAnswer Failed!\n" );
		return FALSE;
	}
	//printf("\n\n[sysHandler]:init sem OK");
	//���´��������ڴ���/**/
	key_t keySys = _STP_KEY_T_SYSHAND;
	//key_t keySys = IPCKEY;
	//printf("\n\n[sysHandler]:keyt is [%d]",keySys);
	//sleep(5);
	shmid_cmd = shmget(keySys, sizeof(SHM_DATA), 0666 | IPC_CREAT);
	if(shmid_cmd == -1)
	{
		printf("\n\n[startup_fbShow]:shmget ini ERRRRR!!!\n");
		return FALSE;
	}
	{
		char* p = (char*)shmat(shmid_cmd, NULL, 0);
		if(p != (char*)-1) { memset(p, 0, sizeof(SHM_DATA)); shmdt(p); }
	}
	//system("reboot");
	return TRUE;
}


//int main(int argc, char **argv)
//{
//    long int screensize = 0;
//    int doGraphicsMode = 1;
//    long oldKdMode = KD_TEXT;
//    const char *devfile = "/dev/fb0";
//    int nextArg = 1;
//
//    if (nextArg < argc) {
//	    if (strncmp("nographicsmodeswitch", argv[nextArg],
//		    strlen("nographicsmodeswitch")) == 0)
//	    {
//		    ++nextArg;
//		    doGraphicsMode = 0;
//	    }
//    }
//    if (nextArg < argc)
//	    devfile = argv[nextArg++];
//
//    /* Open the file for reading and writing */
//    fbFd = open(devfile, O_RDWR);
//    if (fbFd == -1) {
//	perror("Error: cannot open framebuffer device");
//	exit(1);
//    }
//    printf("The framebuffer device was opened successfully.\n\n");
//
//    /* Get fixed screen information */
//    if (ioctl(fbFd, FBIOGET_FSCREENINFO, &finfo) == -1) {
//	perror("Error reading fixed information");
//	exit(2);
//    }
//
//    //printFixedInfo();
//
//    /* Figure out the size of the screen in bytes */
//    screensize = finfo.smem_len;
//
//    /* Map the device to memory */
//    frameBuffer = (char *)mmap(0, screensize,
//                               PROT_READ | PROT_WRITE, MAP_SHARED,
//                               fbFd, 0);
//    fb_bak = (char*)calloc(screensize,sizeof(char));
//    if (frameBuffer == MAP_FAILED) {
//	perror("Error: Failed to map framebuffer device to memory");
//	exit(4);
//    }
////    printf("The framebuffer device was mapped to memory successfully.\n"
////           "\n");
//
//    //if (doGraphicsMode)
//    //    oldKdMode = switchToGraphicsMode();
//
//    /* Get variable screen information */
//    if (ioctl(fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
//	perror("Error reading variable information");
//	exit(3);
//    }
//
//    //printVariableInfo();
//
//    //performSpeedTest(frameBuffer, screensize);
//
//   // initPalette();
//
//    if (paletteSize == 0) {
////        printf("Will draw 3 rectangles on the screen,\n"
////               "they should be colored red, green and blue (in that order).\n");
////        drawRect(vinfo.xres / 8, vinfo.yres / 8,
////                 vinfo.xres / 4, vinfo.yres / 4,
////                 0xffff0000);
////        drawRect(vinfo.xres * 3 / 8, vinfo.yres * 3 / 8,
////                 vinfo.xres / 4, vinfo.yres / 4,
////                 0xff00ff00);
////        drawRect(vinfo.xres * 5 / 8, vinfo.yres * 5 / 8,
////                 vinfo.xres / 4, vinfo.yres / 4,
////                 0xff0000ff);
//        //�Ƚ���Ļȫ�����ɰ�ɫ/**/
//        drawRect(0, 0,
//                 vinfo.xres, vinfo.yres,
//                 0xffFFFFFF);
//    } else {
//        printf("Will rectangles from the 16 first entries in the color palette"
//               " on the screen\n");
//        int y;
//        int x;
//        for (y = 0; y < 4; ++y) {
//            for (x = 0; x < 4; ++x) {
//                drawRect(vinfo.xres / 4 * x, vinfo.yres / 4 * y,
//                         vinfo.xres / 4, vinfo.yres / 4,
//                         4 * y + x);
//            }
//        }
//    }
//
//    //sleep(5);
//
//   // resetPalette();
//
//   // printf("  Done.\n");
//
//    //if (doGraphicsMode)
//    //    restoreTextMode(oldKdMode);
//
//    munmap(frameBuffer, screensize);
//    close(fbFd);
//    free(fb_bak);
//    return 0;
//}

static BOOL SetTS_OnOff(BOOL bOn)
{
	if(g_pStartupDataStru->nTsFd < 0)
	{
		g_pStartupDataStru->nTsFd = open(TOUCH_SCREEN_NAME, O_RDWR);
		if(g_pStartupDataStru->nTsFd < 0)
		{
			return FALSE;
		}
	}
	ioctl(g_pStartupDataStru->nTsFd, bOn ? IOCTL_TOUCH_ENABLE : IOCTL_TOUCH_DISABLE, 1);
	close(g_pStartupDataStru->nTsFd);
	return TRUE;
}

//static BOOL OpenLCDBackLight(void)
//{
//	//�򿪱��⣬����Kernel�Ǳߴ򿪻�������kernal�Ǳ�ȥ���ˣ�ֱ������ߴ򿪼���/**/
//	int fd = open("/dev/led", O_RDWR);
//	if(fd <= 0)
//	{
//		fd = open("/dev/led", O_RDWR);
//		if(fd <= 0)
//		{
//			return FALSE;
//		}
//	}
//	if(fd > 0)
//	{
//		ioctl(fd, IOCTL_BLIGHT_PWM, LCD_ON_BRIGHTNESS);
//		close(fd); //Close it when used up
//	}
//	else
//	{
//		return FALSE;
//	}
//	return TRUE;
//}
static void CleanAll(void)
{
	//SetTS_OnOff(FALSE);
	munmap(g_pStartupDataStru->p_fbmmap, g_pStartupDataStru->nFbSize);
	close(g_pStartupDataStru->nFd_Fb);
	//close(g_pStartupDataStru->nTsFd);
	close(g_pStartupDataStru->nTs_Event);
	DELETE(g_pStartupDataStru->p_fbmemBak);
	int n;
	int m;
	UI_PAGE* pPage;
	for(n = 0; n < g_pStartupDataStru->ui.nTotalPages; n++)
	{
		pPage = g_pStartupDataStru->ui.pPages + n;
		for(m = 0; m < pPage->nTotalBtns; m++)
		{
			DELETE(pPage->pBtns[m].pPressedRGBData);
			DELETE(pPage->pBtns[m].base.pRGBData);
		}
		for(m = 0; m < pPage->nTotalCtrls; m++)
		{
			DELETE(pPage->pCtrls[m].pRGBData);
		}
	}
	DELETE(g_pStartupDataStru->ui.pPages);
	QRcode_free(g_pStartupDataStru->pQrCode_A);
	QRcode_free(g_pStartupDataStru->pQrCode_B);
	DELETE(g_pStartupDataStru);
	shmctl(shmid_cmd, IPC_RMID, 0); //ɾ�������ڴ�/**/
	sem_destroy(psemWaitCmd);
	sem_destroy(psemAnswer);
	system("rm -f /var/startup_show.pid"); //for app to judge if startup process is startup
}

//int Buzz_200(void)
//{
//	int fd = open("/dev/led", O_RDWR);
//	if(fd > 0)
//	{
//		ioctl(fd, IOCTL_BEEP_100, 2);//2 means 200ms
//		close(fd); //Close it when used up
//	}
//	return 0;
//}

static void ResetBtnStatus(void)
{
	UI_PAGE* pPage = g_pStartupDataStru->pCurrPage;
	UI_BUTTON* pBtn = NULL;
	int n;
	for(n = 0; n < pPage->nTotalBtns; n++)
	{
		pBtn = pPage->pBtns + n;
		pBtn->bIsMouseDown = FALSE;
	}
}

static UI_BUTTON* JudgeButtonPressed(int x, int y)
{
	int n;
	UI_PAGE* pPage = g_pStartupDataStru->pCurrPage;
	UI_BUTTON* pBtn = NULL;
	BOOL bHasBtnPressed = FALSE;
	for(n = 0; n < pPage->nTotalBtns; n++)
	{
		pBtn = pPage->pBtns + n;
		if(pBtn->bIsMouseDown)
		{
			return NULL; //Has button not handled
		}
	}

	for(n = 0; n < pPage->nTotalBtns; n++)
	{
		pBtn = pPage->pBtns + n;
		if((x >= pBtn->base.x) && (x <= pBtn->base.x + pBtn->base.width) &&
			(y >= pBtn->base.y) && (y <= pBtn->base.y + pBtn->base.height) && 
			(pBtn->pfnPressHandler != NULL))
		{
			pBtn->bIsMouseDown = TRUE;
			bHasBtnPressed = TRUE;
			break;
		}
	}
	return (bHasBtnPressed) ? pBtn : NULL;
}
static char* strinclude(char* strSrc, char* strDst)
{
	int nLen = strlen(strDst);
	char *p = strSrc;
	int n;
	
	//remove space
	while (*p == ' ')
	{
		p++;
	}
	for(n = 0; n < nLen; n++)
	{
		if(*p != strDst[n])
		{
			return NULL;
		}
		p++;
	}
	return p;
}

static BOOL LoadStartSettings(void)
{
	FILE* pFile = fopen(START_SETTING_FILE, "r");
	if(pFile == NULL)
	{
		g_pStartupDataStru->settings.cAuxType_A = '0';
		g_pStartupDataStru->settings.cAuxType_B = '0';
		memcpy(g_pStartupDataStru->settings.szQrCode_A, "error: undefined(A)", 19);
		memcpy(g_pStartupDataStru->settings.szQrCode_B, "error: undefined(B)", 19);
		printf("CAN NOT open startup setting file-->%s!\n", START_SETTING_FILE);
		return FALSE;
	}
	struct stSettingText
	{
		char*	szId;
		char*	pAttrib;
		int		nLen;
	};
	struct stSettingText settings[] = 
	{
		{"PSW=",		g_pStartupDataStru->settings.szPasswd,		6},
		{"QR_A=",		g_pStartupDataStru->settings.szQrCode_A,		128},
		{"QR_B=",		g_pStartupDataStru->settings.szQrCode_B,		128},
		{"AUX_A=",	&g_pStartupDataStru->settings.cAuxType_A,	1},
		{"AUX_B=",	&g_pStartupDataStru->settings.cAuxType_B,	1},
	};
#define ITEM_OF( array )		(int)(sizeof(array) / sizeof(array[0]))
#define MIN(x,y)    ( ((x)<=(y)) ? (x) : (y) )
	int n;
	char	line[256];
	char* p;
	int nCopyLen = 0;
	do 
	{
		if(fgets( line, 256, pFile) == NULL)
		{
			break;
		}
		//ȥ�����з�/**/
		int nLLL = strlen(line);
		if(nLLL > 0)
		{
			for(n = nLLL - 1; n >= 0; n--)
			{
				if(line[n] == '\n' || line[n] == '\r')
				{
					line[n] = '\0';
				}
				else
				{
					break;
				}
			}
		}
		//printf("line = [%s], nLLL = %d ,strlen = %d\n", line, nLLL, strlen(line));
		for(n = 0; n < ITEM_OF(settings); n++)
		{
			p = strinclude(line, settings[n].szId);
			if(p != NULL)
			{
				memset(settings[n].pAttrib, 0, settings[n].nLen);
				nCopyLen = MIN(settings[n].nLen, strlen(line) - strlen(settings[n].szId));
				if(nCopyLen > 0)
				{
					memcpy(settings[n].pAttrib, p, nCopyLen);
					//printf("setting-->%s[%s], copyLen = %d\n", settings[n].szId, settings[n].pAttrib, nCopyLen);
				}
			}
		}
	} while (1);
	
	fclose(pFile);
	return TRUE;
}

BOOL SendMsg2App(SHM_DATA* pOrgData)
{
	SHM_DATA* pCmd = (SHM_DATA*)shmat(shmid_cmd,0,0);  
	if(pCmd == (SHM_DATA*)-1)
	{
		printf("--------Can not map SHmdaaaaa-----------\n");
		return FALSE;
	}
	memcpy(pCmd, pOrgData, sizeof(SHM_DATA));
	//printf("____Now successfully send Msg-->%d to App___\n", pCmd->emType);
	if(shmdt((void*)pCmd) == -1)
	{
		printf("\n\n[startup_show]:shmdt is fail\n");
		return FALSE;
	}
	sem_post(psemWaitCmd); //���������App��/**/
	return TRUE;
}

int main(int argc, char **argv)
{
	g_pStartupDataStru = NEW(STARTUP_DATA_STRU, 1);
	if(g_pStartupDataStru == NULL)
	{
		perror("Error: cannot create dataStru!");
		exit(1);
	}
	/* Open the file for reading and writing */
	g_pStartupDataStru->nFd_Fb = open(FB_DEV, O_RDWR);
	if (g_pStartupDataStru->nFd_Fb == -1)
	{
		perror("Error: cannot open framebuffer device");
		exit(1);
	}
	//printf("The framebuffer device was opened successfully.\n\n");
	
	/* Get fixed screen information */
	if (ioctl(g_pStartupDataStru->nFd_Fb, FBIOGET_FSCREENINFO, &g_pStartupDataStru->screen_finfo) == -1) 
	{
		perror("Error reading fixed information");
		exit(2);
	}

	//printFixedInfo(g_pStartupDataStru->screen_finfo);
	
	/* Figure out the size of the screen in bytes */
	int screensize = g_pStartupDataStru->screen_finfo.smem_len;
	g_pStartupDataStru->nFbSize = screensize;
	/* Map the device to memory */
	g_pStartupDataStru->p_fbmmap = (FB_PIXEL *)mmap(0, g_pStartupDataStru->nFbSize,
		PROT_READ | PROT_WRITE, MAP_SHARED,
		g_pStartupDataStru->nFd_Fb, 0);
	g_pStartupDataStru->p_fbmemBak = (FB_PIXEL*)NEW(char, g_pStartupDataStru->nFbSize);
	if ((g_pStartupDataStru->p_fbmmap == MAP_FAILED) || (g_pStartupDataStru->p_fbmemBak == NULL))
	{
		perror("Error: Failed to map framebuffer device to memory");
		exit(4);
	}

	/* Get variable screen information */
	if (ioctl(g_pStartupDataStru->nFd_Fb, FBIOGET_VSCREENINFO, &g_pStartupDataStru->screen_vinfo) == -1) 
	{
		perror("Error reading variable information");
		exit(3);
	}
	
	//printVariableInfo();

	g_pStartupDataStru->nScreenHeight = g_pStartupDataStru->screen_vinfo.yres;
	g_pStartupDataStru->nScreenWidth = g_pStartupDataStru->screen_vinfo.xres;
	
	//printf("The framebuffer device was mapped to memory successfully.\n");

	system("touch /var/startup_show.pid"); //for app to judge if startup process is startup
	
	g_pStartupDataStru->ui.nTotalPages = PAGE_MAX_SUPPORT; //��Ҫ��ʼ��/**/
	g_pStartupDataStru->ui.pPages = NEW(UI_PAGE, PAGE_MAX_SUPPORT);
	if(g_pStartupDataStru->ui.pPages == NULL)
	{
		close(g_pStartupDataStru->nFd_Fb);
		munmap(g_pStartupDataStru->p_fbmmap, screensize);
		exit(1);
	}
	
	//if(!OpenLCDBackLight())
	//{
	//	printf("cannot open lcd backlight!\n");
	//	close(g_pStartupDataStru->nFd_Fb);
	//	munmap(g_pStartupDataStru->p_fbmmap, screensize);
	//	exit(1);
	//}

	g_pStartupDataStru->pCurrPage = ShowPage(PAGE_LOGO_SHOW);

	//if(!InitShm())
	//{
	//	//close(g_pStartupDataStru->nFd_Fb);
	//	//close(g_pStartupDataStru->nTs_Event);
	//	//munmap(g_pStartupDataStru->p_fbmmap, screensize);
	//	CleanAll();	
	//	exit(1);
	//}

	//int nWaitTimes = 0;
	////�ȴ��������ļ��Ƿ��Ѿ�����, ��һ�β����ڣ�����ȴ�app�Ǳ�ͬ�������/**/
	//do 
	//{
	//	//����ʱ���ܻ�ȴ��Ͼã���Ϊapp�Ǳ�Ҫ����/**/
	//	if((access(START_SETTING_FILE, R_OK | W_OK ) == 0))
	//	{
	//		break;
	//	}
	//	usleep(500 * 1000);
	//	nWaitTimes++;
	//	if(nWaitTimes > 30) //max wait 15sec
	//	{
	//		//printf("cannot open %s\n", START_SETTING_FILE);
	//		//close(g_pStartupDataStru->nFd_Fb);
	//		//munmap(g_pStartupDataStru->p_fbmmap, screensize);
	//		CleanAll();	
	//		exit(1);
	//	}
	//} while (1);

	//LoadStartSettings();
	//g_pStartupDataStru->pQrCode_A = QRcode_encodeString(g_pStartupDataStru->settings.szQrCode_A, 1, QR_ECLEVEL_H, QR_MODE_8, TRUE);
	//g_pStartupDataStru->pQrCode_B = QRcode_encodeString(g_pStartupDataStru->settings.szQrCode_B, 1, QR_ECLEVEL_H, QR_MODE_8, TRUE);

	//if(nWaitTimes < 2)
	//{
	//	usleep(1000 * 1000);
	//}

	g_pStartupDataStru->nTsFd = -1;

	//g_pStartupDataStru->nLedFd = open(LED_DEV, O_RDWR);
	//if(g_pStartupDataStru->nLedFd <= 0)
	//{
	//	close(g_pStartupDataStru->nFd_Fb);
	//	munmap(g_pStartupDataStru->p_fbmmap, screensize);
	//	exit(1);
	//}

	//g_pStartupDataStru->nTs_Event = open(TS_EVENT_DEV, O_RDONLY|O_NONBLOCK);
	//if(g_pStartupDataStru->nTs_Event < 0)
	//{
	//	//close(g_pStartupDataStru->nFd_Fb);
	//	//munmap(g_pStartupDataStru->p_fbmmap, screensize);
	//	CleanAll();	
	//	exit(1);
	//}
	
	//Now run and monitor until exit
	g_pStartupDataStru->bExit = FALSE;
	SHM_DATA* pShmData;
	int nReadSize;
	int x,y;
	int nEVSIZE = sizeof(struct input_event) * 2;
	int nNoPressCount = 0;
	UI_BUTTON* pBtnPressed;
	EVENT_ARG eventArg;
	int n = 0;
	memset(g_pStartupDataStru->szInputPasswd, 0, 7);
	//����Ƿ���Ҫ��������/**/
	g_pStartupDataStru->bLogInOK = FALSE;
	//g_pStartupDataStru->bReadMeConfirmed = FALSE;
	//g_pStartupDataStru->pCurrPage = ShowPage(PAGE_README);

	//һֱ�ȵ�app�����ſ��Ե����Ļ, ���10s�ԵȲ��������ٵ�/**/
	SHM_DATA shmdd;
	//shmdd.emType = SHM_STP_REQ_STATUS;
	//if(!SendMsg2App(&shmdd))
	//{
	//	printf("--------------ERRRRRRR send msg to app-----------\n");
	//	//close(g_pStartupDataStru->nFd_Fb);
	//	//close(g_pStartupDataStru->nTs_Event);
	//	//munmap(g_pStartupDataStru->p_fbmmap, screensize);
	//	CleanAll();	
	//	exit(1);
	//}
	//do 
	//{
	//	usleep(100 * 1000);
	//	n++;
	//	if(n >= 50)
	//	{
	//		//close(g_pStartupDataStru->nFd_Fb);
	//		//close(g_pStartupDataStru->nTs_Event);
	//		//munmap(g_pStartupDataStru->p_fbmmap, screensize);
	//		printf("No APP found in 5 sec, so quit!\n");
	//		CleanAll();	
	//		exit(1);
	//	}
	//} while (access ("/var/imau.pid", R_OK) != 0);
	//}while(sem_trywait(psemAnswer) != 0);

	//if(strlen(g_pStartupDataStru->settings.szPasswd) > 0)
	//{
	//	g_pStartupDataStru->pCurrPage = ShowPage(PAGE_LOGIN);
	//}
	//else
	//{
	//	g_pStartupDataStru->pCurrPage = ShowPage(PAGE_HOME);
	//}

	//if(!SetTS_OnOff(TRUE))
	//{
	//	printf("can not open touch screen, so quit!\n");
	//}
	////�ٷ���msg�ȵ��ظ�/**/
	//shmdd.emType = SHM_STP_REQ_STATUS;
	//SendMsg2App(&shmdd);
	//int nNoResponseCount = 0;
	////time_t tmNow = time(NULL);
	//while(!g_pStartupDataStru->bExit)
	//{
	//	//1. Monitor if any reply Event from APP
	//	if(sem_trywait(psemAnswer) == 0)
	//	{
	//		nNoResponseCount = 0;
	//		pShmData = (SHM_DATA*)shmat(shmid_cmd,0,0);  
	//		if(pShmData != (SHM_DATA*)-1)
	//		{
	//			//now analyze the reply
	//			if(pShmData->emType == SHM_APP_NOTIFY_EXIT)
	//			{
	//				printf("now received exit msg from app!!!\n");
	//				g_pStartupDataStru->bExit = TRUE; //now exit
	//			}
	//			shmdt((void*)pShmData);
	//			shmdd.emType = SHM_STP_REQ_STATUS;
	//			SendMsg2App(&shmdd); //�յ��ظ����������ַ�һ��/**/
	//		}
	//	}
	//	else
	//	{
	//		nNoResponseCount++;
	//		if(nNoResponseCount >= 1500) //30s��û�յ�app�ǱߵĻظ�,��ֱ���˳�/**/
	//		{
	//			break;
	//		}
	//	}

	//	//2. Monitor if any Event from UI
	//	//�������ް��£����º�ֻҪ̧����button�İ����¼�����/**/
	//	nReadSize = read(g_pStartupDataStru->nTs_Event, (void*) g_pStartupDataStru->event_xy, nEVSIZE);
	//	if(nReadSize == nEVSIZE)
	//	{
	//		//�����Ƿ���Ч���/**/
	//		if((g_pStartupDataStru->event_xy[0].type == EV_ABS) && (g_pStartupDataStru->event_xy[0].code == ABS_X) &&
	//			(g_pStartupDataStru->event_xy[1].type == EV_ABS) && (g_pStartupDataStru->event_xy[1].code == ABS_Y))
	//		{
	//			nNoPressCount = 0;
	//			x = g_pStartupDataStru->event_xy[0].value;
	//			y = g_pStartupDataStru->event_xy[1].value;
	//			//printf("[%d, %d] pressed!\n", x, y);
	//			pBtnPressed = JudgeButtonPressed(x, y);
	//			if(pBtnPressed != NULL)
	//			{
	//				eventArg.x = x;
	//				eventArg.y = y;
	//				eventArg.eType = _EVENT_MOUSE_DOWN;
	//				eventArg.pBtn = pBtnPressed;
	//				pBtnPressed->pfnPressHandler(&eventArg);
	//			}
	//		}
	//	}
	//	else
	//	{
	//		nNoPressCount++;
	//		if(nNoPressCount == 10)
	//		{
	//			//˵���а�������/**/
	//			//printf("Now found button up!\n");
	//			UI_BUTTON* pBtn = NULL;
	//			pBtnPressed = NULL;
	//			for(n = 0; n < g_pStartupDataStru->pCurrPage->nTotalBtns; n++)
	//			{
	//				pBtn = g_pStartupDataStru->pCurrPage->pBtns + n;
	//				if(pBtn->bIsMouseDown)
	//				{
	//					pBtnPressed = pBtn;
	//					break;
	//				}
	//			}
	//			if(pBtnPressed != NULL)
	//			{
	//				eventArg.x = x;
	//				eventArg.y = y;
	//				eventArg.pBtn = pBtnPressed;
	//				pBtnPressed->pfnPressHandler(&eventArg);
	//				if((x >= pBtnPressed->base.x) && (x <= pBtnPressed->base.x + pBtnPressed->base.width) &&
	//					(y >= pBtnPressed->base.y) && (y <= pBtnPressed->base.y + pBtnPressed->base.height) && 
	//					(pBtnPressed->pfnPressHandler != NULL))
	//				{
	//					eventArg.eType = _EVENT_MOUSE_UP;
	//					pBtnPressed->pfnPressHandler(&eventArg);
	//					//printf("Button [%s] is clicked!\n", pBtnPressed->base.szName);
	//					//ioctl(g_pStartupDataStru->nLedFd , IOCTL_BEEP_100, 2);//2 means 200ms
	//					//Buzz_200();
	//				}
	//				else
	//				{
	//					eventArg.eType = _EVENT_CANCEL_CLICK;
	//					pBtnPressed->pfnPressHandler(&eventArg);
	//					//printf("Button [%s] is canceled click!\n", pBtnPressed->base.szName);
	//				}
	//				ResetBtnStatus();
	//			}
	//		}
	//		else if(nNoPressCount > 10)
	//		{
	//			nNoPressCount = 11;
	//		}
	//	}
	//	usleep(20 * 1000); //sleep 20 ms
	//	if((g_pStartupDataStru->bLogInOK) && (g_pStartupDataStru->pCurrPage->nPageId == PAGE_LOGIN))
	//	{
	//		//char* strRawQr_A = "z1239000evgjjjat_4456"; //for Test only
	//		//char* strRawQr_B = "ihj_12300034567754bb"; //for Test only
	//		//g_pStartupDataStru->pQrCode_A = QRcode_encodeString(strRawQr_A, 1, QR_ECLEVEL_H, QR_MODE_8, TRUE);
	//		//g_pStartupDataStru->pQrCode_B = QRcode_encodeString(strRawQr_B, 1, QR_ECLEVEL_H, QR_MODE_8, TRUE);
	//		//usleep(200 * 1000); //sleep 100 ms
	//		shmdd.emType = SHM_STP_PASSWD_DONE;
	//		SendMsg2App(&shmdd);
	//		g_pStartupDataStru->pCurrPage = ShowPage(PAGE_HOME);
	//	}
	//	//else if((g_pStartupDataStru->bReadMeConfirmed) && (g_pStartupDataStru->pCurrPage->nPageId == PAGE_README))
	//	//{
	//	//	if(strlen(g_pStartupDataStru->settings.szPasswd) > 0)
	//	//	{
	//	//		g_pStartupDataStru->pCurrPage = ShowPage(PAGE_LOGIN);
	//	//	}
	//	//	else
	//	//	{
	//	//		g_pStartupDataStru->pCurrPage = ShowPage(PAGE_HOME);
	//	//	}
	//	//}
	//}

	//Now exit
	printf("------------------startup process is exited successfully now!---------------\n");
	CleanAll();	
	return 0;
}
