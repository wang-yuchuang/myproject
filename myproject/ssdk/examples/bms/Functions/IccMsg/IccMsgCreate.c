/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : IccMsgCreate.c
** @Founder      : ZQD808
** @Date         : 2026.01.16
** @Function     : 核间通信通道创建模块
** @Instructions : 创建核间通信通道
**===========================================================================================*/
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "IccMsgCreate.h"
#include "IccMsgHandle.h"
#include "IccMsgInterface.h"
#include "IccMsgTxCycle.h"
#include "iccmsg.h"
#include <CLI.h>
#include <debug.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "BSPTypeDef.h"
#include <malloc.h>
#include "GroupInfo.h"
/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
//暂时先创建4个核间通信通道，后续根据测试结果变动
iccmsg_t *iccmsg_cell = NULL;   //单体信息通道句柄
iccmsg_t *iccmsg_cycle = NULL;  //周期信息通道句柄
iccmsg_t *iccmsg_faq = NULL;    //问答信息通道句柄
iccmsg_t *iccmsg_update = NULL; //周期信息通道句柄
// iccmsg_t *iccmsg_pcs = NULL;    //模块周期信息通道句柄
// iccmsg_t *iccmsg_pcscfg = NULL; //模块配置信息通道句柄

extern u16 *gGCellVoltInfo_1;        /*组1电池单体电压信息*/
extern u16 *gGChgBalanInfo_1;


/*=================================================================================================
** @Static function declaration
**===============================================================================================*/

/*=================================================================================================
** @Interface function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : void IccMsgInit(void)
** @Input     : void
** @Output    : void
** @Function  : 创建核间通信通道
** @The notes :
**===============================================================================================*/
void IccMsgInit(void)
{
    uint32_t i = 0;
    frame_conf_t frm = {0};
    
    iccmsg_rx_msg_t *rx_msg = NULL;

    iccmsg_init();

    //单体信息通道配置
    frm.head_len = 2;
    frm.tail_len = 2;
    memset(frm.frame_head,ICCMSG_CELL_MSG_HEAD,frm.head_len);
    memset(frm.frame_tail,ICCMSG_CELL_MSG_TAIL,frm.tail_len);
    frm.checksum_type = CRC32;


    if(iccmsg_cell == NULL)//防止重复初始化
    {
        iccmsg_cell = iccmsg_add(ICCMSG_CELL_MSG_REMOTE_NAME,frm,NULL,NULL);
        if(iccmsg_cell == NULL){
            printf( "add iccmsg %s failed\n", ICCMSG_CELL_MSG_REMOTE_NAME);
        }else{
            printf( "add iccmsg %s successed\n", ICCMSG_CELL_MSG_REMOTE_NAME);
        }
    }

    //周期信息通道配置
    frm.head_len = 2;
    frm.tail_len = 2;
    memset(frm.frame_head,ICCMSG_CYCLE_MSG_HEAD,frm.head_len);
    memset(frm.frame_tail,ICCMSG_CYCLE_MSG_TAIL,frm.tail_len);
    frm.checksum_type = CRC32;

    if(iccmsg_cycle == NULL)//防止重复初始化
    {
        iccmsg_cycle = iccmsg_add(ICCMSG_CYCLE_MSG_REMOTE_NAME,frm,NULL,NULL);
        if(iccmsg_cycle == NULL){
            printf( "add iccmsg %s failed\n", ICCMSG_CYCLE_MSG_REMOTE_NAME);
        }else{
            printf( "add iccmsg %s successed\n", ICCMSG_CYCLE_MSG_REMOTE_NAME);
        }
        
    }

    //问答信息通道配置
    frm.head_len = 2;
    frm.tail_len = 2;
    memset(frm.frame_head,ICCMSG_FAQ_MSG_HEAD,frm.head_len);
    memset(frm.frame_tail,ICCMSG_FAQ_MSG_TAIL,frm.tail_len);
    frm.checksum_type = CRC32;

    if(iccmsg_faq == NULL)//防止重复初始化
    {
        iccmsg_faq = iccmsg_add(ICCMSG_FAQ_MSG_REMOTE_NAME,frm,NULL,NULL);
        if(iccmsg_faq == NULL){
            printf( "add iccmsg %s failed\n", ICCMSG_FAQ_MSG_REMOTE_NAME);
        }else{
            printf( "add iccmsg %s successed\n", ICCMSG_FAQ_MSG_REMOTE_NAME);
        }
    }

    //升级信息信息通道配置
    frm.head_len = 2;
    frm.tail_len = 2;
    memset(frm.frame_head,ICCMSG_UPDATE_MSG_HEAD,frm.head_len);
    memset(frm.frame_tail,ICCMSG_UPDATE_MSG_TAIL,frm.tail_len);
    frm.checksum_type = CRC32;

    if(iccmsg_update == NULL)//防止重复初始化
    {
        iccmsg_update = iccmsg_add(ICCMSG_UPDATE_MSG_REMOTE_NAME,frm,NULL,NULL);
        if(iccmsg_update == NULL){
            printf( "add iccmsg %s failed\n", ICCMSG_UPDATE_MSG_REMOTE_NAME);
        }else{
            printf( "add iccmsg %s successed\n", ICCMSG_UPDATE_MSG_REMOTE_NAME);
        }
    }
    

    // //模块周期信息信息通道配置
    // frm.head_len = 2;
    // frm.tail_len = 2;
    // memset(frm.frame_head,ICCMSG_PCS_MSG_HEAD,frm.head_len);
    // memset(frm.frame_tail,ICCMSG_PCS_MSG_TAIL,frm.tail_len);
    // frm.checksum_type = CRC32;

    // if(iccmsg_pcs == NULL)//防止重复初始化
    // {
    //     iccmsg_pcs = iccmsg_add(ICCMSG_PCS_MSG_REMOTE_NAME,frm,NULL,NULL);
    //     if(iccmsg_pcs == NULL){
    //         printf( "add iccmsg %s failed\n", ICCMSG_PCS_MSG_REMOTE_NAME);
    //     }else{
    //         printf( "add iccmsg %s successed\n", ICCMSG_PCS_MSG_REMOTE_NAME);
    //     }
    // }

    // //模块配置信息信息通道配置
    // frm.head_len = 2;
    // frm.tail_len = 2;
    // memset(frm.frame_head,ICCMSG_PCSCFG_MSG_HEAD,frm.head_len);
    // memset(frm.frame_tail,ICCMSG_PCSCFG_MSG_TAIL   ,frm.tail_len);
    // frm.checksum_type = CRC32;

    // if(iccmsg_pcscfg == NULL)//防止重复初始化
    // {
    //     iccmsg_pcscfg = iccmsg_add(ICCMSG_PCSCFG_MSG_REMOTE_NAME,frm,NULL,NULL);
    //     if(iccmsg_pcscfg == NULL){
    //         printf( "add iccmsg %s failed\n", ICCMSG_PCSCFG_MSG_REMOTE_NAME);
    //     }else{
    //         printf( "add iccmsg %s successed\n", ICCMSG_PCSCFG_MSG_REMOTE_NAME);
    //     }
    // }

}
#if 0
u8 BattTemp[240] = {0};
u8 BattVolt[816] = {0};


/*=================================================================================================
** @Static function definition
**===============================================================================================*/
static int iccmsg_create_test(int argc, char *argv[])
{

    if (argc == 0) {
        printf("iccmsg create test para error\n");
        return CLI_FALSE;
    }
    if (!strcmp(argv[0], "init"))
    {
        IccMsgInit();
        return 0;
    }
    else if(!strcmp(argv[0], "list"))
    {
        iccmsg_list_all();
        return 0;
    }
    else if(!strcmp(argv[0], "sends"))
    {
        for(u16 i = 0; i < 240; i++)
        {
            BattTemp[i] = 25;
        }
        iccmsg_send(iccmsg_faq, 0, 0, BattTemp, sizeof(BattTemp), ICCMSG_ACK_NO_NEED);
        return 0;
    }
    else if(!strcmp(argv[0], "sendm"))
    {
        for(u16 i = 0; i < 816; i++)
        {
            BattVolt[i] = 25;
        }
        for(u32 j = 0; j < 20000; j++)
        {
            
            iccmsg_send(iccmsg_cell, 0, 0, gGCellVoltInfo_1, GINDEX92_LEN, ICCMSG_ACK_NO_NEED);
            iccmsg_send(iccmsg_faq, 0, 0, gGChgBalanInfo_1, GINDEX90_LEN, ICCMSG_ACK_NO_NEED);
        }
        
        return 0;
    }
    else
    {
        return 0;
    }
}

CLI_CMD("iccmsg_create_test",
        "iccmsg_create_test"
        "\r\n\t iccmsg_demo init/list/send",
        iccmsg_create_test);
#endif


