/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVADC.c
** @Founder      : ZQD808
** @Date         : 2025.11.14
** @Function     : ADC采样通道管理函数模块
** @Instructions : 根据PCB外围设备编写管理函数
**===========================================================================================*/
#include <arch.h>
#include <cmsis_os2.h>
#include <board.h>
#include "driver/kunlun/adc/kunlun_adc.h"
#include "driver/adc.h"
#include <stdio.h>
#include <reg.h>
#include <string.h>
#include <param.h>

#include "regs_base.h"
#include "irq_num.h"
#include "CLI_type.h"
#include "DEVGPIO.h"
#include "BSPTypeDef.h"
#include "DEVADC.h"
#include "DEVGPIO.h"
#include "GroupInfo.h"
#include <stdio.h>
#include <debug.h>
#include "TaskMonitor.h"

static u8 s4051MuxFLag = 0;   //4051通道选中标志
adc_dev_t dev;

static u8 sADCSampExpFlag = 0;                  /*ADC采样异常：bit0启动失败/回调超时，bit1原始值越界*/
static u8 sADCSampPending = 0U;                 /*本轮adc_start成功后，等待有效采样回调的标志*/

static u16 LowAdcSampInfo[e_AdcchannelNum] = {0x00};  //ADC采样结果数组

extern u16 *gDevSampInfo_14;                       /*板级采样信息*/

DEFINE_CHANNEL_MASK(channel);
adc_buff_t adc_buff[3];
static void devadc_callback(adc_buff_t *buff, uint16_t len, void *context)
{
    if(gDevSampInfo_14 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gDevSampInfo_14 null\r\n", __func__);
        return;
    }
    if(len == 0U)
    {
        return;
    }

    /*收到本轮有效ADC回调，采样链路恢复。*/
    sADCSampPending = 0U;
    BitClr(sADCSampExpFlag, 0);
    for (int i = 0; i < len; i++) {
        if(i == 0)
        {
            LowAdcSampInfo[s4051MuxFLag] = ChangeSampADtoData(s4051MuxFLag, buff[i].sample_val);
            if(s4051MuxFLag == e_MuxChannel_VCC1V8)
            {
                gDevSampInfo_14[eDevSamp_1V8] = LowAdcSampInfo[s4051MuxFLag];
            }
            else if(s4051MuxFLag == e_MuxChannel_VCC3V3)
            {
                gDevSampInfo_14[eDevSamp_3V3] = LowAdcSampInfo[s4051MuxFLag];
            }
            else if(s4051MuxFLag == e_MuxChannel_4G3V5)
            {
                gDevSampInfo_14[eDevSamp_3V5] = LowAdcSampInfo[s4051MuxFLag];
            }
            else if(s4051MuxFLag == e_MuxChannel_VCC5V)
            {
                gDevSampInfo_14[eDevSamp_5V0] = LowAdcSampInfo[s4051MuxFLag];
            }
            else if(s4051MuxFLag == e_MuxChannel_VCCH5V)
            {
                gDevSampInfo_14[eDevSamp_5V1] = LowAdcSampInfo[s4051MuxFLag];
            }
            else if(s4051MuxFLag == e_MuxChannel_VCC12V)
            {
                gDevSampInfo_14[eDevSamp_12V] = LowAdcSampInfo[s4051MuxFLag];
            }
        }
    }
}
/*=============================================================================================
** @Name      : u8 DEVADCInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : ADC端口初始化
** @The notes : 只初始化板级提供的资源
**===========================================================================================*/
u8 DEVADCInit(void)
{
    s32 ret;
    adc_config_t adc_config = {
        .irq            = DEVICE_INTR(ADC),
        .base           = DEVICE_BASE(ADC),
        .resolution     = 12,
        .inter_ref_volt = false,
        .sample_freq    = 50000,
        .xfer_mode    = ADC_XFER_INTERRUPT_MODE_E,
        .is_diff        = false,
    };
    ret = kunlun_adc_probe(&dev, &adc_config);
    if(ret != 0) {
        ssdk_printf(SSDK_WARNING, "%s: kunlun_adc_probe error!!!\r\n", __func__);
        return CLI_FALSE;
    }
    adc_set_callback(&dev, devadc_callback, NULL);
    s4051MuxFLag = 0;
    return 0;
}

/*=============================================================================================
** @Name      : void DEVADCSetMuxChannel(e_MUXCHANNEL num)
** @Input     : num :4051复用通道选择
** @Output    : void
** @Function  : 选择复用ADC采集通道
** @The notes : 
**===========================================================================================*/
void DEVADCSetMuxChannel(e_MUXCHANNEL num)
{
    #if(BOARDX1_TYPE == 0)
    switch(num)
    {
        //复用通道选中VCC1.2V采样通道
        case e_MuxChannel_VCC1V2:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_OFF);
        break;

        //复用通道选中VCC1.8V采样通道
        case e_MuxChannel_VCC1V8:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_OFF);
        break;

        //复用通道选中VCC2.5V采样通道
        case e_MuxChannel_VCC2V5:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_OFF);
        break;

        //复用通道选中VCC3.3V采样通道
        case e_MuxChannel_VCC3V3:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_OFF);
        break;

        //复用通道选中4G3.5V采样通道
        case e_MuxChannel_4G3V5:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_ON);
        break;

         //复用通道选中VCC5V采样通道
        case e_MuxChannel_VCC5V:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_ON);
        break;

        //复用通道选中VCCH5V采样通道
        case e_MuxChannel_VCCH5V:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_ON);
        break;

        //复用通道选中VCC12V采样通道
        case e_MuxChannel_VCC12V:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_ON);
        break;

        default:
        break;
    }
    #elif(BOARDX1_TYPE == 1)
    switch(num)
    {
        //复用通道选中VCC1.8V采样通道
        case e_MuxChannel_VCC1V8:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_OFF);
        break;

        //复用通道选中VCC3.3V采样通道
        case e_MuxChannel_VCC3V3:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_OFF);
        break;

        //复用通道选中4G3.5V采样通道
        case e_MuxChannel_4G3V5:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_ON);
        break;

         //复用通道选中VCC5V采样通道
        case e_MuxChannel_VCC5V:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_ON);
        break;

        //复用通道选中VCCH5V采样通道
        case e_MuxChannel_VCCH5V:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_OFF);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_ON);
        break;

        //复用通道选中VCC12V采样通道
        case e_MuxChannel_VCC12V:
        DEVGPIOSetPortState(ePOut_4051_S0,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S1,ePSTATE_ON);
        DEVGPIOSetPortState(ePOut_4051_S2,ePSTATE_ON);
        break;

        default:
        break;
    }
    #endif

}

/*=============================================================================================
** @Name      : void DEVADVSetMuxFlag(u8 flag)
** @Input     : flag:4051复用通道标志
** @Output    : void
** @Function  : 设置4051复用通道标志
** @The notes : 
**===========================================================================================*/
void DEVADVSetMuxFlag(u8 flag)
{
    if(flag > 7)
        return;
    s4051MuxFLag = flag;
}
/*=============================================================================================
** @Name      : vu8 DECADVGetMuxFlag(void)
** @Input     : void
** @Output    : s4051MuxFLag
** @Function  : 获取4051复用通道标志
** @The notes : 
**===========================================================================================*/
u8 DECADVGetMuxFlag(void)
{
    return (s4051MuxFLag);
}
/*=============================================================================================
** @Name      : u8 DEVADCGetSampExpFlagAPI(void)
** @Output    : ADC采样链路异常位(bit0启动失败/回调超时，bit1原始值越界)
** @Function  : 为诊断ID181提供BMU本板低压采样链路状态
** @The notes : 保持100.37口径：检测ADC启动、回调完成和原始值有效性；不以未确认的电源轨容差作故障判定。
**===========================================================================================*/
u8 DEVADCGetSampExpFlagAPI(void)
{
    return sADCSampExpFlag;
}
/*=============================================================================================
** @Name      : void LowInfoSampleTask(void *argument)
** @Input     : *argument:形参,任务函数参数需要
** @Output    : void
** @Function  : ADC通道电压电流采样任务
** @The notes : 通过MCU的ADC通道读取
**===========================================================================================*/
void LowInfoSampleTask(void *argument)
{
    s32 ret;
    static u8 sAdcMuxChan = 0;  
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        DEVADVSetMuxFlag(sAdcMuxChan);
        DEVADCSetMuxChannel(DECADVGetMuxFlag());
        #if(BOARDX1_TYPE == 0)
        if(sAdcMuxChan < 7)
        {
            sAdcMuxChan++;
        }
        else
        {
            sAdcMuxChan = 0;
        }
        #elif(BOARDX1_TYPE == 1)
        if(sAdcMuxChan < 5)
        {
            sAdcMuxChan++;
        }
        else
        {
            sAdcMuxChan = 0;
        }
        #endif
        osDelay(50); //确保引脚复用切换完成
        adc_clear_all_channel_mask(&channel);
        adc_add_channel_mask(&channel, 3, 1, 2, 3);
        /*先置等待标志，避免同步回调发生在adc_start返回之前而被漏记。*/
        sADCSampPending = 1U;
        ret = adc_start(&dev, &channel, adc_buff, ARRAY_SIZE(adc_buff));
        if (ret != 0) {
            sADCSampPending = 0U;
            BitSet(sADCSampExpFlag, 0);
            printf("adc_start error\n");
            osDelay(100);
            continue;
        }
        osDelay(500);
        adc_stop(&dev);
        if(sADCSampPending != 0U)
        {
            /*与100.37中ADC忙超时同义：本轮成功启动后未取得有效采样结果。*/
            BitSet(sADCSampExpFlag, 0);
        }
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_LowAdcSamp, execution_time);
    }
    return;
}
/*=============================================================================================
** @Name      : static u16 ChangeSampADtoData(u8 channel, u16 ADdata)
** @Input     : channel:通道号 ADdata:采样AD值
** @Output    : 采样目标值 10mv
** @Function  : 将采样AD值转换为采样目标值
** @The notes : 电压值
**===========================================================================================*/
static u16 ChangeSampADtoData(u8 channel, u16 ADdata)
{
    s32 tempdata = 0;
    /*MCU12位AD采样最大值4096*/
    if(ADdata >= 4096)
    {
        ADdata = 0;
        BitSet(sADCSampExpFlag, 1);
    }
    else
    {
        BitClr(sADCSampExpFlag, 1);
    }

    switch(channel)
    {
        #if(BOARDX1_TYPE == 0)
        case e_MuxChannel_VCC1V2:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        tempdata = tempdata * (ADC1_MUX0_R1 + ADC1_MUX0_R2) / ADC1_MUX0_R2;
        break;
        #endif

        case e_MuxChannel_VCC1V8:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        tempdata = tempdata * (ADC1_MUX1_R1 + ADC1_MUX1_R2) / ADC1_MUX1_R2;
        break;
        #if(BOARDX1_TYPE == 0)
        case e_MuxChannel_VCC2V5:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        tempdata = tempdata * (ADC1_MUX2_R1 + ADC1_MUX2_R2) / ADC1_MUX2_R2;
        break;
        #endif

        case e_MuxChannel_VCC3V3:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        tempdata = tempdata * (ADC1_MUX3_R1 + ADC1_MUX3_R2) / ADC1_MUX3_R2;
        break;

        case e_MuxChannel_4G3V5:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        tempdata = tempdata * (ADC1_MUX4_R1 + ADC1_MUX4_R2) / ADC1_MUX4_R2;
        break;

        case e_MuxChannel_VCC5V:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        tempdata = tempdata * (ADC1_MUX5_R1 + ADC1_MUX5_R2) / ADC1_MUX5_R2;
        break;

        case e_MuxChannel_VCCH5V:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        tempdata = tempdata * (ADC1_MUX6_R1 + ADC1_MUX6_R2) / ADC1_MUX6_R2;
        break;

        case e_MuxChannel_VCC12V:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        tempdata = tempdata * (ADC1_MUX7_R1 + ADC1_MUX7_R2) / ADC1_MUX7_R2;
        break;
        #if(BOARDX1_TYPE == 0)
        //板载两路AD，暂不处理
        case e_Adcchannel2:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        break;

        case e_Adcchannel3:
        tempdata = (u16)((u32)ADdata * 1800 / 4096);
        break;
        #endif

        default:
        tempdata = 0xffff;
        break;
    }
    return (tempdata);
}
/*=============================================================================================
** @Name      : u16 GetLowAdcInfo(u8 channel)
** @Input     : channel:通道号
** @Output    : 采样目标值 10mv
** @Function  : 将采样AD值转换为采样目标值
** @The notes : 电压值
**===========================================================================================*/
u16 GetLowAdcInfo(u8 channel)
{
    if(channel < e_AdcchannelNum)
    {
        return(LowAdcSampInfo[channel]);
    }
    return (0);
}
