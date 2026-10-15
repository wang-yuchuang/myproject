#include "main.h"

static int s_hWatchDog = -1;

#define WATCH_DOG_MAX_FEED_PERIOD		60	//60 seconds.
#define WATCH_DOG_NAME					"/dev/wd"

#ifdef _DEBUG
#warning ">>>>>>>>>>>>>>>>>> The watch dog is NOT enabled."
#undef _HAS_WATCH_DOG

#else
#define _HAS_WATCH_DOG		1
#endif

#ifndef _HAS_FLASH_MEM
	#warning ">>>>>>>>>>>>>>>>>> x86 platform, so the watch dog is NOT enabled."
	#undef _HAS_WATCH_DOG
#endif

#define WATCH_DOG	"WATCH DOG"


//#define IOCTL_EN_WD		0x6b01
//#define IOCTL_DIS_WD		0x6b02
//#define IOCTL_FEED_WD		0x6b04
//#define IOCTL_SET_TIMER		0x6b10
#define WDOG_MAJOR			241 
#define IOCTL_EN_WD		_IOW(WDOG_MAJOR, 1, unsigned long)
#define IOCTL_DIS_WD		_IOW(WDOG_MAJOR, 2, unsigned long)
#define IOCTL_FEED_WD		_IOW(WDOG_MAJOR, 3, unsigned long)
#define IOCTL_SET_TIMER		_IOW(WDOG_MAJOR, 4, unsigned long)
//added by Jimmy 20210402 for new Linux kernel
#define WDOG_MAJOR_NEW			236 
#define IOCTL_EN_WD_NEW			_IOW(WDOG_MAJOR_NEW, 1, unsigned long)
#define IOCTL_DIS_WD_NEW		_IOW(WDOG_MAJOR_NEW, 2, unsigned long)
#define IOCTL_FEED_WD_NEW		_IOW(WDOG_MAJOR_NEW, 3, unsigned long)
#define IOCTL_SET_TIMER_NEW		_IOW(WDOG_MAJOR_NEW, 4, unsigned long)

static BOOL s_bIsRunInNewKernel = FALSE;

void WatchDog_Feed(void);

BOOL WatchDog_Open(void)
{
    printf("[DEBUG] WatchDog_Open: hardware watchdog disabled for debug\n");
    s_hWatchDog = -1;  // keep -1 so WatchDog_Feed skips ioctl too
    return TRUE;
}



void WatchDog_Feed(void)
{
#ifdef _HAS_WATCH_DOG
	if (s_hWatchDog != -1)
	{
		if(s_bIsRunInNewKernel)
		{
			ioctl(s_hWatchDog, IOCTL_EN_WD_NEW);
			ioctl(s_hWatchDog, IOCTL_FEED_WD_NEW);		
		}
		else
		{
			ioctl(s_hWatchDog, IOCTL_EN_WD);
			ioctl(s_hWatchDog, IOCTL_FEED_WD);			
		}
	}
	
#else
	//TRACEX("Clear watch dog...\n");
	//printf(".");fflush(stdout);
#endif
}



void WatchDog_Close(BOOL bDisableDog)
{
#ifdef _HAS_WATCH_DOG
	TRACEX("Closing watch dog with flag %d...\n", bDisableDog);

	if (bDisableDog)
	{
		if(s_bIsRunInNewKernel)
		{
			ioctl(s_hWatchDog, IOCTL_DIS_WD_NEW);
		}
		else
		{
			ioctl(s_hWatchDog, IOCTL_DIS_WD);			
		}
	}

	close(s_hWatchDog);
#else
	UNUSED(bDisableDog);
#endif

	s_hWatchDog = -1;
}


//open to test wdt
//#define _TEST_WATCH_DOG

#ifdef _TEST_WATCH_DOG
#include <signal.h>

static int s_nRunning = 1;
static void QuitSignalHandler( int a )
{
    if( s_nRunning > 0)
    {
		printf( "WDT: Receives a quit signal. Quiting ... \007\007\n" );
        s_nRunning = 0;
    } 
}


int main(void)
{
	int i = 0;

	signal(SIGINT, QuitSignalHandler);

	printf("Open watch dog...\n");

	if (!WatchDog_Open())
	{
		printf("Fails on opening and enabling the watch-dog.\n");
	}

	printf( "Open Watch dog ok.  CTRL+C to quit...\n");

	while(s_nRunning)
	{
		i++;
		printf("%d: Feeding dog...\n", i);
		WatchDog_Feed();
		usleep(100000);
	}

	printf("Press '0' to enable watchdog before exiting, other disable.\n");
	scanf("%d", &i);
	if (i==0)
	{
		printf ("The WDT is enabled. wait for system restart...\n");
	}	

	WatchDog_Close(i ? 1 : 0);

	return 0;
}
#endif
