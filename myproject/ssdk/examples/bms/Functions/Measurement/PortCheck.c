/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : PortCheck.c
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : IO端口检测模块
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupPara.h"
#include "PortCheck.h"
#include "DiagCheck.h"

extern u16 gGBmuHigLevPara_103[];       /*主控高级参数值*/
extern u8 gGHardPara_104[];             /*主控硬件参数配置信息*/

/*=============================================================================================
** @Name      : void PortStateCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 实时更新IO状态到相应寄存器
** @The notes : 10ms调用,后续补充
**===========================================================================================*/
void PortStateCheckTask(void)
{

}
/*=============================================================================================
** @Name      : u8 GetGSmogIoWarStateAPI(void)
** @Input     : void
** @Output    : 烟感状态:0正常 1报警
** @Function  : 获取烟感报警状态,反馈信号，常态为闭合(高电平)
** @The notes : 用户调用
**===========================================================================================*/
u8 GetGSmogIoWarStateAPI(void)
{
    /*未使能该端口检测*/
    if(0 == gGBmuHigLevPara_103[eBmuHigLevPara103_SmogPort])
    {
        return(0);
    }
	/*A2504,836项目*/
	if(gGHardPara_104[eGHardPara104_ProjectCode] == 0)
	{
	 	 /*PDU外IMEU3无通信中断且使能PDU外IMEU3通信功能*/
	 	 if((0 == DiagGetInImeu3OffCommuStateAPI()) && ((gGHardPara_104[eGHardPara104_Imeu3En] & 0x02) != 0))
	 	 {
	 		 /*常开低电平(高电平1为烟雾报警)[默认配置值:8]*/
	 	    if((BitGet(GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI1_16),3) == 1) && (gGBmuHigLevPara_103[eBmuHigLevPara103_SmogPort] == 8))
	 	    {
	 	        return(1);
	 	    }
	 		 /*常闭高电平(低电平0为烟雾报警)[默认配置值:32776]*/
	 	    else if((BitGet(GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI1_16),3) == 0) && (gGBmuHigLevPara_103[eBmuHigLevPara103_SmogPort] == 32776))
	 	    {
	 	        return(1);
	 	    }
	 	    else
	 	    {
	 	        return(0);
	 	    }
	 	 }
	 	 else
	 	 {
	 		return(0);
	 	 }
	}
    //光储直柔项目后续处理
    else
    {
        return(0);
    }
}
/*=============================================================================================
** @Name      : u8 GetGEmergIoButtonStateAPI(void)
** @Input     : void
** @Output    : 急停按钮状态:0正常 1按下（急停）
** @Function  : 获取急停按钮状态
** @The notes : 用户调用
**===========================================================================================*/
u8 GetGEmergIoButtonStateAPI(void)
{
    /*未使能该端口检测*/
    if((0 == gGBmuHigLevPara_103[eBmuHigLevPara103_EmergPort])||(BSPGetOSMillSecTimeAPI() / 1000 < 3))/*启动3秒内不检测*/
    {
        return(0);
    }
    /*DI9反馈断开(0)=急停按下*/
    if(DEVGPIOGetInPortState(ePIn_DI9) == 0)
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

