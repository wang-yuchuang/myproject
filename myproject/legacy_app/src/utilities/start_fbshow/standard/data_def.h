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
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
//#include <pthread.h>
#include "../qrencode.h"
#include "../share_data_def.h"

#define NEW(T, nItems)	(T *)malloc( sizeof(T)*(nItems) )
#define DELETE(p)	do { if ((p) != NULL) { free(p); (p) = NULL; } } while(0)
typedef unsigned char BYTE;
#define BOOL int
#define TRUE 1
#define FALSE 0
#define NULL ((void *)0)
//#define RGB24_2_32(red, green, blue) ((((red)&0xfc) << 10) | (((green)&0xfc) << 4) | (((blue)&0xfc) >> 2))
#define RGB32_2_32(red, green, blue) ((((red)&0xff) << 16) | (((green)&0xff) << 8) | (((blue)&0xfcf)))
#define COLOR_WHITE	0x00FFFFFF
#define COLOR_BLACK	0x00000000
typedef struct tagRGB
{
	BYTE		r;
	BYTE		g;
	BYTE		b;
}RGB;


typedef struct tagCtrl UI_CTRL;
struct tagCtrl
{
	char			szName[16]; //最多16个字节名字,将以此索引/**/
	int			x; //以矩形框作为控件的定位区域/**/
	int			y;
	int			width;
	int			height;

	int			nImgWidth;
	int			nImgHeight;
	//RGB			rgb_Background; //背景填充色/**/
	RGB*		pRGBData; //以矩形顺序排列RGB值/**/

	//UI_CTRL*		parent;//控件有可能有父控件/**/
};

typedef enum tagEventType
{
	_EVENT_MOUSE_DOWN,
	_EVENT_MOUSE_UP,
	_EVENT_CANCEL_CLICK 
}_EVENT_TYPE;

typedef struct tagButton UI_BUTTON;

typedef struct tagEvenArg
{
	int			x;
	int			y;
	_EVENT_TYPE	eType;
	UI_BUTTON*	pBtn;
}EVENT_ARG;

typedef void (*EVENT_HANDLER)(EVENT_ARG* pEventArg);

struct tagButton
{
	UI_CTRL			base;
	RGB*			pPressedRGBData; //when pressed, 以矩形顺序排列RGB值/**/
	//int				nButtonId;
	char				cPressedValue; //按下后的值/**/
	EVENT_HANDLER	pfnPressHandler; //按下事件处理/**/
	BOOL				bIsMouseDown;//是否被按下/**/
};

typedef enum ragePages
{
	PAGE_LOGO_SHOW = 0,
	//PAGE_README,
	PAGE_LOGIN,
	PAGE_HOME,

	PAGE_MAX_SUPPORT
}PPPPAGE;

typedef struct tagPage
{
	//UI_CTRL		this; //page本身也是一个Ctrl/**/
	int			nPageId;
	int			nTotalCtrls;
	UI_CTRL*		pCtrls;

	int			nTotalBtns;
	UI_BUTTON*	pBtns;
}UI_PAGE;

typedef struct tagUI
{
	int		nTotalPages;
	UI_PAGE* pPages;
}_UI_;

typedef int	FB_PIXEL;

typedef struct tagStartSettings
{
	char		szPasswd[7];
	char		szQrCode_A[128];
	char		szQrCode_B[128];
	char		cAuxType_A;
	char		cAuxType_B;
}START_SETTINGS;

typedef struct tagStartupDataStru
{
	_UI_				ui;
	int				nTsFd; //touchscreen driver
	//int				nLedFd; //led driver
	int				nFd_Fb; //framebuffer fd
	int				nTs_Event; //ts Event get

	int				nFbSize;
	int				nScreenWidth;
	int				nScreenHeight;
	FB_PIXEL*			p_fbmmap; //内存映射/**/
	FB_PIXEL*			p_fbmemBak; //备份/**/

	struct fb_var_screeninfo	screen_vinfo;
	struct fb_fix_screeninfo	screen_finfo;

	BOOL				bExit;

	UI_PAGE*			pCurrPage;

	char				szInputPasswd[7]; //支持6位密码/**/
	BOOL				bLogInOK;
	BOOL				bReadMeConfirmed;

	struct input_event	event_xy[2];

	QRcode			*pQrCode_A;
	QRcode			*pQrCode_B;

	START_SETTINGS	settings;

}STARTUP_DATA_STRU;

//extern STARTUP_DATA_STRU* g_pStartupDataStru;

//BMP图片信息/**/
typedef struct bmp_file_header
{
	char			map_id[2];	//标识符，识别位图类型，一般为‘B’‘M’/**/
	unsigned int	file_size;		//用字节表示整个文件的大小/**/
	int			reserved;		//保留，设置为0/**/
	unsigned int	offset;		//从文件开始到位图数据开始之间的数据（bitmap data）之间的偏移量/**/
}__attribute__((packed)) BMP_FILE_HEADER;

typedef struct bmp_info_header
{
	unsigned int cur_size; //当前结构体的大小，通常是40或56/**/
	int  width;         //位图的宽度，以像素为单位/**/
	int  hight;         //位图的高度，以像素为单位/**/
	short  reserved; //这个字的值永远是1/**/
	short bpp;         //每像素占用的位数，即bpp/**/
	unsigned int compression;//压缩方式/**/
	unsigned int map_size;  //用字节数表示的位图数据的大小。必须是4的倍数/**/
	int  x_ppm;          //用像素/米表示的水平分辨率/**/
	int y_ppm;  //用像素/米表示的垂直分辨率/**/
	unsigned int palette; //调色板规范/**/
	unsigned int bitmapdata;//该域的大小取决于压缩方法，它包含所有的位图数据字节，                                    //这些数据实际就是彩色调色板的索引号/**/

	//严格来说这几个不属于header/**/
	unsigned int R;
	unsigned int    G;
	unsigned int    B;
	unsigned int A;
}__attribute__((packed)) BMP_INFO_HEADER;

UI_PAGE* ShowPage(int nPageId);
BOOL SendMsg2App(SHM_DATA* pOrgData);
