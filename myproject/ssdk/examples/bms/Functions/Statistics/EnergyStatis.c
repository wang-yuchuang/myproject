/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : EnergyStatis.c
** @Founder      : ZQD808
** @Date         : 2026.03.10
** @Function     : 电池组电能量统计模块
** @Instructions :
**===========================================================================================*/
#include "EnergyStatis.h"
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "PublicInfo.h"
#include "CurrIntegral.h"
#include "ParaRead.h"
#include "ParaWrite.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
extern u16 gGBmuGenPara_102[];                          /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	                    /*主控高级参数值*/

extern u16 *gCurrCapEner_55;                            /*当前能量统计信息*/
extern u32 *gHisCapEner_56;                             /*累计能量统计信息*/

/*=============================================================================================
** @Name      : void GroupEnerCapStatisInit(void)
** @Input     : void
** @Output    : void
** @Function  : 累计充放电电能电量统计初始化
** @The notes :
**===========================================================================================*/
void GroupEnerCapStatisInit(void)
{
	u32 info[3] = {0};
    if(gCurrCapEner_55 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gCurrCapEner_55 null!!!\r\n", __func__);
        return;
    }
    if(gHisCapEner_56 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gHisCapEner_56 null!!!\r\n", __func__);
        return;
    }

    /*单次充放电量*/
    gCurrCapEner_55[eCurrCapEner55_1ChgCap] = 0; 
    gCurrCapEner_55[eCurrCapEner55_1DhgCap] = 0; 
    gCurrCapEner_55[eCurrCapEner55_2ChgCap] = 0; 
    gCurrCapEner_55[eCurrCapEner55_2DhgCap] = 0; 

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        /*累计充放电量*/
        if(TRUE == ParaReadStoreHisChgDhgCap(clunum, info, 3))
        {
            if(clunum == 0)
            {
                gHisCapEner_56[eHisCapEner56_1HisChgCap] = info[1];
                gHisCapEner_56[eHisCapEner56_1HisDhgCap] = info[2];
            }
            else
            {
                gHisCapEner_56[eHisCapEner56_2HisChgCap] = info[1];
                gHisCapEner_56[eHisCapEner56_2HisDhgCap] = info[2]; 
            }
        }
        else
        {
            if(clunum == 0)
            {
                gHisCapEner_56[eHisCapEner56_1HisChgCap] = gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap] / 20;
                gHisCapEner_56[eHisCapEner56_1HisDhgCap] = 0;
            }
            else
            {
                gHisCapEner_56[eHisCapEner56_2HisChgCap] = gGBmuGenPara_102[eBmuGenPara102_2ReaAllCap] / 20;
                gHisCapEner_56[eHisCapEner56_2HisDhgCap] = 0;
            }
        }

        /*累计充放电能*/
        if(TRUE == ParaReadStoreHisChgDhgEner(clunum, info, 3))
        {
            if(clunum == 0)
            {
                gHisCapEner_56[eHisCapEner56_1HisChgEner] = info[1];
                gHisCapEner_56[eHisCapEner56_1HisDhgEner] = info[2];
            }
            else
            {
                gHisCapEner_56[eHisCapEner56_2HisChgEner] = info[1];
                gHisCapEner_56[eHisCapEner56_2HisDhgEner] = info[2];
            }
        }
        else
        {
            if(clunum == 0)
            {
                gHisCapEner_56[eHisCapEner56_1HisChgEner] = gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] / 2;
                gHisCapEner_56[eHisCapEner56_1HisDhgEner] = 0;
            }
            else
            {
                gHisCapEner_56[eHisCapEner56_2HisChgEner] = gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] / 2;
                gHisCapEner_56[eHisCapEner56_2HisDhgEner] = 0;
            }
        }
    }
}
/*=============================================================================================
** @Name      : void GroupCycleCapStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组单次循环充放电电量统计任务
** @The notes : 400ms
**===========================================================================================*/
void GroupCycleCapStatisTask(void)
{
    u32 chgCap = 0;             /*充电电量(1AH)*/
    u32 dhgCap = 0;             /*放电电量(1AH)*/
    u16 nowState = 0;           /*当前充放电状态*/

    static u32 sHisState[2] = {0};   /*历史充放电状态*/
    static u32 sFstChgCap[2] = {0};  /*记录开始充电时的充电量*/
    static u32 sFstDhgCap[2] = {0};  /*记录开始放电时的放电量*/

    if(gCurrCapEner_55 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gCurrCapEner_55 null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        chgCap = GetChgIntCapAPI(clunum);
        dhgCap = GetDhgIntCapAPI(clunum);
        nowState = GetGChgDhgStateAPI(clunum);

        /*计算单次充放电电量*/
        if(sHisState[clunum] != nowState)                                       /*充放电状态切换*/
        {
            /*开始充电*/
            if(eCURR_CHG == nowState)                                   /*正在充电*/
            {
                sFstChgCap[clunum] = chgCap;                                    /*记录起始充电电量*/

                sHisState[clunum] = nowState;                                   /*更新充放电状态记录(充电中断再继续也当做一次)*/
            }

            /*开始放电*/
            if(eCURR_DHG == nowState)                                   /*正在充电*/
            {
                sFstDhgCap[clunum] = dhgCap;                                    /*记录起始放电电量*/

                sHisState[clunum] = nowState;                                   /*更新充放电状态记录(放电中断再继续也当做一次)*/
            }
        }

        /*小储能U16只能用10mAH为单位*/
        #if(1 == SMALL_ESS_EN)
        /*显示单次充放电电量*/
        gGStatistCapEner_72[eCapEner72_ChgCap] = (chgCap - sFstChgCap + 5) / 10;/*单次充电电量(1mAH转10mAH)*/
        gGStatistCapEner_72[eCapEner72_DhgCap] = (dhgCap - sFstDhgCap + 5) / 10;/*单次放电电量(1mAH转10mAH)*/
        /*大储能U16直接用1AH为单位*/
        #else
        /*显示单次充放电电量*/
        if(clunum == 0)
        {
            gCurrCapEner_55[eCurrCapEner55_1ChgCap] = chgCap - sFstChgCap[clunum];   /*单次充电电量*/
            gCurrCapEner_55[eCurrCapEner55_1DhgCap] = dhgCap - sFstDhgCap[clunum];   /*单次放电电量*/
        }
        else
        {            
            gCurrCapEner_55[eCurrCapEner55_2ChgCap] = chgCap - sFstChgCap[clunum];   /*单次充电电量*/
            gCurrCapEner_55[eCurrCapEner55_2DhgCap] = dhgCap - sFstDhgCap[clunum];   /*单次放电电量*/
        }
        #endif
    }
}
/*=============================================================================================
** @Name      : void GroupHisCapStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组累计历史充放电电量统计任务
** @The notes : 400ms
**===========================================================================================*/
void GroupHisCapStatisTask(void)
{
    u32 chgCap = 0;             /*充电电量(1AH)*/
    u32 dhgCap = 0;             /*放电电量(1AH)*/
    u16 nowState = 0;           /*当前充放电状态*/
    u32 chgChangCap = 0;        /*充电电量变化量(1AH)*/
    u32 dhgChangCap = 0;        /*放电电量变化量(1AH)*/

    static u32 sHisState[2] = {0};   /*历史充放电状态*/
    static u32 sHisChgCap[2] = {0};  /*记录上次充电电量(1AH)*/
    static u32 sHisDhgCap[2] = {0};  /*记录上次放电电量(1AH)*/
    static u32 sEepChgCap[2] = {0};  /*上次写EEP充电电量(1AH)*/
    static u32 sEepDhgCap[2] = {0};  /*上次写EEP放电电量(1AH)*/

    if(gHisCapEner_56 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gHisCapEner_56 null!!!\r\n", __func__);
        return;
    }

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        chgCap = GetChgIntCapAPI(clunum);
        dhgCap = GetDhgIntCapAPI(clunum);
        nowState = GetGChgDhgStateAPI(clunum);

        if(chgCap > sHisChgCap[clunum])
        {
            chgChangCap = chgCap - sHisChgCap[clunum];
        }
        else
        {
            chgChangCap = 0;        /*历史值错误则抛弃本次计算值*/
        }

        if(dhgCap > sHisDhgCap[clunum])
        {
            dhgChangCap = dhgCap - sHisDhgCap[clunum];
        }
        else
        {
            dhgChangCap = 0;        /*历史值错误则抛弃本次计算值*/
        }
        if(clunum == 0)
        {
            gHisCapEner_56[eHisCapEner56_1HisChgCap] += chgChangCap;
            gHisCapEner_56[eHisCapEner56_1HisDhgCap] += dhgChangCap;
        }
        else
        {
            gHisCapEner_56[eHisCapEner56_2HisChgCap] += chgChangCap;
            gHisCapEner_56[eHisCapEner56_2HisDhgCap] += dhgChangCap;
        }

        /*保证放电量不能多于充电量一个总容量
        if(gGStatistHisCap_73[eHisCap73_HisDhgCap] > (gGStatistHisCap_73[eHisCap73_HisChgCap] + (gGBmuGenPara_102[eBmuGenPara102_ReaAllCap] / 10)))
        {
            gGStatistHisCap_73[eHisCap73_HisDhgCap] = gGStatistHisCap_73[eHisCap73_HisChgCap] + (gGBmuGenPara_102[eBmuGenPara102_ReaAllCap] / 10);
        }*/
        if(clunum == 0)
        {
            if((ABS(gHisCapEner_56[eHisCapEner56_1HisChgCap], sEepChgCap[clunum]) >= HISCAP_CHANG_TO_WRITE_EEP) /*达到写EEP变化值*/
                || ((gHisCapEner_56[eHisCapEner56_1HisChgCap] > sEepChgCap[clunum])                             /*与EEP值不一致*/
                && (eCURR_IDLE == nowState) && (eCURR_IDLE != sHisState[clunum])))                             /*由充放电转为待机*/
            {
                /*电池组历史充电电量写EEP*/
                ParaWriteStoreHisChgDhgCap(clunum, 1, &gHisCapEner_56[eHisCapEner56_1HisChgCap], 1);
                sEepChgCap[clunum] = gHisCapEner_56[eHisCapEner56_1HisChgCap];
            }
        }
        else
        {            
            if((ABS(gHisCapEner_56[eHisCapEner56_2HisChgCap], sEepChgCap[clunum]) >= HISCAP_CHANG_TO_WRITE_EEP) /*达到写EEP变化值*/
                || ((gHisCapEner_56[eHisCapEner56_2HisChgCap] > sEepChgCap[clunum])                             /*与EEP值不一致*/
                && (eCURR_IDLE == nowState) && (eCURR_IDLE != sHisState[clunum])))                             /*由充放电转为待机*/
            {
                /*电池组历史充电电量写EEP*/
                ParaWriteStoreHisChgDhgCap(clunum, 1, &gHisCapEner_56[eHisCapEner56_2HisChgCap], 1);
                sEepChgCap[clunum] = gHisCapEner_56[eHisCapEner56_2HisChgCap];
            }
        }

        if(clunum == 0)
        {
            if((ABS(gHisCapEner_56[eHisCapEner56_1HisDhgCap], sEepDhgCap[clunum]) >= HISCAP_CHANG_TO_WRITE_EEP) /*达到写EEP变化值*/
                || ((gHisCapEner_56[eHisCapEner56_1HisDhgCap] > sEepDhgCap[clunum])                             /*与EEP值不一致*/
                && (eCURR_IDLE == nowState) && (eCURR_IDLE != sHisState[clunum])))                             /*由充放电转为待机*/
            {
                /*电池组历史放电电量写EEP*/
                ParaWriteStoreHisChgDhgCap(clunum, 2, &gHisCapEner_56[eHisCapEner56_1HisDhgCap], 1);
                sEepDhgCap[clunum] = gHisCapEner_56[eHisCapEner56_1HisDhgCap];
            }
        }
        else
        {
            if((ABS(gHisCapEner_56[eHisCapEner56_2HisDhgCap], sEepDhgCap[clunum]) >= HISCAP_CHANG_TO_WRITE_EEP) /*达到写EEP变化值*/
                || ((gHisCapEner_56[eHisCapEner56_2HisDhgCap] > sEepDhgCap[clunum])                             /*与EEP值不一致*/
                && (eCURR_IDLE == nowState) && (eCURR_IDLE != sHisState[clunum])))                             /*由充放电转为待机*/
            {
                /*电池组历史放电电量写EEP*/
                ParaWriteStoreHisChgDhgCap(clunum, 2, &gHisCapEner_56[eHisCapEner56_2HisDhgCap], 1);
                sEepDhgCap[clunum] = gHisCapEner_56[eHisCapEner56_2HisDhgCap];
            }
        }
        sHisChgCap[clunum] = chgCap;
        sHisDhgCap[clunum] = dhgCap;
        sHisState[clunum] = nowState;
    }
}
/*=============================================================================================
** @Name      : void GroupHisEnerStatisTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组累计历史充放电电能统计任务
** @The notes : 400ms
**===========================================================================================*/
void GroupHisEnerStatisTask(void)
{
    u32 chgEner = 0;            /*充电电能(0.1KWH)*/
    u32 dhgEner = 0;            /*放电电能(0.1KWH)*/
    u16 nowState = 0;           /*当前充放电状态*/
    u32 chgChangEner = 0;       /*充电电能变化量(0.1KWH)*/
    u32 dhgChangEner = 0;       /*放电电能变化量(0.1KWH)*/
    static u16 sHisState[2] = {0};   /*历史充放电状态*/
    static u32 sHisChgEner[2] = {0}; /*记录上次充电电能(0.1KWH)*/
    static u32 sHisDhgEner[2] = {0}; /*记录上次放电电能(0.1KWH)*/
    static u32 sEepChgEner[2] = {0}; /*上次写EEP充电电能(0.1KWH)*/
    static u32 sEepDhgEner[2] = {0}; /*上次写EEP放电电能(0.1KWH)*/

    if(gHisCapEner_56 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gHisCapEner_56 null!!!\r\n", __func__);
        return;
    }

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        chgEner = GetChgIntEnerAPI(clunum);
        dhgEner = GetDhgIntEnerAPI(clunum);
        nowState = GetGChgDhgStateAPI(clunum);

        if(chgEner > sHisChgEner[clunum])
        {
            chgChangEner = chgEner - sHisChgEner[clunum];
        }
        else
        {
            chgChangEner = 0;       /*历史值错误则抛弃本次计算值*/
        }

        if(dhgEner > sHisDhgEner[clunum])
        {
            dhgChangEner = dhgEner - sHisDhgEner[clunum];
        }
        else
        {
            dhgChangEner = 0;      /*历史值错误则抛弃本次计算值*/
        }

        if(clunum == 0)
        {
            gHisCapEner_56[eHisCapEner56_1HisChgEner] += chgChangEner;
            gHisCapEner_56[eHisCapEner56_1HisDhgEner] += dhgChangEner;
        }
        else
        {
            gHisCapEner_56[eHisCapEner56_2HisChgEner] += chgChangEner;
            gHisCapEner_56[eHisCapEner56_2HisDhgEner] += dhgChangEner;
        }

        /*保证放电能不能多于充电能多于一个总电能
        if(gGStatistHisEner_74[eHisEner74_HisDhgEner] > (gGStatistHisEner_74[eHisEner74_HisChgEner] + gGBmuGenPara_102[eBmuGenPara102_ReaAllEner]))
        {
            gGStatistHisEner_74[eHisEner74_HisDhgEner] = gGStatistHisEner_74[eHisEner74_HisChgEner] + gGBmuGenPara_102[eBmuGenPara102_ReaAllEner];
        }*/

        if(clunum == 0)
        {
            if((ABS(gHisCapEner_56[eHisCapEner56_1HisChgEner], sEepChgEner[clunum]) >= HISENER_CHANG_TO_WRITE_EEP)/*达到写EEP变化值*/
                || ((gHisCapEner_56[eHisCapEner56_1HisChgEner] > sEepChgEner[clunum])                            /*与EEP值不一致*/
                && (eCURR_IDLE == nowState) && (eCURR_IDLE != sHisState[clunum])))                                /*由充放电转为待机*/
            {
                /*电池组历史充电电能写EEP*/
                ParaWriteStoreHisChgDhgEner(clunum, 1, &gHisCapEner_56[eHisCapEner56_1HisChgEner], 1);
                sEepChgEner[clunum] = gHisCapEner_56[eHisCapEner56_1HisChgEner];
            }
        }
        else
        {
            if((ABS(gHisCapEner_56[eHisCapEner56_2HisChgEner], sEepChgEner[clunum]) >= HISENER_CHANG_TO_WRITE_EEP)/*达到写EEP变化值*/
                || ((gHisCapEner_56[eHisCapEner56_2HisChgEner] > sEepChgEner[clunum])                            /*与EEP值不一致*/
                && (eCURR_IDLE == nowState) && (eCURR_IDLE != sHisState[clunum])))                                /*由充放电转为待机*/
            {
                /*电池组历史充电电能写EEP*/
                ParaWriteStoreHisChgDhgEner(clunum, 1, &gHisCapEner_56[eHisCapEner56_2HisChgEner], 1);
                sEepChgEner[clunum] = gHisCapEner_56[eHisCapEner56_2HisChgEner];
            }
        }
        if(clunum == 0)
        {
            if((ABS(gHisCapEner_56[eHisCapEner56_1HisDhgEner], sEepDhgEner[clunum]) >= HISENER_CHANG_TO_WRITE_EEP)/*达到写EEP变化值*/
                || ((gHisCapEner_56[eHisCapEner56_1HisDhgEner] > sEepDhgEner[clunum])                             /*与EEP值不一致*/
                && (eCURR_IDLE == nowState) && (eCURR_IDLE != sHisState[clunum])))                                 /*由充放电转为待机*/
            {
                /*电池组历史放电电能写EEP*/
                ParaWriteStoreHisChgDhgEner(clunum, 2, &gHisCapEner_56[eHisCapEner56_1HisDhgEner], 1);
                sEepDhgEner[clunum] = gHisCapEner_56[eHisCapEner56_1HisDhgEner];
            }
        }
        else
        {
            if((ABS(gHisCapEner_56[eHisCapEner56_2HisDhgEner], sEepDhgEner[clunum]) >= HISENER_CHANG_TO_WRITE_EEP)/*达到写EEP变化值*/
                || ((gHisCapEner_56[eHisCapEner56_2HisDhgEner] > sEepDhgEner[clunum])                             /*与EEP值不一致*/
                && (eCURR_IDLE == nowState) && (eCURR_IDLE != sHisState[clunum])))                                 /*由充放电转为待机*/
            {
                /*电池组历史放电电能写EEP*/
                ParaWriteStoreHisChgDhgEner(clunum, 2, &gHisCapEner_56[eHisCapEner56_2HisDhgEner], 1);
                sEepDhgEner[clunum] = gHisCapEner_56[eHisCapEner56_2HisDhgEner];
            } 
        }

        sHisChgEner[clunum] = chgEner;
        sHisDhgEner[clunum] = dhgEner;
        sHisState[clunum] = nowState;
    }
}
/*=============================================================================================
** @Name      : void ClrGroupEnerCapStatisInfo(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除充放电电能电量统计值
** @The notes : 单次/累计/电能/电量
**===========================================================================================*/
void ClrGroupEnerCapStatisInfo(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    if(gHisCapEner_56 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gHisCapEner_56 null!!!\r\n", __func__);
        return;
    }
    if(clunum == 0)
    {
        gHisCapEner_56[eHisCapEner56_1HisChgCap] = 0;
        gHisCapEner_56[eHisCapEner56_1HisDhgCap] = 0;
        gHisCapEner_56[eHisCapEner56_1HisChgEner] = 0;
        gHisCapEner_56[eHisCapEner56_1HisDhgEner] = 0;
    }
    else
    {
        gHisCapEner_56[eHisCapEner56_2HisChgCap] = 0;
        gHisCapEner_56[eHisCapEner56_2HisDhgCap] = 0;
        gHisCapEner_56[eHisCapEner56_2HisChgEner] = 0;
        gHisCapEner_56[eHisCapEner56_2HisDhgEner] = 0;
    }
}
