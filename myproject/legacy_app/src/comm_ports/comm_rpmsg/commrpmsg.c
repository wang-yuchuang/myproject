#include "std_sys.h"		
#include "app_incs.h"
#include "commrpmsg.h"

HANDLE Rpmsg_CommOpen(IN char *pPortDescriptor,
					  IN char		*pOpenParams,
	                  IN DWORD	 	dwPortAttr,
					  IN int		nTimeout,
					  OUT int		*pErrCode)
{
	RPMEG_PORT_DRV *pPort = NEW(RPMEG_PORT_DRV, 1);
	
	int ret = sscanf(pPortDescriptor, "%d|%x|%d|%[^|]", \
		&pPort->nFrame_Head, &pPort->nFrame_Tail, &pPort->nRemote_ept, pPort->szRemoteName);

	if(ret != 4) 
	{
		printf("invalid open params: %s.\n", pPortDescriptor);
		DELETE(pPort);
		return NULL;
	}

	// printf("Parsed open params: Frame_Head=%d, Frame_Tail=%d, Remote_ept=%d, RemoteName=%s\n", \
	// 	pPort->nFrame_Head, pPort->nFrame_Tail, pPort->nRemote_ept, pPort->szRemoteName);

	frame_conf_t fc = {
		.head_len =2,
		.tail_len =2,
		.frame_head = {pPort->nFrame_Head, pPort->nFrame_Head},
		.frame_tail = {pPort->nFrame_Tail, pPort->nFrame_Tail},
		.checksum_type = CRC32,
	};

	if(iccmsg_find_name(pPort->szRemoteName)) 
	{
		printf("waiting remote ept :%s.\n", pPort->szRemoteName);
		return NULL;
	}

	char szInitName[64] = {0};
	snprintf(szInitName, sizeof(szInitName), "%s_ap", pPort->szRemoteName);
	// printf("Local endpoint name: %s\n", szInitName);

	pPort->stRpmsg_Hal = iccmsg_hal_init(pPort->nRemote_ept, fc, szInitName);
	
	if(pPort->stRpmsg_Hal == NULL) 
	{
		printf("iccmsg_hal_init failed.\n");
		DELETE(pPort);
		return NULL;
	}

	// printf("fd_ipcc = %d, fd_virtio = %d\n", pPort->stRpmsg_Hal->ll.fd_ipcc, pPort->stRpmsg_Hal->ll.fd_virtio);
	
	return (HANDLE)pPort;
}

HANDLE Rpmsg_CommAccept( IN HANDLE hPort )
{
	return (HANDLE)hPort;
}

#define MSG_SOI   0xABCD
#define MSG_EOI   0xEF
int Rpmsg_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead)
{
	RPMEG_PORT_DRV *Rpmsg_Info = (RPMEG_PORT_DRV *)hPort;
	message_t rx_msg;

	int nTotalReadCount = 50;
	int nTotalBytesRead = 0;

	BYTE MsgSoi[2] = {(MSG_SOI >> 8) & 0xFF, MSG_SOI & 0xFF};
	BYTE MsgEoi = MSG_EOI & 0xFF;
	// printf("Starting to read messages...\n");
	while(nTotalReadCount > 0)
	{		
		if(receive_message_timeout(&Rpmsg_Info->stRpmsg_Hal->rx_que, &rx_msg, 5) == 0) 
		{
			// printf("Received message: ID=%d, DataLen=%zu\n", rx_msg.id, rx_msg.data_len);

			memcpy(pBuffer + nTotalBytesRead, MsgSoi, sizeof(MsgSoi));
			nTotalBytesRead += sizeof(MsgSoi);

			// BYTE MsgDataLen[2] = {(rx_msg.data_len >> 8) & 0xFF, rx_msg.data_len & 0xFF};
			// memcpy(pBuffer + nTotalBytesRead, MsgDataLen, sizeof(MsgDataLen));
			// nTotalBytesRead += sizeof(MsgDataLen);

			memcpy(pBuffer + nTotalBytesRead, rx_msg.data, rx_msg.data_len);
			nTotalBytesRead += rx_msg.data_len;

			memcpy(pBuffer + nTotalBytesRead, &MsgEoi, sizeof(MsgEoi));
			nTotalBytesRead += sizeof(MsgEoi);
		}	

		nTotalReadCount--;
		
		if((nBytesToRead - nTotalBytesRead) < 1024)
		{
			break;
		}
	}

	return nTotalBytesRead;
}

int Rpmsg_CommWrite(IN HANDLE hPort, IN char *pBuffer, IN int nBytesToWrite)
{
	RPMEG_PORT_DRV *Rpmsg_Info = (RPMEG_PORT_DRV *)hPort;
	int ret;

	ret = iccmsg_write(Rpmsg_Info->stRpmsg_Hal, 1, 0, pBuffer, nBytesToWrite, ICCMSG_ACK_NO_NEED);

	if(ret < 0){
		printf("write error.\n");
		return -1;
	}

	return ret;
}


int Rpmsg_CommControl(IN HANDLE hPort, IN int nCmd, 
					  IN OUT void *pBuffer,	IN int nDataLength)
{
	return 0;
}


int Rpmsg_CommClose( IN HANDLE hPort )
{
	return 0;
}
