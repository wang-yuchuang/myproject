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

extern STARTUP_DATA_STRU* g_pStartupDataStru;

static int LoadBMP24(char* szImgPath, RGB** ppRGB, int* pWidth, int *pHeight)
{
	FILE *fp;
	int rc;
	int flen = 0;
	int total_length = 0;

	if(szImgPath == NULL)
	{
		printf("path Error,return\n");
		return -1;
	}
	fp = fopen(szImgPath, "rb" );
	if(fp == NULL)
	{
		printf("open image file [%s] failed\n", szImgPath);
		return -1;
	}

	//求解文件长度/**/
	//fseek(fp,0,SEEK_SET);
	//fseek(fp,0,SEEK_END);
	//flen = ftell(fp);
	//printf("flen is %d\n",flen);

	/* 再移位到文件头部 */
	fseek(fp,0,SEEK_SET);

	BMP_FILE_HEADER f_head;
	rc = fread(&f_head, sizeof(BMP_FILE_HEADER),1, fp);
	if ( rc != 1)
	{
		printf("read header error!\n");
		fclose( fp );
		return -2;
	}

	//printf("-----the BMP file size = %d, offset = %d-------\n", f_head.file_size, f_head.offset);

	//检测是否是bmp图像/**/
	if (memcmp(f_head.map_id, "BM", 2) != 0)
	{
		printf("it's not a BMP file\n");
		fclose( fp );
		return -3;
	}

	BMP_INFO_HEADER i_head;
	rc = fread( (char *)&i_head, sizeof(BMP_INFO_HEADER),1, fp );
	if ( rc != 1)
	{
		printf("read infoheader error!\n");
		fclose( fp );
		return -4;
	}

	int width = i_head.width;
	int height = i_head.hight;

	//printf("--------the BMP map_size = %d----------\n", i_head.map_size);
	//printf("flen = %d\n", flen);
	//printf("width = %d, height = %d\n", width, height);

	//total_length = width * height *3; //这里规定要用24bit色图/**/
	//total_length = width * height *2; //使用RGB565，否则有失真/**/
	total_length = f_head.file_size - f_head.offset; //总大小, 必须为4的倍数/**/

	//跳转到数据区/**/
	fseek(fp, f_head.offset, SEEK_SET);
	//printf(" FileHead.cfoffBits = %d\n",  pImage->pBmpRaw->f_head.offset);
	//printf(" InfoHead.bpp = %d\n",  pImage->pBmpRaw->i_head.bpp);


	//每行字节数/**/
	BYTE* pbyRaw = NEW(BYTE, total_length);
	rc = fread(pbyRaw, 1, total_length, fp);
	if(rc <= 0)
	{
		fclose( fp );
		DELETE(pbyRaw);
		return -6;
	}
	//printf("----------Read total len = %d, ssssssssssssss len = %d---------\n", rc, width * height *3);

	//将数据转换暂存到Ctrl的data域/**/
	int x1, y1;
	int offset;
	*ppRGB = (RGB*)NEW(char, total_length);
	if(pWidth != NULL)
	{
		*pWidth = width;
	}
	if(pHeight != NULL)
	{
		*pHeight = height;
	}
	RGB* pDstData = *ppRGB;
	BYTE* pRawData = pbyRaw;
	//TRACE("-------nMaxX = %d, nMaxY = %d----------\n", nMaxX, nMaxY);
	//计算行偏移，注意BMP每行必须为4的倍数/**/
	int nLineOfSet = (width * 3) % 4;
	nLineOfSet = (nLineOfSet != 0) ? (4 - nLineOfSet) : 0;
	//printf("-------w = %d, h = %d ,nLineOffSet = %d----------\n", width, height, nLineOfSet);
	for (y1 = 0; y1 < height; y1++)
	{
		//计算偏移量 ,注意最先读出的三个字节对应的是图像中的左下角的那个像素,最后三个字节对应图像/**/
		//    中右上 角 的那个像素/**/
		for (x1 = 0; x1 < width; x1++)
		{
			offset = (height - y1 -1) * (width * 3 + nLineOfSet) + x1 * 3;
			pDstData->b = *(pRawData + offset);
			pDstData->g = *(pRawData + offset + 1);
			pDstData->r = *(pRawData + offset + 2);
			pDstData++;
		}
		//TRACE("-------------------Line %d done, offset = %d.-------------------------------\n", y1, offset);
	}
	DELETE(pbyRaw);
	fclose( fp );
	//printf("LoadImge--->[%s] succsessfully!\n", szImgPath);
	return 0;
}

static void UpdateScreen(void)
{
	//将备份拷贝过去即完成更新/**/
	memcpy(g_pStartupDataStru->p_fbmmap, g_pStartupDataStru->p_fbmemBak, g_pStartupDataStru->nFbSize);
}

static void FillRegion(int x, int y, int width, int height, FB_PIXEL color)
{
	int m,n;
	int nOffSet = g_pStartupDataStru->nScreenWidth * y + x;
	//printf("Fill region, x = %d, y = %d, width = %d, h = %d, screensize(%d %d)\n", x, y, width, height, g_pStartupDataStru->nScreenWidth, g_pStartupDataStru->nScreenHeight);
	for(n = 0; n < height; n++)
	{
		for(m = 0; m < width; m++)
		{
			*(g_pStartupDataStru->p_fbmemBak + nOffSet) = color;
			nOffSet ++;
		}
		nOffSet += g_pStartupDataStru->nScreenWidth - width;
	}
	//UpdateScreen();
}

static void UpdateData(UI_CTRL* pCtrl, RGB* pRGBRaw)
{
	if(pCtrl == NULL)
	{
		printf("Nulllllllllllllllll1111111111111111\n");
		return;
	}

	int n, m;
	int x; 
	int y;
	//RGB* pRGBRaw = pCtrl->pRGBData;
	RGB* pRGBData;
	if(pRGBRaw == NULL)
	{
		printf("Nulllllllllllllllll222222222222222\n");
		return;
	}

	//printf("Now update Ctrl-->%s, x = %d, y = %d, w = %d, h = %d, imgW = %d, imgH = %d\n", pCtrl->szName, pCtrl->x, pCtrl->y, pCtrl->width, pCtrl->height,
	//	pCtrl->nImgWidth, pCtrl->nImgHeight);

	x = pCtrl->x ;
	if(pCtrl->x < 0)
	{
		x = 0;
	}
	else if(pCtrl->x >= g_pStartupDataStru->nScreenWidth)
	{
		x = g_pStartupDataStru->nScreenWidth - 1;
	}
	int nOffSet = g_pStartupDataStru->nScreenWidth * pCtrl->y + x;
	for(n = 0; n < pCtrl->height; n++)
	{
		y = n + pCtrl->y;
		if((y >= g_pStartupDataStru->nScreenHeight) || (n >= pCtrl->nImgHeight))
		{
			break;
		}
		for(m = 0; m < pCtrl->width; m++)
		{
			x = m + pCtrl->x;
			if(x >= g_pStartupDataStru->nScreenWidth)
			{
				break;
			}
			pRGBData = pRGBRaw + m + n*pCtrl->nImgWidth;
			*(g_pStartupDataStru->p_fbmemBak + nOffSet) = 
				RGB32_2_32(pRGBData->r, pRGBData->g, pRGBData->b);
			nOffSet ++;
		}
		nOffSet += g_pStartupDataStru->nScreenWidth - m;
	}
	//UpdateScreen();
}

static void UpdateCtrl(UI_CTRL* pCtrl)
{
	return UpdateData(pCtrl, pCtrl->pRGBData);
}

static void UpdateBtn(UI_BUTTON* pBtn, BOOL bPressed)
{
	return (bPressed)? UpdateData(&pBtn->base, pBtn->pPressedRGBData) : UpdateCtrl(&pBtn->base);
}

#define _DEFINE_CTRL(_p, _szName, _x, _y, _width, _height, _szImgPath) \
	do{\
	(_p)->x = (_x); \
	(_p)->y = (_y); \
	(_p)->width = (_width); \
	(_p)->height = (_height); \
	(_p)->pRGBData = NULL; \
	memcpy((_p)->szName, (_szName), sizeof(_szName)); \
	LoadBMP24(_szImgPath, &((_p)->pRGBData), &((_p)->nImgWidth), &((_p)->nImgHeight)); \
	(_p)++;\
	}while(0)

#define _DEFINE_BUTTON(_p, _szName, _x, _y, _width, _height, _szImgPath, _szImgPressPath, _cValue, _pfnHandler) \
	do{\
	(_p)->base.x = (_x); \
	(_p)->base.y = (_y); \
	(_p)->base.width = (_width); \
	(_p)->base.height = (_height); \
	(_p)->base.pRGBData = NULL; \
	(_p)->pPressedRGBData = NULL; \
	(_p)->bIsMouseDown = FALSE; \
	memcpy((_p)->base.szName, (_szName), sizeof(_szName)); \
	LoadBMP24((_szImgPath), &((_p)->base.pRGBData), &((_p)->base.nImgWidth), &((_p)->base.nImgHeight)); \
	LoadBMP24((_szImgPressPath), &((_p)->pPressedRGBData), NULL, NULL); \
	(_p)->cPressedValue = (_cValue); \
	(_p)->pfnPressHandler = (_pfnHandler); \
	(_p)++;\
	}while(0)


static UI_CTRL* FindCtrl(UI_PAGE* pPage, char* szCtrlName)
{
	UI_CTRL* pCtrl = NULL;
	int n;
	for(n = 0; n < pPage->nTotalCtrls; n++)
	{
		UI_CTRL* pTmp = pPage->pCtrls + n;
		if(strcmp(pTmp->szName, szCtrlName) == 0)
		{
			pCtrl = pTmp;
			break;
		}
	}
	return pCtrl;
}

static void drawStar(int nTotalStar)
{
	UI_CTRL* pStar = FindCtrl(g_pStartupDataStru->pCurrPage, "Star");
	if(pStar == NULL)
	{
		printf("Cannot find star sourceImage!\n");
		return;
	}
	FillRegion(235, 90, 120, 18, COLOR_WHITE);
	int n;
	int x_org = pStar->x;
	for(n = 0; n < nTotalStar; n++)
	{
		UpdateCtrl(pStar);
		pStar->x += 20;
	}
	pStar->x = x_org;
}

static void Num_EventHandler(EVENT_ARG* pEventArg)
{
	//printf("Btn[%s], eVentType=%d\n", pEventArg->pBtn->base.szName, pEventArg->eType);
	UpdateBtn(pEventArg->pBtn, (pEventArg->eType == _EVENT_MOUSE_DOWN));
	if(pEventArg->eType == _EVENT_MOUSE_UP)
	{
		int nLen = strlen(g_pStartupDataStru->szInputPasswd);
		if(nLen < 6)
		{
			g_pStartupDataStru->szInputPasswd[nLen] = pEventArg->pBtn->cPressedValue;
			nLen++;
			drawStar(nLen);
		}
		//printf("-------Input Passwd --> [%s]----------\n", g_pStartupDataStru->szInputPasswd);
	}
	UpdateScreen();
}

static void Del_EventHandler(EVENT_ARG* pEventArg)
{
	//printf("Btn[%s], eVentType=%d\n", pEventArg->pBtn->base.szName, pEventArg->eType);
	UpdateBtn(pEventArg->pBtn, (pEventArg->eType == _EVENT_MOUSE_DOWN));
	if(pEventArg->eType == _EVENT_MOUSE_UP)
	{
		memset(g_pStartupDataStru->szInputPasswd, 0, 7);
		drawStar(0);
	}
	UpdateScreen();
}

static void BackSpace_EventHandler(EVENT_ARG* pEventArg)
{
	//printf("Btn[%s], eVentType=%d\n", pEventArg->pBtn->base.szName, pEventArg->eType);
	UpdateBtn(pEventArg->pBtn, (pEventArg->eType == _EVENT_MOUSE_DOWN));
	if(pEventArg->eType == _EVENT_MOUSE_UP)
	{
		int nLen = strlen(g_pStartupDataStru->szInputPasswd);
		if(nLen > 0)
		{
			nLen--;
			g_pStartupDataStru->szInputPasswd[nLen] = '\0';			
			drawStar(nLen);
		}
		//printf("-------Input Passwd --> [%s]----------\n", g_pStartupDataStru->szInputPasswd);
	}
	UpdateScreen();
}

static void OK_EventHandler(EVENT_ARG* pEventArg)
{
	//printf("Btn[%s], eVentType=%d\n", pEventArg->pBtn->base.szName, pEventArg->eType);
	UpdateBtn(pEventArg->pBtn, (pEventArg->eType == _EVENT_MOUSE_DOWN));
	if(pEventArg->eType == _EVENT_MOUSE_UP)
	{
		if(strcmp(g_pStartupDataStru->szInputPasswd, g_pStartupDataStru->settings.szPasswd) == 0)
		{
			//g_pStartupDataStru->pCurrPage = ShowPage(PAGE_HOME);
			g_pStartupDataStru->bLogInOK = TRUE;
		}
		else
		{
			printf("Wrong passwd, input = [%s] and passwd = [%s]\n", 
				g_pStartupDataStru->szInputPasswd, g_pStartupDataStru->settings.szPasswd);
		}
	}
	UpdateScreen();
}

static void GunSel_EventHandler(EVENT_ARG* pEventArg)
{
	//printf("Btn[%s], eVentType=%d\n", pEventArg->pBtn->base.szName, pEventArg->eType);
	if(pEventArg->eType == _EVENT_MOUSE_UP)
	{
		UI_BUTTON* pOtherBtn = NULL;
		SHM_DATA shmdd;
		shmdd.emType = SHM_STP_SET_GUN_SEL;
		if(strcmp(pEventArg->pBtn->base.szName, "btnGunSel_A") == 0)
		{
			pOtherBtn = g_pStartupDataStru->pCurrPage->pBtns + 1;
			shmdd.szExtraData[0] = 0;
		}
		else
		{
			pOtherBtn = g_pStartupDataStru->pCurrPage->pBtns;
			shmdd.szExtraData[1] = 1;
		}
		SendMsg2App(&shmdd);
		UpdateBtn(pEventArg->pBtn, TRUE);
		UpdateBtn(pOtherBtn, FALSE);
		usleep(100 * 1000); //sleep 100 ms
		UpdateScreen();
		usleep(50 * 1000); //sleep 100 ms
	}

}


#define IMG_PATH "/app/startup_show/images/"
static UI_PAGE* CreateLogoPage(void)
{
	UI_PAGE* pPage = g_pStartupDataStru->ui.pPages + PAGE_LOGO_SHOW;
	pPage->nPageId = PAGE_LOGO_SHOW;
	pPage->nTotalBtns = 0;
	pPage->pBtns = NULL;
	//pPage->nTotalCtrls = 2;
	//pPage->pCtrls = NEW(UI_CTRL, 2);
	//UI_CTRL* p = pPage->pCtrls;
	//_DEFINE_CTRL(p, "YT_Logo", 100, 110, 600, 120, IMG_PATH"YT_Logo.bmp");
	//_DEFINE_CTRL(p, "YT_Text", 160, 250, 480, 64, IMG_PATH"YT_Text.bmp");
	pPage->nTotalCtrls = 1;
	pPage->pCtrls = NEW(UI_CTRL, 1);
	UI_CTRL* p = pPage->pCtrls;
	_DEFINE_CTRL(p, "stantardLogo", 200, 130, 391, 231, IMG_PATH"stantardLogo.bmp");
	//初始化画布/**/
	FillRegion(0, 0, g_pStartupDataStru->nScreenWidth, g_pStartupDataStru->nScreenHeight, COLOR_WHITE);
	//FillRegion(100, 100, 600, 120, COLOR_WHITE);
	return pPage;
}

static void ReadMeOK_EventHandler(EVENT_ARG* pEventArg)
{
	//printf("Btn[%s], eVentType=%d\n", pEventArg->pBtn->base.szName, pEventArg->eType);
	UpdateBtn(pEventArg->pBtn, (pEventArg->eType == _EVENT_MOUSE_DOWN));
	if(pEventArg->eType == _EVENT_MOUSE_UP)
	{
		g_pStartupDataStru->bReadMeConfirmed = TRUE;
	}
	UpdateScreen();
}

//static UI_PAGE* CreateReadMePage(void)
//{
//	UI_PAGE* pPage = g_pStartupDataStru->ui.pPages + 1;
//	pPage->nPageId = PAGE_README;
//	pPage->nTotalBtns = 1;
//	pPage->pBtns = NEW(UI_BUTTON, 1); //1 个按钮/**/
//	UI_BUTTON* pp = pPage->pBtns;
//	_DEFINE_BUTTON(pp, "btn_ok",		540,	400,	155,	51,	IMG_PATH"OK_b.bmp",	IMG_PATH"OK_g.bmp",	NULL,ReadMeOK_EventHandler);
//	pPage->nTotalCtrls = 2;
//	pPage->pCtrls = NEW(UI_CTRL, 2);
//	UI_CTRL* p = pPage->pCtrls;
//	_DEFINE_CTRL(p, "ReadMe_Top", 0, 0, 800, 179, IMG_PATH"ReadMe_Top.bmp");
//	_DEFINE_CTRL(p, "ReadMe_Text", 70, 180, 690, 175, IMG_PATH"ReadMe_Text.bmp");
//	//初始化画布/**/
//	FillRegion(0, 0, g_pStartupDataStru->nScreenWidth, g_pStartupDataStru->nScreenHeight, COLOR_WHITE);
//	return pPage;
//}

static UI_PAGE* CreateLoginPage(void)
{
	UI_PAGE* pPage = g_pStartupDataStru->ui.pPages + PAGE_LOGIN;
	pPage->nPageId = PAGE_LOGIN;
	pPage->nTotalBtns = 13;
	pPage->pBtns = NEW(UI_BUTTON, 13); //13 个按钮/**/
	UI_BUTTON* pp = pPage->pBtns;
	_DEFINE_BUTTON(pp, "btn_1",		220,	140,	75,	64,	IMG_PATH"1_b.bmp",		IMG_PATH"1_g.bmp",		'1',	Num_EventHandler);
	_DEFINE_BUTTON(pp, "btn_2",		305,	140,	75,	64,	IMG_PATH"2_b.bmp",		IMG_PATH"2_g.bmp",		'2',	Num_EventHandler);
	_DEFINE_BUTTON(pp, "btn_3",		390,	140,	75,	64,	IMG_PATH"3_b.bmp",		IMG_PATH"3_g.bmp",		'3',	Num_EventHandler);
	_DEFINE_BUTTON(pp, "btn_4",		475,	140,	75,	64,	IMG_PATH"4_b.bmp",		IMG_PATH"4_g.bmp",		'4',	Num_EventHandler);

	_DEFINE_BUTTON(pp, "btn_5",		220,	214,	75,	64,	IMG_PATH"5_b.bmp",		IMG_PATH"5_g.bmp",		'5',	Num_EventHandler);
	_DEFINE_BUTTON(pp, "btn_6",		305,	214,	75,	64,	IMG_PATH"6_b.bmp",		IMG_PATH"6_g.bmp",		'6',	Num_EventHandler);
	_DEFINE_BUTTON(pp, "btn_7",		390,	214,	75,	64,	IMG_PATH"7_b.bmp",		IMG_PATH"7_g.bmp",		'7',	Num_EventHandler);
	_DEFINE_BUTTON(pp, "btn_8",		475,	214,	75,	64,	IMG_PATH"8_b.bmp",		IMG_PATH"8_g.bmp",		'8',	Num_EventHandler);

	_DEFINE_BUTTON(pp, "btn_9",		220,	288,	75,	64,	IMG_PATH"9_b.bmp",		IMG_PATH"9_g.bmp",		'9',	Num_EventHandler);
	_DEFINE_BUTTON(pp, "btn_0",		305,	288,	75,	64,	IMG_PATH"0_b.bmp",		IMG_PATH"0_g.bmp",		'0',	Num_EventHandler);
	_DEFINE_BUTTON(pp, "btn_del",	390,	288,	75,	64,	IMG_PATH"del_b.bmp",	IMG_PATH"del_g.bmp",	NULL,Del_EventHandler);
	_DEFINE_BUTTON(pp, "btn_back",	475,	288,	75,	64,	IMG_PATH"back_b.bmp",	IMG_PATH"back_g.bmp",	NULL,BackSpace_EventHandler);
	
	_DEFINE_BUTTON(pp, "btn_ok",		313,	375,	155,	51,	IMG_PATH"OK_b.bmp",	IMG_PATH"OK_g.bmp",	NULL,OK_EventHandler);
	pPage->nTotalCtrls = 2;
	pPage->pCtrls = NEW(UI_CTRL, 2); //输入框和星星/**/
	UI_CTRL* p = pPage->pCtrls;
	_DEFINE_CTRL(p, "Passwd_Edit",	218,	30,	336,	94,	IMG_PATH"passwd_andtext.bmp");
	_DEFINE_CTRL(p, "Star",			235,	90,	16,	18,	IMG_PATH"star.bmp");
	//_DEFINE_CTRL(p, "btn_1",			215,	140,	75,	64,	IMG_PATH"1_b.bmp");
	//初始化画布/**/
	FillRegion(0, 0, g_pStartupDataStru->nScreenWidth, g_pStartupDataStru->nScreenHeight, COLOR_WHITE);
	return pPage;
}

static void drawRecFrame(int x, int y, int width, int height, int framePx, int color)
{
	int m,n;
	int nOffSet = g_pStartupDataStru->nScreenWidth * y + x;
	//printf("Fill region, x = %d, y = %d, width = %d, h = %d, screensize(%d %d)\n", x, y, width, height, g_pStartupDataStru->nScreenWidth, g_pStartupDataStru->nScreenHeight);
	for(n = 0; n < height; n++)
	{
		for(m = 0; m < width; m++)
		{
			if((m < framePx) || (n < framePx) || (m >= width - framePx) || (n >= height - framePx))
			{
				*(g_pStartupDataStru->p_fbmemBak + nOffSet) = color;
			}
			nOffSet ++;
		}
		nOffSet += g_pStartupDataStru->nScreenWidth - width;
	}
}

static void drawRect(int x, int y, int width, int height, int color)
{
	int m,n;
	int nOffSet = g_pStartupDataStru->nScreenWidth * y + x;
	//printf("Fill region, x = %d, y = %d, width = %d, h = %d, screensize(%d %d)\n", x, y, width, height, g_pStartupDataStru->nScreenWidth, g_pStartupDataStru->nScreenHeight);
	for(n = 0; n < height; n++)
	{
		for(m = 0; m < width; m++)
		{
			*(g_pStartupDataStru->p_fbmemBak + nOffSet) = color;
			nOffSet ++;
		}
		nOffSet += g_pStartupDataStru->nScreenWidth - width;
	}
}

static void drawQrCode(QRcode* qr, int x_ofst, int y_ofst, int width, int height, int color)
{
	int qr_width = qr->width > 0 ? qr->width : 1;
	int x,y;
	unsigned char b;
	double scale_x = width / qr_width;
	double scale_y = height / qr_width;
	for(y = 0; y < qr_width; y++)
	{
		for(x = 0; x < qr_width; x++)
		{
			b = qr->data[y * qr_width + x];
			if(b & 0x01)
			{
				drawRect((int)(x_ofst + x * scale_x), (int)(y_ofst + y * scale_y),
					(int)scale_x, (int)scale_y, color);
			}
		}
	}
}

static UI_PAGE* CreateHomePage(void)
{
	UI_PAGE* pPage = g_pStartupDataStru->ui.pPages + PAGE_HOME;
	pPage->nPageId = PAGE_HOME;
	pPage->nTotalBtns = 4;
	pPage->pBtns = NEW(UI_BUTTON, 4); //2 个按钮/**/
	UI_BUTTON* pp = pPage->pBtns;
	_DEFINE_BUTTON(pp, "btnGunSel_A",85,	135,	232,	43,IMG_PATH"btn_selectgun.bmp",	IMG_PATH"btn_gunslelcted.bmp",	NULL,	GunSel_EventHandler);
	_DEFINE_BUTTON(pp, "btnGunSel_B",485,	135,	232,	43,IMG_PATH"btn_selectgun.bmp",	IMG_PATH"btn_gunslelcted.bmp",	NULL,	GunSel_EventHandler);
	_DEFINE_BUTTON(pp, "btnAuxTp_A",	35,	412,	340,	45,IMG_PATH"aux_type_12.bmp",	IMG_PATH"aux_type_24.bmp",		NULL,	NULL);
	_DEFINE_BUTTON(pp, "btnAuxTp_B",	435,	412,	340,	45,IMG_PATH"aux_type_12.bmp",	IMG_PATH"aux_type_24.bmp",		NULL,	NULL);
	pPage->nTotalCtrls = 10;
	pPage->pCtrls = NEW(UI_CTRL, 10);
	UI_CTRL* p = pPage->pCtrls;
	_DEFINE_CTRL(p, "alarm", 675, 10, 38, 38, IMG_PATH"alarm.bmp");
	_DEFINE_CTRL(p, "offline", 725, 10, 38, 38, IMG_PATH"offline.bmp");
	_DEFINE_CTRL(p, "A-Available", 52, 81, 300, 50, IMG_PATH"A-Available.bmp");
	_DEFINE_CTRL(p, "B-Available", 452, 81, 300, 50, IMG_PATH"B-Available.bmp");
	_DEFINE_CTRL(p, "Card-A", 35, 213, 160, 120, IMG_PATH"Card.bmp");
	_DEFINE_CTRL(p, "Card-A", 435, 213, 160, 120, IMG_PATH"Card.bmp");
	_DEFINE_CTRL(p, "UserTipes-A", 35, 372, 350, 40, IMG_PATH"user_tips.bmp");
	_DEFINE_CTRL(p, "UserTipes-B", 435, 372, 350, 40, IMG_PATH"user_tips.bmp");
	_DEFINE_CTRL(p, "AuxType-12", 35, 412, 340, 45, IMG_PATH"aux_type_12.bmp");
	_DEFINE_CTRL(p, "AuxType-24", 435, 412, 340, 45, IMG_PATH"aux_type_24.bmp");
	//_DEFINE_CTRL(p, "2DCode-A", 225, 192, 136, 136, IMG_PATH"2DCode.bmp");
	//_DEFINE_CTRL(p, "2DCode-B", 625, 192, 136, 136, IMG_PATH"2DCode.bmp");

	//初始化画布/**/
	FillRegion(0, 0, g_pStartupDataStru->nScreenWidth, 55, RGB32_2_32(28, 76, 163));
	FillRegion(0, 55, g_pStartupDataStru->nScreenWidth, g_pStartupDataStru->nScreenHeight - 55, RGB32_2_32(236, 241, 245));
	drawRecFrame(3, 62, 395, 410, 1, RGB32_2_32(0x88, 0x88, 0x88));
	drawRecFrame(402, 62, 395, 410, 1, RGB32_2_32(0x88, 0x88, 0x88));
	//FillRegion(100, 100, 600, 120, COLOR_WHITE);
	return pPage;
}

UI_PAGE* ShowPage(int nPageId)
{
	UI_PAGE* pPage = NULL;
	UI_CTRL* pCtrl;
	int n;
	if(nPageId == PAGE_LOGO_SHOW)
	{
		//First Create
		pPage = CreateLogoPage();
		//UpdateScreen();
		//pCtrl = FindCtrl(pPage, "YT_Logo");
		////pCtrl->width = 0;
		//pCtrl->height = 20;
		//for(n = 0; n < 10; n++)
		//{
		//	pCtrl->height += 10;
		//	//pCtrl->width += 50;
		//	UpdateCtrl(pCtrl);
		//	UpdateScreen();
		//	//usleep(50 * 1000);
		//}
		//usleep(150 * 1000);
		////Now show animte;
		//pCtrl = FindCtrl(pPage, "YT_Text");
		//BOOL bReverse = FALSE;
		//pCtrl->y = 350;
		//for(n = 0; n < 6; n++)
		//{
		//	if(n < 3)
		//	{
		//		pCtrl->y -= 35;
		//	}
		//	else
		//	{
		//		pCtrl->y = 250;
		//		if(n < 5)
		//		{
		//			if(bReverse)
		//			{
		//				bReverse = FALSE;
		//				pCtrl->y -= 1;
		//			}
		//			else
		//			{
		//				bReverse = TRUE;
		//				pCtrl->y += 1;
		//			}
		//		}
		//	}

		//	FillRegion(0, 240, g_pStartupDataStru->nScreenWidth, g_pStartupDataStru->nScreenHeight - 240, COLOR_WHITE);
		//	//FillRegion(0, 240, g_pStartupDataStru->nScreenWidth, 200, COLOR_WHITE);
		//	UpdateCtrl(pCtrl);
		//	UpdateScreen();
		//	usleep((60 - n * 10) * 1000);
		//}
		pCtrl = pPage->pCtrls;
		//pCtrl->width = 5;
		//pCtrl->height = 25;
		//for(n = 0; n < 4; n++)
		//{
		//	pCtrl->width += 60;
		//	pCtrl->height += 30;
		//	//FillRegion(300, 100, 300, 380, COLOR_WHITE);
		//	UpdateCtrl(pCtrl);
		//	UpdateScreen();
		//	usleep((60 - n * 10) * 1000);
		//}

		//BOOL bReverse = FALSE;
		//for(n = 0; n < 3; n++)
		{
			//FillRegion(240, 100, 300, 380, COLOR_WHITE);
			usleep(100 * 1000);
			//bReverse = bReverse ? FALSE : TRUE;
			//bReverse ? (pCtrl->y -= 10) : (pCtrl->y += 10);
			UpdateCtrl(pCtrl);
			UpdateScreen();
		}

	}
	//else if(nPageId == PAGE_README)
	//{
	//	pPage = CreateReadMePage();
	//	for(n = 0; n < pPage->nTotalCtrls; n++)
	//	{
	//		UpdateCtrl(pPage->pCtrls + n);
	//	}
	//	for(n = 0; n < pPage->nTotalBtns; n++)
	//	{
	//		UpdateBtn(pPage->pBtns + n, FALSE);
	//	}
	//	UpdateScreen();
	//}
	else if(nPageId == PAGE_LOGIN)
	{
		pPage = CreateLoginPage();
		//UI_BUTTON* pBtn;
		//for(n = 0; n < pPage->nTotalCtrls; n++)
		{
			//UpdateCtrl(pPage->pCtrls + n);
			UpdateCtrl(pPage->pCtrls);
		}
		for(n = 0; n < pPage->nTotalBtns; n++)
		{
			//pBtn = pPage->pBtns + n;
			UpdateBtn(pPage->pBtns + n, FALSE);
		}
		UpdateScreen();
	}
	else if(nPageId == PAGE_HOME)
	{
		pPage = CreateHomePage();
		UI_BUTTON* pBtn;
		for(n = 0; n < pPage->nTotalCtrls; n++)
		{
			UpdateCtrl(pPage->pCtrls + n);
		}
		//设置A枪为默认选择/**/
		UpdateBtn(pPage->pBtns, TRUE);
		UpdateBtn(pPage->pBtns + 1, FALSE);
		//Set gun aux-type
		UpdateBtn(pPage->pBtns + 2, (g_pStartupDataStru->settings.cAuxType_A > '0'));
		UpdateBtn(pPage->pBtns + 3, (g_pStartupDataStru->settings.cAuxType_B > '0'));
		//画二维码/**/
		//drawQrCode(g_pStartupDataStru->pQrCode_A, 225, 213, 136, 136, COLOR_BLACK);
		//drawQrCode(g_pStartupDataStru->pQrCode_B, 625, 213, 136, 136, COLOR_BLACK);
		drawQrCode(g_pStartupDataStru->pQrCode_A, 225, 213, 136, 136, RGB32_2_32(30, 30, 30));
		drawQrCode(g_pStartupDataStru->pQrCode_B, 625, 213, 136, 136, RGB32_2_32(30, 30, 30));
		UpdateScreen();
	}

	return pPage;
}


