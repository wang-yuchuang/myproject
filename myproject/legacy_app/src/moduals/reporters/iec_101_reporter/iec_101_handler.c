#include "iec_101.h"
/*

Link layer state: IDLE
RCVD: 10 49 01 4a 16 
Link layer state: AVAILABLE
SEND: 10 0b 01 0c 16 
RCVD: 10 40 01 41 16 
SEND: e5 
Received reset CU
RCVD: 10 5b 01 5c 16 
Link layer state: ERROR
Link layer state: IDLE
RCVD: 10 5b 01 5c 16 
Link layer state: ERROR
Link layer state: IDLE
RCVD: 10 5b 01 5c 16 
Link layer state: ERROR
Link layer state: IDLE
RCVD: 10 5b 01 5c 16 
Link layer state: ERROR
Link layer state: IDLE
RCVD: 10 49 01 4a 16 
Link layer state: AVAILABLE
SEND: 10 0b 01 0c 16 
RCVD: 10 40 01 41 16 
SEND: e5 
Received reset CU
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 87 01 00 0d 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 88 01 00 0e 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 89 01 00 0f 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 8a 01 00 10 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 8b 01 00 11 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 8c 01 00 12 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 8d 01 00 13 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 8e 01 00 14 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 8f 01 00 15 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 90 01 00 16 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 91 01 00 17 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 92 01 00 18 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 93 01 00 19 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 94 01 00 1a 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 95 01 00 1b 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 96 01 00 1c 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 97 01 00 1d 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 98 01 00 1e 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 99 01 00 1f 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 9a 01 00 20 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 9b 01 00 21 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 9c 01 00 22 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 9d 01 00 23 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 9e 01 00 24 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 9f 01 00 25 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a0 01 00 26 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a1 01 00 27 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a2 01 00 28 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a3 01 00 29 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a4 01 00 2a 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a5 01 00 2b 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a6 01 00 2c 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a7 01 00 2d 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a8 01 00 2e 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 a9 01 00 2f 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 aa 01 00 30 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 ab 01 00 31 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 ac 01 00 32 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 ad 01 00 33 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 ae 01 00 34 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 af 01 00 35 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 b0 01 00 36 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 b1 01 00 37 16 
Link layer state: IDLE

*/

void printCP56Time2a(CP56Time2a time)
{
    printf("%02i:%02i:%02i %02i/%02i/%04i", CP56Time2a_getHour(time),
                             CP56Time2a_getMinute(time),
                             CP56Time2a_getSecond(time),
                             CP56Time2a_getDayOfMonth(time),
                             CP56Time2a_getMonth(time),
                             CP56Time2a_getYear(time) + 2000);
}

/* Callback handler to log sent or received messages (optional) */
/*
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 fb 00 00 80 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 fc 00 00 81 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 fd 00 00 82 16 
RCVD: 10 7b 01 7c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 fe 00 00 83 16 
RCVD: 10 5b 01 5c 16 
SEND: 68 0e 0e 68 08 01 0b 01 01 00 01 00 6e 00 00 ff 00 00 84 16 
RCVD: 10 7b 01 7c 16 
*/
void rawMessageHandler (void* parameter, uint8_t* msg, int msgSize, bool sent)
{
    if (sent) {
        printf("SEND: ");
    }
    else {
        printf("RCVD: ");
    }

    int i;
    for (i = 0; i < msgSize; i++) {
        printf("%02x ", msg[i]);
    }

    printf("\n");
}

/* Callback handler that is called when a clock synchronization command is received */
bool clockSyncHandler (void* parameter, IMasterConnection connection, CS101_ASDU asdu, CP56Time2a newTime)
{
    printf("Process time sync command with time "); printCP56Time2a(newTime); printf("\n");

    return true;
}

/* Callback handler that is called when an interrogation command is received */
bool interrogationHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu, uint8_t qoi)
{
    printf("Received interrogation for group %i\n", qoi);

    if (qoi == 20) { /* only handle station interrogation 总召唤限定词*/

        CS101_AppLayerParameters alParams = IMasterConnection_getApplicationLayerParameters(connection);

        IMasterConnection_sendACT_CON(connection, asdu, false);

        /* The CS101 specification only allows information objects without timestamp in GI responses */

        CS101_ASDU newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION,
                0, 1, false, false);

        InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, 100, -1, IEC60870_QUALITY_GOOD);

        CS101_ASDU_addInformationObject(newAsdu, io);

        CS101_ASDU_addInformationObject(newAsdu, (InformationObject)
            MeasuredValueScaled_create((MeasuredValueScaled) io, 101, 23, IEC60870_QUALITY_GOOD));

        CS101_ASDU_addInformationObject(newAsdu, (InformationObject)
            MeasuredValueScaled_create((MeasuredValueScaled) io, 102, 2300, IEC60870_QUALITY_GOOD));

        InformationObject_destroy(io);

        IMasterConnection_sendASDU(connection, newAsdu);

        CS101_ASDU_destroy(newAsdu);

        newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION,
                    0, 1, false, false);

        io = (InformationObject) SinglePointInformation_create(NULL, 104, true, IEC60870_QUALITY_GOOD);

        CS101_ASDU_addInformationObject(newAsdu, io);

        CS101_ASDU_addInformationObject(newAsdu, (InformationObject)
            SinglePointInformation_create((SinglePointInformation) io, 105, false, IEC60870_QUALITY_GOOD));

        InformationObject_destroy(io);

        IMasterConnection_sendASDU(connection, newAsdu);

        CS101_ASDU_destroy(newAsdu);

        newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
                0, 1, false, false);

        CS101_ASDU_addInformationObject(newAsdu, io = (InformationObject) SinglePointInformation_create(NULL, 300, true, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 301, false, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 302, true, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 303, false, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 304, true, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 305, false, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 306, true, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 307, false, IEC60870_QUALITY_GOOD));

        InformationObject_destroy(io);

        IMasterConnection_sendASDU(connection, newAsdu);

        CS101_ASDU_destroy(newAsdu);

        IMasterConnection_sendACT_TERM(connection, asdu);
    }
    else {
        IMasterConnection_sendACT_CON(connection, asdu, true);
    }

    return true;
}

bool asduHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu)
{
    if (CS101_ASDU_getTypeID(asdu) == C_SC_NA_1) {
        printf("received single command\n");

        if  (CS101_ASDU_getCOT(asdu) == CS101_COT_ACTIVATION) {
            InformationObject io = CS101_ASDU_getElement(asdu, 0);

            if (io) {
                if (InformationObject_getObjectAddress(io) == 5000) {
                    SingleCommand sc = (SingleCommand) io;

                    printf("IOA: %i switch to %i\n", InformationObject_getObjectAddress(io),
                            SingleCommand_getState(sc));

                    CS101_ASDU_setCOT(asdu, CS101_COT_ACTIVATION_CON);
                }
                else
                    CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_IOA);

                InformationObject_destroy(io);
            }
            else {
                printf("ERROR: ASDU contains no information object!\n");
                return true;
            }

        }
        else
            CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_COT);

        IMasterConnection_sendASDU(connection, asdu);

        return true;
    }

    return false;
}


void resetCUHandler(void* parameter)
{
    printf("Received reset CU\n");

    CS101_Slave_flushQueues((CS101_Slave) parameter);
}


void linkLayerStateChanged(void* parameter, int address, LinkLayerState state)
{
    printf("Link layer state: ");

    switch (state) {
    case LL_STATE_IDLE:
        printf("IDLE\n");
        break;
    case LL_STATE_ERROR:
        printf("ERROR\n");
        break;
    case LL_STATE_BUSY:
        printf("BUSY\n");
        break;
    case LL_STATE_AVAILABLE:
        printf("AVAILABLE\n");
        break;
    }
}