//#define _TRACE_PRINT_
#include "lmlu2.h"

BYTE Led_Command_Buffer[Led_Command_Buffer_Size];
BYTE Raw_Data_Buffer[Raw_Data_Buffer_Size];

static const LED_Info RUN[] = 
{//绿红蓝黄白
{.mode = 1, .ledstart = 0, .ledend = 15, .red = 0, .green = 0xFF, .blue = 0, .interval_high = 0, .interval_low = 0, .duty = 0, .red2 = 0, .green2 = 0, .blue2 = 0, .direction = 0, .u8Change = 0, .layout = 0, .red3 = 0, .green3 = 0, .blue3 = 0, .red4 = 0, .green4 = 0, .blue4 = 0},
{.mode = 1, .ledstart = 0, .ledend = 15, .red = 0xFF, .green = 0, .blue = 0, .interval_high = 0, .interval_low = 0, .duty = 0, .red2 = 0, .green2 = 0, .blue2 = 0, .direction = 0, .u8Change = 0, .layout = 0, .red3 = 0, .green3 = 0, .blue3 = 0, .red4 = 0, .green4 = 0, .blue4 = 0},
{.mode = 1, .ledstart = 0, .ledend = 15, .red = 0, .green = 0, .blue = 0xFF, .interval_high = 0, .interval_low = 0, .duty = 0, .red2 = 0, .green2 = 0, .blue2 = 0, .direction = 0, .u8Change = 0, .layout = 0, .red3 = 0, .green3 = 0, .blue3 = 0, .red4 = 0, .green4 = 0, .blue4 = 0},
{.mode = 1, .ledstart = 0, .ledend = 15, .red = 0xFF, .green = 0xFF, .blue = 0, .interval_high = 0, .interval_low = 0, .duty = 0, .red2 = 0, .green2 = 0, .blue2 = 0, .direction = 0, .u8Change = 0, .layout = 0, .red3 = 0, .green3 = 0, .blue3 = 0, .red4 = 0, .green4 = 0, .blue4 = 0},
{.mode = 1, .ledstart = 0, .ledend = 15, .red = 0xFF, .green = 0xFF, .blue = 0xFF, .interval_high = 0, .interval_low = 0, .duty = 0, .red2 = 0, .green2 = 0, .blue2 = 0, .direction = 0, .u8Change = 0, .layout = 0, .red3 = 0, .green3 = 0, .blue3 = 0, .red4 = 0, .green4 = 0, .blue4 = 0},
};
    

static BYTE u8LEDSOC = 0;
static BYTE u8SOC_RECORD = 0;

USHORT Byte_To_2ASCII(BYTE byte, BYTE is_high_bit_in_low_address)
{
    USHORT data;
    BYTE *pdata = (BYTE *)&data;
    int low_bit = 0;
    int high_bit = 0;

    low_bit = byte & 0x0F;
    high_bit = (byte & 0xF0) >> 4;

    if (high_bit >= 0 && high_bit <= 9)
    {
        if (is_high_bit_in_low_address)
            pdata[0] = high_bit + '0';
        else
            pdata[1] = high_bit + '0';
    }
    else if (high_bit >= 10 && high_bit <= 15)
    {
        if (is_high_bit_in_low_address)
            pdata[0] = high_bit - 10 + 'A';
        else
            pdata[1] = high_bit - 10 + 'A';
    }

    if (low_bit >= 0 && low_bit <= 9)
    {
        if (is_high_bit_in_low_address)
            pdata[1] = low_bit + '0';
        else
            pdata[0] = low_bit + '0';
    }
    else if (low_bit >= 10 && low_bit <= 15)
    {
        if (is_high_bit_in_low_address)
            pdata[1] = low_bit - 10 + 'A';
        else
            pdata[0] = low_bit - 10 + 'A';
    }

    return data;
}

UINT Generate_Length_Area(USHORT ascii_length)
{
    UINT length_area = 0;
    USHORT check_number = 0;
    USHORT *p = 0;

    ascii_length &= 0x0FFF;

    check_number = (ascii_length & 0x000F) + ((ascii_length & 0x00F0) >> 4) + ((ascii_length & 0x0F00) >> 8);
    check_number = ((~(check_number % 16)) + 1) & 0x000F;

    ascii_length |= (check_number << 12);

    p = (USHORT *)&length_area;
    p[0] = Byte_To_2ASCII((ascii_length & 0xFF00) >> 8, 1);
    p[1] = Byte_To_2ASCII(ascii_length & 0x00FF, 1);

    return length_area;
}

void SRV_IMLU2_SetLedPattern(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, BYTE color_type, BYTE u8LenNum, BYTE u8Segment, const LED_Info *pstLedInfo, const LED_Info *pstLedInfo2)
{
    UINT row_data_length = 0;
    UINT data_length = 0;
    const LED_Info *pstLedInfoTemp;

    memset(Raw_Data_Buffer, 0, Raw_Data_Buffer_Size);
    memset(Led_Command_Buffer, 0, Led_Command_Buffer_Size);

    Led_Command_Buffer[Start_Flag_Address] = 0x7E;

    Led_Command_Buffer[Version_Address_0] = '2';
    Led_Command_Buffer[Version_Address_1] = '1';

    Led_Command_Buffer[ADR_Address_0] = 'C';
    Led_Command_Buffer[ADR_Address_1] = '3';

    Led_Command_Buffer[CID1_Address_0] = '6';
    Led_Command_Buffer[CID1_Address_1] = '2';

    Led_Command_Buffer[CID2_Address_0] = '4';
    Led_Command_Buffer[CID2_Address_1] = '2';

    switch(color_type)
    {
        case LEDS_RGB:Raw_Data_Buffer[row_data_length] = 0x12; break; // 控制的对象和颜色顺序
        case LEDS_RBG:Raw_Data_Buffer[row_data_length] = 0x22; break; // 控制的对象和颜色顺序
        case LEDS_GRB:Raw_Data_Buffer[row_data_length] = 0x32; break; // 控制的对象和颜色顺序
        case LEDS_GBR:Raw_Data_Buffer[row_data_length] = 0x42; break; // 控制的对象和颜色顺序
        case LEDS_BRG:Raw_Data_Buffer[row_data_length] = 0x52; break; // 控制的对象和颜色顺序
        case LEDS_BGR:Raw_Data_Buffer[row_data_length] = 0x62; break; // 控制的对象和颜色顺序
        default:Raw_Data_Buffer[row_data_length] = 0xF2; break; // 控制的对象和颜色顺序
    }

    //TRACEI("color type ----%d    %x",  color_type, Raw_Data_Buffer[row_data_length]);

    row_data_length++;
    // 1
    Raw_Data_Buffer[row_data_length] = u8LenNum; // 灯珠的总个数
    row_data_length++;

    Raw_Data_Buffer[row_data_length] = u8Segment; // 灯的段数
    row_data_length++;

     int i = 0;
    for (i = 0; i < u8Segment; i++)
    {
        pstLedInfoTemp = pstLedInfo + i;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->mode; // 运行模式
        row_data_length++;

        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->ledstart; // 首灯序号
        row_data_length++;

        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->ledend; // 尾灯序号
        row_data_length++;

        // 第一段灯的颜色
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->red; // R
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->green; // G
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->blue; // B
        row_data_length++;
        if((pstLedInfoTemp->red == 0xFF)&&(pstLedInfoTemp->green==0)&&(pstLedInfoTemp->blue==0)) 
        {
            return;
        }
        //TRACEI("R  G  B  %d  %d   %d", pstLedInfoTemp->red, pstLedInfoTemp->green, pstLedInfoTemp->blue);

        // 第一段灯的变化间隔
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->interval_high;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->interval_low;
        row_data_length++;

        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->duty; // 第一段灯跑马灯模式的占空比
        row_data_length++;

        // 第一段灯的静态颜色
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->red2; // R
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->green2; // G
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->blue2; // B
        row_data_length++;

        // 第一段灯的跑马方向
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->direction;
        row_data_length++;

        // 第一段灯的渐变颜色开关
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->u8Change;
        row_data_length++;

        // 第一段灯的格子内布局
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->layout;
        row_data_length++;

        // 第一段灯的跑马灯底色
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->red3;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->green3;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->blue3;
        row_data_length++;

        // 第一段灯的渐变目标色
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->red4;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->green4;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->blue4;
        row_data_length++;
        row_data_length += 3; // 预留3个字节
    }
    // 1 END

    // 2
    Raw_Data_Buffer[row_data_length] = u8LenNum; // 灯珠的总个数
    row_data_length++;

    Raw_Data_Buffer[row_data_length] = u8Segment; // 灯的段数
    row_data_length++;

    for (i = 0; i < u8Segment; i++)
    {
        pstLedInfoTemp = pstLedInfo2 + i;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->mode; // 运行模式
        row_data_length++;

        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->ledstart; // 首灯序号
        row_data_length++;

        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->ledend; // 尾灯序号
        row_data_length++;

        // 第一段灯的颜色
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->red; // R
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->green; // G
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->blue; // B
        row_data_length++;
        //TRACEI("R  G  B  %d  %d   %d", pstLedInfoTemp->red, pstLedInfoTemp->green, pstLedInfoTemp->blue);

        // 第一段灯的变化间隔
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->interval_high;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->interval_low;
        row_data_length++;

        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->duty; // 第一段灯跑马灯模式的占空比
        row_data_length++;

        // 第一段灯的静态颜色
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->red2; // R
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->green2; // G
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->blue2; // B
        row_data_length++;

        // 第一段灯的跑马方向
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->direction;
        row_data_length++;

        // 第一段灯的渐变颜色开关
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->u8Change;
        row_data_length++;

        // 第一段灯的格子内布局
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->layout;
        row_data_length++;

        // 第一段灯的跑马灯底色
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->red3;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->green3;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->blue3;
        row_data_length++;

        // 第一段灯的渐变目标色
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->red4;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->green4;
        row_data_length++;
        Raw_Data_Buffer[row_data_length] = pstLedInfoTemp->blue4;
        row_data_length++;
        row_data_length += 3; // 预留3个字节
    }
    // 2 END

    // 将数据域转换成ASCII码填充
    for (i = 0; i < row_data_length; i++)
    {
        *(USHORT *)&Led_Command_Buffer[Data_Area_Address + (i * 2)] = Byte_To_2ASCII(Raw_Data_Buffer[i], 1);
    }

    data_length = row_data_length * 2;

    // 填充长度域
    *(UINT *)&Led_Command_Buffer[Length_Address_Address_0] = Generate_Length_Area(data_length);

    // 填充校验域
    USHORT check_number = 0;
    for (i = Version_Address_0; i < Data_Area_Address + data_length; i++)
    {
        check_number += Led_Command_Buffer[i];
    }
    check_number = (~(check_number % 65535)) + 1;

    ((USHORT *)&Led_Command_Buffer[Data_Area_Address + data_length])[0] = Byte_To_2ASCII(check_number >> 8, 1);
    ((USHORT *)&Led_Command_Buffer[Data_Area_Address + data_length])[1] = Byte_To_2ASCII(check_number, 1);

    // 报文结束标志
    Led_Command_Buffer[Data_Area_Address + data_length + 4] = 0x0D;

    // 发送报文x
    // if (pstUart->fSendCallback != NULL_PTR) {
    // pstUart->fSendCallback(Led_Command_Buffer, Data_Area_Address + data_length + 4 + 1);
    char szSett[]  = "9600, N, 8, 1";
    COMMPORT* pComm = ((SAMPLER*)pDev_485_2_Rough->hSampler)->pUsedCommPort;
    pComm->Control(pDev_485_2_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)szSett, strlen(szSett));
	pComm->Control(pDev_485_2_Rough->hPort, COMM_PURGE_TXCLEAR, NULL, 0);
    TRACEI("data:%s\n ----------size:%d\n", Led_Command_Buffer, Data_Area_Address + data_length + 4 + 1);

    ssize_t nWriteSize = pComm->Write(pDev_485_2_Rough->hPort, Led_Command_Buffer, Data_Area_Address + data_length + 4 + 1);
    Sleep(240);

    if(nWriteSize < 0)
	{
		TRACEI("lmlu2 Write To RS485 Port Failed !!!\n");
	}
    TRACEI("Write To RS485 data size %d\n", nWriteSize);

    return;
}


//WLD01206 LED1 and LED2 control
void LED_RS485_Ctl_State(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough)
{

	HANDLE hSamplerLock = ((SAMPLER*)pDev_485_2_Rough->hSampler)->hSamplerLock;
	if(Mutex_Lock(hSamplerLock, MAX_DEV_485_2_LOCK_WAIT) != ERR_MUTEX_OK)
	{
		return;
	}

    LED_STATE  curLED1State = GREEN;
    LED_STATE  curLED2State = GREEN;

	STBATT_RUN_STATU emSTBatt_WorkStatu = STBATT_RUNSTAT_IDLE;
    int n = 0;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(pDev_485_2_Rough->esRunData.fSTBatt_OutCurr[n] >= 2.0) 
		{
			emSTBatt_WorkStatu = STBATT_RUNSTAT_CHARGE;
			continue;
		}
		if(pDev_485_2_Rough->esRunData.fSTBatt_OutCurr[n] <= -2.0)
		{
			emSTBatt_WorkStatu = STBATT_RUNSTAT_DISCHARGE;
			continue;
		}
	}

    if(pDev_485_2_Rough->bySTSOffGrid > 0) // 0 并网  1 离网
    {
        curLED1State = WHITE;   //离网 白色
    }
    else if(emSTBatt_WorkStatu == STBATT_RUNSTAT_CHARGE)
    {
        curLED1State = YELLOW;   //充电  黄色
    }
    else if(emSTBatt_WorkStatu == STBATT_RUNSTAT_DISCHARGE)
    {
        curLED1State = BLUE;    //放电  蓝色
    }
    else
    {
        curLED1State = GREEN;   //空闲  绿色
    }


    if((pDev_485_2_Rough->emCCUState == CCU_STATE_FAULTED)||(pDev_485_2_Rough->emBattFaultStatu == STBATT_ALMSTAT_FAULT))
	{
        curLED2State = RED;     //故障  红色
	}
    else
    {
        curLED2State = GREEN;   //正常  绿色
    }

    if((curLED1State < 0)||(curLED1State > 4)||(curLED2State < 0)||(curLED2State > 1)||(curLED1State==RED))
    {
        return;
    }
    SRV_IMLU2_SetLedPattern(pDev_485_2_Rough, LEDS_GRB, 15, 1, RUN + curLED1State, RUN + (int)curLED2State);

    Mutex_Unlock(hSamplerLock);
    return;
}