#ifndef __STDSYS_H__
#define __STDSYS_H__

#include <stdlib.h>				/* malloc, free, etc. */
#include <stdio.h>				/* stdin, stdout, stderr */
#include <string.h>				/* strdup */
#include <time.h>				/* localtime, time */
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>				/* OPEN_MAX */
#include <assert.h>
#include <ctype.h>

#if defined(linux) || defined(unix)
#define LINUX	linux
#define UNIX	unix

#include <unistd.h>
#include <sys/time.h>			/* select */
#include <linux/rtc.h>	
#include <sys/mman.h>			/* mmap */
#include <pwd.h>
#include <sys/stat.h>			/* open */
#include <sys/ioctl.h>
#include <pthread.h>
#include "dirent.h"
#include <netinet/in.h>			/* sockaddr_in, sockaddr */
#include <arpa/inet.h>			/* inet_ntoa */
#include <net/route.h>			/*rtentry*/
#include <net/if.h>				/*ifreq */
#include<sys/file.h>

#endif


#endif /*__STDSYS_H__*/
