#include "iec_104.h"
 

static bool running = true;

 static CS104_ConnectionEvent lastEvent = CS104_CONNECTION_CLOSED;

/* Callback handler to log sent or received messages (optional) */
static void
rawMessageHandler (void* parameter, uint8_t* msg, int msgSize, bool sent)
{
    if (sent)
        printf("Client SEND: ");
    else
        printf("Client RCVD: ");

    int i;
    for (i = 0; i < msgSize; i++) {
        printf("%02x ", msg[i]);
    }

    printf("\n");
}

/* Connection event handler */
static void
connectionHandler (void* parameter, CS104_Connection connection, CS104_ConnectionEvent event)
{
    switch (event) {
    case CS104_CONNECTION_OPENED:
        printf("Connection established\n");
        break;
    case CS104_CONNECTION_CLOSED:
        printf("Connection closed\n");
        break;
    case CS104_CONNECTION_STARTDT_CON_RECEIVED:
        printf("Received STARTDT_CON\n");
        break;
    case CS104_CONNECTION_STOPDT_CON_RECEIVED:
        printf("Received STOPDT_CON\n");
        break;
    //case CS104_CONNECTION_FAILED:
     //   printf("Connection failed\n");
    }

    lastEvent = event;
 }

/*
 * CS101_ASDUReceivedHandler implementation
 *
 * For CS104 the address parameter has to be ignored
 */
static bool
asduReceivedHandler (void* parameter, int address, CS101_ASDU asdu)
{
    printf("RECVD ASDU type: %s(%i) elements: %i\n",
            TypeID_toString(CS101_ASDU_getTypeID(asdu)),
            CS101_ASDU_getTypeID(asdu),
            CS101_ASDU_getNumberOfElements(asdu));

    if (CS101_ASDU_getTypeID(asdu) == M_ME_TE_1) {

        printf("  measured scaled values with CP56Time2a timestamp:\n");

        int i;

        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {

            MeasuredValueScaledWithCP56Time2a io =
                    (MeasuredValueScaledWithCP56Time2a) CS101_ASDU_getElement(asdu, i);

            printf("    IOA: %i value: %i\n",
                    InformationObject_getObjectAddress((InformationObject) io),
                    MeasuredValueScaled_getValue((MeasuredValueScaled) io)
            );

            MeasuredValueScaledWithCP56Time2a_destroy(io);
        }
    }
    else if (CS101_ASDU_getTypeID(asdu) == M_SP_NA_1) {
        printf("Client:  single point information:\n");

        int i;

        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {

            SinglePointInformation io =
                    (SinglePointInformation) CS101_ASDU_getElement(asdu, i);

            printf("    IOA: %i value: %i\n",
                    InformationObject_getObjectAddress((InformationObject) io),
                    SinglePointInformation_getValue((SinglePointInformation) io)
            );

            SinglePointInformation_destroy(io);
        }
    }
    else if (CS101_ASDU_getTypeID(asdu) == C_TS_TA_1) {
        printf("  test command with timestamp\n");
    }

    return true;
}
 
static void Create_YCEmsSetting_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Connection con, CS101_AppLayerParameters alParams)
{
	int n = 0;

    //------------- Refresh EMS Setting C2S ----------//

    pIEC104->strC2S_Ems.nEmsSectorNum = MAX_EMS_SECTOR;

    pIEC104->strC2S_Ems.arryEmsParam[0].tmStart = 0;
    pIEC104->strC2S_Ems.arryEmsParam[0].nStopSOC = 6;
    pIEC104->strC2S_Ems.arryEmsParam[0].nBattGrpPwrLmt = 112;
    pIEC104->strC2S_Ems.arryEmsParam[0].nActvPwrLmt = 113;
    pIEC104->strC2S_Ems.arryEmsParam[0].nReActvPwrLmt = 114;

    pIEC104->strC2S_Ems.arryEmsParam[1].tmStart = 480;
    pIEC104->strC2S_Ems.arryEmsParam[1].nStopSOC = 4;
    pIEC104->strC2S_Ems.arryEmsParam[1].nBattGrpPwrLmt = 223;
    pIEC104->strC2S_Ems.arryEmsParam[1].nActvPwrLmt = -224;
    pIEC104->strC2S_Ems.arryEmsParam[1].nReActvPwrLmt = 225;

    pIEC104->strC2S_Ems.arryEmsParam[2].tmStart = 720;
    pIEC104->strC2S_Ems.arryEmsParam[2].nStopSOC = 2;
    pIEC104->strC2S_Ems.arryEmsParam[2].nBattGrpPwrLmt = 333;
    pIEC104->strC2S_Ems.arryEmsParam[2].nActvPwrLmt = 334;
    pIEC104->strC2S_Ems.arryEmsParam[2].nReActvPwrLmt = 335;

    pIEC104->strC2S_Ems.arryEmsParam[3].tmStart = 780;
    pIEC104->strC2S_Ems.arryEmsParam[3].nStopSOC = 12;
    pIEC104->strC2S_Ems.arryEmsParam[3].nBattGrpPwrLmt = 443;
    pIEC104->strC2S_Ems.arryEmsParam[3].nActvPwrLmt = -444;
    pIEC104->strC2S_Ems.arryEmsParam[3].nReActvPwrLmt = 445;

    pIEC104->strC2S_Ems.arryEmsParam[4].tmStart = 840;
    pIEC104->strC2S_Ems.arryEmsParam[4].nStopSOC = 22;
    pIEC104->strC2S_Ems.arryEmsParam[4].nBattGrpPwrLmt = 553;
    pIEC104->strC2S_Ems.arryEmsParam[4].nActvPwrLmt = -100;
    pIEC104->strC2S_Ems.arryEmsParam[4].nReActvPwrLmt = 555;

    pIEC104->strC2S_Ems.arryEmsParam[5].tmStart = 900;
    pIEC104->strC2S_Ems.arryEmsParam[5].nStopSOC = 32;
    pIEC104->strC2S_Ems.arryEmsParam[5].nBattGrpPwrLmt = 553;
    pIEC104->strC2S_Ems.arryEmsParam[5].nActvPwrLmt = -100;
    pIEC104->strC2S_Ems.arryEmsParam[5].nReActvPwrLmt = 555;


    pIEC104->strC2S_Ems.arryEmsParam[6].tmStart = 960;
    pIEC104->strC2S_Ems.arryEmsParam[6].nStopSOC = 42;
    pIEC104->strC2S_Ems.arryEmsParam[6].nBattGrpPwrLmt = 553;
    pIEC104->strC2S_Ems.arryEmsParam[6].nActvPwrLmt = -100;
    pIEC104->strC2S_Ems.arryEmsParam[6].nReActvPwrLmt = 555;



    pIEC104->strC2S_Ems.arryEmsParam[7].tmStart = 1020;
    pIEC104->strC2S_Ems.arryEmsParam[7].nStopSOC = 52;
    pIEC104->strC2S_Ems.arryEmsParam[7].nBattGrpPwrLmt = 553;
    pIEC104->strC2S_Ems.arryEmsParam[7].nActvPwrLmt = 200;
    pIEC104->strC2S_Ems.arryEmsParam[7].nReActvPwrLmt = 555;


    pIEC104->strC2S_Ems.arryEmsParam[8].tmStart = 1080;
    pIEC104->strC2S_Ems.arryEmsParam[8].nStopSOC = 62;
    pIEC104->strC2S_Ems.arryEmsParam[8].nBattGrpPwrLmt = 553;
    pIEC104->strC2S_Ems.arryEmsParam[8].nActvPwrLmt = -100;
    pIEC104->strC2S_Ems.arryEmsParam[8].nReActvPwrLmt = 555;


    pIEC104->strC2S_Ems.arryEmsParam[9].tmStart = 1140;
    pIEC104->strC2S_Ems.arryEmsParam[9].nStopSOC = 72;
    pIEC104->strC2S_Ems.arryEmsParam[9].nBattGrpPwrLmt = 553;
    pIEC104->strC2S_Ems.arryEmsParam[9].nActvPwrLmt = 190;
    pIEC104->strC2S_Ems.arryEmsParam[9].nReActvPwrLmt = 555;


    pIEC104->strC2S_Ems.arryEmsParam[10].tmStart = 1200;
    pIEC104->strC2S_Ems.arryEmsParam[10].nStopSOC = 82;
    pIEC104->strC2S_Ems.arryEmsParam[10].nBattGrpPwrLmt = 553;
    pIEC104->strC2S_Ems.arryEmsParam[10].nActvPwrLmt = -100;
    pIEC104->strC2S_Ems.arryEmsParam[10].nReActvPwrLmt = 555;

    pIEC104->strC2S_Ems.arryEmsParam[11].tmStart = 1260;
    pIEC104->strC2S_Ems.arryEmsParam[11].nStopSOC = 92;
    pIEC104->strC2S_Ems.arryEmsParam[11].nBattGrpPwrLmt = 553;
    pIEC104->strC2S_Ems.arryEmsParam[11].nActvPwrLmt = 1100;
    pIEC104->strC2S_Ems.arryEmsParam[11].nReActvPwrLmt = 555;


    //---------------------End------------------------//

	//InformationObject io = 
    //    (InformationObject) MeasuredValueScaled_create(NULL, 0x4001, 100, IEC60870_QUALITY_GOOD);
	//CS101_ASDU_addInformationObject(newAsdu, io);
    //InformationObject_destroy(io);
    //CS104_Connection_sendASDU(con,  newAsdu);

	//创建一个新的CS101_ASDU实例（使用CS101_COT_PERIODIC定期数据和CS101_COT_SPONTANEOUS自发数据）
    CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_SPONTANEOUS, 0x6201, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYTItem_C2SEms;

    int nRealEmsSectorNum = MIN((pIEC104->strC2S_Ems.nEmsSectorNum*5+1), pIEC104->nC2S_EmsNum);
    
    printf("nRealEmsSectorNum=[%d] nC2S_EmsNum=[%d]\n", nRealEmsSectorNum, pIEC104->nC2S_EmsNum);

	for (n = 0; n < nRealEmsSectorNum; n++)
	{
		pYTItem_C2SEms = pIEC104->pYTEle_C2SEms + n;
 		if(pYTItem_C2SEms == NULL)
		{
			printf("pYTItem_C2SEms == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYTItem_C2SEms->ptrValue);
		tempVal = tempVal*((int)pYTItem_C2SEms->nScale);
		
		printf("Set EMS Config Obj Addr[%d] tempVal=%d\n ", pYTItem_C2SEms->nIOA, tempVal);
		
		//创建一个包含要发送数据的新信息对象实例
		InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYTItem_C2SEms->nIOA, tempVal, pYTItem_C2SEms->emQuality);
		
 
		//将新的信息对象添加到ASDU
		CS101_ASDU_addInformationObject(newAsdu, io);

		//释放信息对象内存
		InformationObject_destroy(io);
	}

	//将ASDU放入数据队列中进行传输:
    CS104_Connection_sendASDU(con,  newAsdu);

	//释放ASDU内存
	CS101_ASDU_destroy(newAsdu);

	return;
}


void* _Thread_IEC104_SimpleClient(void* pArg)
{
	TRACEX("--- _Thread_IEC104_SimpleClient Enter --- \n");
 	IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH*)pArg;

    const char* ip = "localhost";
    uint16_t port = IEC_60870_5_104_DEFAULT_PORT;
    const char* localIp = NULL;
    int localPort = -1;

    printf("Connecting to: %s:%i\n", ip, port);
    CS104_Connection con = CS104_Connection_create(ip, port);

    CS104_APCIParameters apciParameters = CS104_Connection_getAPCIParameters(con);
    apciParameters->t0 = 10; //Connect Timeout Sec

    CS101_AppLayerParameters alParams = CS104_Connection_getAppLayerParameters(con);
    alParams->originatorAddress = 1;

    CS104_Connection_setConnectionHandler(con, connectionHandler, NULL);
    CS104_Connection_setASDUReceivedHandler(con, asduReceivedHandler, NULL);

    /* optional bind to local IP address/interface */
    if (localIp)
        CS104_Connection_setLocalAddress(con, localIp, localPort);

    /* uncomment to log messages */
    CS104_Connection_setRawMessageHandler(con, rawMessageHandler, NULL);

    CS104_Connection_connectAsync(con);

    bool startDTSent = false;

    double lastGiSent = 0;

    while (!pIEC104->bNeedExitThread) 
	{
        if (lastEvent == CS104_CONNECTION_OPENED) 
		{
            CS104_Connection_sendStartDT(con);
            startDTSent = true;
			printf("---Client: CS104_Connection_sendStartDT---\n");
        }
        else if (lastEvent == CS104_CONNECTION_CLOSED) 
		{
            //running = false;
            //CS104_Connection_connectAsync(con);

			printf("Warning: Connect Failed.lastEvent = [%d]\n", lastEvent);

            Sleep(5000);
        }
        else 
		{
			//printf("--- Client: 111 lastEvent=[%d] ---\n", lastEvent);

            if (startDTSent) 
			{
				printf("---Client: 222 lastEvent=[%d]---\n", lastEvent);

                double currentTime = GetCurrentTime();

                if (currentTime < lastGiSent)
                    lastGiSent = currentTime;

                if (currentTime > lastGiSent + 20)
				{
                    lastGiSent = currentTime;

                    CS104_Connection_sendInterrogationCommand(con, CS101_COT_ACTIVATION, 1, IEC60870_QOI_STATION);
                    
                    Create_YCEmsSetting_EnqueueASDU(pIEC104, con, alParams);
                
                //#if 0
                  
	 

                    printf("--- Client Send EMS Plan: ---\n");
                    //InformationObject sc = (InformationObject)
                    //SingleCommand_create(NULL, 5000, true, false, 0);

                    //printf("Send control command C_SC_NA_1\n");
                    //CS104_Connection_sendProcessCommandEx(con, CS101_COT_ACTIVATION, 1, sc);

                    //InformationObject_destroy(sc);
                //#endif

                }
            }
        }

        Sleep(1000);
    }

    CS104_Connection_destroy(con);

    TRACE("exit\n");

	return NULL;
}


