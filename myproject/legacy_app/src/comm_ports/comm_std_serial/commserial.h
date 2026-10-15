
#ifndef __COMMSERIAL_H__
#define __COMMSERIAL_H__

#include <termios.h>

/* 
 * If want to test the module with static mode,
 * please comment out the next line.
 */
#define _MAKE_SHARED_LIB

#ifdef _MAKE_SHARED_LIB

#define Serial_CommOpen		HAL_CommOpen
#define Serial_CommAccept	HAL_CommAccept
#define Serial_CommRead		HAL_CommRead
#define Serial_CommWrite		HAL_CommWrite
#define Serial_CommControl	HAL_CommControl
#define Serial_CommClose		HAL_CommClose

#endif //_MAKE_SHARED_LIB

/**********************************/
#define SERIAL_MAX_CLIENTS_DEFAULT	1	/* The default serial connections	*/
#define SERIAL_MAX_CLIENT_LIMITAION	1	/* only 1 connection is allowed		*/

typedef struct _SERIAL_BAUD_ATTR
{
	int				nBaud;		// the baud rate
	int				nOdd;		// 0: none, 1: odd, 2: even
	int				nData;		// the data bits, 5-8
	int				nStop;		// the stop bits, 1-2
	unsigned long		ulUnixBaud;	// the unix defined baud rates.
}SERIAL_BAUD_ATTR;


typedef struct SSerialPortDriver	//	The HAL driver for direct serial port
{				
	int				nLastErrorCode;	//	NOTE: MUST BE THE FIRST FIELD! 
									//  the last error code.
	int				fdSerial;		//	the handle from open()
	//int				fdRtsFor485;	//	485 of SCU Plus need open 2 devcies
	COMM_TIMEOUTS	toTimeouts;		//	read and write timeout
	int				nWorkMode;		//	see enum COMM_WORK_MODE_ENUM in "halcomm.h"
	int				nMaxClients;	//	always 1
	int				*pCurClients;	//	0 or 1.

	int				nSerialPort;	// the serial port start from 0.
	char				szPortName[16];

	SERIAL_BAUD_ATTR	attr;		// the attr of the port.
}SERIAL_PORT_DRV;

#define COM_RETRY_TIME		3		// the maximum retry time on error.


#endif
